/*
 * $Id: pgpSplit.c,v 1.19 1997/12/12 01:14:18 heller Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpSplit.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPSplitBufSize	16384
#else
#define kPGPSplitBufSize	BUFSIZ
#endif

#define SPLITMAGIC	0x11223344

#define MAXTAILS	2

typedef struct SplitContext
{
	PGPPipeline		pipe;
	
	PGPByte			buffer[kPGPSplitBufSize];
	PGPByte const *	bufptr;
	PGPByte const *	bufend;
	int				curtail;
	int				numtails;
	PGPPipeline *	tail[MAXTAILS];
	int				scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( SplitContext )
} SplitContext;

/*
 * If we got an error while writing, the pending write data gets copied to
 * the context's buffer for re-transmission later.  (Yes, this is needed to
 * preserve the semantics of write().)
 */
static PGPError
DoFlush(SplitContext *context)
{
	PGPPipeline *tailp;
	PGPError	error;
	size_t len;

	/* Try to flush anything that we have buffered */
	len = context->bufend - context->bufptr;
	while (context->curtail != context->numtails) {
		tailp = context->tail[context->curtail];
		while (len != 0) {
			len = tailp->write(tailp, context->bufptr,
					   len, &error);
			context->bufptr += len;
			if (error)
				return error;
		}
		context->bufptr = context->buffer;
		len = context->bufend - context->bufptr;
		context->curtail++;
	}
	pgpClearMemory(context->buffer, len);
	context->bufptr = context->bufend = context->buffer;

	return kPGPError_NoErr;
}

static PGPError
Flush(PGPPipeline *myself)
{
	PGPPipeline *tailp;
	SplitContext *context;
	PGPError	error;
	int			i;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);

	context = (SplitContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	error = DoFlush(context);
	if (error)
		return error;

	for (i = 0; i < context->numtails; i++) {
		tailp = context->tail[i];
		error = tailp->flush(tailp);
		if (error)
			return error;
	}

	return kPGPError_NoErr;
}

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	PGPPipeline *tailp;
	SplitContext *context;
	size_t block, pending, myselfwrite;
	PGPByte const *tmpbuf;
	size_t written = 0;
	int i;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);
	pgpAssert(error);

	context = (SplitContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	/* Write out anything still hanging around from last time */
	*error = DoFlush(context);
	if (*error)
		return written;

	/* Okay, now write out the new stuff. */
	while (size) {
		/* Never write more in one block than we can buffer */
		block = pgpMin(size, sizeof(context->buffer));

		/* Now write out the current block to each of the tails. */
		for (i = 0; i < context->numtails; i++) {
			tailp = context->tail[i];	/* Current tail */
			tmpbuf = buf;		/* Current write pointer */
			pending = block;	/* Current size */
			do {
				myselfwrite = tailp->write(tailp, tmpbuf,
							   pending, error);
				tmpbuf += myselfwrite;
				pending -= myselfwrite;
				if (!*error)
					continue;
				/* -- Error handling -- */
				/* Remember how far we've gotten */
				context->curtail = i;
				/*
				 * If we get an error while writing to the
				 * first tail, we don't need to buffer the
				 * bytes after the current write pointer,
				 * since we've never done anything with them,
				 * so we aren't responsible for them.
				 */
				if (i == 0)
					block -= pending;
				/*
				 * If we get an error while writing to the
				 * last tail, we don't need to remember the
				 * bytes we've managed to write to every
				 * tail; only the following bytes, which
				 * we've accepted responsibility for by
				 * writing out somewhere else.
				 */
				if (i == context->numtails-1) {
					buf = tmpbuf;
					written += block-pending;
					block = pending;
				}
				/* Copy data to buffer for later DoFlush() */
				memcpy(context->buffer, buf, block);
				context->bufend = context->buffer+block;
				context->bufptr = context->bufend-pending;
				/* And return the amount written plus buffer */
				return written + block;
			} while (pending);
		}
		/* Advance pointers and go on... */
		buf += block;
		size -= block;
		written += block;
	}

	return written;	
}

