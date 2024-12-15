/*
 * pgpRndSeed.h -- read and write the randseed.rnd file.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpRndSeed.h,v 1.15 1999/04/13 17:39:33 cpeterson Exp $
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
void		pgpSetIsSeeded( void);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRndSeed_h */