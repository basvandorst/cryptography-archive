//////////////////////////////////////////////////////////////////////////////
// Cast5.cpp
//
// Functions for implementing the CAST5 cipher.
//////////////////////////////////////////////////////////////////////////////

// $Id: Cast5.cpp,v 1.1.2.11 1998/07/06 08:56:32 nryan Exp $

// Portions Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>
#include <stdio.h>
#include <stdlib.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "Cast5.h"
#include "Cast5Box.h"
#include "CipherUtils.h"


/////////
// Macros
/////////

#define ROL(x,r) ((x)<<(r) | (x)>>(32-(r)))

#define B0(x) ((x) >> 24 & 255)
#define B1(x) ((x) >> 16 & 255)
#define B2(x) ((x) >> 8 & 255)
#define B3(x) ((x) & 255)

#define F1(x,xkey,i) (ROL((xkey)[2*(i)] + (x), (xkey)[2*(i)+1]))
#define F2(x,xkey,i) (ROL((xkey)[2*(i)] ^ (x), (xkey)[2*(i)+1]))
#define F3(x,xkey,i) (ROL((xkey)[2*(i)] - (x), (xkey)[2*(i)+1]))

#define G1(x) (((S1[B0(x)] ^ S2[B1(x)]) - S3[B2(x)]) + S4[B3(x)])
#define G2(x) (((S1[B0(x)] - S2[B1(x)]) + S3[B2(x)]) ^ S4[B3(x)])
#define G3(x) (((S1[B0(x)] + S2[B1(x)]) ^ S3[B2(x)]) - S4[B3(x)])

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


/////////////////////////
// CAST5 Cipher Functions
/////////////////////////

// This expands a 128-bit key to a 32-word scheduled key, where each round
// uses two words: a 32-bit XOR mask and a 5-bit rotate amount.  Shorter keys
// are just padded with zeros, and if they are 80 bits or less, the cipher
// is reduced to 12 rounds (not implemented here).
//
// The feed-forward used with x0123 through yCDEF prevent any weak keys,
// and the substitution to set up the xkey tables ensure that the subround
// keys are not easily derivable from each other, so linear cryptanalysis
// won't do very well against CAST.

