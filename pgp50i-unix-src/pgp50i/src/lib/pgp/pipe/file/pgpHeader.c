/*
* pgpHeader.c -- write out a file header.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* currently, the header is: 10101000b \003 P G P == "\250\003PGP". In
* other words, this is an OLD literal-1 packet which PGP 2.6.2 supports
* for backwards compatibility with pre-release versions of PGP 2.0, but
* have never been generated by any released version of PGP. PGP 2.6.2
* will see this as a literal packet of type 'P', which is an unknown type,
* therefore complaining that it needs a newer version of PGP.
*
* PGPlib, on the other hand, will know to just ignore this completely.
*
* This module will wait for input. It will then check the first byte
* of the message. If it is an old-style PGP message (10xxxxxx) then it
* does nothing. If it is a new-style PGP message (11xxxxxx) then it will
* emit the header.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpHeader.c,v 1.3.2.1 1997/06/07 09:50:56 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpHeader.h"
#include "pgpPktByte.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"

#define HEADERMAGIC	 0x4eade640

static byte const pgpheader[] = { 168, 3, 'P', 'G', 'P' };

struct Context {
			byte header[sizeof(pgpheader)];
			size_t hdrlen;
			size_t offset;
			struct PgpPipeline *tail;
			byte done;
	};

static int
DoFlush (struct Context *context)
{
int error = 0;
size_t retlen;

			/* Try to flush anything that we have buffered */
			while (context->hdrlen) {
				 retlen = context->tail->write (context->tail,
							 		context->header+context->offset,
							 		context->hdrlen,
							 		&error);
					context->hdrlen -= retlen;
					context->offset += retlen;
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
			pgpAssert (myself->magic == HEADERMAGIC);

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

pgpAssert (myself);
pgpAssert (myself->magic == HEADERMAGIC);
pgpAssert (error);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

if (!context->done) {
			if (!size)
				return 0;

if (IS_NEW_PKTBYTE (*buf))
				switch (NEW_PKTBYTE_TYPE (*buf)) {
					/* XXX: These should match pgpFileType.c */
				case PKTBYTE_ESK:
				case PKTBYTE_SIG:
				case PKTBYTE_CONVESK:
				case PKTBYTE_1PASSSIG:
				case PKTBYTE_SECKEY:
				case PKTBYTE_PUBKEY:
					case PKTBYTE_SECSUBKEY:
							case PKTBYTE_PUBSUBKEY:
							case PKTBYTE_COMPRESSED:
							case PKTBYTE_CONVENTIONAL:
							case PKTBYTE_LITERAL:
									memcpy (context->header, pgpheader,
										sizeof(pgpheader));
									context->hdrlen = sizeof (pgpheader);
									break;
							}

					context->done = 1;
			}

			*error = DoFlush (context);
			if (*error)
				 return 0;

			return context->tail->write (context->tail, buf, size, error);
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;
			int error;

pgpAssert (myself);
pgpAssert (myself->magic == HEADERMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

error = DoFlush (context);
if (error)
	return error;

return context->tail->annotate (context->tail, origin, type,
				string, size);
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Context *context;
			int error;

pgpAssert (myself);
pgpAssert (myself->magic == HEADERMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

/* Can't send the sizeAdvise until we've figured out who we are */
	
if (!context->done)
				 return 0;

			error = DoFlush (context);
			if (error)
				 return error;

			return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
	{
			struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == HEADERMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

			if (context->tail)
				 context->tail->teardown (context->tail);

			memset (context, 0, sizeof (*context));
			pgpMemFree (context);
			memset (myself, 0, sizeof (*myself));
			pgpMemFree (myself);
	}

struct PgpPipeline **
pgpHeaderCreate (struct PgpPipeline **head)
{
			struct PgpPipeline *mod;
			struct Context *context;

if (!head)
	return NULL;

context = (struct Context *)pgpMemAlloc (sizeof (*context));
if (!context)
	return NULL;
mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
if (!mod) {
	pgpMemFree (context);
	return NULL;
}

mod->magic = HEADERMAGIC;
mod->write = Write;
mod->flush = Flush;
mod->sizeAdvise = SizeAdvise;
mod->annotate = Annotate;
mod->teardown = Teardown;
mod->name = "File Header Module";
mod->priv = context;

memset (context, 0, sizeof (*context));

	context->tail = *head;
	*head = mod;
	return &context->tail;
}