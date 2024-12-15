/*
* pgpSigMod.h -- A module that is used to make a signed message.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a Public API Function Header.
*
* $Id: pgpSigMod.h,v 1.3.2.1 1997/06/07 09:51:05 mhw Exp $
*/

#ifndef PGPSIGMOD_H
#define PGPSIGMOD_H

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

struct PgpSigSpec;
#ifndef TYPE_PGPSIGSPEC
#define TYPE_PGPSIGSPEC 1
typedef struct PgpSigSpec PgpSigSpec;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

/*
* This creates a signing module. Initially, it just passes through its
* input data unmodified.
*/
struct PgpPipeline PGPExport **pgpSigCreate (struct PgpPipeline **head,
    PgpVersion version,
    struct PgpFifoDesc const *fd,
    struct PgpRandomContext const *rc);
/*
* This adds a signature to the signing module. It returns a tail pointer
* which the signatures will come out of. Each call returns the same
* tail pointer - this joins them together internally. If clearsig is
* true it won't try to output the special one-pass signature.
*/
struct PgpPipeline PGPExport **pgpSigSignature (struct PgpPipeline *pipe,
    struct PgpSigSpec *spec, byte clearsig);
/*
* This inserts some text into the signature output in the appropriate place
* to make a signed message, as opposed to separate signatures.
* It is possible that you might take the output of the signature from
* pgpSigCreate, apply some processing (such as compression, or encapsulation
* in a literal which PGP does right now) and feed it back into here.
* (Should we hide *that* processing, too?)
*/
struct PgpPipeline PGPExport **pgpSigTextInsert (struct PgpPipeline *pipe,
	   	struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif

#endif /* PGPSIGMOD_H */
