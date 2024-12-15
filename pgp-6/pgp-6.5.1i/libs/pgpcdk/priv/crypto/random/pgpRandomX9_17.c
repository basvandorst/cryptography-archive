/*
 * pgpRandomX9_17.c -- a general RNG interface
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpRandomX9_17.c,v 1.14 1998/06/18 19:17:19 heller Exp $
 */
#include "pgpConfig.h"

#include "pgpMem.h"
#include "pgpContext.h"
#include "pgpCFBPriv.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRandomX9_17.h"



/* X9.17 interface */

typedef struct X9_17Context
{
	PGPCFBContext *cfb;
	PGPRandomContext const *base_rc;
} X9_17Context;

static void
randomX9_17AddBytes(void *priv, PGPByte const *buf, unsigned len)
{
	X9_17Context *ctx = (X9_17Context *)priv;
	PGPCFBRandomWash(ctx->cfb, buf, len);
}

static void
randomX9_17GetBytesEntropy(
	void *priv,
	PGPByte *buf,
	unsigned len,
	unsigned bits)
{
	X9_17Context *ctx = (X9_17Context *)priv;
	PGPByte salt[ PGP_CFB_MAXBLOCKSIZE ];
	PGPUInt32	blocksize;
	PGPSize t;

	blocksize	= pgpCFBGetBlockSize( ctx->cfb );

	/* Charge the entropy to the base RNG */
	pgpRandomGetBytesEntropy(ctx->base_rc, buf, 0, bits);

	(void)PGPCFBGetRandom(ctx->cfb, len, buf, &t);
	len -= t;
	buf += t;

	while (len) {
		pgpRandomGetBytesEntropy(ctx->base_rc, salt, blocksize, 0);
		PGPCFBRandomCycle(ctx->cfb, salt);
		(void)PGPCFBGetRandom(ctx->cfb, len, buf, &t);
		len -= t;
		buf += t;
	}
}

static void
randomX9_17Stir(void *priv)
{
	X9_17Context *ctx = (X9_17Context *)priv;
	PGPCFBRandomWash(ctx->cfb, (PGPByte const *)"", 0);
	pgpRandomStir(ctx->base_rc);
}

PGPRandomVTBL X9_17Desc = {
	"X9.17-based cryptographic PRNG",
	randomX9_17AddBytes, randomX9_17GetBytesEntropy,
	randomX9_17Stir
};

/* Forward reference */
static void randomX9_17Destroy (PGPRandomContext *rc);

	PGPRandomContext *
pgpRandomCreateX9_17(
	PGPContextRef				context,
	PGPCipherAlgorithm			algorithm,
	PGPRandomContext const *	base)
{
	PGPRandomContext *			rc;
	PGPCFBContextRef	cfb;
	X9_17Context *				ctx;
	PGPByte 					buf[32];
	PGPMemoryMgrRef				memoryMgr	= PGPGetContextMemoryMgr( context );

	pgpAssert( context );
	pgpAssert( base );

	cfb = pgpCFBCreate( memoryMgr, pgpCipherGetVTBL( algorithm ) );
	if (!cfb)
		return 0;
	rc = (PGPRandomContext *)pgpContextMemAlloc( context, sizeof(*rc),
												kPGPMemoryMgrFlags_Clear );
	if (!rc) {
		PGPFreeCFBContext( cfb );
		return 0;
	}

	ctx = (X9_17Context *)pgpContextMemAlloc( context, sizeof(*ctx),
											 kPGPMemoryMgrFlags_Clear );
	if (!ctx) {
		PGPFreeCFBContext( cfb );
		pgpContextMemFree( context, rc );
		return 0;
	}

	/* Get a reasonable initial state */
	pgpRandomGetBytesEntropy( base, buf, sizeof(buf), 0 );
	PGPCFBRandomWash( cfb, buf, sizeof(buf) );
	pgpClearMemory( buf, sizeof(buf) );

	rc->vtbl = &X9_17Desc;
	rc->priv = ctx;
	rc->context	= context;
	rc->destroy = randomX9_17Destroy;
	ctx->cfb = cfb;
	ctx->base_rc = base;

	return rc;
}

/* The default: X9.17, with CAST5, based on the pgpRandomPool. */
	PGPRandomContext *
pgpRandomCreate(PGPContextRef context)
{
	PGPRandomContext * randomContext;
	
	pgpAssert( context );

	randomContext	= pgpContextGetGlobalPoolRandomContext( context );
	pgpAssert( randomContext );

	return pgpRandomCreateX9_17( context, kPGPCipherAlgorithm_CAST5,
								 randomContext );
}

/*
 * Pseudo-random number generator based on the dummy pool (which
 * always produces zeros).  This is useful for public data where we
 * just want uniqueness but don't need cryptographic strength, and where
 * we don't want to draw down the randomness in the true random pool.
 * An example would be generating the public prime in DSA key.
 * Use pgpRandomAddBytes to seed it.
 *
 * Note that you need to seed this with some really random data
 * (using pgpRandomAddBytes) or it will generate the same data every time.
 *
 * DO NOT USE THIS unless you know what you're doing.
 */
	PGPRandomContext *
pgpPseudoRandomCreate(PGPContextRef context)
{
	PGPRandomContext * randomContext;
	
	pgpAssert( context );

	randomContext	= pgpContextGetDummyRandomContext( context );
	pgpAssert( randomContext );

	return pgpRandomCreateX9_17( context, kPGPCipherAlgorithm_CAST5,
								 randomContext );
}


/*
 * The simple get-byte function... entropy charged is the number of
 * bits read out.
 */
void
pgpRandomGetBytes(
	PGPRandomContext const *rc,
	void *					buf,
	PGPSize					len)
{
	pgpRandomGetBytesEntropy(rc, (PGPByte *) buf, len, 8*len);
}

static void
randomX9_17Destroy (PGPRandomContext *rc)
{
	X9_17Context *				ctx = (X9_17Context *)rc->priv;
	PGPCFBContextRef	cfbp = ctx->cfb;
	PGPContextRef				cdkContext;

	pgpAssertAddrValid( rc, PGPRandomContext );
	cdkContext	= rc->context;

	pgpCFBWipe( cfbp );
	PGPFreeCFBContext( cfbp );

	pgpClearMemory( ctx, sizeof(*ctx) );
	pgpContextMemFree( cdkContext, ctx );

	pgpClearMemory( rc, sizeof(*rc) );
	pgpContextMemFree( cdkContext, rc );
}


/*
 * Generate a uniformly distributed random number from 0..range-1.
 * range is limited to 65536.
 *
 * This is very careful to avoid all bias in its selection, without
 * using too many input bytes.  For a range of 256 or less, it uses
 * one byte if possible, and an average of less then two, even for
 * the worst-case range=129.
 */
	PGPUInt32
pgpRandomRange(
	PGPRandomContext const *	rc,
	PGPUInt32					range)
{
    PGPUInt32		d,
					r;
	PGPByte			b[2];

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



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
