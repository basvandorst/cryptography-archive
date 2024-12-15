/*____________________________________________________________________________
	pgpErrors.h

	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Error codes for all PGP errors can be found in this file.

	$Id: pgpErrors.h,v 1.3 1999/03/10 02:50:30 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpErrors_h	/* [ */
#define Included_pgpErrors_h

#include "pgpPubTypes.h"


#include "pgpPFLErrors.h"

#define kPGPErrorRange		1000

enum PGPError_
{
	/*
		NOTE: error code values must not be changed;
		compiled client code depends on them.
	*/
	kPGPError_FirstError		= -11500,
	kPGPError_Last				= -10500,

	kPGPError_OutputBufferTooSmall				= -11491,
	kPGPError_InconsistentEncryptionAlgorithms	= -11490, 
	kPGPError_BadSignature						= -11448,
	kPGPError_BadHashNumber						= -11419,
	kPGPError_BadCipherNumber					= -11418,
	kPGPError_CantHash							= -11399,
	kPGPError_BadPacket							= -11391,
	kPGPError_BigNumNoInverse					= -11150,
	
	kPGPError_DummyEnumValue
	/* kPGPError_Last */
} ;



#endif /* ] Included_pgpErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/