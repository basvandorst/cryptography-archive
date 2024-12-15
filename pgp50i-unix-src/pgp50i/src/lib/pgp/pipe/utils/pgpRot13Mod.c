/*
* pgpRot13Mod.c -- A pipeline module to rotate by 13 characters. A stupid
* encryption/decryption link for PGP!
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRot13Mod.c,v 1.2.2.1 1997/06/07 09:51:18 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpPipeline.h"
#include "pgpRot13Mod.h"

#define ROT13MAGIC	0x13263952

struct Rot13Context {
			byte buffer[BUFSIZ];
			byte *bufptr;
			size_t buflen;
			struct PgpPipeline *tail;
	};

static size_t
rot13 (byte const *input, size_t inlen, byte *output, size_t outlen)
	{
			size_t len = 0;
			char c;

while (inlen && outlen) {
			c = input[len];
			output[len++] = (((c >= 'a' && c <= 'm') ||
				 		(c >= 'A' && c <= 'M')) ? c + 13 :
				 		((c >= 'n' && c <= 'z') ||
				 		(c >= 'N' && c <= 'Z')) ? c - 13 :
				 		c);
			inlen--;
			outlen--;
	}
return len;
}

static int
DoFlush (struct Rot13Context *context)
	{
			size_t retlen;
			int error = 0;

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

	static int
Flush (struct PgpPipeline *myself)
	{
			struct Rot13Context *context;
			int error;

			pgpAssert (myself);
			pgpAssert (myself->magic == ROT13MAGIC);

			context = myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

		error = DoFlush (context);
			if (error)
				 return error;

		return context->tail->flush (context->tail);
}

static size_t
rot13Write (struct PgpPipeline *myself, byte const *buf, size_t size,
			int *error)
	{
			struct Rot13Context *context;
			int myerror = 0;
			size_t retlen, written = 0;

			pgpAssert (myself);
			pgpAssert (myself->magic == ROT13MAGIC);

			context = myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

		do {
				myerror = DoFlush (context);
					if (myerror) {
							if (error)
								 *error = myerror;
							return written;
					}

					/*
					* Now that we dont have anything buffered, bring in more
					* data from the passed-in buffer, rot13 it, and buffer
					* that to write out.
					*/
context->bufptr = context->buffer;
context->buflen = rot13 (buf, size, context->buffer, BUFSIZ);
buf += context->buflen;
size -= context->buflen;
written += context->buflen;

} while (context->buflen > 0);
/* Continue until we have nothing buffered */

return written;
}

static int
rot13Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin,
			int type, byte const *string, size_t size)
	{
		struct Rot13Context *context;
		int error = 0;

		pgpAssert (myself);
		pgpAssert (myself->magic == ROT13MAGIC);

		context = myself->priv;
		pgpAssert (context);
		pgpAssert (context->tail);

		error = DoFlush (context);
		if (error)
			return error;

		return context->tail->annotate (context->tail, origin, type,
						string, size);
	}

static int
rot13SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
			struct Rot13Context *context;
			int error = 0;

pgpAssert (myself);
pgpAssert (myself->magic == ROT13MAGIC);

context = myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

error = DoFlush (context);
if (error)
	return error;

return context->tail->sizeAdvise (context->tail, bytes);
}

static void
rot13Teardown (struct PgpPipeline *myself)
{
	struct Rot13Context *context;

			pgpAssert (myself);
			pgpAssert (myself->magic == ROT13MAGIC);

			context = myself->priv;
			pgpAssert (context);

		if (context->tail)
				 context->tail->teardown (context->tail);

			memset (context, 0, sizeof (*context));
			memFree (context);
			memset (myself, 0, sizeof (*myself));
			memFree (myself);
	}

struct PgpPipeline **
rot13Create (struct PgpPipeline **head)
	{
			struct PgpPipeline *mod;
			struct Rot13Context *context;

			if (!head)
				 return NULL;

			context = memAlloc (sizeof (*context));
			if (!context)
				 return NULL;
			mod = memAlloc (sizeof (*mod));
			if (!mod){
				 memFree (context);
			return NULL;
			}

			mod->magic = ROT13MAGIC;
			mod->write = rot13Write;
			mod->write = Flush;
			mod->sizeAdvise = rot13SizeAdvise;
			mod->annotate = rot13Annotate;
			mod->teardown = rot13Teardown;
			mod->name = "Rot 13 Module";
			mod->priv = context;

			memset (context, 0, sizeof (*context));
			context->bufptr = context->buffer;

			context->tail = *head;
			*head = mod;
			return &context->tail;
	}
