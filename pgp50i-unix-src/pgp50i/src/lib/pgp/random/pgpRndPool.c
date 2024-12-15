/*
* pgpRndPool.c -- True random number computation and storage
*
* Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Wrtten by Colin Plumb.
*
* This performs a strong hash on the data as it is fed into the
* pool, and the pool is again strongly hashed to produce the output
* of the random-number generator. This is overkill, as either step
* should produce high-quality numbers.
*
* $Id: pgpRndPool.c,v 1.8.2.1 1997/06/07 09:51:34 mhw Exp $
*/
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpRndPool.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

/*
* Accumulators to estimate entropy in the pool.
*/
static unsigned randBits = 0; /* Bits of entropy in pool */
static word32 randFrac = 0; /* Fraction of a bit of entropy in pool */
static unsigned randInBits = 0; /* Bits of entropy added to pool */

/* We import the following things from the MD5 code */
void MD5Transform(word32 *block, word32 const *key);
#define HashTransform MD5Transform
#define HASH_INWORDS 16
#define HASH_OUTWORDS 4

/*
* The pool must be a multiple of the hash input size and the hash
* output size. For MD5, these are 16 and 4 words, respectively,
* so the pool must be a multiple of 16 words. If the hash were
* SHA, it would have to be a multiple of 16 and of 5, or a multiple
* of 80 all together. (80 words is 2560 bits.)
*/
#define RANDPOOLWORDS 160	/* 5,120 bits */
#define RANDPOOLBITS (32*RANDPOOLWORDS)
#if RANDPOOLWORDS % HASH_OUTWORDS
#error RANDPOOLWORDS must be a multiple of HASH_OUTWORDS
#endif
#if RANDPOOLWORDS % HASH_INWORDS
#error RANDPOOLWORDS must be a multiple of HASH_INWORDS
#endif

/* Must be word-aligned, so make it words. Cast to bytes as needed. */
static word32 randPool[RANDPOOLWORDS];	/* Random pool */
static unsigned randPoolAddPos = 0;	 /* Position to encrypt into (words) */

static word32 randKey[HASH_INWORDS];	/* Random input buffer */
static unsigned randKeyAddPos = 0;	/* Position to add to (bytes) */

static word32 randOut[HASH_OUTWORDS];	/* Random output buffer */
static unsigned randOutGetPos = sizeof(randOut); /* Position to get from (b) */

/* Keeps track of when we need to do another hashing step */
static unsigned randHashCounter = 0;

static void
randPoolWipe(void *ctx)
	{
			(void)ctx;
			memset(randPool, 0, sizeof(randPool));
			memset(randKey, 0, sizeof(randKey));
			memset(randOut, 0, sizeof(randOut));
			randPoolAddPos = 0;
			randKeyAddPos = 0;
			randOutGetPos = sizeof(randOut);
	}

/*
* To mix input into the pool, we insert it into a key buffer and then
* use the key buffer to CBC-encrypt the pool. Actually, to make sure
* that the input (which includes such sensitive information as passphrase
* keystrokes) is not left in memory, we XOR it with the old contents of
* the key buffer along with some of the the data about to be encrypted.
* Since none of the disguising data depends on the byte, this does
* not destroy any entropy inherent in the input byte.
*
* To reduce burstiness in the timing, rather than wait until the key
* buffer is full to encrypt the entire pool, we update part of the key
* and then encrypt part of the pool. The randHashCounter keeps track
* of what's going on to ensure that a byte of key is not overwritten
* until it has participated in the encryption of every block of the pool.
*
* The pool contains RANDPOOLWORDS/HASH_OUTWORDS blocks. We must do
* that many hash-encrpytion steps before 4*HASH_INWORDS bytes of the
* key have been overwritten. So randHashCounter is incremented by
* RANDPOOLWORDS/HASH_OUTWORDS each time a byte is added, and when this
* exceeds 4*HASH_INWORDS, it is decreased by that amount and a hashing
* step is performed. It should be easy to see that that guarantees
* correct behaviour.
*
* The bits deposited need not have any particular distribution; the
* stirring operation transforms them to uniformly-distributed bits.
*/
static void
randPoolAddByte(byte b)
	{
			int i;
			unsigned pos;

			/* Mix new byte into pool */
			((byte *)randKey)[randKeyAddPos] ^= b;
			if (++randKeyAddPos == sizeof(randKey))
				 randKeyAddPos = 0;

		/*
		* If we've reached the end of a word, mask the next word with
		* a soon-to-be-overwritten byte of the pool.  With a 64-byte input
		* hash, this will use a different word for masking each of the
		* 16 words as long as there are at least 16 output blocks
		* (64 words at 4 words per output block, or 512 bits total) in the
		* pool. Just to be sure, let's enforce it...
		*/
#if HASH_INWORDS > RANDPOOLWORDS / HASH_OUTWORDS
#error Please make RANDPOOLWORDS bigger than HASH_INWORDS * HASH_OUTWORDS.
#endif
			if (randKeyAddPos % sizeof(word32) == 0)
				 *(word32 *)((byte *)randKey+randKeyAddPos) ^=
					randPool[randPoolAddPos];
			/* Do a hashing step if required */
			randHashCounter += RANDPOOLWORDS/HASH_OUTWORDS;
			while (randHashCounter >= 4*HASH_INWORDS) {
					randHashCounter -= 4*HASH_INWORDS;

/* CBC-encrypt the current block */
pos = randPoolAddPos;
HashTransform(randPool + pos, randKey);

/* Get the offset of the next block and XOR this one in */
randPoolAddPos = pos + HASH_OUTWORDS;
if (randPoolAddPos == RANDPOOLWORDS)
	randPoolAddPos = 0;
for (i = 0; i < HASH_OUTWORDS; i++)
	randPool[randPoolAddPos+i] ^= randPool[pos+i];
}
}

