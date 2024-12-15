/*
 * pgpDevNull.h -- A pipeline going nowhere
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpDevNull.h,v 1.8 1997/06/25 19:40:23 lloyd Exp $
 */

#ifndef Included_pgpDevNull_h
#define Included_pgpDevNull_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpPipelineOpaqueStructs.h"
#include "pgpPubTypes.h"

/*
 * Create a module that will eat all of its input.
 */
PGPPipeline  *pgpDevNullCreate ( PGPContextRef, PGPPipeline **head);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpDevNull_h */