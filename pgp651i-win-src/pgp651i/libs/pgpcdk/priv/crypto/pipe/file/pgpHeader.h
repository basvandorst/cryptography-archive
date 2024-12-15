/*
 * pgpHeader.h -- Create a PGP File Header.  It looks like a broken
 * ESK to older PGP versions, but this new code will understand it.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpHeader.h,v 1.8 1997/06/25 19:40:04 lloyd Exp $
 */

#ifndef Included_pgpHeader_h
#define Included_pgpHeader_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


PGPPipeline  **pgpHeaderCreate (PGPContextRef cdkContext,
				PGPPipeline **head);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpHeader_h */