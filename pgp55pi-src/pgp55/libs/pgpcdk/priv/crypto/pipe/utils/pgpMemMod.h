/*
 * pgpMemMod.h -- Module to output to a fixed-size memory buffer
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.
 * All rights reserved
 *
 * $Id: pgpMemMod.h,v 1.8 1997/06/25 19:40:24 lloyd Exp $
 */

#ifndef Included_pgpMemMod_h
#define Included_pgpMemMod_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"


PGPPipeline  *	pgpMemModCreate( PGPContextRef cdkContext,
					PGPPipeline **head, char *buf, size_t buf_size);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpMemMod_h */
