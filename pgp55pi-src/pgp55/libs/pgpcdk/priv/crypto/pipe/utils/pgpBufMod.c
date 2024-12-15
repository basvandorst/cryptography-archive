/*
 * pgpBufMod.c -- A Buffering Module; buffer until the first sizeAdvise
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpBufMod.c,v 1.25 1997/09/18 01:35:27 lloyd Exp $
 */
#include "pgpConfig.h"
#include "pgpMem.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpContext.h"
#include "pgpBufMod.h"

#define kPGPBufModCmdArgSize	4096

/*
 * This is a magic number for use within this module.  It is used to
 * make sure the passed-in module is a member of this type.  Just a
 * little extra sanity-checking.
 *
 * You should set this value to some "random" 32-bit number.
 */
 
#define BUFMODMAGIC	0xb3ffe43d

typedef struct BufModContext
{
	PGPPipeline		pipe;
	
	PGPFifoDesc const *fd;
	PGPFifoContext *fifo;
	PGPFifoContext *ann; /* Fifo to hold the saved annotations */
	PGPPipeline *tail;
	PGPByte cmdarg[kPGPBufModCmdArgSize];
	unsigned cmdlen;
	int scope_depth;
	PGPByte writing;
	DEBUG_STRUCT_CONSTRUCTOR( BufModContext )
} BufModContext;

/*
 * Buffer a command at the current spot in the data stream.  This is only
 * called when ctx->buffering is true.  What it does is find the delta
 * of this command in the data fifo from the last command in order to replay
 * the commands at the proper place.  It saves off context->written as
 * the offset and then resets the value to 0.
 *
 * The return value is 0 on success or an error code.
 *
 * Command Byte Syntax: <arglen><arg>
 */
static PGPError
bufferCommand (BufModContext *ctx, const PGPByte *arg, unsigned arglen)
{
	/* Make sure the command argument fits the buffer size */
	if (arglen > sizeof (ctx->cmdarg))
		return kPGPError_BadParams;

	if (pgpFifoWrite (ctx->fd, ctx->ann, (PGPByte *)&arglen,
			  sizeof (arglen)) != sizeof (arglen))
		return kPGPError_OutOfMemory;

	if (pgpFifoWrite (ctx->fd, ctx->ann, arg, arglen) != arglen)
		return kPGPError_OutOfMemory;

	return( kPGPError_NoErr );
}

/* Buffer up an annotation */
static PGPError
bufferAnnotation (BufModContext *ctx, PGPPipeline *origin, int type,
		  PGPByte const *string, size_t size)
{
	PGPByte *arg, *argp;
	PGPError retval;
	PGPContextRef		cdkContext;

	pgpAssertAddrValid( origin, PGPPipeline );
	cdkContext	= origin->cdkContext;

	arg = argp = (PGPByte *)
		pgpContextMemAlloc( cdkContext, sizeof (origin) + sizeof (type) +
					size + sizeof (size), 0);
	if (!arg)
		return kPGPError_OutOfMemory;

	memcpy (argp, (PGPByte *)&origin, sizeof (origin));
	argp += sizeof (origin);

	memcpy (argp, (PGPByte *)&type, sizeof (type));
	argp += sizeof (type);

	memcpy (argp, (PGPByte *)&size, sizeof (size));
	argp += sizeof (size);

	memcpy (argp, string, size);
	argp += size;

	retval = bufferCommand (ctx, arg, argp-arg);
	pgpContextMemFree( cdkContext, arg);
	return retval;
}

static PGPError
parseAnnotation (BufModContext *ctx)
{
	PGPPipeline *origin;
	int type;
	size_t size;
	PGPByte *string, *argp = ctx->cmdarg;
	PGPError retval;
	PGPContextRef	cdkContext	= ctx->pipe.cdkContext;

	pgpAssert (ctx->cmdlen >= sizeof (origin) + sizeof (type) +
		sizeof (size));

	pgpCopyMemory ( argp, (PGPByte *)&origin, sizeof (origin));
	argp += sizeof (origin);

	pgpCopyMemory ( argp, (PGPByte *)&type, sizeof (type));
	argp += sizeof (type);

	pgpCopyMemory ( argp, (PGPByte *)&size, sizeof (size));
	argp += sizeof (size);

	/* make sure we have the right number of bytes remaining */
	pgpAssert (ctx->cmdlen - size == (unsigned)(argp - ctx->cmdarg));

	string = (PGPByte *)pgpContextMemAlloc( cdkContext, size, 0);
	if (string == NULL)
		return kPGPError_OutOfMemory;

	pgpCopyMemory (argp, string, size);

	retval = ctx->tail->annotate (ctx->tail, origin, type, string, size);

	pgpContextMemFree( cdkContext, string);
	return retval;
}

/* <len><cmd> */
static PGPError
flushCommands (BufModContext *ctx)
{
	PGPError	error;
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
					 (PGPByte *)&ctx->cmdlen,
					 sizeof (ctx->cmdlen)) !=
			    sizeof (ctx->cmdlen))
				return kPGPError_FIFOReadError;
			cmdlen -= sizeof (ctx->cmdlen);
		
			if (pgpFifoRead (ctx->fd, ctx->ann, ctx->cmdarg,
					 ctx->cmdlen) != ctx->cmdlen)
				return kPGPError_FIFOReadError;
			cmdlen -= ctx->cmdlen;
		
		}
	} while (ctx->cmdlen);

	return( kPGPError_NoErr );
}

static PGPError
DoFlush (BufModContext *context)
{
	PGPError	error = kPGPError_NoErr;
	PGPByte const *ptr;
	size_t retlen;
	PGPSize len;

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

static PGPError
Flush (PGPPipeline *myself)
{
	BufModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (BufModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (!context->writing)
		return kPGPError_LazyProgrammer;	/* XXX */

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	BufModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);
	pgpAssert (error);

	context = (BufModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (!context->writing) {
		*error = kPGPError_NoErr;
		return pgpFifoWrite (context->fd, context->fifo, buf, size);
	}
	*error = DoFlush (context);
	if (*error)
		return 0;

	return context->tail->write (context->tail, buf, size, error);
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	BufModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (BufModContext *)myself->priv;
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

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	BufModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (BufModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	context->writing = 1;
	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	BufModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == BUFMODMAGIC);

	context = (BufModContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fd, context->ann);
	pgpFifoDestroy (context->fd, context->fifo);
	
	pgpClearMemory (context, sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpBufferModCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPFifoDesc const *fd)
{
	PGPPipeline *mod;
	BufModContext *context;
	PGPFifoContext *fifo, *ann;

	if (!head || !fd)
		return NULL;

	context = (BufModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear );
	if (!context)
		return NULL;
	mod = &context->pipe;

	fifo = pgpFifoCreate (cdkContext, fd);
	if (!fifo) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	ann = pgpFifoCreate (cdkContext, fd);
	if (!ann) {
		pgpFifoDestroy (fd, fifo);
		pgpContextMemFree( cdkContext, context);
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
	mod->cdkContext	= cdkContext;

	context->fd = fd;
	context->fifo = fifo;
	context->ann = ann;

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
