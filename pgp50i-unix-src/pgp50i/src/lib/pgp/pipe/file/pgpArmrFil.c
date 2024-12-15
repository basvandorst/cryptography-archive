/*
* pgpArmrFil.c -- an Armor File Write module. It watches for MULTIARMOR
*	annotation scopes and will open multiple files if it occurs.
*	Otherwise, it will only a single file when data is first written.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpArmrFil.c,v 1.4.2.1 1997/06/07 09:50:52 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpArmrFil.h"
#include "pgpAnnotate.h"
#include "pgpFileMod.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpFile.h"
#include "pgpPipeline.h"

#define ARMORWRITEFILEMAGIC	 0xa434f11e

struct Context {
			struct PgpPipeline *tail;
			struct PgpFile * (*fileOpen) (void *arg,
				 			void const *baseName, unsigned number);
		void *arg;
			void const *baseName;
			int scope_depth;
	};

static int
Flush (struct PgpPipeline *myself)
	{
			struct Context *context;

			pgpAssert (myself);
			pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert (context);
			if (!context->tail)
				 return 0;

			return context->tail->flush (context->tail);
	}

static int
openFile (struct Context *context, unsigned num)
{
	struct PgpPipeline *tail = NULL;
	struct PgpFile *fp;
		int error;

			pgpAssert (!context->tail);

			fp = context->fileOpen (context->arg, context->baseName, num);
			if (!fp)
				 return PGPERR_NO_FILE;
		
			if (!pgpFileWriteCreate (&tail, fp, 1)) {
					error = pgpFileClose (fp);
					if (error)
						return error;
					return PGPERR_NOMEM;
			}
			context->tail = tail;
			return 0;
	}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
			struct Context *context;

			pgpAssert (myself);
			pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);
			pgpAssert (error);

			context = (struct Context *)myself->priv;
			pgpAssert (context);

			if (!context->tail) {
				 *error = openFile (context, 0);
				 if (*error)
				 	return 0;
			}		

			return context->tail->write (context->tail, buf, size, error);
	}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;
			int error = 0;
			unsigned num;

			(void)origin;	/* Avoid warning */
			(void)size;
			pgpAssert (myself);
			pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

			context = (struct Context *)myself->priv;
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

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
if (!context->tail)
	return 0;

return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
	{
			struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == ARMORWRITEFILEMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (context->tail)
	context->tail->teardown (context->tail);

memset (context, 0, sizeof (*context));
pgpMemFree (context);
memset (myself, 0, sizeof (*myself));
pgpMemFree (myself);
}

struct PgpPipeline *
pgpArmorFileWriteCreate (struct PgpPipeline **head,
							struct PgpFile * (*fileOpen) (void *arg,
								 			void const *baseName,
								 			unsigned number),
							void *arg,
							void const *baseName)
	{
			struct PgpPipeline *mod;
			struct Context *context;

			if (!head || !fileOpen)
				 return NULL;

			context = (struct Context *)pgpMemAlloc (sizeof (*context));
			if (!context)
				 return NULL;
			mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
			if (!mod) {
				 pgpMemFree (context);
				 return NULL;
			}

			mod->magic = ARMORWRITEFILEMAGIC;
			mod->write = Write;
			mod->flush = Flush;
			mod->sizeAdvise = SizeAdvise;
			mod->annotate = Annotate;
			mod->teardown = Teardown;
			mod->name = "Armor File Write Module";
			mod->priv = context;

			memset (context, 0, sizeof (*context));
			context->fileOpen = fileOpen;
			context->arg = arg;
			context->baseName = baseName;

			*head = mod;
			return mod;
	}
