/*
 * pgpRC2.c - C source code for RC2(TM) block cipher.
 * Based on RFC 2268.
 *
 * RC2 is a trademark of RSA Data Security, Inc.
 * 
 * $Id: pgpRC2.c,v 1.3 1999/05/18 23:29:00 hal Exp $
 *
 */
 
#include "pgpSDKBuildFlags.h"

#include "string.h"
#include "pgpConfig.h"

#include "pgpSymmetricCipherPriv.h"
#include "pgpRC2.h"
#include "pgpMem.h"

/* The size of a scheduled RC2 key */
#define RC2_KEYSHORTS 64
#define RC2_KEYBYTES (sizeof(PGPUInt16)*RC2_KEYSHORTS)

static const PGPByte pitable[256] = {
	0xd9, 0x78, 0xf9, 0xc4, 0x19, 0xdd, 0xb5, 0xed,
	0x28, 0xe9, 0xfd, 0x79, 0x4a, 0xa0, 0xd8, 0x9d,
	0xc6, 0x7e, 0x37, 0x83, 0x2b, 0x76, 0x53, 0x8e,
	0x62, 0x4c, 0x64, 0x88, 0x44, 0x8b, 0xfb, 0xa2,
	0x17, 0x9a, 0x59, 0xf5, 0x87, 0xb3, 0x4f, 0x13,
	0x61, 0x45, 0x6d, 0x8d, 0x09, 0x81, 0x7d, 0x32,
	0xbd, 0x8f, 0x40, 0xeb, 0x86, 0xb7, 0x7b, 0x0b,
	0xf0, 0x95, 0x21, 0x22, 0x5c, 0x6b, 0x4e, 0x82,
	0x54, 0xd6, 0x65, 0x93, 0xce, 0x60, 0xb2, 0x1c,
	0x73, 0x56, 0xc0, 0x14, 0xa7, 0x8c, 0xf1, 0xdc,
	0x12, 0x75, 0xca, 0x1f, 0x3b, 0xbe, 0xe4, 0xd1,
	0x42, 0x3d, 0xd4, 0x30, 0xa3, 0x3c, 0xb6, 0x26,
	0x6f, 0xbf, 0x0e, 0xda, 0x46, 0x69, 0x07, 0x57,
	0x27, 0xf2, 0x1d, 0x9b, 0xbc, 0x94, 0x43, 0x03,
	0xf8, 0x11, 0xc7, 0xf6, 0x90, 0xef, 0x3e, 0xe7,
	0x06, 0xc3, 0xd5, 0x2f, 0xc8, 0x66, 0x1e, 0xd7,
	0x08, 0xe8, 0xea, 0xde, 0x80, 0x52, 0xee, 0xf7,
	0x84, 0xaa, 0x72, 0xac, 0x35, 0x4d, 0x6a, 0x2a,
	0x96, 0x1a, 0xd2, 0x71, 0x5a, 0x15, 0x49, 0x74,
	0x4b, 0x9f, 0xd0, 0x5e, 0x04, 0x18, 0xa4, 0xec,
	0xc2, 0xe0, 0x41, 0x6e, 0x0f, 0x51, 0xcb, 0xcc,
	0x24, 0x91, 0xaf, 0x50, 0xa1, 0xf4, 0x70, 0x39,
	0x99, 0x7c, 0x3a, 0x85, 0x23, 0xb8, 0xb4, 0x7a,
	0xfc, 0x02, 0x36, 0x5b, 0x25, 0x55, 0x97, 0x31,
	0x2d, 0x5d, 0xfa, 0x98, 0xe3, 0x8a, 0x92, 0xae,
	0x05, 0xdf, 0x29, 0x10, 0x67, 0x6c, 0xba, 0xc9,
	0xd3, 0x00, 0xe6, 0xcf, 0xe1, 0x9e, 0xa8, 0x2c,
	0x63, 0x16, 0x01, 0x3f, 0x58, 0xe2, 0x89, 0xa9,
	0x0d, 0x38, 0x34, 0x1b, 0xab, 0x33, 0xff, 0xb0,
	0xbb, 0x48, 0x0c, 0x5f, 0xb9, 0xb1, 0xcd, 0x2e,
	0xc5, 0xf3, 0xdb, 0x47, 0xe5, 0xa5, 0x9c, 0x77,
	0x0a, 0xa6, 0x20, 0x68, 0xfe, 0x7f, 0xc1, 0xad
};

