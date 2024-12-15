/*
 * Gifextract.c V1.0 
 * (C) Copyright Henry Hastur 1994
 *
 * Extracts an arbitrary bit-plane from a GIF file. 
 *
 * This program is copyright Henry Hastur 1994, but may be freely distributed,
 * modified, incorporated into other programs and used, as long as the
 * copyright stays attached and you obey any relevant import or export
 * restrictions on the code. No warranty is offered, and no responsibility
 * is taken for any damage use of this program may cause. In other words,
 * do what you want with it, but don't expect me to pay up if anything
 * unexpected goes wrong - you're using it at your own risk...
 *
 * Note : This file contains some fairly bizarre uses of casts. These
 * were neccesary to get DOS to work, and once it was going I wasn't
 * brave enough to remove any that appeared to be unneccesary in case
 * I broke everything again !
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifdef DOS
#include <fcntl.h>
#include <io.h>
#endif

#include "ext.h"
#include "gif.h"

/* The following defines the size of buffer we use. Set it to whatever
   seems reasonable for your machine - the larger the better. */

#define IN_SIZE         8000
#define OUT_SIZE        1024

static  int32   data_size = 0;
static  int32   data_bit_offset = 0;

static  int16   bm_width, bm_height;
static  int16   lzw_code_size, lzw_init_size;

static  word    lzw_clear_code, lzw_eof_code, lzw_max_code;
static  word    lzw_code_read, lzw_read_mask;
static  word    lzw_first_free, lzw_next_free;

typedef struct {

	word    pred;
	word    code;

} ENTRY;

static  ENTRY   code_table[MAX_CODES];
static  byte    lzw_pix_buffr[OUT_SIZE];

static  int16   x_coord = 0, y_coord = 0;

static  byte    same_col = TRUE;
static  byte    data_bit = 0x80;
static  byte    analyse = FALSE;

static  int32   byte_pad = 0;
static  int32   steg_size = 0l;
static  int32   zero_bits = 0;
static  int32   one_bits = 0;

/* Read a word in little-endian form */

static word get_w(fp)

FILE    *fp;

{
	word    c1,c2;

	c1 = getc(fp);
	c2 = getc(fp);

	return ((c2 << 8) + c1);
}

/* Read some data from the file into our buffer */

static void read_some_data(fp, compressed, start_byte_pos)

FILE    *fp;
byte    *compressed;
long    start_byte_pos;

{
	int16   i,c = 0;
	int16   data_block_length;
	long    j;

	j = start_byte_pos;
	do {
		data_block_length = getc(fp);
		for (i=0; c != EOF && i < data_block_length; i++) {
			c = getc(fp);
			if (c != EOF)
				compressed[j++] = c;
		}

	} while(data_block_length > 0 && c != EOF && (j < (IN_SIZE - 256)));

	data_size = j;
}

/* Extract a variable-length code from the compressed data */

static int read_a_code(fp,compressed)

FILE    *fp;
byte    *compressed;

{
	lword   code24;
	long    data_byte_offset;
	word    c1, c2;	/* These are used to keep DOS happy */

	data_byte_offset = data_bit_offset/8;

	if (data_byte_offset > (data_size-8)) {
		int     i;

		for (i=0; i<(data_size - data_byte_offset); i++) {
			compressed[i] = compressed[data_byte_offset+i];
		}

		read_some_data (fp, compressed, (long)i);

		data_byte_offset=0l;
		data_bit_offset%=8;

		if (!data_size)
			return FALSE;
	}

	/* Unix worked fine without temporary storage in c1 and c2,
	   but DOS was getting casts wrong. This works, and I don't
	   want to change it ! */

	c1 = compressed[data_byte_offset]
		+ (compressed[data_byte_offset + 1] << 8);

	if (lzw_code_size >= 8) {
		c2 = (compressed[data_byte_offset + 2]);
		code24 = (lword)c1 + (((lword)c2) << 16);
	}
	else
		code24 = (lword)c1 ;

	code24 >>= (data_bit_offset % 8);
	lzw_code_read = (code24 & (lword)lzw_read_mask);

	data_bit_offset += lzw_code_size;

	return TRUE;
}

