/*
 * pgpRndSeed.h -- read and write the randseed.rnd file.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpRndSeed.h,v 1.14 1998/04/14 05:00:12 wprice Exp $
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
