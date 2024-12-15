/* A set of -*- C++ -*- classes for generic (mixed-type) arithmetic.
   Copyright (C) 1992 Per Bothner.

This file is part of Q.

Q is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Q is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

// Some thought to re-using storage is needed (though I believe
// garbage collection is best in the long run).

// Some of the routines are modification of ones in the PRL
// bignum package.
// These will be replaced soon by gmp-based ones.  FIXME.

// MIXED-TYPE ARITHMETIC:
// Two different scheme for handling mixed types are used.
// Assume we're evaluating x-y, where x has type TX and y has type TY.
// The task is to find the correct routine to do the subtraction.
//
// 1) Double dispatch:
//    x-y is implemented as x->sub(*y), which invokes TX::sub.
//    x->TX::sub(y) calls y->subTX(x), which invokes TY::subTX,
//    which knows the appopriate rule for subtract a TY from a TX.
//
// 2) Promotion:
//    x-y is implemented as x->suv(*y), which invokes TX:sub.
//    TX::sub(y) checks if y can be coerced (promoted) to TX,
//    in which case it does x-(TX)y.
//    Otherwise, TX::sub calls y->subr(*this), which invokes TY::subr.
//    (The 'r' in subr means reversed.)
//    TY::subr(x) checks if x can be coerced to a TY,
//    in which case it does (TY)x-y.
//    Otherwise, the subtraction fails.
//
// Having the two schemes is just experimental, until I decide
// which is better.


#pragma implementation
#include "gennum.h"
#include <iostream.h>
#include "genmap.h"
#include "gkinds.h"
#include "gcompile.h"
#include "exceptions.h"
#include "gfiles.h"
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <gmp.h>
#include <gmp-impl.h>
#include "longlong.h"
/*#include "../bignum/h/BigNum.h"*/
}
#if BITS_PER_MP_LIMB==16
#define BITS_TO_LIMBS(x) ((x) >> 4)
#elif BITS_PER_MP_LIMB==32
#define BITS_TO_LIMBS(x) ((x) >> 5)
#else
#define BITS_TO_LIMBS(x) ((unsigned)(x) / BITS_PER_MP_LIMB)
#endif

#ifndef ALLOCATE_LOCAL
#define ALLOCATE_LOCAL(size) alloca(size)
#endif
#ifndef DEALLOCATE_LOCAL
#define DEALLOCATE_LOCAL(size) /* do nothing */
#endif

#define NOT_IMPLEMENTED() return NULL

inline int _mpn_is_zero(mp_srcptr p, mp_size i)
{ for (; i > 0; i--) if (*p++) return 0; return 1; }
//inline void _mpn_clear(mp_ptr p, mp_size i) { for (; i > 0; i--) *p++ = 0; }
//inline void _mpn_copy(mp_ptr d, mp_srcptr s, mp_size i)
//{ for (; i > 0; i--) *d++ = *s++; }
inline void _mpn_increment(mp_ptr d, mp_srcptr s, mp_size i)
{
    mp_limb one = 1;
    _mpn_add(d, s, i, &one, 1);
}
inline void _mpn_invert(mp_ptr d, mp_srcptr s, mp_size len)
{
    while (--len >= 0) *d++ = ~*s++;
}
inline void _mpn_negate(mp_ptr d, mp_srcptr s, mp_size len)
{
    _mpn_invert(d, s, len);
    _mpn_increment(d, d, len);
}
/* Count the number of limbs needed for p.  Return at least 1. */
inline mp_size _mpn_count_nonzero(mp_srcptr p, mp_size len)
{
    for (p += len; len > 1 && *--p == 0; len--) ;
    return len;
}

#ifdef Q_ENV
long Numeric::magic() const { return NumericKind; }
long Integer::magic() const { return IntegerKind; }
#endif

long print_base = 10;
long print_width = -1;
long print_pad_char = ' ';
long print_readable = 0;
long print_precision = -1;
long print_float_format = 'g';

const Numeric * Numeric::mul(const Numeric&) const { return NULL; }
const Numeric * Numeric::div(const Numeric&) const { return NULL; }
const Numeric * Numeric::addFixInt(fix_int) const { return NULL; }
const Numeric * Numeric::subFixInt(fix_int) const { return NULL; }
const Numeric * Numeric::mulFixInt(fix_int) const { return NULL; }
const Numeric * Numeric::divFixInt(fix_int) const { return NULL; }
const Numeric * Numeric::addBigInt(BigInt&) const {return NULL;}
const Numeric * Numeric::subBigInt(BigInt&) const {return NULL;}
const Numeric * Numeric::mulBigInt(BigInt&) const {return NULL;}
const Numeric * Numeric::divBigInt(BigInt&) const {return NULL;}
const Numeric * Numeric::addDouble(double) const {return NULL;}
const Numeric * Numeric::subDouble(double) const {return NULL;}
const Numeric * Numeric::mulDouble(double) const {return NULL;}
const Numeric * Numeric::divDouble(double) const {return NULL;}
const Numeric * Numeric::ipower(fix_int) const { return NULL; }
int Numeric::compareRational(const Rational&) const { return -2; }
int Numeric::compareFixInt(fix_int) const { return -2; }
int Numeric::compareDouble(double) const { return -2; }
int Numeric::getlong(long *) const  { return 0; }

extern char * BigToString (const Integer* z, int base);

void Numeric::printon(ostream& outs) const { outs << "[?Number?]"; }

Numeric *Numeric::as_exact()
{
    return this;
}

Numeric *Numeric::as_inexact()
{
    return this;
}

void NotANumber::printon(ostream& outs) const
{
    switch (kind)  {
      case PosInfinityCode: outs << "Infinity"; break;
      case NegInfinityCode: outs << "-Infinity"; break;
      default: outs << "NaN[" << kind << "]";
    }
}
void Integer::printon(ostream& outs) const
{
    char base_buf[4];
    int base_len;
    const BigInt *this_abs = this;
    int put_minus = 0;
    if (is_negative()) {
	this_abs = &-(*this);
	put_minus = 1;
    }
    char *str = BigToString(this_abs, print_base);
    if (print_base != 10 && print_readable) {
	sprintf(base_buf, print_base == 16 ? "0x" : "%dr", print_base);
	base_len = strlen(base_buf);
    } else
	base_len = 0;
    int width = put_minus+base_len+strlen(str);
    int padding = (print_width >= 0 ? print_width : -print_width) - width;
    if (padding > 0 && print_width > 0 && print_pad_char != '0') {
	while (--padding >= 0) outs << (char)print_pad_char;
    }
    if (put_minus)
	outs << '-';
    if (padding > 0 && print_width > 0) {
	while (--padding >= 0) outs << (char)print_pad_char;
    }
    if (base_len)
	outs << base_buf;
    outs << str;
    if (padding > 0 && print_width < 0) {
	while (--padding >= 0) outs << (char)print_pad_char;
    }
    free((void*)str);
    if (this_abs != this) ((BigInt*)this_abs)->do_delete();
}
void Complex::printon(ostream& outs) const
{
    const Real& re = realPart();
    const Real& im = imagPart();
    outs << re;
    if (&im != Zero)
	outs << "+%" << im;
}

void RationalInfinity::printon(ostream& outs) const
{
    switch (val)  {
      case -1: outs << "-Infinity"; break;
      case 1: outs << "Infinity"; break;
      default: abort();
    }
}

double Double::as_double() const { return val; }

void Double::printon(ostream& outs) const
{
#if 1
    if (print_width >= 0)
	outs.width(print_width);
    unsigned long new_flags = 0;
    unsigned long mask =
	ios::showpoint|ios::uppercase|ios::fixed|ios::scientific;
    if (print_readable)
	new_flags |= ios::showpoint;
    switch (print_float_format) {
      case 'E':
	new_flags |= ios::uppercase;
      case 'e':
	new_flags |= ios::scientific;
	break;
      case 'F':
	new_flags |= ios::uppercase;
      case 'f':
	new_flags |= ios::fixed;
	break;
      default:
      case 'g':
	;
    }
    unsigned long old_flags = outs.setf(new_flags, mask);
    int old_precision;
    if (print_precision >= 0)
	old_precision = outs.precision(print_precision);
    outs << val;
    if (print_precision >= 0)
	outs.precision(old_precision);
    outs.flags(old_flags);
#else
    char format[20];
    int width = print_width;
    if (width < 0) width = 0;
    width += 10;
    char *buffer = (char*)alloca(width);
    char *fptr = format;
    *fptr++ = '%';
    if (print_width >= 0) {
	sprintf(fptr, "%d", print_width);
	fptr += strlen(fptr);
    }
    if (print_precision >= 0) {
	sprintf(fptr, ".%d", print_precision);
	fptr += strlen(fptr);
    }
    *fptr++ = print_float_format;
    *fptr = '\0';
    sprintf(buffer, format, val);

    // Kludge to make sure the last char isn't '.'.
    fptr = buffer + strlen(buffer);
    if (fptr[-1] == '.') {
	fptr[0] = '0';
	fptr[1] = 0;
    }

    outs << buffer;
#endif
}

const Numeric * Double::addr(const Numeric& other) const
{
    const Real *r_other = other.real();
    if (r_other) return new Double(r_other->as_double() + val);
    else return NULL;
}

const Numeric * Double::subr(const Numeric& other) const
{
    const Real *r_other = other.real();
    if (r_other) return new Double(r_other->as_double() - val);
    else return NULL;
}

const Numeric * Double::mulr(const Numeric& other) const
{
    const Real *r_other = other.real();
    if (r_other) return new Double(r_other->as_double() * val);
    else return NULL;
}

const Numeric * Double::divr(const Numeric& other) const
{
    const Real *r_other = other.real();
    if (r_other) return new Double(r_other->as_double() / val);
    else return NULL;
}

void Fraction::printon(ostream& outs) const { outs << num << '/' << den; }

int NotANumber::sign() const
{
    switch (kind)  {
      case PosInfinityCode: return 1;
      case NegInfinityCode: return -1;
      default: return 0;
    }
}

const Numeric& NotANumber::neg() const
{
    switch (kind)  {
      case PosInfinityCode: return NegInfinity;
      case NegInfinityCode: return PosInfinity;
      default: return *(const Numeric*)NULL;
    }
}

int NotANumber::is_exact() const { return 1; }

