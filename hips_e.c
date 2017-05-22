/**
 * HIPS a.k.a. Hide In Plain Sight
 * A steganography tool with compression, encryption,
 * random and (almost) non-invasive text insertion
 * 
 * made for my CS50 final project but mostly for fun
 * Vladimir Zhelezarov
 * jelezarov.vladimir@gmail.com
 * 
 * hips_e(xtract)
 * extracts the hidden text from the steganography image,
 * decrypting and decompressing it
 * 
 * based on the CS50 pset4 2017
 * https://cs50.harvard.edu/
 * also using dictionary (10 000 most common words) from:
 * https://github.com/first20hours/google-10000-english
 */
       
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "helpers.h"

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

    // open image file 
    FILE *inptr = fopen(inImage, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", inImage);
        return 2;
    }
    
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // temporary variables and buffers
    uint8_t text_in_lsb[14];
    uint8_t bit_pos = 0;
    uint32_t pos_in_file = 0;
    uint8_t rgb;
    uint8_t all_read = 0;
    char str[6];
    char *res;
    uint16_t id = 0;

    // process
    printf("\nHidden message:\n");

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // embed the text in the apropriate LSB
            if ( (pos_in_file >= startAt) && (all_read == 0) )
            {
                rgb = rand_at_most(2);
                switch (rgb)
                {
                    case 0:
                        text_in_lsb[bit_pos] = triple.rgbtRed   ^ (1 & rand());
                        break;
                    case 1:
                        text_in_lsb[bit_pos] = triple.rgbtGreen ^ (1 & rand());
                        break;
                    case 2:
                        text_in_lsb[bit_pos] = triple.rgbtBlue  ^ (1 & rand());
                        break;
                }
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
                            printf("\n---ID not in the dictionary! Maybe wrong bmp?---\n");
                        else
                            printf("%s ",res);
                    }
                }
            }
            pos_in_file++;
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
    }
    printf("\n");
    
    // free memory
    if (res)
        free(res);
    
    // close infile
    fclose(inptr);

    // success
    return 0;
}


/**
 * prints usage
 */
void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "./hips_e password image\n");
}