/* Output a pixel - in our case extract the stegg-ed data */

static void     output_pixel (val)

byte    val;

{
	static  byte    last_startpix = 0;
	static  int     outc = 0;
	static  int     out_count = 0;
	static  byte    opix = 0;
	static  byte    last_missed = FALSE;
	byte    pix;

	if (!x_coord) {
		if (y_coord)
			opix = last_startpix;
		last_startpix = val & (~data_bit);
	}

	pix = val & (~data_bit); 

	if (same_col || (pix != opix && (x_coord || y_coord))) {
		if (last_missed) 
			last_missed = FALSE;
		else {
			outc <<= 1;
			if (val & data_bit)
				outc |= 1;

			if (analyse && !byte_pad) {
				if (val & data_bit)
					one_bits ++;
				else
					zero_bits ++;
			}

			out_count++;

			if (out_count == 8) {

				if (!byte_pad) {
					if (!analyse)
						putchar(outc);
				}
				else
					byte_pad--;

				out_count = 0;
				outc = 0;
			}
		}
	}
	else
		last_missed = TRUE;

	opix = pix;

	x_coord++;
	if (x_coord >= bm_width) {
		x_coord=0;
		y_coord++;
	}
}

/* Print a usage message */

void    usage ()

{
	fprintf(stderr,"Usage: gifextract [-a] [-ns] [-bp padding] [-b bit] [file]\n");
}

/* Do everything else */

main(argc,argv)

int     argc;
char    *argv[];

