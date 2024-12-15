/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	LzssDec.c - Code based on Haruhiko Okumura's LZSS work
	

	$Id: LzssDec.c,v 1.2 1999/02/25 11:14:29 heller Exp $
____________________________________________________________________________*/

#include "lzss.h"

unsigned char  dec_text_buf[RB + F_ARI - 1];	/* ring buffer of size RB,
			with extra F_ARI-1 bytes to facilitate string comparison */

static void Decode(void *pUserValue)	/* Just the reverse of Encode(). */
{
	int  i, j, k, r, c;
	unsigned int  flags;

	for (i = 0; i < RB - F_SS; i++) dec_text_buf[i] = ' ';
	r = RB - F_SS;  flags = 0;
	for ( ; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if ((c = decompress_getc_buffer(pUserValue)) == EOF) break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			if ((c = decompress_getc_buffer(pUserValue)) == EOF) break;
			decompress_putc_buffer(c,pUserValue);  dec_text_buf[r++] = c;  r &= (RB - 1);
		} else {
			if ((i = decompress_getc_buffer(pUserValue)) == EOF) break;
			if ((j = decompress_getc_buffer(pUserValue)) == EOF) break;
			i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = dec_text_buf[(i + k) & (RB - 1)];
				decompress_putc_buffer(c,pUserValue);  dec_text_buf[r++] = c;  r &= (RB - 1);
			}
		}
	}
}

void lzss_decompress(void *pUserValue)
{
	Decode(pUserValue);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
