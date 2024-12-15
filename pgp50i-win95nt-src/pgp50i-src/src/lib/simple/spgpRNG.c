/*
* spgpRNG.c -- Simple PGP API (extended) Random Number Generator
* related routines
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpRNG.c,v 1.1.2.1 1997/06/07 09:51:50 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

#include "pgpRndSeed.h"


/*
 * Detect uninitialized RNG state. This happens if we did not find
 * a randseed file, and the app hasn't added any entropy to the
 * randpool.
 * If no randseed was found and so we returned an error, then the app
 * added entropy to the randpool, we return OK. However when this
 * entropy gets used up, we might spuriously return another error.
 * So we will set pgpRngSeeded when we return OK to prevent this.
 */
PGPError
spgpRngCheck ()
{
	extern PGPKDBExport Boolean pgpRngSeeded;
	if (pgpRngSeeded)
		return PGPERR_OK;
	if (pgpRandPoolEntropy() != 0) {
		pgpRngSeeded = TRUE;
		return PGPERR_OK;
	}
	return PGPERR_RANDSEED_TOOSMALL;
}

/*
* Return the number of random bits which should be requested from the
* user if a function returned an error due to an unseeded RNG.
* The PGP3 library's randomness functions should be used to acquire
* this many bits from the user.
*
* We use a value from pgpRndSeed.h.
*/
int SPGPExport
SimplePGPRandomNeeded (void *handle)
{
	(void)handle;
	if (spgpRngCheck () == PGPERR_OK)
		return 0;
	return PGP_SEED_MIN_BYTES * 8;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
