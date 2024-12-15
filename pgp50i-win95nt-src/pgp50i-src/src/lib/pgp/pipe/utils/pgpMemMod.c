/*
* pgpMemMod.c -- Module to output to a fixed-size memory buffer
*
* If we overflow the buffer, we discard the rest. That is not an error.
* However we provide an annotation to allow reading how much we would
* have written.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMemMod.c,v 1.4.2.3 1997/06/07 09:51:17 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpMemMod.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"

#define MEMMODMAGIC	0xfeedbabe

/*
* Note that byte_count may come to exceed buf_size, in which case we
* discard the overflow. We provide an annotation to read the total number
* of bytes written.
*/
struct memModContext {
	int scope_depth;
	int enable;					/* True if accepting data, false to discard */
	size_t buf_size;			/* Total size of memory buffer */
	size_t byte_count;			/* Number of bytes we have been given */
	byte *buf;					/* Start of memory buffer */
};


static int
memFlush (struct PgpPipeline *myself)
{
	struct memModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (struct memModContext *)myself->priv;
	pgpAssert (context);

	return 0;
}

/* Write what is given, but disard any overflows. */
static size_t
memWrite (struct PgpPipeline *myself, byte const *buffer, size_t size,
	int *error)
{
	struct memModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (struct memModContext *)myself->priv;
	pgpAssert (context);

	/* If disabled, discard data */
	if (!context->enable)
		return size;

	if (context->buf_size > context->byte_count) {
		size_t avail = context->buf_size - context->byte_count;
		size_t to_write = min (avail, size);

		if (to_write)
			memcpy (context->buf + context->byte_count, buffer, to_write);
	}

	if (error)
		*error = 0;
	context->byte_count += size;
	return size;
}

/*
* We accept an annotation to return the number of bytes we were asked
* to write. We also provide an annotation to return the size of the
* buffer.
* We return our results in the passed-in string pointer, which must be
* of size sizeof(size_t).
* The ENABLE annotation causes us to turn on and off whether we accept data.
*/
static int
memAnnotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	struct memModContext *context;

	(void)origin;	 /* Avoid warning */
	(void)size;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (struct memModContext *)myself->priv;
	pgpAssert (context);

	switch (type) {
	case PGPANN_MEM_BYTECOUNT:
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->byte_count;
		return 0;
	case PGPANN_MEM_BUFSIZE:
		pgpAssert (string);
		pgpAssert (size == sizeof(size_t));
		*(size_t *)string = context->buf_size;
		return 0;
	case PGPANN_MEM_ENABLE:
		pgpAssert (string);
		pgpAssert (size == 1);
		context->enable = *(byte *)string;
		return 0;
	default:
		;		 /* do nothing */
	}

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return 0;
}

static int
memSizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
	struct memModContext *context;

	(void)bytes;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (struct memModContext *)myself->priv;
	pgpAssert (context);

	return 0;
}

static void
memTeardown (struct PgpPipeline *myself)
{
	struct memModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MEMMODMAGIC);

	context = (struct memModContext *)myself->priv;
	pgpAssert (context);

	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline *
pgpMemModCreate (struct PgpPipeline **head, char *buf, size_t buf_size)
{
	struct PgpPipeline *mod;
	struct memModContext *context;

	if (!head)
		return NULL;

	*head = 0;

	context = (struct memModContext *)pgpMemAlloc (sizeof(*context));
	if (!context)
		return NULL;
	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof(*mod));
	if (!mod) {
		pgpMemFree(context);
		return NULL;
	}

	mod->magic = MEMMODMAGIC;
	mod->write = memWrite;
	mod->flush = memFlush;
	mod->sizeAdvise = memSizeAdvise;
	mod->annotate = memAnnotate;
	mod->teardown = memTeardown;
	mod->name = "Write memory";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->buf = (unsigned char *) buf;
	context->buf_size = buf_size;
	context->enable = 1;

	*head = mod;
	return mod;
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
