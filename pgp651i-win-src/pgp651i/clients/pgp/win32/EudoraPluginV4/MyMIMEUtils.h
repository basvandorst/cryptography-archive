/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: MyMIMEUtils.h,v 1.3 1999/03/10 02:45:46 heller Exp $
____________________________________________________________________________*/
#ifndef Included_MyMIMEUtils_h	/* [ */
#define Included_MyMIMEUtils_h


 
#include "emssdk/ems-win.h"

long ParseFileForMIMEType(
	const char *		szInputFile,
	emsMIMEtype**		ppMimeType
);

void ParseMIMEType( 
	const char* szText, 
	emsMIMEtype** ppMimeType
);


#endif /* ] Included_MyMIMEUtils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

