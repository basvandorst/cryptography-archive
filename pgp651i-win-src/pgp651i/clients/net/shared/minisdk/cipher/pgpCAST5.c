/*
 * pgpCAST5.c - C source code for CAST block cipher, version 5
 *				AKA CAST5-128 and CAST-128.
 *
 * RFC2144 describes this version of the CAST cipher.
 *
 * 
 * Subject to the terms and conditions set forth below, Northern Telecom
 * Limited, to the extent of its legal right to do so, hereby grants a
 * non-exclusive, royalty free license to make, have made, use, sell and
 * have sold products incorporating or embodying the CAST algorithm as
 * described herein.
 * 
 * DISCLAIMER OF ALL WARRANTIES
 * 
 * IN NO EVENT SHALL NORTHERN TELECOM LIMITED OR ANY OF ITS 
 * SUBSIDIARIES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE GRANT OF 
 * RIGHTS UNDER THIS LICENSE, EVEN IF NORTHERN TELECOM OR ANY OF ITS 
 * SUBSIDIARIES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 * USE OF ANY OF THE RIGHTS GRANTED HEREUNDER SHALL BE AT LICENSEE'S 
 * SOLE RISK.
 * 
 * NORTHERN TELECOM LIMITED AND ITS SUBSIDIARIES SPECIFICALLY 
 * DISCLAIM ANY REPRESENTATIONS, CONDITIONS, OR WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF NON-
 * INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 * PURPOSE.  WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, 
 * NOTHING IN THIS LICENSE SHALL BE CONSTRUED AS:
 * 
 * (A)	A WARRANTY OR REPRESENTATION AS TO THE VALIDITY OR SCOPE OF 
 * ANY PATENT OR ANY RIGHTS UNDER ANY OTHER FORM OF INTELLECTUAL 
 * PROPERTY PROTECTION (WHETHER STATUTORY OR OTHERWISE);
 * 
 * (B)	A WARRANTY OR REPRESENTATION THAT ANY MANUFACTURE, SALE, 
 * LEASE, USE OR IMPORTATION WILL BE FREE FROM INFRINGEMENT OF ANY 
 * INTELLECTUAL PROPERTY RIGHTS OTHER THAN THOSE UNDER WHICH AND 
 * TO THE EXTENT TO WHICH LICENSES ARE IN FORCE HEREUNDER;
 * 
 * (C)	AN OBLIGATION TO MAINTAIN ANY INTELLECTUAL PROPERTY RIGHTS 
 * OR TO CONTINUE TO PROSECUTE ANY APPLICATION IN RESPECT OF ANY 
 * INTELLECTUAL PROPERTY RIGHTS IN ANY COUNTRY;
 * 
 * (D)	AN OBLIGATION TO FURNISH ANY MANUFACTURING, TECHNICAL OR 
 * CONFIDENTIAL INFORMATION OR TO PROVIDE ANY ASSISTANCE, 
 * MAINTENANCE, OR SUPPORT OF ANY KIND;
 * 
 * (E)	CONFERRING ANY RIGHT TO USE, IN ADVERTISING, PUBLICITY OR 
 * OTHERWISE, ANY NAME, TRADE NAME OR TRADEMARK (STATUTORY OR 
 * OTHERWISE), OR ANY CONTRACTION, ABBREVIATION OR SIMULATION 
 * THEREOF (EXCEPT AS SET OUT BELOW);
 * 
 * (F)	CONFERRING BY IMPLICATION, ESTOPPEL OR OTHERWISE ANY 
 * LICENSE OR OTHER RIGHT UNDER ANY INTELLECTUAL PROPERTY RIGHT, 
 * EXCEPT THE LICENSES AND RIGHTS EXPRESSLY GRANTED HEREUNDER;
 * 
 * (G)	AN OBLIGATION TO MAKE ANY DETERMINATION AS TO THE 
 * APPLICABILITY OF ANY INTELLECTUAL PROPERTY RIGHT TO ANY PRODUCT;
 * 
 * (H)	CREATING ANY AGENCY, PARTNERSHIP, JOINT VENTURE OR SIMILAR 
 * RELATIONSHIP.
 * 
 * THIS LICENSE IS EXPRESSLY SUBJECT TO ANY LAWS, REGULATIONS, ORDERS 
 * OR OTHER RESTRICTIONS ON THE EXPORT FROM CANADA, THE UNITED 
 * STATES OR ANY OTHER COUNTRY OF CRYPTOGRAPHIC TECHNOLOGY.  THIS 
 * LICENSE SHALL NOT EXTEND TO ANY SUBJECT MATTER COVERED 
 * HEREUNDER WHICH IS EXPORTED WITHOUT A PROPER EXPORT LICENSE OR 
 * APPROVAL.
 * 
 * License to use CAST as described herein is subject to it being
 * referred to in any inplementation thereof (including all product and
 * markteting materials) as the "Northern Telecom Ltd., CAST Encryption
 * Algorithm".  The name of Northern Telecom Limited (including its
 * subsidiaries) shall not be used to endorse or promote products without
 * the specific prior written consent of Northern Telecom Limited.
 * 
 * 
 * $Id: pgpCAST5.c,v 1.2 1998/12/04 17:20:12 dgal Exp $
 *
 */
 
