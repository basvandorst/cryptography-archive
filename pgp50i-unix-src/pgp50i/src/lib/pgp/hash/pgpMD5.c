/*
* pgpMD5.c
*
* This code implements the MD5 message-digest algorithm.
* The algorithm is due to Ron Rivest. This code was
* written by Colin Plumb in 1993, no copyright is claimed.
* This code is in the public domain; do with it what you wish.
*
* Equivalent code is available from RSA Data Security, Inc.
* This code has been tested against that, and is equivalent,
* except that you don't need to include two pages of legalese
* with every copy.
*
* To compute the message digest of a chunk of bytes, declare an
* MD5Context structure, pass it to MD5Init, call MD5Update as
* needed on buffers full of bytes, and then call MD5Final, which
* will fill a supplied 16-byte array with the digest.
*
* $Id: pgpMD5.c,v 1.1.2.1 1997/06/07 09:49:58 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>		/* for memcpy() */

#include "pgpHash.h"
#include "pgpMD5.h"
#include "pgpUsuals.h"

#define MD5_BLOCKBYTES 64
#define MD5_BLOCKWORDS 16

#define MD5_HASHBYTES 16
#define MD5_HASHWORDS 4

struct MD5Context {
 word32 key[MD5_BLOCKWORDS];
 word32 iv[MD5_HASHWORDS];
#if HAVE64
		word64 bytes;
	#else
		word32 bytesLo, bytesHi;
	#endif
	};

	/*
* Shuffle the bytes into little-endian order within words, as per the
* MD5 spec.
*/
static void
MD5ByteSwap(word32 *dest, byte const *src, unsigned words)
	{
		do {
			*dest++ = (word32)((unsigned)src[3] << 8 | src[2]) << 16 |
			((unsigned)src[1] << 8 | src[0]);
			src += 4;
	} while (--words);
}

