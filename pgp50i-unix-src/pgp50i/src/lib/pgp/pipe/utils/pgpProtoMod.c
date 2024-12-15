/*
* pgpProtoMod.c -- a prototype module for PGP
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpProtoMod.c,v 1.3.2.1 1997/06/07 09:51:18 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

/*
* This is a magic number for use within this module. It is used to
* make sure the passed-in module is a member of this type. Just a
* little extra sanity-checking.
*
* You should set this value to some "random" 32-bit number.
*/

#define MAGIC	0x0

struct Context {
			byte buffer[BUFSIZ];
			byte *bufptr;
			size_t buflen;
			struct PgpPipeline *tail;
			int scope_depth;
	};

static int
DoFlush (struct Context *context)
	{
			int error = 0;
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
Flush (struct PgpPipeline *myself)
	{
			struct Context *context;
			int error;

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
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
			struct Context *context;
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
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;
			int error;

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
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Context *context;
			int error;

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
Teardown (struct PgpPipeline *myself)
{
	struct Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MAGIC);

	context = myself->priv;
	pgpAssert (context);

			if (context->tail)
				 context->tail->teardown (context->tail);

memset (context, 0, sizeof (*context));
pgpMemFree (context);
memset (myself, 0, sizeof (*myself));
pgpMemFree (myself);
}

struct PgpPipeline **
Create (struct PgpPipeline **head)
	{
			struct PgpPipeline *mod;
			struct Context *context;

			if (!head)
			return NULL;

			context = pgpMemAlloc (sizeof (*context));
			if (!context)
				 return NULL;
			mod = pgpMemAlloc (sizeof (*mod));
			if (!mod) {
				 pgpMemFree (context);
				 return NULL;
			}

		mod->magic = MAGIC;
			mod->write = Write;
			mod->flush = Flush;
			mod->sizeAdvise = SizeAdvise;
			mod->annotate = Annotate;
		mod->teardown = Teardown;
		mod->name = "Prototype"; /* XXX Set myself to the module name! */
			mod->priv = context;

		memset (context, 0, sizeof (*context));
			context->bufptr = context->buffer;

			context->tail = *head;
			*head = mod;
			return &context->tail;
	}
