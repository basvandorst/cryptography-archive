/*
 * pgpArmrFil.c -- an Armor File Write module.  It watches for MULTIARMOR
 *	annotation scopes and will open multiple files if it occurs.
 *	Otherwise, it will only a single file when data is first written.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpArmrFil.c,v 1.18 1997/07/26 19:37:24 lloyd Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpArmrFil.h"
#include "pgpAnnotate.h"
#include "pgpFileMod.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpFile.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define ARMORWRITEFILEMAGIC	0xa434f11e

typedef struct ArmorFilContext
{
	/* by placing this here, we can just allocate everything at once */
	PGPPipeline		pipe;
	
	PGPPipeline *tail;
	PGPFile * (*fileOpen) (void *arg,
				      void const *baseName, unsigned number);
        void *arg;
	void const *baseName;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( ArmorFilContext )
} ArmorFilContext;

static PGPError
Flush (PGPPipeline *myself)
{
	ArmorFilContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

	context = (ArmorFilContext *)myself->priv;
	pgpAssert (context);
	if (!context->tail)
		return kPGPError_NoErr;

	return context->tail->flush (context->tail);
}

static PGPError
openFile (ArmorFilContext *context, unsigned num)
{
	PGPPipeline *tail = NULL;
	PGPFile *fp;
	PGPError	error;

	pgpAssert (!context->tail);

	fp = context->fileOpen (context->arg, context->baseName, num);
	if (!fp)
		return kPGPError_CantOpenFile;
	
	if (!pgpFileWriteCreate ( context->tail->cdkContext, &tail, fp, 1)) {
		error = pgpFileClose (fp);
		if (error)
			return error;
		return kPGPError_OutOfMemory;
	}
	context->tail = tail;
	return kPGPError_NoErr;
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	ArmorFilContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);
	pgpAssert (error);

	context = (ArmorFilContext *)myself->priv;
	pgpAssert (context);

	if (!context->tail) {
		*error = openFile (context, 0);
		if (*error)
			return 0;
	}		

	return context->tail->write (context->tail, buf, size, error);
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	ArmorFilContext *context;
	PGPError	error = kPGPError_NoErr;
	unsigned num;

	(void)origin;	/* Avoid warning */
	(void)size;
	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

	context = (ArmorFilContext *)myself->priv;
	pgpAssert (context);

	/* See what we should do here */
	switch (type) {
	case PGPANN_MULTIARMOR_BEGIN:
		pgpAssert (!context->tail);
		pgpAssert (size == sizeof (num));
		num = *((unsigned *)string);
		error = openFile (context, num);
		break;
	case PGPANN_MULTIARMOR_END:
		pgpAssert (context->tail);
		error = context->tail->sizeAdvise (context->tail, 0);
		if (error)
			return error;
		context->tail->teardown (context->tail);
		context->tail = NULL;
		break;
	default:
		;
	}

	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	ArmorFilContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

	context = (ArmorFilContext *)myself->priv;
	pgpAssert (context);
	if (!context->tail)
		return kPGPError_NoErr;

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	ArmorFilContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

	context = (ArmorFilContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpClearMemory( myself, sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return( kPGPError_NoErr );
}

PGPPipeline *
pgpArmorFileWriteCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPFile * 	(*fileOpen) (void *arg, 
					 void const *baseName, unsigned number),
	void *arg,
	void const *baseName)
{
	PGPPipeline *mod;
	ArmorFilContext *context;

	if (!head || !fileOpen)
		return NULL;

	context = (ArmorFilContext *)pgpContextMemAlloc(
		cdkContext, sizeof (*context), kPGPMemoryFlags_Clear );
	if ( IsNull( context ) )
		return NULL;
	mod = &context->pipe;

	mod->magic		= ARMORWRITEFILEMAGIC;
	mod->write		= Write;
	mod->flush		= Flush;
	mod->sizeAdvise	= SizeAdvise;
	mod->annotate	= Annotate;
	mod->teardown	= Teardown;
	mod->name		= "Armor File Write Module";
	mod->priv		= context;
	mod->cdkContext	= cdkContext;

	context->fileOpen	= fileOpen;
	context->arg		= arg;
	context->baseName	= baseName;

	*head = mod;
	return mod;
}

