/*
* pgpVerifyRa.h -- Signature Verification Annotation Reader
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpVerifyRa.h,v 1.4.2.1 1997/06/07 09:51:02 mhw Exp $
*/
#ifndef PGPVERIFYRA_H
#define PGPVERIFYRA_H

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

struct PgpUICb;
#ifndef TYPE_PGPUICB
#define TYPE_PGPUICB 1
typedef struct PgpUICb PgpUICb;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

struct PgpPipeline PGPExport *
pgpVerifyReaderCreate (struct PgpPipeline **texthead,
						struct PgpPipeline **sighead,
						struct PgpEnv const *env,
						struct PgpFifoDesc const *fd,
						byte const *hashlist,
						unsigned hashlen,
						int textmode,
						struct PgpUICb const *ui,
						void *ui_arg);

#ifdef __cplusplus
}
#endif

#endif /* PGPVERIFYRA_H */
