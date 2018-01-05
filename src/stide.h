/**
 * Stide - main definitions
 */

#include <stdint.h>

extern const char *prog;	/* name for stide_create */
extern struct parameter param;	/* cmd line parameters */

#define extBMP 1
#define extPNG 2

#define WLEN_STRICT 14		/* word length in strict mode */
#define WLEN_LOOSE 8		/* word length in loose mode (ASCII) */
#define EOF_STRICT 10005	/* EOF in strict mode */
#define EOF_LOOSE 0		/* EOF in loose mode */
#define MAX_TOKENS_BITS 1400	/* max length */

enum ops {
	UNSET,
	CREATE,
	EXTRACT
};

struct parameter {		/* parameter structure */
	char *stidedb;		/* path to stide.db */
	char *pass;		/* password */
	char *msg;		/* text to hide */
	char *image_in;		/* input image */
	char *image_out;	/* output image */
	int strict;		/* strict mode */
	int verbose;		/* be verbose */
	int debug;		/* print debugging messages */
	int print_dist;		/* show distribution */
	char out_ext;		/* file extension */
	enum ops mode;		/* operation mode */
};

struct image {			/* image structure */
	int width;
	int height;
	int bpp;
	int pixels;		/* pixel count */
	uint8_t *rgb;		/* the actual rgb data */
};

int img_load(struct image *img);
int img_unload(struct image *img);

int create(struct image *img);
int extract(struct image *img);