void 
CAST5schedule(PGPUInt32 xkey[32], const PGPUInt8 *k)
{
	PGPUInt32	x0123, x4567, x89AB, xCDEF;
	PGPUInt32	z0123, z4567, z89AB, zCDEF;

	// Initialize x0123456789ABCDEF with input key
	x0123	= (PGPUInt32) k[0]<<24 | (PGPUInt32) k[1]<<16 | 
		(PGPUInt32) k[2]<<8 | k[3];

	x4567	= (PGPUInt32) k[4]<<24 | (PGPUInt32) k[5]<<16 | 
		(PGPUInt32) k[6]<<8 | k[7];

	x89AB	= (PGPUInt32) k[8]<<24 | (PGPUInt32) k[9]<<16 | 
		(PGPUInt32) k[10]<<8 | k[11];

	xCDEF	= (PGPUInt32) k[12]<<24 | (PGPUInt32) k[13]<<16 | 
		(PGPUInt32) k[14]<<8 | k[15];

	// Now set up the key table
	for (PGPUInt8 i=0; i<4; i++) 
	{
		z0123	= x0123 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
		z4567	= x89AB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
		z89AB	= xCDEF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
		zCDEF	= x4567 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
		x0123	= z89AB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
		x4567	= z0123 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
		x89AB	= z4567 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
		xCDEF	= zCDEF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];

		switch (i)
		{
		case 0:			// Masking keys, rounds 0..7
			xkey[0]		= S5[z8] ^ S6[z9] ^ S7[z7] ^ S8[z6] ^ S5[z2];
			xkey[2]		= S5[zA] ^ S6[zB] ^ S7[z5] ^ S8[z4] ^ S6[z6];
			xkey[4]		= S5[zC] ^ S6[zD] ^ S7[z3] ^ S8[z2] ^ S7[z9];
			xkey[6]		= S5[zE] ^ S6[zF] ^ S7[z1] ^ S8[z0] ^ S8[zC];
			
			xkey[8]		= S5[x3] ^ S6[x2] ^ S7[xC] ^ S8[xD] ^ S5[x8];
			xkey[10]	= S5[x1] ^ S6[x0] ^ S7[xE] ^ S8[xF] ^ S6[xD];
			xkey[12]	= S5[x7] ^ S6[x6] ^ S7[x8] ^ S8[x9] ^ S7[x3];
			xkey[14]	= S5[x5] ^ S6[x4] ^ S7[xA] ^ S8[xB] ^ S8[x7];
			break;

		 case 1:			// Masking keys, rounds 8..15
			xkey[16]	= S5[z3] ^ S6[z2] ^ S7[zC] ^ S8[zD] ^ S5[z9];
			xkey[18]	= S5[z1] ^ S6[z0] ^ S7[zE] ^ S8[zF] ^ S6[zC];
			xkey[20]	= S5[z7] ^ S6[z6] ^ S7[z8] ^ S8[z9] ^ S7[z2];
			xkey[22]	= S5[z5] ^ S6[z4] ^ S7[zA] ^ S8[zB] ^ S8[z6];
			
			xkey[24]	= S5[x8] ^ S6[x9] ^ S7[x7] ^ S8[x6] ^ S5[x3];
			xkey[26]	= S5[xA] ^ S6[xB] ^ S7[x5] ^ S8[x4] ^ S6[x7];
			xkey[28]	= S5[xC] ^ S6[xD] ^ S7[x3] ^ S8[x2] ^ S7[x8];
			xkey[30]	= S5[xE] ^ S6[xF] ^ S7[x1] ^ S8[x0] ^ S8[xD];
			break;

		 case 2:			// Rotation keys, rounds 0..7
			xkey[1]		= (S5[z8]^S6[z9]^S7[z7]^S8[z6]^S5[z2]) & 31;
			xkey[3]		= (S5[zA]^S6[zB]^S7[z5]^S8[z4]^S6[z6]) & 31;
			xkey[5]		= (S5[zC]^S6[zD]^S7[z3]^S8[z2]^S7[z9]) & 31;
			xkey[7]		= (S5[zE]^S6[zF]^S7[z1]^S8[z0]^S8[zC]) & 31;
			
			xkey[9]		= (S5[x3]^S6[x2]^S7[xC]^S8[xD]^S5[x8]) & 31;
			xkey[11]	= (S5[x1]^S6[x0]^S7[xE]^S8[xF]^S6[xD]) & 31;
			xkey[13]	= (S5[x7]^S6[x6]^S7[x8]^S8[x9]^S7[x3]) & 31;
			xkey[15]	= (S5[x5]^S6[x4]^S7[xA]^S8[xB]^S8[x7]) & 31;
			break;

		  case 3:			// Rotation keys, rounds 8..15
			xkey[17]	= (S5[z3]^S6[z2]^S7[zC]^S8[zD]^S5[z9]) & 31;
			xkey[19]	= (S5[z1]^S6[z0]^S7[zE]^S8[zF]^S6[zC]) & 31;
			xkey[21]	= (S5[z7]^S6[z6]^S7[z8]^S8[z9]^S7[z2]) & 31;
			xkey[23]	= (S5[z5]^S6[z4]^S7[zA]^S8[zB]^S8[z6]) & 31;
		
			xkey[25]	= (S5[x8]^S6[x9]^S7[x7]^S8[x6]^S5[x3]) & 31;
			xkey[27]	= (S5[xA]^S6[xB]^S7[x5]^S8[x4]^S6[x7]) & 31;
			xkey[29]	= (S5[xC]^S6[xD]^S7[x3]^S8[x2]^S7[x8]) & 31;
			xkey[31]	= (S5[xE]^S6[xF]^S7[x1]^S8[x0]^S8[xD]) & 31;
			break;
		}
	}

	x0123 = x4567 = x89AB = xCDEF = 0;
	z0123 = z4567 = z89AB = zCDEF = 0;
}

// CAST5encrypt encrypts a block using the CAST5 cipher.

void 
CAST5encrypt(const PGPUInt8 *in, PGPUInt8 *out, const PGPUInt32 *xkey)
{
	PGPUInt32 l, r, t;

	l = (PGPUInt32) (in[0]<<24) | (PGPUInt32) (in[1]<<16) | 
		(PGPUInt32) (in[2]<<8) | in[3];
	r = (PGPUInt32) (in[4]<<24) | (PGPUInt32) (in[5]<<16) | 
		(PGPUInt32) (in[6]<<8) | in[7];

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
	// Stop here if only doing 12 rounds
	t = F1(r, xkey, 12); l ^= G1(t);
	t = F2(l, xkey, 13); r ^= G2(t);
	t = F3(r, xkey, 14); l ^= G3(t);
	t = F1(l, xkey, 15); r ^= G1(t);

	out[0]	= (PGPUInt8) B0(r);
	out[1]	= (PGPUInt8) B1(r);
	out[2]	= (PGPUInt8) B2(r);
	out[3]	= (PGPUInt8) B3(r);
	out[4]	= (PGPUInt8) B0(l);
	out[5]	= (PGPUInt8) B1(l);
	out[6]	= (PGPUInt8) B2(l);
	out[7]	= (PGPUInt8) B3(l);
}

// CAST5decrypt decrypts a block using the CAST5 cipher.

