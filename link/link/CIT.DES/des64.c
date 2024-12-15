/*
 *	des64.c - contains routines to perform encryption/decryption of 64-bit
 *		datablocks using the DES Data Encryption Algorithm.
 *		A description of the algorithm may be found in:
 *
 *	    Australian Standard AS2805.5-1985   The Data Encryption Algorithm
 *	    ANSI Standard X3.92-1981		The Data Encryption Algorithm
 *	    ANSI Standard X3.106-1983		DEA - Modes of Operation
 *
 *  Author:	Lawrence Brown <lpb@cs.adfa.oz.au>		Dec 1987
 *		Computer Science, UC UNSW, Australian Defence Force Academy,
 *			Canberra, ACT 2600, Australia.
 *
 *		The user interface and some code has been inspired by the PD
 *		programs of:	Phil Karn, KA9Q <karn@flash.bellcore.com> 1986
 *		  and		Jim Gillogly 1977.
 *		and by 		Michael Barr-David's honours DES program 1987
 *
 *	nb: if this program is compiled on a little-endian machine (eg Vax)
 *		#define LITTLE_ENDIAN
 *		in order to enable the byte swapping  routines 
 *
 *	    if compiled on a machine requiring strict alignment of words/longs
 *		and cannot guarantee that the 8 char blocks passed are aligned
 *		#define STRICT_ALIGN
 *
 *	    if a detailed trace of DES function f is required for debugging
 *		#define TRACE
 *
 *  Copyright 1988 by Lawrence Brown and CITRAD. All rights reserved.
 *      This program may not be sold or used as inducement to buy a
 *      product without the written permission of the author.
 *
 *  Modifications:
 *		v2.0 - move round calc in-line, expand() in-line, bit() in-line
 *		v2.1 - move round calc in-line without loop
 *		v2.2 - use compiled permutation functions instead of tables
 *		v2.3 - replace compiled functions by macros for added speed
 *		v3.0 - combine S & P operations into a single table
 *		v3.1 - pass subkey as a parameter to keyinit, endes, dedes
 *		v3.2 - dont pass subkey, use char arrays for blocks
 *		v4.0 - tighten coding in fn f() and keyinit()
 */

#ifndef lint
static char rcsid[]  = "$Header: des64.c,v 2.1 90/07/18 10:54:33 lpb Exp $";
#endif

#include <stdio.h>
#include "des.h"	/* include Interface Specification header file       */
#include "despriv.h"	/* include Interface Specification header file       */
#include "gen64.i"	/* include DES Permutation, Substitution & Key tables*/

/*
 *	des64 global variables - may be accessed directly if necessary
 *
 * 	des64_subkeys - Storage for the 16 sub-keys used
 *	iv - Initialization Vector used in CBC CFB & OFB modes
 */
Long	des64_subkeys[ROUNDS][DESBLK/4] = {0};
Long	dea_IV[2]  = {0, 0};

/*	string specifying version and copyright message */
char	*des_lib_ver = "64-bit DES library v4.0, Copyright (C) 1989 Lawrence Brown & CITRAD";

static Long	f();		/* declare DES function f		     */
static void	rotl28();	/* declare local function rotl28             */

/*
 *	desinit - init DES library, for compatibility
 */
void
desinit(mode)
int	mode;	/* DES mode required - 0 is std DES, only legal mode */
{
	dea_IV[0] = dea_IV[1] = 0;	/* reset IV */

	if (mode != 0)
		fprintf(stderr, "Illegal DES mode %d requested\n", mode);
}

/*
 *	desdone - done DES library, null fn for compatibility
 */
void
desdone()
{
}

void
setIV(iv)
char	iv[DESBLK];		/* Key to use, stored as an array of Longs    */
{
#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(iv);			/* swap bytes round if little-endian */
#endif

#if	TRACE == 1
	fprintf(stderr,"\n  setIV(%08lx, %08lx) ",
		((Long *)iv)[0], ((Long *)iv)[1]);
#endif  TRACE

	dea_IV[0] = ((Long *)iv)[0];	/* set IV to specified value */
	dea_IV[1] = ((Long *)iv)[1];

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(iv);			/* restore byte order before return */
#endif
}


/*
 *	endes(b) - main DES encryption routine, this routine encrypts one 64-bit
 *			block b using the DES algorithm with des64_subkeys
 *
 *		The encryption operation involves permuting the input block
 *		using permutation IP, applying a DES round sixteen times
 *		(which ensures the output is a complex function of the input,
 *		and the key), and finally applying permutation FP (inverse IP).
 *
 *		Each round performs the following calculation using a 48-bit
 *		subkeys:
 *			L(i) = R(i-1)
 *			R(i) = L(i-1) XOR f(R(i-1), K(i))
 *
 *		To save swapping, alternate calls to f use L & R respectively
 *
 *		nb: The 64-bit block is passed as two longwords. For the
 *			purposes of the DES algorithm, the bits are numbered:
 *			    [1 2 3 ... 32] [33 34 35 ... 64] in thew two halves.
 *			The L (left) half is b[0], the R (right) half is b[1]
 *
 *   		    The sixteen 48-bit DES subkeys are saved as an array of
 *   			sixteen sets of eight 6-bit values (per byte)
 */
