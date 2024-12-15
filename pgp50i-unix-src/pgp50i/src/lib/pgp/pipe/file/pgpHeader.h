/*
 * pgpHeader.h -- Create a PGP File Header.  It looks like a broken
 *                ESK to older PGP versions, but this new code will
 *                understand it.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpHeader.h,v 1.4.2.1 1997/06/07 09:50:56 mhw Exp $
 */

#ifndef PGPHEADER_H
#define PGPHEADER_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpPipeline PGPExport **pgpHeaderCreate (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif

#endif /* PGPHEADER_H */
