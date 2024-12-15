/*
 *	des - fast & portable DES encryption & decryption.
 *	Copyright (C) 1992  Dana L. How
 *	Please see the file `README' for the complete copyright notice.
 */

#ifndef	lint
static char desCode_hRcs[] = "$Id: desCode.h,v 1.21 1992/05/20 00:00:57 how E $";
#endif

#include	"desCore.h"
extern word des_keymap[], des_bigmap[];


/* optional customization:
 * the idea here is to alter the code so it will still run correctly
 * on any machine,  but the quickest on the specific machine in mind.
 * note that these silly tweaks can give you a 15%-20% speed improvement
 * on the sparc -- it's probably even more significant on the 68000. */

/* take care of machines with incredibly few registers */
#if	defined(i386)
#define	REGISTER		/* only x, y, z will be declared register */
#else
#define	REGISTER	register
#endif	/* i386 */

/* is auto inc/dec faster than 7bit unsigned indexing? */
#if	defined(vax) || defined(mc68000)
#define	FIXR		r += 32;
#define	FIXS		s +=  8;
#define	PREV(v,o)	*--v
#define	NEXT(v,o)	*v++
#define	BUMP(v,n)
#else
#define	FIXR
#define	FIXS
#define	PREV(v,o)	v[o]
#define	NEXT(v,o)	v[o]
#define	BUMP(v,n)	v += n;
#endif

/* if no machine type, default is indexing, 6 registers and cheap literals */
#if	!defined(i386) && !defined(vax) && !defined(mc68000) && !defined(sparc)
#define	vax
#endif

/* handle a compiler which can't reallocate registers */
#if	defined(strange)			/* didn't feel like deleting */
#define	SREGFREE	; s = D
#define	DEST		s
#define	D		m0
#else
#define	SREGFREE
#define	DEST		d
#define	D		d
#endif

/* explanation of configuration macros:
 *	REG.....	declares any extra registers needed.
 *	SET.....	sets those registers which should hold constants.
 *	.F		fiddle with word before xor'ing with key schedule.
 *	.Ln		do table lookup number n.
 *	.An		accumulate table lookups starting at n.
 * where the dots are replaced by one of Q(UICK) or S(MALL).
 */

/* handle constants in the optimal way for vax */
/* we use 6 variables, all declared register;
 * we assume address literals are cheap & unrestricted;
 * we assume immediate constants are cheap & unrestricted.
 *
 * note that the low 2 bits in each bit mask can be turned off,
 * permitting the removal of the pesky -2 and -3 offsets and subsequent
 * identification of the address literals as CSEs.  however, we did _assume_
 * that address literals were cheap, and this would prevent the translation
 * of (value >> bits) & mask into a single CISC bitfield instruction. */
#if	defined(vax)
#define	REGQUICK
#define	SETQUICK
#define	QF(zs)		(zs & 0XFCFCFCFCL)
#define	QL0(z)		ADD((byte *)des_bigmap            , z       &  0XFFFF)
#define	QL1(z)		ADD((byte *)des_bigmap         - 2, z >> 16          )
#define	QL2(z)		ADD((byte *)des_bigmap - 0x100    , z       &  0XFFFF)
#define	QL3(z)		ADD((byte *)des_bigmap - 0x100 - 2, z >> 16          )
#define	QA0(x,A,B)	x ^= A; x ^= B
#define	QA2(x,A,B)	x ^= A; x ^= B
#define	REGSMALL
#define	SETSMALL
#define	SF(zs)		(zs & 0XFCFCFCFCL)
#define	SL0(z)		ADD((byte *)des_keymap         - 0,  z        & 0X3FF)
#define	SL1(z)		ADD((byte *)des_keymap         - 3, (z >>  8) & 0X3FF)
#define	SL2(z)		ADD((byte *)des_keymap         - 2, (z >> 16) & 0X3FF)
#define	SL3(z)		ADD((byte *)des_keymap         - 1,  z >> 24         )
#define	SL4(z)		ADD((byte *)des_keymap + 0x400 - 0,  z        & 0X3FF)
#define	SL5(z)		ADD((byte *)des_keymap + 0x400 - 3, (z >>  8) & 0X3FF)
#define	SL6(z)		ADD((byte *)des_keymap + 0x400 - 2, (z >> 16) & 0X3FF)
#define	SL7(z)		ADD((byte *)des_keymap + 0x400 - 1,  z >> 24         )
#define	SA0(x,A,B,C,D)	x ^= A; x ^= B; x ^= C; x ^= D
#define	SA4(x,A,B,C,D)	x ^= A; x ^= B; x ^= C; x ^= D
#define	UNROLL		1
#endif	/* defined(vax) */