const Numeric * Complex::add(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*this + *c_other);
    else return other.addr(*this);
}
const Numeric * Complex::addr(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*c_other + *this);
    else return NULL;
}
const Numeric * Complex::sub(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*this - *c_other);
    else return other.subr(*this);
}
const Numeric * Complex::subr(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*c_other - *this);
    else return NULL;
}
const Complex& operator*(const Complex &x, const Complex& y)
{
    const Real& xr = x.realPart();
    const Real& xi = x.imagPart();
    const Real& yr = y.realPart();
    const Real& yi = y.imagPart();
    const Real& r = xr * yr - xi * yi;
    const Real& i = xi * yr + xr * yi;
    if (&i == Zero) return r; else return *new ComplexPair(r, i);
}
const Numeric * Complex::mul(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*this * *c_other);
    else return other.subr(*this);
}
const Numeric * Complex::mulr(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*c_other * *this);
    else return NULL;
}
const Complex& operator/(const Complex &x, const Complex& y)
{
    const Real& xr = x.realPart();
    const Real& xi = x.imagPart();
    const Real& yr = y.realPart();
    const Real& yi = y.imagPart();
    const Real& norm = yr * yr + yi * yi;
    return *new ComplexPair((xr * yr + xi * yi) / norm,
			    (xi * yr - xr * yi) / norm);
}
const Numeric * Complex::div(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*this / *c_other);
    else return other.subr(*this);
}
const Numeric * Complex::divr(const Numeric& other) const
{
    const Complex *c_other = other.complex();
    if (c_other) return &(*c_other / *this);
    else return NULL;
}

const Numeric * Complex::addFixInt(fix_int i) const 
{
    return new ComplexPair(i + realPart(), imagPart());
}

const Numeric * Complex::subFixInt(fix_int i) const 
{
    return new ComplexPair(i - realPart(), imagPart());
}

const Numeric * Complex::mulFixInt(fix_int i) const 
{
    if (i == 0) return Zero;
    return new ComplexPair(i * realPart(), i * imagPart());
}
const Numeric * Complex::divFixInt(fix_int i) const 
{
    const Real& yr = realPart();
    const Real& yi = imagPart();
    const Real& norm = yr * yr + yi * yi;
    return new ComplexPair((i * yr) / norm,
			   (i * yi).rneg() / norm);
}

const Numeric* Complex::addDouble(double x) const
{return new ComplexPair(*new Double(x+realPart().as_double()), imagPart());}
const Numeric* Complex::subDouble(double x) const
{return new ComplexPair(*new Double(x-realPart().as_double()), imagPart());}
const Numeric* Complex::mulDouble(double x) const
{return new ComplexPair(*new Double(x*realPart().as_double()), imagPart());}
const Numeric* Complex::divDouble(double x) const
{
    double yrd = realPart().as_double();
    double yid = imagPart().as_double();
    double norm = yrd * yrd + yid * yid;
    return new ComplexPair(*(new Double(x*yrd / norm)),
			   *(new Double(- x*yid / norm)));
}

int Complex::is_exact() const
{
    return realPart().is_exact() && imagPart().is_exact();
}

const Numeric& ComplexPair::neg() const
{
    return *new ComplexPair(re.rneg(), im.rneg());
}
int ComplexPair::sign() const {return re.sign() ? 1 : im.sign();} //ARBITRARY!

Numeric *ComplexPair::as_exact()
{
    return this;
}

Numeric *ComplexPair::as_inexact()
{
    if (is_exact() == 0)
	return this;
    return new ComplexPair(*(new Double(re.as_double())),
			   *(new Double(im.as_double())));
}

Integer * NewBigInt(int len)
{
    Integer* I = (Integer*)malloc(sizeof(Integer) + (len-1) * sizeof(fix_int));
    I->Integer::Integer();
    I->len = len;
    return I;
}

Integer * NewBigInt(int len, fix_unsigned* data)
{
    Integer* I = NewBigInt(len);
    MPN_COPY(I->L, (mp_ptr)data, len);
    return I;
}

Integer * NewBigInt(int len, mp_ptr data)
{
    Integer* I = NewBigInt(len);
    MPN_COPY(I->L, data, len);
    return I;
}

int BigDigitsNeeded(mp_ptr words, int len)
{
    register fix_int *ptr = (fix_int*)words + len;
    int new_len = len;
    fix_int word = *--ptr;
    if (word == -1)
	while (new_len > 1 && (word = *--ptr) < 0) {
	    new_len--;
	    if (word != -1) break;
	}
    else
	while (word == 0 && new_len > 1 && (word = *--ptr) >= 0) new_len--;
    return new_len;
}
int BigDigitsNeeded(fix_unsigned* words, int len)
{
    return BigDigitsNeeded((mp_ptr)words, len);
}

inline int DigitsNeeded(int i) {return BITS_TO_LIMBS(i+(BITS_PER_MP_LIMB-1));}

void div(const Integer& X,  const Integer& Y,  	  enum RealToIntMode
mode, 	  const Integer**  quotient, const  Integer**  remainder)
{ //  register i
    int i;
    //register fix_unsigned* p;
    int dsize = Y.big_len();
    int nsize = X.big_len();
    int nLen = (nsize > dsize ? nsize : dsize) + 1; // size of main work area.
    mp_ptr np = (mp_ptr)alloca(nLen * sizeof(fix_unsigned));
    mp_ptr dp, qp, rp;
    int temp_dp = 0;
    int qNegative; // True if quotient is negative.
    if ((qNegative = Y.is_negative()) || (remainder && mode != TruncateMode)) {
	dp = (mp_ptr)alloca(dsize * sizeof(mp_limb));
	temp_dp = 1;
	if (qNegative)
	    _mpn_negate((mp_ptr)dp, Y.L, dsize);
	else
	    MPN_COPY((mp_ptr)dp, Y.L, dsize);
    }
    else
	dp = Y.L;
    int xNegative = X.is_negative();
    if (xNegative) {
	_mpn_negate(np, X.L, nsize);
	qNegative = 1 - qNegative;
    }
    else
	MPN_COPY(np, X.L, nsize);
    rp = np;
    MPN_ZERO(np+nsize, nLen-nsize);
    for (;; nsize--) {
	if (nsize == 0) {
	    if (quotient) *quotient = Zero;
	    if (remainder) *remainder = &X;
	    return;
	}
	if (np[nsize-1] != 0) break;
    }
    for (;; dsize--) {
	if (dsize == 0) {
	    // ZERO-DIVIDE!
	    return;
	}
	if (dp[dsize-1] != 0) break;
    }
    int qsize = nsize - dsize + 1;     /* qsize cannot be bigger than this.  */
    qp = (mp_ptr) alloca (qsize * BYTES_PER_MP_LIMB);
    int rsize;
    unsigned normalization_steps;
    count_leading_zeros (normalization_steps, dp[dsize - 1]);
    
    mp_ptr tp; /* Normalized copy (if needed) of dp. */
    
    // Normalize the denominator, i.e. make its most significant bit set by
    // shifting it NORMALIZATION_STEPS bits to the left.  Also shift the
    // numerator the same number of steps (to keep the quotient the same!).
    if (normalization_steps != 0) {
	mp_limb ndigit;
	
	// Shift up the denominator setting the most significant bit of
        // the most significant word.  Use temporary storage not to clobber
	// the original contents of the denominator.
	tp = temp_dp ? dp : (mp_ptr) alloca (dsize * BYTES_PER_MP_LIMB);
	(void) _mpn_lshift (tp, dp, dsize, normalization_steps);

	// Shift up the numerator, possibly introducing a new most
	// significant word.  Move the shifted numerator in the remainder
	// meanwhile.
	ndigit = _mpn_lshift (rp, np, nsize, normalization_steps);
      if (ndigit != 0) {
	  rp[nsize] = ndigit;
	  rsize = nsize + 1;
      }
      else
	  rsize = nsize;
    }
    else {
	/* Move the numerator to the remainder.  */
	MPN_COPY (rp, np, nsize);
	
	rsize = nsize;
	tp = dp;
    }
    
    qsize = rsize - dsize + _mpn_div (qp, rp, rsize, tp, dsize);
    
    rsize = dsize;
    
    /* Normalize the remainder.  */
    while (rsize > 0) {
	if (rp[rsize - 1] != 0)
	    break;
	rsize--;
    }
    if (normalization_steps != 0)
	rsize = _mpn_rshift (rp, rp, rsize, normalization_steps);
    int exact = rsize <= 1 && rp[0] == 0;
    int add_one = 0;
    switch (mode) {
      case TruncateMode:
	break;
      case CeilingMode:
	if (qNegative) break;
	goto test_fraction;
      case FloorMode:
	if (!qNegative) break;
      test_fraction:
	if (!exact) add_one = 1;
	break;
      case RoundMode:
	// NOT IMPLEMENTED!
	break;
    }
    if (quotient) {
	if (qsize == 0)
	    *quotient = Zero;
	else {
	    if (qNegative)
		_mpn_invert(qp, qp, qsize);
	    if (qNegative != add_one)
		_mpn_increment(qp, qp, qsize);
	    qsize = BigDigitsNeeded(qp, qsize);
	    if (qsize == 1)
		*quotient = MakeFixInt((fix_int)qp[0]);
	    else
		*quotient = NewBigInt(qsize, qp);
	}
    }
    if (remainder) {
	// The remainder is by definition: X-Q*Y
	if (add_one) {
	    // Subtract the remainder from Y.
	    if (normalization_steps && tp == dp)
		_mpn_rshift (dp, dp, dsize, normalization_steps);
	    _mpn_sub(dp, dp, dsize, rp, rsize);
	    rsize = BigDigitsNeeded(dp, dsize);
	    rp = dp;
	    // In this case, abs(Q*Y) > abs(X).
	    // So sign(remainder) = -sign(X).
	    xNegative = 1 - xNegative;
	} else {
	    // If !add_one, then: abs(Q*Y) <= abs(X).
	    // So sign(remainder) = sign(X).
	}
	if (xNegative) {
	    _mpn_negate((mp_ptr)rp, (mp_ptr)rp,
			rsize);
	}
	if (rsize <= 1)
	    *remainder = MakeFixInt((fix_int)rp[0]);
	else
	    *remainder = NewBigInt(rsize, rp);
    }
}

