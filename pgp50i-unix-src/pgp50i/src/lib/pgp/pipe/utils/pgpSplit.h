/*
* pgpSplit.h -- Split module -- one input, multiple outputs.
* All outputs get data, but ONLY THE ORIGINAL OUTPUT
* GETS ANNOTATIONS. (Error-handling is impossible
* otherwise.)
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpSplit.h,v 1.3.2.1 1997/06/07 09:51:19 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpPipeline PGPExport **pgpSplitCreate (struct PgpPipeline **head);
struct PgpPipeline PGPExport **pgpSplitAdd (struct PgpPipeline *myself);

#ifdef __cplusplus
}
#endif
