/*
 * pgpInfMod.c -- Inflation (decompression) module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpInfMod.c,v 1.18 1999/05/03 21:27:56 heller Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpInfMod.h"
#include "pgpZInflate.h"
#include "pgpAnnotate.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define INFMODMAGIC	0x01258aef

typedef struct InfModContext {
	PGPPipeline		pipe;
	
	InflateContext *inf;
	PGPPipeline *tail;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( InfModContext )
} InfModContext;

static PGPError
DoFlush (InfModContext *context)
{
	PGPError	error = kPGPError_NoErr;
	unsigned len;
	PGPByte const *ptr;
	size_t retlen;

	/* Try to flush anything that we have buffered */
	ptr = infGetBytes (context->inf, &len);
	while (len) {
		retlen = context->tail->write (context->tail,
					       ptr, len,
					       &error);
		infSkipBytes (context->inf, retlen);
		
		if (error)
			return error;

		ptr = infGetBytes (context->inf, &len);
	}
	return error;
}

static PGPError
Flush (PGPPipeline *myself)
{
	InfModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == INFMODMAGIC);

	context = (InfModContext *)myself->priv;
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
	InfModContext *context;
	size_t written, retlen = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == INFMODMAGIC);
	pgpAssert (error);

	context = (InfModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	/* Flush what is buffered */
	*error = DoFlush (context);
	if (*error)
		return retlen;

	do {
		/* Process data */
		written = infWrite (context->inf, (PGPByte *)buf, size, error);
		retlen += written;
		buf += written;
		size -= written;
		if (*error) {
			*error = kPGPError_DeCompressionFailed;
			return retlen;
		}

		/* And flush it! */
		*error = DoFlush (context);
		if (*error)
			return retlen;

	} while (size);
	/* Continue until we have left to read */

	return retlen;
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	InfModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == INFMODMAGIC);

	context = (InfModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	error = context->tail->annotate (context->tail, origin, type,
					string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert (context->scope_depth != -1);
	return error;
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	InfModContext *context;
	PGPError eoferr;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == INFMODMAGIC);

	context = (InfModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (!bytes && !context->scope_depth) {
		do {
			eoferr = (PGPError)infEOF (context->inf);
			if (eoferr < 0)
				return kPGPError_DeCompressionFailed;

			error = DoFlush (context);
			if (error)
				return error;
		} while (eoferr);
	}

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	InfModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == INFMODMAGIC);

	context = (InfModContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	infFree (context->inf);
	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
infModCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head)
{
	PGPPipeline *mod;
	InfModContext *context;

	if (!head)
		return NULL;

	context = (InfModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = INFMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Inflation Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->inf = infAlloc ( cdkContext );

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
