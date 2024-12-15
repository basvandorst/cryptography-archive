/*
 * pgpCiphrMod.c -- A module to perform Block Cipher encryption and Decryption
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCiphrMod.c,v 1.35 1999/04/14 18:51:26 hal Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpAddHdr.h"
#include "pgpAnnotate.h"
#include "pgpCFBPriv.h"
#include "pgpCiphrMod.h"
#include "pgpPktByte.h"
#include "pgpCFB.h"
#include "pgpHash.h"
#include "pgpHashPriv.h"
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
	PGPByte *bufput;			/* Pointer for putting data into buffer */
	PGPByte *buftake;			/* Pointer for taking data out of buffer */
	PGPSize buflen;				/* Amount of data in buffer */
	PGPSize buftaillen;			/* Amount of data to retain in buffer */
	PGPHashContextRef hash;		/* Calculate hash of plaintext if set */
	PGPPipeline *tail;
	PGPCFBContext *cfb;
	ProgressHook	progress;
	PGPByte encrypt;
	int scope_depth;
	DEBUG_STRUCT_CONSTRUCTOR( CiphrModContext )
} CiphrModContext;

static PGPError
DoFlush (CiphrModContext *context)
{
	PGPError	error = kPGPError_NoErr;
	PGPSize retlen;

	/* Try to flush anything that we have buffered */
	while (context->buflen > context->buftaillen) {
		PGPSize bufspace = context->buffer
						  + sizeof(context->buffer) - context->buftake;
		PGPSize buflen = pgpMin (context->buflen - context->buftaillen,
								bufspace);
		retlen = context->tail->write (context->tail,
					       context->buftake,
					       buflen,
					       &error);
		if (!context->encrypt && context->hash) {
			PGPContinueHash( context->hash, context->buftake, retlen );
		}
		context->buflen -= retlen;
		pgpClearMemory (context->buftake, retlen);
		context->buftake += retlen;
		if (context->buftake == context->buffer + sizeof(context->buffer))
			context->buftake = context->buffer;
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
	PGPSize written = 0;
	PGPSize newdatasize = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);
	pgpAssert (error);

	context = (CiphrModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	do {
		PGPSize bufspace;
		PGPByte *bufend;

		*error = DoFlush (context);
		if (*error)
			return written;

		/*
		 * Now that we dont have anything buffered, bring in more
		 * data from the passed-in buffer, process it, and buffer
		 * that to write out.
		 */
		if (context->buftake > context->bufput)
			bufend = context->buftake;
		else
			bufend = context->buffer + sizeof(context->buffer);
		bufspace = bufend - context->bufput;
		
		newdatasize = pgpMin (size, bufspace);
		if (newdatasize > sizeof(context->buffer)
						  - (context->buflen + context->buftaillen) )
			newdatasize = sizeof(context->buffer)
						  - (context->buflen + context->buftaillen);
		context->buflen += newdatasize;

		/* Tell user how many bytes we're doing, allow him to interrupt */
		if (context->progress && newdatasize)
		{
			if (context->progress(newdatasize) < 0)
			{
				/* User requested interruption */
				*error = kPGPError_UserAbort;
				return written;
			}
		}

		if (newdatasize > 0) {
			if (context->encrypt) {
				if (context->hash) {
					PGPContinueHash( context->hash, buf, newdatasize );
				}
				pgpCFBEncryptInternal (context->cfb, (PGPByte *)buf,
						   newdatasize, context->bufput );
			} else {
				/* Hashing is handled in DoFlush */
				pgpCFBDecryptInternal (context->cfb, (PGPByte *)buf,
						   newdatasize, context->bufput );
			}
			buf += newdatasize;
			size -= newdatasize;
			written += newdatasize;
			context->bufput += newdatasize;
			if (context->bufput == context->buffer+sizeof(context->buffer))
				context->bufput = context->buffer;
		}

	} while (newdatasize > 0);
	/* Continue until we have nothing buffered */

	return written;	
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	CiphrModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	switch (type) {
	case PGPANN_CIPHER_HASHSIZE:
		/* Return the hash size, or zero if we're not hashing.
		 * Should equal buftaillen, but we return the max in case they
		 * disagree so that it will report an error.
		 */
		{	PGPSize hashSize = 0;
			pgpAssert (string);
			pgpAssert (size == sizeof(hashSize));
			if (context->hash) {
				PGPGetHashSize( context->hash, &hashSize );
				hashSize = pgpMax (hashSize, context->buftaillen);
			}
			*(PGPSize *)string = hashSize;
			return( kPGPError_NoErr );
		}

	case PGPANN_CIPHER_TAILDATA:
		/* Return the buffer tail data (may be called more than once) */
		/* The only thing in the buffer must be the tail data */
		{	PGPSize buflen;
			pgpAssert (string);
			pgpAssert (size >= context->buftaillen);
			pgpAssert (context->buflen == context->buftaillen);
			buflen = context->buffer+sizeof(context->buffer)-context->buftake;
			buflen = pgpMin (buflen, context->buftaillen);
			pgpClearMemory ((PGPByte *)string, size);
			pgpCopyMemory (context->buftake, (PGPByte *)string, buflen);
			if (context->buftaillen > buflen) {
				/* Tail data wrapped around end of buffer */
				pgpCopyMemory (context->buffer, (PGPByte *)string+buflen,
							   context->buftaillen-buflen);
			}
			return( kPGPError_NoErr );
		}

	case PGPANN_CIPHER_HASHDATA:
		/* Return the hash value.  May be called more than once. */
		/* If not hashing, just zero the buffer */
		{	pgpAssert (string);
			pgpClearMemory ((PGPByte *)string, size);
			if (context->hash) {
				PGPSize hashSize;
				PGPHashContextRef hashCopy;
				PGPGetHashSize( context->hash, &hashSize );
				pgpAssert (size >= hashSize);
				PGPCopyHashContext(context->hash, &hashCopy);
				PGPFinalizeHash( hashCopy, (PGPByte *)string );
				PGPFreeHashContext( hashCopy );
			}
			return( kPGPError_NoErr );
		}
			
	default:
		;		/* do nothing */
	}

	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);

	return context->tail->annotate (context->tail, origin, type,
					string, size);
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	CiphrModContext *context;
	PGPError	error;
	PGPSize		hashSize;
	PGPByte		hashBuf[100];

	pgpAssert (myself);
	pgpAssert (myself->magic == CIPHERMODMAGIC);

	context = (CiphrModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	error = DoFlush (context);
	if (error)
		return error;

	if (context->scope_depth)
		return( kPGPError_NoErr );	/* Can't pass it through */

	if (bytes == 0) {
		/* Closing down */
		if (context->encrypt && context->hash) {
			PGPGetHashSize( context->hash, &hashSize );
			pgpAssert (hashSize <= sizeof(hashBuf));
			PGPFinalizeHash( context->hash, hashBuf );
			PGPFreeHashContext( context->hash );
			context->hash = NULL;
			Write( myself, hashBuf, hashSize, &error );
			if( IsPGPError( error ) )
				return error;
		}
	} else {
		if (context->encrypt && context->hash) {
			PGPGetHashSize( context->hash, &hashSize );
			bytes += hashSize;
		}
	}

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

	if (context->hash)
		PGPFreeHashContext( context->hash );

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
	PGPEnv const *env,
	PGPHashAlgorithm hashAlg,
	PGPSize hashSize)
{
	PGPPipeline *mod;
	CiphrModContext *context;

	pgpAssert (cfb);

	if (!head) {
		PGPFreeCFBContext (cfb);
		return NULL;
	}

	context = (CiphrModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
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

	context->bufput = context->buftake = context->buffer;
	context->cfb = cfb;
	if (hashAlg != kPGPHashAlgorithm_Invalid) {
		PGPHashVTBL const *vtbl = pgpHashByNumber ( hashAlg );
		PGPMemoryMgrRef mgr = PGPGetContextMemoryMgr( cdkContext );

		context->buftaillen = hashSize;
		if( IsntNull( vtbl ) ) {
			context->hash = pgpHashCreate ( mgr, vtbl );
			pgpAssert( IsntNull( context->hash ) );
		}
	}
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
	PGPByte const iv[MAXIVLEN], PGPEnv const *env)
{
	PGPPipeline *mod, **tail;
	CiphrModContext *context;
	PGPByte enc_iv[MAXIVLEN+2];
	PGPSize ivlen;

	pgpAssert (cfb);

	if (!head)
		return NULL;

	context = (CiphrModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear);
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

	context->bufput = context->buftake = context->buffer;
	context->cfb = cfb;
	context->encrypt = CIPHERMOD_ENCRYPT;
	context->progress	= (ProgressHook)
			pgpenvGetPointer (env, PGPENV_ENCRYPTIONCALLBACK, NULL );

	/* Splice in the module */
	context->tail = *head;
	tail = &context->tail;

	/* Created the IV, encrypted */
	ivlen = pgpCFBGetBlockSize( cfb );
	pgpAssert (ivlen <= MAXIVLEN);
	/* Copy IV, duplicate last two bytes */
	pgpCopyMemory( iv, enc_iv, ivlen );
	pgpCopyMemory( enc_iv+ivlen-2, enc_iv+ivlen, 2);
	pgpCFBEncryptInternal (cfb, enc_iv, ivlen+2, enc_iv);
#if 0
/* Leave in sync for now, openpgp list is uncertain */
	/* Only do special sync for back compatibility with small ciphers */
	if ( pgpCFBGetBlockSize( cfb ) == 8 )
#endif
		PGPCFBSync (cfb);
	tail = pgpAddHeaderCreate ( cdkContext, tail, version, fd,
								PKTBYTE_CONVENTIONAL, 0, enc_iv, ivlen+2 );


	pgpClearMemory (enc_iv, sizeof(enc_iv));
	if (!tail) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}

	*head = mod;
	return tail;
}