void
endes(b)
char	b[DESBLK];
{
	register Long	L, R;			/* left & right data halves  */

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(b);			/* swap bytes round if little-endian */
#endif

#if	TRACE == 1
	fprintf(stderr,"\n  endes(%08lx, %08lx) ", ((Long *)b)[0], ((Long *)b)[1]);
#endif  TRACE

	ip(L, R, b);				/* Apply IP to input block   */

	/* Perform the 16 rounds using sub-keys in usual order               */
	L ^= f(R, des64_subkeys[0]);
	R ^= f(L, des64_subkeys[1]);
	L ^= f(R, des64_subkeys[2]);
	R ^= f(L, des64_subkeys[3]);
	L ^= f(R, des64_subkeys[4]);
	R ^= f(L, des64_subkeys[5]);
	L ^= f(R, des64_subkeys[6]);
	R ^= f(L, des64_subkeys[7]);
	L ^= f(R, des64_subkeys[8]);
	R ^= f(L, des64_subkeys[9]);
	L ^= f(R, des64_subkeys[10]);
	R ^= f(L, des64_subkeys[11]);
	L ^= f(R, des64_subkeys[12]);
	R ^= f(L, des64_subkeys[13]);
	L ^= f(R, des64_subkeys[14]);
	R ^= f(L, des64_subkeys[15]);

	fp(b, R, L);				/* Swap & Perform Inverse IP */

#if	TRACE == 1
	fprintf(stderr,"\n  endes returns %08lx, %08lx ", ((Long *)b)[0], ((Long *)b)[1]);
#endif  TRACE

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(b);			/* swap bytes round if little-endian */
#endif
}


/*
 *	dedes(b) - main DES decryption routine, this routine decrypts one 64-bit
 *			block b using the DES algorithm with des64_subkeys
 *
 *		Decryption uses the same algorithm as encryption, except that
 *		the subkeys are used in reverse order.
 */
void
dedes(b)
char	b[DESBLK];
{
	register Long	L, R;			/* left & right data halves  */

#if	TRACE == 1
	fprintf(stderr,"\n  dedes(%08lx, %08lx) ", ((Long *)b)[0], ((Long *)b)[1]);
#endif  TRACE

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(b);			/* swap bytes round if little-endian */
#endif

	ip(L, R, b);				/* Apply IP to input block   */

	/* Perform the 16 rounds using sub-keys in reverse order to encrypt  */
	L ^= f(R, des64_subkeys[15]);
	R ^= f(L, des64_subkeys[14]);
	L ^= f(R, des64_subkeys[13]);
	R ^= f(L, des64_subkeys[12]);
	L ^= f(R, des64_subkeys[11]);
	R ^= f(L, des64_subkeys[10]);
	L ^= f(R, des64_subkeys[9]);
	R ^= f(L, des64_subkeys[8]);
	L ^= f(R, des64_subkeys[7]);
	R ^= f(L, des64_subkeys[6]);
	L ^= f(R, des64_subkeys[5]);
	R ^= f(L, des64_subkeys[4]);
	L ^= f(R, des64_subkeys[3]);
	R ^= f(L, des64_subkeys[2]);
	L ^= f(R, des64_subkeys[1]);
	R ^= f(L, des64_subkeys[0]);

	fp(b, R, L);				/* Swap & Perform Inverse IP */

#if	TRACE == 1
	fprintf(stderr,"\n  dedes returns %08lx, %08lx ", ((Long *)b)[0], ((Long *)b)[1]);
#endif  TRACE

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(b);			/* swap bytes round if little-endian */
#endif
}


/*
 *	f(r, k) - is the complex non-linear DES function, whose output
 *		is a complex function of both input data and sub-key
 *		The input data R(i-1) is expanded to 48-bits via expansion fn E
 *		Due to the regular nature of the expansion matrix E,
 *		this function implements it directly for efficiency reasons
 *		It takes each 4-bit nybble of the input word, and concatenates
 *		it with the adjacent bit on either side to form a 6-bit value.
 *		(nb: bit 32 is assumed to be adjacent to bit 1)
 *		Next it is XOR'd with the subkeys K(i), substituted into the 
 *		S-boxes, and finally permuted by P. ie the calculation is:
 *			A = E(R(i-1)) XOR K(i)		(a 48-bit value)
 *			B = S(A)			(a 32-bit value)
 *			f = P(B)			(a 32-bit value)
 *
 *	nb: the 48-bit values are stored as two 24-bit values 
 *		(in the lower part of 2 Longs). Overall the bit numbering is:
 *	    [x x .... 1 2 3 4 ... 22 23 24] [x x ... 25 26 27 ... 46 47 48]
 *	nb: the 6-bit S-box input value [x x 1 2 3 4 5 6] is interpreted as:
 *		bits [1 6] select a row within each box,
 *		bits [2 3 4 5] then select a column within that row
 *		The SP array has been reordered by desgen to reflect this
 */
