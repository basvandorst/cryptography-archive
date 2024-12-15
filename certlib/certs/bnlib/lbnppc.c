#include "lbnppc.h"
#include "ppcasm.h"

/*
 * lbnppc.c - Assembly primitives for the bignum library, PowerPC version.
 *
 * Copyright (c) 1995  Colin Plumb.  All rights reserved.
 * For licensing and other legal details, see the file legal.c.
 *
 * On the PPC 601, unrolling the loops more doesn't seem to speed things
 * up at all.  I'd be curious if other chips differed.
 */
#ifndef UNROLL
#define UNROLL 1
#endif
/*
 * MulN1 expects (*out, *in, len, k), count >= 1
 *                r3    r4   r5   r6
 */
static const unsigned mulN1[] = {
	PPC_LWZ(7,4,0), 	/* Load first word of in in r7 */
	PPC_MTCTR(5),		/* Move count into CTR */
	PPC_MULLW(8,7,6),	/* Low half of multiply in r8 */
	PPC_ADDIC(0,0,0),	/* Clear carry bit for loop */
	PPC_MULHWU(5,7,6),	/* High half of multiply in r5 */
	PPC_STW(8,3,0),
	PPC_BC(18,31,1+6*UNROLL),	/* Branch to Label if --ctr == 0 */
#if UNROLL >= 4
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5),	/* Add carry word r5 and bit CF to r8 */
	PPC_STWU(8,3,4),	/* *++out = r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_BC(18,31,19),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 3
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5),	/* Add carry word r5 and bit CF to r8 */
	PPC_STWU(8,3,4),	/* *++out = r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_BC(18,31,13),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 2
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5),	/* Add carry word r5 and bit CF to r8 */
	PPC_STWU(8,3,4),	/* *++out = r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_BC(18,31,7),	/* Branch to Label if --ctr == 0 */
#endif
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5),	/* Add carry word r5 and bit CF to r8 */
	PPC_STWU(8,3,4),	/* *++out = r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_BC(16,31,1-6*UNROLL),	/* Branch to Loop if --ctr != 0 */
/* Label: */
	PPC_ADDZE(5,5),		/* Add carry flag to r5 */
	PPC_STW(5,3,4),		/* out[1] = r5 */
	PPC_BLR()
};

/*
 * MulAdd1 expects (*out, *in, len, k), count >= 1
 *                  r3    r4   r5   r6
 */
static unsigned const mulAdd1[] = {
	PPC_LWZ(7,4,0), 	/* Load first word of in in r7 */
	PPC_LWZ(0,3,0),		/* Load first word of out into r0 */
	PPC_MTCTR(5),		/* Move count into CTR */
	PPC_MULLW(8,7,6),	/* Low half of multiply in r8 */
	PPC_MULHWU(5,7,6),	/* High half of multiply in r5 */
	PPC_ADDC(8,8,0),	/* r8 = r8 + r0 */
	PPC_STW(8,3,0),		/* Store result to memory */
	PPC_BC(18,31,1+9*UNROLL),	/* Branch to Label if --ctr == 0 */
#if UNROLL >= 4
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5), 	/* Add carry word r5 and carry bit CF to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_ADDC(8,8,0),	/* r8 = r8 + r0 */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_BC(18,31,28),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 3
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5), 	/* Add carry word r5 and carry bit CF to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_ADDC(8,8,0),	/* r8 = r8 + r0 */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_BC(18,31,19),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 2
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5), 	/* Add carry word r5 and carry bit CF to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_ADDC(8,8,0),	/* r8 = r8 + r0 */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_BC(18,31,10),	/* Branch to Label if --ctr == 0 */
#endif
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDE(8,8,5), 	/* Add carry word r5 and carry bit CF to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_ADDC(8,8,0),	/* r8 = r8 + r0 */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_BC(16,31,1-9*UNROLL),	/* Branch to Loop if --ctr != 0 */
/* Label: */
	PPC_ADDZE(3,5),		/* Add carry flag to r5 and move to r3 */
	PPC_BLR()
};

/*
 * MulSub1 expects (*out, *in, len, k), count >= 1
 *                  r3    r4   r5   r6
 *
 * Multiply and subtract is rather a pain.  If the subtract of the
 * low word of the product from out[i] generates a borrow, we want to
 * increment the carry word (initially in the range 0..0xfffffffe).
 * However, the PPC's carry bit CF is *clear* after a subtract, so
 * we want to add (1-CF) to the carry word.  This is done using two
 * instructions:
 * SUBFME, subtract from minus one extended.  This computes
 * rD = ~rS + 0xffffffff + CF.  Since rS is from 0 to 0xfffffffe,
 * ~rS is from 1 through 0xffffffff, and the sum with 0xffffffff+CF is
 * from 0 through 0xfffffffff, setting the carry flag unconditionally, and
 * NOR, which is used as a bitwise invert NOT instruction.
 * The SUBFME performs the computation rD = ~rS + 0xffffffff + CF,
 * = (-rS - 1) + (CF - 1) = -(rS - CF + 1) - 1 = ~(rS + 1-CF),
 * which is the bitwise complement of the value we want.
 * We want to add the complement of that result to the low word of the
 * product, which is just what a subtract would do, if only we could get
 * the carry flag clear.  But it's always set, except for SUBFE, and the
 * operation we just performed unconditionally *sets* the carry flag.  Ugh.
 * So find the complement in a separate instruction.
 */