#include "pgpConfig.h"

#include "pgpSymmetricCipherPriv.h"
#include "pgpCAST5.h"
#include "pgpCASTBox5.h"
#include "pgpMem.h"

/* The size of a scheduled CAST5 key */
#define CAST5_KEYWORDS 32
#define CAST5_KEYBYTES (sizeof(PGPUInt32)*CAST5_KEYWORDS)

/* CAST is uniformly big-endian - byte access macros */
#define B0(x) ((x) >> 24 & 255)
#define B1(x) ((x) >> 16 & 255)
#define B2(x) ((x) >> 8 & 255)
#define B3(x) ((x) & 255)

/* Some macros used in the key scheduling code */
#define x0 B0(x0123)
#define x1 B1(x0123)
#define x2 B2(x0123)
#define x3 B3(x0123)
#define x4 B0(x4567)
#define x5 B1(x4567)
#define x6 B2(x4567)
#define x7 B3(x4567)
#define x8 B0(x89AB)
#define x9 B1(x89AB)
#define xA B2(x89AB)
#define xB B3(x89AB)
#define xC B0(xCDEF)
#define xD B1(xCDEF)
#define xE B2(xCDEF)
#define xF B3(xCDEF)
#define z0 B0(z0123)
#define z1 B1(z0123)
#define z2 B2(z0123)
#define z3 B3(z0123)
#define z4 B0(z4567)
#define z5 B1(z4567)
#define z6 B2(z4567)
#define z7 B3(z4567)
#define z8 B0(z89AB)
#define z9 B1(z89AB)
#define zA B2(z89AB)
#define zB B3(z89AB)
#define zC B0(zCDEF)
#define zD B1(zCDEF)
#define zE B2(zCDEF)
#define zF B3(zCDEF)

/*
 * This expands a 128-bit key to a 32-word scheduled key, where each round
 * uses two words: a 32-bit XOR mask and a 5-bit rotate amount.  Shorter keys
 * are just padded with zeros, and if they are 80 bits or less, the cipher
 * is reduced to 12 rounds (not implemented here).
 *
 * The feed-forward used with x0123 through yCDEF prevent any weak keys,
 * and the substitution to set up the xkey tables ensure that the subround
 * keys are not easily derivable from each other, so linear cryptanalysis
 * won't do very well against CAST.
 */
