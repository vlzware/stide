/**
 * Stide - image processing functions
 */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stide.h"

int img_load(struct image *img)
{
	img->rgb = stbi_load(param.image_in, &(img->width),
			     &(img->height), &(img->bpp), 0);

	return 0;
}

int img_unload(struct image *img)
{
	stbi_image_free(img->rgb);

	return 0;
}

int img_save(struct image *img)
{
	if (param.out_ext == extPNG) {
		if (!stbi_write_png(param.image_out, img->width, img->height,
				    img->bpp, img->rgb,
				    img->width * img->bpp)) {
			printf("Can't write %s\n", param.image_out);
			return 1;
		}
	} else {
		/* defaults to bmp */
		if (!stbi_write_bmp(param.image_out, img->width, img->height,
				    img->bpp, img->rgb)) {
			printf("Can't write %s\n", param.image_out);
			return 1;
		}
	}

	return 0;
}
