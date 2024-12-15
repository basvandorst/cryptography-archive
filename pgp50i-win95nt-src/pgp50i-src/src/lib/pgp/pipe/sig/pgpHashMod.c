/*
* pgpHashMod.c -- module to hash data
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU> and Colin Plumb
*
* $Id: pgpHashMod.c,v 1.3.2.1 1997/06/07 09:51:04 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpHashMod.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#define HASHMODMAGIC 0x8a5430d1

struct Context {
 struct PgpHashContext *hash;
 struct PgpPipeline *tail;
};

static int
Flush(struct PgpPipeline *myself)
	{
		struct Context *context;

		pgpAssert(myself);
		pgpAssert(myself->magic == HASHMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert(context);
		pgpAssert(context->tail);

		return context->tail->flush(context->tail);
	}

static size_t
Write(struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
		struct Context *context;
		size_t written;

		pgpAssert(myself);
		pgpAssert(myself->magic == HASHMODMAGIC);
		pgpAssert(error);

		context = (struct Context *)myself->priv;
		pgpAssert(context);
		pgpAssert(context->tail);

		written = context->tail->write(context->tail, buf, size, error);

 if (written)
  pgpHashUpdate(context->hash, buf, written);

 return written;
}

static int
Annotate(struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;

		pgpAssert(myself);
		pgpAssert(myself->magic == HASHMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert(context);
		pgpAssert(context->tail);

		return context->tail->annotate(context->tail, origin, type,
		    string, size);
	}

static int
SizeAdvise(struct PgpPipeline *myself, unsigned long bytes)
	{
		struct Context *context;

		pgpAssert(myself);
		pgpAssert(myself->magic == HASHMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);
pgpAssert(context->tail);

return context->tail->sizeAdvise(context->tail, bytes);
}

static void
Teardown(struct PgpPipeline *myself)
{
		struct Context *context;

pgpAssert(myself);
pgpAssert(myself->magic == HASHMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert(context);

if (context->tail)
 context->tail->teardown(context->tail);

memset(context, 0, sizeof(*context));
pgpMemFree(context);
memset(myself, 0, sizeof(*myself));
pgpMemFree(myself);
}

struct PgpPipeline **
pgpHashModCreate(struct PgpPipeline **head, struct PgpHashContext *hash)
{
		struct Context *context;
		struct PgpPipeline *mod;

		pgpAssert(hash);

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

		mod->magic = HASHMODMAGIC;
		mod->write = Write;
		mod->flush = Flush;
		mod->sizeAdvise = SizeAdvise;
		mod->annotate = Annotate;
		mod->teardown = Teardown;
		mod->name = "Hash Module";
		mod->priv = context;

		memset(context, 0, sizeof(*context));
		context->hash = hash;

		context->tail = *head;
		*head = mod;
		return &context->tail;
}

struct PgpPipeline **
pgpHashModListCreate(struct PgpPipeline **head, struct PgpHashContext *hashes,
		unsigned num)
	{
		struct PgpPipeline *th = 0, **tt = &th;

		while (num--) {
			tt = pgpHashModCreate(tt, hashes++);
			if (!tt) {
				if (th)
				 th->teardown(th);
				return 0;
			}
		}
		*tt = *head;
		*head = th;
		return tt;
	}
