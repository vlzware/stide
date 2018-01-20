/**
 * Stide - extract mode
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/random.h"
#include "helpers.h"
#include "stide.h"

uint16_t build_id(uint8_t *payload, int wlen);
void print_res(char *res, int *err_count);
void get_payload_bit(uint8_t *payload, uint8_t bit_pos, uint32_t pixel,
		     struct image *img);
int parse_word(uint8_t *payload, int wlen, int eof, int *eof_found,
		     int *err_count);
int is_unprintable(uint16_t c);

int extract(struct image *img)
{
	/* create the pixel-index array and shuffle it */
	uint32_t *shuffle_arr = NULL;
	shuffle_arr = shuffle(img->pixels);

	printf("Hidden message:\n\n\t");
	int err_count = 0;

	int wlen = param.strict ? WLEN_STRICT : WLEN_LOOSE;
	int eof = param.strict ? EOF_STRICT : EOF_LOOSE;
	int eof_found = 0;		/* eof */

	uint8_t payload[wlen];		/* payload buffer */
	uint8_t bit_pos = 0;		/* position in the payload */

	uint16_t end_pos = img->pixels > MAX_TOKENS_BITS
	    		   ? MAX_TOKENS_BITS
			   : img->pixels;
	uint32_t pixel;

	uint16_t i;
	for (i = 0; (i < end_pos) && (!eof_found); i++) {
		pixel = (img->bpp == 3)
		    ? (shuffle_arr[i] * 3)
		    : (shuffle_arr[i] * 4);

		get_payload_bit(payload, bit_pos, pixel, img);

		/* advance in the text */
		bit_pos++;

		/* whole word read */
		if (bit_pos == wlen) {
			bit_pos = 0;
			if (parse_word(payload, wlen, eof,
					&eof_found, &err_count))
				break;
		}
		if (err_count == MAXERROR) {
			printf("\n(!) Max count of errors reached - ");
			printf("maybe wrong password/image/mode ???");
			break;
		}
	}
	printf("\n\n");

	/* housekeeping */
	SFREE(shuffle_arr);

	return 0;
}

/**
 * Extract a bit of the payload
 */
void get_payload_bit(uint8_t *payload, uint8_t bit_pos, uint32_t pixel,
 		     struct image *img)
{
	uint8_t rgb;

	/* calculate channel */
	rgb = rand_at_most(2);

	/* extract the payload */
	payload[bit_pos] = img->rgb[pixel + rgb] ^ (1 & random());
}

/**
* Build id/symbol from the payload
*/
uint16_t build_id(uint8_t *payload, int wlen)
{
	uint16_t id = 0;
	int8_t i = wlen - 1;
	for (; i >= 0; i--) {
		id ^= (-(payload[wlen - 1 - i] & 1) ^ id) & (1 << i);
	}
	return id;
}

/**
 * Parse a word. Set eof_found to 1 if id == 'eof'.
 */
int parse_word(uint8_t *payload, int wlen, int eof, int *eof_found,
	      int *err_count)
{
	uint16_t id = 0;
	int res;

	/* build the id/symbol from the last bits */
	id = build_id(payload, wlen);

	if (id == eof) {
		*eof_found = 1;
		return 0;
	}

	/* print current word - strict mode */
	if (param.strict) {
		char str[INTPSTR + 1];
		char *sql_res = NULL;
		sprintf(str, "%i", id);			/* convert to string */
		res = sql_get(str, NULL, &sql_res);	/* search the db */
		if (res > 1)
			return res;
		print_res(sql_res, err_count);
		SFREE(sql_res);
		return 0;
	}

	/* print current word - loose mode */
	putchar(id);
	*err_count += is_unprintable(id);
	return 0;
}

/**
 * Print res or error message
 */
void print_res(char *res, int *err_count)
{
	if (!res) {
		printf("\n---ID not in the dictionary!---\n");
		(*err_count)++;
	} else {
		printf("%s ", res);
	}
}

/**
 * Check for allowed characters. Return 0 if printable, 1 - otherwise.
 */
int is_unprintable(uint16_t c)
{
	return (c != '\n' && c != '\t' && !isprint(c));
}
