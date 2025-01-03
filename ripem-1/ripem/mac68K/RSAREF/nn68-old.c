/* NN.C - natural numbers routines
 */

/* Copyright (C) 1991-2 RSA Laboratories, a division of RSA Data
   Security, Inc. All rights reserved.
   
   921017 rwo : recode all low-level routinews into 680[234]0 asm
   		for use with Symantec THINK C V5.02.  Eliminated digit.c.
 */

#include "global.h"
#include "rsaref.h"
#include "nn.h"

static NN_DIGIT NN_LShift PROTO_LIST 
  ((NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int));
static NN_DIGIT NN_RShift PROTO_LIST
  ((NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int));
static void NN_Div PROTO_LIST
  ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *,
    unsigned int));
    
static void NN_DigitDiv PROTO_LIST
  ((NN_DIGIT *, NN_DIGIT [2], NN_DIGIT));

static NN_DIGIT NN_AddDigitMult PROTO_LIST 
  ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int));
static NN_DIGIT NN_SubDigitMult PROTO_LIST 
  ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int));

static unsigned int NN_DigitBits PROTO_LIST ((NN_DIGIT));

/* Decodes character string b into a, where character string is ordered
   from most to least significant.

   Length: a[digits], b[len].
   Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most
   significant bytes are truncated.)
 */
void NN_Decode (a, digits, b, len)
NN_DIGIT *a;
unsigned char *b;
unsigned int digits, len;
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;
  
  for (i = 0, j = len - 1; j >= 0; i++) {
    t = 0;
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      t |= ((NN_DIGIT)b[j]) << u;
    a[i] = t;
  }
  
  for (; i < digits; i++)
    a[i] = 0;
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

   Lengths: a[len], b[digits].
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
   digits are truncated.)
 */
void NN_Encode (a, len, b, digits)
NN_DIGIT *b;
unsigned char *a;
unsigned int digits, len;
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;

  for (i = 0, j = len - 1; i < digits; i++) {
    t = b[i];
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      a[j] = (unsigned char)(t >> u);
  }

  for (; j >= 0; j--)
    a[j] = 0;
}

/* Assigns a = 2^b.

   Lengths: a[digits].
   Requires b < digits * NN_DIGIT_BITS.
 */
void NN_Assign2Exp (a, b, digits)
NN_DIGIT *a;
unsigned int b, digits;
{
  NN_AssignZero (a, digits);

  if (b >= digits * NN_DIGIT_BITS)
    return;

  a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);
}

/* Computes a = b * c.

   Lengths: a[2*digits], b[digits], c[digits].
   Assumes digits < MAX_NN_DIGITS.
 */
