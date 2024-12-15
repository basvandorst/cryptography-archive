/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	MyMIMEUtils.h,v 1.5 1997/10/22 23:05:53 elrod Exp
____________________________________________________________________________*/

#ifndef Included_MyMIMEUtils_h	/* [ */
#define Included_MyMIMEUtils_h

 
#include "emssdk/ems-win.h"

		long
AddMIMEParam(
	emsMIMEtypeP		pMimeType,
	const char*			nameString,
	const char*			valueString
);
			
	long
CreateMIMEType(
	const char*				typeString,
	const char*				subtypeString,
	emsMIMEtypeP *			ppMimeType
);
			
			
	void
DisposeMIMEType( 
	emsMIMEtypeP	pMimeType
);



	long
BuildEncryptedPGPMIMEType(
	emsMIMEtypeP *	ppMimeType,
	const char *	boundaryString 
);
			
	long
BuildSignedPGPMIMEType(
	emsMIMEtypeP *	ppMimeType,
	const char *	boundaryString 
);

long ParseFileForMIMEType(
	const char *		szInputFile,
	emsMIMEtype**		ppMimeType
);

void ParseMIMEType( 
	const char* szText, 
	emsMIMEtype** ppMimeType
);

BOOL GetMIMEParameter(
	emsMIMEtype*			pMimeType,
	const char*				paramName,
	char*					outValue 
);

/*


BOOL MatchMIMEType( emsMIMEtypeP	
					MIMEtypeHandle,
					const char* mType, 
					const char* subtype);



*/


#endif /* ] Included_MyMIMEUtils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
