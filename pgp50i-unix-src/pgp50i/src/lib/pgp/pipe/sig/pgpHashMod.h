/*
* pgpHashMod.h -- A Generic Hash Module.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpHashMod.h,v 1.4.2.1 1997/06/07 09:51:04 mhw Exp $
*/
#ifndef PGPHASHMOD_H
#define PGPHASHMOD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpHashContext;
#ifndef TYPE_PGPHASHCONTEXT
#define TYPE_PGPHASHCONTEXT 1
typedef struct PgpHashContext PgpHashContext;
#endif

/*
* Create a module that will hash its input and copy it to the output,
* using the passed-in PgpHashContext. The caller assumes control of the
* PgpHashContext, and when this module returns success from a
* sizeAdvise(0), the PgpHashContext is valid for use.
*
* Note: the calling function must destroy the PgpHashContext.
*/
struct PgpPipeline PGPExport **
pgpHashModCreate (struct PgpPipeline **head, struct PgpHashContext *hash);

struct PgpPipeline PGPExport **
pgpHashModListCreate (struct PgpPipeline **head, struct PgpHashContext *hashes,
 unsigned num);

#ifdef __cplusplus
}
#endif

#endif
