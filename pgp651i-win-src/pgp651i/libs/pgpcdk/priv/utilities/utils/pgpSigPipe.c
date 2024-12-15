/*
 * pgpSigPipe.c -- Create a Signature Verification Pipeline
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigPipe.c,v 1.6 1997/06/25 19:42:04 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>

#include "pgpSigPipe.h"
#include "pgpFIFO.h"
#include "pgpPrsAsc.h"
#include "pgpVerifyRa.h"

PGPPipeline *
pgpSignatureVerifyCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	texthead,
	PGPPipeline **	sighead,
	PGPEnv const *	env,
	PGPFifoDesc const *fd,
	PGPByte const *hashlist, unsigned hashlen,
	int textmode,
	PGPUICb const *ui, void *ui_arg)
{
	if (!texthead || !sighead)
		return NULL;

	if (!fd)
		fd = &pgpByteFifoDesc;

	sighead = pgpParseAscCreate ( cdkContext, sighead, env, fd, ui, ui_arg);
	if (!sighead)
		return NULL;

	return pgpVerifyReaderCreate ( cdkContext, texthead, sighead, env, fd,
				      hashlist, hashlen, textmode, ui, ui_arg);
}

