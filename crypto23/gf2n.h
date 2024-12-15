#ifndef GF2N_H
#define GF2N_H

#include "cryptlib.h"
#include "misc.h"
#include "algebra.h"

#include "iosfwd"

class PolynomialMod2
{
public:
	//@Man: ENUMS, EXCEPTIONS, and TYPEDEFS
	//@{
		class DivideByZero : public CryptlibException 
		{
		public: 
			DivideByZero() : CryptlibException("PolynomialMod2: division by zero") {}
		};

		typedef unsigned int RandomizationParameter;
	//@}

	//@Man: CREATORS
	//@{
		/// creates the zero polynomial
		PolynomialMod2();
		/// copy constructor
		PolynomialMod2(const PolynomialMod2& t);

		/// convert from word
		/** value should be encoded with the least significant bit as coefficient to x^0
			and most significant bit as coefficient to x^(WORD_BITS-1)
			bitLength denotes how much memory to allocate initially
		*/
		PolynomialMod2(word value, unsigned int bitLength=WORD_BITS);

		/// convert from big-endian byte array
		PolynomialMod2(const byte *encodedPoly, unsigned int byteCount)
			{Decode(encodedPoly, byteCount);}

/*		// not implemented
		PolynomialMod2(const byte *BEREncodedBitString)
			{BERDecode(BEREncodedBitString);}
		PolynomialMod2(BufferedTransformation &bt)
			{BERDecode(bt);}
*/


		/// create a random polynomial uniformly distributed over all polynomials with degree less than bitcount
		PolynomialMod2(RandomNumberGenerator &rng, unsigned int bitcount)
			{Randomize(rng, bitcount);}

		/// return x^i
		static PolynomialMod2 Monomial(unsigned i);
		/// return x^t0 + x^t1 + x^t2
		static PolynomialMod2 Trinomial(unsigned t0, unsigned t1, unsigned t2);
		/// return x^(n-1) + ... + x + 1
		static PolynomialMod2 AllOnes(unsigned n);

		///
		static const PolynomialMod2 &Zero();
		///
		static const PolynomialMod2 &One();
	//@}

	//@Man: ACCESSORS
	//@{
		/// minimum number of bytes to encode this polynomial
		/** MinEncodedSize of 0 is 1 */
		unsigned int MinEncodedSize() const {return STDMAX(1U, ByteCount());}
		/// encode in big-endian format
		/** if outputLen < MinEncodedSize, the most significant bytes will be dropped
			if outputLen > MinEncodedSize, the most significant bytes will be padded
		*/
		unsigned int Encode(byte *output, unsigned int outputLen) const;

/*		// not implemented
		unsigned int DEREncode(byte *output) const;
		unsigned int DEREncode(BufferedTransformation &bt) const;
*/

		/// number of significant bits = floor(log2(abs(*this))) - 1
		unsigned int BitCount() const;
		/// number of significant bytes = ceiling(BitCount()/8)
		unsigned int ByteCount() const;
		/// number of significant words = ceiling(ByteCount()/sizeof(word))
		unsigned int WordCount() const;

		/// return the n-th bit, n=0 being the least significant bit
		bool GetBit(unsigned int n) const {return GetCoefficient(n)!=0;}
		/// return the n-th byte
		byte GetByte(unsigned int n) const;

		/// the zero polynomial will return a degree of -1
		int Degree() const {return BitCount()-1;}
		/// degree + 1
		unsigned int CoefficientCount() const {return BitCount();}
		/// return coefficient for x^i
		int GetCoefficient(unsigned int i) const 
			{return (i/WORD_BITS < reg.size) ? int(reg[i/WORD_BITS] >> (i % WORD_BITS)) & 1 : 0;}
		/// return coefficient for x^i
		int operator[](unsigned int i) const {return GetCoefficient(i);}
	//@}

	//@Man: MANIPULATORS
	//@{
		///
		PolynomialMod2&  operator=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator&=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator^=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator+=(const PolynomialMod2& t) {return *this ^= t;}
		///
		PolynomialMod2&  operator-=(const PolynomialMod2& t) {return *this ^= t;}
		///
		PolynomialMod2&  operator*=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator/=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator%=(const PolynomialMod2& t);
		///
		PolynomialMod2&  operator<<=(unsigned int);
		///
		PolynomialMod2&  operator>>=(unsigned int);

		///
		void Decode(const byte *input, unsigned int inputLen);

/*
		void BERDecode(const byte *input);
		void BERDecode(BufferedTransformation &bt);
*/
		///
		void Randomize(RandomNumberGenerator &rng, unsigned int bitcount);

		///
		void SetBit(unsigned int i, int value = 1);
		/// set the n-th byte to value
		void SetByte(unsigned int n, byte value);

		///
		void SetCoefficient(unsigned int i, int value) {SetBit(i, value);}

		///
		friend inline void swap(PolynomialMod2 &a,PolynomialMod2 &b) {swap(a.reg, b.reg);}
	//@}

	//@Man: UNARY OPERATORS
	//@{
		///
		bool			operator!() const;
		///
		PolynomialMod2	operator+() const {return *this;}
		///
		PolynomialMod2	operator-() const {return *this;}
	//@}

