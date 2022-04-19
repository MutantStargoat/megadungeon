#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <glob.h>
#include <imago2.h>
#include "darray.h"
#include "imgproc.h"

struct view {
	int cx, cy;
	int dir;
	struct img_pixmap img;
	int xoffs, yoffs;
};

int proc_args(int argc, char **argv);

char *pathpat;
char *outdir;
int width, height;
int maxcolors = 16;
int tilesz = 8;

struct view *viewlist;

int main(int argc, char **argv)
{
	int i, xsz, ysz, num, asz;
	glob_t gbuf;
	char *basename;
	struct img_pixmap *img, atlas;
	struct view *view;
	unsigned char *dest;

	if(proc_args(argc, argv) == -1) {
		return 1;
	}

	gbuf.gl_offs = 0;
	if(glob(pathpat, GLOB_TILDE, 0, &gbuf) != 0) {
		fprintf(stderr, "failed to match input file pattern: %s\n", pathpat);
		return 1;
	}
	num = gbuf.gl_pathc;

	if(outdir) {
		struct stat st;
		if(stat(outdir, &st) == 0) {
			if((st.st_mode & S_IFMT) != S_IFDIR) {
				fprintf(stderr, "output path (%s) exists but is not a directory\n", outdir);
				return 1;
			}
		} else {
			if(mkdir(outdir, 0775) == -1) {
				fprintf(stderr, "failed to create output directory: %s: %s\n", outdir, strerror(errno));
				return 1;
			}
		}
	}

	viewlist = darr_alloc(0, sizeof *viewlist);

	for(i=0; i<num; i++) {
		if((basename = strrchr(gbuf.gl_pathv[i], '/'))) {
			basename++;
		} else {
			basename = gbuf.gl_pathv[i];
		}

		darr_push(viewlist, 0);
		view = viewlist + darr_size(viewlist) - 1;
		img = &view->img;

		img_init(img);
		if(img_load(img, gbuf.gl_pathv[i]) == -1) {
			fprintf(stderr, "failed to load image: %s\n", gbuf.gl_pathv[i]);
			return 1;
		}
		if(i && (img->width != xsz || img->height != ysz)) {
			fprintf(stderr, "%s size mismatch, expected %dx%d\n", gbuf.gl_pathv[i],
					xsz, ysz);
		}
		xsz = img->width;
		ysz = img->height;

		img_convert(img, IMG_FMT_RGB24);

		img_halfsize24(img);
		img_halfsize24(img);
	}
	globfree(&gbuf);

	xsz >>= 2;
	ysz >>= 2;

	awidth = sqrt(xsz > ysz ? xsz : ysz);
	aheight = (num * xsz * 2 - 1) / awidth;

	img_init(&atlas);
	img_set_pixels(&atlas, awidth, aheight, IMG_FMT_RGB24, 0);

	dest = atlas.pixels;
	view = viewlist;
	view->xoffs = view->yoffs = 0;
	for(i=0; i<num; i++) {
		memcpy(dest, view->img.pixels, xsz * ysz * 3);
		view++;
		dest += xsz * ysz * 3;
	}

	img_quantize(&atlas, 16, IMG_DITHER_FLOYD_STEINBERG);
	img_save(&atlas, "atlas.png");
}

static const char *usage_fmt =
	"Usage: %s [options] <files pattern>\n"
	"Options:\n"
	"  -t <n>: tile size\n"
	"  -c <n>: max number of colors\n"
	"  -h: print usage and exit\n";

int proc_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] != 0) {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}
			switch(argv[i][1]) {
			case 't':
				if(!argv[++i] || (tilesz = atoi(argv[i])) <= 0) {
					fprintf(stderr, "-t must be followed by a number (tile size)\n");
					return -1;
				}
				break;

			case 'c':
				if(!argv[++i] || (maxcolors = atoi(argv[i])) <= 0) {
					fprintf(stderr, "-c must be followed by a positive number (max colors)\n");
					return -1;
				}
				break;

			case 'd':
				if(!argv[++i]) {
					fprintf(stderr, "-d must be followed by the output directory\n");
					return -1;
				}
				outdir = argv[i];
				break;

			case 'h':
				printf(usage_fmt, argv[0]);
				exit(0);

			default:
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}
		} else {
			if(pathpat) {
				fprintf(stderr, "unexpected argument: %s\n", argv[i]);
				return -1;
			}
			pathpat = argv[i];
		}
	}

	if(!pathpat) {
		pathpat = "*.png";
	}
	return 0;
}
