/*
 * $Id: pgpPKEMod.c,v 1.26 1997/09/18 01:35:19 lloyd Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpMakePKE.h"
#include "pgpPKEMod.h"
#include "pgpPktByte.h"
#include "pgpFIFO.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpContext.h"

#define PKEMODMAGIC	0x90b11c47

typedef struct PKEModContext {
	PGPPipeline		pipe;
	
	PGPByte *key;
	size_t keylen;
	PGPByte *buf;
	size_t buflen;
	PgpVersion version;
	PGPPipeline *tail;
	DEBUG_STRUCT_CONSTRUCTOR( PKEModContext )
} PKEModContext;

static PGPError
Flush (PGPPipeline *myself)
{
	PKEModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	return context->tail->flush (context->tail);
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	PKEModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);
	pgpAssert (error);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	/* Once data is written, clear out the key! */
	if (context->key) {
		pgpClearMemory (context->key, context->keylen);
		pgpContextMemFree( cdkContext, context->key);
		context->key = NULL;
		if (context->buf) {
			pgpClearMemory (context->buf, context->buflen);
			pgpContextMemFree( cdkContext, context->buf);
			context->buf = NULL;
			context->buflen = 0;
		}
	}

	return context->tail->write (context->tail, buf, size, error);
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	PKEModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	return context->tail->annotate (context->tail, origin, type,
					string, size);
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	PKEModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	return context->tail->sizeAdvise (context->tail, bytes);
}

static PGPError
Teardown (PGPPipeline *myself)
{
	PKEModContext *context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	if (context->key) {
		pgpClearMemory (context->key, context->keylen);
		pgpContextMemFree( cdkContext, context->key);
		context->key = NULL;
	}
	if (context->buf) {
		pgpClearMemory (context->buf, context->buflen);
		pgpContextMemFree( cdkContext, context->buf);
		context->buf = NULL;
		context->buflen = 0;
	}
	pgpClearMemory( context,  sizeof (*context));
	pgpContextMemFree( cdkContext, context);
	
	return kPGPError_NoErr;
}

/*
 * This adds a new PKE packet to the head of the message.  If
 * context->key is NULL, return an error.  Also, return an error if
 * what we buffer is not as much as we thought we'd written
 */
int
pgpPkeAddKey (PGPPipeline *myself, PGPPubKey const *pubkey,
	PGPRandomContext const *rng)
{
	PKEModContext *context;
	size_t size;
	int i;
	PGPError	err	= kPGPError_NoErr;
	PGPByte *		p	= NULL;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);

	context = (PKEModContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	size = makePkeMaxSize(pubkey, context->version)+3;
	if (context->buflen < size) {
		if( IsNull( context->buf ) ) {
			context->buf = (PGPByte *)pgpContextMemAlloc( cdkContext, size,
											   0 );
			if( IsNull( context->buf ) )
				err = kPGPError_OutOfMemory;
		} else {
			err = pgpContextMemRealloc( cdkContext,
					(void **)&context->buf, size, 0 );
		}
		if( IsPGPError( err ) ) {
			return err;
		}
		context->buflen = size;
	}

	i = makePke(context->buf+3, pubkey, rng,
	            context->key, context->keylen, context->version);
	if (i < 0)
		return i;

	if (context->version > PGPVERSION_2_6) {
		if (PKTLEN_ONE_BYTE(i)) {
			context->buf[1] = PKTBYTE_BUILD_NEW(PKTBYTE_ESK);
			context->buf[2] = PKTLEN_1BYTE(i);
			p = context->buf+1;
			size = (size_t)i + 2;
		} else {
			context->buf[0] = PKTBYTE_BUILD_NEW(PKTBYTE_ESK);
			context->buf[1] = PKTLEN_BYTE0(i);
			context->buf[2] = PKTLEN_BYTE1(i);
			p = context->buf;
			size = (size_t)i + 3;
		}
	} else {
#if 0
/* PGP 2.X wants to see 2 byte length field */
		if (i < 256) {
			context->buf[1] = PKTBYTE_BUILD (PKTBYTE_ESK, 0);
			context->buf[2] = (PGPByte)i;
			p = context->buf+1;
			size = (size_t)i + 2;
		} else {}
#endif
		context->buf[0] = PKTBYTE_BUILD (PKTBYTE_ESK, 1);
		context->buf[1] = (PGPByte)(i >> 8);
		context->buf[2] = (PGPByte)i;
		p = context->buf;
		size = (size_t)i + 3;
	}

	pgpAssert( IsntNull( p ) );
	if (pgpJoinBuffer (context->tail, p, size) != size)
		return kPGPError_OutOfMemory;

	return 0;
}

PGPPipeline **
pgpPkeCreate (
	PGPContextRef	cdkContext,
	PGPPipeline **	head,
	PgpVersion		version,
	PGPByte const *	key,
	size_t			keylen)
{
	PGPPipeline *mod, *joinhead = NULL, **tail;
	PKEModContext *context;
	PGPByte *buf;

	pgpAssert (key);

	if (!head)
		return NULL;

	context = (PKEModContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryFlags_Clear);
	if (!context)
		return NULL;
	mod = &context->pipe;
	
	tail = pgpJoinCreate ( cdkContext, &joinhead, &pgpByteFifoDesc);
	if (!tail) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	buf = (PGPByte *)
		pgpContextMemAlloc( cdkContext, keylen, kPGPMemoryFlags_Clear);
	if (!buf) {
		pgpContextMemFree( cdkContext, context);
		joinhead->teardown (joinhead);
		return NULL;
	}
	memcpy (buf, key, keylen);

	mod->magic = PKEMODMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "PKE Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->key = buf;
	context->keylen = keylen;
	context->version = version;

	context->tail = joinhead;
	*tail = *head;
	*head = mod;
	return tail;
}