/*
* Make a deposit of information (entropy) into the pool.
*/
static void
randPoolAddBytes(void *ctx, byte const *p, unsigned len)
	{
			(void)ctx;

while (len--)
	randPoolAddByte(*p++);

/* Pool has changed, randOut is out of date */
randOutGetPos = sizeof(randOut);
}

static void
randGetOutput(void)
	{
			int i;

		/*
			* Hash the pending input and the entire pool, with the old
			* randOut as IV.
			*/
			HashTransform(randOut, randKey);
for (i = 0; i < RANDPOOLWORDS; i += HASH_INWORDS)
	HashTransform(randOut, randPool+i);

/* Okay, we can now fetch starting at the beginning of randOut */
randOutGetPos = 0;
}

/*
* Withdraw some bits from the pool. Regardless of the distribution of the
* input bits, the bits returned are uniformly distributed, although they
* cannot, of course, contain more Shannon entropy than the input bits.
*/
static void
randPoolGetBytesEntropy(void *ctx, byte *buf, unsigned len, unsigned bits)
	{
			unsigned t;

(void)ctx;

if (randBits >= bits)
	randBits -= bits;
else
	randFrac = randBits = 0;

while (len > (t = sizeof(randOut) - randOutGetPos)) {
			memcpy(buf, (byte *)randOut+randOutGetPos, t);
			buf += t;
			len -= t;
			randGetOutput();
	}

			if (len) {
				memcpy(buf, (byte *)randOut+randOutGetPos, len);
				randOutGetPos += len;
		}
	}

#if 0
byte
randPoolGetByte(void)
	{
			if (randBits >= 8)
				 randBits -= 8;
			else
				 randFrac = randBits = 0;

			if (randOutGetPos == sizeof(randOut))
				 randGetOutput();

			return ((byte *)randOut)[randOutGetPos++];
	}
	#endif

/*
* Destroys already-used random numbers. Ensures no sensitive data
* remains in memory that can be recovered later. This repeatedly
* mixes the output of the generator back into itself until all internal
* state has been overwritten.
*/
static void
randPoolStir(void *ctx)
	{
			unsigned i;

(void)ctx;
for (i = 0; i < 5*sizeof(randKey)/sizeof(randOut); i++) {
	randPoolAddBytes(NULL, (byte const *)randOut, sizeof(randOut));
	randGetOutput();
}
}

static struct PgpRandom PoolDesc = {
	"Global true random-number pool",
	randPoolAddBytes, randPoolGetBytesEntropy,
	randPoolStir, randPoolWipe
};

/* The destroy function is uncallable anyway... */
struct PgpRandomContext const pgpRandomPool = {
	&PoolDesc, 0, (void (*)(struct PgpRandomContext *))0
};