void 
CAST5decrypt(const PGPUInt8 *in, PGPUInt8 *out, const PGPUInt32 *xkey)
{
	PGPUInt32 l, r, t;

	r = (PGPUInt32) (in[0]<<24) | (PGPUInt32) (in[1]<<16) | 
		(PGPUInt32) (in[2]<<8) | in[3];

	l = (PGPUInt32) (in[4]<<24) | (PGPUInt32) (in[5]<<16) | 
		(PGPUInt32) (in[6]<<8) | in[7];

	t = F1(l, xkey, 15); r ^= G1(t);
	t = F3(r, xkey, 14); l ^= G3(t);
	t = F2(l, xkey, 13); r ^= G2(t);
	t = F1(r, xkey, 12); l ^= G1(t);
	// Start here if only doing 12 rounds
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

	out[0]	= (PGPUInt8) B0(l);
	out[1]	= (PGPUInt8) B1(l);
	out[2]	= (PGPUInt8) B2(l);
	out[3]	= (PGPUInt8) B3(l);
	out[4]	= (PGPUInt8) B0(r);
	out[5]	= (PGPUInt8) B1(r);
	out[6]	= (PGPUInt8) B2(r);
	out[7]	= (PGPUInt8) B3(r);
}

// CAST5encryptCFBdbl encrypts the specified number of blocks in CFB mode.

void 
CAST5encryptCFBdbl(
	const PGPUInt32*	xkey, 
	PGPUInt32			iv0, 
	PGPUInt32			iv1, 
	PGPUInt32			iv2, 
	PGPUInt32			iv3, 
	const PGPUInt32*	src, 
	PGPUInt32*			dest, 
	PGPUInt32			len)
{
	PGPUInt32 iv[4] = {iv0, iv1, iv2, iv3};
	while (len--) 
	{
		CAST5encrypt((const PGPUInt8*) iv, (PGPUInt8*) iv, xkey);
		*dest++	= iv[0]	^= *src++;
		*dest++	= iv[1]	^= *src++;

		CAST5encrypt((const PGPUInt8*) (iv+2), (PGPUInt8*) (iv+2), xkey);
		*dest++	= iv[2]	^= *src++;
		*dest++	= iv[3]	^= *src++;
	}
}

// CAST5decryptCFBdbl decrypts the specified number of blocks in CFB mode.

void 
CAST5decryptCFBdbl(
	const PGPUInt32*	xkey, 
	PGPUInt32			iv0, 
	PGPUInt32			iv1, 
	PGPUInt32			iv2, 
	PGPUInt32			iv3, 
	const PGPUInt32*	src, 
	PGPUInt32*			dest, 
	PGPUInt32			len)
{
	PGPUInt32	iv[4]	= {iv0, iv1, iv2, iv3};
	PGPUInt32	out[4];
	while (len--)
	{
		CAST5encrypt((const PGPUInt8*) iv, (PGPUInt8*) out, xkey);
		*dest++	= out[0] ^ (iv[0] = *src++);
		*dest++	= out[1] ^ (iv[1] = *src++);

		CAST5encrypt((const PGPUInt8*) (iv+2), (PGPUInt8*) (out+2), xkey);
		*dest++	= out[2] ^ (iv[2] = *src++);
		*dest++	= out[3] ^ (iv[3] = *src++);
	}
}


//////////////////////////
// CAST Assembly Functions
//////////////////////////

#if defined(PGP_INTEL)

#define	NAKED	__declspec(naked)

/* NOTE: This expects the rotate amounts to be pre-incremented by 16! */

/*
 * high-speed x86 CAST implementation.  Optimized for Intel P5 and P6.
 * (Pentium, PPro, Pentium II).
 *
 * The pentium is a fairly straightforward dual-issue machine with a
 * number of issue restrictions (but the only false hazard that causes
 * a stall is WAW).  The one significant pipeline hazard is that while
 * loads are complete the cycle after they execute, they require that
 * the address be available the cycle before they execute.  One notable
 * point is that the Pentium can execute two loads in parallel, as long
 * as they hit in different banks of its 8-way-interleaved L1 cache.
 *
 * The PPro is an out-of-order superscalar system which can dispatch 3
 * instructions per cycle (subject to significant limitations) and has
 * two integer and one load/store unit.  After decoding x86 instructions
 * to a series of R-ops, it's pretty straightforward out-of-order engine,
 * executing instructions as their operands become available (register
 * renaming ensures that false sharing doesn't block issue) except for
 * one thing.  The P6 is prone to an evil thing called a "partial register
 * stall", when you write to a partial register and read a larger part.
 * this overlap is resolved by retiring the partial write before the
 * next instruction can be dispatched.  This amounts to a *long* time.
 *
 * Unfortunately, precisely this operation is the best way to extract the
 * bytes from the words in the CAST S-box lookup, so it's very important
 * for an efficient CAST implementation.
 * Fortunately, the processor has an exception built in.  The
 * instructions "xor reg,reg" and "sub reg,reg" sets a magic "clear"
 * flag on the register, following which the merge of the high zero
 * bits causes no delay.
 * The only problem is that a context switch or interrupt will save
 * and restore the register, and a mov does *not* set the flag.
 * Thus, you should explicitly re-clear the register occasionally,
 * so that the stall won't happen.
 *
 * Each block encrypted seems like a good value for "occasionally".
 *
 * Also, loop labels whould not be within 7 bytes of a 16-byte boundary.
 */

