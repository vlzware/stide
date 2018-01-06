/**
 * Stide - create mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stide.h"

int prep_strict(uint32_t * b_text, uint8_t ** t_in_lsb, uint32_t pixels);
int prep_loose(uint32_t * b_text, uint8_t ** t_in_lsb, uint32_t pixels);
int hide(uint32_t * bits_text, uint8_t ** payload, struct image *img);
int img_save(struct image *img);

void print_stats(uint32_t bits_text);
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
		SFREE(payload);
		return(res);
	}

	/* process the loaded image in memory */
	if ((res = hide(&bits_text, &payload, img)) != 0) {
		SFREE(payload);
		return(res);
	}

	SFREE(payload);

	/* save the processed image to file */
	if ((res = img_save(img)))
		return res;

	if (param.verbose)
		print_stats(bits_text);

	return 0;
}

/**
 * Print statistics about compression.
 */
void print_stats(uint32_t bits_text)
{
	int len = (param.msg_len + 1) * 8;
	printf("\n");
	printf("-- Got   %i bits.\n", len);
	printf("-- Wrote %i bits.\n", bits_text);
	printf("-- Compression ratio: %3.2f\n",
		((float) len/bits_text) - 1);
}
