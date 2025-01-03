/*
 * pgpDevNull.c -- sink module to eat all input.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpDevNull.c,v 1.12 1997/09/18 01:35:29 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpDevNull.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define DEVNULLMAGIC	0xde557711

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	/* To shut up compiler warnings */
	(void)myself;
	(void)buf;

	pgpAssert(error);
	*error = kPGPError_NoErr;

	return size;
}

static PGPError
Flush (PGPPipeline *myself)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	/* To shut up compiler warnings */
	(void)myself;

	return( kPGPError_NoErr );
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	/* To shut up compiler warnings */
	(void)myself;
	(void)origin;
	(void)type;
	(void)string;
	(void)size;

	return( kPGPError_NoErr );
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	/* To shut up compiler warnings */
	(void)myself;
	(void)bytes;

	return( kPGPError_NoErr );
}

static PGPError
Teardown (PGPPipeline *myself)
{
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;
	
	pgpAssert(myself);
	pgpAssert(myself->magic == DEVNULLMAGIC);

	pgpClearMemory( myself,  sizeof (*myself));
	pgpContextMemFree( cdkContext, myself);
	
	return kPGPError_NoErr;
}

PGPPipeline *
pgpDevNullCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head)
{
	PGPPipeline *mod;

	if (!head)
		return NULL;

	mod = (PGPPipeline *)
		pgpContextMemAlloc( cdkContext, sizeof(*mod), kPGPMemoryFlags_Clear);
	if (mod) {
		mod->magic = DEVNULLMAGIC;
		mod->write = Write;
		mod->flush = Flush;
		mod->sizeAdvise = SizeAdvise;
		mod->annotate = Annotate;
		mod->teardown = Teardown;
		mod->name = "Dev Null";
		mod->cdkContext	= cdkContext;

		*head = mod;
		return *head;
	}
	return NULL;
}
