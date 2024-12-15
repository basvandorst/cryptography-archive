/*
 * $Id: pgpDSAKey.h,v 1.13 1998/12/01 02:33:07 hal Exp $
 */

#ifndef Included_pgpDSAKey_h
#define Included_pgpDSAKey_h

#include "pgpPubTypes.h"	/* For PGPByte */
#include "pgpOpaqueStructs.h"
#include <stddef.h>	/* For size_t */

PGP_BEGIN_C_DECLARATIONS

/* Bits for uniqueness of p, q */
#define DSADUMMYBITS	64


PGPPubKey *dsaPubFromBuf( PGPContextRef	context,
				PGPByte const *buf, size_t len, PGPError *error);
				
PGPSecKey *dsaSecFromBuf( PGPContextRef	context,
				PGPByte const *buf, size_t len, PGPError *error);
				
int			dsaPubKeyPrefixSize(PGPByte const *buf, size_t size);

PGPSecKey *	dsaSecGenerate(PGPContextRef context,
				unsigned bits, PGPBoolean fastgen,
				PGPRandomContext const *rc,
				int progress(void *arg, int c), void *arg, PGPError *error);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpDSAKey_h */
