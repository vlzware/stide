/**
 * Stide - header for the gnu random.c
 */

#define	ULONG_MAX  ((unsigned long)(~0L))     /* 0xFFFFFFFF for 32-bits */
#define	LONG_MAX   ((long)(ULONG_MAX >> 1))   /* 0x7FFFFFFF for 32-bits*/

long int random(void);
void srandom(unsigned int x);
