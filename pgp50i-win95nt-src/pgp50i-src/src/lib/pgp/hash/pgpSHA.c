/*
* pgpSHA.c - NIST Secure Hash Algorithm, FIPS PUB 180 and 180.1.
* The algorithm is by spook(s) unknown at the U.S. National Security Agency.
*
* Written 2 September 1992, Peter C. Gutmann.
* This implementation placed in the public domain.
*
* Modified 1 June 1993, Colin Plumb.
* Modified for the new SHS based on Peter Gutmann's work,
* 18 July 1994, Colin Plumb.
*
* Renamed to SHA and comments updated a bit 1 November 1995, Colin Plumb.
* These modifications placed in the public domain.
* Hacked on some more for PGP 3, December 1995, Colin Plumb.
* You probably don't *want* these modifications.
*
* Comments to pgut1@cs.aukuni.ac.nz
*
* $Id: pgpSHA.c,v 1.1.2.1 1997/05/19 00:59:57 hal Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "pgpHash.h"
#include "pgpSHA.h"
#include "pgpUsuals.h"

/*
* Define to 1 for FIPS 180.1 version (with extra rotate in prescheduling),
* 0 for FIPS 180 version (with the mysterious "weakness" that the NSA
* isn't talking about).
*/
#define SHA_VERSION 1

#define SHA_BLOCKBYTES 64
#define SHA_BLOCKWORDS 16

#define SHA_HASHBYTES 20
#define SHA_HASHWORDS 5

struct SHAContext {
 word32 key[SHA_BLOCKWORDS];
 word32 iv[SHA_HASHWORDS];
#if HAVE64
	word64 bytes;
#else
	word32 bytesHi, bytesLo;
#endif
};

/*
* Shuffle the bytes into big-endian order within words, as per the
* SHA spec.
*/
static void
shaByteSwap(word32 *dest, byte const *src, unsigned words)
	{
		do {
			*dest++ = (word32)((unsigned)src[0] << 8 | src[1]) << 16 |
			((unsigned)src[2] << 8 | src[3]);
			src += 4;
		} while (--words);
}

/* Initialize the SHA values */

static void
shaInit(void *priv)
	{
		struct SHAContext *ctx = (struct SHAContext *)priv;

		/* Set the h-vars to their initial values */
		ctx->iv[0] = 0x67452301;
		ctx->iv[1] = 0xEFCDAB89;
		ctx->iv[2] = 0x98BADCFE;
		ctx->iv[3] = 0x10325476;
		ctx->iv[4] = 0xC3D2E1F0;

		/* Initialise bit count */
#ifdef HAVE64
		ctx->bytes = 0;
	#else
		ctx->bytesHi = 0;
		ctx->bytesLo = 0;
	#endif
	}

/*
* The SHA f()-functions. The f1 and f3 functions can be optimized to
* save one boolean operation each - thanks to Rich Schroeppel,
* rcs@cs.arizona.edu for discovering this.
* The f3 function can be modified to use an addition to combine the
* two halves rather than OR, allowing more opportunity for using
* associativity in optimization. (Colin Plumb)
*
* Note that it may be necessary to add parentheses to these macros
* if they are to be called with expressions as arguments.
*/
/* f1 is a bit-select function. If (x) then y else z */
/*#define f1(x,y,z) ( (x & y) | (~x & z) )  // Rounds 0-19 */
#define f1(x,y,z) ( z ^ (x & (y ^ z) ) )  /* Rounds 0-19 */
#define f2(x,y,z) ( x ^ y ^ z )   /* Rounds 20-39 */
/* f3 is a majority function */
/*#define f3(x,y,z) ( (x & y) | (y & z) | (z & x) ) // Rounds 40-59 */
/*#define f3(x,y,z) ( (x & y) | (z & (x | y) ) ) // Rounds 40-59 */
#define f3(x,y,z) ( (x & y) + (z & (x ^ y) ) ) /* Rounds 40-59 */
#define f4(x,y,z) ( x ^ y ^ z )   /* Rounds 60-79 */

