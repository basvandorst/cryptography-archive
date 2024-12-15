/*
 * This code implements the MD2 message-digest algorithm.
 * The algorithm is due to Ron Rivest, documented in RFC1319.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD2Context structure, pass it to MD2Init, call MD2Update as
 * needed on buffers full of bytes, and then call MD2Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * $Id: pgpMD2.c,v 1.4 1998/08/05 05:36:54 heller Exp $
 */

#include "pgpConfig.h"

#include "pgpHash.h"
#include "pgpMD2.h"
#include "pgpUsuals.h"
#include "pgpMem.h"
#include "pgpDebug.h"
#include "pgpMSRSAGlue.h"


/* MD2 size parameters */

#define MD2_BLOCKBYTES	16
#define MD2_BLOCKWORDS	4
#define MD2_ITERATIONS	18
#define MD2_HASHBYTES	16


#if PGP_USECAPIFORMD2	/* [ */

/*
 * Due to bugs in CAPI, we can't set the hash value for MD2 hash.
 * Therefore we will go with a very limited implementation which can
 * only verify MD2 signatures, and even then only in simple cases.
 *
 * We will not return a true MD2 hash when asked, but rather we will
 * return a pointer to the hProv and hHash handles.  This will be used
 * by the CAPI verification code to do the actual verification.  That
 * code will be responsible for disposing of the handles.
 */

#include "pgpPubKey.h"

/* Shorthand for accessing CAPI externs */
#define e pgp_CAPI_externs

extern CAPIEntries pgp_CAPI_externs;

typedef struct MD2Context {
	HCRYPTPROV			hProv;			/* Handle to CAPI provider */
	HCRYPTHASH			hHash;			/* Handle to CAPI hash object */
	DEBUG_STRUCT_CONSTRUCTOR( MD2Context )
} MD2Context;

static void
MD2Init(void *priv)
{
	MD2Context *ctx = (MD2Context *)priv;

	pgpClearMemory (ctx, sizeof(*ctx));

	/* Verify CAPI loadability */
	if (!(pgpCAPIuse() & PGP_PKUSE_SIGN))
		return;			/* Unusable */

	/* Acquire context */
	if (!e.CryptAcquireContext(&ctx->hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
								CRYPT_VERIFYCONTEXT)) {
		pgpClearMemory (ctx, sizeof(*ctx));
		return;
	}

	/* Create hash */
	if (!e.CryptCreateHash (ctx->hProv, CALG_MD2, 0, 0, &ctx->hHash)) {
		e.CryptReleaseContext(ctx->hProv, 0);
		pgpClearMemory (ctx, sizeof(*ctx));
		return;
	}
}

static void
MD2Update(void *priv, void const *bufIn, PGPSize len)
{
	MD2Context 	*ctx = (MD2Context *)priv;

	e.CryptHashData (ctx->hHash, bufIn, len, 0);
}

/* Instead of returning hash, return pointer to hProv/hHash */
static void const *
MD2Final(void *priv)
{
	return priv;
}

#else					/* ] PGP_USECAPIFORMD2 [ */


#define DATA(ctx) ((ctx)->X+MD2_BLOCKBYTES)

typedef struct MD2Context {
	PGPByte csum[MD2_BLOCKBYTES];	/* Accumulated checksum */
	PGPByte X[3*MD2_BLOCKBYTES];	/* X array which gets crunched */
	PGPByte data[MD2_BLOCKBYTES];	/* accumulate data up to 1 block */
	PGPUInt32 bytesLo;
	DEBUG_STRUCT_CONSTRUCTOR( MD2Context )
} MD2Context;

/* Substitution table, a permutation of 0..255 based on PI */
static PGPByte S[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};


/*
 * Start MD2 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void
MD2Init(void *priv)
{
	MD2Context *ctx = (MD2Context *)priv;

	pgpClearMemory (ctx, sizeof(*ctx));
}

/*
 * The core of the MD2 algorithm, this alters an existing MD2 hash to
 * reflect the addition of 16 bytes of new data.
 * The incoming data is already in place in X+MD2_BLOCKBYTES.
 */
