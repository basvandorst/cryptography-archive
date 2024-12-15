/*
* pgpInfMod.c -- Inflation (decompression) module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpInfMod.c,v 1.3.2.1 1997/06/07 09:51:08 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpInfMod.h"
#include "pgpZInflate.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#define INFMODMAGIC 0x01258aef

struct Context {
 struct InflateContext *inf;
 struct PgpPipeline *tail;
 int scope_depth;
};

static int
DoFlush (struct Context *context)
	{
		int error = 0;
		unsigned len;
		byte const *ptr;
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

static int
Flush (struct PgpPipeline *myself)
{
 struct Context *context;
		int error;

		pgpAssert (myself);
		pgpAssert (myself->magic == INFMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert (context);
		pgpAssert (context->tail);

		error = DoFlush (context);
		if (error)
		 return error;

		return context->tail->flush (context->tail);
}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
		struct Context *context;
		size_t written, retlen = 0;

		pgpAssert (myself);
		pgpAssert (myself->magic == INFMODMAGIC);
		pgpAssert (error);

		context = (struct Context *)myself->priv;
		pgpAssert (context);
		pgpAssert (context->tail);

		/* Flush what is buffered */
		*error = DoFlush (context);
		if (*error)
			return retlen;

		do {
			/* Process data */
			written = infWrite (context->inf, (byte *)buf, size, error);
			retlen += written;
			buf += written;
			size -= written;
			if (*error)
			 return retlen;

			/* And flush it! */
			*error = DoFlush (context);
			if (*error)
			 return retlen;

} while (size);
/* Continue until we have left to read */

return retlen;
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
 byte const *string, size_t size)
{
		struct Context *context;
		int error;

		pgpAssert (myself);
		pgpAssert (myself->magic == INFMODMAGIC);

		context = (struct Context *)myself->priv;
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

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
		struct Context *context;
		int eoferr;
		int error;

pgpAssert (myself);
pgpAssert (myself->magic == INFMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

if (!bytes && !context->scope_depth) {
		do {
			eoferr = infEOF (context->inf);
			if (eoferr < 0)
			 return eoferr;

			error = DoFlush (context);
			if (error)
			 return error;
		} while (eoferr);
	}

return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
{
struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == INFMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (context->tail)
 context->tail->teardown (context->tail);

infFree (context->inf);
memset (context, 0, sizeof (*context));
pgpMemFree (context);
memset (myself, 0, sizeof (*myself));
pgpMemFree (myself);
}

struct PgpPipeline **
infModCreate (struct PgpPipeline **head)
	{
		struct PgpPipeline *mod;
		struct Context *context;

		if (!head)
		 return NULL;

		context = (struct Context *)pgpMemAlloc (sizeof (*context));
		if (!context)
		 return NULL;
		mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
		if (!mod){
		 pgpMemFree (context);
		 return NULL;
		}

mod->magic = INFMODMAGIC;
mod->write = Write;
mod->flush = Flush;
mod->sizeAdvise = SizeAdvise;
mod->annotate = Annotate;
mod->teardown = Teardown;
mod->name = "Inflation Module";
mod->priv = context;

memset (context, 0, sizeof (*context));
context->inf = infAlloc ();

context->tail = *head;
*head = mod;
return &context->tail;
}
