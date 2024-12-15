/*
 * pgpPrsAsc.h -- An Ascii Armor Parser for PGP.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsAsc.h,v 1.4.2.1 1997/06/07 09:50:57 mhw Exp $
 */

#ifndef PGPPRSASC_H
#define PGPPRSASC_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Pipeline;
#ifndef TYPE_PIPELINE
#define TYPE_PIPELINE 1
typedef struct Pipeline Pipeline;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

struct PgpUICb;
#ifndef TYPE_PGPUICB
#define TYPE_PGPUICB 1
typedef struct PgpUICb PgpUICb;
#endif

struct PgpPipeline PGPExport **pgpParseAscCreate (struct PgpPipeline **head,
			 						struct PgpEnv const *env,
			 						struct PgpFifoDesc const *fd,
			 						struct PgpUICb const *ui,
			 						void *ui_arg);

#ifdef __cplusplus
}
#endif

#endif /* PGPPRSASC_H */
