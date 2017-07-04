/**
 * H.I.P.S. a.k.a. Hide In Plain Sight
 * A steganography tool with compression, encryption,
 * random and (almost) non-invasive text insertion
 * 
 * (originally) made for my CS50 final project but mostly for fun
 * Vladimir Zhelezarov
 * jelezarov.vladimir@gmail.com
 * 
 * hips_e(xtract)
 * extracts the hidden text from the steganography image,
 * decrypting and decompressing it
 * 
 * started from the CS50 pset4 2017
 * https://cs50.harvard.edu/
 * also using dictionary (10 000 most common words) from:
 * https://github.com/first20hours/google-10000-english
 * now using the stb library from here:
 * https://github.com/nothings/stb/
 * 
 * UPDATE 29.06.2017
 * - reworked hiding algorithm - now we use the whole file and disperse
 * the bits in randomly (password dependent) choosen order among all colors
 * of all pixels - see out_20-tokens-distribution.png
 * 
 * UPDATE 25.06.2017
 * - some basic gtk+ interface - uses the binaries hips_c and hips_e 
 * in the same directory
 * - now with added support for png, based on the wonderfull libraries
 * from stb (links above)
 * - replaced all fprintf with printf because otherwise the messages
 * appear in the gtk+ interface out of order (TODO: fix and use fprintf)
 */
       
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

#define extBMP 1
#define extPNG 2

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

void print_usage();

int main(int argc, char *argv[])
{

	// check if properly called
    if ( argc != 5 )
    {
        print_usage();
		return 1;
    }

	// seed using hash from the password
	uint32_t passI = hash(argv[1]);
	srand(passI);
	
	printf("Hashed pass: %u\n",passI);

    char *inImage = argv[3];
    char *outImage = argv[4];
    uint8_t outExt = 0;
    
    // check output file type for jpeg or default to png
    if (strcmp("png", get_filename_ext(outImage)) == 0)
		outExt = extPNG;
	else
		outExt = extBMP;

	// load the input image in memory
	printf("Loading image...\n");
	int imgWidth, imgHeight, imgBpp, imgPixels;
	uint8_t* imgRGB = stbi_load( inImage, &imgWidth, &imgHeight, &imgBpp, 0 );
	if (imgRGB == NULL || imgBpp < 3)
	{
		printf("Could not load %s!\n",inImage);
		return 2;
	}
	printf("Input image loaded\n");
	
	// calculate pixel count
	imgPixels = imgWidth * imgHeight;

	// create the pixel-index array and shuffle it
	uint32_t* pixelArray = shuffle(imgPixels);

    // tokenize the text to hide
    char **text_to_hide = NULL;
    uint8_t count_tokens = 0;
    count_tokens = tokenize(argv[2], &text_to_hide);

    printf("Tokenizing input: ");
    printf("found %d tokens.\n", count_tokens);

    // using 14 bits for every word in the dictionary and for EOF
    uint32_t bits_text = (count_tokens + 1) * 14;
    printf("bits_text: %i\n",bits_text);
    
	// text + EOF too long?
	if ( bits_text > imgPixels )
	{
		printf("The input is too long for the given image!\nExiting\n");
		free(text_to_hide);
		free(pixelArray);
		return 3;
	}

    /* create array with (count + eof)  * 14 * 1111111x ints
    where x is the next bit from the text to hide */
    uint8_t *text_in_lsb;
    text_in_lsb = malloc( sizeof(uint8_t) * bits_text );
    if (text_in_lsb == NULL)
    {
        printf("Memory error\n");
        free(text_to_hide);
        free(pixelArray);
        return 4;
    }
    
    // temporary variable and position watcher
    char *res;
    uint32_t bit_pos = 0;
    
    for (int8_t i = 0; i < count_tokens; i++)
    {
        printf("token #%d: %s\n", i, text_to_hide[i]);
        sql_get(NULL, text_to_hide[i], &res);
        if (res == NULL)
        {
            printf("Token not in the dictionary!\n");
            free(text_to_hide);
            free(text_in_lsb);
			free(pixelArray);
            return 5;
        }
        printf("id found in dictionary: %s\n", res);
        uint16_t id = (uint16_t) atoi(res);
        for (int8_t k = 13; k >= 0; k--)
        {
            text_in_lsb[bit_pos] = 1 & ( (id >> k) );
            //printB(text_in_lsb[bit_pos]);
            bit_pos++;
        }
        if (res != NULL)
            free(res);
    }
    
    // add eof - position 10005(EOF_HIPS) in the dictionary
    for (int8_t k = 13; k >=0; k--)
    {
        text_in_lsb[bit_pos] = 1 & ( (EOF_HIPS >> k) );
        bit_pos++;
    }

    // position watcher
    bit_pos = 0;

    // process the loaded image first in memory
    printf("Hiding route (pixel/channel:  \n");

	// embed the text
	uint32_t pos;
	uint8_t rgb;
	for (uint16_t i = 0; i < bits_text; i++)
	{
		printf("%i",pixelArray[i]);
		pos = (imgBpp == 3)? (pixelArray[i]*3) : (pixelArray[i]*4);

		// calculate channel
		rgb = rand_at_most(2);

		// embed and xor with random number
		imgRGB[pos + rgb] = ( (imgRGB[pos + rgb] & ~1) | (text_in_lsb[bit_pos]^ (1 & random())) );

		switch (rgb)
		{
			case 0:
				printf("r  ");
				break;
			case 1:
				printf("g  ");
				break;
			case 2:
				printf("b  ");
				break;
		}		
		
		/*// for testing purposes - shows distribution
		imgRGB[pos + (rgb + 1)%3] = 0;
		imgRGB[pos + (rgb + 2)%3] = 0;
		imgRGB[pos+3] = 255;*/

		// advance in the text
		bit_pos++;
	}
	printf("\n");

	// save the processed image to file
	if (outExt == extPNG)
	{
		if ( !stbi_write_png(outImage, imgWidth, imgHeight, imgBpp, imgRGB, imgWidth*imgBpp) )
		{
			printf("Could not write %s!\n", outImage);
			stbi_image_free(imgRGB);
			free(text_to_hide);
			free(text_in_lsb);
			free(pixelArray);
			return 6;
		}		
	}
	// defaults to bmp
	else
	{
		if ( !stbi_write_bmp(outImage, imgWidth, imgHeight, imgBpp, imgRGB) )
		{
			printf("Could not write %s!\n", outImage);
			stbi_image_free(imgRGB);
			free(text_to_hide);
			free(text_in_lsb);
			free(pixelArray);
			return 7;
		}
	}

    printf("\n\nDone!\n");

    // clear memory
    stbi_image_free(imgRGB);
    free(text_to_hide);
    free(text_in_lsb);
	free(pixelArray);

    // success
    return 0;
}

/**
 * prints usage
 */
void print_usage()
{
    printf("Usage:\n");
    printf("./hips_c password text imageIn imageOut.[png]\n");
}