static void
CAST5schedule(PGPUInt32 xkey[32], PGPByte const *k)
{
	PGPUInt32 x0123, x4567, x89AB, xCDEF;
	PGPUInt32 z0123, z4567, z89AB, zCDEF;
	int i;

	/* Initialize x0123456789ABCDEF with input key */
	x0123=(PGPUInt32)
		k[ 0]<<24 | (PGPUInt32)k[ 1]<<16 | (PGPUInt32)k[ 2]<<8 | k[ 3];
		
	x4567=(PGPUInt32)
		k[ 4]<<24 | (PGPUInt32)k[ 5]<<16 | (PGPUInt32)k[ 6]<<8 | k[ 7];
		
	x89AB=(PGPUInt32)
		k[ 8]<<24 | (PGPUInt32)k[ 9]<<16 | (PGPUInt32)k[10]<<8 | k[11];
		
	xCDEF=(PGPUInt32)
		k[12]<<24 | (PGPUInt32)k[13]<<16 | (PGPUInt32)k[14]<<8 | k[15];

	/* Now set up the key table */

	for (i = 0; i < 4; i++) {
		z0123 = x0123 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
		z4567 = x89AB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
		z89AB = xCDEF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
		zCDEF = x4567 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];

		x0123 = z89AB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
		x4567 = z0123 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
		x89AB = z4567 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
		xCDEF = zCDEF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];

		switch (i) {
		  case 0:
			/* Masking keys, rounds 0..7 */
			xkey[ 0] = S5[z8] ^ S6[z9] ^ S7[z7] ^ S8[z6] ^ S5[z2];
			xkey[ 2] = S5[zA] ^ S6[zB] ^ S7[z5] ^ S8[z4] ^ S6[z6];
			xkey[ 4] = S5[zC] ^ S6[zD] ^ S7[z3] ^ S8[z2] ^ S7[z9];
			xkey[ 6] = S5[zE] ^ S6[zF] ^ S7[z1] ^ S8[z0] ^ S8[zC];
			
			xkey[ 8] = S5[x3] ^ S6[x2] ^ S7[xC] ^ S8[xD] ^ S5[x8];
			xkey[10] = S5[x1] ^ S6[x0] ^ S7[xE] ^ S8[xF] ^ S6[xD];
			xkey[12] = S5[x7] ^ S6[x6] ^ S7[x8] ^ S8[x9] ^ S7[x3];
			xkey[14] = S5[x5] ^ S6[x4] ^ S7[xA] ^ S8[xB] ^ S8[x7];
			break;
		  case 1:
			/* Masking keys, rounds 8..15 */
			xkey[16] = S5[z3] ^ S6[z2] ^ S7[zC] ^ S8[zD] ^ S5[z9];
			xkey[18] = S5[z1] ^ S6[z0] ^ S7[zE] ^ S8[zF] ^ S6[zC];
			xkey[20] = S5[z7] ^ S6[z6] ^ S7[z8] ^ S8[z9] ^ S7[z2];
			xkey[22] = S5[z5] ^ S6[z4] ^ S7[zA] ^ S8[zB] ^ S8[z6];
			
			xkey[24] = S5[x8] ^ S6[x9] ^ S7[x7] ^ S8[x6] ^ S5[x3];
			xkey[26] = S5[xA] ^ S6[xB] ^ S7[x5] ^ S8[x4] ^ S6[x7];
			xkey[28] = S5[xC] ^ S6[xD] ^ S7[x3] ^ S8[x2] ^ S7[x8];
			xkey[30] = S5[xE] ^ S6[xF] ^ S7[x1] ^ S8[x0] ^ S8[xD];
			break;
		  case 2:
			/* Rotation keys, rounds 0..7 */
			xkey[ 1] = (S5[z8]^S6[z9]^S7[z7]^S8[z6]^S5[z2]) & 31;
			xkey[ 3] = (S5[zA]^S6[zB]^S7[z5]^S8[z4]^S6[z6]) & 31;
			xkey[ 5] = (S5[zC]^S6[zD]^S7[z3]^S8[z2]^S7[z9]) & 31;
			xkey[ 7] = (S5[zE]^S6[zF]^S7[z1]^S8[z0]^S8[zC]) & 31;
			
			xkey[ 9] = (S5[x3]^S6[x2]^S7[xC]^S8[xD]^S5[x8]) & 31;
			xkey[11] = (S5[x1]^S6[x0]^S7[xE]^S8[xF]^S6[xD]) & 31;
			xkey[13] = (S5[x7]^S6[x6]^S7[x8]^S8[x9]^S7[x3]) & 31;
			xkey[15] = (S5[x5]^S6[x4]^S7[xA]^S8[xB]^S8[x7]) & 31;
			break;
		  case 3:
			/* Rotation keys, rounds 8..15 */
			xkey[17] = (S5[z3]^S6[z2]^S7[zC]^S8[zD]^S5[z9]) & 31;
			xkey[19] = (S5[z1]^S6[z0]^S7[zE]^S8[zF]^S6[zC]) & 31;
			xkey[21] = (S5[z7]^S6[z6]^S7[z8]^S8[z9]^S7[z2]) & 31;
			xkey[23] = (S5[z5]^S6[z4]^S7[zA]^S8[zB]^S8[z6]) & 31;
		
			xkey[25] = (S5[x8]^S6[x9]^S7[x7]^S8[x6]^S5[x3]) & 31;
			xkey[27] = (S5[xA]^S6[xB]^S7[x5]^S8[x4]^S6[x7]) & 31;
			xkey[29] = (S5[xC]^S6[xD]^S7[x3]^S8[x2]^S7[x8]) & 31;
			xkey[31] = (S5[xE]^S6[xF]^S7[x1]^S8[x0]^S8[xD]) & 31;
			break;
		}
	}
	x0123 = x4567 = x89AB = xCDEF = 0;
	z0123 = z4567 = z89AB = zCDEF = 0;
}

