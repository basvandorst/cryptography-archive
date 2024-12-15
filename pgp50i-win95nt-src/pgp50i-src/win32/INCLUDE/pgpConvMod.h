/*
* pgpConvMod.h -- Conventional Encryption Module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written By:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpConvMod.h,v 1.4.2.1 1997/06/07 09:50:48 mhw Exp $
*/

#ifndef PGPCONVMOD_H
#define PGPCONVMOD_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

struct PgpConvKey;
#ifndef TYPE_PGPCONVKEY
#define TYPE_PGPCONVKEY 1
typedef struct PgpConvKey PgpConvKey;
#endif

struct PgpStringToKey;
#ifndef TYPE_PGPSTRINGTOKEY
#define TYPE_PGPSTRINGTOKEY 1
typedef struct PgpStringToKey PgpStringToKey;
#endif

struct PgpPipeline PGPExport **
pgpConvModCreate (struct PgpPipeline **head, PgpVersion version,
		struct PgpFifoDesc const *fd,
		struct PgpStringToKey const *s2k,
		struct PgpConvKey const *convkeys,
		byte cipher, byte const *session);

#ifdef __cplusplus
}
#endif

#endif /* PGPCONVMOD_H */