void div(const Real& X, const Real& Y,
	 enum RealToIntMode mode,
	 const Real** quotient, const Real** remainder)
{
    const Integer *Xint = X.integer();
    if (Xint) {
	const Integer *Yint = Y.integer();
	if (Yint) {
	    div(*Xint, *Yint, mode,
		(const Integer**)quotient, (const Integer**)remainder);
	    return;
	}
    }
    const Real& ratio = X / Y;
    const ExtendedInteger& Q = ratio.to_integer(mode);
    if (quotient) *quotient = &Q;
    if (remainder) *remainder = &(X - Q * Y);
}

const ExtendedInteger& Fraction::to_integer(enum RealToIntMode mode) const
{
    Integer* quotient;
    ::div(num, den, mode, &quotient, NULL);
    return *quotient;
}

union double_union {
    double d;
    long l[2];
    struct {
#define EXPONENT_LENGTH 11
#define EXPONENT_MASK 0x7FF0
#define EXPONENT_BIAS 0x400 /* 1024 */
#ifndef LittleEndian
	unsigned sign : 1;
	unsigned exponent : EXPONENT_LENGTH;
	unsigned w1 : 31-EXPONENT_LENGTH;
	unsigned long w2;
#else
	unsigned long w2;
	unsigned w1 : 31-EXPONENT_LENGTH;
	unsigned exponent : EXPONENT_LENGTH;
	unsigned sign : 1;
#endif
    } b;
};

const ExtendedInteger& Double::to_integer(enum RealToIntMode mode) const
{
    // Does not handle NaNs, infinities, -0.0, or unnormalized numbers.
    int i;
#define MAX_DOUBLE_FIXES 34 // Max size in fix_ints for double: 1024+64 bits
    mp_limb buf[MAX_DOUBLE_FIXES]; 
    double_union u;
    u.d = val;
    int sign = u.b.sign;
    int exp = u.b.exponent - EXPONENT_BIAS;
    u.b.exponent = 1; /* add supressed bit */
    u.b.sign = 0;
    /* now l[0..1] is the factor to be shifted */
#define FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA 2
    // Copy mantissa to first FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA
    // fix_ints of buf. MACHINE DEPENDENCY!
#ifdef LittleEndian
    buf[0] = u.l[0];
    buf[1] = u.l[1];
#else
    buf[0] = u.l[1];
    buf[1] = u.l[0];
#endif
    // We consider the buf to be an integer, with the
    // binary point at the far "right" (little end).
    // Adjust exp to compensate for this. MACHINE DEPENDENCY!
    exp -= 51;
    if (exp >= 0) { // shift left
	// exp_words == # of whole fix_ints to shift by.
	int exp_words = BITS_TO_LIMBS(exp);
	int total_length = exp_words+(FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA+1);
	// Set up buf to consist of:
	// a) First exp_words contain 0.
	// b) Next FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA contain mantissa.
	// c) A single high-order fix_int containing 0 (to shift into).
	for (i = FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA; --i >= 0; )
	    buf[i + exp_words] = buf[i];
	for (i = exp_words; --i >= 0; ) buf[i] = 0;
	buf[total_length-1] = 0;
	_mpn_lshift(buf, buf, total_length, exp & BITS_PER_MP_LIMB-1);
	if (sign) // Negative
	    _mpn_negate(buf, buf, total_length);
	return *NewBigInt(BigDigitsNeeded(buf, total_length), buf);
    } else { // shift right
	exp = -exp;
	// exp_words == # of whole fix_ints to shift by.
	int exp_words = BITS_TO_LIMBS(exp);
	int total_length = FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA - exp_words;
	int add_one = 0; // If absolute value of result should be increased.
	mp_limb shifted =
	    _mpn_rshift(buf, buf,
			FIXINTS_NEEDED_FOR_DOUBLE_MANTISSA,
			exp & BITS_PER_MP_LIMB-1);
	register mp_ptr p;
	switch (mode) {
	  case TruncateMode:
	    break;
	  case CeilingMode:
	    if (sign) break;
	    goto test_fraction;
	  case FloorMode:
	    if (!sign) break;
	  test_fraction:
	    if (shifted) add_one = 1;
	    else
		for (p = buf, i = exp_words; --i >= 0; )
		    if (*p++ != 0) { add_one = 1; break; }
	    break;
	  case RoundMode:
	    fix_unsigned half = (fix_unsigned)FixInt::smallest();
	    i = exp_words;
	    p = buf+i;
	    fix_unsigned word = --i >= 0 ? *--p : shifted;
	    if (word < half) break;
	    else if (word > half) { add_one = 1; break; }
	    for (;;) {
		word = --i >= 0 ? *--p : shifted;
		if (word > 0) { add_one = 1; break; }
		if (i <= 0) {
		    // The input is exactly between two integers.
		    // Round to the one that is even.
		    if (buf[exp_words] & 1) add_one = 1;
		    break;
		}
	    }
	}
	if (total_length <= 0)
	    if (!add_one) return *Zero;
	    else if (sign) return *MinusOne;
	    else return *One;
	if (sign) { // Negative
	    for (p = buf+exp_words, i = total_length; --i >= 0; p++)
		*p = ~*p;
	    add_one = 1 - add_one;
	}
	if (add_one)
	    _mpn_increment(buf+exp_words, buf+exp_words, total_length);
	total_length = BigDigitsNeeded(buf+exp_words, total_length);
	if (total_length == 1)
	    return *MakeFixInt((fix_int)buf[exp_words]);
	return *NewBigInt(total_length, buf+exp_words);
    }
}

int Double::is_exact() const { return 0;}

const Real& Real::realPart() const { return *this; }
const Real& Real::imagPart() const { return *Zero; }

const Numeric* Real::addDouble(double x) const
{return new Double(x+as_double());}
const Numeric* Real::subDouble(double x) const
{return new Double(x-as_double());}
const Numeric* Real::mulDouble(double x) const
{return new Double(x*as_double());}
const Numeric* Real::divDouble(double x) const
{return new Double(x/as_double());}

double Real::as_double() const
{
    cerr << " NOT IMPLEMENTED Real::as_double()!\n";
    return 0;
}

Numeric * Real::as_inexact()
{
    if (is_exact() == 0)
	return this;
    return new Double(as_double());
}

const ExtendedInteger& Real::div_floor(const Real& divisor) const
{
    return ConvertReal(div(divisor))->floor();
}

const Numeric * Rational::add(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*this + *r_other);
    else return other.addr(*this);
}
const Numeric * Rational::addr(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*r_other + *this);
    else return NULL;
}

const Numeric * Rational::sub(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*this - *r_other);
    else return other.subr(*this);
}
const Numeric * Rational::subr(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*r_other - *this);
    else return NULL;
}

const Numeric * Rational::mul(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*this * *r_other);
    else return other.mulr(*this);
}
const Numeric * Rational::mulr(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*r_other * *this);
    else return NULL;
}
const Numeric * Rational::div(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*this / *r_other);
    else return other.divr(*this);
}
const Numeric * Rational::divr(const Numeric& other) const
{
    const Rational *r_other = other.rational();
    if (r_other) return &(*r_other / *this);
    else return NULL;
}

const Numeric * Rational::divFixInt(fix_int arg) const
{
    return MakeRational(arg * denominator(), numerator());
}

const Rational *MakeRational(const Integer& num, const Integer& den)
{
    if (&den == Zero)
	return MakeRational(num.sign(), 0);  // Select the proper Infinity.
    const Integer *g = num.gcd(den);
#if 1
    if (&num == Zero)
	return Zero;
#else
    if (g != One) {
	if (g->is_fix()) {
	    if (g->fix_value() == 0) { // In this case num and/or den is Zero.
	    	return MakeRational(num.sign(), den.sign());
	    }
	    if (den.is_fix() && num.is_fix())
	    int den_is_fix = den.getlong(&l_den);
	    int num_is_fix = num.getlong(&l_num);
	    if (den_is_fix) l_den = l_den / l_g;
	    if (num_is_fix) l_num = l_num / l_g;
	    if (den_is_fix + num_is_fix == 2)
	    	return MakeRational(l_num, l_den);
	}
    }
#endif
    if (den.is_negative())
	g = &(- *g);
    const Integer *new_num, *new_den;
    div(num, *g, TruncateMode, &new_num, NULL);
    div(den, *g, TruncateMode, &new_den, NULL);
    if (new_den == One)
	return new_num;
    return new Fraction(*new_num, *new_den);
}

// Return least i such that word&(1<<i). Assumes word!=0.

static int find_lowest_bit(register fix_unsigned word)
{
    register int i = 0;
    while (!(word & 0xF)) word >>= 4, i += 4;
    if (!(word & 3)) word >>= 2, i += 2;
    if (!(word & 1)) i += 1;
    return i;
}

static unsigned char _hi_bit[16] = {
    0, 1, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
};

int integer_length(fix_unsigned i)
{
    int size = 0;
    while (i >= (1 << 16)) size += 16, i >>= 16;
    if (i >= (1 << 8)) size += 8, i >>= 8;
    if (i >= (1 << 4)) size += 4, i >>= 4;
#if 0
    if (i >= (1 << 2)) size += 2, i >>= 2;
    if (i >= (1 << 1)) size += 1, i >>= 1;
    if (i >= 1) size += 1;
    return size;
#else
    return size + _hi_bit[i];
#endif
}

int integer_length(fix_int i)
{
    int size = 0;
    if (i < 0) {
	while (i < (-1 << 16)) size += 16, i >>= 16;
	if (i < (-1 << 8)) size += 8, i >>= 8;
	if (i < (-1 << 4)) size += 4, i >>= 4;
	if (i < (-1 << 2)) size += 2, i >>= 2;
	if (i < (-1 << 1)) size += 1, i >>= 1;
	if (i < -1) size += 1;
    }
    else {
	while (i >= (1 << 16)) size += 16, i >>= 16;
	if (i >= (1 << 8)) size += 8, i >>= 8;
	if (i >= (1 << 4)) size += 4, i >>= 4;
#if 0
	if (i >= (1 << 2)) size += 2, i >>= 2;
	if (i >= (1 << 1)) size += 1, i >>= 1;
	if (i >= 1) size += 1;
#else
	return size + _hi_bit[i];
#endif
    }
    return size;
}

