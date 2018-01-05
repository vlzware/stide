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
struct parameter param;			/* parameter structure */
const char *prog = "stide";		/* program name */

void get_switches(int *set_cust_db, int *argc, char **argv[]);
void get_params(int set_cust_db, int argc, char *argv[]);
void print_params(void);
void print_usage(void);

/**
 * stide: embed/extract hidden text in a image
 *
 * Usage:
 *  stide -c[spvdf] [path/to/stide.db] password text img-in [img-out[.png]]
 * Or   :
 *  stide -e[svdf] [path/to/stide.db] password img-in
 *
 * parameters:
 *  c: 'create' - embed text in image;
 *  e: 'extract' - extract text from image;
 *  s: 'strict' mode - do not work with the dictionary only - embed
 *      arbitrary words/characters;
 *  p: print distribution - output the image with fat visible changes;
 *  v: be verbose;
 *  d: be even more verbose;
 *  f: path to stide.db (defaults to current directory).
 */
int main(int argc, char *argv[])
{
	/* image structure */
	struct image img;

	/* defaults */
	param.stidedb = NULL;
	param.pass = NULL;
	param.payload = NULL;
	param.image_in = NULL;
	param.image_out = NULL;
	param.strict = 1;
	param.verbose = 0;
	param.debug = 0;
	param.print_dist = 0;
	param.mode = UNSET;

	int set_cust_db = 0;

	/* get, parse and set parameters */
	get_switches(&set_cust_db, &argc, &argv);
	get_params(set_cust_db, argc, argv);
	if (param.debug)
		print_params();

	/*
	 * seed using hash from the password
	 */
	uint32_t pass_hash = hash(param.pass);
	srand(pass_hash);
	if (param.verbose)
		printf("Hashed pass: %u\n\n", pass_hash);

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
	if (param.verbose)
		printf("Input image loaded\n\n");
	printf("Processing...\n\n");

	/**
	 * create or extract
	 */
	int res;
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
	printf("\nDone!\n");
	return 0;
}

/**
 * Parse the switches from the command line
 */
void get_switches(int *set_cust_db, int *argc, char **argv[])
{
	int c;
	while (--(*argc) > 0 && (*++(*argv))[0] == '-')
		while ((c = *++(*argv)[0]))
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
			case 'f':
				*set_cust_db = 1;
				break;
			default:
				printf("%s: illegal option %c\n", prog, c);
				param.mode = UNSET;
				break;
			}
}

/**
 * Parse the command line parameters
 */
void get_params(int set_cust_db, int argc, char *argv[])
{
	char def_out[] = "out.png";
	char def_db[] = "stide.db";

	if ((param.strict == 0) && (set_cust_db == 1)) {
		printf("Strict mode is disabled:");
		printf(" ignoring the dictionary %s\n", argv[0]);
	}
	int error = 0;
	int i = 0;
	if (param.mode == EXTRACT) {
		if ((set_cust_db == 1) && (argc == 3)) {
			param.stidedb = argv[0];
			i = 1;
		} else if ((set_cust_db == 0) && (argc == 2)) {
			param.stidedb = def_db;
		} else {
			error = 1;
		}
		if (error == 0)
			param.image_in = argv[1 + i];
	} else if (param.mode == CREATE) {
		if ((set_cust_db == 1)
				&& ((argc == 4) || (argc == 5))) {
			param.stidedb = argv[0];
			i = 1;
			if (argc == 4) {
				param.image_out = def_out;
				param.out_ext = extPNG;
			} else {
				param.image_out = argv[4];
				param.out_ext = parse_ext(param.image_out);
			}
		} else if ((set_cust_db == 0)
				&& ((argc == 3) || (argc == 4))) {
			param.stidedb = def_db;
		} else {
			error = 1;
		}
		if (error == 0) {
			param.payload = argv[1 + i];
			param.image_in = argv[2 + i];
		}
	} else {
		error = 1;
	}
	if (error) {
		print_usage();
	} else {
		param.pass = argv[0 + i];
	}
}

void print_params(void)
{
	printf("%s parameters:\n", prog);
	const char *modes[] = {"UNSET", "CREATE", "EXTRACT"};
	printf("\tmode     : %s\n", modes[param.mode]);
	printf("\tpassword : %s\n", param.pass);
	printf("\ttext     : %s\n", param.payload);
	printf("\tdict     : %s\n", param.stidedb);
	printf("\timg_in   : %s\n", param.image_in);
	printf("\timg_out  : %s\n", param.image_out);
	printf("\tstrict   : %i\n", param.strict);
	printf("\tverbose  : %i\n", param.verbose);
	printf("\tdebug    : %i\n", param.debug);
	printf("\tprint_dis: %i\n", param.print_dist);
}

/*
 * print the help and die
 */
void print_usage(void)
{
	printf("\nUsage:\n");
	printf("%s -c[spvdf] [path/to/stide.db] password text ", prog);
	printf("image-in [image-out[.png]]\n");
	printf("\tc: create image embedding the hidden text\n");
	printf("-OR-\n");
	printf("%s -e[svdf] [path/to/stide.db] password image-in\n", prog);
	printf("\te: extract hidden text from an image\n");
	printf("\nOptional parameters:\n");
	printf("\ts: enable strict mode\n");
	printf("\tv: be verbose\n");
	printf("\td: be even more verbose (this includes 'v')\n");
	printf("\tp: print distribution - ");
	printf("use with 'c' for debugging purposes\n");
	printf("\tf: path to stide.db (defaults to current directory)\n");
	printf("\n");

	exit(1);
}
