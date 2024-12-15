/*
* pgpCopyMod.h -- Copy the input directly to the output.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCopyMod.h,v 1.3.2.1 1997/06/07 09:51:15 mhw Exp $
*/

#ifndef PGPCOPYMOD_H
#define PGPCOPYMOD_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpPipeline **pgpCopyModCreate (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif

#endif /* PGPCOPYMOD_H */