void NN_Mult (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT t[2*MAX_NN_DIGITS];
  unsigned int bDigits, cDigits, i;

  NN_AssignZero (t, 2 * digits);
  
  bDigits = NN_Digits (b, digits);
  cDigits = NN_Digits (c, digits);

  for (i = 0; i < bDigits; i++)
    t[i+cDigits] += NN_AddDigitMult (&t[i], &t[i], b[i], c, cDigits);
  
  NN_Assign (a, t, 2 * digits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b mod c.

   Lengths: a[cDigits], b[bDigits], c[cDigits].
   Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.
 */
void NN_Mod (a, b, bDigits, c, cDigits)
NN_DIGIT *a, *b, *c;
unsigned int bDigits, cDigits;
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];
  
  NN_Div (t, a, b, bDigits, c, cDigits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_NN_DIGITS.
 */
void NN_ModMult (a, b, c, d, digits)
NN_DIGIT *a, *b, *c, *d;
unsigned int digits;
{
  NN_DIGIT t[2*MAX_NN_DIGITS];

  NN_Mult (t, b, c, digits);
  NN_Mod (a, t, 2 * digits, d, digits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes b < d, d > 0, cDigits > 0, dDigits > 0,
           dDigits < MAX_NN_DIGITS.
 */
void NN_ModExp (a, b, c, cDigits, d, dDigits)
NN_DIGIT *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
  NN_DIGIT bPower[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
  int i;
  unsigned int ciBits, j, s;

  /* Store b, b^2 mod d, and b^3 mod d.
   */
  NN_Assign (bPower[0], b, dDigits);
  NN_ModMult (bPower[1], bPower[0], b, d, dDigits);
  NN_ModMult (bPower[2], bPower[1], b, d, dDigits);
  
  NN_ASSIGN_DIGIT (t, 1, dDigits);

  cDigits = NN_Digits (c, cDigits);
  for (i = cDigits - 1; i >= 0; i--) {
    ci = c[i];
    ciBits = NN_DIGIT_BITS;
    
    /* Scan past leading zero bits of most significant digit.
     */
    if (i == (int)(cDigits - 1)) {
      while (! DIGIT_2MSB (ci)) {
        ci <<= 2;
        ciBits -= 2;
      }
    }

    for (j = 0; j < ciBits; j += 2, ci <<= 2) {
      /* Compute t = t^4 * b^s mod d, where s = two MSB's of d.
       */
      NN_ModMult (t, t, t, d, dDigits);
      NN_ModMult (t, t, t, d, dDigits);
      if (s = DIGIT_2MSB (ci))
        NN_ModMult (t, t, bPower[s-1], d, dDigits);
    }
  }
  
  NN_Assign (a, t, dDigits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)bPower, 0, sizeof (bPower));
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Compute a = 1/b mod c, assuming inverse exists.
   
   Lengths: a[digits], b[digits], c[digits].
   Assumes gcd (b, c) = 1, digits < MAX_NN_DIGITS.
 */
void NN_ModInv (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT q[MAX_NN_DIGITS], t1[MAX_NN_DIGITS], t3[MAX_NN_DIGITS],
    u1[MAX_NN_DIGITS], u3[MAX_NN_DIGITS], v1[MAX_NN_DIGITS],
    v3[MAX_NN_DIGITS], w[2*MAX_NN_DIGITS];
  int u1Sign;

  /* Apply extended Euclidean algorithm, modified to avoid negative
     numbers.
   */
  NN_ASSIGN_DIGIT (u1, 1, digits);
  NN_AssignZero (v1, digits);
  NN_Assign (u3, b, digits);
  NN_Assign (v3, c, digits);
  u1Sign = 1;

  while (! NN_Zero (v3, digits)) {
    NN_Div (q, t3, u3, digits, v3, digits);
    NN_Mult (w, q, v1, digits);
    NN_Add (t1, u1, w, digits);
    NN_Assign (u1, v1, digits);
    NN_Assign (v1, t1, digits);
    NN_Assign (u3, v3, digits);
    NN_Assign (v3, t3, digits);
    u1Sign = -u1Sign;
  }
  
  /* Negate result if sign is negative.
    */
  if (u1Sign < 0)
    NN_Sub (a, c, u1, digits);
  else
    NN_Assign (a, u1, digits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)q, 0, sizeof (q));
  R_memset ((POINTER)t1, 0, sizeof (t1));
  R_memset ((POINTER)t3, 0, sizeof (t3));
  R_memset ((POINTER)u1, 0, sizeof (u1));
  R_memset ((POINTER)u3, 0, sizeof (u3));
  R_memset ((POINTER)v1, 0, sizeof (v1));
  R_memset ((POINTER)v3, 0, sizeof (v3));
  R_memset ((POINTER)w, 0, sizeof (w));
}

/* Computes a = gcd(b, c).

   Lengths: a[digits], b[digits], c[digits].
   Assumes b > c, digits < MAX_NN_DIGITS.
 */
void NN_Gcd (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS], v[MAX_NN_DIGITS];

  NN_Assign (u, b, digits);
  NN_Assign (v, c, digits);

  while (! NN_Zero (v, digits)) {
    NN_Mod (t, u, digits, v, digits);
    NN_Assign (u, v, digits);
    NN_Assign (v, t, digits);
  }

  NN_Assign (a, u, digits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
  R_memset ((POINTER)u, 0, sizeof (u));
  R_memset ((POINTER)v, 0, sizeof (v));
}

/* Returns the significant length of a in bits.

   Lengths: a[digits].
 */
unsigned int NN_Bits (a, digits)
NN_DIGIT *a;
unsigned int digits;
{
  if ((digits = NN_Digits (a, digits)) == 0)
    return (0);

  return ((digits - 1) * NN_DIGIT_BITS + NN_DigitBits (a[digits-1]));
}

/* Computes a = c div d and b = c mod d.

   Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
           dDigits < MAX_NN_DIGITS.
 */
static void NN_Div (a, b, c, cDigits, d, dDigits)
NN_DIGIT *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
  NN_DIGIT ai, cc[2*MAX_NN_DIGITS+1], dd[MAX_NN_DIGITS], t;
  int i;
  unsigned int ddDigits, shift;
  
  ddDigits = NN_Digits (d, dDigits);
  if (ddDigits == 0)
    return;
  
  /* Normalize operands.
   */
  shift = NN_DIGIT_BITS - NN_DigitBits (d[ddDigits-1]);
  NN_AssignZero (cc, ddDigits);
  cc[cDigits] = NN_LShift (cc, c, shift, cDigits);
  NN_LShift (dd, d, shift, ddDigits);
  t = dd[ddDigits-1];
  
  NN_AssignZero (a, cDigits);

  for (i = cDigits-ddDigits; i >= 0; i--) {
    /* Underestimate quotient digit and subtract.
     */
    if (t == MAX_NN_DIGIT)
      ai = cc[i+dDigits];
    else
      NN_DigitDiv (&ai, &cc[i+ddDigits-1], t + 1);
    cc[i+ddDigits] -= NN_SubDigitMult (&cc[i], &cc[i], ai, dd, ddDigits);

    /* Correct estimate.
     */
    while (cc[i+ddDigits] || (NN_Cmp (&cc[i], dd, ddDigits) >= 0)) {
      ai++;
      cc[i+ddDigits] -= NN_Sub (&cc[i], &cc[i], dd, ddDigits);
    }
    
    a[i] = ai;
  }
  
  /* Restore result.
   */
  NN_AssignZero (b, dDigits);
  NN_RShift (b, cc, shift, ddDigits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)cc, 0, sizeof (cc));
  R_memset ((POINTER)dd, 0, sizeof (dd));
}

/* Returns nonzero iff a is zero.

   Lengths: a[digits].
 */
int NN_Zero (NN_DIGIT *a, unsigned int digits) {
asm {
		move.l	digits,d0
		beq.s	@zero1
		movea.l	a,a0
		subq.w	#1,d0
@zero2:	move.l	(a0)+,d1
		bne.s	@zero0
		dbf		d0,@zero2
@zero1:	moveq	#1,d0
		bra.s	@zero3
@zero0:	moveq	#0,d0
		}
zero3: return;
}  

/* Assigns a = b.

   Lengths: a[digits], b[digits].
 */
void NN_Assign (NN_DIGIT *a, NN_DIGIT *b, unsigned int digits) {
asm {
		move.l	digits,d0
		beq.s	@asn0
		subq.w	#1,d0
		movea.l	a,a0
		movea.l b,a1
@asn1:	move.l	(a1)+,(a0)+
		dbf		d0,@asn1
		}
asn0: return;
}

/* Assigns a = 0.

   Lengths: a[digits].
 */
void NN_AssignZero (NN_DIGIT *a, unsigned int digits) {
asm {
		move.l	digits,d0
		beq.s	@asz0
		subq.w	#1,d0
		movea.l	a,a0
		moveq	#0,d1
@asz1:	move.l	d1,(a0)+
		dbf		d0,@asz1
		}
asz0: return;
}

/* Returns sign of a - b.

   Lengths: a[digits], b[digits].
 */
int NN_Cmp (NN_DIGIT *a, NN_DIGIT *b, unsigned int digits) {
asm {
		move.l	digits,d0
		beq.s	@cmp0
		move.l	d0,d1
		movea.l	a,a0
		movea.l	b,a1
		lsl.l	#2,d1
		adda.l	d1,a0
		adda.l	d1,a1
		subq.w	#1,d0
@cmp1:	move.l	-(a0),d1
		move.l	-(a1),d2
		cmp.l	d2,d1			; a ? b
		bhi.s	@cmpgt			; a > b
		bcs.s	@cmplt			; a < b
		dbf		d0,@cmp1
		moveq	#0,d0			; a == b
		bra.s	@cmp0
@cmpgt:	moveq	#1,d0
		bra.s	@cmp0
@cmplt:	moveq	#-1,d0
		}
cmp0: return;
}

/* Returns the significant length of a in digits.

   Lengths: a[digits].
 */
unsigned int NN_Digits (NN_DIGIT *a, unsigned int digits) {
asm {
		move.l	digits,d0
		beq.s	@digs0
		move.l	d0,d1
		movea.l	a,a0
		lsl.l	#2,d1
		adda.l	d1,a0
		subq.w	#1,d0
@digs1:	move.l	-(a0),d1
		bne.s	@digs2
		dbf		d0,@digs1
@digs2:	addq.w	#1,d0
		}
digs0: return;
}

/* Returns the significant length of a in bits, where a is a digit.
 */
static unsigned int NN_DigitBits (NN_DIGIT a) {
asm 68030 {
		move.l	a,d0
		moveq	#0,d1
		bfffo	d0{d1:d1},d1	;wow, that was fun!
		moveq	#32,d0
		sub.l	d1,d0
		}
return;
}

/* Computes a = b + c. Returns carry.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Add (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, \
					unsigned int digits) {
asm {
		moveq	#0,d0			;zero return
		move.l  digits,d1 		;# of units
		beq.s	@add0
		movem.l	d3/a2,-(a7)
		movea.l	a,a0
		movea.l	b,a1
		movea.l	c,a2
		subq.w  #1,d1			;one off (dbf counter)
@addlp:	move.l	(a1)+,d2
		move.l	(a2)+,d3
		addx.l	d2,d3
		move.l	d3,(a0)+
		dbf		d1,@addlp
		negx.w	d0
		neg.w	d0				;make carry arithmetic
		movem.l	(a7)+,d3/a2
		}
add0: return;
}

/* Computes a = b - c. Returns borrow.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Sub (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, \
					unsigned int digits) {
asm {
		moveq	#0,d0			;zero return
		move.l  digits,d1 		;# of units
		beq.s	@sub0
		movem.l	d3/a2,-(a7)
		movea.l	a,a0
		movea.l	b,a1
		movea.l	c,a2
		subq.w  #1,d1			;one off (dbf counter)
@sublp:	move.l	(a1)+,d2
		move.l	(a2)+,d3
		subx.l	d3,d2
		move.l	d2,(a0)+
		dbf		d1,@sublp
		negx.w	d0
		neg.w	d0				;make borrow arithmetic
		movem.l	(a7)+,d3/a2
		}
sub0: return;
}

/* Computes a = b + c*d, where c is a digit. Returns carry.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_AddDigitMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, \
					NN_DIGIT *d, unsigned int digits) {
asm 68030 {
		moveq	#0,d0			;zero carry
		move.l	c,d2			;multiplier
		beq.s	@mula0
		move.l  digits,d1 		;# of units
		beq.s	@mula0
		movem.l	d3-d5/a2,-(a7)
		movea.l	a,a0
		movea.l	b,a1
		movea.l	d,a2
		moveq	#0,d5			;fixed zero
		subq.w  #1,d1			;one off (dbf counter)
@mulap:	move.l  (a2)+,d4		;load next multiplicand
		mulu.l	d2,d3:d4		;64-bit multiply
		addx.l	d0,d4			;X = (mpcd * mplr) + carry + X
		addx.l	d5,d3			;catch and carry
		add.l	(a1)+,d4		;add next addend
		move.l	d4,(a0)+
		move.l	d3,d0			;copy next carry
		dbf		d1,@mulap
		addx.l	d5,d0			;catch last carry, if any
		movem.l	(a7)+,d3-d5/a2
		}
mula0: return;
}

/* Computes a = b - c*d, where c is a digit. Returns borrow.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_SubDigitMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, \
					NN_DIGIT *d, unsigned int digits) {
asm 68030 {
		moveq	#0,d0			;zero borrow
		move.l	c,d2			;multiplier
		beq.s	@muls0
		move.l  digits,d1 		;# of units
		beq.s	@muls0
		movem.l	d3-d6/a2,-(a7)
		movea.l	a,a0
		movea.l	b,a1
		movea.l	d,a2
		moveq	#0,d5			;fixed zero
		subq.w  #1,d1			;one off (dbf counter)
@mulsp:	move.l  (a2)+,d4		;load next multiplicand
		mulu.l	d2,d3:d4		;64-bit multiply
		move.l	(a1)+,d6		;b[i]
		subx.l	d0,d6			;a[i] = b[i] - borrow - X
		addx.l	d5,d3			;catch and borrow
		sub.l	d4,d6			;a[i] -= t[0]
		move.l	d6,(a0)+
		move.l	d3,d0			;copy next borrow
		dbf		d1,@mulsp
		addx.l	d5,d0			;catch last borrow, if any
		movem.l	(a7)+,d3-d6/a2
		}
muls0: return;
}

/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires c < NN_DIGIT_BITS.
 */
static NN_DIGIT NN_LShift (NN_DIGIT *a, NN_DIGIT *b, \
					unsigned int c, unsigned int digits) {
asm {
		movem.l	d3-d5,-(a7)
		moveq	#0,d0
		move.l	c,d1
		moveq	#32,d2
		cmp.b	d2,d1
		bge.s	@lsf0			;V1.1
		move.l	digits,d3
		beq.s	@lsf0
		sub.b	d1,d2
		movea.l	a,a0
		movea.l	b,a1
		subq	#1,d3
		move.b	d1,d1			;anything to do?
		bne.s	@lsf1			;yup
@lsf2:	move.l	(a1)+,(a0)+		;nope
		dbf		d3,@lsf2
		bra.s	@lsf0
@lsf1:	move.l	(a1)+,d4
		move.l	d4,d5
		lsl.l	d1,d4
		or.l	d0,d4
		move.l	d4,(a0)+
		lsr.l	d2,d5
		move.l	d5,d0
		dbf		d3,@lsf1
@lsf0:	movem.l	(a7)+,d3-d5
		}
return;
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires: c < NN_DIGIT_BITS.
 */
static NN_DIGIT NN_RShift (NN_DIGIT *a, NN_DIGIT *b, \
					unsigned int c, unsigned int digits) {
asm {
		movem.l	d3-d5,-(a7)
		moveq	#0,d0
		move.l	c,d1
		moveq	#32,d2
		cmp.b	d2,d1
		bge.s	@rsf0			;V1.1
		move.l	digits,d3
		beq.s	@rsf0
		sub.b	d1,d2
		movea.l	a,a0
		movea.l	b,a1
		move.l	d3,d4			;adjust for pre-decrementing
		lsl.l	#2,d4
		adda.l	d4,a0
		adda.l	d4,a1
		subq	#1,d3
		move.b	d1,d1			;anything to do?
		bne.s	@rsf1			;yup
@rsf2:	move.l	-(a1),-(a0)		;nope
		dbf		d3,@rsf2
		bra.s	@rsf0
@rsf1:	move.l	-(a1),d4
		move.l	d4,d5
		lsr.l	d1,d4
		or.l	d0,d4
		move.l	d4,-(a0)
		lsl.l	d2,d5
		move.l	d5,d0
		dbf		d3,@rsf1
@rsf0:	movem.l	(a7)+,d3-d5
		}
return;
}

/* Sets a = b / c, where a and c are digits.

   Lengths: b[2].	[LS digit][MS digit]
   Assumes b[1] < c and HIGH_HALF (c) > 0. For efficiency, c should be
   normalized.
 */
static void NN_DigitDiv (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c) {
asm 68030 {
	move.l	c,d0		/* can't be zero, right? */
	movea.l	b,a0
	move.l	(a0)+,d1	/* first digit is the LS one */
	move.l	(a0),d2
	divu.l	d0,d2:d1	/* 64/32 = d2r, d1q */
	movea.l	a,a0
	move.l	d1,(a0)
	}
return;
}

/*************** end nn.c V1.3 921121 10:14 rwo *********************/
