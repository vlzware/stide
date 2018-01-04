#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stide.h"
#include "helpers.h"

int embed_word(uint8_t **text_in_lsb, uint32_t *bit_pos,
		uint16_t id, int bits);

/**
 * prep_strict: embed the payload using only words from the dictionary
 */
int prep_strict(uint32_t *bits_text, uint8_t **text_in_lsb, uint32_t pixels)
{
	/* tokenize the text to hide */
	char **text_to_hide = NULL;
	uint8_t count_tokens = 0;
	count_tokens = tokenize(param.payload, &text_to_hide);

	if (param.verbose) {
		printf("Tokenizing input: ");
		printf("found %d tokens.\n", count_tokens);
	}

	/* 14 bits for every word from the dictionary plus one for EOF */
	*bits_text = (count_tokens + 1) * WLEN_STRICT;
	if (param.verbose)
		printf("bits_text: %i\n", *bits_text);

	/* text + EOF too long? */
	if (*bits_text > pixels) {
		printf("%s: The input is too long for the given image!\n",
			prog);
		free(text_to_hide);
		return 3;
	}

	/*
	 * create array with (count + eof)  * 14 * 1111111x chars
	 * where x is the next bit from the text to hide
	 */

	*text_in_lsb = (uint8_t *) malloc(sizeof(uint8_t) * (*bits_text));
	if (*text_in_lsb == NULL) {
		printf("%s: Memory error\n", prog);
		free(text_to_hide);
		return 4;
	}

	/* temporary variables and position watcher */
	char *res = NULL;
	uint32_t bit_pos = 0;

	/* embed the id's from the words */
	int i;
	for (i = 0; i < count_tokens; i++) {
		if (param.verbose)
			printf("token #%d: %s\n", i, text_to_hide[i]);

		sql_get(NULL, text_to_hide[i], &res);
		if (res == NULL) {
			printf("Token not in the dictionary!\n");
			printf("Try another word or use without ");
			printf("strict mode: (-s).\n");
			free(text_to_hide);
			return 5;
		}
		if (param.verbose)
			printf("id found in dictionary: %s\n", res);

		uint16_t id = (uint16_t) atoi(res);

		embed_word(text_in_lsb, &bit_pos, id, WLEN_STRICT);

		if (res != NULL)
			free(res);
	}

	/* add EOF_HIPS */
	embed_word(text_in_lsb, &bit_pos, EOF_STRICT, WLEN_STRICT);

	free(text_to_hide);

	return 0;
}

/**
 * prep_loose: embed the payload allowing arbitrary characters
 */
int prep_loose(uint32_t *bits_text, uint8_t **text_in_lsb, uint32_t pixels)
{

	return 0;
}

int embed_word(uint8_t **text_in_lsb, uint32_t *bit_pos,
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

		return 0;
}
