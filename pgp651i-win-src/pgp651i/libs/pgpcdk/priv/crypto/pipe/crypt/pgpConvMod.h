/*
 * pgpConvMod.h -- Conventional Encryption Module
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConvMod.h,v 1.9 1997/06/25 19:39:59 lloyd Exp $
 */

#ifndef Included_pgpConvMod_h
#define Included_pgpConvMod_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS


PGPPipeline  **
pgpConvModCreate ( PGPContextRef cdkContext,
		PGPPipeline **head, PgpVersion version,
		  PGPFifoDesc const *fd,
		  PGPStringToKey const *s2k,
		  PGPConvKey const *convkeys,
		  PGPByte cipher, PGPByte const *session);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpConvMod_h */