{
	char    header_string[7];
	FILE    *input_file;
	byte    *compressed = NULL;
	char    *file_name = NULL;
	word    lzw_current_code = 0, lzw_old_code = 0, lzw_in_code = 0;
	int     i, buffered_pixels = 0;
	byte    has_colormap = 0, last_pixel = 0, pixel_bit_mask = 0;
	word    num_colors = 0, bits_per_pixel = 0;

	i = 1;

	while (i < argc) {
		if (!strcmp(argv[i],"-ns")) {
			same_col = FALSE;
		}
		else if (!strcmp(argv[i],"-a")) {
			analyse = TRUE;
		}
		else if (!strcmp(argv[i],"-b")) {
			if (i == (argc - 1)) {
				usage ();
				exit (1);
			}
			data_bit = ((1 << atoi(argv[++i])) & 0xFF);
			if (!data_bit) {
				fprintf(stderr, "Bad bit specified, defaulting to 7!\n");
				data_bit = 0x80;
			}
		}
		else if (!strcmp(argv[i],"-bp")) {
			if (i == (argc - 1)) {
				usage ();
				exit (1);
			}
			byte_pad = atol (argv[++i]);
		}
		else
			file_name = argv[i];
		i++;
	}
	       
	/* Following needed for binary stdin/stdout on DOS */

#ifdef DOS
	_fmode = O_BINARY;
	setmode (fileno(stdin), O_BINARY);

	if (!analyse) {
		setmode (fileno(stdout), O_BINARY);
	}
#endif

	steg_size = 0;
     
	if ((compressed=(byte *)malloc(IN_SIZE))==NULL) {
		fprintf(stderr,"Not enough memory !\n");
		exit (1);
	}

	if (!file_name)
		input_file = stdin;
	else {
		input_file = fopen(file_name,"rb");
		if (!input_file) {
			fprintf(stderr,"Can't open %s\n",file_name);
			exit (1);
		}
	}

	fread (header_string,6,1,input_file);
	if (strncmp(header_string,"GIF",3)) {
		fprintf(stderr,"This is not a GIF file !\n");
		exit (1);
	}

	(void) get_w(input_file);
	(void) get_w(input_file);

	i = getc(input_file);
     
	has_colormap = (i & COLORMAP_BIT);
	bits_per_pixel = (i & 7) + 1;

	num_colors = (1 << bits_per_pixel);
	pixel_bit_mask = num_colors - 1;

	(void) getc(input_file);
	(void) getc(input_file);

	if (has_colormap) {
		int     n = 0;

		n = num_colors * 3;
		for (i = 0; i < n; i++)
			(void) getc(input_file);
	}

	(void) getc(input_file);
	(void) get_w(input_file);
	(void) get_w(input_file);
	bm_width = get_w(input_file);
	bm_height = get_w(input_file);

	if (analyse)
		printf("Image is %d x %d pixels\n", bm_width, bm_height);

	if (getc (input_file) & INTERLACE_BIT)
		fprintf(stderr, "Interlaced images not supported - dubious output !\n");

	x_coord = 0;    
	y_coord = 0;
	data_bit_offset = 0l;  

	lzw_code_size = getc(input_file);
	lzw_clear_code = (1 << lzw_code_size);
	lzw_eof_code = lzw_clear_code + 1;
	lzw_next_free = lzw_first_free = lzw_clear_code+2;

	lzw_code_size++;
	lzw_init_size = lzw_code_size;
	lzw_max_code = (1 << lzw_code_size);
	lzw_read_mask = (lzw_max_code - 1);

	read_some_data (input_file, compressed, 0l);

	buffered_pixels = 0;

	do {
		if (!read_a_code(input_file,compressed)) {
			for (i = (buffered_pixels - 1); i >= 0; i--)
				output_pixel(lzw_pix_buffr[i]);
			goto gif_exit;
		}

		if (lzw_code_read != lzw_eof_code) {
			if (lzw_code_read == lzw_clear_code) {

				lzw_code_size = lzw_init_size;
				lzw_max_code = (1<<lzw_code_size);
				lzw_next_free = lzw_first_free;
				lzw_read_mask = ((1 << lzw_code_size) - 1);

				if (!read_a_code(input_file,compressed)) {
					goto gif_exit;
				}
				lzw_current_code = lzw_code_read;       
				lzw_old_code = lzw_code_read;
				last_pixel = lzw_code_read & pixel_bit_mask;
				output_pixel(last_pixel);
			}
			else {

				lzw_current_code = lzw_in_code = lzw_code_read;

				if (lzw_code_read >= lzw_next_free) {
					lzw_current_code = lzw_old_code;
					lzw_pix_buffr[buffered_pixels++]=last_pixel;
					if (buffered_pixels > OUT_SIZE) {
						fprintf(stderr,"Oops ! pixels > OUT_SIZE!\n");
						goto gif_exit;
					}
				}

				if (lzw_current_code > pixel_bit_mask) {
					do {
						lzw_pix_buffr[buffered_pixels++] =
							code_table[lzw_current_code].code;
						lzw_current_code = 
							code_table[lzw_current_code].pred;
						if (buffered_pixels > OUT_SIZE) {
							goto gif_exit;
						}
					} while (lzw_current_code > pixel_bit_mask);
				}

			last_pixel = (lzw_current_code & pixel_bit_mask);
			lzw_pix_buffr[buffered_pixels] = last_pixel;

			for (i = buffered_pixels ; i >= 0 ; i--)
				output_pixel (lzw_pix_buffr[i]);

			buffered_pixels=0;

			code_table[lzw_next_free].code = last_pixel;
			code_table[lzw_next_free].pred = lzw_old_code;
			lzw_old_code = lzw_in_code;

			if (++lzw_next_free >= lzw_max_code) { 
				if (lzw_code_size<12) {
					lzw_code_size++;
					lzw_max_code <<= 1;
					lzw_read_mask = ((1 << lzw_code_size) - 1);
				}
			}
		}
	    }
       } while (lzw_code_read != lzw_eof_code);

gif_exit:
	fclose(input_file);
	free(compressed);

	if (analyse) {
		printf("Analysed file : %ld zero bits, %ld one bits in plane\n",
			zero_bits, one_bits);
	}

	exit(0);
}

