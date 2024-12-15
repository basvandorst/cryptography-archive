/*
 * pgpRnd.h - system-specific gathering of high-resolution timers for RNG.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * The interface is system-independent, but the implementation should be
 * highly system-dependent, to get at as much state as possible.
 *
 * $Id: pgpRnd.h,v 1.8 1997/06/25 17:58:53 lloyd Exp $
 */

#include "pgpUsuals.h"
#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

/* formerly rndGetEntropy */

PGPUInt32 pgpRandomCollectEntropy(PGPRandomContext const *rc);

PGP_END_C_DECLARATIONS