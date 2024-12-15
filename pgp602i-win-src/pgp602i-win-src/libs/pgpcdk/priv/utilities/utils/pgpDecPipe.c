/*
 * pgpDecPipe.c -- setup the Decryption Pipeline, given the arguments
 * of the UI.  This is just a helper function to setup the decryption
 * pipeline.  It would be just as easy for an application to do it.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpDecPipe.c,v 1.5 1997/06/25 19:41:44 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDecPipe.h"
#include "pgpFIFO.h"
#include "pgpPrsAsc.h"
#include "pgpReadAnn.h"

PGPPipeline **
pgpDecryptPipelineCreate (
	PGPContextRef		cdkContext,
	PGPPipeline **		head,
	PGPEnv *			env,
	PGPFifoDesc const *	fd,
	PGPUICb const *		ui,
	void *				ui_arg)
{
	if (!head)
		return NULL;

	if (!fd)
		fd = &pgpByteFifoDesc;

	head = pgpParseAscCreate ( cdkContext, head, env, fd, ui, ui_arg);
	if (!head)
		return NULL;

	return pgpAnnotationReaderCreate ( cdkContext, head, env, ui, ui_arg);
}