/*
 * pgpHashMod.c -- module to hash data
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU> and Colin Plumb
 *
 * $Id: pgpHashMod.c,v 1.16 1997/07/26 19:37:28 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpHashMod.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define HASHMODMAGIC	0x8a5430d1

typedef struct HashModContext {
	PGPPipeline		pipe;
	
	PGPHashContext *hash;
	PGPPipeline *tail;
} HashModContext;

static PGPError
Flush(PGPPipeline *myself)
{
	HashModContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == HASHMODMAGIC);

	context = (HashModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	return context->tail->flush(context->tail);
}

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	HashModContext *context;
	size_t written;

	pgpAssert(myself);
	pgpAssert(myself->magic == HASHMODMAGIC);
	pgpAssert(error);

	context = (HashModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	written = context->tail->write(context->tail, buf, size, error);

	if (written)
		PGPContinueHash(context->hash, buf, written);

	return written;	
}

static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	HashModContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == HASHMODMAGIC);

	context = (HashModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	return context->tail->annotate(context->tail, origin, type,
					string, size);
}

static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long bytes)
{
	HashModContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == HASHMODMAGIC);

	context = (HashModContext *)myself->priv;
	pgpAssert(context);
	pgpAssert(context->tail);

	return context->tail->sizeAdvise(context->tail, bytes);
}

static PGPError
Teardown(PGPPipeline *myself)
{
	HashModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == HASHMODMAGIC);

	context = (HashModContext *)myself->priv;
	pgpAssert(context);

	if (context->tail)
		context->tail->teardown(context->tail);

	pgpClearMemory( context,  sizeof(*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpHashModCreate(
	PGPContextRef cdkContext,
	PGPPipeline **head, PGPHashContext *hash)
{
	HashModContext *context;
	PGPPipeline *mod	= NULL;

	pgpAssert(hash);

	if (!head)
		return NULL;

	context = (HashModContext *)pgpContextMemAlloc(
		cdkContext, sizeof(*context), kPGPMemoryFlags_Clear);
	if ( IsNull( context ) )
		return NULL;

	mod				= &context->pipe;
	mod->magic		= HASHMODMAGIC;
	mod->write		= Write;
	mod->flush		= Flush;
	mod->sizeAdvise	= SizeAdvise;
	mod->annotate	= Annotate;
	mod->teardown	= Teardown;
	mod->name		= "Hash Module";
	mod->priv		= context;
	mod->cdkContext	= cdkContext;

	context->hash = hash;

	context->tail = *head;
	*head = mod;
	return &context->tail;
}

PGPPipeline **
pgpHashModListCreate(
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPHashListRef	hashes,
	unsigned num)
{
	PGPPipeline *th = 0, **tt = &th;
	PGPUInt32		hashIndex = 0;

	while (num--) {
		PGPHashContext *	hash;
		
		hash	= pgpHashListGetIndHash( hashes, hashIndex );
		++hashIndex;
		
		tt = pgpHashModCreate( cdkContext, tt, hash);
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
