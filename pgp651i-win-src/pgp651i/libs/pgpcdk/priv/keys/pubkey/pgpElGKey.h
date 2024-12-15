/*
 * $Id: pgpElGKey.h,v 1.10 1997/06/25 19:40:55 lloyd Exp $
 */

#ifndef Included_pgpElGKey_h
#define Included_pgpElGKey_h

#include "pgpPubTypes.h"	/* For PGPByte */
#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

/* Bits for uniqueness of p, q */
#define ELGDUMMYBITS	64

PGPPubKey *	elgPubFromBuf( PGPContextRef context,
				PGPByte const *buf, size_t len, PGPError *error);
				
PGPSecKey *	elgSecFromBuf( PGPContextRef context,
				PGPByte const *buf, size_t len, PGPError *error);
			
int			elgPubKeyPrefixSize(PGPByte const *buf, size_t size);

PGPSecKey *	elgSecGenerate(PGPContextRef context,
				unsigned bits, PGPBoolean fastgen,
				PGPRandomContext const *rc,
				int progress(void *arg, int c), void *arg, PGPError *error);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpElGKey_h */