/* handle constants in the optimal way for 386 */
/* we declare 3 register variables (see above) and use 3 more variables;
 * we assume address literals are cheap & unrestricted;
 * we assume immediate constants are cheap & unrestricted.
 * so that movzx is used we do NOT turn off the low 2 bits in each bit mask. */
#if	defined(i386)
#define	REGQUICK
#define	SETQUICK
#define	QF(zs)		(zs & 0XFCFCFCFCL)
#define	QL0(z)		ADD((byte *)des_bigmap            , z       &  0XFFFF)
#define	QL1(z)		ADD((byte *)des_bigmap         - 2, z >> 16          )
#define	QL2(z)		ADD((byte *)des_bigmap - 0x100    , z       &  0XFFFF)
#define	QL3(z)		ADD((byte *)des_bigmap - 0x100 - 2, z >> 16          )
#define	QA0(x,A,B)	x ^= A; x ^= B
#define	QA2(x,A,B)	x ^= A; x ^= B
#define	REGSMALL
#define	SETSMALL
#define	SF(zs)		(zs & 0XFCFCFCFCL)
#define	SL0(z)		ADD((byte *)des_keymap + 0x300 - 0,  z        & 0X0FF)
#define	SL1(z)		ADD((byte *)des_keymap + 0x200 - 3, (z >>= 8) & 0X0FF)
#define	SL2(z)		ADD((byte *)des_keymap + 0x100 - 2, (z >>= 8) & 0X0FF)
#define	SL3(z)		ADD((byte *)des_keymap         - 1,  z >>= 8         )
#define	SL4(z)		ADD((byte *)des_keymap + 0x700 - 0,  z        & 0X0FF)
#define	SL5(z)		ADD((byte *)des_keymap + 0x600 - 3, (z >>= 8) & 0X0FF)
#define	SL6(z)		ADD((byte *)des_keymap + 0x500 - 2, (z >>= 8) & 0X0FF)
#define	SL7(z)		ADD((byte *)des_keymap + 0x400 - 1,  z >>= 8         )
#define	SA0(x,A,B,C,D)	x ^= A; x ^= B; x ^= C; x ^= D
#define	SA4(x,A,B,C,D)	x ^= A; x ^= B; x ^= C; x ^= D
#define	UNROLL		1
#endif	/* defined(i386) */

/* handle constants in the optimal way for mc68000 */
/* in addition to the core 6 variables, we declare 3 registers holding constants
 * and 3 registers holding address literals.  `a' accumulates lookups.
 * at most 6 data values and 4 address values are actively used at once.
 * we assume address literals are so expensive we never use them;
 * we assume constant index offsets > 127 are expensive, so they are not used.
 * we assume all constants are expensive and put them in registers. */
#if	defined(mc68000)
#define	REGQUICK				\
	register word a;			\
	register word *e;			\
	register word k0, k1;			\
	register byte *m0, *m1;
#define	SETQUICK				\
	; k0 = 0XFFFF				\
	; k1 = 0XFCFCFCFCL			\
	; m0 = (byte *)des_bigmap		\
	; m1 = m0 - 0x100
#define	QF(zs)		(zs & k1)
#define	QL0(z)		ADD(m0    ,  z        & k0)
#define	QL1(z)		ADD(m0 - 2,  z >> 16      )
#define	QL2(z)		ADD(m1    ,  z        & k0)
#define	QL3(z)		ADD(m1 - 2,  z >> 16      )
#define	QA0(x,A,B)	a  = A; a += B
#define	QA2(x,A,B)	a += A; a += B; x ^= a
#define	REGSMALL				\
	register word a;			\
	register word *e;			\
	register word k0, k1;			\
	register byte *m0, *m1;
#define	SETSMALL				\
	; k0 = 0X3FC				\
	; k1 = 0XFCFCFCFCL			\
	; m0 = (byte *)des_keymap		\
	; m1 = m0 + 0x400
