/*
 * File: BUFTYPE.H
 *
 * Date: Wednesday, August 28, 1996
 *
 * Author: Scott Manjourides
 *
 * Copyright (c) 1996 QUALCOMM, Incorporated
 *
 */

#ifndef BUFTYPE_H
#define BUFTYPE_H

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef struct {
	char *buf;
	unsigned int buf_size;
	unsigned int len;
	char *pos;
} BufType;

typedef BufType *BufTypePtr;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

BufTypePtr makesize_buf(unsigned int n);
BufTypePtr make_buf();

/* boolean */ int isempty_buf(BufTypePtr bp);
unsigned int availspace_buf(BufTypePtr bp);
unsigned int buflen_buf(BufTypePtr bp); /* Number of valid chars in buffer. Always < bufsize_buf() */
char *getbuf_buf(BufTypePtr bp);

char *getend_buf(BufTypePtr bp);
unsigned int bufsize_buf(BufTypePtr bp); /* The size of the buffer -- used or not */
unsigned int buflen_buf(BufTypePtr bp);
/* boolean */ int setlen_buf(BufTypePtr bp, const unsigned int n);

unsigned int bufncat_buf(BufTypePtr dst, BufTypePtr src, unsigned int n);
unsigned int bufins_buf(BufTypePtr b1, BufTypePtr b2);
unsigned int bufnins_buf(BufTypePtr b1, BufTypePtr b2, unsigned int n);

unsigned int poslen_buf(BufTypePtr bp); /* Number of valid chars, starting at current position */

void incpos_buf(BufTypePtr bp, unsigned int n);
void delpos_buf(BufTypePtr bp);
char *getpos_buf(BufTypePtr bp); /* NULL problems */
void resetpos_buf(BufTypePtr bp);

unsigned int strcpy_buf(BufTypePtr dst, const char *src);
unsigned int strncpy_buf(BufTypePtr dst, const char *src, const unsigned int n);
unsigned int strcat_buf(BufTypePtr dst, const char *src);
unsigned int strncat_buf(BufTypePtr dst, const char *src, const unsigned int n);

void delete_buf(BufTypePtr bp);
void free_buf(BufTypePtr bp);
void clear_buf(BufTypePtr bp);
void emptybuf_buf(BufTypePtr bp);

unsigned int completecount_buf(BufTypePtr src, BufTypePtr find, const unsigned int n);
unsigned int skipcount_buf(BufTypePtr buf, BufTypePtr find);

#endif
