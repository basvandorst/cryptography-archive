/*
* pgpCompMod.h -- this defines the compression and decompression
* functions for PGP.. It provides a generalized interface to the
* compression modules, and keeps applications from requiring
* knowledge of all the different types of compression that may be
* supported.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCompMod.h,v 1.4.2.1 1997/06/07 09:51:07 mhw Exp $
*/

#ifndef PGPCOMPMOD_H
#define PGPCOMPMOD_H

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

/*
* Create a compression module of the appropriate type (must be one of
* the COMPRESSALG_* types) with the appropriate compression quality.
*
* Note: There can only be one ZIP compression module in existance
* at a time, since the underlying ZIP code is not re-entrant.
*/
struct PgpPipeline PGPExport **pgpCompressModCreate (
 struct PgpPipeline **head, PgpVersion version,
 struct PgpFifoDesc const *fd, byte type, int quality);

/*
* Create a decompression module of the appropriate type.
*/
struct PgpPipeline PGPExport **pgpDecompressModCreate (
 struct PgpPipeline **head, byte type);

#ifdef __cplusplus
}
#endif

#endif /* PGPCOMPMOD_H */
