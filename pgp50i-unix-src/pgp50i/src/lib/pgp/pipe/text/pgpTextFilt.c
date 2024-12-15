/*
* pgpTextFilt.c -- filter text
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU> and Colin Plumb
*
* $Id: pgpTextFilt.c,v 1.4.2.1 1997/06/07 09:51:09 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpTextFilt.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPFiltBufSize 16384
#else
#define kPGPFiltBufSize BUFSIZ
#endif

#define TEXTFILTMAGIC 0x1e81f111

struct Context {
		byte buffer[kPGPFiltBufSize];
		byte *bufptr;
		size_t buflen;
		struct PgpPipeline *tail;
		byte const *map;	/* 256-entry mapping table */
		int crlf;	 /* 0=nothing, 1=LF, 2=CRLF, 3=CR */
		int stripspace;
		size_t spaces;
		int ignorelf;
};

static int
DoFlush (struct Context *context)
	{
		int error = 0;
		size_t retlen;

		/* Try to flush anything that we have buffered */
		while (context->buflen) {
			retlen = context->tail->write (context->tail,
			   context->bufptr,
			   context->buflen,
			   &error);
			context->buflen -= retlen;
			memset (context->bufptr, 0, retlen);
			context->bufptr += retlen;
			if (error)
		 	return error;
		}
		return error;
	}

/*
* Note: it is assumed that \r, \n and ' ' use the identity mapping.
* This will need fixing for EBCDIC.
*/
static size_t
textFilt (struct Context *context, byte const *in, size_t inlen)
	{
		size_t spaces = context->spaces;
		int ignorelf = context->ignorelf;
		byte *out = context->bufptr+context->buflen;
		size_t outlen = context->buffer+sizeof(context->buffer)-out;
		size_t inlen0 = inlen;
		byte c;

		pgpAssert (outlen);

		while (inlen && outlen) {
			c = *in;

			if (ignorelf) {
				ignorelf = 0;
				if (c == '\n') {
					in++;
					inlen--;
					continue;
				}
			}
			if (c == '\n' || c == '\r') {
				if (context->crlf) {
					if (context->crlf == PGP_TEXTFILT_CRLF) {
					 if (outlen < 2)
					  break;
					 outlen--;
					}
					if (context->crlf & PGP_TEXTFILT_CR)
					 *out++ = '\r';
					if (context->crlf & PGP_TEXTFILT_LF)
					 *out++ = '\n';
					outlen--;
					in++;
					inlen--;
					ignorelf = (c == '\r');
					spaces = 0;
					continue;
				}
				spaces = 0;
			}
			if (context->stripspace && c == ' ') {
				spaces++;
				in++;
				inlen--;
				continue;
			}
			if (spaces) {
				int i = min (spaces, outlen);

				memset (out, ' ', i);
				outlen -= i;
				spaces -= i;
				out += i;
				continue;
			}
			*out++ = context->map[c];
			in++;
			outlen--;
			inlen--;
		}

		context->spaces = spaces;
		context->ignorelf = ignorelf;
		context->buflen = out - context->bufptr;
		return inlen0 - inlen;
	}

static int
Flush (struct PgpPipeline *myself)
{
struct Context *context;
int error;

pgpAssert (myself);
pgpAssert (myself->magic == TEXTFILTMAGIC);

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
		size_t written, retlen = 0;

		pgpAssert (myself);
		pgpAssert (myself->magic == TEXTFILTMAGIC);
		pgpAssert (error);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

do {
 *error = DoFlush (context);
 if (*error)
			 return retlen;

			/*
			* Now that we dont have anything buffered, bring in more
			* data from the passed-in buffer, process it, and buffer
			* that to write out.
			*/
			context->bufptr = context->buffer;
			written = textFilt (context, buf, size);
			/* context->buflen is set by textFilt() */
			buf += written;
			size -= written;
			retlen += written;

		} while (context->buflen > 0);
		/* Continue until we have nothing buffered */

		return retlen;
	}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;
		int error;

		pgpAssert (myself);
		pgpAssert (myself->magic == TEXTFILTMAGIC);

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
pgpAssert (myself->magic == TEXTFILTMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

error = DoFlush (context);
if (error || (bytes && (context->stripspace || context->crlf)))
 return error;

  return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
	{
		struct Context *context;

		pgpAssert (myself);
		pgpAssert (myself->magic == TEXTFILTMAGIC);

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
pgpTextFiltCreate (struct PgpPipeline **head, byte const *map, int stripspace,
		 int crlf)
	{
		struct PgpPipeline *mod;
		struct Context *context;

		pgpAssert (map);

if (!head)
 return NULL;

context = (struct Context *)pgpMemAlloc (sizeof (*context));
if (!context)
 return NULL;
mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
if (!mod){
 pgpMemFree (context);
 return NULL;
}

		mod->magic = TEXTFILTMAGIC;
		mod->write = Write;
		mod->flush = Flush;
		mod->sizeAdvise = SizeAdvise;
		mod->annotate = Annotate;
		mod->teardown = Teardown;
		mod->name = "Text Filter Module";
		mod->priv = context;

		memset (context, 0, sizeof (*context));
		context->bufptr = context->buffer;
		context->map = map;
		context->stripspace = stripspace;
context->crlf = crlf;

context->tail = *head;
*head = mod;
return &context->tail;
}
