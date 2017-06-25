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

	// TODO: calculate freeS from the bitmap
	// free space in the bitmap after inserting the text
	uint16_t freeS = 100;

	// seed using hash from the password
	uint32_t passI = hash(argv[1]);
	srand(passI);
	
	// starting position of the embedded text
	uint16_t startAt = rand_at_most(freeS);
	
	printf("Hashed pass: %u\n",passI);
	printf("Starting at pos: %i\n", startAt);

    char *inImage = argv[2];

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

    // temporary variables and buffers
    uint8_t text_in_lsb[14];
    uint8_t bit_pos = 0;
    uint8_t rgb;
    uint8_t all_read = 0;
    char str[6];
    char *res;
    uint16_t id = 0;

    // process
    printf("\nHidden message:\n");

    // imgRGB is like R(8 bits), G(8 bits), B(8 bits) and evtl. A(8 bits)
	for (int i = startAt*imgBpp*8, k = imgHeight*imgWidth*imgBpp; i < k;)
	{
		if (all_read == 1)
			break;
		rgb = rand_at_most(2);
		switch (rgb)
		{
			case 0:	// R
//				printf("r");
				text_in_lsb[bit_pos] = imgRGB[i] ^ (1 & rand());
				break;
			case 1:	// G
//				printf("g");
				text_in_lsb[bit_pos] = imgRGB[i + 8] ^ (1 & rand());
				break;
			case 2:	// B
//				printf("b");
				text_in_lsb[bit_pos] = imgRGB[i + 16]  ^ (1 & rand());
				break;
		}
		bit_pos++;
		i += imgBpp*8;

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
					printf("\n---ID not in the dictionary! Maybe wrong image/password?---\n");
				else
					printf("%s ",res);
			}
		}
	}
    printf("\n");
    
    // free memory
    if (res)
        free(res);
    
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
