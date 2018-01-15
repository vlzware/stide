/**
 * Stide - helper functions
 */

#include "helpers.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../lib/sqlite3/sqlite3.h"
#include "stide.h"

/* http://stackoverflow.com/a/3208376/6049386 */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

/**
 * djb2 hash algorithm by dan bernstein:
 * http://www.cse.yorku.ca/~oz/hash.html
 */
uint32_t hash(const char *str)
{
	uint32_t hash = 5381;
	uint8_t c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return (hash);
}

/**
 * Return random number in the closed interval [0, max]
 * https://stackoverflow.com/a/6852396/6049386
 *
 * Assumes 0 <= max <= RAND_MAX
 */
long rand_at_most(long max)
{
	/* max <= RAND_MAX < ULONG_MAX, so this is okay. */
	unsigned long
	    num_bins = (unsigned long)max + 1,
	    num_rand = (unsigned long)RAND_MAX + 1,
	    bin_size = num_rand / num_bins,
	    defect = num_rand % num_bins;

	long x;
	do {
		x = random();
	}
	/* This is carefully written not to overflow */
	while (num_rand - defect <= (unsigned long)x);

	/* Truncated division is intentional */
	return x / bin_size;
}

/**
 * Fisher–Yates shuffle
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
 */
uint32_t *shuffle(uint32_t size)
{
	uint32_t *res;
	res = (uint32_t *) malloc(sizeof(uint32_t) * size);
	if (res == NULL) {
		printf("(!) Memory error in 'shuffle'!\n");
		exit(1);
	}

	for (uint32_t i = 0; i < size; i++) {
		res[i] = i;
	}
	uint32_t n, tmp;
	for (uint32_t i = size - 1; i > 0; i--) {
		n = rand_at_most(i);
		tmp = res[n];
		res[n] = res[i];
		res[i] = tmp;
	}

	/* do not forget to use free in the caller */
	return res;
}

/**
 * Convert all characters from str to lower case
 */
void str_tolower(char *str)
{
	while (*str) {
		*str = tolower(*str);
		str++;
	}
}

/**
 * Return the number of substrings in str
 */
int count_substr(char *str)
{
	int count = 0;
	while (*str) {
		if (isalnum(*str)) {
			count++;
			do {
				str++;
			} while (isalnum(*str));
			if (*str)
				str++;
		} else
			str++;
	}

	return count;
}

/**
 * Tokenize a string removing all punctuation; converts all to lowercase.
 * Returns number of tokens. Assings the pointer array to the substrings.
 */
int tokenize(char *str, char ***arr)
{
	/* make all lower case */
	str_tolower(str);

	/* calculate how much substrings */
	uint8_t count = count_substr(str);

	/* allocate 'count' pointers */
	*arr = (char **) malloc(sizeof(char *) * (count));
	if (arr == NULL) {
		printf("(!) Memory error in 'tokenize'!\n");
		exit(1);
	}

	/* assign the new pointers to the proper substrings */
	int c = 0;
	while (*str) {
		/* alphanumeric found */
		if (isalnum(*str)) {
			/* pointer to the start of the substring */
			(*arr)[c] = str;
			c++;

			/* read until the end of the substring */
			do {
				str++;
			} while (isalnum(*str));

		} else {
			/* terminate and advance */
			*str = '\0';
			do {
				str++;
			} while (*str != '\0' && isalnum(*str) == 0);
		}
	}

	return count;
}

/**
 * Build a sql query
 */
void build_sql_cmd(char *cmd, const char *select, const char *from,
		    const char *where, char *query, const int maxlen)
{
	snprintf(cmd, maxlen, "SELECT %s FROM %s WHERE %s='%s';",
		  select, from, where, query);
}

/**
 * build_sql_cmd caller
 */
int sql_id_or_word(char *cmd, char *id, char *word, int maxquery)
{
	if (word != NULL)
		build_sql_cmd(cmd, "id", "words", "word", word, maxquery);
	else if (id != NULL)
		build_sql_cmd(cmd, "word", "words", "id", id, maxquery);
	else
		return 1;
	return 0;
}

/**
 * Read sql output. Return 0 on success, 1 - otherwise.
 * Copy the first found element in res.
 */
int read_sql(sqlite3_stmt *stmt, char **res)
{
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		/* allocate memory and copy the first found element in *res */
		size_t tmps = strlen((char *)sqlite3_column_text(stmt, 0));
		*res = (char *)malloc(sizeof(char) * (tmps + 1));
		if (res == NULL) {
			printf("(!) Memory error in 'read_sql'!\n");
			exit(1);
		}
		strcpy(*res, (char *)sqlite3_column_text(stmt, 0));
		return 0;
	}
	*res = NULL;
	return 1;
}

/**
 * Get id or word from the database given a word or id as parameter.
 *   Returns 0 if succesfull, otherwise:
 *   1 - no id found; 2 - problem with the table; 3 - missing parameter.
 * Points *res to the first found element.
 */
uint8_t sql_get(char *id, char *word, char **res)
{
	sqlite3 *db;
	uint8_t rc;
	sqlite3_stmt *stmt;
	const int maxquery = 100;
	char sqlcmd[maxquery];

	/* open the database */
	rc = sqlite3_open(param.stidedb, &db);
	if (rc) {
		printf("(!) Can't open database: %s\n", sqlite3_errmsg(db));
		return 2;
	}

	/* build a SQL statement in sqlcmd according to the arguments */
	if (sql_id_or_word(sqlcmd, id, word, maxquery))
		/* wrong parameters */
		return 3;

	/* execute SQL statement */
	rc = sqlite3_prepare_v2(db, sqlcmd, 1000, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("(!) SQL error: \"%s\"\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 2;
	}

	int err = 0;
	/* read the sql output */
	err = read_sql(stmt, res);

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return err;
}

/**
 * Print num in binary.
 */
void printB(uint8_t num)
{
	printf(BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(num));
}

/**
 * Return the file extension
 * https://stackoverflow.com/a/5309508/6049386
 */
const char *get_filename_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

/**
 * Check if the extension is png otherwise defaults to bmp.
 */
char parse_ext(char *fname)
{
	return (strcmp("png",get_filename_ext(fname)) == 0)
		? extPNG : extBMP;
}

/**
 * Print info about the max allowed words/symbols.
 */
void print_max_allowed(void)
{
	printf("-----\n");
	printf("Max supported bits count is %i bits.\n",
		MAX_TOKENS_BITS);
	printf("This is about %i words in strict mode,\n",
		(MAX_TOKENS_BITS - WLEN_STRICT)/WLEN_STRICT);
	printf("or about %i characters in loose mode.\n",
		(MAX_TOKENS_BITS - WLEN_LOOSE)/WLEN_LOOSE);
	printf("-----\n");
}
