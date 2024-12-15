/*
 * pgpVMemMod.h -- Module to output to a variable-size memory buffer
 *
 * Copyright (C) 1996, 1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpVMemMod.h,v 1.7 1999/03/10 02:59:59 heller Exp $
 */

#ifndef Included_pgpVMemMod_h
#define Included_pgpVMemMod_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpPipelineOpaqueStructs.h"
#include "pgpPubTypes.h"
#include "pgpMemoryMgr.h"


PGPPipeline  *	pgpVariableMemModCreate(
		PGPContextRef context,
		PGPPipeline **head, size_t max_size);

PGPPipeline  *	pgpSecureVariableMemModCreate(
		PGPContextRef context,
		PGPPipeline **head, size_t max_size);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpVMemMod_h */
