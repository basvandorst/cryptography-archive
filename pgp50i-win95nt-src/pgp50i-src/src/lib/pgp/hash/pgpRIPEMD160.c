/*
* pgpRIPEMD160.c - European RIPE Message Digest, 160 bit (RIPEMD-160)
*
* The algorithm is by Hans Dobbertin, Antoon Bosselaers, and Bart Preneel.
*
* The code below is based on the reference implementation by Bosselaers.
* It is available at the time of writing from
* http://www.esat.kuleuven.ac.be/~bosselae/ripemd160.html
*
* $Id: pgpRIPEMD160.c,v 1.1.2.3 1997/05/22 22:53:50 hal Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "pgpHash.h"
#include "pgpRIPEMD160.h"
#include "pgpUsuals.h"

#define RIPEMD160_BLOCKBYTES 64
#define RIPEMD160_BLOCKWORDS 16

#define RIPEMD160_HASHBYTES 20
#define RIPEMD160_HASHWORDS 5

struct RIPEMD160Context {
 word32 key[RIPEMD160_BLOCKWORDS];
 word32 iv[RIPEMD160_HASHWORDS];
 word32 bytesHi, bytesLo;
};


/* Define types used in the reference code based on PGP library types */

/* typedef byte byte;  [* unsigned 8-bit integer *] */
typedef word16  word;  /* unsigned 16-bit integer */
typedef word32  dword;  /* unsigned 32-bit integer */


/************************ File rmd160.h **********************/
/* Extracted from rmd160.h in the reference implementation */

/* macro definitions */

/* ROL(x, n) cyclically rotates x over n bits to the left */
/* x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* the three basic functions F(), G() and H() */
	#define F(x, y, z)	((x) ^ (y) ^ (z))
	#define G(x, y, z)	(((x) & (y)) | (~(x) & (z)))
	#define H(x, y, z)	(((x) | ~(y)) ^ (z))
	#define I(x, y, z)	(((x) & (z)) | ((y) & ~(z)))
	#define J(x, y, z)	((x) ^ ((y) | ~(z)))

/* the eight basic operations FF() through III() */
	#define FF(a, b, c, d, e, x, s)	{\
