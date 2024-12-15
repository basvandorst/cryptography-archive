/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpTLSPrimes.h,v 1.4 1999/03/25 18:36:29 melkins Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTLSPrimes_h	/* [ */
#define Included_pgpTLSPrimes_h

#include "pgpPubTypes.h"

typedef struct DHPrime
{
	PGPInt16	length;		/* byte length of the prime */
	PGPByte		prime[512];	/* the prime */
} DHPrime;

extern DHPrime	DH_1024bitPrime;
extern DHPrime	DH_1280bitPrime;
extern DHPrime	DH_1536bitPrime;
extern DHPrime	DH_2048bitPrime;
extern DHPrime	DH_3072bitPrime;
extern DHPrime	DH_4096bitPrime;

#endif /* ] Included_pgpTLSPrimes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/