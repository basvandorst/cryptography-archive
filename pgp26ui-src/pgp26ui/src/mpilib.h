/*	C include file for MPI multiprecision integer math routines.

	Boulder Software Engineering
	3021 Eleventh Street
	Boulder, CO 80304
	(303) 541-0140

	(c) Copyright 1986-92 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use
	of this software, even if the damage results from defects in this
	software.  No warranty is expressed or implied.

	These routines implement all of the multiprecision arithmetic necessary
	for Rivest-Shamir-Adleman (RSA) public key cryptography, as well as
	other number-theoretic algorithms such as ElGamal, Diffie-Hellman,
	or Rabin.

	Although originally developed in Microsoft C for the IBM PC, this code
	contains few machine dependencies.  It assumes 2's complement
	arithmetic.  It can be adapted to 8-bit, 16-bit, or 32-bit machines,
	lowbyte-highbyte order or highbyte-lowbyte order.  This version
	has been converted to ANSI C.

	Modified 8 Apr 92 - HAJK - Implement new VAX/VMS primitive support.
	Modified 29 Nov 92 - Thad Smith
*/

#include <string.h>
#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */
#include "platform.h" /* customization for different environments */

/* Platform customization:
 * A version which runs on almost any computer can be implemented by
 * defining PORTABLE and MPORTABLE, preferably as a command line
 * parameter.  Faster versions can be generated by specifying specific
 * parameters, such as size of unit and MULTUNIT, and by supplying some
 * of the critical in assembly.  See the file platform.h for more 
 * details on customization.
 *
 * The symbol HIGHFIRST, designating that integers and longs are stored
 * with the most significant bit in the lowest address, should be defined
 * on the command line for compiling all files, since it is used by files
 * other than the mpilib routines.
 */
 
#ifndef ALIGN
#define ALIGN
#endif

#ifndef PEASANT /* if not Russian peasant modulo multiply algorithm */
#ifndef MERRITT /* if not Merritt's modmult */
#ifndef UPTON   /* if not Upton's modmult */
#ifndef SMITH
#define SMITH   /* default: use Smith's modmult algorithm */
#endif
#endif
#endif
#endif

#ifdef SMITH
#define UPTON_OR_SMITH  /* enable common code */
#endif
#ifdef UPTON
#define UPTON_OR_SMITH  /* enable common code */
#endif

#ifndef UNIT32
#ifndef UNIT8
#define UNIT16			/* default--use 16-bit units */
#endif
#endif

/***	CAUTION:  If your machine has an unusual word size that is not a
	power of 2 (8, 16, 32, or 64) bits wide, then the macros here that
	use the symbol "LOG_UNITSIZE" must be changed.
***/

#ifdef UNIT8
typedef unsigned char unit;
typedef signed char signedunit;
#define UNITSIZE 8 /* number of bits in a unit */
#define LOG_UNITSIZE 3
#define uppermostbit ((unit) 0x80)
#define BYTES_PER_UNIT 1 /* number of bytes in a unit */
#define units2bits(n) ((n) << 3) /* fast multiply by UNITSIZE */
#define units2bytes(n) (n)
#define bits2units(n) (((n)+7) >> 3)
#define bytes2units(n) (n)
#endif

#ifdef UNIT16
typedef word16 unit;
typedef short signedunit;
#define UNITSIZE 16 /* number of bits in a unit */
#define LOG_UNITSIZE 4
#define uppermostbit ((unit) 0x8000)
#define BYTES_PER_UNIT 2 /* number of bytes in a unit */
#define units2bits(n) ((n) << 4) /* fast multiply by UNITSIZE */
#define units2bytes(n) ((n) << 1)
#define bits2units(n) (((n)+15) >> 4)
#define bytes2units(n) (((n)+1) >> 1)
#endif