/*
 * Do round n from x into y, with op1/op2/op3
 * This also loads in the next round's keys into
 * eax and ecx.
 *
 * The minimum length dependency chain is 9 cycles for
 * a round, but achieving that would require issuing
 * three instructions per cycle - perhaps the PPro can
 * manage that.  Issuing two instructions per cycle,
 * a round can be done in 10 cycles.  Unfortunately,
 * the processor does not have enough register to use more
 * parallelism than that.
 *
 * Dependency chain:
 * op3 x,%eax/xor %edx,%edx
 * rol %cl,%eax
 * mov %ah,%bl/mov %al,%dl
 * shr $16,%eax/XXX/AGI
 * mov S1[%ebx],%ecx/mov %ah,%ebx	\ Could be mult-issued
 * mov S2[%edx],%edx/and $255,%eax	/ if we had the bandwidth
 * op1 %edx,%ecx/mov S3[%ebx],%edx
 * op2 %edx,%ecx/mov S4[%eax],%eax
 * op3 %eax,%ecx/mov key,%eax
 * xor %ecx,y/mov shift,%ecx
 *
 */
/*
 * This requires that %ebx be kept "clear" in the P6 sense.
 *
 * NOTE that the NOP *saves* a cycle on the Pentium!  Without it,
 * the following mov is paired with the shr, and suffers an AGI stall,
 * which stalls the shr as well and thus loses two issue slots instead
 * of one.  We have a spare issue slot, but the shr is on the critical
 * path and delaying it adds a cycle.
 */

#define ROUND(x,y,op1,op2,op3,n)					\
	__asm											\
	{												\
		__asm op3 eax, x				/* 1 U  */	\
		__asm xor edx, edx				/*    V */	\
		__asm rol eax, cl				/*2-5NP */	\
		__asm mov bl, ah				/* 6 U  */	\
		__asm mov dl, al				/*    V */	\
		__asm shr eax, 16				/* 7 Ux */	\
		__asm nop /* !!! */				/* AGI  */	\
		__asm mov ecx, S1[ebx*4]		/* 8 U  */	\
		__asm mov bl, ah				/*    V */	\
		__asm mov edx, S2[edx*4]		/* 9 U  */	\
		__asm and eax, 255				/*    V */	\
		__asm op1 ecx, edx				/*10 U  */	\
		__asm mov edx, S3[ebx*4]		/*    V */	\
		__asm op2 ecx, edx				/*11 U  */	\
		__asm mov edx, S4[eax*4]		/*    V */	\
		__asm op3 ecx, edx				/*12 U  */	\
		__asm mov eax, 8+[esi+8*n]		/*    V */	\
		__asm xor y, ecx				/*13 U  */	\
		__asm mov ecx, 12+[esi+8*n]		/*    V */	\
	}

/* void
 * CAST5encrypt(byte const *in, byte *out, word32 const *xkey)
 *                          4         8                  12
 *
 * Register usage:
 * esi points to key schedule
 * eax is used as a temporary, and used for the primary round subkey
 * ebx is all-zero in the high 24 bits, and is used for indexing
 * ecx is used as a temporary, and for the rotate round subkey (PLUS 16)
 * edx is all-zero in the high 24 bits, and is used for indexing
 * esi points to the key schedule
 * edi is the right half of the current block
 * ebp is the left half of the current block
 */

#define	left	ebp
#define	right	edi

