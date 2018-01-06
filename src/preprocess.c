/**
 * Stide - preprocessing
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stide.h"
#include "helpers.h"

void embed_word(int *bit_pos, uint16_t id, int bits,
		     uint8_t **payload);
int embed_all_strict(uint8_t count_tokens, char **text_tokens,
		     uint8_t **payload);
int check_text_length(uint32_t bits, uint32_t pixels);
void err_no_db(void);
void err_no_id(void);

/**
 * prep_strict: embed the payload using only words from the dictionary
 */
int prep_strict(uint32_t *bits_text, uint8_t **payload, uint32_t pixels)
{
	/* tokenize the text to hide */
	char **text_tokens = NULL;
	int count_tokens = 0;
	count_tokens = tokenize(param.msg, &text_tokens);

	if (param.verbose)
		printf("Tokenizing input: found %d tokens.\n\n", count_tokens);

	/* 14 bits for every word from the dictionary plus one for EOF */
	*bits_text = (count_tokens + 1) * WLEN_STRICT;

	int res;

	/* check length */
	if ((res = check_text_length(*bits_text, pixels)) != 0) {
		SFREE(text_tokens);
		return res;
	}

	*payload = (uint8_t *) malloc(sizeof(uint8_t) * (*bits_text));
	if (*payload == NULL) {
		printf("(!) Memory error in prep_strict!\n");
		SFREE(text_tokens);
		return 1;
	}

	if ((res = embed_all_strict(count_tokens,
		    text_tokens, payload)) != 0) {
		printf("(!) Error in embed_all_strict!\n");
		return res;
	}

	SFREE(text_tokens);

	return 0;
}

/**
 * Embed the payload one char at a time
 */
int prep_loose(uint32_t *bits_text, uint8_t **payload, uint32_t pixels)
{
	/* size of msg + 1 for EOF */
	*bits_text = (param.msg_len + 1) * WLEN_LOOSE;

	/* check length */
	if (check_text_length(*bits_text, pixels))
		return 1;

	*payload = (uint8_t *) malloc(*bits_text);
	if (*payload == NULL) {
		printf("(!) Memory error in prep_loose!\n");
		return 1;
	}

	int bit_pos = 0;
	int i;
	for (i = 0; i < param.msg_len; i++)
		embed_word(&bit_pos, param.msg[i], WLEN_LOOSE, payload);

	embed_word(&bit_pos, EOF_LOOSE, WLEN_LOOSE, payload);

	if (param.debug)
		printf("%i from %i bits processed.\n", bit_pos, *bits_text);

	return 0;
}

/**
 * Embed the payload in the tokens using 'strict' mode
 */
int embed_all_strict(uint8_t count_tokens, char **text_tokens,
		     uint8_t **payload)
{
	/* temporary variables and position watcher */
	char *sql_res = NULL;
	int bit_pos = 0;

	/* embed the id's from the words */
	int i, res;
	for (i = 0; i < count_tokens; i++) {
		if (param.verbose)
			printf("token #%d: %s\n", i, text_tokens[i]);

		res = sql_get(NULL, text_tokens[i], &sql_res);
		switch (res) {
		case 1:
			err_no_id();
			return 1;
		case 2:
			err_no_db();
			return 1;
		case 3:
			printf("(!) Error in sql_get!\n");
			return 1;
		}
		if (param.verbose)
			printf("id found in dictionary: %s\n\n", sql_res);

		uint16_t id = (uint16_t) atoi(sql_res);
		embed_word(&bit_pos, id, WLEN_STRICT, payload);

		SFREE(sql_res);
	}

	embed_word(&bit_pos, EOF_STRICT, WLEN_STRICT, payload);

	return 0;
}

/**
 * Embed a word/token
 */
void embed_word(int *bit_pos, uint16_t word, int wlen,
		uint8_t **payload)
{
	int i = wlen - 1;
	for (; i >= 0; i--) {
		(*payload)[*bit_pos] = 1 & (word >> i);
		if (param.debug)
			printB((*payload)[*bit_pos]);
		(*bit_pos)++;
	}
	if (param.debug)
		printf("\n");
}

/**
 * Check the length of the message.
 * Return 0 if OK, 1 - otherwise.
 */
int check_text_length(uint32_t bits, uint32_t pixels)
{
	if (param.debug)
		printf("\nbits_text: %i\n", bits);

	/* less than max supported */
	if (bits > MAX_TOKENS_BITS) {
		printf("(!) The input is too long: %i bits incl EOF!\n", bits);
		print_max_allowed();
		return 1;
	}

	/* text + EOF too long? */
	if (bits > pixels) {
		printf("(!) The input is too long for the given image!\n");
		return 1;
	}

	return 0;
}

void err_no_db(void)
{
	printf("(!) Can't find/open the database %s!\n", param.stidedb);
	printf("(!) Provide stide.db or a path to it ('-f' switch)\n\n");
}

void err_no_id(void)
{
	printf("(!) Token not in the dictionary!\n");
	printf("(!) Try another word or use without strict mode.\n\n");
}
