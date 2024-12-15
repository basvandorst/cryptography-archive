/*
 * pgpDefMod.c -- Deflate (compression) module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpDefMod.c,v 1.19 1997/12/12 01:14:08 heller Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpByteFIFO.h"
#include "pgpDefMod.h"
#include "pgpZip.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define DEFMODMAGIC	0x0def1a1e

typedef struct DefModContext {
	PGPPipeline	pipe;
	
	PGPFifoContext *fifo;
	PGPPipeline *tail;
	struct ZDeflateContext *zdcontext;
	struct ZTreesContext *ztcontext;
	struct ZBitsContext *zbcontext;
	int finished;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( DefModContext )
} DefModContext;

static PGPError
DoFlush (DefModContext *context)
{
	PGPError	error = kPGPError_NoErr;
	PGPByte const *ptr;
	PGPSize len;
	size_t retlen;

	/* Try to flush anything that we have buffered */
	bi_flush(context->zbcontext);
	ptr = byteFifoPeek (context->fifo, &len);
	while (len) {
		retlen = context->tail->write (context->tail, ptr, len,
					       &error);
		byteFifoSeek (context->fifo, retlen);
		if (error)
			return error;
		ptr = byteFifoPeek (context->fifo, &len);
	}
	return error;
}

static PGPError
Flush (PGPPipeline *myself)
{
	DefModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEFMODMAGIC);

	context = (DefModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	DefModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEFMODMAGIC);
	pgpAssert (error);

	context = (DefModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	zip_input (context->zdcontext, context->ztcontext, context->zbcontext,
			   (char const *)buf, size);
	*error = DoFlush (context);

	return size;
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	DefModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEFMODMAGIC);

	context = (DefModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	error = context->tail->annotate (context->tail, origin, type,
					string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	DefModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEFMODMAGIC);

	context = (DefModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (bytes || context->scope_depth)
		return( kPGPError_NoErr );

	if (!context->finished) {
		/* It is a Bad Thing to call zip_finish multiple times. */
		zip_finish (context->zdcontext, context->ztcontext,
					context->zbcontext);
		context->finished = 1;
	}
	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->sizeAdvise (context->tail, 0);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	DefModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == DEFMODMAGIC);

	context = (DefModContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	bi_free (context->zbcontext);		/* Free bits data structures */
	ct_free (context->ztcontext);		/* Free code tree buffers */
	lm_free (context->zdcontext);		/* Free longest match buffers */
	byteFifoDestroy (context->fifo);
	
	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);

	return kPGPError_NoErr;
}

PGPPipeline **
defModCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head, int quality)
{
	PGPPipeline *mod;
	DefModContext *context;
	PGPFifoContext *fifo;
	struct ZDeflateContext *zdcontext;
	struct ZTreesContext *ztcontext;

	if (!head)
		return NULL;

	if (IsNull( zdcontext = lm_init (cdkContext, quality) ) )
		return NULL;
	if (IsNull( ztcontext = ct_init (cdkContext) ) ) {
		lm_free (zdcontext);
		return NULL;
	}
	context = (DefModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
	if (!context) {
		ct_free (ztcontext);
		lm_free (zdcontext);
		return NULL;
	}
	mod =  &context->pipe;
	
	fifo = byteFifoCreate (cdkContext);
	if (!fifo) {
		ct_free (ztcontext);
		lm_free (zdcontext);
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	
	mod->magic = DEFMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Deflation Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->fifo = fifo;
	context->zdcontext = zdcontext;
	context->ztcontext = ztcontext;
	context->zbcontext = bi_init (cdkContext, fifo);

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
