/*
* pgpBufMod.h -- A Buffering Module; buffer until the first sizeAdvise
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpBufMod.h,v 1.4.2.1 1997/06/07 09:51:14 mhw Exp $
*/

#ifndef PGPBUFMOD_H
#define PGPBUFMOD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpFifoDesc;

struct PgpPipeline PGPExport **
pgpBufferModCreate (struct PgpPipeline **head, struct PgpFifoDesc const *fd);

#ifdef __cplusplus
}
#endif

#endif /* PGPBUFMOD_H */