static void
MD2Transform(PGPByte *X, PGPByte *csum)
{
	PGPByte t;
	PGPUInt32 j, k;
	PGPUInt32 *XW = (PGPUInt32 *)X;

	if (csum) {
		/* Update checksum */
		t = csum[MD2_BLOCKBYTES-1];
		for (j=0; j<MD2_BLOCKBYTES; ++j) {
			/* Note that RFC1319 says "=" not "^=", but that is wrong */
			t = csum[j] ^= S[ X[MD2_BLOCKBYTES + j] ^ t ];
		}
	}

	/* XOR to produce third part of X; shortcut doing 32 bits at once */
	for (j=0; j<MD2_BLOCKWORDS; ++j)
		XW[2*MD2_BLOCKWORDS + j] = XW[j] ^ XW[MD2_BLOCKWORDS + j];

	t = 0;
	for (j=0; j<MD2_ITERATIONS; ++j) {
		for (k=0; k<3*MD2_BLOCKBYTES; ++k) {
			t = X[k] ^= S[t];
		}
		t = (t + j) & 0xff;
	}
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void
MD2Update(void *priv, void const *bufIn, PGPSize len)
{
	MD2Context 	*ctx = (MD2Context *)priv;
	PGPByte		*buf = (PGPByte *) bufIn;
	unsigned 	i;

	PGPUInt32 t = ctx->bytesLo;
	ctx->bytesLo = t + len;
	i = (unsigned)t % MD2_BLOCKBYTES;	/* Bytes already in ctx->key */

	if (MD2_BLOCKBYTES-i > len) {
		/* Insufficient data to fill out a block */
		pgpCopyMemory (buf, DATA(ctx)+i, len);
		return;
	}

	if (i) {	/* First chunk is an odd size */
		pgpCopyMemory (buf, DATA(ctx) + i, MD2_BLOCKBYTES - i);
		MD2Transform(ctx->X, ctx->csum);
		buf += MD2_BLOCKBYTES-i;
		len -= MD2_BLOCKBYTES-i;
	}

	/* Process data in 16-byte chunks */
	while (len >= MD2_BLOCKBYTES) {
		pgpCopyMemory (buf, DATA(ctx), MD2_BLOCKBYTES);
		MD2Transform(ctx->X, ctx->csum);
		buf += MD2_BLOCKBYTES;
		len -= MD2_BLOCKBYTES;
	}

	/* Handle any remaining bytes of data. */
	if (len)
		pgpCopyMemory (buf, DATA(ctx), len);
}

/*
 * Final wrapup - pad and add checksum.
 */
static void const *
MD2Final(void *priv)
{
	MD2Context *ctx = (MD2Context *)priv;
	unsigned i = (unsigned)ctx->bytesLo % MD2_BLOCKBYTES;
	unsigned j = i;

	/* Fill the second to last block with padding */
	for ( ; i<MD2_BLOCKBYTES; ++i) {
		DATA(ctx)[i] = 16 - j;
	}
	MD2Transform(ctx->X, ctx->csum);

	/* Finally transform the checksum as the last block */
	pgpCopyMemory (ctx->csum, DATA(ctx), MD2_BLOCKBYTES);
	MD2Transform(ctx->X, NULL);

	/* In case it's sensitive */
/* XXX	memset(DATA(ctx), 0, 2*MD2_BLOCKBYTES); */
	return (PGPByte const *)ctx->X;
}


#endif /* ] PGP_USECAPIFORMD2 */


/*
 * The Basic Encoding Rules (of which the Distinguished Encoding Rules
 * are a simple minimal-sized subset) are supposed to be compact.  Humph.
 */
static PGPByte const MD2DERprefix[] = {
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
				0x02, /* 2 = MD2 */
			0x05, /* Universal, Primitive, NULL */
			0x00, /* Length 0 */
		0x04, /* Universal, Primitive, Octet string */
		0x10 /* Length 16 */
			/* 16 MD2 digest bytes go here */
};

PGPHashVTBL const HashMD2 = {
	"MD2", (PGPHashAlgorithm) kPGPHashAlgorithm_MD2,
	MD2DERprefix, sizeof(MD2DERprefix),
	MD2_HASHBYTES,
	sizeof(MD2Context),
	sizeof(struct{char _a; MD2Context _b;}) -
		sizeof(MD2Context),
	MD2Init, MD2Update, MD2Final
};

#if TESTMAIN	/* [ */

#include <stdio.h>
#include <string.h>


static char const * const md2TestResults[] = {
	"8350e5a3e24c153df2275c9f80692773",
	"32ec01ec4a6dac72c0ab96fb34c0b5d1",
	"da853b0d3f88d99b30283a69e6ded6bb",
	"ab4f496bfb2a530b219ff33031fe06b0",
	"4e8ddff3650292ab5a4108c3aa47940b",
	"da33def2a42df13975352846c30338cd",
	"d5976f79d83d3a0dc9806c3c66f3efd8"
};

static char const * const md2TestValues[] = {
	"",
	"a",
	"abc",
	"message digest",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"1234567890123456789012345678901234567890" /* No comma here */
	"1234567890123456789012345678901234567890"
};

static int
compareMD2results(PGPByte const *hash, int level)
{
	char buf[33];
	unsigned i;

	for (i = 0; i < MD2_HASHBYTES; i++)
		sprintf(buf+2*i, "%02x", hash[i]);
	if (strcmp(buf, md2TestResults[level]) == 0) {
		printf("Test %d passed, result = %s\n", level+1, buf);
		return 0;
	} else {
		printf("Error in MD2 implementation; test %d failed\n", level+1);
		printf("  Result = %s\nExpected = %s\n", buf,
			md2TestResults[level]);
		return -1;
	}
}

int
main(void)
{
	MD2Context md2;
	PGPByte const *hash;
	unsigned i;

	for (i=0; i<sizeof(md2TestValues)/sizeof(md2TestValues[0]); ++i) {
		MD2Init(&md2);
		MD2Update(&md2, md2TestValues[i], strlen(md2TestValues[i]));
		hash = MD2Final(&md2);
		if (compareMD2results(hash, i) < 0)
			return -1;	
}
	return 0;
}

#endif /* ] TESTMAIN */
