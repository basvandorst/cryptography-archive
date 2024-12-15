/*
 * pgpIDEA.c - C source code for IDEA block cipher.
 * Algorithm developed by Xuejia Lai and James L. Massey, of ETH Zurich.
 *
 * $Id: pgpIDEA.c,v 1.2 1998/12/04 17:20:13 dgal Exp $
 *
 * There are two adjustments that can be made to this code to speed it
 * up.  Defaults may be used for PCs.  Only the -DIDEA32 pays off
 * significantly if selectively set or not set.  Experiment to see what
 * works best for your machine.
 *
 * Multiplication: default is inline, -DAVOID_JUMPS uses a different
 * 	version that does not do any conditional jumps (a few percent
 * 	worse on a SPARC, better on other machines), while
 * 	-DSMALL_CACHE takes it out of line to stay within a small
 * 	on-chip code cache.  (Not really applicable with current L1
 * 	cache sizes.)
 * Variables: normally, 16-bit variables are used, but some machines do
 * 	not have 16-bit registers, so they do a great deal of masking.
 * 	-DUSE_IDEA32 uses "int" register variables and masks explicitly
 * 	only where necessary.  On a SPARC, for example, this boosts
 * 	performance by 30%.
 *
 * The IDEA(tm) block cipher is covered by a patent held by ETH and a
 * Swiss company called Ascom-Tech AG.  The Swiss patent number is
 * PCT/CH91/00117.  International patents are pending. IDEA(tm) is a
 * trademark of Ascom-Tech AG.  There is no license fee required for
 * noncommercial use.  Commercial users may obtain licensing details from
 * Dieter Profos, Ascom Tech AG, Solothurn Lab, Postfach 151, 4502
 * Solothurn, Switzerland, Tel +41 65 242885, Fax +41 65 235761.
 *
 * The IDEA block cipher uses a 64-bit block size, and a 128-bit key 
 * size.  It breaks the 64-bit cipher block into four 16-bit words
 * because all of the primitive inner operations are done with 16-bit 
 * arithmetic.  It likewise breaks the 128-bit cipher key into eight 
 * 16-bit words.
 *
 * For further information on the IDEA cipher, see these papers:
 * 1) Xuejia Lai, "Detailed Description and a Software Implementation of 
 *    the IPES Cipher", Institute for Signal and Information
 *    Processing, ETH-Zentrum, Zurich, Switzerland, 1991
 * 2) Xuejia Lai, James L. Massey, Sean Murphy, "Markov Ciphers and 
 *    Differential Cryptanalysis", Advances in Cryptology - EUROCRYPT'91
 *
 * This code runs on arrays of bytes by taking pairs in big-endian order
 * to make the 16-bit words that IDEA uses internally.  This produces the
 * same result regardless of the byte order of the native CPU.
 */
 
#include "pgpConfig.h"

#include "pgpSymmetricCipherPriv.h"
#include "pgpIDEA.h"
#include "pgpMem.h"
#include "pgpUsuals.h"

/* If IDEA32 isn't predefined as 1 or 0, make a guess. */
#ifndef USE_IDEA32
#if UINT_MAX > 0xffff
#define USE_IDEA32 1
#endif
#endif

#if USE_IDEA32	/* Use >16-bit temporaries */
#define low16(x) ((x) & 0xFFFF)
typedef unsigned int uint16;	/* at LEAST 16 bits, maybe more */
#else
#define low16(x) (uint16)(x)
typedef PGPUInt16 uint16;
#endif

/* A few handy definitions */
#define IDEA_ROUNDS 8
#define IDEA_KEYLEN (6*IDEA_ROUNDS+4)
#define IDEA_KEYBYTES (sizeof(PGPUInt16) * IDEA_KEYLEN)


/*
 * Flags in priv array to record whether key schedule is in encrypt
 * or decrypt mode
 */
#define IDEA_ENCRYPTION_MODE	0x11
#define IDEA_DECRYPTION_MODE	0x22

