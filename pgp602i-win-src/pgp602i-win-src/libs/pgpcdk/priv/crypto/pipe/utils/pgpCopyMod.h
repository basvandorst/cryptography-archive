/*
 * pgpCopyMod.h -- Copy the input directly to the output.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCopyMod.h,v 1.6 1997/06/25 19:40:23 lloyd Exp $
 */

#ifndef Included_pgpCopyMod_h
#define Included_pgpCopyMod_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"


PGPPipeline **pgpCopyModCreate ( PGPContextRef cdkContext, PGPPipeline **head);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpCopyMod_h */