#ifdef UNIT32
typedef word32 unit;
typedef long signedunit;
#define UNITSIZE 32 /* number of bits in a unit */
#define LOG_UNITSIZE 5
#define uppermostbit ((unit) 0x80000000L)
#define BYTES_PER_UNIT 4 /* number of bytes in a unit */
#define units2bits(n) ((n) << 5) /* fast multiply by UNITSIZE */
#define units2bytes(n) ((n) << 2)
#define bits2units(n) (((n)+31) >> 5)
#define bytes2units(n) (((n)+3) >> 2)
#endif

#define power_of_2(b) ((unit) 1 << (b)) /* computes power-of-2 bit masks */
#define bits2bytes(n) (((n)+7) >> 3)
/*	Some C compilers (like the ADSP2101) will not always collapse constant 
	expressions at compile time if the expressions contain shift operators. */
/* #define uppermostbit power_of_2(UNITSIZE-1) */
/* #define UNITSIZE units2bits(1) */ /* number of bits in a unit */
/* #define bytes2units(n) bits2units((n)<<3) */
/* #define BYTES_PER_UNIT (UNITSIZE >> 3) */
/* LOG_UNITSIZE is the log base 2 of UNITSIZE, ie: 4 for 16-bit units */
/* #define units2bits(n) ((n) << LOG_UNITSIZE) */ /* fast multiply by UNITSIZE */
/* #define units2bytes(n) ((n) << (LOG_UNITSIZE-3)) */
/* #define bits2units(n) (((n)+(UNITSIZE-1)) >> LOG_UNITSIZE) */
/* #define bytes2units(n) (((n)+(BYTES_PER_UNIT-1)) >> (LOG_UNITSIZE-3)) */

typedef unit *unitptr;


/*--------------------- Byte ordering stuff -------------------*/
#ifdef HIGHFIRST

/* these definitions assume MSB comes first */
#define tohigher(n)		(-(n))  /* offset towards higher unit */
#define pre_higherunit(r)	(--(r))
#define pre_lowerunit(r)	(++(r))
#define post_higherunit(r)	((r)--)
#define post_lowerunit(r)	((r)++)
#define bit_index(n)		(global_precision-bits2units((n)+1))
#define lsbptr(r,prec)		((r)+(prec)-1)
#define make_lsbptr(r,prec)	(r) = lsbptr(r,prec)  
#define unmake_lsbptr(r,prec) (r) = ((r)-(prec)+1) 
#define msbptr(r,prec)		(r)
#define make_msbptr(r,prec)	/* (r) = msbptr(r,prec) */

/* The macro rescale(r,current_precision,new_precision) rescales
   a multiprecision integer by adjusting r and its precision to new values.
   It can be used to reverse the effects of the normalize
   routine given above.  See the comments in normalize concerning
   Intel vs. Motorola LSB/MSB conventions.
   WARNING:  You can only safely call rescale on registers that
   you have previously normalized with the above normalize routine,
   or are known to be big enough for the new precision.  You may
   specify a new precision that is smaller than the current precision.
   You must be careful not to specify a new_precision value that is
   too big, or which adjusts the r pointer out of range.
*/
#define rescale(r,currentp,newp) r -= ((newp) - (currentp))

/* The macro normalize(r,precision) "normalizes" a multiprecision integer
   by adjusting r and precision to new values.  For Motorola-style processors
   (MSB-first), r is a pointer to the MSB of the register, and must
   be adjusted to point to the first nonzero unit.  For Intel/VAX-style
   (LSB-first) processors, r is a  pointer to the LSB of the register,
   and must be left unchanged.  The precision counter is always adjusted,
   regardless of processor type.  In the case of precision = 0,
   r becomes undefined.
*/
#define normalize(r,prec) \
  { prec = significance(r); r += (global_precision-(prec)); }

#else	/* LOWFIRST byte order */

