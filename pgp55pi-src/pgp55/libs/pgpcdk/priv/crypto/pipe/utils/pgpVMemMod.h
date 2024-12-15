/*
 * pgpVMemMod.h -- Module to output to a variable-size memory buffer
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.
 * All rights reserved
 *
 * $Id: pgpVMemMod.h,v 1.4 1997/06/25 19:40:27 lloyd Exp $
 */

#ifndef Included_pgpVMemMod_h
#define Included_pgpVMemMod_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpPipelineOpaqueStructs.h"
#include "pgpPubTypes.h"


PGPPipeline  *	pgpVariableMemModCreate(
		PGPContextRef cdkContext,
		PGPPipeline **head, size_t max_size);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpVMemMod_h */