NAKED 
void 
__cdecl 
CAST5encryptAsm(const PGPUInt8 *in, PGPUInt8 *out, const PGPUInt32 *xkey)
{
	__asm
	{
		ALIGN 16

		push esi				/*   U  */
		push right				/*    V */

		mov right, 8+[esp+4]	/*   U  */
		mov esi, 8+[esp+12]		/*    V */
		push left				/*   U  */
		push ebx				/*    V */
		mov left, [right]		/*   U  */
		mov right, [right+4]	/*    V */

		bswap right				/*  NP  */
		bswap left				/*  NP  */

		mov eax, [esi]			/*   U  */
		xor ebx,ebx				/*    V */
		mov ecx, [esi+4]		/*   U  */

		ROUND(right, left, xor, sub, add, 0)
		ROUND(left, right, sub, add, xor, 1)
		ROUND(right, left, add, xor, sub, 2)
		ROUND(left, right, xor, sub, add, 3)
		ROUND(right, left, sub, add, xor, 4)
		ROUND(left, right, add, xor, sub, 5)
		ROUND(right, left, xor, sub, add, 6)
		ROUND(left, right, sub, add, xor, 7)
		ROUND(right, left, add, xor, sub, 8)
		ROUND(left, right, xor, sub, add, 9)
		ROUND(right, left, sub, add, xor,10)
		ROUND(left, right, add, xor, sub,11)
		ROUND(right, left, xor, sub, add,12)
		ROUND(left, right, sub, add, xor,13)
		ROUND(right, left, add, xor, sub,14)
	/*	ROUND(left, right, xor, sub, add,15)
	 * Last round: omit loading of keys for next round
	 * Fetch out pointer and store data there instead
	 */
		add eax, left			/* 1 U  */
		xor edx, edx			/*    V */
		rol eax, cl				/*2-5NP */
		mov bl, ah				/* 6 U  */
		mov dl, al				/*    V */
		shr eax, 16				/* 7 Ux */
		mov esi, 16+[esp+8]		/*    V */
		mov ecx, S1[ebx*4]		/* 8 U  */
		mov bl, ah				/*    V */
		mov edx, S2[edx*4]		/* 9 U  */
		and eax, 255			/*    V */
		xor ecx, edx			/*10 U  */
		mov edx, S3[ebx*4]		/*    V */
		sub ecx, edx			/*11 U  */
		mov eax, S4[eax*4]		/*    V */
		add ecx, eax			/*12 U  */
		pop ebx					/*    V */
		bswap left				/*13 NP */
		xor right, ecx			/*14 U  */

		mov [esi+4], left		/*    V */
		bswap right				/*   NP */
		pop left				/*   U  */
		mov [esi], right		/*    V */
		pop right				/*   U  */
		pop esi					/*    V */
		ret						/*   NP */
	}
}

/* 
 * asm void
 * CAST5encryptCFBdbl(
 * 	register word32 const *xkey,	// esp+pushes+ 4
 * 	register word32 in0,		// esp+pushes+ 8
 * 	register word32 in1,		// esp+pushes+12
 * 	register word32 in2,		// esp+pushes+16
 * 	register word32 in3,		// esp+pushes+20
 * 	register word32 const *src,	// esp+pushes+24
 * 	register word32 *dest,		// esp+pushes+28
 * 	register word32 len)		// esp+pushes+32
 *
 * Note that "len" is the number of 16-byte units to encrypt.
 * Since this function only encrypts one block per time
 * around the loop, it has to be doubled.
 *
 * Doing the dbl part...
 * We use the srgument slots on the stack as IV registers, but
 * actually only use one IV, in the in2/in3 slots.
 * Each iteration, we fetch from them the data to be encrypted
 * for the next iteration before storing the current
 * ciphertext for the ineration after the next, thus achieving the
 * necessary interleaving.
 */

#define	left	ebp
#define	right	edi

