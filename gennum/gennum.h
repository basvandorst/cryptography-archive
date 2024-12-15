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

#ifndef GENNUM_H
#define GENNUM_H
#ifdef __GNUG__
#pragma interface
#endif
#include "genob.h"

#define INLINE inline
#define BigInt Integer

class Integer; class ExtendedInteger; class SmallInt; class FixInt;
class Fraction; class Real; class Rational; class Complex; class Double;
class CFile; class StringC; class ostream;

#ifndef DECLARE_MEMBERS
#define DECLARE_MEMBERS(name) public:
#endif
#ifndef DECLARE_MEMBERS_STATIC
#define DECLARE_MEMBERS_STATIC(name1, name2) public:
#endif

class Numeric : public Root 
{
    DECLARE_MEMBERS(Numeric)
 public:
#ifdef Q_ENV
    virtual long magic() const;
#endif
    virtual void printon(ostream&) const;
    virtual Numeric * numeric() { return this; }
    inline const Numeric * numeric() const { return this; }
    virtual const Numeric& neg() const = 0;
    virtual int sign() const = 0;
    virtual const Numeric * addFixInt(fix_int) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const;
    virtual const Numeric * divFixInt(fix_int) const;
    virtual const Numeric * addBigInt(Integer&) const;
    virtual const Numeric * subBigInt(Integer&) const;
    virtual const Numeric * mulBigInt(Integer&) const;
    virtual const Numeric * divBigInt(Integer&) const;
    virtual const Numeric * addDouble(double) const;
    virtual const Numeric * subDouble(double) const;
    virtual const Numeric * mulDouble(double) const;
    virtual const Numeric * divDouble(double) const;
    virtual const Numeric * add(const Numeric&y) const {return y.addr(*this);}
    virtual const Numeric * sub(const Numeric&y) const {return y.subr(*this);}
    virtual const Numeric * mul(const Numeric&) const;
    virtual const Numeric * div(const Numeric&) const;
    virtual const Numeric * addr(const Numeric&) const { return NULL; };
    virtual const Numeric * subr(const Numeric&) const { return NULL; };
    virtual const Numeric * mulr(const Numeric&) const { return NULL; };
    virtual const Numeric * divr(const Numeric&) const { return NULL; };
    virtual const Numeric * ipower(fix_int) const;
    virtual int compareRational(const Rational& x) const;
    virtual int compareDouble(double) const;
    virtual int compareFixInt(fix_int) const;
    virtual int getlong(long *i) const;
    virtual const Complex * complex() const { return NULL; }
    virtual const Real * real() const { return NULL; }
    virtual const Integer * integer() const { return NULL; }
    virtual const Rational * rational() const { return NULL; }
    virtual is_exact() const = 0;
    virtual Numeric * as_exact();
    virtual Numeric * as_inexact();
};

class NotANumber : public Numeric {
  protected:
    int kind;
  public:
    NotANumber(int k) { kind = k;}
    virtual void printon(ostream&) const;
    virtual const Numeric& neg() const;
    virtual int sign() const;
    virtual is_exact() const;
};

class Complex : public Numeric {
  public:
    virtual const Numeric& neg() const = 0;
    virtual int sign() const = 0;
    virtual const Real& realPart() const = 0;
    virtual const Real& imagPart() const = 0;
    virtual is_exact() const;
    virtual void printon(ostream&) const;
    virtual const Complex * complex() const { return this; }
    virtual const Numeric * add(const Numeric&) const;
    virtual const Numeric * sub(const Numeric&) const;
    virtual const Numeric * mul(const Numeric&) const;
    virtual const Numeric * div(const Numeric&) const;
    virtual const Numeric * addr(const Numeric&) const;
    virtual const Numeric * subr(const Numeric&) const;
    virtual const Numeric * mulr(const Numeric&) const;
    virtual const Numeric * divr(const Numeric&) const;
    virtual const Numeric * addFixInt(fix_int) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const;
    virtual const Numeric * divFixInt(fix_int) const;
    virtual const Numeric* addDouble(double x) const;
    virtual const Numeric* subDouble(double x) const;
    virtual const Numeric* mulDouble(double x) const;
    virtual const Numeric* divDouble(double x) const;
};

class ComplexPair : public Complex {
  public:
    const Real& re;
    const Real& im;
    ComplexPair(const Complex& val) : re(val.realPart()), im(val.imagPart()) {}
    ComplexPair(const Real& r, const Real& i) : re(r), im(i) { }
    virtual const Real& realPart() const { return re; }
    virtual const Real& imagPart() const { return im; }
    virtual const Numeric& neg() const;
    virtual int sign() const;
    virtual Numeric * as_exact();
    virtual Numeric * as_inexact();
};