/*
* True random bit accumulation. These are extra functions exported
* by pgpRndPool.h for entropy estimation.
*
* Entropy is expressed in terms of a delta, essentially the difference
* between the observed value and the expected value, whose logarithm
* is a measure of entropy. (The logarithm to the base 2 is entropy
* expressed in bits.)
*
* The internal amount-of-entropy accumulator is maintained in two
* halves: a counter of bits, and a fraction of a bit, expressed
* in the form of a normalized delta. If 0 <= f < 1 is a fraction
* of a bit, then 1 <= 2^f < 2, so it's remembered as
* 0 <= x = 2^f - 1 < 1, a fixed-point number.
*
* Given a new fractional-bit delta, 1+y in similar form (obtained
* by normalizing the delta into the desired form), the output
* fractional bit delta 1+z = (1+x) * (1+y). If this exceeds 2,
* divide it by 2 and add a full bit to the bit counter.
*
* The implementation, of course, actually computes z = x + y + x*y,
* where 0 <= z < 3. This can be done by adding each of the three terms
* (each of which is less than 1) and noticing the overflow. If the
* addition does not overflow, z < 1 and nothing needs to be done.
*
* If one addition overflows, 1 <= z < 2, but after overflow we get
* z-1. We want (1+z)/2-1 = (z-1)/2, so just divide the result of
* the wrapped addition by 2.
*
* If both additions overflow, the addition wraps to z-2, but we
* still want (z-1)/2 = (z-2)/2 + 1/2, so divide the wrapped result
* by 2 and add 1/2.
*
* Due to the way that the fixed-point numbers are stored, the x*y part is
* the high half of the 32-bit unsigned product of x and y. This can be
* safely underestimated if desired, if a 64-bit product is difficult to
* compute.
*
* The simplest snd safest definition is
* #define UMULH_32(r,a,b) (r) = 0
*/
#ifndef UMULH_32
#if defined(__GNUC__) && defined(__i386__)
/* Inline asm goodies */
#define UMULH_32(r,a,b) __asm__("mull %2" : "=d"(r) : "%a"(a), "mr"(b) : "ax")
#elif HAVE64
#define UMULH_32(r,a,b) ((r) = (word32)((word64)(a) * (b) >> 32))
#else
/* Underestimate the product */
#define UMULH_32(r,a,b) ((r) = ((a) >> 16) * ((b) >> 16))
#endif
#endif /* !UMULH_32 */

#define DERATING 2	/* # of bits to underestimate */

unsigned
pgpRandPoolAddEntropy(word32 delta)
	{
			word32 frac, t;
			unsigned n;

			if (delta < 1 << DERATING)
				 return 0;
		
			n = 31-DERATING;
			if (!(delta & 0xffff0000))
				 delta <<= 16, n -= 16;
			if (!(delta & 0xff000000))
				 delta <<= 8, n -= 8;
			if (!(delta & 0xf0000000))
				 delta <<= 4, n -= 4;
			if (!(delta & 0xc0000000))
				 delta <<= 2, n -= 2;
			if (!(delta & 0x80000000))
				 delta <<= 1, n -= 1;
			pgpAssert(n < 32);

			/* Lose high-order bit of delta */
			pgpAssert(delta & 0x80000000);
			delta <<= 1;

			frac = randFrac;
			UMULH_32(t,delta,frac);
			if ((frac += t) < t) {
					if ((frac += delta) < delta)
						frac = (frac >> 1) + 0x80000000ul;
					else
						frac >>= 1;
					n++;
			} else if ((frac += delta) < delta) {
				 frac >>= 1;
				 n++;
				}

				/* n is now the count of whole bits */
				if ((randBits += n) >= RANDPOOLBITS) {
					/* Overflow - saturate at RANDPOOLBITS */
					randBits = RANDPOOLBITS;
					randFrac = 0;
				}
				/* Also count the inflow without regard to outflow */
				if ((randInBits += n) >= RANDPOOLBITS) {
					randInBits = RANDPOOLBITS;
				}
				return n;
	}

unsigned
pgpRandPoolEntropy(void)
{
	return randBits;
}

unsigned
pgpRandPoolSize(void)
{
return RANDPOOLBITS;
}

unsigned
pgpRandPoolInflow(void)
{
return randInBits;
}



/*
	*			DUMMY POOL
*
	*			The dummy pool is a pluggable replacement for the RandPool to be used
	*			for generating values which we want to be unique but don't have to be
	*			cryptographically strong: such as the p and q values used in discrete
	*			log keys. We prefer not to use the regular RandPool for such values
	*			so that we don't leak information about the secret values which are
	*			calculated in close proximity.
*
	*			The dummy pool should be used as the "base" paramater for
	*			pgpRandomCreateX9_17. It always returns 0 values, and the X9.17 RNG
	*			will then produce non-cryptographically-strong pseudo-random numbers.
*
	*			Use pgpRandomAddBytes on the X9.17 RandomContext to seed it.
*/

/* Dummy pool always returns zeros */
static void
dumPoolGetBytesEntropy(void *ctx, byte *buf, unsigned len, unsigned bits)
{
	(void)ctx;
	(void)bits;
	while (len--) {
		*buf++ = 0;
	}
}
static void
dumPoolAddBytes(void *ctx, byte const *p, unsigned len)
	{
			(void)ctx;
			(void)p;
			(void)len;
	}
static void
dumPoolStir(void *ctx)
{
	(void)ctx;
}
static void
dumPoolWipe(void *ctx)
{
	(void)ctx;
}

static struct PgpRandom DummyPoolDesc = {
	"Dummy random-number pool",
	dumPoolAddBytes, dumPoolGetBytesEntropy, dumPoolStir, dumPoolWipe
};

struct PgpRandomContext const pgpDummyPool = {
	&DummyPoolDesc, 0, (void (*)(struct PgpRandomContext *))0
};
