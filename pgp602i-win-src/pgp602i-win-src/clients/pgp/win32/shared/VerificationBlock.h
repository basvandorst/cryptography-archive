/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: VerificationBlock.h,v 1.2.8.1 1998/11/12 03:14:15 heller Exp $
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
