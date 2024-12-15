/*
* pgpMemMod.h -- Module to output to a fixed-size memory buffer
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMemMod.h,v 1.2.2.1 1997/06/07 09:51:17 mhw Exp $
*/

#ifndef PGPMEMMOD_H
#define PGPMEMMOD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpPipeline PGPExport *
pgpMemModCreate (struct PgpPipeline **head, char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* PGPMEMMOD_H */
