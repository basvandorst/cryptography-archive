/*
* pgpDevNull.h -- A pipeline going nowhere
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpDevNull.h,v 1.4.2.1 1997/06/07 09:51:16 mhw Exp $
*/

#ifndef PGPDEVNULL_H
#define PGPDEVNULL_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

/*
* Create a module that will eat all of its input.
*/
struct PgpPipeline PGPExport *pgpDevNullCreate (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif

#endif /* PGPDEVNULL_H */