(a) += F((b), (c), (d)) + (x);\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define GG(a, b, c, d, e, x, s)	{\
(a) += G((b), (c), (d)) + (x) + 0x5a827999UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define HH(a, b, c, d, e, x, s) {\
(a) += H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define II(a, b, c, d, e, x, s) {\
(a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define JJ(a, b, c, d, e, x, s) {\
(a) += J((b), (c), (d)) + (x) + 0xa953fd4eUL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define FFF(a, b, c, d, e, x, s)	{\
(a) += F((b), (c), (d)) + (x);\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define GGG(a, b, c, d, e, x, s)	{\
(a) += G((b), (c), (d)) + (x) + 0x7a6d76e9UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define HHH(a, b, c, d, e, x, s) {\
(a) += H((b), (c), (d)) + (x) + 0x6d703ef3UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define III(a, b, c, d, e, x, s) {\
(a) += I((b), (c), (d)) + (x) + 0x5c4dd124UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}
#define JJJ(a, b, c, d, e, x, s) {\
(a) += J((b), (c), (d)) + (x) + 0x50a28be6UL;\
(a) = ROL((a), (s)) + (e);\
(c) = ROL((c), 10);\
}



/************************ File rmd160.c **********************/
/* Extracted from rmd160.c in the reference implementation */


/*
* initializes MDbuffer to "magic constants"
*/
static void RMDinit(dword *MDbuf)
{
MDbuf[0] = 0x67452301UL;
MDbuf[1] = 0xefcdab89UL;
MDbuf[2] = 0x98badcfeUL;
MDbuf[3] = 0x10325476UL;
MDbuf[4] = 0xc3d2e1f0UL;

return;
}


/*
* the compression function.
* transforms MDbuf using message bytes X[0] through X[15]
*/
static void RMDcompress(dword *MDbuf, dword *X)
{
	dword aa = MDbuf[0], bb = MDbuf[1], cc = MDbuf[2],
		dd = MDbuf[3], ee = MDbuf[4];
	dword aaa = MDbuf[0], bbb = MDbuf[1], ccc = MDbuf[2],
		ddd = MDbuf[3], eee = MDbuf[4];

/* round 1 */
FF(aa, bb, cc, dd, ee, X[ 0], 11);
FF(ee, aa, bb, cc, dd, X[ 1], 14);
FF(dd, ee, aa, bb, cc, X[ 2], 15);
FF(cc, dd, ee, aa, bb, X[ 3], 12);
FF(bb, cc, dd, ee, aa, X[ 4], 5);
FF(aa, bb, cc, dd, ee, X[ 5], 8);
FF(ee, aa, bb, cc, dd, X[ 6], 7);
FF(dd, ee, aa, bb, cc, X[ 7], 9);
FF(cc, dd, ee, aa, bb, X[ 8], 11);
FF(bb, cc, dd, ee, aa, X[ 9], 13);
FF(aa, bb, cc, dd, ee, X[10], 14);
FF(ee, aa, bb, cc, dd, X[11], 15);
FF(dd, ee, aa, bb, cc, X[12], 6);
FF(cc, dd, ee, aa, bb, X[13], 7);
FF(bb, cc, dd, ee, aa, X[14], 9);
FF(aa, bb, cc, dd, ee, X[15], 8);

/* round 2 */
GG(ee, aa, bb, cc, dd, X[ 7], 7);
GG(dd, ee, aa, bb, cc, X[ 4], 6);
GG(cc, dd, ee, aa, bb, X[13], 8);
GG(bb, cc, dd, ee, aa, X[ 1], 13);
GG(aa, bb, cc, dd, ee, X[10], 11);
GG(ee, aa, bb, cc, dd, X[ 6], 9);
GG(dd, ee, aa, bb, cc, X[15], 7);
GG(cc, dd, ee, aa, bb, X[ 3], 15);
GG(bb, cc, dd, ee, aa, X[12], 7);
GG(aa, bb, cc, dd, ee, X[ 0], 12);
GG(ee, aa, bb, cc, dd, X[ 9], 15);
GG(dd, ee, aa, bb, cc, X[ 5], 9);
GG(cc, dd, ee, aa, bb, X[ 2], 11);
GG(bb, cc, dd, ee, aa, X[14], 7);
GG(aa, bb, cc, dd, ee, X[11], 13);
GG(ee, aa, bb, cc, dd, X[ 8], 12);

/* round 3 */
HH(dd, ee, aa, bb, cc, X[ 3], 11);
HH(cc, dd, ee, aa, bb, X[10], 13);
HH(bb, cc, dd, ee, aa, X[14], 6);
HH(aa, bb, cc, dd, ee, X[ 4], 7);
HH(ee, aa, bb, cc, dd, X[ 9], 14);
HH(dd, ee, aa, bb, cc, X[15], 9);
HH(cc, dd, ee, aa, bb, X[ 8], 13);
HH(bb, cc, dd, ee, aa, X[ 1], 15);
HH(aa, bb, cc, dd, ee, X[ 2], 14);
HH(ee, aa, bb, cc, dd, X[ 7], 8);
HH(dd, ee, aa, bb, cc, X[ 0], 13);
HH(cc, dd, ee, aa, bb, X[ 6], 6);
HH(bb, cc, dd, ee, aa, X[13], 5);
HH(aa, bb, cc, dd, ee, X[11], 12);
HH(ee, aa, bb, cc, dd, X[ 5], 7);
HH(dd, ee, aa, bb, cc, X[12], 5);

/* round 4 */
II(cc, dd, ee, aa, bb, X[ 1], 11);
II(bb, cc, dd, ee, aa, X[ 9], 12);
II(aa, bb, cc, dd, ee, X[11], 14);
II(ee, aa, bb, cc, dd, X[10], 15);
II(dd, ee, aa, bb, cc, X[ 0], 14);
II(cc, dd, ee, aa, bb, X[ 8], 15);
II(bb, cc, dd, ee, aa, X[12], 9);
II(aa, bb, cc, dd, ee, X[ 4], 8);
II(ee, aa, bb, cc, dd, X[13], 9);
II(dd, ee, aa, bb, cc, X[ 3], 14);
II(cc, dd, ee, aa, bb, X[ 7], 5);
II(bb, cc, dd, ee, aa, X[15], 6);
II(aa, bb, cc, dd, ee, X[14], 8);
II(ee, aa, bb, cc, dd, X[ 5], 6);
II(dd, ee, aa, bb, cc, X[ 6], 5);
II(cc, dd, ee, aa, bb, X[ 2], 12);

/* round 5 */
JJ(bb, cc, dd, ee, aa, X[ 4], 9);
JJ(aa, bb, cc, dd, ee, X[ 0], 15);
JJ(ee, aa, bb, cc, dd, X[ 5], 5);
JJ(dd, ee, aa, bb, cc, X[ 9], 11);
JJ(cc, dd, ee, aa, bb, X[ 7], 6);
JJ(bb, cc, dd, ee, aa, X[12], 8);
JJ(aa, bb, cc, dd, ee, X[ 2], 13);
JJ(ee, aa, bb, cc, dd, X[10], 12);
JJ(dd, ee, aa, bb, cc, X[14], 5);
JJ(cc, dd, ee, aa, bb, X[ 1], 12);
JJ(bb, cc, dd, ee, aa, X[ 3], 13);
JJ(aa, bb, cc, dd, ee, X[ 8], 14);
JJ(ee, aa, bb, cc, dd, X[11], 11);
JJ(dd, ee, aa, bb, cc, X[ 6], 8);
JJ(cc, dd, ee, aa, bb, X[15], 5);
JJ(bb, cc, dd, ee, aa, X[13], 6);

/* parallel round 1 */
JJJ(aaa, bbb, ccc, ddd, eee, X[ 5], 8);
JJJ(eee, aaa, bbb, ccc, ddd, X[14], 9);
JJJ(ddd, eee, aaa, bbb, ccc, X[ 7], 9);
JJJ(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
JJJ(bbb, ccc, ddd, eee, aaa, X[ 9], 13);
JJJ(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
JJJ(eee, aaa, bbb, ccc, ddd, X[11], 15);
JJJ(ddd, eee, aaa, bbb, ccc, X[ 4], 5);
JJJ(ccc, ddd, eee, aaa, bbb, X[13], 7);
JJJ(bbb, ccc, ddd, eee, aaa, X[ 6], 7);
JJJ(aaa, bbb, ccc, ddd, eee, X[15], 8);
JJJ(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
JJJ(ddd, eee, aaa, bbb, ccc, X[ 1], 14);
JJJ(ccc, ddd, eee, aaa, bbb, X[10], 14);
JJJ(bbb, ccc, ddd, eee, aaa, X[ 3], 12);
JJJ(aaa, bbb, ccc, ddd, eee, X[12], 6);

/* parallel round 2 */
III(eee, aaa, bbb, ccc, ddd, X[ 6], 9);
III(ddd, eee, aaa, bbb, ccc, X[11], 13);
III(ccc, ddd, eee, aaa, bbb, X[ 3], 15);
III(bbb, ccc, ddd, eee, aaa, X[ 7], 7);
III(aaa, bbb, ccc, ddd, eee, X[ 0], 12);
III(eee, aaa, bbb, ccc, ddd, X[13], 8);
III(ddd, eee, aaa, bbb, ccc, X[ 5], 9);
III(ccc, ddd, eee, aaa, bbb, X[10], 11);
III(bbb, ccc, ddd, eee, aaa, X[14], 7);
III(aaa, bbb, ccc, ddd, eee, X[15], 7);
III(eee, aaa, bbb, ccc, ddd, X[ 8], 12);
III(ddd, eee, aaa, bbb, ccc, X[12], 7);
III(ccc, ddd, eee, aaa, bbb, X[ 4], 6);
III(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
III(aaa, bbb, ccc, ddd, eee, X[ 1], 13);
III(eee, aaa, bbb, ccc, ddd, X[ 2], 11);

/* parallel round 3 */
HHH(ddd, eee, aaa, bbb, ccc, X[15], 9);
HHH(ccc, ddd, eee, aaa, bbb, X[ 5], 7);
HHH(bbb, ccc, ddd, eee, aaa, X[ 1], 15);
HHH(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
HHH(eee, aaa, bbb, ccc, ddd, X[ 7], 8);
HHH(ddd, eee, aaa, bbb, ccc, X[14], 6);
HHH(ccc, ddd, eee, aaa, bbb, X[ 6], 6);
HHH(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
HHH(aaa, bbb, ccc, ddd, eee, X[11], 12);
HHH(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
HHH(ddd, eee, aaa, bbb, ccc, X[12], 5);
HHH(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
HHH(bbb, ccc, ddd, eee, aaa, X[10], 13);
HHH(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
HHH(eee, aaa, bbb, ccc, ddd, X[ 4], 7);
HHH(ddd, eee, aaa, bbb, ccc, X[13], 5);

/* parallel round 4 */
GGG(ccc, ddd, eee, aaa, bbb, X[ 8], 15);
GGG(bbb, ccc, ddd, eee, aaa, X[ 6], 5);
GGG(aaa, bbb, ccc, ddd, eee, X[ 4], 8);
GGG(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
GGG(ddd, eee, aaa, bbb, ccc, X[ 3], 14);
GGG(ccc, ddd, eee, aaa, bbb, X[11], 14);
GGG(bbb, ccc, ddd, eee, aaa, X[15], 6);
GGG(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
GGG(eee, aaa, bbb, ccc, ddd, X[ 5], 6);
GGG(ddd, eee, aaa, bbb, ccc, X[12], 9);
GGG(ccc, ddd, eee, aaa, bbb, X[ 2], 12);
GGG(bbb, ccc, ddd, eee, aaa, X[13], 9);
GGG(aaa, bbb, ccc, ddd, eee, X[ 9], 12);
GGG(eee, aaa, bbb, ccc, ddd, X[ 7], 5);
GGG(ddd, eee, aaa, bbb, ccc, X[10], 15);
GGG(ccc, ddd, eee, aaa, bbb, X[14], 8);

/* parallel round 5 */
FFF(bbb, ccc, ddd, eee, aaa, X[12] , 8);
FFF(aaa, bbb, ccc, ddd, eee, X[15] , 5);
FFF(eee, aaa, bbb, ccc, ddd, X[10] , 12);
FFF(ddd, eee, aaa, bbb, ccc, X[ 4] , 9);
FFF(ccc, ddd, eee, aaa, bbb, X[ 1] , 12);
FFF(bbb, ccc, ddd, eee, aaa, X[ 5] , 5);
FFF(aaa, bbb, ccc, ddd, eee, X[ 8] , 14);
FFF(eee, aaa, bbb, ccc, ddd, X[ 7] , 6);
FFF(ddd, eee, aaa, bbb, ccc, X[ 6] , 8);
FFF(ccc, ddd, eee, aaa, bbb, X[ 2] , 13);
FFF(bbb, ccc, ddd, eee, aaa, X[13] , 6);
FFF(aaa, bbb, ccc, ddd, eee, X[14] , 5);
FFF(eee, aaa, bbb, ccc, ddd, X[ 0] , 15);
FFF(ddd, eee, aaa, bbb, ccc, X[ 3] , 13);
FFF(ccc, ddd, eee, aaa, bbb, X[ 9] , 11);
FFF(bbb, ccc, ddd, eee, aaa, X[11] , 11);

	/* combine results */
	ddd += cc + MDbuf[1];	/* final result for MDbuf[0] */
	MDbuf[1] = MDbuf[2] + dd + eee;
	MDbuf[2] = MDbuf[3] + ee + aaa;
	MDbuf[3] = MDbuf[4] + aa + bbb;
	MDbuf[4] = MDbuf[0] + bb + ccc;
	MDbuf[0] = ddd;

	return;
}


/*
* puts bytes from strptr into X and pad out; appends length
* and finally, compresses the last block(s)
* note: length in bits == 8 * (lswlen + 2^32 mswlen).
* note: there are (lswlen mod 64) bytes left in strptr.
*/
static void RMDfinish(dword *MDbuf, byte *strptr, dword lswlen, dword mswlen)
{
	dword i;	/* counter */
	dword X[16];	/* message words */

memset(X, 0, 16*sizeof(dword));

/* put bytes from strptr into X */
for (i=0; i<(lswlen&63); i++) {
/* byte i goes into word X[i div 4] at pos. 8*(i mod 4) */
X[i>>2] ^= (dword) *strptr++ << (8 * (i&3));
}

	/* append the bit m_n == 1 */
	X[(lswlen>>2)&15] ^= (dword)1 << (8*(lswlen&3) + 7);

	if ((lswlen & 63) > 55) {
	/* length goes to next block */
	RMDcompress(MDbuf, X);
	memset(X, 0, 16*sizeof(dword));
	}

	/* append length in bits*/
	X[14] = lswlen << 3;
	X[15] = (lswlen >> 29) | (mswlen << 3);
	RMDcompress(MDbuf, X);

	return;
}

/************************ end of file rmd160.c **********************/
/* Remainder provides common interface used by PGP library */


/*
* Shuffle the bytes into little-endian order within words, as per the
* RIPEMD-160 spec (which follows MD4 conventions).
*/
static void
rmd160ByteSwap(word32 *dest, byte const *src, unsigned words)
	{
		do {
		 *dest++ = (word32)((unsigned)src[3] << 8 | src[2]) << 16 |
			((unsigned)src[1] << 8 | src[0]);
		 src += 4;
		} while (--words);
	}


/* Initialize the RIPEMD-160 values */

static void
rmd160Init(void *priv)
{
	struct RIPEMD160Context *ctx = (struct RIPEMD160Context *)priv;

	/* Set the h-vars to their initial values */
	RMDinit (ctx->iv);

	/* Initialise bit count */
	ctx->bytesHi = 0;
	ctx->bytesLo = 0;
}

/* Update the RIPEMD-160 hash state for a block of data. */

static void
rmd160Update(void *priv, byte const *buf, size_t len)
{
 struct RIPEMD160Context *ctx = (struct RIPEMD160Context *)priv;
 unsigned i;

 /* Update bitcount */

 word32 t = ctx->bytesLo;
 if ( ( ctx->bytesLo = t + len ) < t )
  ctx->bytesHi++; /* Carry from low to high */

 i = (unsigned)t % RIPEMD160_BLOCKBYTES; /* Bytes already in ctx->key */

 /* i is always less than RIPEMD160_BLOCKBYTES. */
 if (RIPEMD160_BLOCKBYTES-i > len) {
  memcpy((byte *)ctx->key + i, buf, len);
  return;
 }

 if (i) { /* First chunk is an odd size */
  memcpy((byte *)ctx->key + i, buf, RIPEMD160_BLOCKBYTES - i);
  rmd160ByteSwap(ctx->key, (byte *)ctx->key, RIPEMD160_BLOCKWORDS);
  RMDcompress(ctx->iv, ctx->key);
  buf += RIPEMD160_BLOCKBYTES-i;
  len -= RIPEMD160_BLOCKBYTES-i;
 }

 /* Process data in 64-byte chunks */
 while (len >= RIPEMD160_BLOCKBYTES) {
  rmd160ByteSwap(ctx->key, buf, RIPEMD160_BLOCKWORDS);
  RMDcompress(ctx->iv, ctx->key);
  buf += RIPEMD160_BLOCKBYTES;
  len -= RIPEMD160_BLOCKBYTES;
 }

 /* Handle any remaining bytes of data. */
 if (len)
  memcpy(ctx->key, buf, len);
}


/* Final wrapup - MD4 style padding on last block. */

static byte const *
rmd160Final(void *priv)
{
 struct RIPEMD160Context *ctx = (struct RIPEMD160Context *)priv;
 byte *digest;
 int i;
 word32 t;

 RMDfinish(ctx->iv, (byte *)ctx->key, ctx->bytesLo, ctx->bytesHi);

 digest = (byte *)ctx->iv;
 for (i = 0; i < RIPEMD160_HASHWORDS; i++) {
  t = ctx->iv[i];
  digest[0] = (byte)t;
  digest[1] = (byte)(t >> 8);
  digest[2] = (byte)(t >> 16);
  digest[3] = (byte)(t >> 24);
  digest += 4;
 }
 /* In case it's sensitive */
/* XXX memset(ctx, 0, sizeof(ctx)); */
 return (byte const *)ctx->iv;
}



/*
* RIPEM OID is 1.3.36.3.2.1, from URL above.
* The rest of the format is stolen from MD5. Do we need the @#$@$
* NULL in there?
*/
static byte const RIPEMD160DERprefix[] = {
 0x30, /* Universal, Constructed, Sequence */
 0x21, /* Length 33 (bytes following) */
  0x30, /* Universal, Constructed, Sequence */
  0x09, /* Length 9 */
   0x06, /* Universal, Primitive, object-identifier */
   0x05, /* Length 8 */
    43, /* 43 = ISO(1)*40 + 3 */
    36,
    3,
    2,
    1,
   0x05, /* Universal, Primitive, NULL */
   0x00, /* Length 0 */
  0x04, /* Universal, Primitive, Octet string */
  0x14 /* Length 20 */
   /* 20 RIPEMD-160 digest bytes go here */
};

struct PgpHash const HashRIPEMD160 = {
 "RIPEMD160", PGP_HASH_RIPEMD160,
 RIPEMD160DERprefix, sizeof(RIPEMD160DERprefix),
 RIPEMD160_HASHBYTES,
 sizeof(struct RIPEMD160Context),
 sizeof(struct{char _a; struct RIPEMD160Context _b;}) -
  sizeof(struct RIPEMD160Context),
 rmd160Init, rmd160Update, rmd160Final
};


#if TESTMAIN

/* --------------------------- RMD160 Test code --------------------------- */
#include <stdio.h>
#include <stdlib.h> /* For exit() */
#include <time.h>

/* Size of buffer for RMD160 speed test data */

#define TEST_BLOCK_SIZE	( RIPEMD160_HASHBYTES * 100 )

/* Number of bytes of test data to process */

#define TEST_BYTES 10000000L
#define TEST_BLOCKS	( TEST_BYTES / TEST_BLOCK_SIZE )

static char const *rmd160TestResults[] = {
		"9C1185A5C5E9FC54612808977EE8F548B2258D31",	/* "" */
		"0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE",	/* "a" */
		"8EB208F7E05D987A9B044A8E98C6B087F15A0BFC",	/* "abc" */
		"5D0689EF49D2FAE572B881B123A85FFA21595F36",	/* "message digest" */
		"F71C27109C692C1B56BBDCEB5B9D2865B3708DBC",	/* "a..z" */
		"12A053384A9C0C88E405A06C27DCF49ADA62EB2B",	/* "abcdbcde...nopq" */
		"B0E20B6E3116640286ED3A87A5713079B21F5189",	/* "A..Za..z0..9" */
		"9B752E45573D4B39F4DBD3323CAB82BF63326BFB",	/* 8 * "1234567890" */
		"52783243C1697BDBE16D37F97F68F08325DC1528",	/* 1,000,000 "a" */
		"52783243C1697BDBE16D37F97F68F08325DC1528",	/* 1,000,000 "a" */
		"52783243C1697BDBE16D37F97F68F08325DC1528" };	/* 1,000,000 "a" */

static int
compareRMD160results(byte const *hash, int level)
{
 char buf[41];
 int i;

 for (i = 0; i < RIPEMD160_HASHBYTES; i++)
  sprintf(buf+2*i, "%02X", hash[i]);

 if (strcmp(buf, rmd160TestResults[level-1]) == 0) {
  printf("Test %d passed, result = %s\n", level, buf);
  return 0;
 } else {
  printf("Error in RMD160 implementation: Test %d failed\n", level);
  printf("  Result = %s\n", buf);
  printf("Expected = %s\n", rmd160TestResults[level-1]);
  return -1;
 }
}


int
main(void)
{
 struct RIPEMD160Context rmd160;
 byte data[TEST_BLOCK_SIZE];
 byte const *hash;
 clock_t ticks;
 long i;

 /*
 * Test output data, based on URL above.
 */
 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"", 0);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 1) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"a", 1);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 2) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"abc", 3);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 3) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"message digest", 14);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 4) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"abcdefghijklmnopqrstuvwxyz", 26);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 5) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"abcdbcdecdefdefgefghfghighijhijkijkl\
jklmklmnlmnomnopnopq", 56);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 6) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz0123456789", 62);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 7) < 0)
  exit (-1);

 rmd160Init(&rmd160);
 rmd160Update(&rmd160, (byte *)"1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890", 80);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 8) < 0)
  exit (-1);

 /* 1,000,000 bytes of ASCII 'a' (0x61), by 64's */
 rmd160Init(&rmd160);
 for (i = 0; i < 15625; i++)
  rmd160Update(&rmd160, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 9) < 0)
  exit (-1);

 /* 1,000,000 bytes of ASCII 'a' (0x61), by 25's */
 rmd160Init(&rmd160);
 for (i = 0; i < 40000; i++)
  rmd160Update(&rmd160, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa", 25);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 10) < 0)
  exit (-1);

 /* 1,000,000 bytes of ASCII 'a' (0x61), by 125's */
 rmd160Init(&rmd160);
 for (i = 0; i < 8000; i++)
  rmd160Update(&rmd160, (byte *)"aaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 125);
 hash = rmd160Final(&rmd160);
 if (compareRMD160results(hash, 11) < 0)
  exit (-1);

 /* Now perform time trial, generating MD for 10MB of data. First,
 initialize the test data */
 memset(data, 0, TEST_BLOCK_SIZE);

 /* Get start time */
 printf("RMD160 time trial.  Processing %ld characters...\n", TEST_BYTES);
 ticks = clock();

 /* Calculate RMD160 message digest in TEST_BLOCK_SIZE byte blocks */
 rmd160Init(&rmd160);
 for (i = TEST_BLOCKS; i > 0; i--)
  rmd160Update(&rmd160, data, TEST_BLOCK_SIZE);
 hash = rmd160Final(&rmd160);

 /* Get finish time and print difference */
 ticks = clock() - ticks;
 printf("Ticks to process test input: %lu\n", (unsigned long)ticks);

 return 0;
}
#endif /* Test driver */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
