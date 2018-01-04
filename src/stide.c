/**
 * stide:
 * 	steganography tool featuring heavy compression,
 * 	ecryption and (almost) non-invasive hiding techniques
 *
 * author:
 * 	jelezarov.vladimir@gmail.com
 *
 * - started as a final project on CS50 2017
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

#include "stide.h"
#include "helpers.h"

/* global variables */
struct parameter param;			      /* parameter structure */
const char *prog = "stide";                   /* program name */

void print_usage(void);

/**
 * stide: embed/extract hidden text in a image
 *
 * Usage: stide -c[spvd] password text image-in [image-out[.png]]
 * Or   : stide -e[svd] password image-in
 *
 * parameters:
 *  c: 'create' - embed text in image;
 *  e: 'extract' - extract text from image;
 *  s: 'strict' mode - do not work with the dictionary only - embed
 *      arbitrary words/characters;
 *  p: print distribution - output the image with fat visible changes;
 *  v: be verbose;
 *  d: be even more verbose.
 */
int main(int argc, char *argv[])
{
        /* image structure */
        struct image img;

	/* defaults */
        param.pass = NULL;
        param.payload = NULL;
        param.image_in = NULL;
        param.image_out = NULL;
	param.strict = 1;
	param.verbose = 1;
	param.debug = 0;
        param.print_dist = 0;
        param.mode = UNSET;

        char def_out[] = "out.png";
        int res;

	/*
	 * get, check and set parameters
	 */
	int c;
	while (--argc > 0 && (*++argv)[0] == '-')
		while ((c = *++argv[0]))
			switch (c) {
                        case 'c':
                                if (param.mode == EXTRACT)
                                        print_usage();
                                param.mode = CREATE;
                                break;
                        case 'e':
                                if (param.mode == CREATE)
                                        print_usage();
                                param.mode = EXTRACT;
                                break;
			case 's':
				param.strict = 1;
				break;
			case 'p':
				param.print_dist = 1;
				break;
			case 'v':
				param.verbose = 1;
				break;
			case 'd':
				param.debug = 1;
				param.verbose = 1;
				break;
			default:
				printf("%s: illegal option %c\n", prog, c);
				param.mode = UNSET;
				break;
	}
        if ((param.mode == EXTRACT) && (argc == 2)) {
                param.image_in = argv[1];
        } else if ((param.mode == CREATE)
                        && (argc == 3 || argc == 4)) {
                param.payload = argv[1];
                param.image_in = argv[2];
                if (argc == 3) {
                        param.image_out = def_out;
                        param.out_ext = extPNG;
                } else {
                        param.image_out = argv[3];
                        /* check file extension and default to .bmp */
                        param.out_ext = (strcmp("png",
                        get_filename_ext(param.image_out)) == 0)?
                        extPNG : extBMP;
                }
	} else {
		print_usage();
	}

        param.pass = argv[0];

	/*
	 * seed using hash from the password
	 */
	uint32_t pass_hash = hash(param.pass);
	srand(pass_hash);
	if (param.verbose)
		printf("Hashed pass: %u\n",pass_hash);

	/*
	 * load the input image in memory
	 */
	if (param.verbose)
		printf("Loading image...\n");
        img_load(&img);
	if (img.rgb == NULL || img.bpp < 3) {
		printf("%s: Could not load %s!\n", prog, param.image_in);
		exit(2);
	}
        /* calculate pixel count */
        img.pixels = img.width * img.height;
	printf("Input image loaded\n");
	printf("Processing...\n");

        /**
         * create or extract
         */
        if (param.mode == CREATE) {
                if ((res = create(&img)) != 0)
                        exit(res);
        } else if (param.mode == EXTRACT) {
                if ((res = extract(&img)) != 0)
                        exit(res);
        }

	/*
	 * housekeeping
	 */
	img_unload(&img);

	/*
	 * success
	 */
	printf("\n\nDone!\n");
	return 0;
}

/*
 * print the help and die
 */
void print_usage(void)
{
	printf("\nUsage:\n");
        printf("%s -c[spvd] password text image-in [image-out[.png]]\n",
                prog);
        printf("\tc: create image embedding the hidden text\n");
        printf("-OR-\n");
        printf("%s -e[svd] password image-in\n",
                prog);
        printf("\ts: extract hidden text from an image\n");
        printf("\nOptional parameters:\n");
        printf("\ts: enable strict mode\n");
	printf("\tv: be verbose\n");
	printf("\td: be even more verbose (this includes 'v')\n");
	printf("\tp: print distribution - ");
        printf("use with 'c' for debugging purposes\n");
        printf("\n");

        exit(1);
}
