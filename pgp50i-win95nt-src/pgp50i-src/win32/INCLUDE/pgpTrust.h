/*
* pgpTrust.h
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpTrust.h,v 1.4.2.1 1997/06/07 09:50:44 mhw Exp $
*/

#ifndef PGPTRUST_H
#define PGPTRUST_H

#include "pgpUsuals.h"

#define PGP_TRUST_DECADE (40*64)
#define PGP_TRUST_OCTAVE (12*64)
#define PGP_TRUST_MAX 0xfffe
#define PGP_TRUST_INFINITE 0xffff

#ifdef __cplusplus
extern "C" {
#endif

	byte	PGPExport ringTrustToExtern(word16 trust);
	word16	PGPExport ringTrustToIntern(byte trust);
	unsigned	PGPExport ringIntToTrust(unsigned long r);
unsigned long PGPExport ringTrustToInt(unsigned t);
unsigned PGPExport ringDoubleToTrust(double d);
double PGPExport ringTrustToDouble(unsigned t);
int PGPExport ringTrustValid(struct RingSet const *set,
	word16 validity);
#ifdef __GNUC__		/* XXX: Should be something like HAS_LONG_LONG */
unsigned long long PGPExport ringTrustToUll(unsigned t);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PGPTRUST_H */