#undef x0
#undef x1
#undef x2
#undef x3
#undef x4
#undef x5
#undef x6
#undef x7
#undef x8
#undef x9
#undef xA
#undef xB
#undef xC
#undef xD
#undef xE
#undef xF
#undef z0
#undef z1
#undef z2
#undef z3
#undef z4
#undef z5
#undef z6
#undef z7
#undef z8
#undef z9
#undef zA
#undef zB
#undef zC
#undef zD
#undef zE
#undef zF

/* Some macros used in the encryption/decryption code */
#define ROL(x,r) ((x)<<(r) | (x)>>(32-(r)))

#ifdef __GNUC__
#if __i386__
/* Redefine using GCC inline assembler */
#undef ROL
#define ROL(x,r)	\
	({unsigned _y;	\
	__asm__("rol %%cl,%0" : "=g" (_y) : "0" (x), "c" (r)); _y;})
#endif /* __i386__ */
#endif /* __GNUC__ */

#define F1(x,xkey,i) (ROL((xkey)[2*(i)] + (x), (xkey)[2*(i)+1]))
#define F2(x,xkey,i) (ROL((xkey)[2*(i)] ^ (x), (xkey)[2*(i)+1]))
#define F3(x,xkey,i) (ROL((xkey)[2*(i)] - (x), (xkey)[2*(i)+1]))

#define G1(x) (((S1[B0(x)] ^ S2[B1(x)]) - S3[B2(x)]) + S4[B3(x)])
#define G2(x) (((S1[B0(x)] - S2[B1(x)]) + S3[B2(x)]) ^ S4[B3(x)])
#define G3(x) (((S1[B0(x)] + S2[B1(x)]) ^ S3[B2(x)]) - S4[B3(x)])

/*
 * Encrypt the 8 bytes at *in into the 8 bytes at *out using the expanded
 * key schedule from *xkey.
 */
