#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stide.h"

int prep_strict(uint32_t *b_text, uint8_t **t_in_lsb, uint32_t pixels);
int prep_loose(uint32_t *b_text, uint8_t **t_in_lsb, uint32_t pixels);
int hide(uint32_t *bits_text, uint8_t **text_in_lsb, struct image *img);
int img_save(struct image *img);

/**
 * create: embed and save a 'stided' image
 */
int create(struct image *img)
{
	int res;
	/*
	 * create: preprocess the payload
	 */
	uint32_t bits_text;			/* text bits counter */
	uint8_t *text_in_lsb = NULL;		/* payload holder */

	res = param.strict
		? prep_strict(&bits_text, &text_in_lsb, img->pixels)
		: prep_loose(&bits_text, &text_in_lsb, img->pixels);

	if (res != 0) {
                img_unload(img);
                free(text_in_lsb);
		exit(res);
	}

	/*
	 * create: process the loaded image in memory
	 */
        if ((res = hide(&bits_text, &text_in_lsb, img)) != 0) {
                free(text_in_lsb);
                img_unload(img);
                exit(res);
        }
        free(text_in_lsb);

	/*
	 * create: save the processed image to file
	 */
        if ((res = img_save(img)) != 0) {
                printf("%s: Could not write %s!\n", prog, param.image_out);
                img_unload(img);
                return res;
        }

	return 0;
}
