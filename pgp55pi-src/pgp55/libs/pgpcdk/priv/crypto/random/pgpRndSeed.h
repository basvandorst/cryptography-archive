/*
 * pgpRndSeed.h -- read and write the randseed.bin file.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpRndSeed.h,v 1.13 1997/09/30 20:00:23 heller Exp $
 */

#ifndef Included_pgpRndSeed_h
#define Included_pgpRndSeed_h

#include <stdio.h>

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

/* We want to see at least this many bytes in randseed */
#define PGP_SEED_MIN_BYTES		24



PGPError	pgpSeedGlobalRandomPool( PGPContextRef context );
PGPError	pgpSaveGlobalRandomPool( PGPContextRef context );
PGPBoolean	pgpGlobalRandomPoolIsSeeded( void );

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRndSeed_h */
