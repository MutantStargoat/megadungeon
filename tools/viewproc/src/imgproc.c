#include <stdio.h>
#include <stdlib.h>
#include <imago2.h>
#include "imgproc.h"

void img_halfsize24(struct img_pixmap *img)
{
	int i, j, r, g, b;
	unsigned char *src, *dest;
	register int pitch = img->width * 3;

	src = img->pixels;
	dest = img->pixels;

	img->width >>= 1;
	img->height >>= 1;

	for(i=0; i<img->height; i++) {
		for(j=0; j<img->width; j++) {
			r = src[0] + src[3] + src[pitch] + src[pitch + 3];
			src++;
			g = src[0] + src[3] + src[pitch] + src[pitch + 3];
			src++;
			b = src[0] + src[3] + src[pitch] + src[pitch + 3];
			src++;
			dest[0] = r >> 2;
			dest[1] = g >> 2;
			dest[2] = b >> 2;
			dest += 3;
			src += 3;
		}
		src += pitch;
	}

	if((dest = realloc(img->pixels, img->width * img->height * img->pixelsz))) {
		img->pixels = dest;
	}
}

