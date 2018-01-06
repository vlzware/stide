/**
 * Stide - helper functions
 */

#include <stdint.h>
#include <sqlite3.h>

/**
 * djb2 hash algorithm by dan bernstein:
 * http://www.cse.yorku.ca/~oz/hash.html
 */
uint32_t hash(const char *str);

/**
 * Return random number in the closed interval [0, max]
 * http://stackoverflow.com/a/6852396/6049386
 */
long rand_at_most(long max);

/**
 * Fisher–Yates shuffle
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
 */
uint32_t *shuffle(uint32_t size);

/**
* Convert all characters from str to lower case
*/
void str_tolower(char *str);

/**
* Return the number of substrings in str
*/
int count_substr(char *str);

/**
 * Tokenize a string removing all punctuation,
 * convert all to lowercase
 * Return number of tokens
 * Assing the pointer array to the substrings
 */
int tokenize(char *str, char ***arr);

/**
 * Build a sql query
 */
 void build_sql_cmd(char *cmd, const char *select, const char *from,
 		    const char *where, char *query, const int maxlen);

/**
* build_sql_cmd caller
*/
int sql_id_or_word(char *cmd, char *id, char *word, int maxquery);

/**
* Read sql output. Return 0 on success, 1 - otherwise.
* Copy the first found element in res.
 */
int read_sql(sqlite3_stmt *stmt, char **res);

/**
* Get id or word from the database given a word or id as parameter.
* Returns 0 if succesfull, otherwise:
*   1 - no id found; 2 - problem with the table; 3 - missing parameter.
* Points *res to the first found element.
*/
uint8_t sql_get(char *id, char *word, char **res);

/**
 * Print num in binary
 */
void printB(uint8_t num);

/**
 * Return the file extension
 * https://stackoverflow.com/a/5309508/6049386
 */
const char *get_filename_ext(const char *filename);

/**
 * Check if the extension is png otherwise defaults to bmp.
 */
char parse_ext(char *fname);

/**
 * Print info about the max allowed words/symbols.
 */
void print_max_allowed(void);
