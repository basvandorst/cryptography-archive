/*
 * pgpCiphrMod.c -- A module to perform Block Cipher encryption and Decryption
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCiphrMod.c,v 1.28 1997/10/08 02:10:46 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpAddHdr.h"
#include "pgpCiphrMod.h"
#include "pgpPktByte.h"
#include "pgpCFB.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpUsuals.h"
#include "pgpContext.h"

#if (BUFSIZ < 16384) && (PGP_MACINTOSH || PGP_WIN32)
#define kPGPCipherModBufSize	16384
#else
#define kPGPCipherModBufSize	BUFSIZ
#endif

#define CIPHERMODMAGIC	0x0c1fec0de
#define CIPHERMOD_DECRYPT 0
#define CIPHERMOD_ENCRYPT 1

typedef PGPError	(*ProgressHook)( size_t len );

typedef struct CiphrModContext {
	PGPPipeline	pipe;
	
	PGPByte buffer[kPGPCipherModBufSize];
	PGPByte *bufptr;
	size_t buflen;
	PGPPipeline *tail;
	PGPCFBContext *cfb;
	ProgressHook	progress;
	PGPByte encrypt;
	DEBUG_STRUCT_CONSTRUCTOR( CiphrModContext )
} CiphrModContext;

static PGPError
DoFlush (CiphrModContext *context)
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

static PGPError
Flush (PGPPipeline *myself)
{
	CiphrModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
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
	CiphrModContext *context;
	size_t written = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);
	pgpAssert (error);

	context = (CiphrModContext *)myself->priv;
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
		context->buflen = pgpMin (size, sizeof (context->buffer));

		/* Tell user how many bytes we're doing, allow him to interrupt */
		if (context->progress && context->buflen)
		{
			if (context->progress(context->buflen) < 0)
			{
				/* User requested interruption */
				*error = kPGPError_UserAbort;
				return written;
			}
		}

		if (context->buflen > 0) {
			if (context->encrypt)
				PGPCFBEncrypt (context->cfb, (PGPByte *)buf,
						   context->buflen, context->buffer );
			else
				PGPCFBDecrypt (context->cfb, (PGPByte *)buf,
						   context->buflen, context->buffer );
		}
		buf += context->buflen;
		size -= context->buflen;
		written += context->buflen;

	} while (context->buflen > 0);
	/* Continue until we have nothing buffered */

	return written;	
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	CiphrModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
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
	CiphrModContext *context;
	PGPError	error;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	CiphrModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	PGPFreeCFBContext (context->cfb);
	
	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

PGPPipeline **
pgpCipherModDecryptCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **head,
	PGPCFBContext *cfb,
	PGPEnv const *env)
{
	PGPPipeline *mod;
	CiphrModContext *context;

	pgpAssert (cfb);

	if (!head) {
		PGPFreeCFBContext (cfb);
		return NULL;
	}

	context = (CiphrModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear);
	if (!context) {
		PGPFreeCFBContext (cfb);
		return NULL;
	}
	mod = &context->pipe;

	mod->magic = CIPHERMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Cipher Decryption Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->bufptr = context->buffer;
	context->cfb = cfb;
	context->encrypt = CIPHERMOD_DECRYPT;
	context->progress	= (ProgressHook)
			pgpenvGetPointer (env, PGPENV_ENCRYPTIONCALLBACK, NULL);

	context->tail = *head;
	*head = mod;
	return &context->tail;
}

PGPPipeline **
pgpCipherModEncryptCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **head, PgpVersion version,
	PGPFifoDesc const *fd,
	PGPCFBContext *cfb,
	PGPByte const iv[IVLEN-2], PGPEnv const *env)
{
	PGPPipeline *mod, **tail;
	CiphrModContext *context;
	PGPByte enc_iv[IVLEN];

	pgpAssert (cfb);

	if (!head)
		return NULL;

	context = (CiphrModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;

	mod->magic = CIPHERMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "Cipher Encryption Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->bufptr = context->buffer;
	context->cfb = cfb;
	context->encrypt = CIPHERMOD_ENCRYPT;
	context->progress	= (ProgressHook)
			pgpenvGetPointer (env, PGPENV_ENCRYPTIONCALLBACK, NULL );

	/* Splice in the module */
	context->tail = *head;
	tail = &context->tail;

	/* Created the encrypted IV */
	memcpy (enc_iv, iv, IVLEN-2);
	memcpy (enc_iv+IVLEN-2, enc_iv+IVLEN-4, 2);
	PGPCFBEncrypt (cfb, enc_iv, IVLEN, enc_iv);
	PGPCFBSync (cfb);

	tail = pgpAddHeaderCreate ( cdkContext, tail,
		version, fd, PKTBYTE_CONVENTIONAL, 0, enc_iv, IVLEN);
	
	pgpClearMemory (enc_iv, IVLEN);
	if (!tail) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}

	*head = mod;
	return tail;
}
