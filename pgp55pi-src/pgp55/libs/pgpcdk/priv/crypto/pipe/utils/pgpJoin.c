/*
 * pgpJoin.c -- join multiple modules into a single stream
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpJoin.c,v 1.21 1997/09/18 01:35:29 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpJoin.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define JOINMAGIC	0x00112233

typedef struct JoinContext {
	PGPPipeline		pipe;
	
	PGPFifoDesc const *fd;
	PGPFifoContext *fifo;
	PGPPipeline *tail;
	PGPPipeline *next;
	PGPPipeline *top;
	unsigned long size;
	int sizevalid;
	int canwrite;
	int eof;
	int teardown;
	DEBUG_STRUCT_CONSTRUCTOR( JoinContext )
} JoinContext;

static PGPError
DoFlush(JoinContext *context)
{
	PGPError	error = kPGPError_NoErr;
	PGPByte const *ptr;
	PGPSize len;
	size_t retlen;
	JoinContext *topp = (JoinContext *)context->top->priv;
	PGPPipeline *tailp = topp->tail;

	pgpAssert(tailp);

	/* Try to flush anything that we have buffered */
	if (context->fifo) {
		ptr = pgpFifoPeek (context->fd, context->fifo, &len);
		while (len) {
			retlen = tailp->write(tailp, ptr, len, &error);
			pgpFifoSeek (context->fd, context->fifo, retlen);
			if (context->sizevalid)
				context->size -= retlen;
			if (error)
				return error;
			
			ptr = pgpFifoPeek (context->fd, context->fifo, &len);
		}
	}
	/* If we get here, we have nothing left buffered in the fifo */

	/*
	 * Now, check if we are in an EOF condition.  If so, then we
	 * can tell the next input that it is allowed to write, and
	 * then we can try to flush out that buffer as well.
	 */
	if (context->eof && context->next) {
		JoinContext *ctx = (JoinContext *)context->next->priv;

		ctx->canwrite = 1;
		error = DoFlush (ctx);
	}

	return error;
}

static PGPError
Flush(PGPPipeline *myself)
{
	JoinContext *context, *topp;
	PGPPipeline *tailp;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	topp = (JoinContext *)context->top->priv;
	pgpAssert(topp);

	tailp = topp->tail;

	if (context->canwrite) {
		pgpAssert(tailp);
		error = DoFlush (context);
		if (error)
			return error;
	}

	if (tailp)
		return tailp->flush(tailp);
	return( kPGPError_NoErr );
}

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t len, PGPError *error)
{
	JoinContext *context, *topp;
	PGPPipeline *tailp;
	size_t written;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);
	pgpAssert(error);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	topp = (JoinContext *)context->top->priv;
	pgpAssert(topp);

	/* If we're not allowed to write, then buffer things up until we can */
	*error = kPGPError_NoErr;
	if (!context->canwrite)
		return pgpFifoWrite (context->fd, context->fifo, buf, len);

	/* At this point we are writing -- make sure we have a tail! */
	tailp = topp->tail;
	pgpAssert(tailp);

	*error = DoFlush (context);
	if (*error)
		return 0;

 	written = tailp->write(tailp, buf, len, error);
	if (context->sizevalid)
		context->size -= written;

	return written;
}

static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t len)
{
	JoinContext *context, *topp;
	PGPPipeline *tailp = NULL;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	topp = (JoinContext *)context->top->priv;
	if (topp)
		tailp = topp->tail;

	/*
	 * Pass most annotations through.  Previously we did not do so,
	 * and just silently returned on the special ones below.  For minimal
	 * impact, continue with the old behavior on those but pass others
	 * through.
	 */

	switch (type) {
	case PGPANN_FILE_BEGIN:
	case PGPANN_FILE_END:
	case PGPANN_INPUT_BEGIN:
	case PGPANN_INPUT_END:
		return( kPGPError_NoErr );
		/*NOTREACHED*/
	default:
		if (tailp)
			return tailp->annotate(tailp, origin, type, string, len);
		return kPGPError_BadParams;
	}
	/*NOTREACHED*/
}

/*
 * context->size is the number of bytes in the fifo plus the number of
 * bytes we still should expect to receive.  Once this is set, we
 * expect it to be held by the caller.  Each input gets one of these.
 * We decrement it, if it is valid, whenever we write data out to the
 * tail.
 *
 * context->sizevalid is if the number in context->size is valid
 *
 * This function will first check to make sure that the size is valid,
 * and that the passed-in size matches the expected size.
 *
 * We also keep track of EOF -- if we get a sizeAdvise of 0 bytes,
 * then we set context->eof to 1, to signify that we have an EOF
 * condition and we do not expect any more bytes to be written from
 * that input.  This means that context->size is the number of bytes
 * buffered in that context.
 *
 * Then it will see if all inputs have a valid size.  If they do, then
 * it will add up all the sizes and send that as a sizeAdvise to the
 * tail.
 *
 * Next, we will flush out the buffered data, and if that returns
 * without an error we know that everything is flushed from all the
 * modules from this one to the end.
 *
 * Finally we check if everything is EOF.  We know that if everything
 * is at EOF, and we got this far, that there must not be anything in
 * any buffers, and we are not expecting anymore data to be written.
 * Therefore, we pass a sizeAdvise (0) to the tail.
 */