/* The SHA Mysterious Constants. */
#define K2 0x5A827999L /* Rounds 0-19 - floor(sqrt(2) * 2^30) */
#define K3 0x6ED9EBA1L /* Rounds 20-39 - floor(sqrt(3) * 2^30) */
#define K5 0x8F1BBCDCL /* Rounds 40-59 - floor(sqrt(5) * 2^30) */
#define K10 0xCA62C1D6L /* Rounds 60-79 - floor(sqrt(10) * 2^30) */
/* I wonder why not use K7=0xA953FD4E, K11=0xD443949F or K13=0xE6C15A23 */

/* 32-bit rotate left - kludged with shifts */

#define ROTL(n,X) ( (X << n) | (X >> (32-n)) )

/*
* The initial expanding function
*
* The hash function is defined over an 80-word expanded input array W,
* where the first 16 are copies of the input data, and the remaining 64
* are defined by W[i] = W[i-16] ^ W[i-14] ^ W[i-8] ^ W[i-3]. This
* implementation generates these values on the fly in a circular buffer.
*
* The new "corrected" FIPS 180.1 added a 1-bit left rotate to this
* computation of W[i].
*
* The expandx() version doesn't write the result back, which can be
* used for the last three rounds since those outputs are never used.
*/

#if SHA_VERSION /* FIPS 180.1 */
#define expandx(W,i) (t = W[i&15] ^ W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15], \
ROTL(1, t))
#define expand(W,i) (W[i&15] = expandx(W,i))
#else /* Old FIPS 180 */
#define expandx(W,i) (W[i&15] ^ W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15])
#define expand(W,i) (W[i&15] ^= W[(i-14)&15] ^ W[(i-8)&15] ^ W[(i-3)&15])
#endif

/*
* The prototype SHA sub-round
*
* The fundamental sub-round is
* a' = e + ROTL(5,a) + f(b, c, d) + k + data;
* b' = a;
* c' = ROTL(30,b);
* d' = c;
* e' = d;
* ... but this is implemented by unrolling the loop 5 times and renaming
* the variables (e,a,b,c,d) = (a',b',c',d',e') each iteration.
*/
#define subRound(a, b, c, d, e, f, k, data) \
 ( e += ROTL(5,a) + f(b, c, d) + k + data, b = ROTL(30, b) )
/*
* The above code is replicated 20 times for each of the 4 functions,
* using the next 20 values from the W[] array for "data" each time.
*/

/*
* Perform the SHA transformation. Note that this code, like MD5, seems to
* break some optimizing compilers due to the complexity of the expressions
* and the size of the basic block. It may be necessary to split it into
* sections, e.g. based on the four subrounds
*
* Note that this corrupts the sha->key area.
*/

