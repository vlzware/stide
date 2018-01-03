/**
 * stide_c:
 * 	embed hidden text in an image,
 * 	ecrypting and as much as possible compressing it.
 *
 * author:
 * 	jelezarov.vladimir@gmail.com
 *
 * - started as final project on CS50 2017
 * 	https://cs50.harvard.edu/
 * - using a dictionary (10 000 most common words) from:
 * 	https://github.com/first20hours/google-10000-english
 * - the stb library from here:
 * 	https://github.com/nothings/stb/
 * - and libpng:
 * 	http://libpng.org/pub/png/libpng.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include "helpers.h"

#define extBMP 1
#define extPNG 2

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

const char *prog = "stide_c";           /* program name */
char *pass;                             /* password */
char *payload;				/* text to hide */
char *image_in;                         /* input image */
char *image_out;                        /* output image */
int strict = 0;                         /* strict mode */
int verbose = 0;			/* be verbose */
int debug = 0;                          /* print debugging messages */
int print_dist = 0;			/* show distribution */
uint8_t out_ext;			/* extension of image_out */

int process_strict(uint32_t *b_text, uint8_t **t_in_lsb, int img_pix);
int process_all(uint32_t *b_text, uint8_t **t_in_lsb, int img_pix);
void print_usage(void);


/**
 * stide_c: embed hidden text in a image
 * Usage: ./stide_c [-nds] password image-in [image-out[.png]]
 *
 * parameters:
 *  s: "strict" mode - do not work with the dictionary only - embed
 *      arbitrary words/characters;
 *  p: print distribution - output the image with fat visible changes.
 * 	Note: Using the programm with 's' does not hide anything;
 *  v: be verbose;
 *  d: be even more verbose.
 */
