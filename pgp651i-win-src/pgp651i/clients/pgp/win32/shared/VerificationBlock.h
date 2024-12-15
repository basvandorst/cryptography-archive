/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: VerificationBlock.h,v 1.3 1999/03/10 03:04:53 heller Exp $
____________________________________________________________________________*/
#ifndef Included_VerificationBlock_h	/* [ */
#define Included_VerificationBlock_h

#include "pgpEncode.h"

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

PGPError CreateVerificationBlock(HINSTANCE hInst, 
								 PGPContextRef context,
								 PGPEventSignatureData *sigData, 
								 PGPBoolean wasEncrypted,
								 char **blockBegin,
								 char **blockEnd);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif /* ] Included_VerificationBlock_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