/* these definitions assume LSB comes first */
#define tohigher(n)		(n)     /* offset towards higher unit */
#define pre_higherunit(r)	(++(r))
#define pre_lowerunit(r)	(--(r))
#define post_higherunit(r)	((r)++)
#define post_lowerunit(r)	((r)--)
#define bit_index(n)		(bits2units((n)+1)-1)
#define lsbptr(r,prec)		(r)
#define make_lsbptr(r,prec)	/* (r) = lsbptr(r,prec) */  
#define unmake_lsbptr(r,prec) /* (r) = (r) */  
#define msbptr(r,prec)		((r)+(prec)-1)
#define make_msbptr(r,prec)	(r) = msbptr(r,prec)

#define rescale(r,currentp,newp) /* nil statement */
#define normalize(r,prec) prec = significance(r) 

#endif	/* LOWFIRST byte order */
/*------------------ End byte ordering stuff -------------------*/

/*	Note that the address calculations require that lsbptr, msbptr, 
	make_lsbptr, make_msbptr, mp_tstbit, mp_setbit, mp_clrbit, 
	and bitptr all have unitptr arguments, not byte pointer arguments.	*/
#define bitptr(r,n) &((r)[bit_index(n)])
#define bitmsk(n) power_of_2((n) & (UNITSIZE-1))
	/* bitmsk() assumes UNITSIZE is a power of 2 */
#define mp_tstbit(r,n) (*bitptr(r,n) &   bitmsk(n))
#define mp_setbit(r,n) (*bitptr(r,n) |=  bitmsk(n))
#define mp_clrbit(r,n) (*bitptr(r,n) &= ~bitmsk(n))
#define msunit(r) (*msbptr(r,global_precision))
#define lsunit(r) (*lsbptr(r,global_precision))
/* #define mp_tstminus(r) ((msunit(r) & uppermostbit)!=0) */
#define mp_tstminus(r) ((signedunit) msunit(r) < 0)


	/* set working precision to specified number of bits. */
#ifdef mp_setp
void mp_setp(short nbits);
#define set_precision(prec)	mp_setp(units2bits(global_precision=(prec)))
#else
#define set_precision(prec) (global_precision = (prec))
#endif


#ifdef PEASANT

/* Define C names for Russian peasant modmult primitives. */
#define stage_modulus	stage_peasant_modulus
#define mp_modmult	peasant_modmult
#define modmult_burn	peasant_burn
#define SLOP_BITS	PEASANT_SLOP_BITS

#else  /* not PEASANT */
#ifdef MERRITT
/* Define C names for Merritt's modmult primitives. */
#define stage_modulus	stage_merritt_modulus
#define mp_modmult	merritt_modmult
#define modmult_burn	merritt_burn
#define SLOP_BITS	MERRITT_SLOP_BITS

#else	/* not PEASANT, MERRITT */
#ifdef UPTON
/* Define C names for Upton's modmult primitives. */
#define stage_modulus	stage_upton_modulus
#define mp_modmult	upton_modmult
#define modmult_burn	upton_burn
#define SLOP_BITS	UPTON_SLOP_BITS

#else	/* not PEASANT, MERRITT, UPTON */
#ifdef SMITH
/* Define C names for Smith's modmult primitives. */
#define stage_modulus	stage_smith_modulus
#define mp_modmult	smith_modmult
#define modmult_burn	smith_burn
#define SLOP_BITS	SMITH_SLOP_BITS

#endif	/* SMITH */
#endif	/* UPTON */
#endif	/* MERRITT */
#endif	/* PEASANT */


#define mp_shift_left(r1) mp_rotate_left(r1,(boolean)0)
	/* multiprecision shift left 1 bit */

#define mp_add(r1,r2) mp_addc(r1,r2,(boolean)0)
	/* multiprecision add with no carry */

#define mp_sub(r1,r2) mp_subb(r1,r2,(boolean)0)
	/* multiprecision subtract with no borrow */

#define mp_abs(r) (mp_tstminus(r) ? (mp_neg(r),TRUE) : FALSE)

