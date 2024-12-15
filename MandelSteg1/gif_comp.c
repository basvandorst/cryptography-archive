
/*
 * gif_compress.c : (C) Copyright 1994 Henry Hastur
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

#include "ext.h"
#include "gif.h"

typedef struct {

	word    pred;
	byte    code;
	word    next;
	
} ENTRY;

#define INIT_CODE_SIZE  9
#define BAD_CODE        0x4000
#define FIRST_CODE      0x4fff
#define CLEAR_CODE      256
#define EOF_CODE        257
#define HASH_SIZE       300

static  char    gif_head[] = "GIF87a";
static  ENTRY   code_table[MAX_CODES];
static  int16   code_size;
static  byte    out_buff[256];
static  lword   data_bit_offset = 0;
static  lword   in_offset = 0;
static  lword   in_size = 0;
static  word    max_code = 0;
static  word    size_max = 512;
static  word    hash[HASH_SIZE];

static void write_output(f)

FILE    *f;

{
	int16   i;

	fputc ((int)(data_bit_offset/8), f);

	for (i=0; i<data_bit_offset/8; i++) 
		fputc (out_buff[i], f);

	out_buff[0] = out_buff[i];

	data_bit_offset %= 8;
}

static void output_code(c,f)

word    c;
FILE    *f;

{
	int     data_byte_offset;
	lword   code;

	code = c;
	code <<= (data_bit_offset%8);

	data_byte_offset = data_bit_offset/8;

	out_buff[data_byte_offset++]|=(code&0xff);
	out_buff[data_byte_offset++]=((code>>8)&0xff);
	out_buff[data_byte_offset++]=((code>>16)&0xff);

	data_bit_offset += code_size;

	if (data_byte_offset > 250)
		write_output (f);
}

static do_clear()

{
	int     i;

	for (i=0; i<256; i++) {
		code_table[i].pred = FIRST_CODE;
		code_table[i].code = i;
		code_table[i].next = BAD_CODE;
	}

	code_table[CLEAR_CODE].pred = BAD_CODE;
	code_table[EOF_CODE].pred = BAD_CODE;

	for (i=0; i<HASH_SIZE; i++)
		hash[i] = BAD_CODE;

	code_size = INIT_CODE_SIZE;
	max_code = EOF_CODE+1;
	size_max = 512;
}

static word hash_val(p,c)

word    p;
byte    c;

{
	return ((p+c) % HASH_SIZE);
}

static file_word(w,fp)

word    w;
FILE    *fp;

{
	fputc (w&0xff, fp);
	fputc ((((int)(w&0xff00))>>8), fp);
}

void    save_gif_image(width,height,depth,d,gif_name,r,g,b)

int32	width;
int32	height;
int     depth;
byte    *d,*r,*g,*b;
char    *gif_name;

{
	int16   i,c;
	int16   code;
	word    current;
	FILE    *gif_file;

	if (gif_name)
		gif_file=fopen(gif_name,"wb");
	else
		gif_file = stdout;

	if (!gif_file) {
		fprintf(stderr,"Cannot open file %s\n",gif_name);
		return;
	}

	memset(hash, 0, sizeof(hash));
	memset(out_buff, 0, sizeof(out_buff));

	data_bit_offset = 0;

	/* Write 'magic string' to gif file */

	fwrite (gif_head, 6, 1, gif_file);

	/* Write header */

	file_word((word)width, gif_file);
	file_word((word)height, gif_file);
	fputc(0x80+0x70+8-1, gif_file);
	fputc(0, gif_file);
	fputc(0, gif_file);

	for (i=0;i<256;i++) {
		fputc(r[i], gif_file);
		fputc(g[i], gif_file);
		fputc(b[i], gif_file);
	}

	fputc(0x2C, gif_file);
	file_word(0, gif_file);
	file_word(0, gif_file);
	file_word((word)width, gif_file);
	file_word((word)height, gif_file);
	
	fputc(0, gif_file);
	fputc(INIT_CODE_SIZE-1, gif_file);

	/* Header finished */

	do_clear ();
	output_code (CLEAR_CODE, gif_file);

	current=FIRST_CODE;

	in_size = (width*height);
	in_offset = 0;

	do {
		if (in_offset < in_size) {
#ifdef LOW_MEM
			code = calc_pixel (in_offset++);
#else
			code = d[in_offset++];
#endif
			if (current == FIRST_CODE)
				current = code;
			else {
				word new;
				word hash_no;
				word prev;

				hash_no = hash_val(current,code);
				new = hash[hash_no];
				prev = BAD_CODE;

				while(new < max_code) {
					if (code_table[new].pred==current&&
						code_table[new].code==code) 
						break;
					prev=new;
					new=code_table[new].next;
				}

				if (new<max_code) {
					current=new;
				}
				else {
					output_code(current,gif_file);

					code_table[max_code].pred=current;
					code_table[max_code].code=code;
					code_table[max_code].next=BAD_CODE;

					if (hash[hash_no]==BAD_CODE) {
						hash[hash_no]=max_code;
					}
					else {
						if (prev!=BAD_CODE)
							code_table[prev].next=
								max_code;
					}

					max_code++;
					if (max_code>size_max) {
						size_max*=2;
						code_size++;
					}
					if (max_code >= (word)(MAX_CODES - 2)) {
						output_code(CLEAR_CODE,
							gif_file);
						do_clear();
					}
					current=code;
				}
			}
		}

	} while(in_offset<in_size);

	output_code(current,gif_file);
	output_code(EOF_CODE,gif_file);

	data_bit_offset += 8;

	write_output(gif_file);

	putc(0, gif_file);
	putc(0x3B, gif_file);
	fclose(gif_file);
}