#define MASK6a	077000000		/* mask used in fn E, mask LS6-bits */
#define MASK6b	0770000			/* mask used in fn E, mask LS6-bits */
#define MASK6c	07700			/* mask used in fn E, mask LS6-bits */
#define MASK6d	077			/* mask used in fn E, mask LS6-bits */

static Long
f(r, k)
register Long	r;	/* Data value R(i-1) */
Long		*k;	/* Subkey     K(i)   */
{
	register Long	b;		/* 32 bit SP-box output block       */
	register Long	a1, a2;		/* expanded 48-bit r value          */

	/* expand input data R(i) to 48 bits using fn E    */
	a1 = ((r<<23) | (r>>9)) & MASK6a |	/* bits 32  1  2  3  4  5 */
	     (r >> 11) & MASK6b |		/* bits  4  5  6  7  8  9 */
	     (r >> 13) & MASK6c |		/* bits  8  9 10 11 12 13 */
	     (r >> 15) & MASK6d;		/* bits 12 13 14 15 16 17 */
	a2 = (r << 07) & MASK6a |		/* bits 16 17 18 19 20 21 */
	     (r << 05) & MASK6b |		/* bits 20 21 22 23 24 25 */
	     (r << 03) & MASK6c |		/* bits 24 25 26 27 28 29 */
	     ((r<<01) | (r>>31)) & MASK6d;	/* bits 28 29 30 31 32  1 */

	a1 ^= k[0];			/* A = A XOR K */
	a2 ^= k[1];

	/* Lookup SP-boxes to get B = P(S( E(R(i-1)) XOR K(i) ) )          */
	b =	SP[0][(a1>>18) & MASK6d] |
		SP[1][(a1>>12) & MASK6d] |
		SP[2][(a1>> 6) & MASK6d] |
		SP[3][(a1    ) & MASK6d] |
		SP[4][(a2>>18) & MASK6d] |
		SP[5][(a2>>12) & MASK6d] |
		SP[6][(a2>> 6) & MASK6d] |
		SP[7][(a2    ) & MASK6d];

#if	TRACE == 2	/* If Tracing, dump R(i-1), K(i), and f(R(i-1),K(i)) */
	fprintf(stderr,"\n  f(%08lx, %08lo %08lo) = %08lx", r, k[0], k[1], b);
#endif

	return(b);			/* f returns the result B          */
}


/*
 *	keyinit(key) - expands a key for use in subsequent encryptions 
 *		and decryptions. 
 *
 *		The key is passed as a 64-bit value, of which the 56 non
 *		parity bits are used. The parity bits are in DES bits
 *		8, 16, 24, 32, 40, 48, 56, 64 (nb: these do NOT correspond to
 *		the parity bits in ascii chars when packed in the usual way).
 *		The function performs the key scheduling calculation, saving
 *		the resulting sixteen 48-bit sub-keys for use in subsequent
 *		encryption/decryption calculations. These 48-bit values are
 *		saved as eight 6-bit values, as detailed previously.
 *
 *		The key scheduling calculation involves
 *			permuting the input key by PC1 which selects 56-bits
 *			dividing the 56-bit value into two halves C, D
 *			sixteen times 
 *			  rotates each half left by 1 or 2 places according
 *			    to schedule in keyrot
 *			  concatenates the two 28-bit values, and permutes with
 *			    PC2 which selects 48-bits to become the subkey
 *
 *	nb: the two 28-bit halves are stored in two longwords, with bits 
 *		numbered as [1 2 ... 27 28 x x x x] ... [29 30 ... 48 x x x x]
 *		in Longs	     n[0]		n[1]	(MSB to LSB)
 *		the bottom 4 bits in each longword are ignored
 *		This scheme is used in keyinit(), and rotl28()
 *
 * 
 *	rotl28(b, pos) - macro to rotate block b (where b is a 28-bit key half)
 *		left by pos bits, (0 <= pos <= 32)
 */

#define	MASK28	0xFFFFFFF0L		/* Mask DES key bits 1 to 28      */

#define rotl28(b, pos) { b = ((b << pos) | (b >> (28-pos)) ) & MASK28; }

void
keyinit(key)
char	key[DESBLK];		/* Key to use, stored as an array of Longs    */
{
	register Long	C, D;	/* Storage for two 28-bit key halves C and D  */
	register int	i, rot;	/* tmp counter & current rotation value       */

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(key);			/* swap bytes round if little-endian */
#endif

#if	TRACE == 1
	fprintf(stderr,"\n  keyinit(%08lx, %08lx) ", ((Long *)key)[0], ((Long *)key)[1]);
#endif  TRACE

	pc1(C, D, key);			/* Permute key with PC1               */

	for (i=0; i<16; i++) {		/* Form sixteen subkeys               */
		rot = keyrot[i];
		rotl28(C, rot);		/* Rotate by 1 or 2 bits              */
		rotl28(D, rot);		/*   according to schedule            */
		pc2(des64_subkeys[i], C, D);	/* Apply PC2 to form subkey  i*/
	}

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
	bswap(key);			/* restore byte order before return */
#endif
}