static unsigned const mulSub1[] = {
	PPC_LWZ(7,4,0), 	/* Load first word of in in r7 */
	PPC_LWZ(0,3,0),		/* Load first word of out into r0 */
	PPC_MTCTR(5),		/* Move count into CTR */
	PPC_MULLW(8,7,6),	/* Low half of multiply in r8 */
	PPC_MULHWU(5,7,6),	/* High half of multiply in r5 */
	PPC_SUBFC(8,8,0),	/* r8 = r0 - r8, setting CF */
	PPC_STW(8,3,0),		/* Store result to memory */
	PPC_SUBFME(5,5),	/* First of two instructions to add (1-CF) to r5 */
	PPC_BC(18,31,1+11*UNROLL),	/* Branch to Label if --ctr == 0 */
#if UNROLL >= 4
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_NOR(5,5,5),		/* Second of two instructions to add (1-CF) to r5 */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDC(8,8,5), 	/* Add carry word r5 to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_SUBFC(8,8,0),	/* r8 = r0 - r8, setting CF */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_SUBFME(5,5),	/* First of two instructions to add (1-CF) to r5 */
	PPC_BC(18,31,34),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 3
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_NOR(5,5,5),		/* Second of two instructions to add (1-CF) to r5 */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDC(8,8,5), 	/* Add carry word r5 to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_SUBFC(8,8,0),	/* r8 = r0 - r8, setting CF */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_SUBFME(5,5),	/* First of two instructions to add (1-CF) to r5 */
	PPC_BC(18,31,23),	/* Branch to Label if --ctr == 0 */
#endif
#if UNROLL >= 2
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_NOR(5,5,5),		/* Second of two instructions to add (1-CF) to r5 */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDC(8,8,5), 	/* Add carry word r5 to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_SUBFC(8,8,0),	/* r8 = r0 - r8, setting CF */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_SUBFME(5,5),	/* First of two instructions to add (1-CF) to r5 */
	PPC_BC(18,31,12),	/* Branch to Label if --ctr == 0 */
#endif
/* Loop: */
	PPC_LWZU(7,4,4),	/* r7 = *++in */
	PPC_LWZU(0,3,4),	/* r0 = *++out */
	PPC_NOR(5,5,5),		/* Second of two instructions to add (1-CF) to r5 */
	PPC_MULLW(8,7,6),	/* r8 = low word of product */
	PPC_ADDC(8,8,5), 	/* Add carry word r5 to r8 */
	PPC_MULHWU(5,7,6),	/* r5 is high word of product, for carry word */
	PPC_ADDZE(5,5),		/* Add carry bit from low add to r5 */
	PPC_SUBFC(8,8,0),	/* r8 = r0 - r8, setting CF */
	PPC_STW(8,3,0), 	/* *out = r8 */
	PPC_SUBFME(5,5),	/* First of two instructions to add (1-CF) to r5 */
	PPC_BC(16,31,1-UNROLL*11),	/* Branch to Loop if --ctr != 0 */
/* Label: */
	PPC_NOR(3,5,5),		/* Finish adding (1-CF) to r5, store in r3 */
	PPC_BLR()
};

#if 0
/*
 * Args: BNWORD32 *n, BNWORD32 const *mod, unsigned mlen, BNWORD32 inv)
 *                r3                  r4            r5             r6
 * r7, r8 and r9 are the triple-width accumulator.
 * r0 and r10 are temporary registers.
 * r11 and r12 are temporary pointers into n and mod, respectively. 
 * r2 (!) is another temporary register.
 */
static unsigned const montReduce[] = {
	PPC_MTCTR(5),	/* ??? */
	PPC_LWZ(7,3,0),		/* Load low word of n into r7 */
	PPC_LWZ(10,4,0),	/* Fetch low word of mod */
	PPC_MULLW(0,7,6),	/* Invert r7 into r0 */
	PPC_STW(0,3,0),		/* Store back for future use */
	PPC_MULHWU(8,10,7),	/* Get high word of whatnot */
	PPC_MULLW(10,10,7),	/* Get low word of it */
	PPC_ADDC(7,7,10),	/* Add low word of product to r7 */
	PPC_ADDZE(8,8),		/* Add carry to high word */
	PPC_
	

	PPC_MULHW(8,7,6),
	PPC_ADDC(7,7,0),	/* Add inverse back to r7 */
	PPC_ADDZE(8,8),
	PPC_
	
	PPC_LWZU(
/* Loop: */
	PPC_LWZU(0,11,4),
	PPC_LWZU(10,23,-4),
	PPC_MULLW(2,0,10),
	PPC_ADDC(7,7,2),
	PPC_MULHWU(0,0,10),
	PPC_ADDE(8,8,0),
	PPC_ADDZE(9,9),
	PPC_BC(16,31,-7),	/* Branch to Loop if --ctr != 0 */

	PPC_ADDIC_(count,-1),
	PPC_LWZU(0,x,4),
	PPC_ADDC(0,7,0),
	PPC_STW(0,x,0),
	PPC_ADDZE(7,8),
	PPC_ADDZE(8,9),
	PPC_LI(9,0),
	PPC_BC(xx,2,yy),
	
};
#endif

/*
 * Three overlapped transition vectors for three functions.
 * A PowerPC transition vector for a (potentially) inter-module
 * jump or call consists of two words, an instruction address
 * and a Table Of Contents (TOC) pointer, which is loaded into
 * r1.  Since none of the routines here have global variables,
 * they don't need a TOC pointer, so the value is unimportant.
 * This array places an unintersting 32-bit value after each address.
 */
unsigned const * const lbnPPC_tv[] = {
	mulN1,
	mulAdd1,
	mulSub1,
	0
};


/* 45678901234567890123456789012345678901234567890123456789012345678901234567 */