int Integer::integer_length() const
// ceiling(log2(i < 0 ? -i : i+1))
// See Common Lisp: the Language, 2nd ed, p. 361
{
    // The loop is just to catch non-normalized integers.
    for (register int index = len-1; index >= 0; index--) {
	int size = ::integer_length(S[index]);
	if (size) return size + BITS_PER_MP_LIMB * index;
    }
    return 0;
}

// bit_count[I] is number of '1' bits in I.
const unsigned char four_bit_count[16] = {
    0, 1, 1, 2,
    1, 2, 2, 3,
    1, 2, 2, 3,
    2, 3, 3, 4};
    

int bit_count(register unsigned long i)
{
    register int count = 0;
    while (i > 0) {
	count += four_bit_count[i & 15];
	i >>= 4;
    }
    return count;
}
int bit_count(register fix_unsigned* words, register int length)
{
    register int count = 0;
    while (--length >= 0) {
	register fix_unsigned i = *words++;
	while (i > 0) {
	    count += four_bit_count[i & 15];
	    i >>= 4;
	}
    }
    return count;
}

// Implement Common Lisp (log-count I)
int bit_count(Integer& arg)
{
    int i = bit_count(arg.U, arg.len);
    return arg.is_negative() ? arg.len * BITS_PER_MP_LIMB - i : i;
}

const Integer& gcd(const Integer& a0, const Integer& a1)
{
    int i = a0.big_len();
    int j = a1.big_len();
    if (_mpn_is_zero(a0.L, 1) || _mpn_is_zero(a1.L, j))
	return *Zero;
    int alen = max(i, i);
    mp_ptr a0p = ALLOCATE_LOCAL(2 * sizeof(fix_unsigned)*alen);
    mp_ptr a1p = a0p + alen;

    // Copy absolute value of a0 to a0p.
    if (a0.is_negative())
	_mpn_negate(a0p, a0.L, i);
    else
	MPN_COPY(a0p, a0.L, i);
    if (alen > i) MPN_ZERO(a0p + i, alen - i);

    // Copy absolute value of a1 to a1p.
    MPN_COPY(a1p, a1.L, j);
    if (a1.is_negative())
	_mpn_negate(a1p, a1p, alen);
    if (alen > j) MPN_ZERO(a1p + j, alen - j);

    // Find i such that both a0p and a1p are divisible by 2**i.
    mp_limb *p0 = a0p, *p1 = a1p;
    mp_limb word;
    for (i = 0; ; i++) {
	word = *p0|*p1;
	if (word) break; // Must terminate, since a0 and a1 are non-zero.
    }
    int initShiftWords = i;
    int initShiftBits = find_lowest_bit(word);
    // Logically: i = initShiftWords * BITS_PER_MP_LIMB + initShiftBits

    // Temporarily devide both a0p and a1p by 2**i.
    a0p += initShiftWords;
    a1p += initShiftWords;
    alen -= initShiftWords;
    if (initShiftBits) {
	_mpn_rshift(a0p, a0p, alen, initShiftBits);
	_mpn_rshift(a1p, a1p, alen, initShiftBits);
    }

    mp_ptr odd_arg; // One of a0p or a1p which is odd.
    mp_ptr other_arg; // The other one; can be even or odd.
    if (a0p[0] & 1) odd_arg = a0p, other_arg = a1p;
    else odd_arg = a1p, other_arg = a0p;

    for (;;) {
	// Shift other_arg until it is odd; this doesn't
	// affect the gcd, since we divide by 2**k, which does not
	// divide odd_arg.
	for (p0 = other_arg, i = 0; !*p0; p0++, i++) ;
	if (i) {
	    for (j = 0; j < alen-i; j++)
		other_arg[j] = other_arg[j+i];
	    MPN_ZERO(other_arg+alen-i, i);
	    p0 = other_arg;
	}
	i = find_lowest_bit(*p0);
	if (i)
	    if (_mpn_rshift(other_arg, other_arg, alen, i) != alen)
		other_arg[alen-1] = 0;
	
	// Now both odd_arg and other_arg are odd.

	// Subtract the smaller from the larger.
	// This does not change the result, since gcd(a-b,b)==gcd(a,b).
	int i = _mpn_cmp(odd_arg, alen, other_arg, alen);
	if (i == 0)
	    break;
	if (i > 0) { // odd_arg > other_arg
	    _mpn_sub(odd_arg, odd_arg, alen, other_arg, alen);
	    // Now odd_arg is even, so swap with other_arg;
	    mp_ptr tmp = odd_arg; odd_arg = other_arg; other_arg = tmp;
	}
	else { // other_arg > odd_arg
	    _mpn_sub(other_arg, other_arg, alen, odd_arg, alen);
	}
    }
    _mpn_lshift(a0p, a0p, alen, initShiftBits);
    alen += initShiftWords;
    a0p -= initShiftWords;
    a1p -= initShiftWords;
    while (alen > 1 && a0p[alen-1] == 0) alen--;
    // Always safe, since at worst a1p (continguous with a0p) may be trashed.
    if ((fix_int)a0p[alen-1] < 0) a0p[alen++] = 0;
    const Integer *result = alen>1 ? NewBigInt(alen,a0p) : MakeFixInt(a0p[0]);
    DEALLOCATE_LOCAL(a0p);
    return *result;
}

#if 0
const Integer& lcm(const Integer& x, const Integer& y)
{
    if (&x == Zero || &y == Zero)
	return Zero;
    Integer& g = gcd(x, y);
    return (x IDIV g) * y;
}
#endif

fix_unsigned gcd(fix_unsigned a, fix_unsigned b)
{ // Euclid's algorithm, copied from libg++.
    fix_unsigned tmp;

    if (b > a) {
	tmp = a; a = b; b = tmp;
    }
    for(;;) {
	if (b == 0)
	    return a;
	else if (b == 1)
	    return b;
	else {
	    tmp = b;
	    b = a % b;
	    a = tmp;
	}
    }
}

inline fix_unsigned fix_abs(fix_int x) { return x < 0 ? -x : x; }

const Rational *MakeRational(fix_int num, fix_int den)
{
    if (den == 0) {
	if (num > 0) return &PosInfinity;
	else if (num == 0) return Zero; // Some prefer One
	else return &NegInfinity;
    }
    if (den == FixInt::smallest() || num == FixInt::smallest()) {
	// Not safe to deal with these values.
	SmallBigInt num_big(num);
	SmallBigInt den_big(den);
	return MakeRational(num_big, den_big);
    }
	
    fix_unsigned g = gcd(fix_abs(num), fix_abs(den));
    if (g == 0)
	return Zero;

    if (g != 1) {
	num = num / (long)g;
	den = den / (long)g;
    }
    
    if (den < 0) {
	den = -den;
	num = -num;
    }
    if (den == 1) return MakeFixInt(num);
    return new Fraction(*MakeFixInt(num), *MakeFixInt(den));
}

#ifdef CORAL
int Integer::hash(Binding *context = 0) const
#else
int Integer::hash() const
#endif
{ return IntToHash(S[0]); }
void Integer::do_delete() { delete this; }
const Integer& Integer::numerator() const { return *this; }
const Integer& Integer::denominator() const { return *One; }
int Integer::sign() const { return bsign(); }
const Integer * Integer::gcd(const Integer& y) const {return &::gcd(*this,y); }
const Integer * Integer::gcdFixInt(fix_int i) const
{
    SmallBigInt tmp(i);
    return &::gcd(*this, tmp);
}
const Numeric* Integer::add(const Numeric&x) const {return x.addBigInt(*this);}
const Numeric* Integer::sub(const Numeric&x) const {return x.subBigInt(*this);}
const Numeric* Integer::mul(const Numeric&x) const {return x.mulBigInt(*this);}
const Numeric* Integer::div(const Numeric&x) const {return x.divBigInt(*this);}

const Integer& power(const Integer& x, fix_unsigned apow)
{
    int maxsize =  ((x.integer_length() * apow) / BITS_PER_MP_LIMB) + 1;
    int in_len = maxsize;
    // _mpn_mul requires that the size of the result be
    // the sum of the input sizes.
    // But perhaps something more clever than this is possible.
    int out_len = 2*in_len;
    Integer *r = NewBigInt(out_len);
    Integer *work = NewBigInt(out_len);
    int negate = 0;
    Integer *b = NewBigInt(out_len);
    MPN_ZERO(b->L, in_len);
    MPN_COPY(b->L, x.L, x.len);
    if (x.S[x.len-1] < 0) {
	negate = apow & 1;
	_mpn_negate(b->L, b->L, x.len);
    }

    Integer *tmp;
    MPN_ZERO(&r->L[1], r->len-1); r->U[0] = 1; // r = 1;
    for (;;) {
	if (apow & 1) { // r *= b
//	    MPN_ZERO(work->L, in_len);
	    _mpn_mul(work->L, r->L, in_len, b->L, in_len);
	    tmp = r; r = work; work = tmp; // swap to avoid a copy
	}
	if ((apow >>= 1) == 0)
	    break;
	// b *= b;
//	MPN_ZERO(work->L, in_len);
	_mpn_mul(work->L, b->L, in_len, b->L, in_len);
	tmp = b; b = work; work = tmp; // swap to avoid a copy
    }
    work->do_delete();
    b->do_delete();
    MPN_ZERO(r->L+in_len, out_len-in_len);
    if (negate)
	_mpn_negate(r->L, r->L, r->len);
    return *r->simplify();
}

const Numeric * Integer::ipower(fix_int pow) const
{
    if (pow >= 0)
	return &power(*this, pow);
    else // "new Fraction" would do except when this is negative.
	return MakeRational(*One, power(*this, -pow));
}

const Numeric* power(const Numeric* x, const Numeric* y)
{
#if 0
    if (x == Zero) {
	if (y == Zero) return 1;
	return Zero;
    }
#endif
    if (x == One) return One;
    long y_l;
    if (y->getlong(&y_l))
	return x->ipower((fix_int)y_l);
    // return exp(y * ln(x));
    return NULL;
}

const Numeric * Rational::ipower(fix_int pow) const
{
    if (pow >= 0)
	return new Fraction(power(numerator(), pow),
			    power(denominator(),pow));
    else // MakeRational is only necessary in case numerator is negative
	return MakeRational(power(denominator(), -pow),
			    power(numerator(), -pow));
}

