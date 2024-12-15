/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	Lzss.h - Code based on Haruhiko Okumura's LZSS work


	$Id: Lzss.h,v 1.2 1999/02/25 11:14:28 heller Exp $
____________________________________________________________________________*/
#ifndef Included_LZSS_h	/* [ */
#define Included_LZSS_h

#include <stdio.h>

#define RB		 4096	/* size of ring buffer */
#define F_SS       18	/* upper limit for match_length */
#define F_ARI      60
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			RB	/* index for root of binary search trees */

void lz_init(void);

// Depending on whether LzssDec.c or LzssEnc.c
// (or both) is included, you may have two or only
// one of these functions. -wjb
void lzss_decompress(void *pUserValue);
void lzss_compress(void *pUserValue);

// Define these routines in _your_ code to give
// the LZSS code a way to input and output individual
// characters. By encoding/decoding at these points
// we can pipeline the crypto. -wjb

int compress_getc_buffer(void *pUserValue);
int compress_putc_buffer(int invoer,void *pUserValue);

int decompress_getc_buffer(void *pUserValue);
int decompress_putc_buffer(int invoer,void *pUserValue);

#endif /* ] Included_LZSS_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