#define	SF(zs)		(zs & k1)
#define	SL0(z)		ADD(m0    ,  z        & k0)
#define	SL1(z)		ADD(m0    , (z >>= 8) & k0)
#define	SL2(z)		ADD(m0    , (z >>= 8) & k0)
#define	SL3(z)		ADD(m0 - 1,  z >>= 8      )
#define	SL4(z)		ADD(m1    ,  z        & k0)
#define	SL5(z)		ADD(m1    , (z >>= 8) & k0)
#define	SL6(z)		ADD(m1    , (z >>= 8) & k0)
#define	SL7(z)		ADD(m1 - 1,  z >>= 8      )
#define	SA0(x,A,B,C,D)	a  = A; a += B; a += C; a += D
#define	SA4(x,A,B,C,D)	a += A; a += B; a += C; a += D; x ^= a
#define	UNROLL		0
#endif	/* defined(mc68000) */

/* handle constants in the optimal way for sparc */
/* in addition to the core 6 variables, we either declare:
 * 4 registers holding address literals and 1 register holding a constant, or
 * 8 registers holding address literals.
 * up to 14 register variables are declared (sparc has %i0-%i5, %l0-%l7).
 * we assume address literals are so expensive we never use them;
 * we assume any constant with >10 bits is expensive and put it in a register,
 * and any other is cheap and is coded in-line. */
#if	defined(sparc)
#define	REGQUICK				\
	register word k0;			\
	register byte *m0, *m1, *m2, *m3;
#define	SETQUICK				\
	; k0 = 0XFCFC				\
	; m0 = (byte *)des_bigmap		\
	; m1 = m0 + 0x100			\
	; m2 = m1 + 0x100			\
	; m3 = m2 + 0x100
#define	QF(zs)		zs
#define	QL0(z)		ADD(m3,  z        & k0)
#define	QL1(z)		ADD(m1, (z >> 16) & k0)
#define	QL2(z)		ADD(m2,  z        & k0)
#define	QL3(z)		ADD(m0, (z >> 16) & k0)
#define	QA0(x,A,B)	x ^= A + B
#define	QA2(x,A,B)	x ^= A + B
#define	REGSMALL				\
	register byte *m0, *m1, *m2, *m3, *m4, *m5, *m6, *m7;
#define	SETSMALL				\
	; m0 = (byte *)des_keymap		\
	; m1 = m0 + 0x100			\
	; m2 = m1 + 0x100			\
	; m3 = m2 + 0x100			\
	; m4 = m3 + 0x100			\
	; m5 = m4 + 0x100			\
	; m6 = m5 + 0x100			\
	; m7 = m6 + 0x100
#define	SF(zs)		zs
#define	SL0(z)		ADD(m3,  z        & 0XFC)
#define	SL1(z)		ADD(m2, (z >>  8) & 0XFC)
#define	SL2(z)		ADD(m1, (z >> 16) & 0XFC)
#define	SL3(z)		ADD(m0, (z >> 24) & 0XFC)
#define	SL4(z)		ADD(m7,  z        & 0XFC)
#define	SL5(z)		ADD(m6, (z >>  8) & 0XFC)
#define	SL6(z)		ADD(m5, (z >> 16) & 0XFC)
#define	SL7(z)		ADD(m4, (z >> 24) & 0XFC)
#define	SA0(x,A,B,C,D)	x ^= A + B + C + D
#define	SA4(x,A,B,C,D)	x ^= A + B + C + D
#define	UNROLL		1
#endif	/* defined(sparc) */


/* some basic stuff */

/* generate addresses from a base and an index */
#define	ADD(b,x)	(word *) (b + (x))

/* low level rotate operations */
#define	NOP(d,s,c,o)
#define	ROL(d,s,c,o)	d = s << c | s >> o
#define	ROR(d,s,c,o)	d = s >> c | s << o
#define	ROL1(d)		ROL(d, d, 1, 31)
#define	ROR1(d)		ROR(d, d, 1, 31)

/* elementary swap for doing IP/FP */
#define	SWAP(x,y,m,b)				\
	z  = ((x >> b) ^ y) & m;		\
	x ^= z << b;				\
	y ^= z


/* the following macros contain all the important code fragments */

/* load input data, then setup special registers holding constants */
#define	TEMPQUICK(LOAD)				\
	REGQUICK				\
	LOAD()					\
	SETQUICK
#define	TEMPSMALL(LOAD)				\
	REGSMALL				\
	LOAD()					\
	SETSMALL

