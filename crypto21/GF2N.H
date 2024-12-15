#ifndef GF2N_H
#define GF2N_H

#include "cryptlib.h"
#include "nbtheory.h"
#include "misc.h"

class ostream;

class PolynomialMod2
{
public:
    class DivideByZero : public CryptlibException 
	{
	public: 
		DivideByZero() : CryptlibException("PolynomialMod2: division by zero") {}
	};

	PolynomialMod2();
	PolynomialMod2(word value, unsigned int bitLength=WORD_BITS);
	PolynomialMod2(const PolynomialMod2& t);
    PolynomialMod2(const byte *encodedPoly, unsigned int byteCount)
		{Decode(encodedPoly, byteCount);}
    PolynomialMod2(const byte *BEREncodedBitString)
        {BERDecode(BEREncodedBitString);}
    PolynomialMod2(BufferedTransformation &bt)
        {BERDecode(bt);}
    PolynomialMod2(RandomNumberGenerator &rng, unsigned int bitcount)
        {Randomize(rng, bitcount);}

	static PolynomialMod2 Monomial(unsigned i);
	static PolynomialMod2 Trinomial(unsigned t0, unsigned t1, unsigned t2);
	static PolynomialMod2 AllOnes(unsigned i);
    static const PolynomialMod2 ZERO;

    // encode polynomial as a big-endian byte array, returns size of output
    unsigned int Encode(byte *output) const;
    // use this to make sure output size is exactly outputLen
    unsigned int Encode(byte *output, unsigned int outputLen) const;

    void Decode(const byte *input, unsigned int inputLen);

    // encode PolynomialMod2 using Distinguished Encoding Rules, returns size of output
    unsigned int DEREncode(byte *output) const;
    unsigned int DEREncode(BufferedTransformation &bt) const;

    void BERDecode(const byte *input);
    void BERDecode(BufferedTransformation &bt);

    void Randomize(RandomNumberGenerator &rng, unsigned int bitcount);

	unsigned int WordCount() const;
    unsigned int ByteCount() const;
    unsigned int BitCount() const;
	int Degree() const {return BitCount()-1;}
	unsigned int Parity() const;

    /*  Various member unary operator functions. */

    PolynomialMod2&  operator++();
    PolynomialMod2&  operator--();
    boolean          operator!() const;
    const PolynomialMod2&  operator-() const {return *this;}

    /*  Various member binary operator functions. */

    PolynomialMod2&  operator=(const PolynomialMod2& t);
    PolynomialMod2&  operator&=(const PolynomialMod2& t);
    PolynomialMod2&  operator^=(const PolynomialMod2& t);
    PolynomialMod2&  operator+=(const PolynomialMod2& t) {return *this ^= t;}
    PolynomialMod2&  operator-=(const PolynomialMod2& t) {return *this ^= t;}
    PolynomialMod2&  operator*=(const PolynomialMod2& t);
    PolynomialMod2&  operator/=(const PolynomialMod2& t);
    PolynomialMod2&  operator%=(const PolynomialMod2& t);
    PolynomialMod2&  operator<<=(unsigned int);
    PolynomialMod2&  operator>>=(unsigned int);

    // returns the n-th bit, n=0 being the least significant bit
    inline int operator[](unsigned int n) const
	{
		if (n/WORD_BITS >= reg.size)
			return 0;
		else
	    	return int(reg[n/WORD_BITS] >> (n % WORD_BITS)) & 1;
	}

    /*  Various const member binary operator functions. */

    PolynomialMod2 operator&(const PolynomialMod2 &b) const;
    PolynomialMod2 operator^(const PolynomialMod2 &b) const;
    inline PolynomialMod2 operator+(const PolynomialMod2 &b) const {return operator^(b);}
    inline PolynomialMod2 operator-(const PolynomialMod2 &b) const {return operator^(b);}
    PolynomialMod2 operator*(const PolynomialMod2 &b) const;
    PolynomialMod2 operator/(const PolynomialMod2 &b) const;
    PolynomialMod2 operator%(const PolynomialMod2 &b) const;
    PolynomialMod2 operator>>(unsigned int n) const;
    PolynomialMod2 operator<<(unsigned int n) const;
    boolean operator==(const PolynomialMod2 &b) const;
    boolean operator!=(const PolynomialMod2 &b) const;

	PolynomialMod2 Double() const {return (word)0;}
	PolynomialMod2 Square() const;

	boolean IsNegative() const {return FALSE;}

	PolynomialMod2 MultiplicativeInverseMod(const PolynomialMod2 &) const;

    class DivideErr {};

    friend void Divide(PolynomialMod2 &r, PolynomialMod2 &q,
                       const PolynomialMod2 &a, const PolynomialMod2 &d);
    // POST: (a == d*q + r) && (deg(r) < deg(d))

