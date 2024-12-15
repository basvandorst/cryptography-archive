/*
 * pgpMemMod.c -- Module to output to a fixed-size memory buffer
 *
 * If we overflow the buffer, we discard the rest.  That is not an error.
 * However we provide an annotation to allow reading how much we would
 * have written.
 *
 * Copyright (C) 1996, 1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpMemMod.c,v 1.23 1999/03/10 02:52:58 heller Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpMemMod.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#define MEMMODMAGIC	0xfeedbabe

/*
 * Note that byte_count may come to exceed buf_size, in which case we
 * discard the overflow.  We provide an annotation to read the total number
 * of bytes written.
 */
typedef struct memModContext {
	PGPPipeline	pipe;
	
	int scope_depth;
	PGPBoolean enable;	/* True if accepting data, false to discard */
	size_t buf_size;	/* Total size of memory buffer */
	size_t byte_count;	/* Number of bytes we have been given */
	PGPByte *buf;		/* Start of memory buffer */
	DEBUG_STRUCT_CONSTRUCTOR( memModContext )
} memModContext;


static PGPError
memFlush (PGPPipeline *myself)
{
	memModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (memModContext *)myself->priv;
	pgpAssert (context);

	return( kPGPError_NoErr );
}

/* Write what is given, but disard any overflows. */
static size_t
memWrite (PGPPipeline *myself, PGPByte const *buffer, size_t size,
	   PGPError *error)
{
	memModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (memModContext *)myself->priv;
	pgpAssert (context);

	/* If disabled, discard data */
	if (!context->enable)
		return size;

	if (context->buf_size > context->byte_count) {
		size_t avail = context->buf_size - context->byte_count;
		size_t to_write = pgpMin (avail, size);

		if (to_write)
			memcpy (context->buf + context->byte_count, buffer, to_write);

		/* Store an uncounted null beyond the end of the buffer */
		if (context->byte_count+to_write < context->buf_size)
			context->buf[context->byte_count+to_write] = '\0';
	}

	if (error)
		*error = kPGPError_NoErr;
	context->byte_count += size;
	return size;
}

/*
 * We accept an annotation to return the number of bytes we were asked
 * to write.  We also provide an annotation to return the size of the
 * buffer.
 * We return our results in the passed-in string pointer, which must be
 * of size sizeof(size_t).
 * The ENABLE annotation causes us to turn on and off whether we accept data.
 */
static PGPError
memAnnotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	      PGPByte const *string, size_t size)
{
	memModContext *context;

	(void)origin;	/* Avoid warning */
	(void)size;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (memModContext *)myself->priv;
	pgpAssert (context);

	switch (type) {
	case PGPANN_MEM_BYTECOUNT:
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->byte_count;
		return( kPGPError_NoErr );
	case PGPANN_MEM_BUFSIZE:
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->buf_size;
		return( kPGPError_NoErr );
	case PGPANN_MEM_ENABLE:
		pgpAssert (string);
		pgpAssert (size == 1);
		context->enable = *(PGPByte *)string;
		return( kPGPError_NoErr );
	case PGPANN_MEM_PREPEND:
		/* This means that we have some data in buffer already to skip */
		if ( context->buf != (PGPByte *)string )
			return kPGPError_BadParams;
		context->byte_count += size;		/* Skip past data */
		if (context->byte_count > context->buf_size)
			context->byte_count = context->buf_size;
		return( kPGPError_NoErr );
	default:
		;		/* do nothing */
	}

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return( kPGPError_NoErr );
}

static PGPError
memSizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	memModContext *context;

	(void)bytes;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (memModContext *)myself->priv;
	pgpAssert (context);

	return( kPGPError_NoErr );
}

static PGPError
memTeardown (PGPPipeline *myself)
{
	memModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (memModContext *)myself->priv;
	pgpAssert (context);

	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return( kPGPError_NoErr );
}

PGPPipeline *
pgpMemModCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	char *buf,
	size_t buf_size)
{
	PGPPipeline *mod;
	memModContext *context;

	if (!head)
		return NULL;

	*head = 0;

	context = (memModContext *)pgpContextMemAlloc( cdkContext,
		sizeof(*context), kPGPMemoryMgrFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = MEMMODMAGIC;
	mod->write = memWrite;
	mod->flush = memFlush;
	mod->sizeAdvise = memSizeAdvise;
	mod->annotate = memAnnotate;
	mod->teardown = memTeardown;
	mod->name = "Write memory";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->buf = (PGPByte *)buf;
	context->buf_size = buf_size;
	context->enable = 1;

	*head = mod;
	return mod;
}
