/*
 * pgpInfMod.h -- inflation module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpInfMod.h,v 1.5 1997/06/25 19:40:16 lloyd Exp $
 */

PGP_BEGIN_C_DECLARATIONS

#include "pgpPipelineOpaqueStructs.h"
#include "pgpPubTypes.h"

PGPPipeline **infModCreate (PGPContextRef cdkContext, PGPPipeline **head);

PGP_END_C_DECLARATIONS