/*
 * Expand a variable-length user key to a 64-short working rc2 key.
 * We work with buffer as little endian, then at the end convert to
 * native endianness.
 *
 * The RFC refers to klen as T, and to effbits as T1.  The xkey array is K,
 * and the byte-oriented lkey alias to it is L.
 */
static void
RC2schedule(PGPUInt16 xkey[RC2_KEYSHORTS], PGPByte const *k, PGPSize klen,
	PGPSize effbits)
{
	PGPInt32 i;
	PGPSize effbytes;
	PGPByte tm;
	PGPByte *lkey = (PGPByte *)xkey;

	/* First put bytes of key into first bytes of buffer */
	pgpCopyMemory(k, xkey, klen);

	/* Calculate effective key length in bytes and mask tm */
	effbytes = (effbits + 7) / 8;
	tm = (PGPByte) (255 >> ((-effbits) & 7));

	/* Do key expansion */
	for (i=klen; i<(PGPInt32)RC2_KEYBYTES; ++i)
		lkey[i] = pitable[(lkey[i-1] + lkey[i-klen]) & 0xff];

	lkey[128-effbytes] = pitable[lkey[128-effbytes] & tm];

	for (i=RC2_KEYBYTES-1-effbytes; i>=0; --i)
		lkey[i] = pitable[lkey[i+1] ^ lkey[i+effbytes]];

	/* Convert to little-endian format */
	for (i=0; i<RC2_KEYSHORTS; ++i)
		xkey[i] = lkey[2*i] + 256*lkey[2*i+1];
}


#define ROL(x,r) ((x)<<(r) | (x)>>(16-(r)))
#define SEL(a,b,c) (((a)&(b)) | (~(a)&(c)))

/*
 * Encrypt the 8 bytes at *in into the 8 bytes at *out using the expanded
 * key schedule from *xkey.
 */
static void
RC2encrypt(PGPByte const *in, PGPByte *out, PGPUInt16 const *xkey)
{
	PGPUInt16 r0, r1, r2, r3;
	PGPInt32 i, j;

	/* Convert to little-endian shorts */
	r0 = (in[1] << 8) + in[0];
	r1 = (in[3] << 8) + in[2];
	r2 = (in[5] << 8) + in[4];
	r3 = (in[7] << 8) + in[6];

	j = 0;
	for (i = 0; i < 16; i++) {
		/* Do a mixing round */
		r0 += SEL(r3,r2,r1) + xkey[j++];
		r0 = ROL(r0, 1);

		r1 += SEL(r0,r3,r2) + xkey[j++];
		r1 = ROL(r1, 2);

		r2 += SEL(r1,r0,r3) + xkey[j++];
		r2 = ROL(r2, 3);

		r3 += SEL(r2,r1,r0) + xkey[j++];
		r3 = ROL(r3, 5);

		/* Do a mashing round after 5th and 11th mixing rounds */
		if (i == 4 || i == 10) {
			r0 += xkey[r3 & 63];
			r1 += xkey[r0 & 63];
			r2 += xkey[r1 & 63];
			r3 += xkey[r2 & 63];
		}
	}

	/* Convert back to output array */
	out[0] = (PGPByte)r0;
	out[1] = (PGPByte)(r0 >> 8);
	out[2] = (PGPByte)r1;
	out[3] = (PGPByte)(r1 >> 8);
	out[4] = (PGPByte)r2;
	out[5] = (PGPByte)(r2 >> 8);
	out[6] = (PGPByte)r3;
	out[7] = (PGPByte)(r3 >> 8);
}

/*
 * Decrypt the 8 bytes at *in into the 8 bytes at *out using the expanded
 * key schedule from *xkey.
 */