NAKED 
void 
__cdecl 
CAST5encryptCFBdblAsm(
	const PGPUInt32	*xkey, 
	PGPUInt32		iv0, 
	PGPUInt32		iv1, 
	PGPUInt32		iv2, 
	PGPUInt32		iv3, 
	const PGPUInt32	*src, 
	PGPUInt32		*dest, 
	PGPUInt32		len)
{
	__asm
	{
		ALIGN 16

		push esi
		push right

		mov esi, 8+[esp+4]		/*   U  - load key schedule pointer */
		push left				/*    V */
		mov left, 12+[esp+8]	/*   U  - load in0 as left */
		mov right, 12+[esp+12]	/*    V - load in1 as right */

		push ebx				/*   U  */
		xor ebx,ebx				/*    V */

		shl dword ptr 16+[esp+32], 1	/*   NP - double loop counter */

	encryptloop:
		mov eax, [esi]			/*   U  - preload key material */
		mov ecx, [esi+4]		/*    V - preload key material */

		bswap right				/*   NP  */
		bswap left				/*   NP  */

		ROUND(right, left, xor, sub, add, 0)
		ROUND(left, right, sub, add, xor, 1)
		ROUND(right, left, add, xor, sub, 2)
		ROUND(left, right, xor, sub, add, 3)
		ROUND(right, left, sub, add, xor, 4)
		ROUND(left, right, add, xor, sub, 5)
		ROUND(right, left, xor, sub, add, 6)
		ROUND(left, right, sub, add, xor, 7)
		ROUND(right, left, add, xor, sub, 8)
		ROUND(left, right, xor, sub, add, 9)
		ROUND(right, left, sub, add, xor,10)
		ROUND(left, right, add, xor, sub,11)
		ROUND(right, left, xor, sub, add,12)
		ROUND(left, right, sub, add, xor,13)
		ROUND(right, left, add, xor, sub,14)
	/*	ROUND(left, right, xor, sub, add,15)
	 * Last round: omit loading of keys for next round
	 * Instead, start the CFB operations.  Including the
	 * swap of the halves, that ends up as:
	 *
	 * %eax = src[0] ^ bswap(right)
	 * %ecx = src[1] ^ bswap(left)
	 * src += 8bytes
	 * left = bswap(in2)
	 * right = bswap(in3)
	 * in2 = %eax
	 * dest[0] = %eax
	 * in3 = %ecx
	 * dest[1] = %eax
	 * dest += 8bytes
	 *
	 * For the '486, we can just use bswap.  For the '386, it's
	 * xchg ah,al
	 * rol $16,eax
	 * xchg ah,al
	 *
	 * Annoyingly, this really *sucks* on a PPro, due to fierce
	 * partial-register stalls.  So it pretty much has to go two ways.
	 * Options are:
	 * - Duplicate the entire encryption loop?
	 * - Do it in a pre-pass and a post-pass.  Makes life easy, but we
	 *   end up being memory bound.
	 * - Something truly wierd?
	 */
		add eax, left			/* 1 U  */
		xor edx, edx			/*    V */
		rol eax, cl				/*2-5NP */
		mov bl, ah				/* 6 U  */
		mov dl, al				/*    V */
		shr eax, 16				/* 7 Ux */
		nop						/*    V */
		mov ecx, S1[ebx*4]		/* 8 U  */
		mov bl, ah				/*    V */
		mov edx, S2[edx*4]		/* 9 U  */
		and eax, 255			/*    V */
		xor ecx, edx			/*10 U  */
		mov edx, S3[ebx*4]		/*    V */
		sub ecx, edx			/*11 U  */
		mov edx, S4[eax*4]		/*    V */
		add ecx, edx			/*12 U  */
		mov ebx, 16+[esp+24]	/*    V - fetch src pointer */
		xor right, ecx			/*13 U  */
		add ebx, 8				/*    V - increment src ptr */
		bswap left
  		bswap right
		mov eax, [ebx-8]		/*   U  - get src word */
		mov ecx, [ebx-4]		/*    V - other src word */
		mov 16+[esp+24], ebx	/*   U  - store src pointer back */
		mov edx, 16+[esp+28]	/*    V - fetch dest pointer */
		xor eax, right			/*   U  */
		xor ecx, left			/*    V */
		mov left, 16+[esp+16]	/*   U  - fetch in2 for new left */
		mov right, 16+[esp+20]	/*    V - fetch in3 for new right */
		mov 16+[esp+16], eax	/*   U  - store ciphertext for next time */
		mov 16+[esp+20], ecx	/*    V - store ciphertext for next time */
		mov [edx+0], eax		/*   U  - store result */
		mov [edx+4], ecx		/*    V - store result */
		add edx, 8				/*   U  - increment dest ptr */
		xor ebx, ebx			/*    V - clear %ebx for next iteration */
		dec dword ptr 16+[esp+32]	/*   U  - decrement loop counter (set ZF) */
		mov 16+[esp+28], edx	/*    V - store dest pointer back */
		/* Pairing opportunity lost, sigh */
		jnz encryptloop	

		/* And we're done!  Pop registers and return */

		pop ebx					/*   U  */
		pop left				/*    V */
		pop right				/*   U  */
		pop esi					/*    V */
		ret						/*   NP */
	}
}

/*
 * dbl decode... plain[i] = cipher[i] ^ CRYPT(cipher[i-2])
 * Can we do this with only one temporary (and still allow in-place)?

 * Crypt cipher[i-2] -> plain[i]
 * Load cipher[i-1] from temp
 * Load cipher[i]
 * Store cipher[i] into temp, plain[i] ^= cipher[i] (or vice-versa)
 * Store plain[i] 
 * i++
 * Um... do we have enough regs?  It seems that we have cipher[i],
 * cipher[i-1] and plain[i] in registers at the same time.

 * Try this order of operations:
 * Crypt cipher[i-2] -> plain[i]
 * Load cipher[i]
 * plain[i] ^= cipher[i]
 * store plain[i]
 * load temp into plain[i], which is now cipher[i-1] (cipher[i-2] after i++)
 * store cipher[i] into temp
 * increment i
 * That works, and doesn't use an excessive number of registers.
 *
 * Written out in full,
 * %eax = src[0];
 * %ecx = src[1];
 * src += 8bytes
 * BSWAP(left)
 * BSWAP(right)
 * right ^= %eax
 * left ^= %ecx
 * dest[0] = right
 * dest[1] = left
 * dest += 8bytes
 * left = in2
 * right = in3
 * in2 = %eax
 * in3 = %ecx
 */

