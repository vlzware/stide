/**
 * Helper functions for Stide
 */

#include <stdint.h>

/**
 * djb2 hash algorithm by dan bernstein:
 * http://www.cse.yorku.ca/~oz/hash.html
 */
uint32_t hash(const char *str);

/**
 * Returns random number in the closed interval [0, max]
 * http://stackoverflow.com/a/6852396/6049386
 */
uint16_t rand_at_most(uint16_t max);

/**
 * Fisher–Yates shuffle
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
 */
uint32_t* shuffle(uint32_t size);

/**
 * Tokenize a string removing all punctuation,
 * converts all to lowercase
 * Returns number of tokens
 * Assings the pointer array to the substrings
 */
uint8_t tokenize (char *str, char ***arr);

/**
 * Gets id or word from the database
 * given word or id as parameter.
 * Returns 0 if succesfull, otherwise 1.
 * Points *res to the first found element.
 */
uint8_t sql_get (char *id, char *word, char **res);

/**
 * prints num in binary
 */
void printB(uint8_t num);

/**
 * returns the file extension
 * https://stackoverflow.com/a/5309508/6049386
 */
const char *get_filename_ext(const char *filename);
