/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: MyMIMEUtils.c,v 1.2.8.1 1998/11/12 03:11:47 heller Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpMem.h"

// Eudora Headers
#include "emssdk/ems-win.h"
#include "emssdk/mimetype.h"
#include "emssdk/encoding.h"

// Project Headers
#include "MapFile.h"
#include "TranslatorUtils.h"
#include "MyMIMEUtils.h"


/*_______________________________________________________________
	*ppMimeType is set to NULL if no valid Content-type is found.
________________________________________________________________*/

	void
ParseMIMEType(
	const char*			szText,
	emsMIMEtype**	ppMimeType
)
{
	*ppMimeType	= NULL;
	
	*ppMimeType = parse_make_mime_type(szText);

	if ( *ppMimeType )
	{
		// parse_mime_ct doesn't fill in mime_version
		(*ppMimeType)->version = strdup( "1.0");
	}
}

long
ParseFileForMIMEType(
	const char *		szInputFile,
	emsMIMEtype**		ppMimeType
)
{
	long err = 0;
	char* buffer = NULL;
	DWORD length = 0;

	buffer	= MapFile( szInputFile, &length );
	
	if ( buffer )
	{
		ParseMIMEType( buffer, ppMimeType);

		free(buffer);
	}
	else
	{
		err = 1;
	}
	
	return( err );
}