NAKED 
void 
__cdecl 
CAST5decryptCFBdblAsm(
	const PGPUInt32	*xkey, 
	PGPUInt32		iv0, 
	PGPUInt32		iv1, 
	PGPUInt32		iv2, 
	PGPUInt32		iv3, 
	const PGPUInt32	*src, 
	PGPUInt32		*dest, 
	PGPUInt32		len)
{
	__asm
	{
		ALIGN 16

		push esi
		push right

		mov esi, 8+[esp+4]		/*   U  - load key schedule pointer */
		push left				/*    V */
		mov left, 12+[esp+8]	/*   U  - load in0 as left */
		mov right, 12+[esp+12]	/*    V - load in1 as right */

		push ebx				/*   U  */
		xor ebx,ebx				/*    V */

		shl dword ptr 16+[esp+32], 1	/*   NP - double loop counter */

	decryptloop:
		mov eax, [esi]			/*   U  - preload key material */
		mov ecx, [esi+4]		/*    V - preload key material */

		bswap right				/*   NP  */
		bswap left				/*   NP  */

		ROUND(right, left, xor, sub, add, 0)
		ROUND(left, right, sub, add, xor, 1)
		ROUND(right, left, add, xor, sub, 2)
		ROUND(left, right, xor, sub, add, 3)
		ROUND(right, left, sub, add, xor, 4)
		ROUND(left, right, add, xor, sub, 5)
		ROUND(right, left, xor, sub, add, 6)
		ROUND(left, right, sub, add, xor, 7)
		ROUND(right, left, add, xor, sub, 8)
		ROUND(left, right, xor, sub, add, 9)
		ROUND(right, left, sub, add, xor,10)
		ROUND(left, right, add, xor, sub,11)
		ROUND(right, left, xor, sub, add,12)
		ROUND(left, right, sub, add, xor,13)
		ROUND(right, left, add, xor, sub,14)
	/*	ROUND(left, right, xor, sub, add,15)
	 * Last round: omit loading of keys for next round
	 * Instead, start the CFB operations.  Including the
	 * swap of the halves, that ends up as:
	 *
	 * %eax = src[0];
	 * %ecx = src[1];
	 * src += 8bytes
	 * BSWAP(left)
	 * BSWAP(right)
	 * right ^= %eax
	 * left ^= %ecx
	 * dest[0] = right
	 * dest[1] = left
	 * dest += 8bytes
	 * left = in2
	 * right = in3
	 * in2 = %eax
	 * in3 = %ecx
	 */
		add eax, left			/* 1 U  */
		xor edx, edx			/*    V */
		rol eax, cl				/*2-5NP */
		mov bl, ah				/* 6 U  */
		mov dl, al				/*    V */
		shr eax, 16				/* 7 Ux */
		nop						/*    V */
		mov ecx, S1[ebx*4]		/* 8 U  */
		mov bl, ah				/*    V */
		mov edx, S2[edx*4]		/* 9 U  */
		and eax, 255			/*    V */
		xor ecx, edx			/*10 U  */
		mov edx, S3[ebx*4]		/*    V */
		sub ecx, edx			/*11 U  */
		mov edx, S4[eax*4]		/*    V */
		add ecx, edx			/*12 U  */
		mov ebx, 16+[esp+24]	/*    V - fetch src pointer */
		xor right, ecx			/*13 U  */
		add ebx, 8				/*    V - increment src ptr */
		bswap left
  		bswap right
		mov eax, [ebx-8]		/*   U  - get src word */
		mov ecx, [ebx-4]		/*    V - other src word */
		mov 16+[esp+24], ebx	/*   U  - store src pointer back */
		mov edx, 16+[esp+28]	/*    V - fetch dest pointer */
		xor right, eax			/*   U  */
		xor left, ecx			/*    V */
		mov [edx+0], right		/*   U  - store result */
		mov [edx+4], left		/*    V - store result */
		mov left, 16+[esp+16]	/*   U  - fetch in2 for new left */
		mov right, 16+[esp+20]	/*    V - fetch in3 for new right */
		mov 16+[esp+16], eax	/*   U  - store ciphertext for next time */
		mov 16+[esp+20], ecx	/*    V - store ciphertext for next time */
		add edx, 8				/*   U  - increment dest ptr */
		xor ebx, ebx			/*    V - clear %ebx for next iteration */
		dec dword ptr 16+[esp+32]	/*   U  - decrement loop counter (set ZF) */
		mov 16+[esp+28], edx	/*    V - store dest pointer back */
		/* Pairing opportunity lost, sigh */
		jnz decryptloop	

		/* And we're done!  Pop registers and return */

		pop ebx					/*   U  */
		pop left				/*    V */
		pop right				/*   U  */
		pop esi					/*    V */
		ret						/*   NP */
	}
}

#endif	// PGP_INTEL


/////////////////////////
// CAST Testing Functions
/////////////////////////

// VerifyCastSBoxesValid verifies that the CAST S-boxes haven't become
// corrupted in memory.
	
