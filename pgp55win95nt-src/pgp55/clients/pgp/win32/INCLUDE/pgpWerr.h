/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPWerr.h	- 	common Windows error return codes

	$Id: pgpWerr.h,v 1.3 1997/10/10 19:54:18 pbj Exp $
	
____________________________________________________________________________*/
#ifndef Included_pgpWErr_h	/* [ */
#define Included_pgpWErr_h


#include "pgpPubTypes.h"
#include "pgpErrors.h"

enum PGPErrorWin32_
{
	kPGPError_Win32_FirstError = kPGPError_Last + 1,
	
	kPGPError_Win32_AlreadyOpen,		/* common dialog already open */
	kPGPError_Win32_NoSecretKeys,		/* no secret keys in keyring  */
	kPGPError_Win32_Expired,			/* eval period expired        */
	kPGPError_Win32_NegativeResponse,	/* user responded "No"		  */
	kPGPError_Win32_NoSigningKey,		/* no signing key for search  */

	kPGPError_Win32_Last
};


#endif /* ] Included_pgpWErr_h */