static void
RC2decrypt(PGPByte const *in, PGPByte *out, PGPUInt16 const *xkey)
{
	PGPUInt16 r3, r2, r1, r0;
	PGPInt32 i, j;

	r0 = (in[1] << 8) + in[0];
	r1 = (in[3] << 8) + in[2];
	r2 = (in[5] << 8) + in[4];
	r3 = (in[7] << 8) + in[6];

	j = RC2_KEYSHORTS-1;
	for (i=15; i>=0; --i) {
		/* Do a r-mixing round */
		r3 = ROL(r3, 11);
		r3 -= SEL(r2,r1,r0) + xkey[j--];

		r2 = ROL(r2, 13);
		r2 -= SEL(r1,r0,r3) + xkey[j--];

		r1 = ROL(r1, 14);
		r1 -= SEL(r0,r3,r2) + xkey[j--];

		r0 = ROL(r0, 15);
		r0 -= SEL(r3,r2,r1) + xkey[j--];

		/* Do a r-mashing round after 5 and 11 r-mixing rounds */
		if (i == 5 || i == 11) {
			r3 -= xkey[r2 & 63];
			r2 -= xkey[r1 & 63];
			r1 -= xkey[r0 & 63];
			r0 -= xkey[r3 & 63];
		}
	}

	out[0] = (PGPByte)r0;
	out[1] = (PGPByte)(r0 >> 8);
	out[2] = (PGPByte)r1;
	out[3] = (PGPByte)(r1 >> 8);
	out[4] = (PGPByte)r2;
	out[5] = (PGPByte)(r2 >> 8);
	out[6] = (PGPByte)r3;
	out[7] = (PGPByte)(r3 >> 8);
}


/*
 * Exported functions
 */

/* Expand 40 bit key into expanded key *priv, 40 effective bits */
static void
rc2_40Key(void *priv, void const *key)
{
	RC2schedule((PGPUInt16 *)priv, (const PGPByte *) key, 40/8, 40);
}

/* Expand 128 bit key into expanded key *priv, full key length */
static void
rc2_128Key(void *priv, void const *key)
{
	RC2schedule((PGPUInt16 *)priv, (const PGPByte *) key, 128/8, 1024);
}

/* Encrypt 8 bytes at *in into 8 bytes at *out using key sched from *priv */
static void
rc2Encrypt(void *priv, void const *in, void *out)
{
	RC2encrypt((const PGPByte *) in, (PGPByte *) out, (PGPUInt16 *)priv);
}

/* Decrypt 8 bytes at *in into 8 bytes at *out using key sched from *priv */
static void
rc2Decrypt(void *priv, void const *in, void *out)
{
	RC2decrypt( (const PGPByte *) in, (PGPByte *) out, (PGPUInt16 *)priv);
}




/*
 * Define a Cipher for the generic cipher.  This is the only
 * real exported thing -- everything else can be static, since everything
 * is referenced through function pointers!
 */
PGPCipherVTBL const cipherRC2_40 =
{
	"RC2_40",
	(PGPCipherAlgorithm)kPGPCipherAlgorithm_RC2_40,
	8,			/* Blocksize */
	5,			/* Keysize */
	RC2_KEYBYTES,				/* Priv array size */
	alignof(PGPUInt32),
	rc2_40Key,
	NULL,						/* Decryption only, for PKCS-12 imports */
	rc2Decrypt,
	NULL
};

PGPCipherVTBL const cipherRC2_128 =
{
	"RC2_128",
	(PGPCipherAlgorithm)kPGPCipherAlgorithm_RC2_128,
	8,			/* Blocksize */
	16,			/* Keysize */
	RC2_KEYBYTES,				/* Priv array size */
	alignof(PGPUInt32),
	rc2_128Key,
	rc2Encrypt,
	rc2Decrypt,
	NULL
};





#if UNITTEST




/* Test vectors */

PGPUInt32 eff1 = 63;
PGPByte K1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte P1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C1[] = {
	0xeb, 0xb7, 0x73, 0xf9, 0x93, 0x27, 0x8e, 0xff
};


