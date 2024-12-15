/*
 * pgpCopyMod.c -- Module to copy input to output
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCopyMod.c,v 1.14 1997/12/12 01:14:15 heller Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpCopyMod.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define COPYMODMAGIC	0xc09430d

static size_t
Write(PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	PGPPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);
	pgpAssert(error);

	tail = *(PGPPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->write(tail, buf, size, error);
}

static PGPError
Flush(PGPPipeline *myself)
{
	PGPPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(PGPPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->flush(tail);
}

static PGPError
Annotate(PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	PGPPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(PGPPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->annotate(tail, origin, type, string, size);
}

static PGPError
SizeAdvise(PGPPipeline *myself, unsigned long bytes)
{
	PGPPipeline *tail;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(PGPPipeline **)&myself->priv;
	pgpAssert(tail);

	return tail->sizeAdvise(tail, bytes);
}

static PGPError
Teardown(PGPPipeline *myself)
{
	PGPPipeline *		tail;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert(myself);
	pgpAssert(myself->magic == COPYMODMAGIC);

	tail = *(PGPPipeline **)&myself->priv;
	if (tail)
		tail->teardown(tail);

	pgpClearMemory( myself,  sizeof(*myself));
	pgpContextMemFree( cdkContext, myself);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpCopyModCreate(
	PGPContextRef cdkContext,
	PGPPipeline **head)
{
	PGPPipeline *mod;

	if (!head)
		return NULL;

	mod = (PGPPipeline *)pgpContextMemAlloc( cdkContext,
		sizeof(*mod), kPGPMemoryMgrFlags_Clear);
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
	mod->cdkContext	= cdkContext;

	*head = mod;

	return (PGPPipeline **)&mod->priv;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