enum RealToIntMode { FloorMode, CeilingMode, TruncateMode, RoundMode };

class Real : public Complex {
    DECLARE_MEMBERS(Real)
  public:
    virtual int sign() const = 0;
    virtual const Real& realPart() const;
    virtual const Real& imagPart() const;
    virtual const Numeric& neg() const { return rneg(); }
    virtual const Real& rneg() const = 0;
    virtual const ExtendedInteger& to_integer(enum RealToIntMode) const = 0;
    inline const ExtendedInteger& floor() const {return to_integer(FloorMode);}
    virtual const ExtendedInteger& div_floor(const Real& divisor) const;
    virtual const Real * real() const { return this; }
    virtual Numeric * as_inexact();
    virtual double as_double() const;
    virtual const Numeric* addDouble(double x) const;
    virtual const Numeric* subDouble(double x) const;
    virtual const Numeric* mulDouble(double x) const;
    virtual const Numeric* divDouble(double x) const;
};

struct _double {
    DECLARE_MEMBERS_STATIC(_double, PrimitiveType)
    double val;
};

class Double : public Real , public _double {
    DECLARE_MEMBERS(Double)
 public:
    inline Double(double d) { val = d; }
    virtual void printon(ostream&) const;
#ifdef Q_ENV
    virtual void dumpPtr(CFile *cf) const;
#endif
#ifdef CORAL
    virtual void dump(int arg_number, FILE *file);
#endif

    virtual const Real& rneg() const { return *new Double(-val); }
    virtual is_exact() const;
    virtual int sign() const { return val > 0 ? 1 : val < 0 ? -1 : 0; }
    virtual const Numeric* addFixInt(fix_int x) const
	{return new Double((double)x+val);}
    virtual const Numeric* subFixInt(fix_int x) const
	{return new Double((double)x-val);}
    virtual const Numeric* mulFixInt(fix_int x) const
	{return new Double((double)x*val);}
    virtual const Numeric* divFixInt(fix_int x) const
	{return new Double((double)x/val);}
    virtual const Numeric* addDouble(double x)const {return new Double(x+val);}
    virtual const Numeric* subDouble(double x)const {return new Double(x-val);}
    virtual const Numeric* mulDouble(double x)const {return new Double(x*val);}
    virtual const Numeric* divDouble(double x)const {return new Double(x/val);}
    virtual const Numeric * add(const Numeric& x) const {return x.addDouble(val);}
    virtual const Numeric * sub(const Numeric& x) const {return x.subDouble(val);}
    virtual const Numeric * mul(const Numeric& x) const {return x.mulDouble(val);}
    virtual const Numeric * div(const Numeric& x) const {return x.divDouble(val);}
    virtual const Numeric * addr(const Numeric&) const;
    virtual const Numeric * subr(const Numeric&) const;
    virtual const Numeric * mulr(const Numeric&) const;
    virtual const Numeric * divr(const Numeric&) const;
    virtual int compare(const Root&) const;
    virtual int compareDouble(double) const;
    virtual int compareFixInt(fix_int) const;
    virtual const ExtendedInteger& to_integer(enum RealToIntMode) const;
    virtual double as_double() const;
};

class Rational : public Real {
    DECLARE_MEMBERS(Rational)
    virtual is_exact() const;
    virtual int sign() const = 0;
    virtual const ExtendedInteger& to_integer(enum RealToIntMode) const = 0;
    virtual const Real& rneg() const = 0;
    virtual const Integer& numerator() const = 0;
    virtual const Integer& denominator() const = 0;
    virtual const Rational * rational() const { return this; }
    virtual const Numeric * add(const Numeric&) const;
    virtual const Numeric * addr(const Numeric&) const;
    virtual const Numeric * sub(const Numeric&) const;
    virtual const Numeric * subr(const Numeric&) const;
    virtual const Numeric * mul(const Numeric&) const;
    virtual const Numeric * mulr(const Numeric&) const;
    virtual const Numeric * div(const Numeric&) const;
    virtual const Numeric * divr(const Numeric&) const;
    virtual const Numeric * ipower(fix_int) const;
    friend int compare(const Rational& x, const Rational& y);
    virtual int compare(const Root& other) const;
    virtual int compareRational(const Rational& x) const;
    virtual int compareFixInt(fix_int) const;
    virtual const Numeric * divFixInt(fix_int arg) const;
    virtual double as_double() const;
};

const Rational *MakeRational(const Integer& num, const Integer& den);
const Rational *MakeRational(fix_int num, fix_int den);