    friend ostream& operator<<(ostream& out, const PolynomialMod2 &a);

    void SetBit(unsigned int n);
	friend void swap(PolynomialMod2&,PolynomialMod2&);

	PolynomialMod2 RotatedLeftBy(unsigned int n, unsigned int m) const;

private:
	void RotateRightByOne(unsigned int bitLength);

	friend class GF2N;
	friend class GF2NO;

    SecBlock<word> reg;
};

class GF2NP
{
public:
	typedef PolynomialMod2 Element;

	GF2NP(const PolynomialMod2 &basis)
		: basis(basis), m(basis.Degree()) {}

	boolean Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Identity() const
		{return (word)0;}

	Element Add(const Element &a, const Element &b) const
		{return a^b;}

	void Accumulate(Element &a, const Element &b) const
		{a^=b;}

	Element Inverse(const Element &a) const
		{return a;}

	Element Subtract(const Element &a, const Element &b) const
		{return a^b;}

	Element Double(const Element &) const
		{return (word)0;}

	Element MultiplicativeIdentity() const
		{return (word)1;}

	Element Multiply(const Element &a, const Element &b) const
		{return a*b%basis;}

	Element Square(const Element &a) const
		{return a.Square()%basis;}

	boolean IsUnit(const Element &a) const
		{return !!a;}

	Element MultiplicativeInverse(const Element &a) const
		{return EuclideanMultiplicativeInverse(a, basis);}

	Element Divide(const Element &a, const Element &b) const
		{return Multiply(a, MultiplicativeInverse(b));}

	Integer FieldSize() const
		{return Integer::Power2(m);}

	unsigned int MaxElementBitLength() const
		{return m;}

	unsigned int MaxElementByteLength() const
		{return bitsToBytes(MaxElementBitLength());}

private:
	const PolynomialMod2 basis;
	const unsigned m;
};

class GF2N
{
public:
	typedef PolynomialMod2 Element;

	GF2N(unsigned int t0, unsigned int t1, unsigned int t2);

	boolean Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Identity() const
		{return (word)0;}

	Element Add(const Element &a, const Element &b) const
		{return a^b;}

	Element& Accumulate(Element &a, const Element &b) const
		{return a^=b;}

	Element Inverse(const Element &a) const
		{return a;}

	Element Subtract(const Element &a, const Element &b) const
		{return a^b;}

	Element Double(const Element &) const
		{return (word)0;}

	Element MultiplicativeIdentity() const
		{return (word)1;}

	Element Multiply(const Element &a, const Element &b) const;

	Element Square(const Element &a) const
		{return Reduce(a.Square());}

	boolean IsUnit(const Element &a) const
		{return !!a;}

	Element MultiplicativeInverse(const Element &a) const;

	Element Divide(const Element &a, const Element &b) const
		{return Multiply(a, MultiplicativeInverse(b));}

	Integer FieldSize() const
		{return Integer::Power2(m);}

	unsigned int MaxElementBitLength() const
		{return m;}

	unsigned int MaxElementByteLength() const
		{return bitsToBytes(MaxElementBitLength());}

private:
	Element Reduce(const Element &a) const;

	const unsigned t0, t1;
	const unsigned m;
	const PolynomialMod2 basis;
	PolynomialMod2 result;
};

class GF2NO
{
public:
	typedef PolynomialMod2 Element;

	// an exception thrown by the contructor
	class FieldNotAvailable : public CryptlibException 
	{
	public: 
		FieldNotAvailable() 
			: CryptlibException("GF2NO: no ONB exists for specified field") {}
	};

	GF2NO(unsigned int n);
	~GF2NO();

	boolean Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Identity() const
		{return (word)0;}

	Element Add(const Element &a, const Element &b) const
		{return a^b;}

	Element Inverse(const Element &a) const
		{return a;}

	Element Subtract(const Element &a, const Element &b) const
		{return a^b;}

	Element Double(const Element &) const
		{return (word)0;}

	Element MultiplicativeIdentity() const
		{return PolynomialMod2::AllOnes(m);}

	Element Multiply(const Element &a, const Element &b) const;
	Element Square(const Element &a) const;
	Element RepeatedSquare(const Element &a, unsigned int times) const;
	Element MultiplicativeInverse(const Element &a) const;

	boolean IsUnit(const Element &a) const
		{return !!a;}

	Element Divide(const Element &a, const Element &b) const
		{return a*MultiplicativeInverse(b);}

	Integer FieldSize() const
		{return Integer::Power2(m);}

	unsigned int MaxElementBitLength() const
		{return m-1;}

	unsigned int MaxElementByteLength() const
		{return bitsToBytes(MaxElementBitLength());}

private:
	unsigned int m;
	unsigned int (*lambda)[2];	// pointer to array of unsigned longs
};

#endif
