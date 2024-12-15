/*
 * $Id: bnprime.h,v 1.6 1997/05/13 01:18:53 mhw Exp $
 */

#ifndef Included_bnprime_h
#define Included_bnprime_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpBigNumOpaqueStructs.h"

/* Generate a prime >= bn. leaving the result in bn. */
int bnPrimeGen(BigNum *bn, unsigned (*randnum)(unsigned),
	       int (*f)(void *arg, int c), void *arg, unsigned exponent, ...);

/*
 * Generate a prime of the form bn + k*step.  Step must be even and
 * bn must be odd.
 */
int bnPrimeGenStrong(BigNum *bn, BigNum const *step,
		     int (*f)(void *arg, int c), void *arg);

PGP_END_C_DECLARATIONS

#endif /* Included_bnprime_h */