int compare(const Integer& x, const Integer& y)
{
    if (x.is_negative()) // x < 0
	if (!y.is_negative())
	    return -1;
	else { // Both are negative
	    int xl = x.big_len();
	    register mp_srcptr xp = x.L + xl;
	    while (xl > 0 && xp[-1] == ~0) xp--, xl--;
	    int yl = y.big_len();
	    register mp_srcptr yp = y.L + yl;
	    while (yl > 0 && yp[-1] == ~0) yp--, yl--;

	    if (xl != yl)
		return (xl < yl ? 1 : -1);
	    return _mpn_cmp(xp, xl, yp, yl);
	}
    else // x >= 0
	if (y.is_negative())
	    return 1;
	else // Both are non-negative
	    return _mpn_cmp(x.L, x.big_len(), y.L, y.big_len());
}

int compare(const Rational& x, const Rational& y)
{
    const Integer& x_num = x.numerator();
    const Integer& x_den = x.denominator();
    const Integer& y_num = y.numerator();
    const Integer& y_den = y.denominator();
    return ::compare(&y_den == One ? x_num : x_num * y_den,
		     &x_den == One ? y_num : y_num * x_den);
}

int Rational::compareRational(const Rational& x) const
{
    return ::compare(x, *this);
}

int Rational::compareFixInt(fix_int left) const
{
    FixInt left_int(left);
    Integer& y_den = denominator();
    return ::compare(&y_den==One ? *(const Integer*)&left_int : left_int*y_den,
		     numerator());
}

int Rational::is_exact() const { return 1;}

int Integer::compareRational(const Rational& x) const
{
    const Integer& x_den = x.denominator();
    if (&x_den == One)
	return ::compare(x.numerator(), *this);
    else
	return ::compare(x.numerator(), *this * x_den);
}

const RationalInfinity PosInfinity(1);
const RationalInfinity NegInfinity(-1);
const Real& RationalInfinity::rneg() const
{
    switch (val)  {
      case 1: return NegInfinity;
      case -1: return PosInfinity;
      default: return *(Real*)NULL;
    }
}
const Integer& RationalInfinity::numerator() const { return MkSmallInt(val);;}
const Integer& RationalInfinity::denominator() const { return *Zero; }
const Numeric * RationalInfinity::addFixInt(fix_int) const { return this; }
const Numeric * RationalInfinity::subFixInt(fix_int) const { return this; }
const Numeric * RationalInfinity::mulFixInt(fix_int) const { return this; }
const Numeric * RationalInfinity::divFixInt(fix_int) const { return this; }
const Numeric * RationalInfinity::addBigInt(Integer&) const { return this; }
const Numeric * RationalInfinity::subBigInt(Integer&) const { return this; }
const Numeric * RationalInfinity::mulBigInt(Integer&) const { return this; }
const Numeric * RationalInfinity::divBigInt(Integer&) const { return this; }

const FixInt* MakeFixInt(long i)
{
    register long j = i - LeastSmallInt;
    if ((unsigned long)j < CountSmallInt) return &SmallIntTable[j];
    return new FixInt(i);
}

#ifdef Q_ENV
void SmallInt::dumpPtr(CFile *cf) const
{
    cf->asm_stream() << "SMALL_INT(" << val << ")";
}
#endif

#ifdef Q_ENV
static void DoubleDumpProc(struct PrevDumped *dump, CFile *cf)
{
    cf->asm_stream() << "Double " << dump->name() << "("
	<< ((Double*)dump->addr())->val << ");\n";
}

void Double::dumpPtr(CFile *cf) const
{
    struct PrevDumped *dump = NewPendingDump(cf, this, &DoubleDumpProc, NULL);
    cf->aux_stream() << "extern Double " << dump->name() << ";\n";
    cf->asm_stream() << "&" << dump->name();
}
#endif