PGPBoolean 
VerifyCastSBoxesValid()
{
	PGPBoolean	valid	= TRUE;
	CRC32		crc		= 0;

	pgpAssert(((PGPUInt32) &S1[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S2[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S3[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S4[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S5[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S6[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S7[0] & 0x3) == 0);
	pgpAssert(((PGPUInt32) &S8[0] & 0x3) == 0);

	crc	= ContinueCRC32(S1, sizeof(S1) / sizeof(S1[0]), crc);
	crc	= ContinueCRC32(S2, sizeof(S2) / sizeof(S2[0]), crc);
	crc	= ContinueCRC32(S3, sizeof(S3) / sizeof(S3[0]), crc);
	crc	= ContinueCRC32(S4, sizeof(S4) / sizeof(S4[0]), crc);
	crc	= ContinueCRC32(S5, sizeof(S5) / sizeof(S5[0]), crc);
	crc	= ContinueCRC32(S6, sizeof(S6) / sizeof(S6[0]), crc);
	crc	= ContinueCRC32(S7, sizeof(S7) / sizeof(S7[0]), crc);
	crc	= ContinueCRC32(S8, sizeof(S8) / sizeof(S8[0]), crc);

	valid = (crc == 0xD8D4C8D7);

	return valid;
}


#if 0	// KEEP [
/*
 * Appendix C.
 * This appendix provides test vectors for the CAST-128 cipher described
 * in Section 5 and in Appendices A and B.
 * C.1. Single Key-Plaintext-Ciphertext Set
 * 128-bit key       = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A (hex)
 * 64-bit plaintext  = 01 23 45 67 89 AB CD EF (hex)
 * 64-bit ciphertext = 23 8B 4F E5 84 7E 44 B2 (hex)
 * C.2. Full Maintenance Test
 * A maintenance test for CAST-128 has been defined to verify the correctness
 * of implementations.  It is defined in pseudo-code as follows, where a and
 * b are 128-bit vectors, aL and aR are the leftmost and rightmost halves of
 * a, bL and bR are the leftmost and rightmost halves of b, and encrypt(d,
 * k) is the encryption in ECB mode of block d under key k.
 * 
 * .
 * Initial 
 * a = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A (hex)
 * Initial 
 * b = 01 23 45 67 12 34 56 78 23 45 67 89 34 56 78 9A (hex)
 * do 1,000,000 times
 * {
 *     aL = encrypt( aL , b)
 *     aR = encrypt( aR , b)
 *     bL = encrypt( bL , a)
 *     bR = encrypt( bR , a)
 * }
 * Verify 
 * a == EE A9 D0 A2 49 FD 3B A6 B3 43 6F B8 9D 6D CA 92 (hex)
 * Verify 
 * b == B2 C9 5E B0 0C 31 AD 71 80 AC 05 B8 E8 3D 69 6E (hex)
 * 	
 * 
 */

#include <stdio.h>
#include <strings.h>

void 
printhex(const char *msg, const PGPUInt8 *num, int len)
{
	int i;

	printf(msg);

	for (i=0; i<len; i++)
		printf("%X", num[i]);

	printf("\n");
}

int
main(void)
{
	PGPUInt8 a[16] = 
	{
		0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
		0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
	};
	PGPUInt8 b[16] = 
	{
		0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78,
		0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A
	};
	PGPUInt8 test[8] =
	{
		0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
	};
	static PGPUInt8 const expect1[8] =
	{
		 0x23, 0x8B, 0x4F, 0xE5, 0x84, 0x7E, 0x44, 0xB2
	};
	static PGPUInt8 const expecta[16] =
	{
		0xEE, 0xA9, 0xD0, 0xA2, 0x49, 0xFD, 0x3B, 0xA6,
		0xB3, 0x43, 0x6F, 0xB8, 0x9D, 0x6D, 0xCA, 0x92
	};
	static PGPUInt8 const expectb[16] =
	{
		0xB2, 0xC9, 0x5E, 0xB0, 0x0C, 0x31, 0xAD, 0x71,
		0x80, 0xAC, 0x05, 0xB8, 0xE8, 0x3D, 0x69, 0x6E
	};
	PGPUInt32	xkey[32];
	long	count;
	printf("Simple sanity check...\n");
	
	printhex("       Key =", a, sizeof(a));
	CAST5schedule(xkey, a);
	printhex(" Plaintext =", test, sizeof(test));
	CAST5encrypt(test, test, xkey);
	printhex("Ciphertext =", test, sizeof(test));
	printhex("    Expect =", expect1, sizeof(expect1));
	if (memcmp(test, expect1, sizeof(test)) != 0)
		printf("		***** ERROR *****\n");
	
	printf("Full maintenance test (4,000,000 encryptions)...\n");
	for (count = 0 ;count < 1000000; count++) {
		CAST5schedule(xkey, b);
		CAST5encrypt(a, a, xkey);
		CAST5encrypt(a+8, a+8, xkey);
		CAST5schedule(xkey, a);
		CAST5encrypt(b, b, xkey);
		CAST5encrypt(b+8, b+8, xkey);
	}
	printhex("     a =", a, sizeof(a));
	printhex("Expect =", expecta, sizeof(expecta));
	if (memcmp(a, expecta, sizeof(a)) != 0)
		printf("		***** ERROR *****\n");
	printhex("     b =", b, sizeof(b));
	printhex("Expect =", expectb, sizeof(expectb));
	if (memcmp(b, expectb, sizeof(b)) != 0)
		printf("		***** ERROR *****\n");
	{
		char bob[1];
		gets(bob);
	}

	return 0;
}

#endif	// ] KEEP