// ExtendedInteger is Integer or Infinity or -Infinity

#define InfiniteLenField ((unsigned long)(-1))

class ExtendedInteger : public Rational {
  public:
    unsigned long len;
//  short sgn;
//  unsigned short extra; /* sz = len + extra */
    union {
	fix_unsigned U[1];
#define _mp_limb unsigned long int /* from gmp package */
	_mp_limb L[1];
	fix_int S[1];
	fix_int val;
    };
    int finite() const { return len != InfiniteLenField; }
    virtual const ExtendedInteger& to_integer(enum RealToIntMode) const {return *this;}
    const Integer& byte(int size, int position, int sign_extend = 0,
			int dest_pos = 0) const;
    inline int is_negative() const { return S[len-1] < 0; }
};

class RationalInfinity :  public ExtendedInteger {
  public:
//    int val;
    RationalInfinity(int s) { len = InfiniteLenField; val = s; }
    virtual const Real& rneg() const;
    virtual int sign() const { return val; }
    virtual void printon(ostream& outs) const;
    virtual const Integer& numerator() const;
    virtual const Integer& denominator() const;

    virtual const Numeric * addFixInt(fix_int) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const;
    virtual const Numeric * divFixInt(fix_int) const;
    virtual const Numeric * addBigInt(Integer&) const;
    virtual const Numeric * subBigInt(Integer&) const;
    virtual const Numeric * mulBigInt(Integer&) const;
    virtual const Numeric * divBigInt(Integer&) const;
};
extern const RationalInfinity PosInfinity, NegInfinity;

class Integer : public ExtendedInteger {
    DECLARE_MEMBERS(Integer)
 public:
    int big_len() const { return len; }
    int is_fix() const { return len==1; }
    fix_int fix_value() const { return val; } // Only valid iff is_fix()
    int integer_length() const; // Implements Common Lisp's integer-length
    Integer();

    virtual const Integer& numerator() const;
    virtual const Integer& denominator() const;
#ifdef Q_ENV
    virtual long magic() const;
#endif
    virtual int hash() const;
//    virtual const Integer * gcd(const Integer&) const;
    virtual void do_delete();
    inline const Integer& operator>>(int count) { return *this << (-count); }
    virtual const Integer *gcd(const Integer&) const;
    virtual const Integer *gcdFixInt(fix_int) const;
    const Integer& boolean(const Integer& arg, int op) const;
    virtual const Integer* integer() const { return this; }

    friend int compare(const Integer& x, const Integer& y);
//    virtual int compare(const Root& other) const;
    virtual int compareRational(const Rational& x) const;
    virtual int compareFixInt(fix_int) const;

    virtual void printon(ostream&) const;

    inline int odd() const { return U[0] & 1; }
    // bit(i) is 1 if (after sign-extending) the i'th bit is 1.
    inline int bit(fix_unsigned i) const;
    virtual const Real& rneg() const;
    virtual const Numeric * addFixInt(fix_int i) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const;

    virtual const Numeric * addBigInt(Integer&) const;
    virtual const Numeric * subBigInt(Integer&) const;
    virtual const Numeric * mulBigInt(Integer&) const;
    virtual const Numeric * divBigInt(Integer&) const;

    virtual const Numeric * add(const Numeric& x) const;
    virtual const Numeric * sub(const Numeric& x) const;
    virtual const Numeric * mul(const Numeric& x) const;
    virtual const Numeric * div(const Numeric& x) const;

    inline int bsign() const;
    const Integer& operator-() const;
    virtual const Numeric * ipower(fix_int) const;
    virtual const Integer& operator<<(int count) const;
    virtual int sign() const;
    const Integer * simplify();

    const Integer& byte(int size, int position, int sign_extend = 0,
			int dest_pos = 0) const;
    const Integer&deposit_byte(Integer& v, int sz, int pos, int v_pos=0) const;
};

extern int bit_count(register fix_unsigned* words, register int length);
extern int bit_count(Integer&);
extern void div(const Integer& X, const Integer& Y,
		enum RealToIntMode mode,
		const Integer** quotient, const Integer** remainder);
extern void div(const Real& X, const Real& Y,
		enum RealToIntMode mode,
		const Real** quotient, const Real** remainder);
extern int bit_test(const Integer& arg1, const Integer& arg2);
extern const Integer& operator+(const Integer &x, const Integer& y);
extern const Integer& operator+(const Integer &X, fix_int Y);
extern const Integer& operator-(const Integer &x, const Integer& y);
extern const Integer& operator*(const Integer &x, const Integer& y);
extern const Integer& itimes(const Integer &x, const Integer& y);
inline const Integer& operator*(int x, const Integer& y)
{
    return *(Integer*)y.mulFixInt(x);
}

