/**
 * Helper functions for the H.I.P.S. steganography tool
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <sqlite3.h>
#include <string.h>

#define DATABASE "hips.db"

// http://stackoverflow.com/a/3208376/6049386
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
 * Returns random number in the closed interval [0, max]
 * https://stackoverflow.com/a/6852396/6049386
 */
// Assumes 0 <= max <= RAND_MAX
// Returns in the closed interval [0, max]
long rand_at_most(long max) {
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = random();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

/**
 * Fisher–Yates shuffle
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
 */
uint32_t* shuffle(uint32_t size)
{
	uint32_t* res;
	res = (uint32_t*) malloc( sizeof(uint32_t) * size );
	
	for (uint32_t i = 0; i < size; i++)
	{
		res[i] = i;
	}
	uint32_t n,tmp;
	for (uint32_t i = size -1; i > 0; i--)
	{
		n = rand_at_most(i);
		tmp = res[n];
		res[n] = res[i];
		res[i] = tmp;
	}
	return res;
	// do not forget to use free in the caller
}

/**
 * Tokenize a string removing all punctuation,
 * converts all to lowercase
 * Returns number of tokens
 * Assings the pointer array to the substrings
 */
uint8_t tokenize (char *str, char ***arr)
{
    uint8_t count = 0;
    char *p;
    
    for (int o=0; str[o]; o++)
		str[o] = tolower(str[o]);

	// calculate how much substrings
	p = str;
	while (*p != '\0')
	{
		if (isalnum(*p))
		{
			count++;
			do
			{
				p++;
			} while (isalnum(*p));
			if (*p != '\0')
				p++;
		} else
			p++;
	}

	// malloc 'count' pointers
    *arr = (char**) malloc(sizeof(char*) * (count));
    if (arr == NULL)
        exit(1);

	// assign the new pointers to the prpoper substrings
    int c = 0;
    while (*str != '\0')
    {
		// alphanumeric found
		if (isalnum(*str))
		{
			// pointer to the start of the substring
			(*arr)[c] = str;
			c++;
			
			// read until the end of the substring
			do
			{
				str++;
			} while (isalnum(*str));

		} else
		{
			// terminate and advance
			*str = '\0';
			do
			{
				str++;
			} while (*str != '\0' && isalnum(*str) == 0);
		}
    }

    return count;
}

/**
 * Gets id or word from the database
 * given word or id as parameter.
 * Returns 0 if succesfull, otherwise 1.
 * Points *res to the first found element.
 */
uint8_t sql_get (char *id, char *word, char **res)
{
   sqlite3 *db;
   uint8_t rc;
   sqlite3_stmt *stmt;
   char sqlquery[100];
   enum modes {GET_ID, GET_WORD};
   enum modes mode;

   if (id)
      mode = GET_WORD;
   else if (word)
      mode = GET_ID;
   else
      return 1;

   // open database
   rc = sqlite3_open(DATABASE, &db);
   if( rc ){
      printf("Can't open database: %s\n", sqlite3_errmsg(db));
      return 1;
   }

   // create SQL statement according to the arguments
   if (mode == GET_ID)
      snprintf(sqlquery, sizeof sqlquery, "%s%s%s", "SELECT id FROM words WHERE word='", word, "';");
   else
      snprintf(sqlquery, sizeof sqlquery, "%s%s%s", "SELECT word FROM words WHERE id='", id, "';");

   // execute SQL statement
   rc = sqlite3_prepare_v2(db, sqlquery, 1000, &stmt, 0);
   if( rc != SQLITE_OK ){
      printf("SQL error:\n%s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
   }
   
   // read the sql output
   if (sqlite3_step(stmt) == SQLITE_ROW)
   {
       // allocate memory and copy the first found element in *res
      *res = malloc(sizeof(char) * (strlen((char*)sqlite3_column_text(stmt, 0)) + 1));
      if (res == NULL)
      {
          printf("Memory error\n");
          return 1;
      }
      strcpy(*res, (char*)sqlite3_column_text(stmt, 0));
   } else
   {
      *res = NULL;
      sqlite3_finalize(stmt);
      sqlite3_close(db);
      return 1;
   }
   
   sqlite3_finalize(stmt);
   sqlite3_close(db);
   return 0;
}

/**
 * prints num in binary
 */
void printB(uint8_t num)
{
    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(num));
}

/**
 * returns the file extension
 * https://stackoverflow.com/a/5309508/6049386
 */
const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}