PGPUInt32 eff2 = 64;
PGPByte K2[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
PGPByte P2[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
PGPByte C2[] = {
	0x27, 0x8b, 0x27, 0xe4, 0x2e, 0x2f, 0x0d, 0x49
};


PGPUInt32 eff3 = 64;
PGPByte K3[] = {
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte P3[] = {
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
PGPByte C3[] = {
	0x30, 0x64, 0x9e, 0xdf, 0x9b, 0xe7, 0xd2, 0xc2
};


PGPUInt32 eff4 = 64;
PGPByte K4[] = {
	0x88
};
PGPByte P4[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C4[] = {
	0x61, 0xa8, 0xa2, 0x44, 0xad, 0xac, 0xcc, 0xf0
};


PGPUInt32 eff5 = 64;
PGPByte K5[] = {
	0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a
};
PGPByte P5[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C5[] = {
	0x6c, 0xcf, 0x43, 0x08, 0x97, 0x4c, 0x26, 0x7f
};


PGPUInt32 eff6 = 64;
PGPByte K6[] = {
	0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x7f,
	0x0f, 0x79, 0xc3, 0x84, 0x62, 0x7b, 0xaf, 0xb2
};
PGPByte P6[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C6[] = {
	0x1a, 0x80, 0x7d, 0x27, 0x2b, 0xbe, 0x5d, 0xb1
};


PGPUInt32 eff7 = 128;
PGPByte K7[] = {
	0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x7f,
	0x0f, 0x79, 0xc3, 0x84, 0x62, 0x7b, 0xaf, 0xb2
};
PGPByte P7[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C7[] = {
	0x22, 0x69, 0x55, 0x2a, 0xb0, 0xf8, 0x5c, 0xa6
};


PGPUInt32 eff8 = 129;
PGPByte K8[] = {
	0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x7f,
	0x0f, 0x79, 0xc3, 0x84, 0x62, 0x7b, 0xaf, 0xb2,
	0x16, 0xf8, 0x0a, 0x6f, 0x85, 0x92, 0x05, 0x84,
	0xc4, 0x2f, 0xce, 0xb0, 0xbe, 0x25, 0x5d, 0xaf,
	0x1e
};
PGPByte P8[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte C8[] = {
	0x5b, 0x78, 0xd3, 0xa4, 0x3d, 0xff, 0xf1, 0xf1
};


int
main(void)
{	/* Test driver for CAST cipher */
	PGPByte priv[RC2_KEYBYTES];
	PGPByte X[8], Y[8];

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K1, sizeof(K1), eff1);
	rc2Encrypt(priv, P1, X);
	if (memcmp(C1, X, sizeof(X)) == 0)
		printf ("Encryption test 1 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 1\n");
	rc2Decrypt(priv, C1, Y);
	if (memcmp(P1, Y, sizeof(Y)) == 0)
		printf ("Decryption test 1 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 1\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K2, sizeof(K2), eff2);
	rc2Encrypt(priv, P2, X);
	if (memcmp(C2, X, sizeof(X)) == 0)
		printf ("Encryption test 2 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 2\n");
	rc2Decrypt(priv, C2, Y);
	if (memcmp(P2, Y, sizeof(Y)) == 0)
		printf ("Decryption test 2 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 2\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K3, sizeof(K3), eff3);
	rc2Encrypt(priv, P3, X);
	if (memcmp(C3, X, sizeof(X)) == 0)
		printf ("Encryption test 3 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 3\n");
	rc2Decrypt(priv, C3, Y);
	if (memcmp(P3, Y, sizeof(Y)) == 0)
		printf ("Decryption test 3 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 3\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K4, sizeof(K4), eff4);
	rc2Encrypt(priv, P4, X);
	if (memcmp(C4, X, sizeof(X)) == 0)
		printf ("Encryption test 4 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 4\n");
	rc2Decrypt(priv, C4, Y);
	if (memcmp(P4, Y, sizeof(Y)) == 0)
		printf ("Decryption test 4 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 4\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K5, sizeof(K5), eff5);
	rc2Encrypt(priv, P5, X);
	if (memcmp(C5, X, sizeof(X)) == 0)
		printf ("Encryption test 5 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 5\n");
	rc2Decrypt(priv, C5, Y);
	if (memcmp(P5, Y, sizeof(Y)) == 0)
		printf ("Decryption test 5 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 5\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K6, sizeof(K6), eff6);
	rc2Encrypt(priv, P6, X);
	if (memcmp(C6, X, sizeof(X)) == 0)
		printf ("Encryption test 6 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 6\n");
	rc2Decrypt(priv, C6, Y);
	if (memcmp(P6, Y, sizeof(Y)) == 0)
		printf ("Decryption test 6 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 6\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K7, sizeof(K7), eff7);
	rc2Encrypt(priv, P7, X);
	if (memcmp(C7, X, sizeof(X)) == 0)
		printf ("Encryption test 7 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 7\n");
	rc2Decrypt(priv, C7, Y);
	if (memcmp(P7, Y, sizeof(Y)) == 0)
		printf ("Decryption test 7 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 7\n");

	RC2schedule((PGPUInt16 *)priv, (const PGPByte *)K8, sizeof(K8), eff8);
	rc2Encrypt(priv, P8, X);
	if (memcmp(C8, X, sizeof(X)) == 0)
		printf ("Encryption test 8 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 8\n");
	rc2Decrypt(priv, C8, Y);
	if (memcmp(P8, Y, sizeof(Y)) == 0)
		printf ("Decryption test 8 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 8\n");




	return 0;	/* normal exit */
} /* main */

#endif /* UNITTEST */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