/* load data */
#define	LOADDATA(x,y)				\
	FIXS					\
	y  = PREV(s, 7); y<<= 8;		\
	y |= PREV(s, 6); y<<= 8;		\
	y |= PREV(s, 5); y<<= 8;		\
	y |= PREV(s, 4);			\
	x  = PREV(s, 3); x<<= 8;		\
	x |= PREV(s, 2); x<<= 8;		\
	x |= PREV(s, 1); x<<= 8;		\
	x |= PREV(s, 0)				\
	SREGFREE
/* load data without initial permutation and put into efficient position */
#define	LOADCORE()				\
	LOADDATA(x, y);				\
	ROR1(x);				\
	ROR1(y)
/* load data, do the initial permutation and put into efficient position */
#define	LOADFIPS()				\
	LOADDATA(y, x);				\
	SWAP(x, y, 0X0F0F0F0FL, 004);		\
	SWAP(y, x, 0X0000FFFFL, 020);		\
	SWAP(x, y, 0X33333333L, 002);		\
	SWAP(y, x, 0X00FF00FFL, 010);		\
	ROR1(x);				\
	z  = (x ^ y) & 0X55555555L;		\
	y ^= z;					\
	x ^= z;					\
	ROR1(y)


/* core encryption/decryption operations */
/* S box mapping and P perm with 64k of tables */
#define	KEYMAPQUICK(x,z,zs,r,m,LOAD,qf,ql0,ql1,qa,sf,sl0,sl1,sl2,sl3,sa) \
	z  = qf(zs) ^ LOAD(r, m);		\
	qa(x, *ql0(z), *ql1(z))
/* small version: use 2k of tables */
#define	KEYMAPSMALL(x,z,zs,r,m,LOAD,qf,ql0,ql1,qa,sf,sl0,sl1,sl2,sl3,sa) \
	z  = sf(zs) ^ LOAD(r, m);		\
	sa(x, *sl0(z), *sl1(z), *sl2(z), *sl3(z))
/* apply 24 key bits and do the odd  s boxes */
#define	S7S1(x,y,z,r,m,KEYMAP,LOAD,qa,sa)	\
	KEYMAP(x,z,y,r,m,LOAD,QF,QL0,QL1,qa,SF,SL0,SL1,SL2,SL3,sa)
/* apply 24 key bits and do the even s boxes */
#define	S6S0(x,y,z,r,m,KEYMAP,LOAD,qa,sa)	\
	ROL(z, y, 4, 28);			\
	KEYMAP(x,z,z,r,m,LOAD,QF,QL2,QL3,qa,SF,SL4,SL5,SL6,SL7,sa)
/* an actual iteration.  equivalent except for UPDATE & swapping m and n */
#define	ENCR(x,y,z,r,m,n,KEYMAP)		\
	S7S1(x,y,z,r,m,KEYMAP,NEXT,QA0,SA0);	\
	S6S0(x,y,z,r,n,KEYMAP,NEXT,QA2,SA4)
#define	DECR(x,y,z,r,m,n,KEYMAP)		\
	S6S0(x,y,z,r,m,KEYMAP,PREV,QA0,SA0);	\
	S7S1(x,y,z,r,n,KEYMAP,PREV,QA2,SA4)

/* write out result in correct byte order */
#define	SAVEDATA(x,y)				\
	NEXT(DEST, 0) = x; x>>= 8;		\
	NEXT(DEST, 1) = x; x>>= 8;		\
	NEXT(DEST, 2) = x; x>>= 8;		\
	NEXT(DEST, 3) = x;			\
	NEXT(DEST, 4) = y; y>>= 8;		\
	NEXT(DEST, 5) = y; y>>= 8;		\
	NEXT(DEST, 6) = y; y>>= 8;		\
	NEXT(DEST, 7) = y
/* write out result */
#define	SAVECORE()				\
	ROL1(x);				\
	ROL1(y);				\
	SAVEDATA(y, x)
/* do final permutation and write out result */
#define	SAVEFIPS()				\
	ROL1(x);				\
	z  = (x ^ y) & 0X55555555L;		\
	y ^= z;					\
	x ^= z;					\
	ROL1(y);				\
	SWAP(x, y, 0X00FF00FFL, 010);		\
	SWAP(y, x, 0X33333333L, 002);		\
	SWAP(x, y, 0X0000FFFFL, 020);		\
	SWAP(y, x, 0X0F0F0F0FL, 004);		\
	SAVEDATA(x, y)


