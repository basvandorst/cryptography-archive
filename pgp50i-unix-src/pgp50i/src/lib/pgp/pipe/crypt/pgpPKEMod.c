/*
* pgpPKEMod.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpPKEMod.c,v 1.2.2.1 1997/06/07 09:50:49 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpMakePKE.h"
#include "pgpPKEMod.h"
#include "pgpPktByte.h"
#include "pgpFIFO.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"

#define PKEMODMAGIC	 0x90b11c47

struct Context {
			byte *key;
			size_t keylen;
			byte *buf;
			size_t buflen;
			PgpVersion version;
			struct PgpPipeline *tail;
	};

static int
Flush (struct PgpPipeline *myself)
{
			struct Context *context;

			pgpAssert (myself);
			pgpAssert (myself->magic == PKEMODMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

			return context->tail->flush (context->tail);
	}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	struct Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == PKEMODMAGIC);
	pgpAssert (error);

			context = (struct Context *)myself->priv;
			pgpAssert (context);
			pgpAssert (context->tail);

			/* Once data is written, clear out the key! */
			if (context->key) {
					memset (context->key, 0, context->keylen);
					pgpMemFree (context->key);
					context->key = NULL;
					if (context->buf) {
							memset (context->buf, 0, context->buflen);
							pgpMemFree (context->buf);
							context->buf = NULL;
							context->buflen = 0;
					}
			}

			return context->tail->write (context->tail, buf, size, error);
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
			byte const *string, size_t size)
	{
			struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == PKEMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

return context->tail->annotate (context->tail, origin, type,
				string, size);
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
			struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == PKEMODMAGIC);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->tail);

return context->tail->sizeAdvise (context->tail, bytes);
}

static void
Teardown (struct PgpPipeline *myself)
{
	struct Context *context;

			pgpAssert (myself);
			pgpAssert (myself->magic == PKEMODMAGIC);

			context = (struct Context *)myself->priv;
			pgpAssert (context);

			if (context->tail)
				 context->tail->teardown (context->tail);

			if (context->key) {
				 memset (context->key, 0, context->keylen);
				 pgpMemFree (context->key);
				 context->key = NULL;
			}
			if (context->buf) {
					memset (context->buf, 0, context->buflen);
					pgpMemFree (context->buf);
					context->buf = NULL;
					context->buflen = 0;
			}
			memset (context, 0, sizeof (*context));
			pgpMemFree (context);
			memset (myself, 0, sizeof (*myself));
			pgpMemFree (myself);
	}

/*
 * This adds a new PKE packet to the head of the message. If
 * context->key is NULL, return an error. Also, return an error if
 * what we buffer is not as much as we thought we'd written
 */
int
pgpPkeAddKey (struct PgpPipeline *myself, struct PgpPubKey const *pubkey,
			struct PgpRandomContext const *rng)
	{
		struct Context *context;
		byte *p;
		size_t size;
		int i;

		pgpAssert (myself);
		pgpAssert (myself->magic == PKEMODMAGIC);

		context = (struct Context *)myself->priv;
		pgpAssert (context);
		pgpAssert (context->tail);

		size = makePkeMaxSize(pubkey, context->version)+3;
		if (context->buflen < size) {
				p = (byte *)pgpMemRealloc(context->buf, size);
				if (!p)
					return PGPERR_NOMEM;
				context->buf = p;
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
					context->buf[2] = (byte)i;
					p = context->buf+1;
					size = (size_t)i + 2;
				} else {}
#endif
				context->buf[0] = PKTBYTE_BUILD (PKTBYTE_ESK, 1);
				context->buf[1] = (byte)(i >> 8);
				context->buf[2] = (byte)i;
				p = context->buf;
				size = (size_t)i + 3;
		}

			if (pgpJoinBuffer (context->tail, p, size) != size)
				 return PGPERR_NOMEM;

			return 0;
}

struct PgpPipeline **
pgpPkeCreate (struct PgpPipeline **head, PgpVersion version, byte const *key,
			size_t keylen)
	{
			struct PgpPipeline *mod, *joinhead = NULL, **tail;
			struct Context *context;
			byte *buf;

			pgpAssert (key);

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
			tail = pgpJoinCreate (&joinhead, &pgpByteFifoDesc);
			if (!tail) {
				 pgpMemFree (context);
				 pgpMemFree (mod);
				 return NULL;
			}
			buf = (byte *)pgpMemAlloc (keylen);
			if (!buf) {
					pgpMemFree (context);
					pgpMemFree (mod);
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

			memset (context, 0, sizeof (*context));
			context->key = buf;
			context->keylen = keylen;
			context->version = version;

			context->tail = joinhead;
			*tail = *head;
			*head = mod;
			return tail;
	}
