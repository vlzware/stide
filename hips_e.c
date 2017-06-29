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

#include "helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void print_usage();

int main(int argc, char *argv[])
{
	// check if properly called
    if ( argc != 3 )
    {
        print_usage();
		return 1;
    }

	// seed using hash from the password
	uint32_t passI = hash(argv[1]);
	srand(passI);
	
	printf("Hashed pass: %u\n",passI);

    char *inImage = argv[2];

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

	// pixel count
	imgPixels = imgWidth * imgHeight;

	// create the pixel-index array and shuffle it
	uint32_t* pixelArray = shuffle(imgPixels);

    // temporary variables and buffers
    uint8_t text_in_lsb[14];
    uint8_t bit_pos = 0;
    uint8_t all_read = 0;
    char str[6];
    char *res = NULL;
    uint16_t id = 0;

    // process and show (bold and blue)
    printf("\nHidden message:\n");
   	//printf("\033[01;34m"); // not working in the gtk+ window

	uint16_t stopAt = (imgPixels > MAX_TOKENS_BITS)? MAX_TOKENS_BITS : imgPixels;
	uint32_t pos;
	uint8_t rgb;
	for (uint16_t i = 0; i < stopAt; i++)
	{
		pos = (imgBpp == 3)? (pixelArray[i]*3) : (pixelArray[i]*4);

		// calculate channel
		rgb = rand_at_most(2);

		// extract and xor with the "random" bit
		text_in_lsb[bit_pos] = imgRGB[pos + rgb] ^ (1 & rand());

		// advance in the text
		bit_pos++;
		
		// whole word read
		if (bit_pos == 14)
		{
			bit_pos = 0;
			id = 0;
			
			// build the int from the last bits
			for (int8_t i = 13; i >= 0; i--)
			{
				id ^= (-(text_in_lsb[13-i] & 1) ^ id) & (1 << i);
			}
			if (id == EOF_HIPS) // eof_hips
			{
				all_read = 1;
			} else              // print current word
			{
				sprintf(str, "%i", id);
				sql_get(str, NULL, &res);
				if (!res)
				{
					//printf("\033[01;31m");
					printf("\n---ID not in the dictionary! Maybe wrong image/password?---\n");
					//printf("\033[34m");
				}
				else
					printf("%s ",res);
			}
		}

		// break if EOF
		if (all_read)
			break;
	}
   	//printf("\033[00m");
    printf("\n\nDone!\n");
    
    // free memory
    if (res)
        free(res);
    free(pixelArray);
    
    // close infile
    stbi_image_free(imgRGB);

    // success
    return 0;
}


/**
 * prints usage
 */
void print_usage()
{
    printf("Usage:\n");
    printf("./hips_e password image\n");
}
