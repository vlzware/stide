#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "helpers.h"
#include "stide.h"

int extract(struct image *img)
{
	/* create the pixel-index array and shuffle it */
	uint32_t* shuffle_arr = NULL;
	shuffle_arr = shuffle(img->pixels);
	if (shuffle_arr == NULL)
		return 7;

	printf("\nHidden message:\n");

	/* temporary variables and buffers */
        uint8_t text_in_lsb[14];
        uint8_t bit_pos = 0;
        int all_read = 0;
        char str[6];
        char *res = NULL;
        uint16_t id = 0;
	uint16_t end_pos = img->pixels > MAX_TOKENS_BITS
		? MAX_TOKENS_BITS
		: img->pixels;
	uint32_t pos_rgb;
	uint8_t rgb;

	uint16_t i;
	for (i = 0; (i < end_pos) && (!all_read); i++) {
		pos_rgb = (img->bpp == 3)
			? (shuffle_arr[i]*3)
			: (shuffle_arr[i]*4);

		/* calculate channel */
		rgb = rand_at_most(2);

		/* extract and xor with the 'random' bit */
		text_in_lsb[bit_pos] =
			img->rgb[pos_rgb + rgb] ^ (1 & random());

		/* advance in the text */
		bit_pos++;

		/* whole word read */
		if (bit_pos == WLEN_STRICT) {
			bit_pos = 0;
			id = 0;

			/* build the int from the last bits */
			int8_t i = WLEN_STRICT - 1;
			for (; i >= 0; i--) {
				id ^= (-(text_in_lsb[WLEN_STRICT - 1 - i] & 1) ^ id) & (1 << i);
			}
			if (id == EOF_STRICT) {
				all_read = 1;
			} else {
				/* print current word */
				sprintf(str, "%i", id);
				sql_get(str, NULL, &res);
				if (!res) {
					printf("\n---ID not in the dictionary! Maybe wrong image/password?---\n");
				}
				else
					printf("%s ",res);
			}
		}

	}

    	/* housekeeping */
    	free(res);
    	free(shuffle_arr);


	return 0;
}