static void
CAST5encrypt(PGPByte const *in, PGPByte *out, PGPUInt32 const *xkey)
{
	PGPUInt32 l, r, t;

	l = (PGPUInt32)
		in[0]<<24 | (PGPUInt32)in[1]<<16 | (PGPUInt32)in[2]<<8 | in[3];
	r = (PGPUInt32)
		in[4]<<24 | (PGPUInt32)in[5]<<16 | (PGPUInt32)in[6]<<8 | in[7];

	t = F1(r, xkey,  0); l ^= G1(t);
	t = F2(l, xkey,  1); r ^= G2(t);
	t = F3(r, xkey,  2); l ^= G3(t);
	t = F1(l, xkey,  3); r ^= G1(t);
	t = F2(r, xkey,  4); l ^= G2(t);
	t = F3(l, xkey,  5); r ^= G3(t);
	t = F1(r, xkey,  6); l ^= G1(t);
	t = F2(l, xkey,  7); r ^= G2(t);
	t = F3(r, xkey,  8); l ^= G3(t);
	t = F1(l, xkey,  9); r ^= G1(t);
	t = F2(r, xkey, 10); l ^= G2(t);
	t = F3(l, xkey, 11); r ^= G3(t);
	/* Stop here if only doing 12 rounds */
	t = F1(r, xkey, 12); l ^= G1(t);
	t = F2(l, xkey, 13); r ^= G2(t);
	t = F3(r, xkey, 14); l ^= G3(t);
	t = F1(l, xkey, 15); r ^= G1(t);

	out[0] = B0(r);
	out[1] = B1(r);
	out[2] = B2(r);
	out[3] = B3(r);
	out[4] = B0(l);
	out[5] = B1(l);
	out[6] = B2(l);
	out[7] = B3(l);
}

/*
 * Decrypt the 8 bytes at *in into the 8 bytes at *out using the expanded
 * key schedule from *xkey.
 */
static void
CAST5decrypt(PGPByte const *in, PGPByte *out, PGPUInt32 const *xkey)
{
	PGPUInt32 l, r, t;

	r = (PGPUInt32)
		in[0]<<24 | (PGPUInt32)in[1]<<16 | (PGPUInt32)in[2]<<8 | in[3];
	l = (PGPUInt32)
		in[4]<<24 | (PGPUInt32)in[5]<<16 | (PGPUInt32)in[6]<<8 | in[7];

	t = F1(l, xkey, 15); r ^= G1(t);
	t = F3(r, xkey, 14); l ^= G3(t);
	t = F2(l, xkey, 13); r ^= G2(t);
	t = F1(r, xkey, 12); l ^= G1(t);
	/* Start here if only doing 12 rounds */
	t = F3(l, xkey, 11); r ^= G3(t);
	t = F2(r, xkey, 10); l ^= G2(t);
	t = F1(l, xkey,  9); r ^= G1(t);
	t = F3(r, xkey,  8); l ^= G3(t);
	t = F2(l, xkey,  7); r ^= G2(t);
	t = F1(r, xkey,  6); l ^= G1(t);
	t = F3(l, xkey,  5); r ^= G3(t);
	t = F2(r, xkey,  4); l ^= G2(t);
	t = F1(l, xkey,  3); r ^= G1(t);
	t = F3(r, xkey,  2); l ^= G3(t);
	t = F2(l, xkey,  1); r ^= G2(t);
	t = F1(r, xkey,  0); l ^= G1(t);

	out[0] = B0(l);
	out[1] = B1(l);
	out[2] = B2(l);
	out[3] = B3(l);
	out[4] = B0(r);
	out[5] = B1(r);
	out[6] = B2(r);
	out[7] = B3(r);
}


/*
 * Exported functions
 */

/* Expand 16 byte key into expanded key *priv */
static void
cast5Key(void *priv, void const *key)
{
	CAST5schedule((PGPUInt32 *)priv, (const PGPByte *) key);
}

/* Encrypt 8 bytes at *in into 8 bytes at *out using key sched from *priv */
static void
cast5Encrypt(void *priv, void const *in, void *out)
{
	CAST5encrypt((const PGPByte *) in, (PGPByte *) out, (PGPUInt32 *)priv);
}

