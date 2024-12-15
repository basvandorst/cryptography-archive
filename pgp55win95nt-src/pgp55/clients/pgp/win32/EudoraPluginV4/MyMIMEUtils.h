/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MyMIMEUtils.h,v 1.1.2.1 1997/11/14 06:19:44 elrod Exp $
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

