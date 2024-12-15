/*
 * pgpEncPipe.h -- setup the Encryption Pipeline, given a set of arguments
 * from the UI.  This will setup a pipeline, or return NULL if there is
 * any error along the way.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpEncPipe.h,v 1.13 1997/08/16 22:05:25 hal Exp $
 */

#ifndef Included_pgpEncPipe_h
#define Included_pgpEncPipe_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS



typedef struct PgpLiteralParams
{
	char const *filename;
	PGPUInt32	timestamp;
} PgpLiteralParams ;

/*
 * Create the encryption pipeline.  This function will use parameters
 * set in the environment and parameters passed in to determine which
 * filters to create.  This will perform some limited sanity checking
 * on the arguments.  For example, it is illegal to create a separate
 * signature while encrypting.
 *
 * Pass in a pointer to where the head of the pipeline should go.
 * This function will return you pointer to the tail of the pipeline.
 *
 * If you pass in needoutput, it will be filled in with 0 or 1, depending
 * on if the pipeline requires an output module.  When using multipart
 * armor, for example, no output file is required -- the armor module will
 * open the appropriate files.
 *
 * Arguments:
 *	head: The head of the pipeline created.
 *	env: The current PGP Environment
 *	fd: The Fifo Descriptor to use for buffering
 *	rng: The Random Number Generator to use for padding and randomness
 *	convkeys: The list of Conventional Keys to use for conventional
 *		encryption (encrypting with a passphrase)
 *	pubkeys: The list of Public Keys to use for public key encryption.
 *	sigspecs: The list of Signature Specifications to use for making
 *		signatures.
 *	literal: should this be processed as a literal packet, or is it
 *		a PGP message or a clearsigned message, or a separate
 *		signature?  This includes the input filename to be included
 *		in the literal packet, and other literal information.
 *	sepsig: perform a separate signature.  Only valid with a few other
 *		options.  If the special value PGP_SEPSIGMSG is passed, 
 *		separate signature will say "BEGIN PGP MESSAGE" instead of
 *		"BEGIN PGP SIGNATURE", so that it can be used to form a PGP/MIME
 *		clear-signed message.  XXX: This is a hack until PGP/MIME is
 *		properly implemented in the library.
 */
PGPPipeline  **
pgpEncryptPipelineCreate (
				PGPContextRef cdkContext,
				PGPPipeline **head,
				PGPEnv const *env,
				PGPFifoDesc const *fd,
				PGPRandomContext const *rng,
				PGPConvKey *convkeys,
				PGPPubKey *pubkeys,
				PGPSigSpec *sigspecs,
				PgpLiteralParams *literal,
				int sepsig,
				int (*progress)(void *arg, int c),
				void *arg,
				PGPError *error);

#define PGP_SEPSIGMSG	7361

PGP_END_C_DECLARATIONS

#endif /* Included_pgpEncPipe_h */
