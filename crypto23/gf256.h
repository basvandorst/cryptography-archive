#ifndef GF256_H
#define GF256_H

#include "cryptlib.h"

class GF256
{
public:
	typedef byte Element;
	typedef int RandomizationParameter;

	GF256(byte modulus) : m_modulus(modulus) {}

	Element RandomElement(RandomNumberGenerator &rng, int ignored = 0) const
		{return rng.GetByte();}

	bool Equal(Element a, Element b) const
		{return a==b;}

	Element Zero() const
		{return 0;}

	Element Add(Element a, Element b) const
		{return a^b;}

	Element& Accumulate(Element &a, Element b) const
		{return a^=b;}

	Element Inverse(Element a) const
		{return a;}

	Element Subtract(Element a, Element b) const
		{return a^b;}

	Element& Reduce(Element &a, Element b) const
		{return a^=b;}

	Element Double(Element a) const
		{return 0;}

	Element One() const
		{return 1;}

	Element Multiply(Element a, Element b) const;

	Element Square(Element a) const
		{return Multiply(a, a);}

	bool IsUnit(Element a) const
		{return a != 0;}

	Element MultiplicativeInverse(Element a) const;

	Element Divide(Element a, Element b) const
		{return Multiply(a, MultiplicativeInverse(b));}

private:
	word m_modulus;
};

#endif