/* Decrypt 8 bytes at *in into 8 bytes at *out using key sched from *priv */
static void
cast5Decrypt(void *priv, void const *in, void *out)
{
	CAST5decrypt( (const PGPByte *) in, (PGPByte *) out, (PGPUInt32 *)priv);
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
CAST5StepTandemDM(PGPByte *hash, PGPByte const *buf, PGPUInt32 *xkey)
{
	int i;

	/* key1 = G << 64 + M, remembering that CAST is big-endian */
	pgpCopyMemory(buf, hash+16, 8);
	CAST5schedule(xkey, hash+8);
	/* W = E_key1(H), key2 = M << 64 + W */
	CAST5encrypt(hash, hash+24, xkey);
	CAST5schedule(xkey, hash+16);
	/* V = E_key2(G) */
	CAST5encrypt(hash+8, hash+16, xkey);
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
cast5Wash(void *priv, void const *bufIn, PGPSize len)
{
	PGPSize i;
	PGPByte hash[32];
	PGPUInt32 	*xkey = (PGPUInt32 *)priv;
	PGPByte		*buf = (PGPByte *) bufIn;
	
	/* Read out the key in canonical byte order for the IV */
	for (i = 0; i < 8; i++) {
		hash[2*i] = (PGPByte)(xkey[i]>>8);
		hash[2*i+1] = (PGPByte)xkey[i];
	}

	/* Do the initial blocks of the hash */
	i = len;
	while (i >= 8) {
		CAST5StepTandemDM(hash, buf, xkey);
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
		CAST5StepTandemDM(hash, hash+24, xkey);
		i = 0;
	}
	pgpClearMemory(hash+24+i, 6-i);
	hash[30] = (PGPByte)(len >> 8);
	hash[31] = (PGPByte)len;
	CAST5StepTandemDM(hash, hash+24, xkey);

	/* Re-schedule the key */
	CAST5schedule(xkey, hash);

	pgpClearMemory( hash,  sizeof(hash));
}

/*
 * Define a Cipher for the generic cipher.  This is the only
 * real exported thing -- everything else can be static, since everything
 * is referenced through function pointers!
 */
PGPCipherVTBL const cipherCAST5 =
{
	"CAST5",
	kPGPCipherAlgorithm_CAST5,
	8,			/* Blocksize */
	16,			/* Keysize */
	CAST5_KEYBYTES,				/* Priv array size */
	alignof(PGPUInt32),
	cast5Key,
	cast5Encrypt,
	cast5Decrypt,
	cast5Wash
};





#if UNITTEST


/*
 * From RFC2144:
 *
 * Appendix B. Test Vectors
 * 
 *    This appendix provides test vectors for the CAST-128 cipher described
 *    this document.
 * 
 * B.1. Single Plaintext-Key-Ciphertext Sets
 * 
 *    In order to ensure that the algorithm is implemented correctly, the
 *    following test vectors can be used for verification (values given in
 *    hexadecimal notation).
 * 
 *    128-bit key         = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A
 *            plaintext   = 01 23 45 67 89 AB CD EF
 *            ciphertext  = 23 8B 4F E5 84 7E 44 B2
 * 
 *    80-bit  key         = 01 23 45 67 12 34 56 78 23 45
 *                        = 01 23 45 67 12 34 56 78 23 45 00 00 00 00 00 00
 *            plaintext   = 01 23 45 67 89 AB CD EF
 *            ciphertext  = EB 6A 71 1A 2C 02 27 1B
 * 
 *    40-bit  key         = 01 23 45 67 12
 *                        = 01 23 45 67 12 00 00 00 00 00 00 00 00 00 00 00
 *            plaintext   = 01 23 45 67 89 AB CD EF
 *            ciphertext  = 7A C8 16 D1 6E 9B 30 2E
 * 
 * B.2. Full Maintenance Test
 * 
 *    A maintenance test for CAST-128 has been defined to verify the
 *    correctness of implementations.  It is defined in pseudo-code as
 *    follows, where a and b are 128-bit vectors, aL and aR are the
 *    leftmost and rightmost halves of a, bL and bR are the leftmost and
 *    rightmost halves of b, and encrypt(d,k) is the encryption in ECB mode
 *    of block d under key k.
 * 
 *    Initial a = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A (hex)
 *    Initial b = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A (hex)
 * 
 *    do 1,000,000 times
 *    {
 *        aL = encrypt(aL,b)
 *        aR = encrypt(aR,b)
 *        bL = encrypt(bL,a)
 *        bR = encrypt(bR,a)
 *    }
 * 
 * Verify a == EE A9 D0 A2 49 FD 3B A6 B3 43 6F B8 9D 6D CA 92 (hex)
 * Verify b == B2 C9 5E B0 0C 31 AD 71 80 AC 05 B8 E8 3D 69 6E (hex)
 * 
 */


/* Test vectors */
PGPByte K1[] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
	0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
};
PGPByte P1[] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
};
PGPByte C1[] = {
	0x23, 0x8B, 0x4F, 0xE5, 0x84, 0x7E, 0x44, 0xB2
};