/* rolled or unrolled iterations */
#if	UNROLL
#define	ENCODE(x,y,z,r,e,KEYMAP)		\
	ENCR(x,y,z,r, 0, 1,KEYMAP);		\
	ENCR(y,x,z,r, 2, 3,KEYMAP);		\
	ENCR(x,y,z,r, 4, 5,KEYMAP);		\
	ENCR(y,x,z,r, 6, 7,KEYMAP);		\
	ENCR(x,y,z,r, 8, 9,KEYMAP);		\
	ENCR(y,x,z,r,10,11,KEYMAP);		\
	ENCR(x,y,z,r,12,13,KEYMAP);		\
	ENCR(y,x,z,r,14,15,KEYMAP);		\
	ENCR(x,y,z,r,16,17,KEYMAP);		\
	ENCR(y,x,z,r,18,19,KEYMAP);		\
	ENCR(x,y,z,r,20,21,KEYMAP);		\
	ENCR(y,x,z,r,22,23,KEYMAP);		\
	ENCR(x,y,z,r,24,25,KEYMAP);		\
	ENCR(y,x,z,r,26,27,KEYMAP);		\
	ENCR(x,y,z,r,28,29,KEYMAP);		\
	ENCR(y,x,z,r,30,31,KEYMAP)
#define	DECODE(x,y,z,r,e,KEYMAP)		\
	FIXR					\
	DECR(x,y,z,r,31,30,KEYMAP);		\
	DECR(y,x,z,r,29,28,KEYMAP);		\
	DECR(x,y,z,r,27,26,KEYMAP);		\
	DECR(y,x,z,r,25,24,KEYMAP);		\
	DECR(x,y,z,r,23,22,KEYMAP);		\
	DECR(y,x,z,r,21,20,KEYMAP);		\
	DECR(x,y,z,r,19,18,KEYMAP);		\
	DECR(y,x,z,r,17,16,KEYMAP);		\
	DECR(x,y,z,r,15,14,KEYMAP);		\
	DECR(y,x,z,r,13,12,KEYMAP);		\
	DECR(x,y,z,r,11,10,KEYMAP);		\
	DECR(y,x,z,r, 9, 8,KEYMAP);		\
	DECR(x,y,z,r, 7, 6,KEYMAP);		\
	DECR(y,x,z,r, 5, 4,KEYMAP);		\
	DECR(x,y,z,r, 3, 2,KEYMAP);		\
	DECR(y,x,z,r, 1, 0,KEYMAP)
#else
#define	ENCODE(x,y,z,r,e,KEYMAP)		\
	e = r + 32;				\
	do {					\
		ENCR(x,y,z,r, 0, 1,KEYMAP);	\
		ENCR(y,x,z,r, 2, 3,KEYMAP);	\
		BUMP(r,  4)			\
	} while ( r < e )
#define	DECODE(x,y,z,r,e,KEYMAP)		\
	e = r;					\
	r += 32;				\
	do {					\
		DECR(x,y,z,r,-1,-2,KEYMAP);	\
		DECR(y,x,z,r,-3,-4,KEYMAP);	\
		BUMP(r, -4)			\
	} while ( r > e )
#endif


/* the following macros contain the encryption/decryption skeletons */

#define	ENCRYPT(NAME, TEMP, LOAD, KEYMAP, SAVE)	\
						\
void						\
NAME(D, r, s)					\
REGISTER byte * D;				\
REGISTER word * r;				\
REGISTER byte * s;				\
{						\
	register word x, y, z;			\
						\
	/* declare temps & load data */		\
	TEMP(LOAD);				\
						\
	/* do the 16 iterations */		\
	ENCODE(x,y,z,r,e,KEYMAP);		\
						\
	/* save result */			\
	SAVE();					\
						\
	return;					\
}

#define	DECRYPT(NAME, TEMP, LOAD, KEYMAP, SAVE)	\
						\
void						\
NAME(D, r, s)					\
REGISTER byte * D;				\
REGISTER word * r;				\
REGISTER byte * s;				\
{						\
	register word x, y, z;			\
						\
	/* declare temps & load data */		\
	TEMP(LOAD);				\
						\
	/* do the 16 iterations */		\
	DECODE(x,y,z,r,e,KEYMAP);		\
						\
	/* save result */			\
	SAVE();					\
						\
	return;					\
}