extern Integer* NewBigInt(int len);
extern Integer* NewBigInt(int len, fix_unsigned* data);

class SmallBigInt : public BigInt {
    // BigInt created from a fix_int
 public:
    SmallBigInt(fix_int i) { len = 1; U[0] = i; }
};

extern "C" const FixInt *MakeFixInt(long i);

struct _long {
    DECLARE_MEMBERS_STATIC(_long, PrimIntType)
    long val;
//    void coerceTo(Class *type, void *dest);
};
struct _int {
    DECLARE_MEMBERS_STATIC(_int, PrimIntType)
    int val;
//    void coerceTo(Class *type, void *dest);
};

class FixInt : public Integer
{
    DECLARE_MEMBERS(FixInt)
 public:
#ifdef DIGITon16BITS
    static fix_int smallest() { return -0x8000; }
    static fix_int biggest() { return 0x7FFF; }
#else
    static fix_int smallest() { return -0x80000000; }
    static fix_int biggest() { return 0x7FFFFFFF; }
#endif
    FixInt() { len = 1; }
    FixInt(fix_int i) { len = 1; val = i; }
#ifdef Q_ENV
    virtual void dumpPtr(CFile *cf) const;
#endif
#ifdef CORAL
    virtual void dump(int arg_number, FILE *file);
#endif
    fix_int ival() const { return val; }
    virtual const Real& rneg() const;
    virtual int sign() const { return val > 0 ? 1 : val < 0 ? -1 : 0; }

    virtual const Numeric * add(const Numeric& x) const {return x.addFixInt(val);}
    virtual const Numeric * sub(const Numeric& x) const {return x.subFixInt(val);}
    virtual const Numeric * mul(const Numeric& x) const {return x.mulFixInt(val);}
    virtual const Numeric * div(const Numeric& x) const {return x.divFixInt(val);}

    virtual const Numeric* addDouble(double x)const {return new Double(x+val);}
    virtual const Numeric* subDouble(double x)const {return new Double(x-val);}
    virtual const Numeric* mulDouble(double x)const {return new Double(x*val);}
    virtual const Numeric* divDouble(double x)const {return new Double(x/val);}

    virtual const Numeric* addBigInt(Integer& x)const {return &(x + *this); }
    virtual const Numeric* subBigInt(Integer& x)const {return &(x - *this); }
    virtual const Numeric* mulBigInt(Integer& x)const {return &(x * *this); }

    virtual const Numeric * addFixInt(fix_int) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const;
    virtual const Numeric * divFixInt(fix_int arg) const
	{return MakeRational(arg, fix_value());}
//  friend const Integer& operator+(int x, const FixInt& y) ...;
//  virtual const Numeric& addFixInt(fix_int i) { return i+*this; }
//  friend const Integer& operator+(const Integer& x, const FixInt& y) ...;
//  virtual const Numeric& addBigInt(Integer&x) { return x+*this; }

    virtual int compare(const Root&) const;
    virtual int compareDouble(double) const;
    virtual int compareFixInt(fix_int) const;
    virtual const Integer& operator<<(int count) const;
    virtual const Integer *gcd(const Integer& arg) const
	{ return arg.gcdFixInt(fix_value()); }
    virtual const Integer *gcdFixInt(fix_int) const;
    virtual int getlong(long *i) const;
    virtual double as_double() const { return (double)val; }
#ifdef Q_ENV
    virtual const StringC *asString(int format=0) const;
#endif
};

#define LeastSmallInt (-128)
#define BiggestSmallInt 1001
#define CountSmallInt (BiggestSmallInt-LeastSmallInt)

class SmallInt : public FixInt { // These are statically allocated
    DECLARE_MEMBERS(SmallInt)
  public:
    static fix_int smallest() { return LeastSmallInt; }
    static fix_int biggest() { return BiggestSmallInt; }

    SmallInt() : FixInt() { }
    SmallInt(int i) : FixInt(i) { }
#ifdef Q_ENV
    virtual void dumpPtr(CFile *cf) const;
#endif
#ifdef CORAL
    virtual void dump(int arg_number, FILE *file);
#endif

    virtual const Numeric * addFixInt(fix_int x) const;
    virtual const Numeric * subFixInt(fix_int x) const;
    virtual const Numeric * mulFixInt(fix_int x) const;
    virtual const Numeric * add(const Numeric& x) const {return x.addFixInt(val);}
    virtual const Numeric * sub(const Numeric& x) const {return x.subFixInt(val);}
    virtual const Numeric * mul(const Numeric& x) const {return x.mulFixInt(val);}
    virtual void do_delete() { } // Never deleted
};

