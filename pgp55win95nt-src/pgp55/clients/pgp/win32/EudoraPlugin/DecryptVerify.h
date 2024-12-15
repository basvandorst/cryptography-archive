/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: DecryptVerify.h,v 1.1 1997/07/28 17:23:06 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_DecryptVerify_h	/* [ */
#define Included_DecryptVerify_h


#ifdef __cplusplus
extern "C" {
#endif

PGPError 
DecryptVerify(	HWND				hwndParent,		// Parent Window
				PGPOptionListRef	pgpOptions		// Decoding Options
		
);

#ifdef __cplusplus
}
#endif


#endif /* ] Included_DecryptVerify_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