/*
 * Annotate is a bit magic.  The problem is that annotations are
 * defined, as a class, to atomically succeed or fail.  One particular
 * annotation might play fast & loose with this, but we aren't allowed
 * to break it for *all* annotations.
 *
 * But if we pass an annotation down the first tail, and it succeeds
 * (no error returned), then pass it down the second, and get an error,
 * things are a bit screwed up, because lots of modules saw the annotation
 * succeed and altered their state accordingly.
 *
 * The solution seems to be to pass annotations down *one tail only*,
 * which we use the first tail for.  Other tails see data, but no
 * annotations.
 */
static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	SplitContext *context;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);

	context = (SplitContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	error = DoFlush(context);
	if (error)
		return error;

	error = context->tail[0]->annotate(context->tail[0], origin, type,
					   string, size);
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

/*
 * Note that because of the way we do annotations, if a SizeAdvise
 * arrives while we're inside a scope, we can't pass it down to any but
 * the first tail; it is meaningless to the others.  So we bail out
 * of the loop early in that case.
 */
static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long bytes)
{
	SplitContext *context;
	PGPError	error;
	int i;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);

	context = (SplitContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	error = DoFlush(context);
	if (error)
		return error;

	for (i = 0; i < context->numtails; i++)  {
		error = context->tail[i]->sizeAdvise(context->tail[i], bytes);
		if (error || context->scope_depth)
			return error;
	}
	return kPGPError_NoErr;
}

static PGPError
Teardown(PGPPipeline *myself)
{
	SplitContext *	myContext;
	int				i;
	PGPContextRef	cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);

	myContext = (SplitContext *)myself->priv;
	pgpAssert(myself);
	cdkContext	= myself->cdkContext;

	for (i = 0; i < myContext->numtails; i++)
	{
		if ( IsntNull( myContext->tail[i] ) )
			myContext->tail[i]->teardown(myContext->tail[i]);
	}

	pgpClearMemory( myContext,  sizeof(*myContext));
	pgpContextMemFree( cdkContext, myContext);
	
	return kPGPError_NoErr;
}

	PGPPipeline **
pgpSplitCreate(
	PGPContextRef	cdkContext,
	PGPPipeline **	head)
{
	PGPPipeline *	mod;
	SplitContext *	myContext;

	if (!head)
		return NULL;

	myContext = (SplitContext *)pgpContextMemAlloc( cdkContext,
		sizeof(*myContext), kPGPMemoryMgrFlags_Clear );
	if ( IsNull( myContext ) )
		return NULL;
		
	mod = &myContext->pipe;

	mod->magic		= SPLITMAGIC;
	mod->write		= Write;
	mod->flush		= Flush;
	mod->sizeAdvise	= SizeAdvise;
	mod->annotate	= Annotate;
	mod->teardown	= Teardown;
	mod->name		= "Split module";
	mod->priv 		= myContext;
	mod->cdkContext	= cdkContext;

	myContext->bufptr	= myContext->buffer;
	myContext->curtail 	= myContext->numtails = 1;
	myContext->bufptr	= myContext->bufend = myContext->buffer;

	myContext->tail[0] = *head;
	*head = mod;
	return &myContext->tail[0];
}

PGPPipeline **
pgpSplitAdd(PGPPipeline *myself)
{
	SplitContext *context;
	PGPPipeline **tailp;

	pgpAssert(myself);
	pgpAssert(myself->magic == SPLITMAGIC);

	context = (SplitContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	if (context->numtails == MAXTAILS)
		return 0;
	if (context->curtail != context->numtails)
		return 0;	/* We're in the middle of things! */

	tailp =  &context->tail[context->numtails++];
	context->curtail = context->numtails;
	return tailp;
}
