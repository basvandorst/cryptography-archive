
/*  idea.c - C source code for IDEA block cipher.
 *  IDEA (International Data Encryption Algorithm), formerly known as
 *  IPES (Improved Proposed Encryption Standard).
 *  Algorithm developed by Xuejia Lai and James L. Massey, of ETH Zurich.
 *  This implementation modified and derived from original C code
 *  developed by Xuejia Lai.
 *  Zero-based indexing added, names changed from IPES to IDEA.
 *  CFB functions added.  Random number routines added.
 *
 *  Optimized for speed 21 Oct 92 by Colin Plumb.
 *  Very minor speedup on 23 Feb 93 by Colin Plumb.
 *  idearand() given a separate expanded key on 25 Feb 93, Colin Plumb.
 *
 *  There are two adjustments that can be made to this code to
 *  speed it up.  Defaults may be used for PCs.  Only the -DIDEA32
 *  pays off significantly if selectively set or not set.
 *  Experiment to see what works better for you.
 *
 *  Multiplication: default is inline, -DAVOID_JUMPS uses a
 *      different version that does not do any conditional
 *      jumps (a few percent worse on a SPARC), while
 *      -DSMALL_CACHE takes it out of line to stay
 *      within a small on-chip code cache.
 *  Variables: normally, 16-bit variables are used, but some
 *      machines (notably RISCs) do not have 16-bit registers,
 *      so they do a great deal of masking.  -DIDEA32 uses "int"
 *      register variables and masks explicitly only where
 *      necessary.  On a SPARC, for example, this boosts
 *      performace by 30%.
 *
 *  The IDEA(tm) block cipher is covered by a patent held by ETH and a
 *  Swiss company called Ascom-Tech AG.  The Swiss patent number is
 *  PCT/CH91/00117.  International patents are pending. IDEA(tm) is a
 *  trademark of Ascom-Tech AG.  There is no license fee required for
 *  noncommercial use.  Commercial users may obtain licensing details
 *  from Dieter Profos, Ascom Tech AG, Solothurn Lab, Postfach 151, 4502
 *  Solothurn, Switzerland, Tel +41 65 242885, Fax +41 65 235761.
 *
 *  The IDEA block cipher uses a 64-bit block size, and a 128-bit key
 *  size.  It breaks the 64-bit cipher block into four 16-bit words
 *  because all of the primitive inner operations are done with 16-bit
 *  arithmetic.  It likewise breaks the 128-bit cipher key into eight
 *  16-bit words.
 *
 *  For further information on the IDEA cipher, see these papers:
 *  1) Xuejia Lai, "Detailed Description and a Software Implementation of
 *         the IPES Cipher", Institute for Signal and Information
 *         Processing, ETH-Zentrum, Zurich, Switzerland, 1991
 *  2) Xuejia Lai, James L. Massey, Sean Murphy, "Markov Ciphers and
 *         Differential Cryptanalysis", Advances in Cryptology- EUROCRYPT'91
 *
 *  This code assumes that each pair of 8-bit bytes comprising a 16-bit
 *  word in the key and in the cipher block are externally represented
 *  with the Most Significant Byte (MSB) first, regardless of the
 *  internal native byte order of the target CPU.
 */

#include "l3.h"

#ifdef TEST
#include <stdio.h>
#include <time.h>
#endif
#include <stdio.h>

#define ROUNDS  16	/* Don't change this value, should be 8 */
#define KEYLEN  (6*ROUNDS+4)	/* length of key schedule */

typedef word16 IDEAkey[KEYLEN];

#ifdef IDEA32	/* Use >16-bit temporaries */
#define low16(x) ((x) & 0xFFFF)
typedef unsigned int uint16;	/* at LEAST 16 bits, maybe more */
#else
#define low16(x) (x)	/* this is only ever applied to uint16's */
typedef word16 uint16;
#endif

#ifdef _GNUC_
/* __const__ simply means there are no side effects for this function,
 * which is useful info for the gcc optimizer */
#define CONST __const__
#else
#define CONST
#endif

static IDEAkey Z;