/* Private functions */

/* Expand a 128-bit user key to a working encryption key EK */

static void
ideaExpandKey(PGPByte const *userkey, PGPUInt16 *EK)
{
	int i, j;

	for (j=0; j<8; j++) {
		EK[j] = (userkey[0]<<8) + userkey[1];
		userkey += 2;
	}
	for (i=0; j < IDEA_KEYLEN; j++) {
		i++;
		EK[i+7] = EK[i & 7] << 9 | EK[(i+1) & 7] >> 7;
		EK += i & 8;
		i &= 7;
	}
} /* ideaExpandKey */

/*
 * Compute the multiplicative inverse of x, modulo 65537, using Euclid's
 * algorithm. It is unrolled twice to avoid swapping the registers each
 * iteration, and some subtracts of t have been changed to adds.
 */
static uint16
mulInv(uint16 x)     
{
	uint16 t0, t1;
	uint16 q, y;

	if (x <= 1)
		return x;	/* 0 and 1 are self-inverse */
	t1 = 0x10001L / x;	/* Since x >= 2, this fits into 16 bits */
	y = 0x10001L % x;
	if (y == 1)
		return low16(1-t1);
	t0 = 1;
	do {
		q = x / y;
		x = x % y;
		t0 += q * t1;
		if (x == 1)
			return t0;
		q = y / x;
		y = y % x;
		t1 += q * t0;
	} while (y != 1);
	return low16(1-t1);
} /* mulInv */

/*
 * Compute IDEA decryption key DK from an expanded IDEA encryption key EK
 * Note that the input and output may be the same.  Thus, the key is
 * inverted into an internal buffer, and then copied to the output.
 */
static void
ideaInvertKey(PGPUInt16 const EK[IDEA_KEYLEN], PGPUInt16 DK[IDEA_KEYLEN])
{
	int i;
	uint16 t1, t2, t3;
	PGPUInt16 temp[IDEA_KEYLEN];
	PGPUInt16 *p = temp + IDEA_KEYLEN;

	t1 = mulInv(*EK++);
	t2 = -*EK++;
	t3 = -*EK++;
	*--p = mulInv(*EK++);
	*--p = t3;
	*--p = t2;
	*--p = t1;

	for (i = 0; i < IDEA_ROUNDS-1; i++) {
		t1 = *EK++;
		*--p = *EK++;
		*--p = t1;

		t1 = mulInv(*EK++);
		t2 = -*EK++;
		t3 = -*EK++;
		*--p = mulInv(*EK++);
		*--p = t2;
		*--p = t3;
		*--p = t1;
	}
	t1 = *EK++;
	*--p = *EK++;
	*--p = t1;

	t1 = mulInv(*EK++);
	t2 = -*EK++;
	t3 = -*EK++;
	*--p = mulInv(*EK++);
	*--p = t3;
	*--p = t2;
	*--p = t1;
/* Copy and destroy temp copy */
	pgpCopyMemory(temp, DK, sizeof(temp));
	pgpClearMemory( temp,  sizeof(temp));
} /* ideaInvertKey */


/*
 * MUL(x,y) computes x *= y, modulo 0x10001.  Requires two temps, 
 * t16 and t32.  x is modified, and must be a side-effect-free lvalue.
 * y may be anything, and is guaranteed to be evaluated exactly once,
 * but unlike x, must be strictly 16 bits even if low16() is #defined.
 * All of these are equivalent - see which is faster on your machine.
 */
#if SMALL_CACHE

#define MUL(x,y) (x = mul(low16(x),y))
static uint16
mul(register uint16 a, register uint16 b)
{
	register PGPUInt32 p;

	p = (PGPUInt32)a * b;
	if (p) {
		b = low16(p);
		a = p>>16;
		return (b - a) + (b < a);
	} else {
		return 1-a-b;
	}
} /* mul */

#elif AVOID_JUMPS

