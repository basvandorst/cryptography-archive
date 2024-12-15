/*
 * pgpMoreMod.h -- Interface-specific pipeline module to
 * display files on console
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMoreMod.h,v 1.3.2.1 1997/06/07 09:52:01 mhw Exp $
 */

#ifndef PGPMOREMOD_H
#define PGPMOREMOD_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

/*
 * Create a module to write to the console, pausing after each page of
 * output.
 */
struct PgpPipeline *pgpMoreModCreate (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif

#endif /* PGPMOREMOD_H */