static void
shaTransform(struct SHAContext *sha)
{
 register word32 A, B, C, D, E;
#if SHA_VERSION
		register word32 t;
	#endif

		/* Set up first buffer */
		A = sha->iv[0];
		B = sha->iv[1];
		C = sha->iv[2];
		D = sha->iv[3];
		E = sha->iv[4];

		/* Heavy mangling, in 4 sub-rounds of 20 interations each. */
		subRound( A, B, C, D, E, f1, K2, sha->key[ 0] );
		subRound( E, A, B, C, D, f1, K2, sha->key[ 1] );
		subRound( D, E, A, B, C, f1, K2, sha->key[ 2] );
		subRound( C, D, E, A, B, f1, K2, sha->key[ 3] );
		subRound( B, C, D, E, A, f1, K2, sha->key[ 4] );
		subRound( A, B, C, D, E, f1, K2, sha->key[ 5] );
		subRound( E, A, B, C, D, f1, K2, sha->key[ 6] );
		subRound( D, E, A, B, C, f1, K2, sha->key[ 7] );
		subRound( C, D, E, A, B, f1, K2, sha->key[ 8] );
		subRound( B, C, D, E, A, f1, K2, sha->key[ 9] );
		subRound( A, B, C, D, E, f1, K2, sha->key[10] );
		subRound( E, A, B, C, D, f1, K2, sha->key[11] );
		subRound( D, E, A, B, C, f1, K2, sha->key[12] );
		subRound( C, D, E, A, B, f1, K2, sha->key[13] );
		subRound( B, C, D, E, A, f1, K2, sha->key[14] );
		subRound( A, B, C, D, E, f1, K2, sha->key[15] );
		subRound( E, A, B, C, D, f1, K2, expand(sha->key, 16) );
		subRound( D, E, A, B, C, f1, K2, expand(sha->key, 17) );
		subRound( C, D, E, A, B, f1, K2, expand(sha->key, 18) );
		subRound( B, C, D, E, A, f1, K2, expand(sha->key, 19) );

		subRound( A, B, C, D, E, f2, K3, expand(sha->key, 20) );
		subRound( E, A, B, C, D, f2, K3, expand(sha->key, 21) );
		subRound( D, E, A, B, C, f2, K3, expand(sha->key, 22) );
		subRound( C, D, E, A, B, f2, K3, expand(sha->key, 23) );
		subRound( B, C, D, E, A, f2, K3, expand(sha->key, 24) );
		subRound( A, B, C, D, E, f2, K3, expand(sha->key, 25) );
		subRound( E, A, B, C, D, f2, K3, expand(sha->key, 26) );
		subRound( D, E, A, B, C, f2, K3, expand(sha->key, 27) );
		subRound( C, D, E, A, B, f2, K3, expand(sha->key, 28) );
		subRound( B, C, D, E, A, f2, K3, expand(sha->key, 29) );
		subRound( A, B, C, D, E, f2, K3, expand(sha->key, 30) );
		subRound( E, A, B, C, D, f2, K3, expand(sha->key, 31) );
		subRound( D, E, A, B, C, f2, K3, expand(sha->key, 32) );
		subRound( C, D, E, A, B, f2, K3, expand(sha->key, 33) );
		subRound( B, C, D, E, A, f2, K3, expand(sha->key, 34) );
		subRound( A, B, C, D, E, f2, K3, expand(sha->key, 35) );
		subRound( E, A, B, C, D, f2, K3, expand(sha->key, 36) );
		subRound( D, E, A, B, C, f2, K3, expand(sha->key, 37) );
		subRound( C, D, E, A, B, f2, K3, expand(sha->key, 38) );
		subRound( B, C, D, E, A, f2, K3, expand(sha->key, 39) );

		subRound( A, B, C, D, E, f3, K5, expand(sha->key, 40) );
		subRound( E, A, B, C, D, f3, K5, expand(sha->key, 41) );
		subRound( D, E, A, B, C, f3, K5, expand(sha->key, 42) );
		subRound( C, D, E, A, B, f3, K5, expand(sha->key, 43) );
		subRound( B, C, D, E, A, f3, K5, expand(sha->key, 44) );
		subRound( A, B, C, D, E, f3, K5, expand(sha->key, 45) );
		subRound( E, A, B, C, D, f3, K5, expand(sha->key, 46) );
		subRound( D, E, A, B, C, f3, K5, expand(sha->key, 47) );
		subRound( C, D, E, A, B, f3, K5, expand(sha->key, 48) );
		subRound( B, C, D, E, A, f3, K5, expand(sha->key, 49) );
		subRound( A, B, C, D, E, f3, K5, expand(sha->key, 50) );
		subRound( E, A, B, C, D, f3, K5, expand(sha->key, 51) );
		subRound( D, E, A, B, C, f3, K5, expand(sha->key, 52) );
		subRound( C, D, E, A, B, f3, K5, expand(sha->key, 53) );
		subRound( B, C, D, E, A, f3, K5, expand(sha->key, 54) );
		subRound( A, B, C, D, E, f3, K5, expand(sha->key, 55) );
		subRound( E, A, B, C, D, f3, K5, expand(sha->key, 56) );
		subRound( D, E, A, B, C, f3, K5, expand(sha->key, 57) );
		subRound( C, D, E, A, B, f3, K5, expand(sha->key, 58) );
		subRound( B, C, D, E, A, f3, K5, expand(sha->key, 59) );

		subRound( A, B, C, D, E, f4, K10, expand(sha->key, 60) );
		subRound( E, A, B, C, D, f4, K10, expand(sha->key, 61) );
		subRound( D, E, A, B, C, f4, K10, expand(sha->key, 62) );
		subRound( C, D, E, A, B, f4, K10, expand(sha->key, 63) );
		subRound( B, C, D, E, A, f4, K10, expand(sha->key, 64) );
		subRound( A, B, C, D, E, f4, K10, expand(sha->key, 65) );
		subRound( E, A, B, C, D, f4, K10, expand(sha->key, 66) );
		subRound( D, E, A, B, C, f4, K10, expand(sha->key, 67) );
		subRound( C, D, E, A, B, f4, K10, expand(sha->key, 68) );
		subRound( B, C, D, E, A, f4, K10, expand(sha->key, 69) );
		subRound( A, B, C, D, E, f4, K10, expand(sha->key, 70) );
		subRound( E, A, B, C, D, f4, K10, expand(sha->key, 71) );
		subRound( D, E, A, B, C, f4, K10, expand(sha->key, 72) );
		subRound( C, D, E, A, B, f4, K10, expand(sha->key, 73) );
		subRound( B, C, D, E, A, f4, K10, expand(sha->key, 74) );
		subRound( A, B, C, D, E, f4, K10, expand(sha->key, 75) );
		subRound( E, A, B, C, D, f4, K10, expand(sha->key, 76) );
		subRound( D, E, A, B, C, f4, K10, expandx(sha->key, 77) );
		subRound( C, D, E, A, B, f4, K10, expandx(sha->key, 78) );
		subRound( B, C, D, E, A, f4, K10, expandx(sha->key, 79) );

		/* Build message digest */
		sha->iv[0] += A;
		sha->iv[1] += B;
		sha->iv[2] += C;
		sha->iv[3] += D;
		sha->iv[4] += E;
	}

