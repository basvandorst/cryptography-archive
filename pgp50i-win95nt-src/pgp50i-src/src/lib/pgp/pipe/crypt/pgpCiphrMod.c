/*
* pgpCiphrMod.c -- A module to perform Block Cipher encryption and Decryption
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCiphrMod.c,v 1.6.2.2 1997/06/07 09:50:47 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpAddHdr.h"
#include "pgpCiphrMod.h"
#include "pgpPktByte.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"

#if (BUFSIZ < 16384) && (MACINTOSH || WIN32)
#define kPGPCipherModBufSize	16384
#else
#define kPGPCipherModBufSize	BUFSIZ
#endif

#define CIPHERMODMAGIC	0x0c1fec0de
#define CIPHERMOD_DECRYPT 0
#define CIPHERMOD_ENCRYPT 1

struct Context {
	byte buffer[kPGPCipherModBufSize];
	byte *bufptr;
	size_t buflen;
	struct PgpPipeline *tail;
	struct PgpCfbContext *cfb;
	int (*progress)();
	byte encrypt;
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

static int
Flush (struct PgpPipeline *myself)
{
	struct Context *context;
	int error;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

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
	size_t written = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);
	pgpAssert (error);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	do {
		*error = DoFlush (context);
		if (*error)
			return written;

				/*
		* Now that we dont have anything buffered, bring in more
		* data from the passed-in buffer, process it, and buffer
		* that to write out.
				*/
		context->bufptr = context->buffer;
		context->buflen = min (size, sizeof (context->buffer));

		/* Tell user how many bytes we're doing, allow him to interrupt */
		if (context->progress && context->buflen) {
			if (context->progress(context->buflen) < 0) {
				/* User requested interruption */
				*error = PGPERR_INTERRUPTED;
				return written;
			}
		}

		if (context->encrypt)
			pgpCfbEncrypt (context->cfb, (byte *)buf,
				context->buffer, context->buflen);
		else
			pgpCfbDecrypt (context->cfb, (byte *)buf,
				context->buffer, context->buflen);
		buf += context->buflen;
		size -= context->buflen;
		written += context->buflen;

	} while (context->buflen > 0);
	/* Continue until we have nothing buffered */

	return written;	
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	byte const *string, size_t size)
{
	struct Context *context;
	int error;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

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
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

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
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpCfbDestroy (context->cfb);
	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline **
pgpCipherModDecryptCreate (struct PgpPipeline **head,
			struct PgpCfbContext *cfb, struct PgpEnv const *env)
{
	struct PgpPipeline *mod;
	struct Context *context;

	pgpAssert (cfb);

	if (!head) {
		pgpCfbDestroy (cfb);
		return NULL;
	}

	context = (struct Context *)pgpMemAlloc (sizeof (*context));
	if (!context) {
		pgpCfbDestroy (cfb);
		return NULL;
	}
	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
	if (!mod){
		pgpCfbDestroy (cfb);
		pgpMemFree (context);
		return NULL;
	}

	mod->magic = CIPHERMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Cipher Decryption Module";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->bufptr = context->buffer;
	context->cfb = cfb;
	context->encrypt = CIPHERMOD_DECRYPT;
	context->progress = pgpenvGetPointer (env, PGPENV_ENCRYPTIONCALLBACK,
		NULL, NULL);

	context->tail = *head;
	*head = mod;
	return &context->tail;
}

struct PgpPipeline **
pgpCipherModEncryptCreate (struct PgpPipeline **head, PgpVersion version,
			struct PgpFifoDesc const *fd,
			struct PgpCfbContext *cfb,
			byte const iv[IVLEN-2], struct PgpEnv const *env)
{
	struct PgpPipeline *mod, **tail;
	struct Context *context;
	byte enc_iv[IVLEN];

	pgpAssert (cfb);

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

	mod->magic = CIPHERMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Cipher Encryption Module";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->bufptr = context->buffer;
	context->cfb = cfb;
	context->encrypt = CIPHERMOD_ENCRYPT;
	context->progress = pgpenvGetPointer (env, PGPENV_ENCRYPTIONCALLBACK,
		NULL, NULL);

	/* Splice in the module */
	context->tail = *head;
	tail = &context->tail;

	/* Created the encrypted IV */
	memcpy (enc_iv, iv, IVLEN-2);
	memcpy (enc_iv+IVLEN-2, enc_iv+IVLEN-4, 2);
	pgpCfbEncrypt (cfb, enc_iv, enc_iv, IVLEN);
	pgpCfbSync (cfb);

	tail = pgpAddHeaderCreate (tail, version, fd, PKTBYTE_CONVENTIONAL, 0,
				enc_iv, IVLEN);
	
	memset (enc_iv, 0, IVLEN);
	if (!tail) {
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}

	*head = mod;
	return tail;
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
