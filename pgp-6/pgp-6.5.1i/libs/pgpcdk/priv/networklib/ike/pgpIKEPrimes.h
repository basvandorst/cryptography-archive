/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIKEPrimes.h,v 1.6 1998/12/04 12:11:32 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIKEPrimes_h	/* [ */
#define Included_pgpIKEPrimes_h

#include "pgpPubTypes.h"

typedef struct IKEGroup
{
	PGPUInt8	ikeID;
	PGPInt16	length;		// byte length of the prime
	PGPByte		prime[256];	// the prime
} IKEGroup;

extern IKEGroup kPGPike_Groups[];

#define kPGPikeNumIKEGroups		3

#endif /* ] Included_pgpIKEPrimes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/