/* Update SHA for a block of data. */

static void
shaUpdate(void *priv, byte const *buf, size_t len)
{
 struct SHAContext *ctx = (struct SHAContext *)priv;
 unsigned i;

/* Update bitcount */

#ifdef HAVE64
		i = (unsigned)ctx->bytes % SHA_BLOCKBYTES;
		ctx->bytes += len;
	#else
		word32 t = ctx->bytesLo;
		if ( ( ctx->bytesLo = t + len ) < t )
		 ctx->bytesHi++; /* Carry from low to high */

		i = (unsigned)t % SHA_BLOCKBYTES; /* Bytes already in ctx->key */
	#endif

		/* i is always less than SHA_BLOCKBYTES. */
		if (SHA_BLOCKBYTES-i > len) {
		 memcpy((byte *)ctx->key + i, buf, len);
		 return;
		}

		if (i) { /* First chunk is an odd size */
		memcpy((byte *)ctx->key + i, buf, SHA_BLOCKBYTES - i);
		shaByteSwap(ctx->key, (byte *)ctx->key, SHA_BLOCKWORDS);
		shaTransform(ctx);
		buf += SHA_BLOCKBYTES-i;
		len -= SHA_BLOCKBYTES-i;
	}

/* Process data in 64-byte chunks */
while (len >= SHA_BLOCKBYTES) {
		shaByteSwap(ctx->key, buf, SHA_BLOCKWORDS);
		shaTransform(ctx);
		buf += SHA_BLOCKBYTES;
		len -= SHA_BLOCKBYTES;
	}

/* Handle any remaining bytes of data. */
if (len)
 memcpy(ctx->key, buf, len);
}

