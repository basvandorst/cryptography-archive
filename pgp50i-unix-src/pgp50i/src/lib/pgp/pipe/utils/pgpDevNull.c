/*
* pgpDevNull.c -- sink module to eat all input.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpDevNull.c,v 1.3.2.1 1997/06/07 09:51:15 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpDevNull.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#define DEVNULLMAGIC	0xde557711

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
			pgpAssert(myself);
			pgpAssert(myself->magic == DEVNULLMAGIC);

			/* To shut up compiler warnings */
			(void)myself;
			(void)buf;

			pgpAssert(error);
			*error = 0;

			return size;
}

static int
Flush (struct PgpPipeline *myself)
	{
			pgpAssert(myself);
			pgpAssert(myself->magic == DEVNULLMAGIC);

			/* To shut up compiler warnings */
			(void)myself;

			return 0;
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			pgpAssert(myself);
			pgpAssert(myself->magic == DEVNULLMAGIC);

			/* To shut up compiler warnings */
			(void)myself;
			(void)origin;
			(void)type;
			(void)string;
			(void)size;

			return 0;
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
			pgpAssert(myself);
			pgpAssert(myself->magic == DEVNULLMAGIC);

			/* To shut up compiler warnings */
			(void)myself;
			(void)bytes;

			return 0;
}

static void
Teardown (struct PgpPipeline *myself)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	memset(myself, 0, sizeof (*myself));
	pgpMemFree(myself);
}

struct PgpPipeline *
pgpDevNullCreate (struct PgpPipeline **head)
{
			struct PgpPipeline *mod;

			if (!head)
				 return NULL;

			mod = (struct PgpPipeline *)pgpMemAlloc(sizeof(*mod));
			if (mod) {
					mod->magic = DEVNULLMAGIC;
					mod->write = Write;
					mod->flush = Flush;
					mod->sizeAdvise = SizeAdvise;
					mod->annotate = Annotate;
					mod->teardown = Teardown;
					mod->name = "Dev Null";

					*head = mod;
					return *head;
			}
			return NULL;
	}
