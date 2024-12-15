/*
 * MandelSteg.c V1.0
 * (C) Copyright Henry Hastur 1994
 *
 * Hides arbitrary data in a mandelbrot GIF image.
 *
 * May or may not be covered by US ITAR encryption export regulations, if
 * in doubt, don't export it. It would be pretty stupid if it was, but
 * hey, governments have done some pretty stupid things before now....
 *
 * This program is copyright Henry Hastur 1994, but may be freely distributed,
 * modified, incorporated into other programs and used, as long as the
 * copyright stays attached and you obey any relevant import or export
 * restrictions on the code. No warranty is offered, and no responsibility
 * is taken for any damage use of this program may cause. In other words,
 * do what you want with it, but don't expect me to pay up if anything
 * unexpected goes wrong - you're using it at your own risk...
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef DOS
#include <fcntl.h>
#include <io.h>
#endif

#include "ext.h"

#define WRITE_PREFIX    0
#define WRITE_DATA      1
#define WRITE_SUFFIX    2

static  int32   w,h;
static  byte    r[256],g[256],b[256];
static  byte    *data = 0;

static  byte    encoding = FALSE;
static  byte    same_col = TRUE;
static  byte    counting = FALSE;
static  byte    full_palette = FALSE;
static  byte    random_pad = FALSE;
static  byte    data_bit = 0x80;
static  byte    write_state = WRITE_PREFIX;
static  long    byte_pad = 0;

void    usage()

{
	printf("Usage:\tmandelsteg [-sz w h] [-md x y w h] [-c] [-b n]\n");
	printf("\t\t[-ns] [-fp] [-bp n] [-r] [-e [file]]\n\n");
	printf(" Use:\n");
	printf("\t-sz to specify width and height in pixels\n");
	printf("\t-md to specify mandelbrot area : start (x,y) size (w,h)\n");
	printf("\t-c  to count available space in the image\n");
	printf("\t-b  to specify the bit number to use (default 7)\n");
	printf("\t-ns to specify no storing of data in consecutive identical pixels\n");
	printf("\t-fp for a full 256-colour palette rather than 128\n");
	printf("\t-bp to store the data n bytes into the available space\n");
	printf("\t-r to pad the data with random bytes\n");
	printf("\t-e  to encode data from stdin into the image file. If no file\n");
	printf("\t\tis specified, output goes to stdout.\n");

	exit (2);
}

#ifdef LOW_MEM
static  double  incx,incy,nx,ny,rx,ry,nx2,ny2;
static  double  dw,dh,dxs,dys;
static  int16   pix,opix,mpix,lpix;
static  int     inc = 0;
static  int     in_count = 0;
static  int     lim;
static  byte    last_missed = FALSE;
static  byte    eof_reached = FALSE;
static  long    steg_count = 0;

/* Calculate a pixel value */

int16   calc_pixel (off)

long    off;

{
	register        int     z;
	double  x,y;

	if (!(off % w))
		opix = lpix;

	rx = (dxs + dw * (double)(off % w));
	ry = (dys + dh * (double)(off / w));

	x = 0.0;
	y = 0.0;

	nx2 = ny2 = 0.0;
	for (z = 0 ; z <= lim; z++) {
		nx = nx2-ny2+rx;
		ny = x*y*2.0+ry;

		if ((nx2=nx*nx)+(ny2=ny*ny) > 4.0)
			break;

		x = nx;
		y = ny;
	}

	if (z > lim)
		pix = (int16) lim;
	else
		pix = (int16) z;

	mpix = pix & ~data_bit;

	if (same_col || (mpix != opix && off)) {
		if (last_missed) {
			last_missed = FALSE;
		}
		else {
		if (encoding) {
			if (!in_count) {
				in_count = 8;
				if (!byte_pad) {
					write_state = WRITE_DATA;
					inc = getchar();
				}

				/* We could not generate random bytes
				   when not needed, but a) one call
				   per 8 pixels is probably not
				   important given the amount of FP
				   math involved in generating those
				   pixels, and b) it may help to make
				   finding a pattern in any generated
				   data harder (though they'll have the
				   entire palette to analyse. */

				if (inc == EOF) {
					inc = random() & 0xFF;
					eof_reached = TRUE;
					write_state = WRITE_SUFFIX;
				}
				else if (byte_pad) {
					inc = random() & 0xFF;
					byte_pad--;
				}
			}

			in_count--;
			if (random_pad || write_state == WRITE_DATA) {
				pix = mpix;
				if ((inc & 0x80))
					pix = mpix | data_bit;
				inc <<= 1;
			}
		}
		steg_count++;
		}
	}
	else
		last_missed = TRUE;

	opix = mpix;

	if (!(off % w))
		lpix = mpix;

	return pix;
}
#endif

main(argc,argv)

int     argc;
char    *argv[];

