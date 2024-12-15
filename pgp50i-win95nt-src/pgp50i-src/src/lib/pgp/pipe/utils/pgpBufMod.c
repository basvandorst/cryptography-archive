/*
* pgpBufMod.c -- A Buffering Module; buffer until the first sizeAdvise
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpBufMod.c,v 1.4.2.1 1997/06/07 09:51:14 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"
#include "pgpBufMod.h"

#define kPGPBufModCmdArgSize	4096

/*
* This is a magic number for use within this module. It is used to
* make sure the passed-in module is a member of this type. Just a
* little extra sanity-checking.
*
* You should set this value to some "random" 32-bit number.
*/

#define BUFMODMAGIC	 0xb3ffe43d

struct Context {
			struct PgpFifoDesc const *fd;
			struct PgpFifoContext *fifo;
			struct PgpFifoContext *ann; /* Fifo to hold the saved annotations */
			struct PgpPipeline *tail;
			byte cmdarg[kPGPBufModCmdArgSize];
			unsigned cmdlen;
			int scope_depth;
			byte writing;
	};

/*
* Buffer a command at the current spot in the data stream. This is only
* called when ctx->buffering is true. What it does is find the delta
* of this command in the data fifo from the last command in order to replay
* the commands at the proper place. It saves off context->written as
* the offset and then resets the value to 0.
*
* The return value is 0 on success or an error code.
*
* Command Byte Syntax: <arglen><arg>
*/
static int
bufferCommand (struct Context *ctx, const byte *arg, unsigned arglen)
	{
			/* Make sure the command argument fits the buffer size */
			if (arglen > sizeof (ctx->cmdarg))
				 return PGPERR_CMD_TOOBIG;

		if (pgpFifoWrite (ctx->fd, ctx->ann, (byte *)&arglen,
			sizeof (arglen)) != sizeof (arglen))
		return PGPERR_NOMEM;

	if (pgpFifoWrite (ctx->fd, ctx->ann, arg, arglen) != arglen)
		return PGPERR_NOMEM;

	return 0;
}

/* Buffer up an annotation */
static int
bufferAnnotation (struct Context *ctx, struct PgpPipeline *origin, int type,
				 byte const *string, size_t size)
	{
			byte *arg, *argp;
			int retval;

		arg = argp = (byte *)pgpMemAlloc (sizeof (origin) + sizeof (type) +
					size + sizeof (size));
	if (!arg)
	return PGPERR_NOMEM;

	memcpy (argp, (byte *)&origin, sizeof (origin));
	argp += sizeof (origin);

	memcpy (argp, (byte *)&type, sizeof (type));
	argp += sizeof (type);

	memcpy (argp, (byte *)&size, sizeof (size));
	argp += sizeof (size);

	memcpy (argp, string, size);
	argp += size;

	retval = bufferCommand (ctx, arg, argp-arg);
	pgpMemFree (arg);
	return retval;
}

static int
parseAnnotation (struct Context *ctx)
	{
			struct PgpPipeline *origin;
			int type;
			size_t size;
			byte *string, *argp = ctx->cmdarg;
			int retval;

			pgpAssert (ctx->cmdlen >= sizeof (origin) + sizeof (type) +
				 sizeof (size));

			memcpy ((byte *)&origin, argp, sizeof (origin));
			argp += sizeof (origin);

			memcpy ((byte *)&type, argp, sizeof (type));
			argp += sizeof (type);

			memcpy ((byte *)&size, argp, sizeof (size));
			argp += sizeof (size);

			/* make sure we have the right number of bytes remaining */
			pgpAssert (ctx->cmdlen - size == argp - ctx->cmdarg);

			string = (byte *)pgpAlloc(size);
			if (string == NULL)
				 return PGPERR_NOMEM;

			pgpCopyMemory (argp, string, size);

			retval = ctx->tail->annotate (ctx->tail, origin, type, string, size);

			pgpFree(string);
			return retval;
	}

/* <len><cmd> */
static int
flushCommands (struct Context *ctx)
	{
			int error;
			unsigned cmdlen = pgpFifoSize (ctx->fd, ctx->ann);

			do {
					if (ctx->cmdlen) {
							error = parseAnnotation (ctx);
							if (error)
								 return error;
						
							ctx->cmdlen = 0;
					}

					if (cmdlen) {			
							if (pgpFifoRead (ctx->fd, ctx->ann,
								 	(byte *)&ctx->cmdlen,
								 	sizeof (ctx->cmdlen)) !=
							sizeof (ctx->cmdlen))
								 return PGPERR_FIFO_READ;
							cmdlen -= sizeof (ctx->cmdlen);
				
							if (pgpFifoRead (ctx->fd, ctx->ann, ctx->cmdarg,
								 	ctx->cmdlen) != ctx->cmdlen)
								 return PGPERR_FIFO_READ;
							cmdlen -= ctx->cmdlen;
				
					}
			} while (ctx->cmdlen);

			return 0;
}

static int
DoFlush (struct Context *context)
	{
			int error = 0;
			byte const *ptr;
			size_t retlen;
			unsigned len;

			/* first, flush out any commands */
			error = flushCommands (context);
			if (error)
				 return error;

			ptr = pgpFifoPeek (context->fd, context->fifo, &len);
			while (len) {
					retlen = context->tail->write (context->tail,
							 	ptr, len, &error);
					pgpFifoSeek (context->fd, context->fifo, retlen);
					if (error)
						return error;
				
					ptr = pgpFifoPeek (context->fd, context->fifo, &len);
			}

			return error;
}

static int
Flush (struct PgpPipeline *myself)
	{
			struct Context *context;
			int error;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (!context->writing)
			return -1;	/* XXX */

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	struct Context *context;

pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);
	pgpAssert (error);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (!context->writing) {
		*error = 0;
		return pgpFifoWrite (context->fd, context->fifo, buf, size);
	}
	*error = DoFlush (context);
	if (*error)
		return 0;

	return context->tail->write (context->tail, buf, size, error);
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;
			int error;

			pgpAssert (myself);
			pgpAssert (myself->magic == BUFMODMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

			if (!context->writing) {
					/* Make sure there isn't any data buffered! */
					pgpAssert (!pgpFifoSize (context->fd, context->fifo));

					error = bufferAnnotation (context, origin, type, string, size);
			} else
					error = context->tail->annotate (context->tail, origin, type,
							 		 string, size);
			if (!error)
				 PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
			pgpAssert(context->scope_depth != -1);
			return error;
	}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Context *context;
			int error;

			pgpAssert (myself);
			pgpAssert (myself->magic == BUFMODMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

	context->writing = 1;
	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
{
	struct Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fd, context->ann);
	pgpFifoDestroy (context->fd, context->fifo);
	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline **
pgpBufferModCreate (struct PgpPipeline **head, struct PgpFifoDesc const *fd)
	{
			struct PgpPipeline *mod;
			struct Context *context;
			struct PgpFifoContext *fifo, *ann;

			if (!head || !fd)
				 return NULL;

			context = (struct Context *)pgpMemAlloc (sizeof (*context));
			if (!context)
				 return NULL;
			mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
			if (!mod) {
				 pgpMemFree (context);
				 return NULL;
			}
			fifo = pgpFifoCreate (fd);
			if (!fifo) {
				 pgpMemFree (context);
				 pgpMemFree (mod);
				 return NULL;
			}
			ann = pgpFifoCreate (fd);
			if (!ann) {
				 pgpFifoDestroy (fd, fifo);
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}

	mod->magic = BUFMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Buffer Module";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->fd = fd;
	context->fifo = fifo;
	context->ann = ann;

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