static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long bytes)
{
	JoinContext *ctx, *context, *topp;
	PGPPipeline *current, *tailp;
	unsigned long fifolen;
	unsigned long total;
	int flag = 0;
	PGPError	error;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	topp = (JoinContext *)context->top->priv;
	pgpAssert(topp);

	tailp = topp->tail;

	/*
	 * XXX If annotations get added, make sure we only consider calls
	 * outside them.
	 * I.e. "if (context->scope_depth) return( kPGPError_NoErr );"
	 */

	/* save off the size of the buffered data */
	fifolen = pgpFifoSize (context->fd, context->fifo);

	/* Check/set the size */
	if (context->sizevalid) {
		if (bytes != context->size - fifolen)
			/* It doesn't jibe -- this is an error */
			return kPGPError_SizeAdviseFailure;
	} else {
		context->sizevalid = 1;
		context->size = bytes + fifolen;
	}
	
	/* Set EOF, if needed */
	if (!bytes)
		context->eof = 1;

	/* Check if all modules have valid size; add up the sizes */
	total = 0;
	for (current = context->top; current; current = ctx->next) {
		ctx = (JoinContext *)current->priv;
		pgpAssert(ctx);

		if (!ctx->sizevalid) {
			flag = 1;
			break;
		}
		total += ctx->size;
	}
	if (!flag) {
		/* All the inputs have a valid sizes... Send a sizeAdvise */
		pgpAssert(tailp);

		error = tailp->sizeAdvise(tailp, total);
		if (error)
			return error;
	}

	/*
	 * If we cannot write, then we cannot flush!  Just accept that
	 * the sizeAdvise worked to this point and return success.
	 */
	if (!context->canwrite)
		return( kPGPError_NoErr );

	pgpAssert(tailp);
	/*
	 * Flush the data.  This will return an error, or everything
	 * will be flushed out of the fifo.
	 */
	error = DoFlush (context);
	if (error)
		return error;


	/* At this point, check if everything is EOF. */
	for (current = context->top; current; current = ctx->next) {
		ctx = (JoinContext *)current->priv;
		pgpAssert(ctx);
		
		if (!ctx->eof)
			/* Something isn't done -- just return success */
			return( kPGPError_NoErr );
	}

	/* Send a sizeAdvise (0) -- all the inputs are done writing. */
	return tailp->sizeAdvise(tailp, 0);
}

static PGPError
Teardown(PGPPipeline *myself)
{
	PGPPipeline *current, *tailp;
	JoinContext *ctx, *context, *topp;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	topp = (JoinContext *)context->top->priv;
	pgpAssert(topp);

	tailp = topp->tail;

	context->teardown = 1;

	for (current = context->top; current; current = ctx->next) {
		ctx = (JoinContext *)current->priv;
		pgpAssert(ctx);

		if (!ctx->teardown)
			return( kPGPError_NoErr );;
	}

	/* Everything can be torn down, now. */
	if (tailp)
		tailp->teardown(tailp);

	for (current = context->top; current; current = myself) {
		ctx = (JoinContext *)current->priv;
		pgpAssert(ctx);
		
		myself = ctx->next;

		pgpFifoDestroy (ctx->fd, ctx->fifo);
		pgpClearMemory( ctx,  sizeof(*ctx));
		pgpContextMemFree( cdkContext, ctx);
	}
	
	return( kPGPError_NoErr );
}

static PGPPipeline *
joinDoCreate(
	PGPContextRef cdkContext,
	int dowrite, 
	PGPFifoDesc const *fifod)
{
	PGPPipeline *mod;
	PGPFifoContext *fifop = NULL;
	JoinContext *context;

	context = (JoinContext *)pgpContextMemAlloc( cdkContext,
		sizeof(*context), kPGPMemoryFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	fifop = pgpFifoCreate (cdkContext, fifod);
	if (!fifop) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}

	mod->magic = JOINMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Join Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->fd = fifod;
	context->fifo = fifop;
	context->canwrite = dowrite;

	return mod;
}

PGPPipeline **
pgpJoinCreate(
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPFifoDesc const *fifod)
{
	PGPPipeline *mod;
	JoinContext *context;

	if (!head)
		return NULL;

	pgpAssert(fifod);

	mod = joinDoCreate ( cdkContext, 1, fifod);
	if (!mod)
		return NULL;

	context = (JoinContext *)mod->priv;

	context->top = mod;
	context->tail = *head;
	*head = mod;
	return &context->tail;
}

PGPPipeline *
pgpJoinAppend(PGPPipeline *head)
{
	PGPPipeline *mod;
	JoinContext *context, *ctx;

	if (!head)
		return NULL;

	pgpAssert(head->magic == JOINMAGIC);

	context = (JoinContext *)head->priv;
	pgpAssert(context);

	mod = joinDoCreate( head->cdkContext, 0, context->fd);
	if (!mod)
		return NULL;

	mod->name = "Join Module Appendage";
	ctx = (JoinContext *)mod->priv;

	/* Splice in myself module */
	ctx->next = context->next;
	context->next = mod;

	ctx->top = context->top;

	return mod;
}

size_t
pgpJoinBuffer(PGPPipeline *myself, PGPByte const *buf, size_t len)
{
	JoinContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (JoinContext *)myself->priv;
	pgpAssert(context);

	return pgpFifoWrite (context->fd, context->fifo, buf, len);
}
