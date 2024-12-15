/*____________________________________________________________________________
	pgpClientErrors.h

	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	Error codes for all PGP errors can be found in this file.

	$Id: pgpClientErrors.h,v 1.7.8.1 1998/11/12 03:11:05 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpClientErrors_h	/* [ */
#define Included_pgpClientErrors_h

#include "pgpErrors.h"

enum PGPClientError_
{
	kPGPClientError_FirstError					=	-12500,
	kPGPClientError_Last						=	-12001,

	/* Share errors */
	kPGPClientError_NotEnoughSharesInObject		=	-12500,
	kPGPClientError_DifferentSplitKeys			=	-12499,
	kPGPClientError_DifferentSharePool			=	-12498,
	kPGPClientError_IdenticalShares				=	-12497,

	/* SKEP errors */
	kPGPClientError_IncorrectSKEPVersion		=	-12479,
	kPGPClientError_RejectedSKEPAuthentication	=	-12478,

	/* Pref errors */
	kPGPClientError_AdminPrefsNotFound			=	-12450,
	
	/* Reconstitution Errors */
	kPGPClientError_UnsplitPrivateKeyNotFound	=	-12425,

	kPGPClientError_Dummy
} ;


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError 	PGPGetClientErrorString( PGPError theError,
				PGPSize bufferSize, char * theString );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpClientErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/