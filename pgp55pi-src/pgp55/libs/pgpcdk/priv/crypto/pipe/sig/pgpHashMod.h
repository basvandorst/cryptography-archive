/*
 * pgpHashMod.h -- A Generic Hash Module.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpHashMod.h,v 1.10 1997/09/18 01:35:24 lloyd Exp $
 */
#ifndef Included_pgpHashMod_h
#define Included_pgpHashMod_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"


/*
 * Create a module that will hash its input and copy it to the output,
 * using the passed-in PGPHashContext.  The caller assumes control of the
 * PGPHashContext, and when this module returns success from a
 * sizeAdvise(0), the PGPHashContext is valid for use.
 *
 * Note: the calling function must destroy the PGPHashContext.
 */
PGPPipeline  **
pgpHashModCreate ( PGPContextRef cdkContext,
				PGPPipeline **head, PGPHashContext *hash);

PGPPipeline  **
pgpHashModListCreate ( PGPContextRef cdkContext,
	PGPPipeline **head, PGPHashListRef hashes, unsigned num);

PGP_END_C_DECLARATIONS

#endif
