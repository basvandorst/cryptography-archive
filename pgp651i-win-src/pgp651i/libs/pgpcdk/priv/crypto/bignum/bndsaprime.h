/*
 * $Id: bndsaprime.h,v 1.4 1997/05/09 23:48:50 lloyd Exp $
 */

PGP_BEGIN_C_DECLARATIONS

#include "pgpBigNumOpaqueTypes.h"


/* Generate a pair of DSS primes */
int dsaPrimeGen(BigNum *bnq, BigNum *bnp,
	int (*f)(void *arg, int c), void *arg);

PGP_END_C_DECLARATIONS
