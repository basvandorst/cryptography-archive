/*
* pgpCopyMod.c -- Module to copy input to output
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCopyMod.c,v 1.3.2.1 1997/06/07 09:51:15 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpCopyMod.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#define COPYMODMAGIC	0xc09430d

static size_t
Write(struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
			struct PgpPipeline *tail;

			pgpAssert(myself);
			pgpAssert(myself->magic == COPYMODMAGIC);
			pgpAssert(error);

			tail = *(struct PgpPipeline **)&myself->priv;
			pgpAssert(tail);

			return tail->write(tail, buf, size, error);
	}

static int
Flush(struct PgpPipeline *myself)
	{
			struct PgpPipeline *tail;

			pgpAssert(myself);
			pgpAssert(myself->magic == COPYMODMAGIC);

			tail = *(struct PgpPipeline **)&myself->priv;
			pgpAssert(tail);

			return tail->flush(tail);
	}

static int
Annotate(struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	struct PgpPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(struct PgpPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->annotate(tail, origin, type, string, size);
}

static int
SizeAdvise(struct PgpPipeline *myself, unsigned long bytes)
{
	struct PgpPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(struct PgpPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->sizeAdvise(tail, bytes);
}

static void
Teardown(struct PgpPipeline *myself)
	{
			struct PgpPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(struct PgpPipeline **)&myself->priv;
	if (tail)
	tail->teardown(tail);

	memset(myself, 0, sizeof(*myself));
	pgpMemFree(myself);
}

struct PgpPipeline **
pgpCopyModCreate(struct PgpPipeline **head)
	{
			struct PgpPipeline *mod;

	if (!head)
		return NULL;

	mod = (struct PgpPipeline *)pgpMemAlloc(sizeof(*mod));
	if (!mod)
		return NULL;

	mod->magic = COPYMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Copy Module";
	mod->priv = *head;

	*head = mod;

	return (struct PgpPipeline **)&mod->priv;
}
