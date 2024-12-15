/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: TranslatorUtils.h,v 1.4 1999/03/10 03:04:13 heller Exp $
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


#endif /* ] Included_TranslatorUtils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/