/*
* Start MD5 accumulation. Set bit count to 0 and buffer to mysterious
* initialization constants.
*/
static void
MD5Init(void *priv)
	{
		struct MD5Context *ctx = (struct MD5Context *)priv;

ctx->iv[0] = 0x67452301;
ctx->iv[1] = 0xefcdab89;
ctx->iv[2] = 0x98badcfe;
ctx->iv[3] = 0x10325476;

#if HAVE64
		ctx->bytes = 0;
	#else
		ctx->bytesHi = ctx->bytesLo = 0;
	#endif
	}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) \
 (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

/*
* The core of the MD5 algorithm, this alters an existing MD5 hash to
* reflect the addition of 16 longwords of new data. MD5Update blocks
* the data and converts bytes into longwords for this routine.
*
* NOTE This is not declared as static because pgpRndPool.c references it.
* (Sorry for that modularity violation.)
*/
void MD5Transform(word32 *block, word32 const *key);
void
MD5Transform(word32 *block, word32 const *key)
	{
		register word32 a, b, c, d;

		a = block[0];
		b = block[1];
		c = block[2];
		d = block[3];

		MD5STEP(F1, a, b, c, d, key[0] + 0xd76aa478, 7);
		MD5STEP(F1, d, a, b, c, key[1] + 0xe8c7b756, 12);
		MD5STEP(F1, c, d, a, b, key[2] + 0x242070db, 17);
		MD5STEP(F1, b, c, d, a, key[3] + 0xc1bdceee, 22);
		MD5STEP(F1, a, b, c, d, key[4] + 0xf57c0faf, 7);
		MD5STEP(F1, d, a, b, c, key[5] + 0x4787c62a, 12);
		MD5STEP(F1, c, d, a, b, key[6] + 0xa8304613, 17);
		MD5STEP(F1, b, c, d, a, key[7] + 0xfd469501, 22);
		MD5STEP(F1, a, b, c, d, key[8] + 0x698098d8, 7);
		MD5STEP(F1, d, a, b, c, key[9] + 0x8b44f7af, 12);
		MD5STEP(F1, c, d, a, b, key[10] + 0xffff5bb1, 17);
		MD5STEP(F1, b, c, d, a, key[11] + 0x895cd7be, 22);
		MD5STEP(F1, a, b, c, d, key[12] + 0x6b901122, 7);
		MD5STEP(F1, d, a, b, c, key[13] + 0xfd987193, 12);
		MD5STEP(F1, c, d, a, b, key[14] + 0xa679438e, 17);
		MD5STEP(F1, b, c, d, a, key[15] + 0x49b40821, 22);

		MD5STEP(F2, a, b, c, d, key[1] + 0xf61e2562, 5);
		MD5STEP(F2, d, a, b, c, key[6] + 0xc040b340, 9);
		MD5STEP(F2, c, d, a, b, key[11] + 0x265e5a51, 14);
		MD5STEP(F2, b, c, d, a, key[0] + 0xe9b6c7aa, 20);
		MD5STEP(F2, a, b, c, d, key[5] + 0xd62f105d, 5);
		MD5STEP(F2, d, a, b, c, key[10] + 0x02441453, 9);
		MD5STEP(F2, c, d, a, b, key[15] + 0xd8a1e681, 14);
		MD5STEP(F2, b, c, d, a, key[4] + 0xe7d3fbc8, 20);
		MD5STEP(F2, a, b, c, d, key[9] + 0x21e1cde6, 5);
		MD5STEP(F2, d, a, b, c, key[14] + 0xc33707d6, 9);
		MD5STEP(F2, c, d, a, b, key[3] + 0xf4d50d87, 14);
		MD5STEP(F2, b, c, d, a, key[8] + 0x455a14ed, 20);
		MD5STEP(F2, a, b, c, d, key[13] + 0xa9e3e905, 5);
		MD5STEP(F2, d, a, b, c, key[2] + 0xfcefa3f8, 9);
		MD5STEP(F2, c, d, a, b, key[7] + 0x676f02d9, 14);
		MD5STEP(F2, b, c, d, a, key[12] + 0x8d2a4c8a, 20);

		MD5STEP(F3, a, b, c, d, key[5] + 0xfffa3942, 4);
		MD5STEP(F3, d, a, b, c, key[8] + 0x8771f681, 11);
		MD5STEP(F3, c, d, a, b, key[11] + 0x6d9d6122, 16);
		MD5STEP(F3, b, c, d, a, key[14] + 0xfde5380c, 23);
		MD5STEP(F3, a, b, c, d, key[1] + 0xa4beea44, 4);
		MD5STEP(F3, d, a, b, c, key[4] + 0x4bdecfa9, 11);
		MD5STEP(F3, c, d, a, b, key[7] + 0xf6bb4b60, 16);
		MD5STEP(F3, b, c, d, a, key[10] + 0xbebfbc70, 23);
		MD5STEP(F3, a, b, c, d, key[13] + 0x289b7ec6, 4);
		MD5STEP(F3, d, a, b, c, key[0] + 0xeaa127fa, 11);
		MD5STEP(F3, c, d, a, b, key[3] + 0xd4ef3085, 16);
		MD5STEP(F3, b, c, d, a, key[6] + 0x04881d05, 23);
		MD5STEP(F3, a, b, c, d, key[9] + 0xd9d4d039, 4);
		MD5STEP(F3, d, a, b, c, key[12] + 0xe6db99e5, 11);
		MD5STEP(F3, c, d, a, b, key[15] + 0x1fa27cf8, 16);
		MD5STEP(F3, b, c, d, a, key[2] + 0xc4ac5665, 23);

		MD5STEP(F4, a, b, c, d, key[0] + 0xf4292244, 6);
		MD5STEP(F4, d, a, b, c, key[7] + 0x432aff97, 10);
		MD5STEP(F4, c, d, a, b, key[14] + 0xab9423a7, 15);
		MD5STEP(F4, b, c, d, a, key[5] + 0xfc93a039, 21);
		MD5STEP(F4, a, b, c, d, key[12] + 0x655b59c3, 6);
		MD5STEP(F4, d, a, b, c, key[3] + 0x8f0ccc92, 10);
		MD5STEP(F4, c, d, a, b, key[10] + 0xffeff47d, 15);
		MD5STEP(F4, b, c, d, a, key[1] + 0x85845dd1, 21);
		MD5STEP(F4, a, b, c, d, key[8] + 0x6fa87e4f, 6);
		MD5STEP(F4, d, a, b, c, key[15] + 0xfe2ce6e0, 10);
		MD5STEP(F4, c, d, a, b, key[6] + 0xa3014314, 15);
		MD5STEP(F4, b, c, d, a, key[13] + 0x4e0811a1, 21);
		MD5STEP(F4, a, b, c, d, key[4] + 0xf7537e82, 6);
		MD5STEP(F4, d, a, b, c, key[11] + 0xbd3af235, 10);
		MD5STEP(F4, c, d, a, b, key[2] + 0x2ad7d2bb, 15);
		MD5STEP(F4, b, c, d, a, key[9] + 0xeb86d391, 21);

block[0] += a;
block[1] += b;
block[2] += c;
block[3] += d;
}

/*
* Update context to reflect the concatenation of another buffer full
* of bytes.
*/
static void
MD5Update(void *priv, byte const *buf, size_t len)
{
 struct MD5Context *ctx = (struct MD5Context *)priv;
 unsigned i;

#if HAVE64
		i = (unsigned)ctx->bytes % MD5_BLOCKBYTES;
		ctx->bytes += len;
	#else
		word32 t = ctx->bytesLo;
		if ((ctx->bytesLo = t + len) < t)
		 ctx->bytesHi++;  /* Carry from low to high */
		i = (unsigned)t % MD5_BLOCKBYTES; /* Bytes already in ctx->key */
	#endif
		if (MD5_BLOCKBYTES-i > len) {
		 memcpy((byte *)ctx->key + i, buf, len);
		 return;
		}

		if (i) { /* First chunk is an odd size */
			memcpy((byte *)ctx->key + i, buf, MD5_BLOCKBYTES - i);
			MD5ByteSwap(ctx->key, (byte *)ctx->key, MD5_BLOCKWORDS);
			MD5Transform(ctx->iv, ctx->key);
			buf += MD5_BLOCKBYTES-i;
			len -= MD5_BLOCKBYTES-i;
		}

/* Process data in 64-byte chunks */
while (len >= MD5_BLOCKBYTES) {
			MD5ByteSwap(ctx->key, buf, MD5_BLOCKWORDS);
			MD5Transform(ctx->iv, ctx->key);
			buf += MD5_BLOCKBYTES;
			len -= MD5_BLOCKBYTES;
		}

		/* Handle any remaining bytes of data. */
		if (len)
		 memcpy(ctx->key, buf, len);
	}

/*
* Final wrapup - pad to 64-byte boundary with the bit pattern
* 1 0* (64-bit count of bits processed, LSB-first)
*/
static byte const *
MD5Final(void *priv)
{
 struct MD5Context *ctx = (struct MD5Context *)priv;
 byte *digest;
#if HAVE64
		unsigned i = (unsigned)ctx->bytes % MD5_BLOCKBYTES;
	#else
		unsigned i = (unsigned)ctx->bytesLo % MD5_BLOCKBYTES;
	#endif
		byte *p = (byte *)ctx->key + i; /* First unused byte */
		word32 t;

/* Set the first char of padding to 0x80. There is always room. */
*p++ = 0x80;

/* Bytes of padding needed to make 64 bytes (0..63) */
i = MD5_BLOCKBYTES - 1 - i;

		if (i < 8) { /* Padding forces an extra block */
			memset(p, 0, i);
			MD5ByteSwap(ctx->key, (byte *)ctx->key, 16);
			MD5Transform(ctx->iv, ctx->key);
			p = (byte *)ctx->key;
			i = 64;
		}
		memset(p, 0, i-8);
		MD5ByteSwap(ctx->key, (byte *)ctx->key, 14);

		/* Append length in bits and transform */
#if HAVE64
		ctx->key[14] = (word32)ctx->bytes << 3;
		ctx->key[15] = (word32)(ctx->bytes >> 29);
	#else
		ctx->key[14] = ctx->bytesLo << 3;
		ctx->key[15] = ctx->bytesHi << 3 | ctx->bytesLo >> 29;
	#endif
		MD5Transform(ctx->iv, ctx->key);

		/* Convert digest to the correct byte order */
		digest = (byte *)ctx->iv;
		for (i = 0; i < MD5_HASHWORDS; i++) {
			t = ctx->iv[i];
			digest[0] = (byte)t;
			digest[1] = (byte)(t >> 8);
			digest[2] = (byte)(t >> 16);
			digest[3] = (byte)(t >> 24);
			digest += 4;
		}
		/* In case it's sensitive */
/* XXX memset(ctx->key, 0, sizeof(ctx->key)); */
 return (byte const *)ctx->iv;
}


/*
* The Basic Encoding Rules (of which the Distinguished Encoding Rules
* are a simple minimal-sized subset) are supposed to be compact. Humph.
*/
static byte const MD5DERprefix[] = {
		0x30, /* Universal, Constructed, Sequence */
		0x20, /* Length 32 (bytes following) */
			0x30, /* Universal, Constructed, Sequence */
			0x0c, /* Length 12 */
				0x06, /* Universal, Primitive, object-identifier */
				0x08, /* Length 8 */
					0x2a, /* 42 = ISO(1)*40 + Member bodies(2) */
					0x86, 0x48, /* 840 = US (ANSI) */
					0x86, 0xF7, 0x0D, /* 113549 = RSADSI */
					0x02, /* 2 = Hash functions */
					0x05, /* 5 = MD5 */
				0x05, /* Universal, Primitive, NULL */
				0x00, /* Length 0 */
			0x04, /* Universal, Primitive, Octet string */
			0x10 /* Length 16 */
			 /* 16 MD5 digest bytes go here */
	};

struct PgpHash const HashMD5 = {
		"MD5", PGP_HASH_MD5,
		MD5DERprefix, sizeof(MD5DERprefix),
		MD5_HASHBYTES,
		sizeof(struct MD5Context),
		sizeof(struct{char _a; struct MD5Context _b;}) -
		 sizeof(struct MD5Context),
		MD5Init, MD5Update, MD5Final
	};

#if TESTMAIN

#include <stdio.h>
#include <string.h>
#include <time.h>

#define TEST_BYTES 10000000

static char const * const md5TestResults[] = {
		"0cc175b9c0f1b6a831c399e269772661",
		"900150983cd24fb0d6963f7d28e17f72",
		"7707d6ae4e027c70eea2a935c2296f21",
		"7707d6ae4e027c70eea2a935c2296f21",
		"7707d6ae4e027c70eea2a935c2296f21"
	};

static int
compareMD5results(byte const *hash, int level)
{
 char buf[33];
 unsigned i;

 for (i = 0; i < MD5_HASHSIZE; i++)
  sprintf(buf+2*i, "%02x", hash[i]);
		if (strcmp(buf, md5TestResults[level-1]) == 0) {
		 printf("Test %d passed, result = %s\n", level, buf);
		 return 0;
		} else {
			printf("Error in MD5 implementation; test %d failed\n", level);
			printf("  Result = %s\nExpected = %s\n", buf,
			 md5TestResults[level-1]);
			return -1;
		}
	}

	int
main(void)
{
clock_t ticks;
struct MD5Context md5;
byte hash[MD5_HASHSIZE];
byte data[1999];
unsigned i;

MD5Init(&md5);
MD5Update(&md5, "a", 1);
MD5Final(&md5, hash);
if (compareMD5results(hash, 1) < 0)
 return -1;

MD5Init(&md5);
MD5Update(&md5, "abc", 3);
MD5Final(&md5, hash);
if (compareMD5results(hash, 2) < 0)
 return -1;

		/* 1,000,000 bytes of ASCII 'a', by 64s */
		MD5Init(&md5);
		for (i = 0; i < 15625; i++) {
		 MD5Update(&md5, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64);
		}
		MD5Final(&md5, hash);
		if (compareMD5results(hash, 3) < 0)
		 return -1;

/* 1,000,000 bytes of ASCII 'a', by 25s */
MD5Init(&md5);
for (i = 0; i < 40000; i++)
 MD5Update(&md5, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa", 25);
MD5Final(&md5, hash);
if (compareMD5results(hash, 4) < 0)
 return -1;

		/* 1,000,000 bytes of ASCII 'a', by 125s */
		MD5Init(&md5);
		for (i = 0; i < 8000; i++)
		 MD5Update(&md5, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 125);
 MD5Final(&md5, hash);
 if (compareMD5results(hash, 5) < 0)
		 return -1;

		memset(data, 0, sizeof(data));

		ticks = clock();
		MD5Init(&md5);
		for (i = 0; i < TEST_BYTES/sizeof(data); i++)
		 MD5Update(&md5, data, sizeof(data));
		MD5Update(&md5, data,TEST_BYTES % sizeof(data));

		MD5Final(&md5, hash);

		ticks = clock() - ticks;

		printf("Elapsed time for %lu characters: %lu ticks\n",
		 (unsigned long)TEST_BYTES, (unsigned long)ticks);
		return 0;
	}

#endif /* TESTMAIN */