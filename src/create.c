/**
 * Stide - create mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stide.h"

int prep_strict(uint32_t * b_text, uint8_t ** t_in_lsb, uint32_t pixels);
int prep_loose(uint32_t * b_text, uint8_t ** t_in_lsb, uint32_t pixels);
int hide(uint32_t * bits_text, uint8_t ** payload, struct image *img);
int img_save(struct image *img);

/**
 * create: embed and save a 'stided' image
 */
int create(struct image *img)
{
	int res;
	uint32_t bits_text;		/* text bits counter */
	uint8_t *payload = NULL;	/* payload holder */

	/* preprocess the payload */
	res = param.strict
	    ? prep_strict(&bits_text, &payload, img->pixels)
	    : prep_loose(&bits_text, &payload, img->pixels);

	if (res != 0) {
		img_unload(img);
		free(payload);
		exit(res);
	}

	/* process the loaded image in memory */
	if ((res = hide(&bits_text, &payload, img)) != 0) {
		free(payload);
		img_unload(img);
		exit(res);
	}

	free(payload);

	/* save the processed image to file */
	if ((res = img_save(img)) != 0) {
		printf("%s: Could not write %s!\n", prog, param.image_out);
		img_unload(img);
		return res;
	}

	return 0;
}