/*
* Final wrapup - pad to 64-byte boundary with the bit pattern
* 1 0* (64-bit count of bits processed, MSB-first)
*/
static byte const *
shaFinal(void *priv)
{
 struct SHAContext *ctx = (struct SHAContext *)priv;
 byte *digest;
#if HAVE64
		unsigned i = (unsigned)ctx->bytes % SHA_BLOCKBYTES;
	#else
		unsigned i = (unsigned)ctx->bytesLo % SHA_BLOCKBYTES;
	#endif
		byte *p = (byte *)ctx->key + i; /* First unused byte */
		word32 t;

		/* Set the first char of padding to 0x80. There is always room. */
		*p++ = 0x80;

		/* Bytes of padding needed to make 64 bytes (0..63) */
		i = SHA_BLOCKBYTES - 1 - i;

		if (i < 8) { /* Padding forces an extra block */
			memset(p, 0, i);
			shaByteSwap(ctx->key, (byte *)ctx->key, 16);
			shaTransform(ctx);
			p = (byte *)ctx->key;
			i = 64;
		}
		memset(p, 0, i-8);
		shaByteSwap(ctx->key, (byte *)ctx->key, 14);

		/* Append length in bits and transform */
#if HAVE64
		ctx->key[14] = (word32)(ctx->bytes >> 29);
		ctx->key[15] = (word32)ctx->bytes << 3;
	#else
		ctx->key[14] = ctx->bytesHi << 3 | ctx->bytesLo >> 29;
		ctx->key[15] = ctx->bytesLo << 3;
	#endif
		shaTransform(ctx);

		digest = (byte *)ctx->iv;
		for (i = 0; i < SHA_HASHWORDS; i++) {
			t = ctx->iv[i];
			digest[0] = (byte)(t >> 24);
			digest[1] = (byte)(t >> 16);
			digest[2] = (byte)(t >> 8);
			digest[3] = (byte)t;
			digest += 4;
		}
		/* In case it's sensitive */
/* XXX memset(ctx, 0, sizeof(ctx)); */
 return (byte const *)ctx->iv;
}

#if 0
/*
* Nobody knows an SHA format, so we make one up out of the
* context-dependent tag space.
*/
static byte const SHADERprefix[] = {
		0x82, /* context-specific [2], primitive */
		0x14 /* Length 20 */
		 /* 20 SHA digest bytes go here */
	};
	#else
/*
* Actually, there is one. SHA.1 has an OID of 1.3.14.3.2.26
* (From the 1994 Open Systems Environment Implementor's Workshop (OIW))
* The rest of the format is stolen from MD5. Do we need the @#$@$
* NULL in there?
*/
static byte const SHADERprefix[] = {
		0x30, /* Universal, Constructed, Sequence */
		0x21, /* Length 33 (bytes following) */
			0x30, /* Universal, Constructed, Sequence */
			0x09, /* Length 9 */
				0x06, /* Universal, Primitive, object-identifier */
				0x05, /* Length 8 */
					43, /* 43 = ISO(1)*40 + 3 */
					14,
					3,
					2,
					26,
				0x05, /* Universal, Primitive, NULL */
				0x00, /* Length 0 */
			0x04, /* Universal, Primitive, Octet string */
			0x14 /* Length 20 */
			 /* 20 SHA.1 digest bytes go here */
	};
#endif

struct PgpHash const HashSHA = {
		"SHA1", PGP_HASH_SHA,
		SHADERprefix, sizeof(SHADERprefix),
		SHA_HASHBYTES,
		sizeof(struct SHAContext),
		sizeof(struct{char _a; struct SHAContext _b;}) -
			sizeof(struct SHAContext),
		shaInit, shaUpdate, shaFinal
	};

#if TESTMAIN

/* ----------------------------- SHA Test code --------------------------- */
#include <stdio.h>
#include <stdlib.h> /* For exit() */
#include <time.h>

/* Size of buffer for SHA speed test data */

#define TEST_BLOCK_SIZE	( SHA_HASHBYTES * 100 )

/* Number of bytes of test data to process */

#define TEST_BYTES 10000000L
#define TEST_BLOCKS ( TEST_BYTES / TEST_BLOCK_SIZE )

