/*
 * $Id: bngermain.h,v 1.6 1997/05/13 01:18:37 mhw Exp $
 */

#ifndef Included_bngermain_h
#define Included_bngermain_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpBigNumOpaqueStructs.h"

/* Generate a Sophie Germain prime */
int bnGermainPrimeGen(BigNum *bn, unsigned dbl,
	int (*f)(void *arg, int c), void *arg);

PGP_END_C_DECLARATIONS

#endif /* Included_bngermain_h */
