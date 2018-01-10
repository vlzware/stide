/**
 * Stide - hiding algorithm
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "stide.h"
#include "helpers.h"

void set_bit(uint8_t rgb, uint32_t pixel, uint32_t bit_pos,
	     uint8_t **payload, struct image *img);
void print_channel(uint8_t rgb);
void make_fat_pixels(uint8_t rgb, uint32_t pixel, struct image *img);

/**
 * Embed the text bits in the image
 */
int hide(uint32_t *bits_text, uint8_t **payload, struct image *img)
{
	if (param.debug)
		printf("\nHiding route (pixel/channel):  \n");

	/* create the pixel-index array and shuffle it */
	uint32_t *shuffle_arr = NULL;
	shuffle_arr = shuffle(img->pixels);
	if (shuffle_arr == NULL) {
		printf("Memory error!\n");
		return 1;
	}

	uint8_t rgb;		/* color channel */
	uint32_t pixel;		/* pixel number */
	uint32_t bit_pos;	/* position in payload */
	for (bit_pos = 0; bit_pos < (*bits_text); bit_pos++) {
		if (param.debug)
			printf("%i", shuffle_arr[bit_pos]);

		pixel = (img->bpp == 3)
		    ? (shuffle_arr[bit_pos] * 3)
		    : (shuffle_arr[bit_pos] * 4);

		/* calculate channel */
		rgb = rand_at_most(2);

		if (param.print_dist) {
			/* make big fat pixels to show distribution */
			make_fat_pixels(rgb, pixel, img);
		} else {
			/* embed */
			set_bit(rgb, pixel, bit_pos, payload, img);
		}
		if (param.debug)
			print_channel(rgb);
	}

	if (param.debug)
		printf("\n");

	SFREE(shuffle_arr);
	return 0;
}

/**
 * stide's actual hiding algorithm
 */
void set_bit(uint8_t rgb, uint32_t pixel, uint32_t bit_pos,
	     uint8_t **payload, struct image *img)
{
	/* position in the rgb array */
	uint32_t pos = pixel + rgb;

	/* mask */
	uint8_t mask = img->rgb[pos] & ~1;

	/* bit to hide xored with a random number */
	uint8_t bit_xor = (*payload)[bit_pos] ^ (1 & random());

	/* build */
	img->rgb[pos] = mask | bit_xor;
}

/**
 * Print the current rgb channel
 */
void print_channel(uint8_t rgb)
{
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

/**
 * Make bit fat pixels to show the distribution in a visible way
 */
void make_fat_pixels(uint8_t rgb, uint32_t pixel, struct image *img)
{
	img->rgb[pixel] = 0;
	img->rgb[pixel + 1] = 0;
	img->rgb[pixel + 2] = 0;

	img->rgb[pixel + rgb] = 255;

	/* deal with alpha channel if any */
	if (img->bpp == 4)
		img->rgb[pixel + 3] = 255;
}
