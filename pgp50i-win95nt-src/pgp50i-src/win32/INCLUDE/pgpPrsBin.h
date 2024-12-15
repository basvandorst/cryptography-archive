/*
 * pgpPrsBin.h -- The PGP Binary Message parser
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsBin.h,v 1.4.2.1 1997/06/07 09:51:01 mhw Exp $
 */

#ifndef PGPPRSBIN_H
#define PGPPRSBIN_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

/*
 * Create a binary parser module. This module will read in data,
 * figure out what kind of PGP message it is, and create the
 * appropriate sub-modules to actually decode the message, depending
 * on the user's wishes.
 *
 * It communicated with the Annotation Reader, which is a part of the
 * User Interface, to explain exactly what is going on with the
 * decoding process. This module, once the pipeline is put into
 * action, will add and delete modules downstream.
 */
struct PgpPipeline PGPExport **pgpParseBinCreate (struct PgpPipeline **head,
						struct PgpEnv const *env);

#ifdef __cplusplus
}
#endif

#endif /* PGPPRSBIN_H */
