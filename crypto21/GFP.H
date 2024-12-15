#ifndef GFP_H
#define GFP_H

#include "bignum.h"
#include "nbtheory.h"

class GFP
{
public:
	typedef bignum Element;

	GFP(const bignum &modulus)	// modulus must be prime
		: modulus(modulus) {}

	boolean Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Identity() const
		{return (word)0;}

	Element Add(const Element &a, const Element &b) const
		{return (a+b)%modulus;}

	Element Inverse(const Element &a) const
		{return !a ? a : modulus-a;}

	Element Subtract(const Element &a, const Element &b) const
		{return Add(a, Inverse(b));}

	Element Double(const Element &a) const
		{return a.Double()%modulus;}

	Element MultiplicativeIdentity() const
		{return (word)1;}

	Element Multiply(const Element &a, const Element &b) const
		{return a*b%modulus;}

	Element Square(const Element &a) const
		{return a.Square()%modulus;}

	boolean IsUnit(const Element &a) const
		{return !!a;}

	Element MultiplicativeInverse(const Element &a) const
		{return EuclideanMultiplicativeInverse(a, modulus);}

	Element Divide(const Element &a, const Element &b) const
		{return a*MultiplicativeInverse(b)%modulus;}

	bignum FieldSize() const
		{return modulus;}

	unsigned int MaxElementBitLength() const
		{return (modulus-1).BitCount();}

	unsigned int MaxElementByteLength() const
		{return (modulus-1).ByteCount();}

private:
	bignum modulus;
};

#endif
