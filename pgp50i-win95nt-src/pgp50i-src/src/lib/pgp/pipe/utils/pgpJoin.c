/*
* pgpJoin.c -- join multiple modules into a single stream
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpJoin.c,v 1.3.2.1 1997/06/07 09:51:16 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpJoin.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"

#define JOINMAGIC	0x00112233

struct Context {
			struct PgpFifoDesc const *fd;
			struct PgpFifoContext *fifo;
			struct PgpPipeline *tail;
			struct PgpPipeline *next;
			struct PgpPipeline *top;
			unsigned long size;
			int sizevalid;
			int canwrite;
			int eof;
			int teardown;
	};

static int
DoFlush(struct Context *context)
	{
			int error = 0;
			byte const *ptr;
			unsigned len;
			size_t retlen;
			struct Context *topp = (struct Context *)context->top->priv;
			struct PgpPipeline *tailp = topp->tail;

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
			* Now, check if we are in an EOF condition. If so, then we
			* can tell the next input that it is allowed to write, and
			* then we can try to flush out that buffer as well.
			*/
			if (context->eof && context->next) {
				 struct Context *ctx = (struct Context *)context->next->priv;

				 ctx->canwrite = 1;
				 error = DoFlush (ctx);
			}

			return error;
}

static int
Flush(struct PgpPipeline *myself)
	{
			struct Context *context, *topp;
			struct PgpPipeline *tailp;
			int error;

			pgpAssert(myself);
			pgpAssert(myself->magic == JOINMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert(context);

			topp = (struct Context *)context->top->priv;
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
			return 0;
	}

static size_t
Write(struct PgpPipeline *myself, byte const *buf, size_t len, int *error)
{
	struct Context *context, *topp;
	struct PgpPipeline *tailp;
	size_t written;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);
	pgpAssert(error);

	context = (struct Context *)myself->priv;
	pgpAssert(context);

	topp = (struct Context *)context->top->priv;
	pgpAssert(topp);

	/* If we're not allowed to write, then buffer things up until we can */
	*error = 0;
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

static int
Annotate(struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
				byte const *string, size_t len)
	{
				struct Context *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert(context);

	/* XXX -- Don't allow annotations */
	(void)origin;
	(void)string;
	(void)len;

	switch (type) {
	case PGPANN_FILE_BEGIN:
	case PGPANN_FILE_END:
	case PGPANN_INPUT_BEGIN:
	case PGPANN_INPUT_END:
		return 0;
		/*NOTREACHED*/
	default:
		return PGPERR_JOIN_BADANN;
	}
	/*NOTREACHED*/
}

/*
* context->size is the number of bytes in the fifo plus the number of
* bytes we still should expect to receive. Once this is set, we
* expect it to be held by the caller. Each input gets one of these.
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
* that input. This means that context->size is the number of bytes
* buffered in that context.
*
* Then it will see if all inputs have a valid size. If they do, then
* it will add up all the sizes and send that as a sizeAdvise to the
* tail.
*
* Next, we will flush out the buffered data, and if that returns
* without an error we know that everything is flushed from all the
* modules from this one to the end.
*
* Finally we check if everything is EOF. We know that if everything
* is at EOF, and we got this far, that there must not be anything in
* any buffers, and we are not expecting anymore data to be written.
* Therefore, we pass a sizeAdvise (0) to the tail.
*/
static int
SizeAdvise(struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Context *ctx, *context, *topp;
			struct PgpPipeline *current, *tailp;
			unsigned long fifolen;
			unsigned long total;
			int flag = 0;
			int error;

			pgpAssert(myself);
			pgpAssert(myself->magic == JOINMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert(context);

			topp = (struct Context *)context->top->priv;
			pgpAssert(topp);

			tailp = topp->tail;

			/*
			* XXX If annotations get added, make sure we only consider calls
			* outside them. I.e. "if (context->scope_depth) return 0;"
			*/

	/* save off the size of the buffered data */
	fifolen = pgpFifoSize (context->fd, context->fifo);

	/* Check/set the size */
	if (context->sizevalid) {
		if (bytes != context->size - fifolen)
			/* It doesn't jibe -- this is an error */
			return PGPERR_SIZEADVISE;
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
				 ctx = (struct Context *)current->priv;
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
	* If we cannot write, then we cannot flush! Just accept that
	* the sizeAdvise worked to this point and return success.
	*/
	if (!context->canwrite)
	return 0;

	pgpAssert(tailp);
	/*
	* Flush the data. This will return an error, or everything
	* will be flushed out of the fifo.
	*/
	error = DoFlush (context);
	if (error)
		return error;


			/* At this point, check if everything is EOF. */
			for (current = context->top; current; current = ctx->next) {
					ctx = (struct Context *)current->priv;
					pgpAssert(ctx);
				
					if (!ctx->eof)
						/* Something isn't done -- just return success */
						return 0;
			}

			/* Send a sizeAdvise (0) -- all the inputs are done writing. */
			return tailp->sizeAdvise(tailp, 0);
	}

static void
Teardown(struct PgpPipeline *myself)
{
			struct PgpPipeline *current, *tailp;
			struct Context *ctx, *context, *topp;

			pgpAssert(myself);
			pgpAssert(myself->magic == JOINMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert(context);

			topp = (struct Context *)context->top->priv;
			pgpAssert(topp);

			tailp = topp->tail;

			context->teardown = 1;

			for (current = context->top; current; current = ctx->next) {
					ctx = (struct Context *)current->priv;
					pgpAssert(ctx);

					if (!ctx->teardown)
						return;
			}

			/* Everything can be torn down, now. */
			if (tailp)
				 tailp->teardown(tailp);

			for (current = context->top; current; current = myself) {
					ctx = (struct Context *)current->priv;
					pgpAssert(ctx);
				
					myself = ctx->next;

					pgpFifoDestroy (ctx->fd, ctx->fifo);
					memset(ctx, 0, sizeof(*ctx));
					pgpMemFree(ctx);
					memset(current, 0, sizeof(*current));
					pgpMemFree(current);
			}
}

static struct PgpPipeline *
joinDoCreate(int dowrite, struct PgpFifoDesc const *fifod)
	{
			struct PgpPipeline *mod;
			struct PgpFifoContext *fifop = NULL;
			struct Context *context;

			context = (struct Context *)pgpMemAlloc(sizeof(*context));
			if (!context)
				 return NULL;
			mod = (struct PgpPipeline *)pgpMemAlloc(sizeof(*mod));
			if (!mod) {
				 pgpMemFree(context);
				 return NULL;
			}
			fifop = pgpFifoCreate (fifod);
			if (!fifop) {
				 pgpMemFree(context);
				 pgpMemFree(mod);
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

			memset(context, 0, sizeof(*context));
			context->fd = fifod;
			context->fifo = fifop;
			context->canwrite = dowrite;

			return mod;
}

struct PgpPipeline **
pgpJoinCreate(struct PgpPipeline **head, struct PgpFifoDesc const *fifod)
	{
			struct PgpPipeline *mod;
			struct Context *context;

			if (!head)
				 return NULL;

			pgpAssert(fifod);

			mod = joinDoCreate (1, fifod);
			if (!mod)
				 return NULL;

			context = (struct Context *)mod->priv;

				context->top = mod;
	context->tail = *head;
	*head = mod;
	return &context->tail;
}

struct PgpPipeline *
pgpJoinAppend(struct PgpPipeline *head)
{
	struct PgpPipeline *mod;
	struct Context *context, *ctx;

	if (!head)
		return NULL;

	pgpAssert(head->magic == JOINMAGIC);

	context = (struct Context *)head->priv;
	pgpAssert(context);

	mod = joinDoCreate(0, context->fd);
	if (!mod)
		return NULL;

	mod->name = "Join Module Appendage";
	ctx = (struct Context *)mod->priv;

	/* Splice in myself module */
	ctx->next = context->next;
	context->next = mod;

	ctx->top = context->top;

	return mod;
}

size_t
pgpJoinBuffer(struct PgpPipeline *myself, byte const *buf, size_t len)
{
	struct Context *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == JOINMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert(context);

	return pgpFifoWrite (context->fd, context->fifo, buf, len);
}