static void en_key_idea(word16 * userkey, word16 * Z);
static void de_key_idea(IDEAkey Z, IDEAkey DK);
static void cipher_idea(word16 in[4], word16 out[4], CONST IDEAkey Z);

/*
 *  Multiplication, modulo (2**16)+1
 * Note that this code is structured like this on the assumption that
 * untaken branches are cheaper than taken branches, and the compiler
 * doesn't schedule branches.
 */
#ifdef SMALL_CACHE
CONST static uint16 mul(register uint16 a, register uint16 b)
{
	register word32 p;

	if (a) {
		if (b) {
			p = (word32) a *b;
			b = low16(p);
			a = p >> 16;
			return b - a + (b < a);
		}
		else {
			return 1 - a;
		}
	}
	else {
		return 1 - b;
	}
}/* mul */

#endif							/* SMALL_CACHE */

/*
 *  Compute multiplicative inverse of x, modulo (2**16)+1,
 *  using Euclid's GCD algorithm.  It is unrolled twice to
 *  avoid swapping the meaning of the registers each iteration,
 *  and some subtracts of t have been changed to adds.
 */
CONST static uint16 inv(uint16 x)
{
	uint16 t0, t1;
	uint16 q, y;

	if (x <= 1)
		return x;	/* 0 and 1 are self-inverse */
	t1 = 0x10001L / x;	/* Since x >= 2, this fits into 16 bits */
	y = 0x10001L % x;
	if (y == 1)
		return low16(1 - t1);
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
	return low16(1 - t1);
}/* inv */

/*  Compute IDEA encryption subkeys Z */
static void en_key_idea(word16 * userkey, word16 * Z)
{
	int i, j;

	/*
     * shifts
     */
	for (j = 0; j < 8; j++)
		Z[j] = *userkey++;

	for (i = 0; j < KEYLEN; j++) {
		i++;
		Z[i + 7] = Z[i & 7] << 9 | Z[i + 1 & 7] >> 7;
		Z += i & 8;
		i &= 7;
	}
}/* en_key_idea */

/*  Compute IDEA decryption subkeys DK from encryption subkeys Z */
/* Note: these buffers *may* overlap! */
static void de_key_idea(IDEAkey Z, IDEAkey DK)
{
	int j;
	uint16 t1, t2, t3;
	IDEAkey T;
	word16 *p = T + KEYLEN;

	t1 = inv(*Z++);
	t2 = -*Z++;
	t3 = -*Z++;
	*--p = inv(*Z++);
	*--p = t3;
	*--p = t2;
	*--p = t1;

	for (j = 1; j < ROUNDS; j++) {
		t1 = *Z++;
		*--p = *Z++;
		*--p = t1;

		t1 = inv(*Z++);
		t2 = -*Z++;
		t3 = -*Z++;
		*--p = inv(*Z++);
		*--p = t2;
		*--p = t3;
		*--p = t1;
	}
	t1 = *Z++;
	*--p = *Z++;
	*--p = t1;

	t1 = inv(*Z++);
	t2 = -*Z++;
	t3 = -*Z++;
	*--p = inv(*Z++);
	*--p = t3;
	*--p = t2;
	*--p = t1;
/* Copy and destroy temp copy */
	for (j = 0, p = T; j < KEYLEN; j++) {
		*DK++ = *p;
		*p++ = 0;
	}
}/* de_key_idea */

/*
 * MUL(x,y) computes x = x*y, modulo 0x10001.  Requires two temps,
 * t16 and t32.  x must me a side-effect-free lvalue.  y may be
 * anything, but unlike x, must be strictly 16 bits even if low16()
 * is #defined.
 * All of these are equivalent - see which is faster on your machine
 */
#ifdef SMALL_CACHE
#define MUL(x,y) (x = mul(low16(x),y))
#else
#ifdef AVOID_JUMPS
#define MUL(x,y) (x = low16(x-1), t16 = low16((y)-1), \
        t32 = (word32)x*t16+x+t16+1, x = low16(t32), \
        t16 = t32>>16, x = x-t16+(x<t16) )
