/*
* pgpLiteral.h -- Create a Literal Packet Module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpLiteral.h,v 1.4.2.1 1997/06/07 09:51:09 mhw Exp $
*/

#ifndef PGPLITERAL_H
#define PGPLITERAL_H

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
* This creates a module for a literal packet. The type should be
* either 'b' or 't' for binary or text. The name is len bytes which
* specifies the name of the file. The timestamp is the timestamp of
* the file.
*/
struct PgpPipeline PGPExport **
pgpLiteralCreate (struct PgpPipeline **head, PgpVersion version,
  struct PgpFifoDesc const *fd, byte type, byte *name,
  byte len, word32 timestamp);

#ifdef __cplusplus
}
#endif

#endif /* PGPLITERAL_H */
