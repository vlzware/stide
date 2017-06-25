/**
 * HIPS a.k.a. Hide In Plain Sight
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

	// TODO: calculate freeS from the bitmap's
	// remaining free space after inserting the text
	uint16_t freeS = 100;

	// seed using hash from the password
	uint32_t passI = hash(argv[1]);
	srand(passI);
	
	// starting position of the embedded text
	uint16_t startAt = rand_at_most(freeS);
	
	printf("Hashed pass: %u\n",passI);
	printf("Starting at pos: %i\n", startAt);

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
	int imgWidth, imgHeight, imgBpp;
	uint8_t* imgRGB = stbi_load( inImage, &imgWidth, &imgHeight, &imgBpp, 0 );
	if (imgRGB == NULL || imgBpp < 3)
	{
		printf("Could not load %s!\n",inImage);
		return 2;
	}
	printf("Input image loaded\n");

    // tokenize the text to hide
    char **text_to_hide = NULL;
    uint8_t count_tokens = 0;
    count_tokens = tokenize(argv[2], &text_to_hide);

    printf("Tokenizing input: ");
    printf("found %d tokens.\n", count_tokens);

    /* create array with (count + eof)  * 14 * 1111111x ints
    where x is the next bit from the text to hide */
    uint8_t *text_in_lsb;
    text_in_lsb = malloc( sizeof(uint8_t) * 14 * (count_tokens + 1) );
    if (text_in_lsb == NULL)
    {
        printf("Memory error\n");
        free(text_to_hide);
        return 3;
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
            return 4;
        }
        printf("id found in dictionary: %s\n", res);
        uint16_t id = (uint16_t) atoi(res);
        for (int8_t k = 13; k >= 0; k--)
        {
            text_in_lsb[bit_pos] = 1 & ( (id >> k) );
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
    
    // random number in [0,1,2]
    uint8_t rgb;

    // using 14 bits for every word in the dictionary
    uint32_t bits_text = (count_tokens + 1) * 14;
    printf("bits_text: %i\n",bits_text);
    
    // TODO: disperse the text in the whole file
    uint32_t stopAt = startAt + bits_text;
    
    // process the loaded image first in memory
    printf("Hiding route:  \n");

    // imgRGB is like R(8 bits), G(8 bits), B(8 bits) and evtl. A(8 bits)
	for (int i = startAt*imgBpp*8, k = stopAt*imgBpp*8, j = imgWidth*imgHeight*imgBpp; (i < k && i < j);)
	{
		rgb = rand_at_most(2);
		/* change the last bit: http://stackoverflow.com/a/47990/6049386
		and XOR it with value from the PRNG */
		switch (rgb)
		{
			case 0:
				printf("r");
				// first byte from the pixel is R
				imgRGB[i]   = ((imgRGB[i] ^ ( -text_in_lsb[bit_pos] ^ imgRGB[i] )) & 1) ^ (1 & rand());
				break;
			case 1:
				printf("g");
				// second byte from the pixel is G
				imgRGB[i + 8]   = ((imgRGB[i + 8] ^ ( -text_in_lsb[bit_pos] ^ imgRGB[i + 8] )) & 1) ^ (1 & rand());
				break;
			case 2:
				printf("b");
				// third byte from the pixel is B
				imgRGB[i + 16]   = ((imgRGB[i + 16] ^ ( -text_in_lsb[bit_pos] ^ imgRGB[i + 16] )) & 1) ^ (1 & rand());
				break;
		}
		bit_pos++;
		i += imgBpp*8;
	}

	// save the processes image to file
	if (outExt == extPNG)
	{
		if ( !stbi_write_png(outImage, imgWidth, imgHeight, imgBpp, imgRGB, imgWidth*imgBpp) )
		{
			printf("Could not write %s!\n", outImage);
			stbi_image_free(imgRGB);
			free(text_to_hide);
			free(text_in_lsb);
			return 5;
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
			return 6;
		}
	}

    printf("\n\nDone!\n");

    // clear memory
    stbi_image_free(imgRGB);
    free(text_to_hide);
    free(text_in_lsb);

    // success
    return 0;
}

/**
 * prints usage
 */
void print_usage()
{
    printf("Usage:\n");
    printf("./hips_c password text imageIn imageOut.[jpg]\n");
}
