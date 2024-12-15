/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPWerr.h	- 	common Windows error return codes

	$Id: pgpWerr.h,v 1.8 1998/08/11 14:06:21 pbj Exp $
	
____________________________________________________________________________*/
#ifndef Included_pgpWErr_h	/* [ */
#define Included_pgpWErr_h


#include "pgpPubTypes.h"
#include "pgpClientErrors.h"

enum PGPErrorWin32_
{
	kPGPError_Win32_FirstError = kPGPClientError_FirstError - 100,
	
	kPGPError_Win32_AlreadyOpen,		/* common dialog already open */
	kPGPError_Win32_NoSecretKeys,		/* no secret keys in keyring  */
	kPGPError_Win32_Expired,			/* eval period expired        */
	kPGPError_Win32_NegativeResponse,	/* user responded "No"		  */
	kPGPError_Win32_NoSigningKey,		/* no signing key for search  */

	kPGPError_Win32_InvalidImage,		/* corrupt or unsupported image */
	kPGPError_Win32_ImageTooBig,		/* image beyond limits          */

	kPGPError_Win32_InvalidComctl32,	/* incorrect version of COMCTL32*/

	kPGPError_Win32_Last
};


#endif /* ] Included_pgpWErr_h */