class Fraction : public Rational {
    DECLARE_MEMBERS(Fraction)
  public:
    const Integer& num;
    const Integer& den;
    Fraction(const Integer& n, const Integer& d) : num(n), den(d) { }
    virtual const ExtendedInteger& to_integer(enum RealToIntMode) const;
    virtual const Integer& numerator() const { return num; }
    virtual const Integer& denominator() const { return den; }
    virtual void printon(ostream&) const;
    virtual const Real& rneg() const;
    virtual const Numeric * addFixInt(fix_int) const;
    virtual const Numeric * subFixInt(fix_int) const;
    virtual const Numeric * mulFixInt(fix_int) const ;
    virtual const Numeric * divFixInt(fix_int) const;
    virtual const Numeric * addBigInt(Integer& x) const;
    virtual const Numeric * subBigInt(Integer& x) const;
    virtual const Numeric * mulBigInt(Integer& x) const;
    virtual const Numeric * divBigInt(Integer& x) const;
    virtual int sign() const { return num.sign(); }
};

extern SmallInt SmallIntTable[CountSmallInt];
#define MkSmallInt(i)  SmallIntTable[(i)-LeastSmallInt]
#define Zero (&MkSmallInt(0))
#define One (&MkSmallInt(1))
#define MinusOne (&MkSmallInt(-1))

extern const Integer *ConvertInteger(const Root *);
extern Integer *ConvertInteger(Root *);
extern const Real *ConvertReal(const Root *);
extern Real *ConvertReal(Root *);
extern "C" const Integer *StrToInt(const char *str, int base, int str_len=-1);

INLINE const Real& operator+(fix_int x, const Real& y)
{
    return *(Real*)y.addFixInt(x);
}
INLINE const Real& operator-(fix_int x, const Real& y)
{
    return *(Real*)y.subFixInt(x);
}
INLINE const Real& operator*(fix_int x, const Real& y)
{
    return *(Real*)y.mulFixInt(x);
}
INLINE const Real& operator+(const Real &x, const Real& y)
{
    return *(const Real*)x.add(y);
}
INLINE const Real& operator-(const Real &x, const Real& y)
{
    return *(const Real*)x.sub(y);
}
INLINE const Real& operator*(const Real &x, const Real& y)
{
    return *(const Real*)x.mul(y);
}
INLINE const Real& operator/(const Real &x, const Real& y)
{
    return *(const Real*)x.div(y);
}
INLINE const Rational& operator+(const Rational &x, const Rational& y)
{
    const Integer& x_num = x.numerator();
    const Integer& x_den = x.denominator();
    const Integer& y_num = y.numerator();
    const Integer& y_den = y.denominator();
    if (&x_den == &y_den)
	return *MakeRational(x_num + y_num, x_den);
    return *MakeRational(y_den * x_num + y_num * x_den, x_den * y_den);
}
extern const Rational& operator-(const Rational &x, const Rational& y);
INLINE const Rational& operator*(const Rational &x, const Rational& y)
{
    return *MakeRational(x.numerator() * y.numerator(),
			 x.denominator() * y.denominator());
}
INLINE const Rational& operator/(const Rational &x, const Rational& y)
{
    return *MakeRational(x.numerator() * y.denominator(),
			 x.denominator() * y.numerator());
}
INLINE const Complex& operator+(const Complex &x, const Complex& y)
{
    const Real& r = x.realPart() + y.realPart();
    const Real& i = x.imagPart() + y.imagPart();
    if (&i == Zero) return r; else return *new ComplexPair(r, i);
}
INLINE const Complex& operator-(const Complex &x, const Complex& y)
{
    const Real& r = x.realPart() - y.realPart();
    const Real& i = x.imagPart() - y.imagPart();
    if (&i == Zero) return r; else return *new ComplexPair(r, i);
}
extern const Complex& operator*(const Complex &x, const Complex& y);
extern const Complex& operator/(const Complex &x, const Complex& y);
extern const Numeric* power(const Numeric* x, const Numeric* y);
INLINE int Integer::bsign() const
{  return this == (const Integer*)Zero ? 0 : S[len-1] < 0 ? -1 : 1; }

inline unsigned long max(unsigned long x, unsigned long y)
{ return x>y ? x : y; }
#define PosInfinityCode 1
#define NegInfinityCode (-PosInfinityCode)
#endif /* !GENNUM_H */
