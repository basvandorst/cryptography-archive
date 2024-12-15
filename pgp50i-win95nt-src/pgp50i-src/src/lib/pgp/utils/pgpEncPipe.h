/*
 * pgpEncPipe.h -- setup the Encryption Pipeline, given a set of arguments
 * from the UI. This will setup a pipeline, or return NULL if there is
 * any error along the way.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpEncPipe.h,v 1.5.2.1 1997/06/07 09:51:40 mhw Exp $
 */

#ifndef PGPENCPIPE_H
#define PGPENCPIPE_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpConvKey;
#ifndef TYPE_PGPCONVKEY
#define TYPE_PGPCONVKEY 1
typedef struct PgpConvKey PgpConvKey;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpSigSpec;
#ifndef TYPE_PGPSIGSPEC
#define TYPE_PGPSIGSPEC 1
typedef struct PgpSigSpec PgpSigSpec;
#endif

struct PgpPubKey;
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

struct PgpLiteralParams {
	char const *filename;
	word32	timestamp;
};
#ifndef TYPE_PGPLITERALPARAMS
#define TYPE_PGPLITERALPARAMS 1
typedef struct PgpLiteralParams PgpLiteralParams;
#endif

/*
 * Create the encryption pipeline. This function will use parameters
 * set in the environment and parameters passed in to determine which
 * filters to create. This will perform some limited sanity checking
 * on the arguments. For example, it is illegal to create a separate
 * signature while encrypting.
 *
 * Pass in a pointer to where the head of the pipeline should go.
 * This function will return you pointer to the tail of the pipeline.
 *
 * If you pass in needoutput, it will be filled in with 0 or 1, depending
 * on if the pipeline requires an output module. When using multipart
 * armor, for example, no output file is required -- the armor module will
 * open the appropriate files.
 *
 * Arguments:
 *		head: The head of the pipeline created.
 *		env: The current PGP Environment
 *		fd: The Fifo Descriptor to use for buffering
 *		rng: The Random Number Generator to use for padding and randomness
 *		convkeys: The list of Conventional Keys to use for conventional
 *			encryption (encrypting with a passphrase)
 *		pubkeys: The list of Public Keys to use for public key encryption.
 *		sigspecs: The list of Signature Specifications to use for making
 *			signatures.
 *		literal: should this be processed as a literal packet, or is it
 *			a PGP message or a clearsigned message, or a separate
 *			signature? This includes the input filename to be included
 *			in the literal packet, and other literal information.
 *		sepsig: perform a separate signature. Only valid with a few other
 *				options. If the special value PGP_SEPSIGMSG is passed,
 *				separate signature will say "BEGIN PGP MESSAGE" instead of
 *				"BEGIN PGP SIGNATURE", so that it can be used to form a PGP/MIME
 *				clear-signed message. XXX: This is a hack until PGP/MIME is
 *				properly implemented in the library.
 */
struct PgpPipeline PGPExport **
pgpEncryptPipelineCreate (struct PgpPipeline **head,
			struct PgpEnv const *env,
			struct PgpFifoDesc const *fd,
			struct PgpRandomContext const *rng,
			struct PgpConvKey *convkeys,
			struct PgpPubKey *pubkeys,
			struct PgpSigSpec *sigspecs,
			struct PgpLiteralParams *literal,
			int sepsig);

#define PGP_SEPSIGMSG	7361

#ifdef __cplusplus
}
#endif

#endif /* PGPENCPIPE_H */
