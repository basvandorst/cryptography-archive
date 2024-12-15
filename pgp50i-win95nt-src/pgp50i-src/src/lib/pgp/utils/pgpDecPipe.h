/*
 * pgpDecPipe.h -- Create a Decryption Pipeline
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpDecPipe.h,v 1.4.2.1 1997/06/07 09:51:39 mhw Exp $
 */

#ifndef PGPDECPIPE_H
#define PGPDECPIPE_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
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

struct PgpPipeline PGPExport **
pgpDecryptPipelineCreate (struct PgpPipeline **head,
			 		struct PgpEnv *env,
			 		struct PgpFifoDesc const *fd,
			 		struct PgpUICb const *ui,
			 		void *ui_arg);

#ifdef __cplusplus
}
#endif

#endif /* PGPDECPIPE_H */