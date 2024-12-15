/*
* pgpDefMod.c -- Deflate (compression) module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpDefMod.c,v 1.3.2.1 1997/06/07 09:51:07 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpByteFIFO.h"
#include "pgpDefMod.h"
#include "pgpZip.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"

#define DEFMODMAGIC 0x0def1a1e

static int defModLock = 0;

struct Context {
		struct PgpFifoContext *fifo;
		struct PgpPipeline *tail;
		int finished;
		int scope_depth;
	};

static int
DoFlush (struct Context *context)
	{
		int error = 0;
		byte const *ptr;
		unsigned len;
		size_t retlen;

		/* Try to flush anything that we have buffered */
		bi_flush();
		ptr = byteFifoPeek (context->fifo, &len);
		while (len) {
			retlen = context->tail->write (context->tail, ptr, len,
			   &error);
			byteFifoSeek (context->fifo, retlen);
			if (error)
			 return error;
			ptr = byteFifoPeek (context->fifo, &len);
		}
		return error;
	}

static int
Flush (struct PgpPipeline *myself)
{
		struct Context *context;
		int error;

		pgpAssert (myself);
		pgpAssert (myself->magic == DEFMODMAGIC);

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
	  pgpAssert (myself->magic == DEFMODMAGIC);
	  pgpAssert (error);

	  context = (struct Context *)myself->priv;
	  pgpAssert (context);
	  pgpAssert (context->tail);

	  zip_input ((char const *)buf, size);
	  *error = DoFlush (context);

	  return size;
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;
		int error;

		pgpAssert (myself);
		pgpAssert (myself->magic == DEFMODMAGIC);

		context = (struct Context *)myself->priv;
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
		pgpAssert (myself->magic == DEFMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert (context);
		pgpAssert (context->tail);

		if (bytes || context->scope_depth)
		 return 0;

		if (!context->finished) {
		 /* It is a Bad Thing to call zip_finish multiple times. */
		 zip_finish ();
		 context->finished = 1;
		}
		error = DoFlush (context);
		if (error)
		 return error;

		return context->tail->sizeAdvise (context->tail, 0);
	}

static void
Teardown (struct PgpPipeline *myself)
	{
		struct Context *context;

		pgpAssert (myself);
		pgpAssert (myself->magic == DEFMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (context->tail)
 context->tail->teardown (context->tail);

ct_free ();  /* Free code tree buffers */
lm_free ();  /* Free longest match buffers */
byteFifoDestroy (context->fifo);
memset (context, 0, sizeof (*context));
pgpMemFree (context);
memset (myself, 0, sizeof (*myself));
pgpMemFree (myself);
defModLock--;
}

struct PgpPipeline **
defModCreate (struct PgpPipeline **head, int quality)
{
struct PgpPipeline *mod;
struct Context *context;
struct PgpFifoContext *fifo;

if (!head)
 return NULL;

if (defModLock)
 return NULL;
defModLock++;

if (lm_init (quality))
 return NULL;
if (ct_init ()) {
 lm_free ();
 return NULL;
}
context = (struct Context *)pgpMemAlloc (sizeof (*context));
if (!context) {
 ct_free ();
 lm_free ();
 return NULL;
}
mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
if (!mod) {
		ct_free ();
		lm_free ();
		pgpMemFree (context);
		return NULL;
	}
fifo = byteFifoCreate ();
if (!fifo) {
		ct_free ();
		lm_free ();
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}

mod->magic = DEFMODMAGIC;
mod->write = Write;
mod->flush = Flush;
mod->sizeAdvise = SizeAdvise;
mod->annotate = Annotate;
mod->teardown = Teardown;
mod->name = "Deflation Module";
mod->priv = context;

memset (context, 0, sizeof (*context));
context->fifo = fifo;

bi_init (fifo);

context->tail = *head;
*head = mod;
return &context->tail;
	}
