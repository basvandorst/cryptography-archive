/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpRandomX9_17.h,v 1.4.12.1 1998/11/12 03:21:32 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpRandomX9_17_h	/* [ */
#define Included_pgpRandomX9_17_h

#include "pgpSymmetricCipher.h"
#include "pgpRandomContext.h"

PGP_BEGIN_C_DECLARATIONS

void	pgpRandomGetBytes(PGPRandomContext const *rc,
			void *buf, PGPSize len);

/*
 * Another: The X9.17 generator, with a specified cipher and
 * base random generator used to seed it.
 */
PGPRandomContext  *	pgpRandomCreateX9_17(
						PGPContextRef context, PGPCipherAlgorithm alg,
						PGPRandomContext const *base_rc);

/* The default: X9.17, with CAST5, based on the pgpRandomPool */
PGPRandomContext  *	pgpRandomCreate (PGPContextRef context);

/*
 * A non-strong pseudo-random number generator.  Use this only when
 * you don't need cryptographic strength, but just want some unique
 * values, and don't want to draw down the random pool (for example,
 * generating the public primes in a DSA key).  Based on the dummy
 * pool, which always produces zeros, and a CAST5 cipher used per
 * X9.17 to generate the pseudo random values.
 *
 * Note that you need to seed this with some really random data
 * (using pgpRandomAddBytes) or it will generate the same data every time.
 *
 * DO NOT USE THIS unless you know what you're doing.
 */
PGPRandomContext  *pgpPseudoRandomCreate ( PGPContextRef context );

/* Generate a random number in the specified range */
PGPUInt32		pgpRandomRange(PGPRandomContext const *rc, PGPUInt32 range);
	
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRandomX9_17_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