char NumChar[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int IntPut(
    register unsigned long x,
    char *buffer, /* points to the RIGHT OF the UNITS position of result */
    unsigned base) /* a number in the range 2..36 */
    /* returns number of significant digits */
  { register char *ptr = buffer;
    while (x != 0)
      { *--ptr = NumChar[x % base]; x /= base; }
    return buffer - ptr;
  }

void UIntPrint(
    unsigned long i,
    register FILE *f,
    int base) /* 2..36 */
{
#define IntPutBufLen 34
    char buffer[IntPutBufLen]; /* max size if binary */
    register char *b;
    register count;
    if (i == 0) { putc('0', f); return; }
    count = IntPut(i, buffer+IntPutBufLen, base);
    b = buffer+IntPutBufLen-count;
    while (--count >= 0) putc(*b++, f);
}

void SIntPrint(long i, FILE *f, int base)
{
    if (i < 0) { putc('-', f); i = -i; }
    if (base != 10 && print_readable)
	fprintf(f, "%dr", print_base);
    UIntPrint((unsigned long)i, f, base);
}

const Real& FixInt::rneg() const
{
    if (val == (int)0x80000000) { abort(); return *(const Real*)0; }
    return *MakeFixInt(-val);
}

const Numeric * FixInt::addFixInt(fix_int j) const
{
    fix_int sum = j + val;
    if (j >= 0)
	if (sum >= 0 || val <= 0) return MakeFixInt(sum);
	else {
	    Integer* result = NewBigInt(2);
	    result->S[0] = sum;
	    result->U[1] = 0;
	    return result;
	}
    else
	if (sum < 0 || val >= 0) return MakeFixInt(sum);
	else {
	    Integer* result = NewBigInt(2);
	    result->S[0] = sum;
	    result->S[1] = -1;
	    return result;
	}
}

const Numeric * FixInt::subFixInt(fix_int j) const
{
    fix_int diff = j - val;
    if (j >= 0)
	if (diff >= 0 || val >= 0)
	    return MakeFixInt(diff);
	else {
	    Integer* result = NewBigInt(2);
	    result->S[0] = diff;
	    result->S[1] = 0;
	    return result;
	}
    else
	if (val <= 0 || diff < 0)
	    return MakeFixInt(diff);
	else {
	    Integer* result = NewBigInt(2);
	    result->S[0] = diff;
	    result->S[1] = -1;
	    return result;
	}
}

const Numeric * FixInt::mulFixInt(fix_int j) const
{
    mp_limb x, y; int neg;
    if (j >= 0) neg = 0, x = j;
    else neg = 1, x = -j;
    if (val >= 0) y = val;
    else neg = 1-neg, y = -val;
    mp_limb prod[2];
#if 1
    umul_ppmm(prod[1], prod[0], x, y);
#else
    _mpn_mul(prod, &x, 1, &y, 1);
#endif
    // Does the result fit into a FixInt?
    if (prod[1] == 0) {
	if (!neg) {
	    j = (fix_int)prod[0];
	    if (j >= 0) return MakeFixInt(j);
	}
	else {
	    j = -(fix_int)prod[0];
	    if (j <= 0) return MakeFixInt(j);
	}
    }
    Integer& result = *NewBigInt(2);
    if (neg) {
	// -result == ~result + 1
	_mpn_negate(result.L, (mp_ptr)prod, 2);
    } else {
	result.U[0] = prod[0];
	result.U[1] = prod[1];
    }
    return &result;
}

int FixInt::compare(const Root& other) const
{
    const Numeric *num = other.numeric();
    if (!num) return -2;
    return num->compareFixInt(val);
}

int Double::compare(const Root& other) const
{
    const Numeric *num = other.numeric();
    if (!num) return -2;
    return num->compareDouble(val);
}

int Rational::compare(const Root& other) const
{
    const Numeric *num = other.numeric();
    if (!num) return -2;
    return num->compareRational(*this);
}

double Rational::as_double() const
{
    // FIXME:  This can overflow needlessly!
    double dnum = numerator().as_double();
    double dden = denominator().as_double();
    return dnum / dden;    
}

const Integer * FixInt::gcdFixInt(fix_int arg) const
{
    unsigned long g = ::gcd(abs(val), abs(arg));
    if ((long)g < 0) { NOT_IMPLEMENTED(); }
    return MakeFixInt(g);
}

const Integer& FixInt::operator<<(int count) const
{
    if (count <= 0) // shift right
	if (count < -(BITS_PER_MP_LIMB-1))
	    if (ival() >= 0) return *Zero;
	    else return *MinusOne;
	else
	    return *MakeFixInt(ival() >> -count);

    // Shift left
    if (ival() > 0) {
	if (count < (BITS_PER_MP_LIMB-1)
	    && ival() < (1 << ((BITS_PER_MP_LIMB-1) - count)))
	    return *MakeFixInt(ival() << count);
    } else {
	if (ival() == 0) return *Zero;
	if (count <= (BITS_PER_MP_LIMB-1)
	    && (~ival()) < (1 << ((BITS_PER_MP_LIMB-1) - count)))
	    return *MakeFixInt(ival() << count);
    }
    return this->Integer::operator<<(count);
}

int Integer::compareFixInt(fix_int left) const
{
    FixInt left_int(left);
    return ::compare(left_int, *this);
}

int FixInt::compareFixInt(fix_int left) const
{ return left > val ? 1 : left==val ? 0 : -1; } 

int FixInt::getlong(long *i) const { *i = val; return 1; }

int Double::compareFixInt(fix_int other) const
{   double left = (double)other;
    return left > val ? 1 : left==val ? 0 : -1;
} 

int FixInt::compareDouble(double left) const
{   double right = (double)val;
    return left > right ? 1 : left==right ? 0 : -1;
} 

#ifdef Q_ENV
const StringC *FixInt::asString(int format=0) const
{
    char buf[20];
    sprintf(buf, "%d", val);
    return NewString(strlen(buf), buf);
}

static void FixIntDumpProc(struct PrevDumped *dump, CFile *cf)
{
    cf->asm_stream() << "FixInt " << dump->name() << "("
	<< ((FixInt*)dump->addr())->val << ");\n";
}

void FixInt::dumpPtr(CFile *cf) const
{
    struct PrevDumped *dump = NewPendingDump(cf, this, &FixIntDumpProc, NULL);
    cf->aux_stream() << "extern FixInt " << dump->name() << ";\n";
    cf->asm_stream() << "" << dump->name();
}
#endif

const Numeric * SmallInt::addFixInt(fix_int x) const
{
    // Use unsigned arithmetric, because that should not raise an
    // exception on overflow. (Does ANSI mandate this?)
    unsigned index = (unsigned)x + (unsigned)val - (unsigned)LeastSmallInt;
    if (index < CountSmallInt) return &SmallIntTable[index];
    return FixInt::addFixInt(x);
}

const Numeric * SmallInt::subFixInt(fix_int x) const
{
    // Use unsigned arithmetric, because that should not raise an
    // exception on overflow. (Does ANSI mandate this?)
    unsigned index = (unsigned)x - (unsigned)val - (unsigned)LeastSmallInt;
    if (index < CountSmallInt) return &SmallIntTable[index];
    return FixInt::subFixInt(x);
}

const Numeric * SmallInt::mulFixInt(fix_int x) const
{
    if (x + (unsigned)LeastSmallInt < CountSmallInt) {
	unsigned index = (unsigned)x * (unsigned)val - (unsigned)LeastSmallInt;
	if (index < CountSmallInt) return &SmallIntTable[index];
    }
    return FixInt::mulFixInt(x);
}


int Double::compareDouble(double left) const
{ return left > val ? 1 : left==val ? 0 : -1; } 

const Real& Fraction::rneg() const
{
    return *MakeRational(*(const Integer*)&num.rneg(), den);
}

const Numeric * Fraction::addFixInt(fix_int i) const
{
    return MakeRational(num + i * den, den);
}

const Numeric * Fraction::subFixInt(fix_int i) const
{
    return MakeRational(i * den - num, den);
}

const Numeric * Fraction::mulFixInt(fix_int i) const
{
    return MakeRational(i * num, den);
}

const Numeric * Fraction::divFixInt(fix_int i) const
{
    return MakeRational(i * den, num);
}

const Numeric * Fraction::addBigInt(BigInt& big) const
{
    return MakeRational(
	num + *(Integer*)den.mulBigInt(big),
	den);
}

const Numeric * Fraction::subBigInt(BigInt &x) const
{
    return MakeRational(
	*(Integer*)den.mulBigInt(x)->sub(num),
	den);
}

const Numeric * Fraction::mulBigInt(BigInt &x) const
{
    return MakeRational(
	*(Integer*)num.mulBigInt(x),
	den);
}

const Numeric * Fraction::divBigInt(BigInt &x) const
{
    return MakeRational(
	*(Integer*)den.mulBigInt(x),
	num);
}

Integer::Integer() { }

// Extract the bits in byte (position, size), and return as an integer.
// If sign_extend is 0, the byte is taken to be unsigned; otherwise signed.
// The result is multiplied by 2**dest_pos. // dest_pos NOT IMPLEMENTED!

const Integer&
ExtendedInteger::byte(int size, int position, int sign_extend, int dest_pos) const
{
    if (finite())
	return ((const Integer*)this)->byte(size, position, sign_extend, dest_pos);
    // Extract from +Infinity or -Infinity.
    // Think  of -Infinity == 2r10.....0.
    // Think  of +Infinity == 2r01.....1.
    if (is_negative())
	return *Zero;
    if (dest_pos)
	abort();
    if (sign_extend)
	return *MinusOne;
    else if (size <= 1)
	return MkSmallInt(size);
    // Return 2r1....1 (size bits).
    Integer* dest = NewBigInt(DigitsNeeded(size + 1));
    fix_unsigned *ptr = dest->U;
    while (size >= BITS_PER_MP_LIMB) size -= BITS_PER_MP_LIMB, *ptr++ = ~0;
    *ptr = ~(~0 << size);
    return *dest;
}

const Integer& Integer::byte(int size, int position, int sign_extend, int dest_pos) const
{
    if (dest_pos != 0)
	abort();
    int low_word = BITS_TO_LIMBS(position);
    int high_word = BITS_TO_LIMBS(size+position);
    fix_unsigned mask = ~((~0) << ((size+position) & (BITS_PER_MP_LIMB-1)));
    // Add extra 1 bits at end.
    int extra_ones = 0;
    if (high_word > len) {
	if (is_negative() && !sign_extend)
	    extra_ones = size+position - len*BITS_PER_MP_LIMB;
	high_word = len - 1;
	mask = ~0;
    }
    else if (mask == 0)
	mask = ~0, high_word--;
    int make_negative = 0;
    if (sign_extend) {
	// Check if negative.
	if (size == 0) // Semi-valid.
	    return bit(position) ? *MinusOne : *Zero;
	// Check for high-order bit of byte.
	if (U[high_word] & (1 << ((size + position - 1) & (BITS_PER_MP_LIMB-1))))
	    make_negative = 1;
    }
    int ilen;
    for (;;) {
	// Set ilen to number of interesting bits in U[high_word].
	// FIXME: Not correct for negative fields!
	ilen = ::integer_length(U[high_word] & mask);
	if (ilen) break;
	if (high_word == low_word)
	    return *Zero;
	high_word--;
	mask = ~0;
    }
    // One extra bit is needed if high-order bit is set, unless sign-extending.
    int bits_needed = high_word * BITS_PER_MP_LIMB
	+ ilen - position + extra_ones + !sign_extend;
    register int dest_length = DigitsNeeded(bits_needed);
    Integer* dest;
    _mp_limb *dest_words;
    _mp_limb short_dest;
    if (dest_length <= 1) {
	dest_words = &short_dest;
    }
    else {
	dest = NewBigInt(dest_length);
	dest_words = dest->L;
    }
    int shift = position & (BITS_PER_MP_LIMB-1);
    _mp_limb old_word = U[high_word] & mask;
    if (extra_ones) {
	int i = shift >= extra_ones ? extra_ones : shift; // minimum.
	old_word = ~(~0 << i);
	extra_ones -= i;
	_mp_limb *ptr = &dest_words[high_word-low_word];
	while (extra_ones >= BITS_PER_MP_LIMB)
	    *ptr++ = ~0, extra_ones -= BITS_PER_MP_LIMB;
    }
    _mpn_rshiftci (dest_words, L+low_word, high_word-low_word+1, shift, old_word);
    // Set mask to 1 for the (size mod BITS_PER_MP_LIMB) lowest-order bits.
    mask = ~(~0 << (size & (BITS_PER_MP_LIMB-1)));
    if (make_negative)
	dest_words[dest_length-1] |= ~mask;
    else
	dest_words[dest_length-1] &= mask;
    if (dest_length > 1)
	return *dest;
    else
	return *MakeFixInt((fix_int)short_dest);
}

int Integer::bit(fix_unsigned i) const
{
    fix_unsigned w = BITS_TO_LIMBS(i);
    if (w >= len) return is_negative();
    return (U[w] >> (i & (BITS_PER_MP_LIMB-1))) & 1;
};

// Create an integer that is the same as this, except
// that the bits specified by (size, pos) are replaced by bits from v.
// bit(i) in result is:
//     (i < pos || i >= size + pos) ? this->bit(i) : v.bit(i-pos+v_pos)

const Integer&
Integer::deposit_byte(Integer& v, int size, int pos, int v_pos) const
{
    if (size <= 0)
	return *this;

    // First figure out how big the result is.
    int must_simplify = 0;
    int this_len = integer_length();
    int result_size; // in bits.
    if (this_len > pos+size) result_size = this_len; // Exact
    else result_size = pos+size, must_simplify = 1; // Conservative
    // Add 1 bit for sign and (BITS_PER_MP_LIMB-1) to round up.
    int dest_len = ((unsigned)result_size+BITS_PER_MP_LIMB) / BITS_PER_MP_LIMB;

    // Allocate the result (unless it fits in a FixInt).
    Integer *dest;
    mp_ptr dest_limbs;
    mp_limb short_dest;
    if (dest_len <= 1) {
	if (dest_len <= 0) dest_len = 1;
	dest_limbs = &short_dest;
    }
    else {
	dest = NewBigInt(dest_len);
	dest_limbs = dest->L;
    }
    register int i;

    // Next copy 'this' to the destination.
    for (i = dest_len > len ? len : dest_len; --i >= 0; )
	dest_limbs[i] = L[i];
    i = dest_len - big_len();
    if (i > 0) { // Sign-extend if dest is longer than this.
	mp_ptr ptr = dest_limbs + big_len();
	mp_limb extra = is_negative() ? ~0 : 0;
	while (--i >= 0) *ptr++ = extra;
    }

    mp_ptr dst_ptr = dest_limbs + (pos / BITS_PER_MP_LIMB);
    int lo_dest = pos / BITS_PER_MP_LIMB;
    int hi_dest = (pos+size) / BITS_PER_MP_LIMB;

#define GET_SRC_WORD() \
    src0 = src1; \
    src1 = src_ptr < src_limit ? src1 = *src_ptr++ : src_extend; \
    src_val = (src0 >> (BITS_PER_MP_LIMB-shift)) | (src1 << shift);

    // Now mask in the value from v.
    mp_limb src0;  // Previous sources word.
    mp_limb src1 = 0;  // Current sources word.
    mp_limb src_val; // Source word shifted to match dest alignment.
    mp_limb src_extend = v.is_negative() ? ~0 : 0;
    mp_ptr src_limit = v.L + v.big_len();
    int shift = (pos - v_pos) & (BITS_PER_MP_LIMB-1);
    mp_ptr src_ptr;
    if (v_pos >= (v.big_len() * BITS_PER_MP_LIMB)) {
	src_ptr = src_limit;
	src1 = src0 = src_extend;
    }
    else {
	v_pos -= pos & (BITS_PER_MP_LIMB-1);
	if (v_pos <= 0) {
	    src1 = 0;
	    src_ptr = v.L;
	}
	else {
	    src_ptr = v.L + (v_pos / BITS_PER_MP_LIMB);
	    GET_SRC_WORD();
	}
	GET_SRC_WORD();
    }

    mp_limb mask = ~0 << (pos & (BITS_PER_MP_LIMB-1));
    mp_limb end_mask = ~(~0 << ((pos+size) & (BITS_PER_MP_LIMB-1)));
    if (lo_dest == hi_dest)
	mask &= end_mask;
    *dst_ptr = (*dst_ptr & ~mask) | (src_val & mask);
    dst_ptr++;
    i = size - (BITS_PER_MP_LIMB - (pos & (BITS_PER_MP_LIMB-1)));
    while (i >= BITS_PER_MP_LIMB) {
	GET_SRC_WORD();
	*dst_ptr++ = src_val;
	i -= BITS_PER_MP_LIMB;
    }
    if (i > 0) {
	GET_SRC_WORD();
	*dst_ptr = (*dst_ptr & ~end_mask) | (src_val & end_mask);
    }
    if (dest_len <= 1)
	return *MakeFixInt((fix_int)short_dest);
    else if (must_simplify)
	return *dest->simplify();
    else
	return *dest;
}

const Integer& Integer::operator<<(int count) const
{
    if (count == 0) return *this;
    else if (count > 0) { // Shift left
	int shiftBigDigits = BITS_TO_LIMBS(count);
	int shiftBits = count & (BITS_PER_MP_LIMB-1);
	Integer* dest = NewBigInt(big_len() + shiftBigDigits + 1);
	MPN_ZERO(dest->L, shiftBigDigits);
	MPN_COPY(dest->L + shiftBigDigits, L, big_len());
	dest->U[dest->big_len()] = is_negative() ? ~0 : 0;
	_mpn_lshift(dest->L + shiftBigDigits, dest->L + shiftBigDigits,
		    big_len() + 1, shiftBits);
	return *dest->simplify();
    } else { // Shift right
#if 0
	return byte((len * BITS_PER_MP_LIMB) - count, count, 1);
#else
	count = -count;
	int shiftBigDigits = BITS_TO_LIMBS(count);
	int shiftBits = count & (BITS_PER_MP_LIMB-1);
	int d_len = big_len() - shiftBigDigits;
	int is_neg = is_negative();
	if (d_len <= 0)
	    if (is_neg) return *MinusOne;
	    else return *Zero;
	Integer* dest = (Integer*)NewBigInt(d_len);
	_mpn_rshiftci(dest->L, L+shiftBigDigits, d_len,shiftBits, -is_neg);
	return *dest->simplify();
#endif
    }
}

const Integer& Integer::operator-() const
{
    // -(*this) == ~(*this) + 1
    int new_len = len;
    if (L[len-1] == (mp_limb)(1 << (BITS_PER_MP_LIMB-1))) new_len++;
    BigInt *dest = NewBigInt(new_len);
    register mp_srcptr src_p = L;
    register mp_ptr dst_p = dest->L;
    for (register int i = len; --i >= 0; )
	*dst_p++ = ~*src_p++;
    if (new_len > (int)len)
	*dst_p++ = ~0;
    _mpn_increment(dest->L, dest->L, dest->len);
    return *dest->simplify();
}

const Real& Integer::rneg() const
{
    return -(*this);
}

const Numeric * Integer::addFixInt(fix_int i) const
{return &(*this + i);}

const Numeric * Integer::subFixInt(fix_int i) const
{
    SmallBigInt tmp(i);
    return subBigInt(tmp);
}

const Numeric * Integer::mulFixInt(fix_int i) const
{
    int negate = 0;
    mp_limb ui;
    if (i > 0) ui = i;
    else if (i == 0) return Zero;
    else ui = -i, negate = 1;
    int numDigits = _mpn_count_nonzero(L, len);
    if (numDigits == 0) return Zero;
    Integer* result = NewBigInt(numDigits + 1);
    register mp_ptr src = L;
    if (is_negative()) {
	mp_ptr abs_this =
	    (mp_ptr)alloca(numDigits * sizeof(mp_limb));
	_mpn_negate(abs_this, src, numDigits);
	src = abs_this;
	negate = 1-negate;
    }
    _mpn_mul(result->L, src, numDigits, &ui, 1);
    if (negate)
	_mpn_negate(result->L, result->L, numDigits+1);
    return result->simplify();
}

// True if arg1 and arg2 have 1 bits in common.
// Implements Common Lisp: (logtest xy) == (not (zerop (logand x y)))

int bit_test(const Integer& arg1, const Integer& arg2)
{
    const Integer *x, *y;
    if (arg1.len > arg2.len) x = &arg1, y = &arg2;
    else x = &arg2, y = &arg1;
    // x is longer than y.
    register mp_srcptr x_ptr = x->L, y_ptr = y->L;
    register int i = y->len;
    int y_neg = i > 0 && y->S[i-1] < 0;

    while (--i >= 0)
	if (*x_ptr++ & *y_ptr++)
	    return 1;
    if (!y_neg) 
	return 0;

    for (i = x->len - y->len; --i >= 0; )
	if (*x_ptr++)
	    return 1;

    return 0;
}

const Integer& operator+(const Integer &X, const Integer& Y)
{
    const BigInt *x, *y;

    if (X.len > Y.len) x = &X, y = &Y;
    else x = &Y, y = &X;
    // x is longer than y.
    int x_neg = x->is_negative();
    int y_neg = y->is_negative();
    // Allocate one extra digit if x and y have the same sign.
    int extra_digit = x_neg==y_neg;  // Overly conservative.
    BigInt *dest = NewBigInt(x->len + extra_digit);
    if (y_neg) { // y is negative
        MPN_COPY(dest->L, y->L, y->len);
        register mp_ptr d_ptr = dest->L + y->len;
        for (register int i = x->len - y->len; --i >= 0; ) *d_ptr++ = ~0;
        _mpn_add(dest->L, dest->L, x->len, x->L, x->len);
        if (x_neg) dest->L[x->len] = ~0;
    } else { // y is non-negative
        _mpn_add (dest->L, x->L, x->len, y->L, y->len);
        if (!x_neg) dest->L[x->len] = 0;
    }
    return *dest->simplify();
}

const Integer& operator+(const Integer &X, fix_int Y)
{
    const BigInt *x = &X;
    // x is longer than y.
    int x_neg = x->is_negative();
    int y_neg = Y < 0;
    // Allocate one extra digit if x and y have the same sign.
    BigInt *dest = NewBigInt(x->len + (x_neg==y_neg));
    if (y_neg) { // y is negative
	MPN_COPY(dest->L, (mp_srcptr)&Y, 1);
	register mp_ptr d_ptr = dest->L + 1;
	for (register int i = x->len - 1; --i >= 0; ) *d_ptr++ = ~0;
	_mpn_add(dest->L, dest->L, x->len, x->L, x->len);
	if (x_neg) dest->L[x->len] = ~0;
    } else { // y is non-negative
	_mpn_add (dest->L, x->L, x->len, (mp_ptr)&Y, 1);
	if (!x_neg) dest->L[x->len] = 0;
    }
    return *dest->simplify();
}

const Integer& operator-(const Integer &X, const Integer& Y)
{
    // Perhaps not the most clever or optimal algorithm...
    const Integer& Yneg = -Y;
    const Integer& sum = X + Yneg;
    ((Integer*)&Yneg)->do_delete();
    return sum;
}

const Rational& operator-(const Rational &x, const Rational& y)
{
    const Integer& x_num = x.numerator();
    const Integer& x_den = x.denominator();
    const Integer& y_num = y.numerator();
    const Integer& y_den = y.denominator();
    if (&x_den == &y_den)
	return *MakeRational(x_num - y_num, x_den);
    else if (&x_den == One)
	return *MakeRational(y_den * x_num - y_num, y_den);
    else if (&y_den == One)
	return *MakeRational(x_num - y_num * x_den, x_den);
    else
	return *MakeRational(y_den * x_num - y_num * x_den, x_den * y_den);
}

const Numeric * Integer::addBigInt(BigInt& arg) const
{
    return &(arg + *this);
}
const Numeric * Integer::subBigInt(BigInt& arg) const
{
    return &(arg - *this);
}

const Integer& operator*(const Integer &X, const Integer& Y)
{
    int x_neg = X.is_negative();
    int y_neg = Y.is_negative();
    const Integer* x = &(x_neg ? -X : X);
    const Integer* y = &(y_neg ? -Y : Y);
    mp_srcptr m = x->L;
    mp_srcptr n = y->L;
    int ml = _mpn_count_nonzero(m, x->big_len());
    int nl = _mpn_count_nonzero(n, y->big_len());
    if (nl > ml) { // swap m and n
	mp_srcptr t = m; m = n; n = t;
	int tl = ml; ml = nl; nl = tl;
    }
    Integer* dest = NewBigInt(ml+nl);
    _mpn_mul (dest->L, m, ml, n, nl);
    if (x_neg) ((Integer*)x)->do_delete();
    if (y_neg) ((Integer*)y)->do_delete();
    if (x_neg != y_neg) {
	const BigInt& negated = -(*dest);
	dest->do_delete();
	return negated;
    }
    return *dest->simplify();
}

const Numeric * Integer::mulBigInt(BigInt& arg) const
{
    return &(arg * *this);
}

const Numeric * Integer::divBigInt(BigInt& x)  const
{
    return MakeRational(x, *this);
}

#define Bool0(X,Y) 0
#define Bool1(X,Y) X & Y
#define Bool2(X,Y) X & ~Y
#define Bool3(X,Y) X
#define Bool4(X,Y) ~X & Y
#define Bool5(X,Y) Y
#define Bool6(X,Y) X^Y
#define Bool7(X,Y) X|Y
#define Bool8(X,Y) ~(X|Y)
#define Bool9(X,Y) ~(X^Y)
#define Bool10(X,Y) ~Y
#define Bool11(X,Y) X|~Y
#define Bool12(X,Y) ~X
#define Bool13(X,Y) ~X|Y
#define Bool14(X,Y) ~(X&Y)
#define Bool15(X,Y) ~0

static char swap_ops[16] = {
    // swap_ops[op](x,y) == op(y,x)
    // Same as swaping two middle bits of op-code
    0, 1, 4, 5,
    2, 3, 6, 7,
    8, 9,12,13,
   10,11,14,15};

const Integer& Integer::boolean(const Integer& arg, int op) const
{
    switch (op) {
      case 0: return *Zero;
      case 3: return arg;
      case 5: return *this;
      case 15: return *MinusOne;
    }
    const Integer *x, *y;
    if (len > arg.len) x = this, y = &arg;
    else x = &arg, y = this, op = swap_ops[op];
    // x is longer than y.
    register mp_srcptr x_ptr = x->L, y_ptr = y->L;
    BigInt *dest = NewBigInt(x->len);
    register mp_ptr d_ptr = dest->L;
    register int i = y->len;
    int y_neg = i > 0 && y->S[i-1] < 0;
    switch (op) {
      case 0:
	while (--i >= 0) *d_ptr++ = 0;
	goto clear_rest;
      case 1:
	while (--i >= 0) *d_ptr++ = *x_ptr++ & *y_ptr++;
	if (y_neg) goto copy_rest; else goto clear_rest;
      case 2:
	while (--i >= 0) *d_ptr++ = *x_ptr++ & ~ *y_ptr++;
	if (y_neg) goto clear_rest; else goto copy_rest;
      case 3:
	while (--i >= 0) *d_ptr++ = *y_ptr++;
	if (y_neg) goto set_rest; else goto clear_rest;
      case 4:
	while (--i >= 0) *d_ptr++ = ~ *x_ptr++ & *y_ptr++;
	if (y_neg) goto invert_rest; else goto clear_rest;
      case 5:
	while (--i >= 0) *d_ptr++ = *x_ptr++;
	goto copy_rest;
      case 6:
	while (--i >= 0) *d_ptr++ = *x_ptr++ ^ *y_ptr++;
	if (y_neg) goto invert_rest; else goto copy_rest;
      case 7:
	while (--i >= 0) *d_ptr++ = *x_ptr++ | *y_ptr++;
	if (y_neg) goto set_rest; else goto copy_rest;
      case 8:
	while (--i >= 0) *d_ptr++ = ~ (*x_ptr++ | *y_ptr++);
	if (y_neg) goto clear_rest; else goto invert_rest;
      case 9:
	while (--i >= 0) *d_ptr++ = ~ (*x_ptr++ ^ *y_ptr++);
	if (y_neg) goto copy_rest; else goto invert_rest;
      case 10:
	while (--i >= 0) *d_ptr++ = ~ *y_ptr++;
	if (y_neg) goto clear_rest; else goto set_rest;
      case 11:
	while (--i >= 0) *d_ptr++ = *x_ptr++ | ~ *y_ptr++;
	if (y_neg) goto copy_rest; else goto set_rest;
      case 12:
	while (--i >= 0) *d_ptr++ = ~ *x_ptr++;
	goto invert_rest;
      case 13:
	while (--i >= 0) *d_ptr++ = ~ *x_ptr++ | *y_ptr++;
	if (y_neg) goto set_rest; else goto invert_rest;
      case 14:
	while (--i >= 0) *d_ptr++ = ~ (*x_ptr++ & *y_ptr++);
	if (y_neg) goto invert_rest; else goto set_rest;
      case 15:
	while (--i >= 0) *d_ptr++ = ~0;
	goto set_rest;
    }

  clear_rest:
    for (i = x->len - y->len; --i >= 0; ) *d_ptr++ = 0;
    goto done;
  set_rest:
    for (i = x->len - y->len; --i >= 0; ) *d_ptr++ = ~0;
    goto done;
  copy_rest:
    for (i = x->len - y->len; --i >= 0; ) *d_ptr++ = *x_ptr++;
    goto done;
  invert_rest:
    for (i = x->len - y->len; --i >= 0; ) *d_ptr++ = ~*x_ptr++;
    goto done;

  done:
    return *dest->simplify();
}

#if 0
double IntRationToFloat(Integer& num, Integer& den)
{
// Algorithm taken from CMU Common Lisp.
// Given a Ratio with arbitrarily large numerator and denominator, convert
// it to a float in the specified Format without causing spurious floating
// overflows.  What we do is discard all of the numerator and denominator
// except for the format's precision + guard bits.  We float these
// integers, do the floating division, and then scaled the result accordingly.

//    The use of digit-size as the number of guard bits is relatively
// arbitrary.  We want to keep around some extra bits so that we rarely do
// round-to-even when there were low bits that could have caused us to round
// up.  We really only need to discard enough bits to ensure that floating the
// result doesn't overflow.
    int bits_to_keep = float_format_digits(format) + digit-size;
    int num_len = num.integer_length();
    int num_shift = bits_to_keep - num_len;
    if (num_shift > 0)
	num_shift = 0;
    int den_len = den.integer_length();
    int den_shift = bits_to_keep - den_len;
    if (den_shift > 0)
	den_shift = 0;
    (multiple-value-bind
        (decoded exp sign)
        (decode-float (/ (coerce (ash num num-shift) format)
                         (coerce (ash den den-shift) format)))
     (* sign (scale-float decoded (+ exp (- num-shift) den-shift))))))
}
#endif

