/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	$Id: DeflateWrapper.h,v 1.1 1999/02/19 17:09:50 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_DeflateWrapper_h	/* [ */
#define Included_DeflateWrapper_h

void Deflate_Decompress(void *pUserValue);
void Deflate_Compress(void *pUserValue);

// Define these routines in _your_ code to give
// the Deflate code a way to input and output individual
// characters. By encoding/decoding at these points
// we can pipeline the crypto. -wjb

int getc_buffer(void *pUserValue);
int putc_buffer(int invoer,void *pUserValue);

#endif /* ] Included_DeflateWrapper_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

