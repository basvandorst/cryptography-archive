/*
* pgpSplit.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpSplit.c,v 1.3.2.1 1997/06/07 09:51:18 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpSplit.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPSplitBufSize	16384
#else
#define kPGPSplitBufSize	BUFSIZ
#endif

#define SPLITMAGIC	0x11223344

#define MAXTAILS	2

struct Context {
			byte buffer[kPGPSplitBufSize];
			byte const *bufptr, *bufend;
			int curtail, numtails;
			struct PgpPipeline *tail[MAXTAILS];
			int scope_depth;
	};

/*
* If we got an error while writing, the pending write data gets copied to
* the context's buffer for re-transmission later. (Yes, this is needed to
* preserve the semantics of write().)
*/
static int
DoFlush(struct Context *context)
	{
			struct PgpPipeline *tailp;
			int error;
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
			memset(context->buffer, 0, len);
			context->bufptr = context->bufend = context->buffer;

		return 0;
}

static int
Flush(struct PgpPipeline *myself)
	{
			struct PgpPipeline *tailp;
			struct Context *context;
			int error, i;

		pgpAssert(myself);
			pgpAssert(myself->magic == SPLITMAGIC);

			context = (struct Context *)myself->priv;
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

		return 0;
}

static size_t
Write(struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
			struct PgpPipeline *tailp;
			struct Context *context;
			size_t block, pending, myselfwrite;
			byte const *tmpbuf;
			size_t written = 0;
			int i;

pgpAssert(myself);
pgpAssert(myself->magic == SPLITMAGIC);
pgpAssert(error);

			context = (struct Context *)myself->priv;
			pgpAssert(context);
			pgpAssert(context->tail);

/* Write out anything still hanging around from last time */
*error = DoFlush(context);
if (*error)
	return written;

/* Okay, now write out the new stuff. */
while (size) {
			/* Never write more in one block than we can buffer */
			block = min(size, sizeof(context->buffer));

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
* Annotate is a bit magic. The problem is that annotations are
* defined, as a class, to atomically succeed or fail. One particular
* annotation might play fast & loose with this, but we aren't allowed
* to break it for *all* annotations.
*
* But if we pass an annotation down the first tail, and it succeeds
* (no error returned), then pass it down the second, and get an error,
* things are a bit screwed up, because lots of modules saw the annotation
* succeed and altered their state accordingly.
*
* The solution seems to be to pass annotations down *one tail only*,
* which we use the first tail for. Other tails see data, but no
* annotations.
*/
static int
Annotate(struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;
			int error;

			pgpAssert(myself);
			pgpAssert(myself->magic == SPLITMAGIC);

			context = (struct Context *)myself->priv;
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
* the first tail; it is meaningless to the others. So we bail out
* of the loop early in that case.
*/
static int
SizeAdvise(struct PgpPipeline *myself, unsigned long bytes)
{
	struct Context *context;
int error;
int i;

pgpAssert(myself);
pgpAssert(myself->magic == SPLITMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);
pgpAssert(context->tail);

error = DoFlush(context);
if (error)
	return error;

for (i = 0; i < context->numtails; i++) {
	error = context->tail[i]->sizeAdvise(context->tail[i], bytes);
	if (error || context->scope_depth)
				return error;
	}
	return 0;
	}

static void
Teardown(struct PgpPipeline *myself)
	{
			struct Context *context;
			int i;

			pgpAssert(myself);
			pgpAssert(myself->magic == SPLITMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert(context);

			for (i = 0; i < context->numtails; i++) {
				 if (context->tail[i])
				 	context->tail[i]->teardown(context->tail[i]);
			}

			memset(context, 0, sizeof(*context));
			pgpMemFree(context);
			memset(myself, 0, sizeof(*myself));
			pgpMemFree(myself);
	}

struct PgpPipeline **
pgpSplitCreate(struct PgpPipeline **head)
	{
			struct PgpPipeline *mod;
			struct Context *context;

			if (!head)
				 return NULL;

			context = (struct Context *)pgpMemAlloc(sizeof(*context));
			if (!context)
				 return NULL;
			mod = (struct PgpPipeline *)pgpMemAlloc(sizeof(*mod));
if (!mod) {
	pgpMemFree(context);
	return NULL;
}

mod->magic = SPLITMAGIC;
mod->write = Write;
mod->flush = Flush;
mod->sizeAdvise = SizeAdvise;
mod->annotate = Annotate;
mod->teardown = Teardown;
mod->name = "Split module";
mod->priv = context;

memset(context, 0, sizeof(*context));
context->bufptr = context->buffer;

context->curtail = context->numtails = 1;
context->bufptr = context->bufend = context->buffer;

context->tail[0] = *head;
*head = mod;
return &context->tail[0];
}

struct PgpPipeline **
pgpSplitAdd(struct PgpPipeline *myself)
	{
			struct Context *context;
			struct PgpPipeline **tailp;

			pgpAssert(myself);
			pgpAssert(myself->magic == SPLITMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert(context);
			pgpAssert(context->tail);

			if (context->numtails == MAXTAILS)
				 return 0;
			if (context->curtail != context->numtails)
				 return 0;	/* We're in the middle of things! */

			tailp = &context->tail[context->numtails++];
			context->curtail = context->numtails;
			return tailp;
	}
