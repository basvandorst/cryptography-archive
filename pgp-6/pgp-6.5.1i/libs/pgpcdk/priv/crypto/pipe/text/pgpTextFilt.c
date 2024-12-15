/*
 * pgpTextFilt.c -- filter text
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU> and Colin Plumb
 *
 * $Id: pgpTextFilt.c,v 1.23 1997/12/12 01:14:09 heller Exp $
 */
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpTextFilt.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpErrors.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPFiltBufSize		16384
#else
#define kPGPFiltBufSize		BUFSIZ
#endif

#define TEXTFILTMAGIC	0x1e81f111

typedef struct TextFiltContext {
	PGPPipeline	pipe;
	
	PGPByte buffer[kPGPFiltBufSize];
	PGPByte *bufptr;
	size_t buflen;
	PGPPipeline *tail;
	PGPByte const *map;	/* 256-entry mapping table */
	int crlf;		/* 0=nothing, 1=LF, 2=CRLF, 3=CR */
	int stripspace;
	size_t spaces;
	int ignorelf;
	DEBUG_STRUCT_CONSTRUCTOR( TextFiltContext )
} TextFiltContext;

static PGPError
DoFlush (TextFiltContext *context)
{
	PGPError	error = kPGPError_NoErr;
	size_t retlen;

	/* Try to flush anything that we have buffered */
	while (context->buflen) {
		retlen = context->tail->write (context->tail,
					       context->bufptr,
					       context->buflen,
					       &error);
		context->buflen -= retlen;
		pgpClearMemory (context->bufptr, retlen);
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
textFilt (TextFiltContext *context, PGPByte const *in, size_t inlen)
{
	size_t spaces = context->spaces;
	int ignorelf = context->ignorelf;
	PGPByte *out = context->bufptr+context->buflen;
	size_t outlen = context->buffer+sizeof(context->buffer)-out;
	size_t inlen0 = inlen;
	PGPByte c;

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
				if (context->crlf == kPGPLineEnd_CRLF) {
					if (outlen < 2)
						break;
					outlen--;
				}
				if (context->crlf & kPGPLineEnd_CR)
					*out++ = '\r';
				if (context->crlf & kPGPLineEnd_LF)
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
			int i = pgpMin (spaces, outlen);

			pgpFillMemory (out, i, ' ');
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

static PGPError
Flush (PGPPipeline *myself)
{
	TextFiltContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == TEXTFILTMAGIC);

	context = (TextFiltContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->flush (context->tail);
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	TextFiltContext *context;
	size_t written, retlen = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == TEXTFILTMAGIC);
	pgpAssert (error);

	context = (TextFiltContext *)myself->priv;
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

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	TextFiltContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == TEXTFILTMAGIC);

	context = (TextFiltContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->annotate (context->tail, origin, type,
					string, size);
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	TextFiltContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == TEXTFILTMAGIC);

	context = (TextFiltContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error || (bytes && (context->stripspace || context->crlf)))
		return error;

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	TextFiltContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == TEXTFILTMAGIC);

	context = (TextFiltContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpTextFiltCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPByte const *map,
	int stripspace,
	PGPLineEndType crlf)
{
	PGPPipeline *mod;
	TextFiltContext *context;

	pgpAssert (map);

	if (!head)
		return NULL;

	context = (TextFiltContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear );
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = TEXTFILTMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Text Filter Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->bufptr = context->buffer;
	context->map = map;
	context->stripspace = stripspace;
	context->crlf = crlf;

	context->tail = *head;
	*head = mod;
	return &context->tail;
}