#define MUL(x,y) ( x = low16(x-1), t16 = low16((y)-1), \
		   t32 = (PGPUInt32)x*t16 + x + t16, \
		   x = low16(t32), t16 = t32>>16, \
		   x = (x-t16) + (x<t16) + 1 )

#else /* default */

#define MUL(x,y) (x = \
		  ( ( t32 = (PGPUInt32)(x=low16(x))*(t16=(y)) ) != 0 ) ? \
			x = low16(t32), \
			t16 = (uint16)(t32>>16), \
			(x-t16)+(x<t16) \
		   : \
			(uint16)(1-x-t16) \
		 )

#endif

/*
 * IDEA encryption/decryption algorithm.  In and out may be the same buffer.
 * key is a pointer to IDEAKEYLEN words.  (Borland C 3.1 gives an error if
 * you declare it as a "PGPUInt16 const key[IDEAKEYLEN]" array.)
 */
static void
ideaCipher(PGPByte const inbuf[8], PGPByte outbuf[8], PGPUInt16 const *key)
{
	register uint16 x1, x2, x3, x4, s2, s3;
#if !SMALL_CACHE
	register uint16 t16;	/* Temporaries needed by MUL macro */
	register PGPUInt32 t32;
#endif
	int r = IDEA_ROUNDS;

	x1 = (uint16)inbuf[0] << 8 | inbuf[1];
	x2 = (uint16)inbuf[2] << 8 | inbuf[3];
	x3 = (uint16)inbuf[4] << 8 | inbuf[5];
	x4 = (uint16)inbuf[6] << 8 | inbuf[7];

	do {
		MUL(x1,*key++);
		x2 += *key++;
		x3 += *key++;
		MUL(x4, *key++);

		s3 = x3;
		x3 ^= x1;
		MUL(x3, *key++);
		s2 = x2;
		x2 ^= x4;
		x2 += x3;
		MUL(x2, *key++);
		x3 += x2;

		x1 ^= x2;  x4 ^= x3;

		x2 ^= s3;  x3 ^= s2;
	} while (--r);
	MUL(x1, *key++);
	x3 += *key++;
	x2 += *key++;
	MUL(x4, *key);

	outbuf[0] = (PGPByte)(x1>>8);
	outbuf[1] = (PGPByte)x1;
	outbuf[2] = (PGPByte)(x3>>8);
	outbuf[3] = (PGPByte)x3;
	outbuf[4] = (PGPByte)(x2>>8);
	outbuf[5] = (PGPByte)x2;
	outbuf[6] = (PGPByte)(x4>>8);
	outbuf[7] = (PGPByte)x4;
} /* ideaCipher */

/*
 * Exported functions
 */

static void
ideaKey(void *priv, void const *key)
{
	/* Do key schedule for encryption, can be converted later */
	ideaExpandKey((const PGPByte *) key, (PGPUInt16 *)priv);
	*((PGPByte *)priv + IDEA_KEYBYTES) = IDEA_ENCRYPTION_MODE;
}

static void
ideaEncrypt(void *priv, void const *in, void *out)
{
	/* Make sure key schedule is in the right mode */
	if (*((PGPByte *)priv + IDEA_KEYBYTES) != IDEA_ENCRYPTION_MODE) {
		ideaInvertKey ((PGPUInt16 *)priv, (PGPUInt16 *)priv);
		*((PGPByte *)priv + IDEA_KEYBYTES) = IDEA_ENCRYPTION_MODE;
	}
	/* Do the operation */
	ideaCipher((const PGPByte *) in, (PGPByte *) out, (PGPUInt16 *)priv);
}

static void
ideaDecrypt(void *priv, void const *in, void *out)
{
	/* Make sure key schedule is in the right mode */
	if (*((PGPByte *)priv + IDEA_KEYBYTES) != IDEA_DECRYPTION_MODE) {
		ideaInvertKey ((PGPUInt16 *)priv, (PGPUInt16 *)priv);
		*((PGPByte *)priv + IDEA_KEYBYTES) = IDEA_DECRYPTION_MODE;
	}
	/* Do the operation */
	ideaCipher((const PGPByte *) in, (PGPByte *) out, (PGPUInt16 *)priv);
}

