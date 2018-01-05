#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stide.h"
#include "helpers.h"

void embed_word(uint8_t ** text_in_lsb, uint32_t * bit_pos,
	       uint16_t id, int bits);
void err_no_db(void);
void err_no_id(void);

/**
 * prep_strict: embed the payload using only words from the dictionary
 */
int prep_strict(uint32_t * bits_text, uint8_t ** text_in_lsb, uint32_t pixels)
{
	/* tokenize the text to hide */
	char **text_to_hide = NULL;
	uint8_t count_tokens = 0;
	count_tokens = tokenize(param.payload, &text_to_hide);

	if (param.verbose)
		printf("Tokenizing input: found %d tokens.\n", count_tokens);

	/* 14 bits for every word from the dictionary plus one for EOF */
	*bits_text = (count_tokens + 1) * WLEN_STRICT;
	if (param.verbose)
		printf("bits_text: %i\n\n", *bits_text);

	/* text + EOF too long? */
	if (*bits_text > pixels) {
		printf("%s: The input is too long for the given image!\n",
		       prog);
		free(text_to_hide);
		return 1;
	}

	*text_in_lsb = (uint8_t *) malloc(sizeof(uint8_t) * (*bits_text));
	if (*text_in_lsb == NULL) {
		printf("%s: Memory error\n", prog);
		free(text_to_hide);
		return 1;
	}

	/* temporary variables and position watcher */
	char *sql_res = NULL;
	uint32_t bit_pos = 0;

	/* embed the id's from the words */
	int i, res;
	for (i = 0; i < count_tokens; i++) {
		if (param.verbose)
			printf("token #%d: %s\n", i, text_to_hide[i]);

		res = sql_get(NULL, text_to_hide[i], &sql_res);
		if (res == 1) {
			err_no_id();
			return 1;
		} else if (res == 2) {
			err_no_db();
			free(text_to_hide);
			return 1;
		}
		if (param.verbose)
			printf("id found in dictionary: %s\n\n", sql_res);

		uint16_t id = (uint16_t) atoi(sql_res);
		embed_word(text_in_lsb, &bit_pos, id, WLEN_STRICT);

		free(sql_res);
	}

	/* add EOF_HIPS */
	embed_word(text_in_lsb, &bit_pos, EOF_STRICT, WLEN_STRICT);

	free(text_to_hide);

	return 0;
}

/**
 * prep_loose: embed the payload allowing arbitrary characters
 */
int prep_loose(uint32_t * bits_text, uint8_t ** text_in_lsb, uint32_t pixels)
{
	/* TODO: WIP */

	return 0;
}

void embed_word(uint8_t ** text_in_lsb, uint32_t * bit_pos,
	       uint16_t word, int wlen)
{
	int i = wlen - 1;
	for (; i >= 0; i--) {
		(*text_in_lsb)[*bit_pos] = 1 & (word >> i);
		if (param.debug)
			printB((*text_in_lsb)[*bit_pos]);
		(*bit_pos)++;
	}
	if (param.debug)
		printf("\n");
}

void err_no_db(void)
{
	printf("Can't find/open the database %s!\n", param.stidedb);
	printf("Provide stide.db or a path to it ('-f' switch)\n\n");
}

void err_no_id(void)
{
	printf("Token not in the dictionary!\n");
	printf("Try another word or use without strict mode.\n\n");
}