const Integer * Integer::simplify()
{
    int new_len = BigDigitsNeeded(U, len);
    if (new_len > 1)
	if (new_len < len) {
#if 1
	    Integer *new_int = (Integer*)
		realloc((void*)this,
			sizeof(Integer) + (new_len-1)*sizeof(fix_int));
	    new_int->len = new_len;
	    return (const Integer*)new_int;
#else
	    const Integer *new_int = NewBigInt(new_len, U);
	    do_delete();
	    return this;
#endif
	}
	else
	    return (const Integer*)this;
    fix_int i = S[0];
    register long j = i - LeastSmallInt;
    if ((unsigned long)j < CountSmallInt) {
	do_delete();
	return &SmallIntTable[j];
    }
    if (len > 1) {
	do_delete();
	return new FixInt(i);
    }
    ((FixInt*)this)->FixInt::FixInt(i);
    return this;
}

const Integer *ConvertInteger(const Root *num)
{
    const Numeric *n = num->numeric();
    if (n != NULL) return n->integer();
    return NULL;
}
Integer *ConvertInteger(Root *num)
{
    Numeric *n = (Numeric*)num->value()->numeric();
    if (n != NULL) return n->integer();
    return NULL;
}

const Real *ConvertReal(const Root *num)
{
    const Numeric *n = num->numeric();
    if (n == NULL) return NULL;
    return n->real();
}
Real *ConvertReal(Root *num)
{
    Numeric *n = num->numeric();
    if (n == NULL) return NULL;
    return (Real*)n->real();
}

