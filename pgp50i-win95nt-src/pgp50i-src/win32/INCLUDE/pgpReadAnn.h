/*
* pgpReadAnn.h -- A debugging Annotation Reader.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpReadAnn.h,v 1.4.2.1 1997/06/07 09:51:01 mhw Exp $
*/

#ifndef PGPREADANN_H
#define PGPREADANN_H

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

/*
* Create an annotation reader. This will create output files as
* appropriate. If filename is non-NULL, it will use that filename
* as the output file. Otherwise it will ask the user or use the name
* out of the message.
*/
struct PgpPipeline PGPExport **
pgpAnnotationReaderCreate (struct PgpPipeline **head,
	  	struct PgpEnv const *env,
	  	struct PgpUICb const *ui, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* PGPREADANN_H */
