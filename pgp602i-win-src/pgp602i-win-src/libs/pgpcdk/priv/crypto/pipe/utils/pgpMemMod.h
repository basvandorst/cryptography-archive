/*
 * pgpMemMod.h -- Module to output to a fixed-size memory buffer
 *
 * Copyright (C) 1996, 1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpMemMod.h,v 1.8.30.1 1998/11/12 03:21:28 heller Exp $
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