extern "C" Double * NewDouble(double d) { return new Double(d); }

// The remaining routines and definitions are taken (and modified) from c/bz.c 
// in the DEC/PRL BigNum package.
/* Copyright     Digital Equipment Corporation & INRIA     1988, 1989 */
/* Modified for C++ by Per Bothner, Spetember 1990. */

/* constants used by BzToString() and BzFromString() */
#define M_LN2	                0.69314718055994530942
#define M_LN10	                2.30258509299404568402
static double BzLog [] = 
{
    0, 
    0, 			/* log (1) */
    M_LN2,  		/* log (2) */
    1.098612, 		/* log (3) */
    1.386294, 		/* log (4) */
    1.609438, 		/* log (5) */
    1.791759, 		/* log (6) */
    1.945910, 		/* log (7) */
    2.079442, 		/* log (8) */
    2.197225, 		/* log (9) */
    M_LN10, 		/* log (10) */
    2.397895, 		/* log (11) */
    2.484907, 		/* log (12) */
    2.564949, 		/* log (13) */
    2.639057, 		/* log (14) */
    2.708050, 		/* log (15) */
    2.772588, 		/* log (16) */
};    
#define CTOI(c)			(c >= '0' && c <= '9' ? c - '0' :\
                                 c >= 'a' && c <= 'z' ? c - 'a' + 10:\
                                 c >= 'A' && c <= 'Z' ? c - 'A' + 10:\
                                 0)

// This is extern "C", so it can be called by lex.

const Integer *StrToInt(const char *str, int base, int str_len /* = -1 */)
{
    if (str_len == -1) {
	str_len = strlen(str);
	if ((unsigned)str_len <= 2 * sizeof(fix_int) && base == 10)
	    return MakeFixInt(atoi(str));  // Optimization
    }
    const char *limit = str + str_len;
/*
 * Creates a BigZ whose value is represented by "string" in the
 * specified base.  The "string" may contain leading spaces,
 * followed by an optional sign, followed by a series of digits.
 * Assumes 2 <= base <= 36.
 * When called from C, only the first 2 arguments are passed.
 */

    Integer 	*z, *p, *t;
    int 	zl;
    int neg = 0;
    mp_limb base_limb = base;

    
    /* Throw away any initial space */
    while (str < limit && *str == ' ') str++;
    if (str >= limit)
	return Zero;
    if (*str == '-')
	neg = 1, str++;
    else if (*str == '+')
	str++;
    while (str < limit && *str == ' ') str++;
    
    /* Allocate BigNums */
    /* Add 1 extra bit for sign bit. -PB */
    zl = (int)((1 + (limit-str) * BzLog[base]) / (BzLog[2]*BITS_PER_MP_LIMB)) + 1;

    z = NewBigInt(zl + 1);
    MPN_ZERO(z->L, zl);
    p = NewBigInt(zl + 1);

    if (!z || !p) 
        return (NULL);
    
    /* Multiply in the digits of the string, one at a time */
    for (;  str < limit;  str++)
    {
	mp_limb digit = CTOI (*str);
	_mpn_mul(p->L, z->L, zl, &base_limb, 1);
	_mpn_add(p->L, p->L, zl, &digit, 1);
	/* exchange z and p (to avoid a copy) */
	t = p,  p = z,  z = t;
    }
    
    /* Free temporary BigNums */
    p->do_delete();

    /* Set sign of result */
    if (neg) {
	_mpn_invert(z->L, z->L, zl);
	z->U[zl] = ~0;
	_mpn_increment(z->L, z->L, z->len);
    }
    else
	z->U[zl] = 0;
    return z->simplify();
}

/* Modified from BzToString() */

extern "C" char *
mpx_get_str (char *str, int base, mp_ptr mptr, mp_size msize,
	     int negative, int can_trash);

char * BigToString (const Integer* z, int base)

/*
 * Returns a pointer to a string that represents Z in the specified base.
 */

{
    mp_ptr src;
    char *result;
    if (z->len == 1 && z->L[0] == 0)
	return strdup("0");
    int negative = z->is_negative();
    if (negative) {
	src = (mp_ptr) alloca(z->len * BYTES_PER_MP_LIMB);
	_mpn_negate(src, z->L, z->len);
    }
    else
	src = z->L;
    result = mpx_get_str (NULL, base, src, z->len, negative, negative);
    return result;
}
#if 0
double Integer::as_double() const
{
    // Algorithm from CMU Common Lisp bignum-to-float (bignum.lisp).
    int plusp = ...;
    int x = abs this;
    int len = integer_length();
    int digits = float_format_digits?();
#defien digit_size 32
    int keep = digits + digit_size;
    int shift = keep - len;
}
#endif
