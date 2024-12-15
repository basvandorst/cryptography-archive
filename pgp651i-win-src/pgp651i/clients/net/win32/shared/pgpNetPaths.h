/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPnetPaths.h - PGPnet module paths header
	

	$Id: pgpNetPaths.h,v 1.4 1999/04/08 17:19:43 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGPNETPATHS_H
#define _PGPNETPATHS_H

// includes
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS
// prototypes
PGPError
PGPnetGetPGPnetFullPath (
		LPSTR			szPath, 
		UINT			uLen,
		UINT*			puLen);
 
PGPError
PGPnetGetPrefsFullPath (
		LPSTR			pszFileName,
		UINT			uLen);

PGPError
PGPnetGetLogFileFullPath (
		LPSTR			pszFileName,
		UINT			uLen);

PGPError
PGPnetGetTrayAppFullPath (
		LPSTR			pszFileName,
		UINT			uLen);

PGPError
PGPnetGetAppFullPath (
		LPSTR			pszFileName,
		UINT			uLen);

PGPError
PGPnetGetHelpFullPath (
		LPSTR			pszFileName,
		UINT			uLen);

PGPError
PGPnetLoadSDKPrefs (
		PGPContextRef	context);


PGP_END_C_DECLARATIONS

#endif //_PGPNETPATHS_H