	//@Man: BINARY OPERATORS
	//@{
		///
		friend PolynomialMod2 operator&(const PolynomialMod2 &a, const PolynomialMod2 &b);
		///
		friend PolynomialMod2 operator^(const PolynomialMod2 &a, const PolynomialMod2 &b);
		///
		friend PolynomialMod2 operator+(const PolynomialMod2 &a, const PolynomialMod2 &b) {return a^b;}
		///
		friend PolynomialMod2 operator-(const PolynomialMod2 &a, const PolynomialMod2 &b) {return a^b;}
		///
		friend PolynomialMod2 operator*(const PolynomialMod2 &a, const PolynomialMod2 &b);
		///
		friend PolynomialMod2 operator/(const PolynomialMod2 &a, const PolynomialMod2 &b);
		///
		friend PolynomialMod2 operator%(const PolynomialMod2 &a, const PolynomialMod2 &b);

		///
		PolynomialMod2 operator>>(unsigned int n) const;
		///
		PolynomialMod2 operator<<(unsigned int n) const;

		///
		friend bool operator==(const PolynomialMod2 &a, const PolynomialMod2 &b);
		///
		friend bool operator!=(const PolynomialMod2 &a, const PolynomialMod2 &b)
			{return !(a==b);}
		/// compares degree
		friend bool operator> (const PolynomialMod2 &a, const PolynomialMod2 &b)
			{return a.Degree() > b.Degree();}
		/// compares degree
		friend bool operator>=(const PolynomialMod2 &a, const PolynomialMod2 &b)
			{return a.Degree() >= b.Degree();}
		/// compares degree
		friend bool operator< (const PolynomialMod2 &a, const PolynomialMod2 &b)
			{return a.Degree() < b.Degree();}
		/// compares degree
		friend bool operator<=(const PolynomialMod2 &a, const PolynomialMod2 &b)
			{return a.Degree() <= b.Degree();}
	//@}

	//@Man: OTHER ARITHMETIC FUNCTIONS
	//@{
		///	sum modulo 2 of all coefficients
		unsigned int Parity() const;

		/// is always zero since we're working modulo 2
		PolynomialMod2 Doubled() const {return Zero();}
		///
		PolynomialMod2 Squared() const;

		/// only 1 is a unit
		bool IsUnit() const {return *this == One();}
		/// return inverse if *this is a unit, otherwise return 0
		PolynomialMod2 MultiplicativeInverse() const {return IsUnit() ? One() : Zero();}

		/// greatest common divisor
		static PolynomialMod2 Gcd(const PolynomialMod2 &a, const PolynomialMod2 &n);
		/// calculate multiplicative inverse of *this mod n
		PolynomialMod2 InverseMod(const PolynomialMod2 &) const;

		/// calculate r and q such that (a == d*q + r) && (deg(r) < deg(d))
		static void Divide(PolynomialMod2 &r, PolynomialMod2 &q, const PolynomialMod2 &a, const PolynomialMod2 &d);
	//@}

	//@Man: INPUT/OUTPUT
	//@{
		///
		friend std::ostream& operator<<(std::ostream& out, const PolynomialMod2 &a);
	//@}

private:
	friend class GF2NT;
	friend class GF2NO;

	SecWordBlock reg;
};

// polynomial basis
class GF2NP : public QuotientRing<EuclideanDomainOf<PolynomialMod2> >
{
public:
	GF2NP(const PolynomialMod2 &modulus);

	bool Equal(const Element &a, const Element &b) const
		{assert(a.Degree() < m_modulus.Degree() && b.Degree() < m_modulus.Degree()); return a==b;}

	bool IsUnit(const Element &a) const
		{assert(a.Degree() < m_modulus.Degree()); return !!a;}

	unsigned int MaxElementBitLength() const
		{return m;}

	unsigned int MaxElementByteLength() const
		{return bitsToBytes(MaxElementBitLength());}

protected:
	unsigned int m;
};

// trinomial basis
class GF2NT : public GF2NP
{
public:
	// polynomial modulus = x^t0 + x^t1 + x^t2, t0 > t1 > t2
	GF2NT(unsigned int t0, unsigned int t1, unsigned int t2);

	Element Multiply(const Element &a, const Element &b) const;

	Element Square(const Element &a) const
		{return Reduce(a.Squared());}

	Element MultiplicativeInverse(const Element &a) const;

private:
	Element Reduce(const Element &a) const;

	unsigned int t0, t1;
	PolynomialMod2 result;
};

// optimal normal basis
class GF2NO : public AbstractRing<PolynomialMod2>
{
public:
	// an exception thrown by the contructor
	class FieldNotAvailable : public CryptlibException 
	{
	public: 
		FieldNotAvailable() 
			: CryptlibException("GF2NO: no ONB exists for specified field") {}
	};

	GF2NO(unsigned int n);
	~GF2NO();

	bool Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Zero() const
		{return (word)0;}

	Element Add(const Element &a, const Element &b) const
		{return a^b;}

	Element Inverse(const Element &a) const
		{return a;}

	Element Subtract(const Element &a, const Element &b) const
		{return a^b;}

	Element Double(const Element &) const
		{return (word)0;}

	Element One() const
		{return PolynomialMod2::AllOnes(m);}

	Element Multiply(const Element &a, const Element &b) const;
	Element Square(const Element &a) const;
	Element RepeatedSquare(const Element &a, unsigned int times) const;
	Element MultiplicativeInverse(const Element &a) const;

	bool IsUnit(const Element &a) const
		{return !!a;}

	Element Divide(const Element &a, const Element &b) const
		{return a*MultiplicativeInverse(b);}

	unsigned int MaxElementBitLength() const
		{return m-1;}

	unsigned int MaxElementByteLength() const
		{return bitsToBytes(MaxElementBitLength());}

private:
	unsigned int m;
	unsigned int (*lambda)[2];	// pointer to array of unsigned longs
};

typedef GF2NT GF2N;

#endif