#define msub(r,m) if (mp_compare(r,m) >= 0) mp_sub(r,m)
	/* Prevents r from getting bigger than modulus m */

#define testeq(r,i)	\
	( (lsunit(r)==(i)) && (significance(r)<=1) )

#define testne(r,i)	\
	( (lsunit(r)!=(i)) || (significance(r)>1) )

#define testge(r,i)	\
	( (lsunit(r)>=(i)) || (significance(r)>1) )

#define testle(r,i)	\
	( (lsunit(r)<=(i)) && (significance(r)<=1) )

#define mp_square(r1,r2) mp_mult(r1,r2,r2)
	/* Square r2, returning product in r1 */

#define mp_modsquare(r1,r2) mp_modmult(r1,r2,r2)
	/* Square r2, returning modulo'ed product in r1 */

#define countbytes(r) ((countbits(r)+7)>>3)

/*	SLOP_BITS is how many "carry bits" to allow for intermediate
	calculation results to exceed the size of the modulus.
	It is used by modexp to give some overflow elbow room for
	modmult to use to perform modulo operations with the modulus.
	The number of slop bits required is determined by the modmult
	algorithm.  The Russian peasant modmult algorithm only requires
	1 slop bit, for example.  Note that if we use an external assembly
	modmult routine, SLOP_BITS may be meaningless or may be defined in a
	non-constant manner.
*/
#define PEASANT_SLOP_BITS	1
#define MERRITT_SLOP_BITS	UNITSIZE
#define UPTON_SLOP_BITS	(UNITSIZE/2)
#ifdef  mp_smul /* old version requires MS word = 0 */
#define SMITH_SLOP_BITS	UNITSIZE
#else           /* mp_smula or C version of mp_smul */
#define SMITH_SLOP_BITS	0
#endif /* mp_smul */

/*	MAX_BIT_PRECISION is upper limit that assembly primitives can handle.
	It must be less than 32704 bits, or 4088 bytes.  It should be an
	integer multiple of UNITSIZE*2.
*/
#if (SLOP_BITS > 0)
#define MAX_BIT_PRECISION (1280+(2*UNITSIZE))
#else
#define MAX_BIT_PRECISION 1280
#endif
#define MAX_BYTE_PRECISION (MAX_BIT_PRECISION/8)
#define MAX_UNIT_PRECISION (MAX_BIT_PRECISION/UNITSIZE)


/* global_precision is the unit precision last set by set_precision */
extern short global_precision;


/*	The "bit sniffer" macros all begin sniffing at the MSB.
	They are used internally by all the various multiply, divide, 
	modulo, exponentiation, and square root functions.
*/
#define sniff_bit(bptr,bitmask)	(*(bptr) & bitmask)

#define init_bitsniffer(bptr,bitmask,prec,bits) \
{ normalize(bptr,prec); \
  if (!prec) \
    return(0); \
  bits = units2bits(prec); \
  make_msbptr(bptr,prec); bitmask = uppermostbit; \
  while (!sniff_bit(bptr,bitmask)) \
  { bitmask >>= 1; bits--; \
  } \
}

#define bump_bitsniffer(bptr,bitmask) \
{ if (!(bitmask >>= 1)) \
  { bitmask = uppermostbit; \
	post_lowerunit(bptr); \
  } \
}

/* bump_2bitsniffers is used internally by mp_udiv. */
#define bump_2bitsniffers(bptr,bptr2,bitmask) \
{ if (!(bitmask >>= 1)) \
  { bitmask = uppermostbit; \
    post_lowerunit(bptr); \
    post_lowerunit(bptr2); \
  } \
}

/* stuff_bit is used internally by mp_udiv and mp_sqrt. */
#define stuff_bit(bptr,bitmask)	*(bptr) |= bitmask


boolean mp_addc
	(register unitptr r1,register unitptr r2,register boolean carry);
	/* multiprecision add with carry r2 to r1, result in r1 */

