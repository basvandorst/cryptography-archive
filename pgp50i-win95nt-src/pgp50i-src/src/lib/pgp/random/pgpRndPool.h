/*
* pgpRndPool.h -- The interfaces to the pgpRandomPool. This exports
* the randompool RNG as well as interfaces to the entropy in
* the random pool, including random events
*
* Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRndPool.h,v 1.5.2.1 1997/06/07 09:51:35 mhw Exp $
*/

#ifndef PGPRNDPOOL_H
#define PGPRNDPOOL_H

#include "pgpRndom.h"
#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One true random number generator: the global random number pool. */
extern struct PgpRandomContext const PGPExport pgpRandomPool;

/* Dummy pool allows non-crypto-strong RNG's for public values */
extern struct PgpRandomContext const PGPExport pgpDummyPool;

/*
* Add the 'event' to the random pool. Returns the number of bits of
* entropy that was added to the random pool as a result of the
* irregularity in timing of this event. (This can also be used
* for button-clocks, menu selections, and so on - just assign
* each such event a unique "event" code.)
*/
unsigned PGPExport pgpRandPoolKeystroke (int event);

/*
* Add entropy based on the current mouse position to the random pool.
* Returns the number of bits of entropy that was added to the random
* pool as a result of the irregularity in timing of this event.
*/
unsigned PGPExport pgpRandPoolMouse (word32 x, word32 y);

/* Extra functions for entropy estimation */
unsigned PGPExport pgpRandPoolAddEntropy (word32 delta);
unsigned PGPExport pgpRandPoolEntropy (void);
unsigned PGPExport pgpRandPoolSize (void);

/*
* Return the number of bits added to the rand pool over its lifetime.
* This does not count any seed bits from pgpRndSeed.c.
*/
unsigned PGPExport pgpRandPoolInflow (void);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNDPOOL_H */