int main(int argc, char *argv[])
{
	/*
	 * get, check and set parameters
	 */
	int c;
	while (--argc > 0 && (*++argv)[0] == '-')
		while ((c = *++argv[0]))
			switch (c) {
			case 's':
				strict = 1;
				break;
			case 'p':
				print_dist = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'd':
				debug = 1;
				verbose = 1;
				break;
			default:
				printf("%s: illegal option %c\n", prog, c);
				argc = 0;
				break;
	}
	if (argc == 3) {
		pass = argv[0];
		payload = argv[1];
		image_in = argv[2];
		image_out = "out.png";
		out_ext = extPNG;
	} else if (argc == 4) {
		pass = argv[0];
		payload = argv[1];
		image_in = argv[2];
		image_out = argv[3];
		/* check file extension and default to .bmp */
		out_ext = (strcmp("png", get_filename_ext(image_out)) == 0)?
			extPNG : extBMP;
	} else {
		print_usage();
		exit(1);
	}


	/*
	 * seed using hash from the password
	 */
	uint32_t pass_hash = hash(pass);
	srand(pass_hash);
	if (verbose)
		printf("Hashed pass: %u\n",pass_hash);


	/*
	 * load the input image in memory
	 */
	if (verbose)
		printf("Loading image...\n");
	int img_width, img_height, img_bpp, img_pixels;
	uint8_t* imgRGB = stbi_load(image_in, &img_width,
				&img_height, &img_bpp, 0);

	if (imgRGB == NULL || img_bpp < 3) {
		printf("%s: Could not load %s!\n", prog, image_in);
		exit(2);
	}
	printf("Input image loaded\n");
	printf("Processing...\n");

	/* calculate pixel count */
	img_pixels = img_width * img_height;

	/* create the pixel-index array and shuffle it */
	uint32_t* pixel_array = shuffle(img_pixels);


	/*
	 * preprocess the payload
	 */
	uint32_t bits_text;			/* text bits counter */
	uint8_t *text_in_lsb = NULL;		/* payload holder */

	strict = 1;

	int res = strict
		? process_strict(&bits_text, &text_in_lsb, img_pixels)
		: process_all(&bits_text, &text_in_lsb, img_pixels);

	if (res != 0) {
		free(pixel_array);
		free(text_in_lsb);
		free(pixel_array);
		stbi_image_free(imgRGB);
		exit(res);
	}

	/*
	 * process the loaded image first in memory
	 */
	if (debug)
		printf("Hiding route (pixel/channel):  \n");
	uint32_t pos;
	uint8_t rgb;
	uint32_t bit_pos;
	for (bit_pos = 0; bit_pos < bits_text; bit_pos++) {
		if (verbose)
			printf("%i", pixel_array[bit_pos]);

		pos = (img_bpp == 3)?
			(pixel_array[bit_pos] * 3) : (pixel_array[bit_pos] * 4);

		/* calculate channel */
		rgb = rand_at_most(2);

		if (print_dist) {
			/* make big fat pixels to show distribution */
			imgRGB[pos + (rgb + 1)%3] = 0;
			imgRGB[pos + (rgb + 2)%3] = 0;
			imgRGB[pos+3] = 255;
		} else {
			/* embed and xor with random number */
			imgRGB[pos + rgb] = (
				(imgRGB[pos + rgb] & ~1)
				| (text_in_lsb[bit_pos] ^ (1 & random()))
			);
		}
		if (debug)
			switch (rgb) {
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
	}


	/*
	 * save the processed image to file
	 */
	if (out_ext == extPNG) {
		if (!stbi_write_png(image_out, img_width, img_height, img_bpp,
				imgRGB, img_width*img_bpp)) {
			printf("%s: Could not write %s!\n", prog, image_out);
			stbi_image_free(imgRGB);
			free(text_in_lsb);
			free(pixel_array);
			return 6;
		}
	} else {
		/* defaults to bmp */
		if ( !stbi_write_bmp(image_out, img_width, img_height, img_bpp,
				imgRGB) ) {
			printf("Could not write %s!\n", image_out);
			stbi_image_free(imgRGB);
			free(text_in_lsb);
			free(pixel_array);
			return 7;
		}
	}


	/*
	 * housekeeping
	 */
	stbi_image_free(imgRGB);
	free(text_in_lsb);
	free(pixel_array);

	/*
	 * success
	 */
	printf("\n\nDone!\n");
	return 0;
}

/**
 * prints usage
 */
void print_usage(void)
{
	printf("Usage:\n");
	printf("%s [-spvd] password text image-in [image-out[.png]]\n", prog);
	printf("\ts: enable strict mode\n");
	printf("\tv: be verbose\n");
	printf("\td: be even more verbose (this includes 'v')\n");
	printf("\tp: print distribution - not (!) hiding anything\n");
}

/**
 * process_strict: embed the payload using only words from the dictionary
 */
int process_strict(uint32_t *b_text, uint8_t **t_in_lsb, int img_pix)
{
	/* tokenize the text to hide */
	char **text_to_hide = NULL;
	uint8_t count_tokens = 0;
	count_tokens = tokenize(payload, &text_to_hide);

	if (verbose) {
		printf("Tokenizing input: ");
		printf("found %d tokens.\n", count_tokens);
	}

	/* 14 bits for every word from the dictionary plus one for EOF */
	*b_text = (count_tokens + 1) * 14;
	if (verbose)
		printf("bits_text: %i\n", *b_text);

	/* text + EOF too long? */
	if (*b_text > img_pix) {
		printf("%s: The input is too long for the given image!\n",
			prog);
		free(text_to_hide);
		return 3;
	}

	/*
	 * create array with (count + eof)  * 14 * 1111111x chars
	 * where x is the next bit from the text to hide
	 */

	*t_in_lsb = (uint8_t *) malloc(sizeof(uint8_t) * (*b_text));
	if (*t_in_lsb == NULL) {
		printf("%s: Memory error\n", prog);
		free(text_to_hide);
		return 4;
	}

	/* temporary variables and position watcher */
	char *res;
	uint32_t bit_pos = 0;
	int i, k;

	/* embed the id's from the words */
	for (i = 0; i < count_tokens; i++) {
		if (verbose)
			printf("token #%d: %s\n", i, text_to_hide[i]);

		sql_get(NULL, text_to_hide[i], &res);
		if (res == NULL) {
			printf("Token not in the dictionary!\n");
			printf("Try another word or use without ");
			printf("strict mode: (-s).\n");
			free(text_to_hide);
			return 5;
		}
		if (verbose)
			printf("id found in dictionary: %s\n", res);

		uint16_t id = (uint16_t) atoi(res);
		for (k = 13; k >= 0; k--) {
			(*t_in_lsb)[bit_pos] = 1 & (id >> k);
			if (debug)
				printB((*t_in_lsb)[bit_pos]);
			bit_pos++;
		}
		if (res != NULL)
			free(res);
	}

	/* add EOF_HIPS */
	int l;
	for (l = 13; l >= 0; l--) {
		(*t_in_lsb)[bit_pos] = 1 & (EOF_HIPS >> l);
		bit_pos++;
	}

	free(text_to_hide);

	return 0;
}

/**
 * process_all: embed the payload allowing arbitrary characters
 */
int process_all(uint32_t *b_text, uint8_t **t_in_lsb, int img_pix)
{

	return 0;
}