#if SHA_VERSION
static char const *shaTestResults[] = {
		"A9993E364706816ABA3E25717850C26C9CD0D89D",
		"84983E441C3BD26EBAAE4AA1F95129E5E54670F1",
		"34AA973CD4C4DAA4F61EEB2BDBAD27316534016F",
		"34AA973CD4C4DAA4F61EEB2BDBAD27316534016F",
		"34AA973CD4C4DAA4F61EEB2BDBAD27316534016F" };
#else
static char const *shaTestResults[] = {
 "0164B8A914CD2A5E74C4F7FF082C4D97F1EDF880",
 "D2516EE1ACFA5BAF33DFC1C471E438449EF134C8",
 "3232AFFA48628A26653B5AAA44541FD90D690603",
 "3232AFFA48628A26653B5AAA44541FD90D690603",
"3232AFFA48628A26653B5AAA44541FD90D690603" };
#endif

static int
compareSHAresults(byte const *hash, int level)
	{
		char buf[41];
		int i;

		for (i = 0; i < SHA_HASHBYTES; i++)
		 sprintf(buf+2*i, "%02X", hash[i]);

		if (strcmp(buf, shaTestResults[level-1]) == 0) {
		 printf("Test %d passed, result = %s\n", level, buf);
		 return 0;
		} else {
			printf("Error in SHA implementation: Test %d failed\n", level);
			printf("  Result = %s\n", buf);
			printf("Expected = %s\n", shaTestResults[level-1]);
			return -1;
		}
	}


int
main(void)
	{
		struct SHAContext sha;
		byte data[TEST_BLOCK_SIZE];
		byte const *hash;
		clock_t ticks;
		long i;

		/*
		* Test output data (these are the only test data given in the
		* Secure Hash Standard document, but chances are if it works
		* for this it'll work for anything)
		*/
		shaInit(&sha);
		shaUpdate(&sha, (byte *)"abc", 3);
		hash = shaFinal(&sha);
		if (compareSHAresults(hash, 1) < 0)
		 exit (-1);

		shaInit(&sha);
		shaUpdate(&sha, (byte *)"abcdbcdecdefdefgefghfghighijhijki\
jkljklmklmnlmnomnopnopq", 56);
		hash = shaFinal(&sha);
		if (compareSHAresults(hash, 2) < 0)
		 exit (-1);

		/* 1,000,000 bytes of ASCII 'a' (0x61), by 64's */
		shaInit(&sha);
		for (i = 0; i < 15625; i++)
		 shaUpdate(&sha, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64);
 hash = shaFinal(&sha);
 if (compareSHAresults(hash, 3) < 0)
		 exit (-1);

		/* 1,000,000 bytes of ASCII 'a' (0x61), by 25's */
		shaInit(&sha);
		for (i = 0; i < 40000; i++)
		 shaUpdate(&sha, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa", 25);
		hash = shaFinal(&sha);
		if (compareSHAresults(hash, 4) < 0)
		 exit (-1);

		/* 1,000,000 bytes of ASCII 'a' (0x61), by 125's */
		shaInit(&sha);
		for (i = 0; i < 8000; i++)
		 shaUpdate(&sha, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 125);
		hash = shaFinal(&sha);
		if (compareSHAresults(hash, 5) < 0)
		 exit (-1);

		/* Now perform time trial, generating MD for 10MB of data. First,
		initialize the test data */
		memset(data, 0, TEST_BLOCK_SIZE);

		/* Get start time */
		printf("SHA time trial.  Processing %ld characters...\n", TEST_BYTES);
		ticks = clock();

		/* Calculate SHA message digest in TEST_BLOCK_SIZE byte blocks */
		shaInit(&sha);
		for (i = TEST_BLOCKS; i > 0; i--)
		 shaUpdate(&sha, data, TEST_BLOCK_SIZE);
		hash = shaFinal(&sha);

		/* Get finish time and print difference */
		ticks = clock() - ticks;
		printf("Ticks to process test input: %lu\n", (unsigned long)ticks);

		return 0;
	}
#endif /* Test driver */