{
	int     i,z;
	double  x,y;
#ifndef LOW_MEM
	byte    pix,opix,mpix;
	double  incx,incy,nx,ny,rx,ry,nx2,ny2;
	double  dw,dh,dxs,dys;
	int     inc = 0;
	int     in_count = 0;
	int     lim;
	byte    last_missed = FALSE;
	byte    eof_reached = FALSE;
	long    steg_count = 0;
#endif
	int     ix,iy;
	char    *output_name = NULL;
	int     c;
	byte    *d = NULL;

	i = 1;
	opix = 0;

	/* Default size */

	w = 640l;
	h = 480l;

	/* Default coordinates */

	dxs = -0.555;
	dys = -0.555;
	dw = 0.01;
	dh = 0.01;

	/* Check arguments */

	while (i < argc && argv[i]) {
		if (!strcmp(argv[i],"-h"))
			usage ();
		if (!strcmp(argv[i],"-e")) {
			encoding = TRUE;
			if ((i+1) == argc) {
				fprintf(stderr,"Outputting to stdout\n");
				output_name = NULL;
				i = argc;
			}
			else {
				i++;
				if (!strcmp(argv[i],"-")) {
					output_name = NULL;
				}
				else 
					output_name = argv[i];
			}
		}
		else if (!strcmp(argv[i],"-md")) {
			i++;
			if (i > (argc-4)) {
				fprintf(stderr,"use -md start-x start-y width height !\n");
				i = argc;
			}
			else {
				dxs = atof(argv[i]);
				dys = atof(argv[i+1]);
				dw = atof(argv[i+2]);
				dh = atof(argv[i+3]);

				i+= 3;
			}
		}
		else if (!strcmp(argv[i],"-sz")) {
			i++;
			if (i > (argc-2)) {
				fprintf(stderr,"use -sz width height !\n");
			}
			else {
				w = atol(argv[i]);
				h = atol(argv[i+1]);
				i++;
			}
		}
		else if (!strcmp(argv[i],"-ns")) {
			same_col = FALSE;
		}
		else if (!strcmp(argv[i],"-c")) {
			counting = TRUE;
		}
		else if (!strcmp(argv[i],"-fp")) {
			full_palette = TRUE;
		}
		else if (!strcmp(argv[i],"-b")) {
			if (i != (argc-1)) {
				data_bit = (1 << atoi(argv[++i])) & 0xFF;
				if (!data_bit)
					data_bit = 0x80;
			}
		}
		else if (!strcmp(argv[i],"-r")) {
			random_pad = TRUE;
		}
		else if (!strcmp(argv[i],"-bp")) {
			if (i != (argc-1)) {
				byte_pad = atol(argv[++i]);
			}
		}
		i++;
	}
	/* Following needed for binary stdin/stdout on DOS */

#ifdef DOS
	_fmode = O_BINARY;
	setmode (fileno(stdin), O_BINARY);
	setmode (fileno(stdout), O_BINARY);
#endif

	if (w <= 0 || h <= 0) {
		fprintf (stderr, "Width and height must be > 0 !\n");
		exit (1);
	}

	/* Calculate delta */

	dw /= (double)w;
	dh /= (double)h;

	/* If we have a full palette, we'll create all 256 colours */

	if (same_col || !full_palette)
		lim = 255 & ~data_bit;
	else
		lim = 255;

	/* I know random() isn't crypto-secure, but for our purposes it
	   doesn't neccesarily need to be. If you're really trying hard
	   to hide things, you ought to used idea_rand() from PGP or
	   something similar.  */

	srandom(time(0)+getpid());

#ifndef LOW_MEM
	/* Allocate data buffer */

	data = (byte *)malloc (w*h);

	if (!data) {
		fprintf(stderr, "Can't allocate %d bytes !\n",w*h);
		exit (1);
	}

	d = data;
#endif

	/* Generate a random palette */

	for (i=0; i<256; i++) {
		r[i] = random()&0xFF;
		g[i] = random()&0xFF;
		b[i] = random()&0xFF;
	}

	/* If -fp not specified, cut palette down to 128 colours */

	if (!full_palette) {
		int     j;

		for (i=0; i<256; i++) {
			j = i & (~data_bit);

			r[j|data_bit] = r[j];
			g[j|data_bit] = g[j];
			b[j|data_bit] = b[j];
		}
	}

#ifndef LOW_MEM
	ry = dys;

	for (iy = 0; iy < h; iy++) {
		ry += dh;
		rx = dxs;

		if (iy)
			opix = data[(iy-1)*w] & ~data_bit;

		for (ix = 0; ix < w; ix++) {
			rx += dw;
			x = 0.0;
			y = 0.0;

			nx2 = ny2 = 0.0;
			for (z = 0 ; z <= lim; z++) {
				nx = nx2-ny2+rx;
				ny = x*y*2.0+ry;

				if ((nx2=nx*nx)+(ny2=ny*ny) > 4.0)
					break;

				x = nx;
				y = ny;
			}

			if (z > lim)
				pix = (byte) lim;
			else
				pix = (byte) z;

			mpix = pix & ~data_bit;

			if (same_col || (mpix != opix && (ix || iy))) {
				if (last_missed) {
					last_missed = FALSE;
				}
				else {
				if (encoding) {
					if (!in_count) {
						in_count = 8;
						if (!byte_pad) {
							inc = getchar();
							write_state = WRITE_DATA;
						}
						if (inc == EOF) {
							inc = random() & 0xFF;
							write_state = WRITE_SUFFIX;
							eof_reached = TRUE;
						}
						else if (byte_pad) {
							inc = random() & 0xFF;
							byte_pad--;
						}
					}

					in_count--;
					if (random_pad || write_state == WRITE_DATA) {
						pix = mpix;
						if ((inc & 0x80))
							pix = mpix | data_bit;
						inc <<= 1;
					}
				}
				steg_count++;
				}
			}
			else
				last_missed = TRUE;

			*d++ = pix;
			opix = mpix;
		}
	}

	if (counting) {
		fprintf(stderr,"Image has space for %d bits (%d bytes)\n",
			steg_count,steg_count/8l);
	}

	if (encoding && !eof_reached) {
		fprintf(stderr,"WARNING : INPUT DATA TRUNCATED !\n");
	}

	save_gif_image(w,h,256,data,output_name,r,g,b);
#else
	save_gif_image(w,h,256,NULL,output_name,r,g,b);

	if (counting) {
		fprintf(stderr,"Image has space for %ld bits (%ld bytes)\n",
			steg_count,steg_count/8l);
	}

	if (encoding && !eof_reached) {
		fprintf(stderr,"WARNING : INPUT DATA TRUNCATED !\n");
	}
#endif
}

