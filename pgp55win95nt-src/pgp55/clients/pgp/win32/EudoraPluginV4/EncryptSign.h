/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: EncryptSign.h,v 1.1 1997/11/05 16:23:32 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_EncryptSign_h	/* [ */
#define Included_EncryptSign_h

 
#ifdef __cplusplus
extern "C" {
#endif

PGPError 
EncryptSign(HWND				hwndParent,		// Parent Window
			PGPKeySetRef*		keyset,			// main keyset
			PGPOptionListRef	pgpOptions,		// Encoding Options
			PGPOptionListRef	userOptions,	// Optional User Options Out
			char**				origRecipients,	// Array of email addresses
			DWORD				numRecipients,	// Length of Array
			BOOL				bEncrypt,		// Encrypt??
			BOOL				bSign			// Sign??
);

#ifdef __cplusplus
}
#endif


#endif /* ] Included_EncryptSign_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

