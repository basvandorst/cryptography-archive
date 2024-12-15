/*
* pgpRndom.c -- a general RNG interface
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpRndom.c,v 1.6.2.1 1997/06/07 09:51:36 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpMem.h"
#include "pgpRndom.h"
#include "pgpRndPool.h"
#include "pgpUsuals.h"

/* X9.17 interface */

struct X9_17Context {
	struct PgpCfbContext *cfb;
	struct PgpRandomContext const *base_rc;
};

static void
randomX9_17AddBytes(void *priv, byte const *buf, unsigned len)
{
	struct X9_17Context *ctx = (struct X9_17Context *)priv;
	pgpCfbRandWash(ctx->cfb, buf, len);
}

static void
randomX9_17GetBytesEntropy(void *priv, byte *buf, unsigned len, unsigned bits)
	{
			struct X9_17Context *ctx = (struct X9_17Context *)priv;
			byte salt[sizeof ctx->cfb->iv];
			unsigned blocksize = ctx->cfb->cipher->cipher->blocksize;
			unsigned t;

			/* Charge the entropy to the base RNG */
			pgpRandomGetBytesEntropy(ctx->base_rc, buf, 0, bits);

			t = pgpCfbRandBytes(ctx->cfb, buf, len);
			len -= t;
			buf += t;

while (len) {
			pgpRandomGetBytesEntropy(ctx->base_rc, salt, blocksize, 0);
			pgpCfbRandCycle(ctx->cfb, salt);
			t = pgpCfbRandBytes(ctx->cfb, buf, len);
			len -= t;
			buf += t;
	}
}

static void
randomX9_17Stir(void *priv)
{
struct X9_17Context *ctx = (struct X9_17Context *)priv;
pgpCfbRandWash(ctx->cfb, (byte const *)"", 0);
pgpRandomStir(ctx->base_rc);
}

static void
randomX9_17Wipe(void *priv)
{
struct X9_17Context *ctx = (struct X9_17Context *)priv;
pgpCfbWipe(ctx->cfb);
}

struct PgpRandom X9_17Desc = {
	"X9.17-based cryptographic PRNG",
	randomX9_17AddBytes, randomX9_17GetBytesEntropy,
	randomX9_17Stir, randomX9_17Wipe
};

/* Forward reference */
static void randomX9_17Destroy (struct PgpRandomContext *rc);

struct PgpRandomContext *
pgpRandomCreateX9_17(struct PgpCipher const *c,
struct PgpRandomContext const *base)
{
struct PgpRandomContext *rc;
struct PgpCfbContext *cfb;
struct X9_17Context *ctx;
byte buf[32];

cfb = pgpCfbCreate(c);
if (!cfb)
	return 0;
rc = (struct PgpRandomContext *)pgpMemAlloc(sizeof(*rc));
if (!rc) {
	pgpCfbDestroy(cfb);
	return 0;
}
ctx = (struct X9_17Context *)pgpMemAlloc(sizeof(*ctx));
if (!ctx) {
	pgpCfbDestroy(cfb);
	pgpMemFree(rc);
	return 0;
}
/* Get a reasonable initial state */
pgpRandomGetBytesEntropy(base, buf, sizeof(buf), 0);
pgpCfbRandWash(cfb, buf, sizeof(buf));
memset(buf, 0, sizeof(buf));

rc->random = &X9_17Desc;
rc->priv = ctx;
rc->destroy = randomX9_17Destroy;
ctx->cfb = cfb;
ctx->base_rc = base;

	return rc;
}

/* The default: X9.17, with CAST5, based on the pgpRandomPool. */
struct PgpRandomContext *
pgpRandomCreate(void)
	{
			struct PgpCipher const *c;

			c = pgpCipherByNumber(PGP_CIPHER_CAST5);
			if (!c)
				 return 0;
			return pgpRandomCreateX9_17(c, &pgpRandomPool);
	}

/*
* Pseudo-random number generator based on the dummy pool (which
* always produces zeros). This is useful for public data where we
* just want uniqueness but don't need cryptographic strength, and where
* we don't want to draw down the randomness in the true random pool.
* An example would be generating the public prime in DSA key.
* Use pgpRandomAddBytes to seed it.
*/
struct PgpRandomContext *
pgpPseudoRandomCreate(void)
	{
			struct PgpCipher const *c;

			c = pgpCipherByNumber(PGP_CIPHER_CAST5);
			if (!c)
				 return 0;
			return pgpRandomCreateX9_17(c, &pgpDummyPool);
	}


/*
* The simple get-byte function... entropy charged is the number of
* bits read out.
*/
void
pgpRandomGetBytes(struct PgpRandomContext const *rc, byte *buf, unsigned len)
{
	pgpRandomGetBytesEntropy(rc, buf, len, 8*len);
}

static void
randomX9_17Destroy (struct PgpRandomContext *rc)
{
struct X9_17Context *ctx = (struct X9_17Context *)rc->priv;
struct PgpCfbContext *cfbp = ctx->cfb;

pgpCfbWipe(cfbp);
pgpCfbDestroy(cfbp);
memset(ctx, 0, sizeof(*ctx));
pgpMemFree(ctx);
memset(rc, 0, sizeof(*rc));
pgpMemFree(rc);
}


/*
* Generate a uniformly distributed random number from 0..range-1.
* range is limited to 65536.
*
* This is very careful to avoid all bias in its selection, without
* using too many input bytes. For a range of 256 or less, it uses
* one byte if possible, and an average of less then two, even for
* the worst-case range=129.
*/
unsigned
pgpRandomRange(struct PgpRandomContext const *rc, unsigned range)
{
unsigned d, r;
byte b[2];

if (range <= 1)
return 0;

if (range <= 256) {
			d = 256/range;
			do {
			pgpRandomGetBytes(rc, b, 1);
			r = b[0]/d;
			} while (r >= range);
		} else {
			d = (unsigned)(65536/range);
			do {
			pgpRandomGetBytes(rc, b, 2);
			r = ((unsigned)b[0] << 8 | b[1])/d;
			} while (r >= range);
					b[1] = 0;
		}
b[0] = 0;
return r;
}