#else
#define MUL(x,y) ((t16 = (y)) ? (x=low16(x)) ? \
     t32 = (word32)x*t16, x = low16(t32), t16 = t32>>16, \
     x = x-t16+(x<t16) : \
     (x = 1-t16) : (x = 1-x))
#endif
#endif

/*  IDEA encryption/decryption algorithm */
/* Note that in and out can be the same buffer */
static void cipher_idea(word16 in[4], word16 out[4], register CONST IDEAkey Z)
{
	register uint16 x1, x2, x3, x4, s2, s3;
#ifndef SMALL_CACHE
	register uint16 t16;
	register word32 t32;
#endif

	int r = ROUNDS;

	x1 = *in++;
	x2 = *in++;
	x3 = *in++;
	x4 = *in;
	do {
		MUL(x1, *Z++);
		x2 += *Z++;
		x3 += *Z++;
		MUL(x4, *Z++);

		s3 = x3;
		x3 ^= x1;
		MUL(x3, *Z++);
		s2 = x2;
		x2 ^= x4;
		x2 += x3;
		MUL(x2, *Z++);
		x3 += x2;

		x1 ^= x2;
		x4 ^= x3;

		x2 ^= s3;
		x3 ^= s2;
	} while (--r);
	MUL(x1, *Z++);
	*out++ = x1;
	*out++ = x3 + *Z++;
	*out++ = x2 + *Z++;
	MUL(x4, *Z);
	*out = x4;
}/* cipher_idea */

/* initkey_idea initializes IDEA for ECB mode operations */
void initkey_idea(byte key[16], boolean decryp)
{
	word16 userkey[8];	/* IDEA key is 16 bytes long */
	int i;
	/* Assume each pair of bytes comprising a word is ordered MSB-first. */
	for (i = 0; i < 8; i++) {
		userkey[i] = (key[0] << 8) + key[1];
		key++;
		key++;
	}
	en_key_idea(userkey, Z);
	for (i = 0; i < (6 * ROUNDS) + 4; i++)
		Z[i] ^= 0x0dae;
	if (decryp) {
		de_key_idea(Z, Z);	/* compute inverse key schedule DK */
	}
	for (i = 0; i < 8; i++)	/* Erase dangerous traces */
		userkey[i] = 0;
}/* initkey_idea */


/*  Run a 64-bit block thru IDEA in ECB (Electronic Code Book) mode,
    using the currently selected key schedule.
*/
void idea_ecb(word16 * inbuf, word16 * outbuf)
{
	/* Assume each pair of bytes comprising a word is ordered MSB-first. */
#ifdef LITTLE_ENDIAN	/* If this is a least-significant-byte-first CPU */
	word16 x;

	/* Invert the byte order for each 16-bit word for internal use. */
	x = inbuf[0];
	outbuf[0] = x >> 8 | x << 8;
	x = inbuf[1];
	outbuf[1] = x >> 8 | x << 8;
	x = inbuf[2];
	outbuf[2] = x >> 8 | x << 8;
	x = inbuf[3];
	outbuf[3] = x >> 8 | x << 8;
	cipher_idea(outbuf, outbuf, Z);
	x = outbuf[0];
	outbuf[0] = x >> 8 | x << 8;
	x = outbuf[1];
	outbuf[1] = x >> 8 | x << 8;
	x = outbuf[2];
	outbuf[2] = x >> 8 | x << 8;
	x = outbuf[3];
	outbuf[3] = x >> 8 | x << 8;
#else							/* LITTLE_ENDIAN */
	/* Byte order for internal and external representations is the same. */
	 cipher_idea(inbuf, outbuf, Z);
#endif							/* LITTLE_ENDIAN */
}/* idea_ecb */

/*
    close_idea function erases all the key schedule information when
    we are all done with a set of operations for a particular IDEA key
    context.  This is to prevent any sensitive data from being left
    around in memory.
*/
void close_idea(void)
{/* erase current key schedule tables */
	short i;
	for (i = 0; i < KEYLEN; i++)
		Z[i] = 0;
}/* close_idea() */
