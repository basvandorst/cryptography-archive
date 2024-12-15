/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorUtils.h,v 1.1.2.2 1997/11/22 01:08:08 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_TranslatorUtils_h	/* [ */
#define Included_TranslatorUtils_h

#define DELIMITERLENGTH 256

#include "emssdk/ems-win.h"

long ExtractAddressesFromMailHeader(
	emsHeaderDataP header, 
	char*** addresses
	);

void 
FreeRecipientList(
	char** Recipients, 
	unsigned long  numRecipients
	);

void 
StripMIMEHeader(
	const char* path
	);

void 
WrapFile(const char* szInputFile, 
		 int wrapLength
		 );



#endif /* ] Included_TranslatorUtils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/