boolean mp_subb
	(register unitptr r1,register unitptr r2,register boolean borrow);
	/* multiprecision subtract with borrow, r2 from r1, result in r1 */

boolean mp_rotate_left(register unitptr r1,register boolean carry);
	/* multiprecision rotate left 1 bit with carry, result in r1. */

void mp_shift_right_bits(register unitptr r1,register short bits);
	/* multiprecision shift right bits, result in r1. */

short mp_compare(register unitptr r1,register unitptr r2);
	/* Compares registers *r1, *r2, and returns -1, 0, or 1 */

boolean mp_inc(register unitptr r);
	/* Increment multiprecision integer r. */

boolean mp_dec(register unitptr r);
	/* Decrement multiprecision integer r. */

void mp_neg(register unitptr r);
	/* Compute 2's complement, the arithmetic negative, of r */

#ifndef mp_move
#define mp_move(d,s)    memcpy((void*)(d), (void*)(s), \
		units2bytes(global_precision))
#endif  
#ifndef unitfill0
#define unitfill0(r,ct) memset((void*)(r), 0, units2bytes(ct))
#endif

#ifndef mp_burn
#define mp_burn(r) mp_init(r,0) /* for burning the evidence */
#define mp_init0(r) mp_init(r,0)
#endif

#define empty_array(r)  unitfill0(r, sizeof(r)/sizeof(r[0])/sizeof(unit))

void mp_init(register unitptr r, word16 value);
	/* Init multiprecision register r with short value. */

short significance(register unitptr r);
	/* Returns number of significant units in r */

int mp_udiv(register unitptr remainder,register unitptr quotient,
	register unitptr dividend,register unitptr divisor);
	/* Unsigned divide, treats both operands as positive. */

int mp_recip(register unitptr quotient,register unitptr divisor);
	/* Compute reciprocal as 1/divisor.  Used by faster modmult. */

int mp_div(register unitptr remainder,register unitptr quotient,
	register unitptr dividend,register unitptr divisor);
	/* Signed divide, either or both operands may be negative. */

word16 mp_shortdiv(register unitptr quotient,
	register unitptr dividend,register word16 divisor);
	/* Returns short remainder of unsigned divide. */

int mp_mod(register unitptr remainder,
	register unitptr dividend,register unitptr divisor);
	/* Unsigned divide, treats both operands as positive. */

word16 mp_shortmod(register unitptr dividend,register word16 divisor);
	/* Just returns short remainder of unsigned divide. */

int mp_mult(register unitptr prod,
	register unitptr multiplicand,register unitptr multiplier);
	/* Computes multiprecision prod = multiplicand * multiplier */

int countbits(unitptr r);
	/* Returns number of significant bits in r. */

int stage_peasant_modulus(unitptr n);
int stage_merritt_modulus(unitptr n);
int stage_upton_modulus(unitptr n);
int stage_smith_modulus(unitptr n);
	/* Must pass modulus to stage_modulus before calling modmult. */

int peasant_modmult(register unitptr prod,
	unitptr multiplicand,register unitptr multiplier);
int merritt_modmult(register unitptr prod,
	unitptr multiplicand,register unitptr multiplier);
int upton_modmult(register unitptr prod,
	unitptr multiplicand,register unitptr multiplier);
int smith_modmult(register unitptr prod,
	unitptr multiplicand,register unitptr multiplier);
	/* Performs combined multiply/modulo operation, with global modulus */
 
 

int mp_modexp(register unitptr expout,register unitptr expin,
	register unitptr exponent,register unitptr modulus);
	/* Combined exponentiation/modulo algorithm. */

int mp_modexp_crt(unitptr expout, unitptr expin,
	unitptr p, unitptr q, unitptr ep, unitptr eq, unitptr u);
	/* exponentiation and modulo using Chinese Remainder Theorem */

/****************** end of MPI library ****************************/