#if 0
/*
 * We do not support 80-bit and lower keys, which require 12 rather
 * than 16 rounds of encryption.
 */
PGPByte K2[] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
	0x23, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte P2[] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
};
PGPByte C2[] = {
	0xEB, 0x6A, 0x71, 0x1A, 0x2C, 0x02, 0x27, 0x1B
};

PGPByte K3[] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
PGPByte P3[] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
};
PGPByte C3[] = {
	0x7A, 0xC8, 0x16, 0xD1, 0x6E, 0x9B, 0x30, 0x2E
};
#endif


/* Maintenance test */
PGPByte A[] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
	0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
};
PGPByte B[] = {
	0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
	0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
};
PGPByte VA[] = {
	0xEE, 0xA9, 0xD0, 0xA2, 0x49, 0xFD, 0x3B, 0xA6,
	0xB3, 0x43, 0x6F, 0xB8, 0x9D, 0x6D, 0xCA, 0x92
};
PGPByte VB[] = {
	0xB2, 0xC9, 0x5E, 0xB0, 0x0C, 0x31, 0xAD, 0x71,
	0x80, 0xAC, 0x05, 0xB8, 0xE8, 0x3D, 0x69, 0x6E
};


int
main(void)
{	/* Test driver for CAST cipher */
	int i;
	PGPByte priv[CAST5_KEYBYTES];
	PGPByte X[8], Y[8];

	/* First test */
	cast5Key(priv, K1);
	cast5Encrypt(priv, P1, X);
	if (memcmp(C1, X, sizeof(X)) == 0)
		printf ("Encryption test 1 passed\n");
	else
		printf ("ERROR ON ENCRYPTION TEST 1\n");
	cast5Decrypt(priv, C1, Y);
	if (memcmp(P1, Y, sizeof(Y)) == 0)
		printf ("Decryption test 1 passed\n");
	else
		printf ("ERROR ON DECRYPTION TEST 1\n");

	/* Maintenance test */
	printf ("Performing maintenance test...\n");
	for (i=0; i<1000000; ++i) {
		cast5Key(priv, B);
		cast5Encrypt(priv, A, A);
		cast5Encrypt(priv, A+8, A+8);
		cast5Key(priv, A);
		cast5Encrypt(priv, B, B);
		cast5Encrypt(priv, B+8, B+8);
	}
	if (memcmp(A, VA, sizeof(A)) == 0)
		printf ("Maintenance test A passed\n");
	else
		printf ("ERROR ON MAINTENANCE TEST A\n");
	if (memcmp(B, VB, sizeof(B)) == 0)
		printf ("Maintenance test B passed\n");
	else
		printf ("ERROR ON MAINTENANCE TEST B\n");

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
