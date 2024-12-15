/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: TranslatorUtils.h,v 1.3.8.1 1998/11/12 03:11:51 heller Exp $
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