/*
 * Do one 64-bit step of a Tandem Davies-Meyer hash computation.
 * The hash buffer is 32 bytes long and contains H (0..7), then G (8..15),
 * then 16 bytes of scratch space.  The buf is 8 bytes long.
 * xkey is a temporary key schedule buffer.
 * This and the extra data in the hash buffer are allocated by the
 * caller to reduce the amount of buffer-wiping we have to do.
 * (It's only called from ideaWash, so the interface can be a bit
 * specialized.)
 */
static void
ideaStepTandemDM(PGPByte *hash, PGPByte const *buf, PGPUInt16 *xkey)
{
	int i;

	/* key1 = G << 64 + M, remembering that IDEA is big-endian */
	pgpCopyMemory(buf, hash+16, 8);
	ideaExpandKey(hash+8, xkey);
	/* W = E_key1(H), key2 = M << 64 + W */
	ideaCipher(hash, hash+24, xkey);
	ideaExpandKey(hash+16, xkey);
	/* V = E_key2(G) */
	ideaCipher(hash+8, hash+16, xkey);
	/* H ^= W, G ^= V */
	for (i = 0; i < 8; i++) {
		hash[i] ^= hash[i+24];
		hash[i+8] ^= hash[i+16];
	}
}

/*
 * Munge the key of the CipherContext based on the supplied bytes.
 * This is for random-number generation, so the exact technique is
 * unimportant, but it happens to use the current key as the
 * IV for computing a tandem Davies-Meyer hash of the bytes,
 * and uses the output as the new key.
 */
static void
ideaWash(void *priv, void const *bufIn, PGPSize len)
{
	unsigned i;
	PGPByte hash[32];
	PGPUInt16 	*xkey 	= (PGPUInt16 *)priv;
	PGPByte		*buf	= (PGPByte *) bufIn;

	/* Read out the key in canonical byte order for the IV */
	for (i = 0; i < 8; i++) {
		hash[2*i] = (PGPByte)(xkey[i]>>8);
		hash[2*i+1] = (PGPByte)xkey[i];
	}

	/* Do the initial blocks of the hash */
	i = len;
	while (i >= 8) {
		ideaStepTandemDM(hash, buf, xkey);
		buf += 8;
		i -= 8;
	}
	/*
	 * At the end, we do Damgard-Merkle strengthening, just like
	 * MD5 or SHA.  Pad with 0x80 then 0 bytes to 6 mod 8, then
	 * add the length.  We use a 16-bit length in bytes instead
	 * of a 64-bit length in bits, but that is cryptographically
	 * irrelevant.
	 */
	/* Do the first partial block - i <= 7 */
	pgpCopyMemory(buf, hash+24, i);
	hash[24 + i++] = 0x80;
	if (i > 6) {
		pgpClearMemory(hash+24+i, 8-i);
		ideaStepTandemDM(hash, hash+24, xkey);
		i = 0;
	}
	pgpClearMemory(hash+24+i, 6-i);
	hash[30] = (PGPByte)(len >> 8);
	hash[31] = (PGPByte)len;
	ideaStepTandemDM(hash, hash+24, xkey);

	/* Re-schedule the key */
	ideaExpandKey(hash, xkey);

	pgpClearMemory( hash,  sizeof(hash));
}

/*
 * Define a Cipher for the generic cipher.  This is the only
 * real exported thing -- everything else can be static, since everything
 * is referenced through function pointers!
 */
PGPCipherVTBL const cipherIDEA = {
	"IDEA",
	kPGPCipherAlgorithm_IDEA,
	8,			/* Blocksize */
	16,			/* Keysize */
	IDEA_KEYBYTES + 1,			/* Last one remembers encrypt vs decrypt */
	alignof(PGPUInt16),
	ideaKey,
	ideaEncrypt,
	ideaDecrypt,
	ideaWash
};

