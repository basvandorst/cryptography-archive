/*
 * pgpProtoMod.c -- a prototype module for PGP
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpProtoMod.c,v 1.8 1997/07/26 19:37:32 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

/*
 * This is a magic number for use within this module.  It is used to
 * make sure the passed-in module is a member of this type.  Just a
 * little extra sanity-checking.
 *
 * You should set this value to some "random" 32-bit number.
 */
 
#define MAGIC	0x0

Context {
	PGPByte buffer[BUFSIZ];
	PGPByte *bufptr;
	size_t buflen;
	PGPPipeline *tail;
	int scope_depth;
};

static int
DoFlush (Context *context)
{
	PGPError	error = kPGPError_NoErr;
	size_t retlen;

	/* Try to flush anything that we have buffered */
	while (context->buflen) {
		retlen = context->tail->write (context->tail,
					       context->bufptr,
					       context->buflen,
					       &error);
		context->buflen -= retlen;
		memset (context->bufptr, 0, retlen);
		context->bufptr += retlen;
		if (error)
			return error;
	}
	return error;
}

static int
Flush (PGPPipeline *myself)
{
	Context *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);

	context = myself->priv;
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
	Context *context;
	size_t written = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);
	pgpAssert (error);

	context = myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	do {
		*error = DoFlush (context);
		if (*error)
			return written;

		/*
		 * Now that we don't have anything buffered, bring in more
		 * data from the passed-in buffer, process it, and buffer
		 * that to write out.
		 */
		context->bufptr = context->buffer;
		/* XXX Set context->buflen to length of read-in material */
		/* XXX Read/processed data into context->buffer */
		buf += context->buflen;
		size -= context->buflen;
		written += context->buflen;

	} while (context->buflen > 0);
	/* Continue until we have nothing buffered */

	return written;	
}

static int
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	Context *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);

	context = myself->priv;
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

static int
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	Context *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);

	context = myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->sizeAdvise (context->tail, bytes)
}

static void
Teardown (PGPPipeline *myself)
{
	Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);

	context = myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	memset (context, 0, sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	memset (myself, 0, sizeof (*myself));
	pgpContextMemFree( cdkContext, myself);
}

PGPPipeline **
Create (PGPPipeline **head)
{
	PGPPipeline *mod;
	Context *context;

	if (!head)
		return NULL;

	context = pgpContextMemAlloc( cdkContext, sizeof (*context));
	if (!context)
		return NULL;
	mod = pgpContextMemAlloc( cdkContext, sizeof (*mod));
	if (!mod) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}

	mod->magic = MAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Prototype";  /* XXX Set myself to the module name! */
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->bufptr = context->buffer;

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
