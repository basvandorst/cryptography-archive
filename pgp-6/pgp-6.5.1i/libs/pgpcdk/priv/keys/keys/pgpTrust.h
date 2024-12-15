/*
 * $Id: pgpTrust.h,v 1.9 1997/07/14 04:27:14 hal Exp $
 */

#ifndef Included_pgpTrust_h
#define Included_pgpTrust_h

#include "pgpUsuals.h"

#define PGP_TRUST_DECADE (40*64)
#define PGP_TRUST_OCTAVE (12*64)
#define PGP_TRUST_MAX 0xfffe
#define PGP_TRUST_INFINITE 0xffff

PGP_BEGIN_C_DECLARATIONS

PGPByte        ringTrustToExtern(PGPUInt16 trust);
PGPUInt16      ringTrustToIntern(PGPByte trust);
PGPByte		   ringTrustExternToOld(RingPool const *pool, PGPByte externTrust);
PGPByte		   ringTrustOldToExtern(RingPool const *pool, PGPByte oldTrust);
unsigned       ringIntToTrust(unsigned long r);
unsigned long  ringTrustToInt(unsigned t);
unsigned       ringDoubleToTrust(double d);
double         ringTrustToDouble(unsigned t);
int            ringTrustValid(RingSet const *set,
	PGPUInt16 validity);
#ifdef __GNUC__		/* XXX: Should be something like HAS_LONG_LONG */
unsigned long long  ringTrustToUll(unsigned t);
#endif

PGP_END_C_DECLARATIONS

#endif /* Included_pgpTrust_h */