#if UNITTEST	/* Currently unused; left in in case of future need */

/* Test driver proper starts here */
#include <stdio.h>
#include <time.h>
/*
 * This is the number of Kbytes of test data to encrypt.
 * It defaults to 1 MByte.
 */
#ifndef BLOCKS
#ifndef KBYTES
#define KBYTES 1024
#endif
#define BLOCKS (64*KBYTES)
#endif

int
main(void)
{	/* Test driver for IDEA cipher */
	int i, j, k;
	PGPByte userkey[16];
	PGPByte priv[IDEA_KEYBYTES+1];
	PGPByte XX[8], YY[8], ZZ[8];
	clock_t start, end;
	long l;

	/* Make a sample user key for testing... */
	for(i=0; i<16; i++)
		userkey[i] = i+1;

	/* Compute encryption subkeys from user key... */
	ideaKey(priv, userkey);
#if 0
	ideaExpandKey(userkey, EK);
	printf("\nEncryption key subblocks: ");
	for (j=0; j<IDEA_ROUNDS+1; j++) {
		printf("\nround %d:   ", j+1);
		if (j < IDEA_ROUNDS)
			for(i=0; i<6; i++)
				printf(" %6u", EK[j*6+i]);
		else
			for(i=0; i<4; i++)
				printf(" %6u", EK[j*6+i]);
	}

	/* Compute decryption subkeys from encryption subkeys... */
	ideaInvertKey(EK, DK);
	printf("\nDecryption key subblocks: ");
	for (j=0; j<IDEA_ROUNDS+1; j++) {
		printf("\nround %d:   ", j+1);
		if (j < IDEA_ROUNDS)
			for(i=0; i<6; i++)
				printf(" %6u", DK[j*6+i]);
		else
			for(i=0; i<4; i++)
				printf(" %6u", DK[j*6+i]);
	}
#endif

	/* Make a sample plaintext pattern for testing... */
	for (k=0; k<8; k++)
		XX[k] = k;

	printf("\n Encrypting %d bytes (%ld blocks)...", BLOCKS*16, BLOCKS);
	fflush(stdout);
	start = clock();
	pgpCopyMemory(XX, YY, 8);
	for (l = 0; l < BLOCKS; l++)
		ideaEncrypt(priv, YY, YY);	/* repeated encryption */
	pgpCopyMemory(YY, ZZ, 8);
	for (l = 0; l < BLOCKS; l++)
		ideaDecrypt(priv, ZZ, ZZ);	/* repeated decryption */
	end = clock() - start;
	l = end * 1000 / CLOCKS_PER_SEC + 1;
	i = l/1000;
	j = l%1000;
	l = BLOCKS * 16 * CLOCKS_PER_SEC / end;
	printf("%d.%03d seconds = %ld bytes per second\n", i, j, l);

	printf("\nX %3u  %3u  %3u  %3u  %3u  %3u  %3u \n",    
	  XX[0], XX[1],  XX[2], XX[3], XX[4], XX[5],  XX[6], XX[7]);
	printf("\nY %3u  %3u  %3u  %3u  %3u  %3u  %3u \n",    
	  YY[0], YY[1],  YY[2], YY[3], YY[4], YY[5],  YY[6], YY[7]);
	printf("\nZ %3u  %3u  %3u  %3u  %3u  %3u  %3u \n",    
	  ZZ[0], ZZ[1],  ZZ[2], ZZ[3], ZZ[4], ZZ[5],  ZZ[6], ZZ[7]);

	/* Now decrypted ZZ should be same as original XX */
	for (k=0; k<8; k++)
		if (XX[k] != ZZ[k]) {
			printf("\n\07Error!  Noninvertable encryption.\n");
			exit(-1);	/* error exit */ 
		}
	printf("\nNormal exit.\n");
	return 0;	/* normal exit */
} /* main */

#endif /* 0 */




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/