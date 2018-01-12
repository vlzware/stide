/**
 * Stide - image processing functions
 */

#include <stdio.h>
#include <png.h>

/* change this to 0 to use stb for image writing */
#define USELIBPNG 1

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stide.h"

int _img_load_stb(struct image *img);
// int _img_load_libpng(struct image *img);
int _img_unload_stb(struct image *img);
int _img_unload_libpng(struct image *img);
int _img_save_stb(struct image *img);
int _img_save_libpng(struct image *img);

int img_load(struct image *img)
{
	return _img_load_stb(img);
}

int img_unload(struct image *img)
{
	free(img->rgb);
	return 0;
	// return _img_unload_libpng(img);
}

int img_save(struct image *img)
{
	if (param.out_ext == extPNG) {
		return USELIBPNG
			? _img_save_libpng(img)
			: _img_save_stb(img);
	} else {
		/* defaults to bmp write with stb */
		if (!stbi_write_bmp(param.image_out,
				    img->width, img->height,
				    img->bpp, img->rgb)) {
			printf("\n(!) Could not write %s\n", param.image_out);
			return 1;
		}
		return 0;
	}
	return 1;
}

/* stb load */
int _img_load_stb(struct image *img)
{
	img->rgb = stbi_load(param.image_in,
			     &(img->width), &(img->height), &(img->bpp), 0);

	return 0;
}

/* stb unload */
int _img_unload_stb(struct image *img)
{
	stbi_image_free(img->rgb);

	return 0;
}

/* stb save */
int _img_save_stb(struct image *img)
{
	/* Note: this expects a png */
	if (!stbi_write_png(param.image_out,
			    img->width, img->height,
			    img->bpp, img->rgb,
			    img->width * img->bpp)) {
		printf("\n(!) Could not write %s\n", param.image_out);
		return 1;
	}

	return 0;
}

/*
 * Write an image using libpng.
 * Mostly based on:
 * http://www.labbookpages.co.uk/software/imgProc/libPNG.html
 */
int _img_save_libpng(struct image *img)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;

	/* Open file for writing (binary mode) */
	fp = fopen(param.image_out, "wb");
	if (fp == NULL) {
		printf("(!) Could not open %s for writing\n", param.image_out);
		code = 1;
		goto finalise;
	}

	/* Initialize write structure */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
					  NULL, NULL, NULL);
	if (png_ptr == NULL) {
		printf("(!) Could not allocate write struct.\n");
		code = 1;
		goto finalise;
	}

	/* Initialize info structure */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		printf("(!) Could not allocate info struct.\n");
		code = 1;
		goto finalise;
	}

	/* Setup Exception handling */
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("(!) Error during png creation.\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	/* Write header (8 bit colour depth) */
	png_set_IHDR(png_ptr, info_ptr, img->width, img->height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	/* Write image data */
	row = (png_bytep) malloc(img->bpp * img->width * sizeof(uint8_t));
	if (row == NULL) {
		printf("(!) Memory error in _img_save_libpng\n");
		code = 1;
		goto finalise;
	}
	int x, y, bit_pos;
	bit_pos = 0;
	for (y = 0; y < img->height; y++) {
		for (x = 0; x < (img->width * img->bpp); x++) {
			row[x] = img->rgb[bit_pos];
			bit_pos++;
		}
		png_write_row(png_ptr, row);
	}
	SFREE(row);

	/* End write */
	png_write_end(png_ptr, NULL);

	finalise:
	if (fp != NULL)
		fclose(fp);
	if (info_ptr != NULL) {
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_info_struct(png_ptr, &info_ptr);
	}
	if (png_ptr != NULL)
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	return code;
}
