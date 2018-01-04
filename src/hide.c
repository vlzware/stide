#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "stide.h"
#include "helpers.h"


int hide(uint32_t *bits_text, uint8_t **text_in_lsb, struct image *img)
{
	if (param.debug)
		printf("Hiding route (pixel/channel):  \n");

	/* create the pixel-index array and shuffle it */
	uint32_t* shuffle_arr = NULL;
	shuffle_arr = shuffle(img->pixels);
	if (shuffle_arr == NULL)
		return 6;

	uint8_t rgb;
	uint32_t pos_rgb;			/* position in img->rgb */
	uint32_t bit_pos;			/* position in payload */
	for (bit_pos = 0; bit_pos < (*bits_text); bit_pos++) {
		if (param.verbose)
			printf("%i", shuffle_arr[bit_pos]);

		pos_rgb = (img->bpp == 3)
			? (shuffle_arr[bit_pos] * 3)
			: (shuffle_arr[bit_pos] * 4);

		/* calculate channel */
		rgb = rand_at_most(2);

		if (param.print_dist) {
			/* make big fat pixels to show distribution */
			img->rgb[pos_rgb + (rgb + 1)%3] = 0;
			img->rgb[pos_rgb + (rgb + 2)%3] = 0;
			img->rgb[pos_rgb + 3] = 255;
		} else {
			/* embed and xor with random number */
			img->rgb[pos_rgb + rgb] = (
				(img->rgb[pos_rgb + rgb] & ~1)
				| ((*text_in_lsb)[bit_pos] ^ (1 & random()))
			);
		}
		if (param.debug)
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

	free(shuffle_arr);
	return 0;
}
