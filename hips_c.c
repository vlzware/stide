/**
 * HIPS a.k.a. Hide In Plain Sight
 * A steganography tool with compression, encryption,
 * random and (almost) non-invasive text insertion
 * 
 * made for my CS50 final project but mostly for fun
 * Vladimir Zhelezarov
 * jelezarov.vladimir@gmail.com
 * 
 * hips_c(reate)
 * hide message in a bitmap using compression and encryption
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
    if ( argc != 4 )
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
    char *outImage = "out.bmp";

    // open image file 
    FILE *inptr = fopen(inImage, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", inImage);
        return 2;
    }
    
    // open output image
    FILE *outptr = fopen(outImage, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outImage);
        return 3;
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
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

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
        fprintf(stderr, "Memory error\n");
        fclose(inptr);
        fclose(outptr);
        free(text_to_hide);
        return 5;
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
            fprintf(stderr, "Token not in the dictionary!\n");
            fclose(inptr);
            fclose(outptr);
            free(text_to_hide);
            free(text_in_lsb);
            return 6;
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
    
    // add eof (10005)
    for (int8_t k = 13; k >=0; k--)
    {
        text_in_lsb[bit_pos] = 1 & ( (EOF_HIPS >> k) );
        bit_pos++;
    }

    // position watchers
    uint32_t pos_in_file = 0;
    bit_pos = 0;
    uint8_t rgb;

    // using 14 bits for every word in the dictionary
    uint32_t bits_text = (count_tokens + 1) * 14;
    printf("bits_text: %i\n",bits_text);
    
    // TODO: disperse the text in the whole file
    uint32_t stopAt = startAt + bits_text;
    
    // process
    printf("Hiding route:  \n");

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
            if ( (pos_in_file >= startAt) && (pos_in_file < stopAt) )
            {
                rgb = rand_at_most(2);
                switch (rgb)
                {
                    case 0:
                        printf("r");
                        /* change the last bit: http://stackoverflow.com/a/47990/6049386
                        and XOR it with value from the PRNG */
                        triple.rgbtRed   = ((triple.rgbtRed   ^ ( -text_in_lsb[bit_pos] ^ triple.rgbtRed   )) & 1) ^ (1 & rand());
                        break;
                    case 1:
                        printf("g");
                        triple.rgbtGreen = ((triple.rgbtGreen ^ ( -text_in_lsb[bit_pos] ^ triple.rgbtGreen )) & 1) ^ (1 & rand());
                        break;
                    case 2:
                        printf("b");
                        triple.rgbtBlue  = ((triple.rgbtBlue  ^ ( -text_in_lsb[bit_pos] ^ triple.rgbtBlue  )) & 1) ^ (1 & rand());
                        break;
                }
                bit_pos++;
            }
            
            // copy RGB triple to outfile
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
            
            pos_in_file++;
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

        // then add it back
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }
    
    printf("\n\nDone!\n");

    // clear memory
    fclose(inptr);
    fclose(outptr);
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
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "./hips_c password text image\n");
}
