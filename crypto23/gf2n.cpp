// gf2n.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "gf2n.h"
#include "algebra.h"
#include "words.h"

#include "iostream"
USING_NAMESPACE(std)

#include "algebra.cpp"

PolynomialMod2::PolynomialMod2()
{
}

PolynomialMod2::PolynomialMod2(word value, unsigned int bitLength)
	: reg(bitsToWords(bitLength))
{
	assert(reg.size>0);

	reg[0] = value;
	SetWords(reg+1, 0, reg.size-1);
}

PolynomialMod2::PolynomialMod2(const PolynomialMod2& t)
	: reg(t.reg.size)
{
	CopyWords(reg, t.reg, reg.size);
}

void PolynomialMod2::Randomize(RandomNumberGenerator &rng, unsigned int nbits)
{
	const unsigned int nbytes = nbits/8 + 1;
	SecByteBlock buf(nbytes);
	rng.GetBlock(buf, nbytes);
	buf[0] = (byte)Crop(buf[0], nbits % 8);
	Decode(buf, nbytes);
}

PolynomialMod2 PolynomialMod2::AllOnes(unsigned int bitLength)
{
	PolynomialMod2 result((word)0, bitLength);
	SetWords(result.reg, ~(word)0, result.reg.size);
	if (bitLength%WORD_BITS)
		result.reg[result.reg.size-1] = (word)Crop(result.reg[result.reg.size-1], bitLength%WORD_BITS);
	return result;
}

void PolynomialMod2::SetBit(unsigned int n, int value)
{
	if (value)
	{
		reg.CleanGrow(n/WORD_BITS + 1);
		reg[n/WORD_BITS] |= (word(1) << (n%WORD_BITS));
	}
	else
	{
		if (n/WORD_BITS < reg.size)
			reg[n/WORD_BITS] &= ~(word(1) << (n%WORD_BITS));
	}
}

byte PolynomialMod2::GetByte(unsigned int n) const
{
	if (n/WORD_SIZE >= reg.size)
		return 0;
	else
		return byte(reg[n/WORD_SIZE] >> ((n%WORD_SIZE)*8));
}

void PolynomialMod2::SetByte(unsigned int n, byte value)
{
	reg.CleanGrow(bytesToWords(n+1));
	reg[n/WORD_SIZE] &= ~(word(0xff) << 8*(n%WORD_SIZE));
	reg[n/WORD_SIZE] |= (word(value) << 8*(n%WORD_SIZE));
}

PolynomialMod2 PolynomialMod2::Monomial(unsigned i) 
{
	PolynomialMod2 r((word)0, i+1); 
	r.SetBit(i); 
	return r;
}

PolynomialMod2 PolynomialMod2::Trinomial(unsigned t0, unsigned t1, unsigned t2) 
{
	PolynomialMod2 r((word)0, t0+1);
	r.SetBit(t0);
	r.SetBit(t1);
	r.SetBit(t2);
	return r;
}

const PolynomialMod2 &PolynomialMod2::Zero()
{
	static const PolynomialMod2 zero;
	return zero;
}

const PolynomialMod2 &PolynomialMod2::One()
{
	static const PolynomialMod2 one = 1;
	return one;
}

void PolynomialMod2::Decode(const byte *input, unsigned int inputLen)
{
	reg.CleanNew(bytesToWords(inputLen));

	for (unsigned int i=0; i<inputLen; i++)
		reg[i/WORD_SIZE] |= input[inputLen-1-i] << (i%WORD_SIZE)*8;
}

unsigned int PolynomialMod2::Encode(byte *output, unsigned int outputLen) const
{
	unsigned int byteCount = STDMIN(outputLen, reg.size*WORD_SIZE);

	for (unsigned int i=0; i<byteCount; i++)
		output[outputLen-1-i] = byte(reg[i/WORD_SIZE] >> (i%WORD_SIZE)*8);

	memset(output, 0, outputLen-byteCount);
	return outputLen;
}

unsigned int PolynomialMod2::WordCount() const
{
	return CountWords(reg, reg.size);
}

unsigned int PolynomialMod2::ByteCount() const
{
	unsigned wordCount = WordCount();
	if (wordCount)
		return (wordCount-1)*WORD_SIZE + BytePrecision(reg[wordCount-1]);
	else
		return 0;
}

unsigned int PolynomialMod2::BitCount() const
{
	unsigned wordCount = WordCount();
	if (wordCount)
		return (wordCount-1)*WORD_BITS + BitPrecision(reg[wordCount-1]);
	else
		return 0;
}

unsigned int PolynomialMod2::Parity() const
{
	unsigned i;
	word temp=0;
	for (i=0; i<reg.size; i++)
		temp ^= reg[i];
	return ::Parity(temp);
}

PolynomialMod2& PolynomialMod2::operator=(const PolynomialMod2& t)
{
	reg.CopyFrom(t.reg);
	return *this;
}

PolynomialMod2& PolynomialMod2::operator^=(const PolynomialMod2& t)
{
	reg.CleanGrow(t.reg.size);
	XorWords(reg, t.reg, t.reg.size);
	return *this;
}

PolynomialMod2 operator^(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	if (b.reg.size >= a.reg.size)
	{
		PolynomialMod2 result((word)0, b.reg.size*WORD_BITS);
		XorWords(result.reg, a.reg, b.reg, a.reg.size);
		CopyWords(result.reg+a.reg.size, b.reg+a.reg.size, b.reg.size-a.reg.size);
		return result;
	}
	else
	{
		PolynomialMod2 result((word)0, a.reg.size*WORD_BITS);
		XorWords(result.reg, a.reg, b.reg, b.reg.size);
		CopyWords(result.reg+b.reg.size, a.reg+b.reg.size, a.reg.size-b.reg.size);
		return result;
	}
}

PolynomialMod2 operator&(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	PolynomialMod2 result((word)0, WORD_BITS*STDMIN(a.reg.size, b.reg.size));
	AndWords(result.reg, a.reg, b.reg, result.reg.size);
	return result;
}

PolynomialMod2 operator*(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	PolynomialMod2 result((word)0, a.BitCount() + b.BitCount());

	for (int i=b.Degree(); i>=0; i--)
	{
		result <<= 1;
		if (b[i])
			XorWords(result.reg, a.reg, a.reg.size);
	}
	return result;
}

PolynomialMod2 PolynomialMod2::Squared() const
{
	static const word map[16] = {0, 1, 4, 5, 16, 17, 20, 21, 64, 65, 68, 69, 80, 81, 84, 85};

	PolynomialMod2 result((word)0, 2*reg.size*WORD_BITS);

	for (unsigned i=0; i<reg.size; i++)
	{
		unsigned j;

		for (j=0; j<WORD_BITS; j+=8)
			result.reg[2*i] |= map[(reg[i] >> (j/2)) % 16] << j;

		for (j=0; j<WORD_BITS; j+=8)
			result.reg[2*i+1] |= map[(reg[i] >> (j/2 + WORD_BITS/2)) % 16] << j;
	}

	return result;
}

void PolynomialMod2::Divide(PolynomialMod2 &remainder, PolynomialMod2 &quotient,
				   const PolynomialMod2 &dividend, const PolynomialMod2 &divisor)
{
	if (!divisor)
	{
#ifdef THROW_EXCEPTIONS
		throw PolynomialMod2::DivideByZero();
#else
		return;
#endif
	}

	int degree = divisor.Degree();
	remainder.reg.CleanNew(bitsToWords(degree+1));
	if (dividend.BitCount() >= divisor.BitCount())
		quotient.reg.CleanNew(bitsToWords(dividend.BitCount() - divisor.BitCount() + 1));
	else
		quotient.reg.CleanNew(0);

	for (int i=dividend.Degree(); i>=0; i--)
	{
		remainder <<= 1;
		remainder.reg[0] |= dividend[i];
		if (remainder[degree])
		{
			remainder -= divisor;
			quotient.SetBit(i);
		}
	}
}

PolynomialMod2 operator/(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	PolynomialMod2 remainder, quotient;
	PolynomialMod2::Divide(remainder, quotient, a, b);
	return quotient;
}

PolynomialMod2 operator%(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	PolynomialMod2 remainder, quotient;
	PolynomialMod2::Divide(remainder, quotient, a, b);
	return remainder;
}

PolynomialMod2& PolynomialMod2::operator<<=(unsigned int n)
{
	if (!reg.size)
		return *this;

	int i;
	word u;
	word carry=0;
	word *r=reg;

	if (n==1)	// special case code for most frequent case
	{
		i = reg.size;
		while (i--)
		{
			u = *r;
			*r = (u << 1) | carry;
			carry = u >> (WORD_BITS-1);
			r++;
		}

		if (carry)
		{
			reg.Grow(reg.size+1);
			reg[reg.size-1] = carry;
		}

		return *this;
	}

	int shiftWords = n / WORD_BITS;
	int shiftBits = n % WORD_BITS;

	if (shiftBits)
	{
		i = reg.size;
		while (i--)
		{
			u = *r;
			*r = (u << shiftBits) | carry;
			carry = u >> (WORD_BITS-shiftBits);
			r++;
		}
	}

	if (carry)
	{
		reg.Grow(reg.size+shiftWords+1);
		reg[reg.size-1] = carry;
	}
	else
		reg.Grow(reg.size+shiftWords);

	if (shiftWords)
	{
		for (i = reg.size-1; i>=shiftWords; i--)
			reg[i] = reg[i-shiftWords];
		for (; i>=0; i--)
			reg[i] = 0;
	}

	return *this;
}

PolynomialMod2& PolynomialMod2::operator>>=(unsigned int n)
{
	if (!reg.size)
		return *this;

	int shiftWords = n / WORD_BITS;
	int shiftBits = n % WORD_BITS;

	unsigned i;
	word u;
	word carry=0;
	word *r=reg+reg.size-1;

	if (shiftBits)
	{
		i = reg.size;
		while (i--)
		{
			u = *r;
			*r = (u >> shiftBits) | carry;
			carry = u << (WORD_BITS-shiftBits);
			r--;
		}
	}

	if (shiftWords)
	{
		for (i=0; i<reg.size-shiftWords; i++)
			reg[i] = reg[i+shiftWords];
		for (; i<reg.size; i++)
			reg[i] = 0;
	}

	return *this;
}

PolynomialMod2 PolynomialMod2::operator<<(unsigned int n) const
{
	PolynomialMod2 result(*this);
	return result<<=n;
}

PolynomialMod2 PolynomialMod2::operator>>(unsigned int n) const
{
	PolynomialMod2 result(*this);
	return result>>=n;
}

bool PolynomialMod2::operator!() const
{
	for (unsigned i=0; i<reg.size; i++)
		if (reg[i]) return false;
	return true;
}

bool operator==(const PolynomialMod2 &a, const PolynomialMod2 &b)
{
	unsigned i, smallerSize = STDMIN(a.reg.size, b.reg.size);

	for (i=0; i<smallerSize; i++)
		if (a.reg[i] != b.reg[i]) return false;

	for (i=smallerSize; i<a.reg.size; i++)
		if (a.reg[i] != 0) return false;

	for (i=smallerSize; i<b.reg.size; i++)
		if (b.reg[i] != 0) return false;

	return true;
}

ostream& operator<<(ostream& out, const PolynomialMod2 &a)
{
	// Get relevant conversion specifications from ostream.
	long f = out.flags() & ios::basefield;	// Get base digits.
	int bits, block;
	char suffix;
	switch(f)
	{
	case ios::oct :
		bits = 3;
		block = 4;
		suffix = 'o';
		break;
	case ios::hex :
		bits = 4;
		block = 2;
		suffix = 'h';
		break;
	default :
		bits = 1;
		block = 8;
		suffix = 'b';
	}

	if (!a)
		return out << '0' << suffix;

	SecBlock<char> s(a.BitCount()/bits+1);
	unsigned i;
	const char vec[]="0123456789ABCDEF";

	for (i=0; i*bits < a.BitCount(); i++)
	{
		int digit=0;
		for (int j=0; j<bits; j++)
			digit |= a[i*bits+j] << j;
		s[i]=vec[digit];
	}

	while (i--)
	{
		out << s[i];
		if (i && (i%block)==0)
			out << ',';
	}

	return out << suffix;
}

// ********************************************************

static EuclideanDomainOf<PolynomialMod2>& PolynomialMod2Domain()
{
	static EuclideanDomainOf<PolynomialMod2> domain;
	return domain;
}

GF2NP::GF2NP(const PolynomialMod2 &modulus)
	: QuotientRing<EuclideanDomainOf<PolynomialMod2> >(PolynomialMod2Domain(), modulus), m(modulus.Degree()) 
{
}

// ********************************************************

GF2NT::GF2NT(unsigned int t0, unsigned int t1, unsigned int t2)
	: GF2NP(PolynomialMod2::Trinomial(t0, t1, t2))
	, t0(t0), t1(t1)
	, result((word)0, m)
{
	assert(t0 > t1 && t1 > t2 && t2==0);
}

GF2NT::Element GF2NT::MultiplicativeInverse(const Element &a) const
{
	SecWordBlock T(m_modulus.reg.size * 4);
	word *b = T;
	word *c = T+m_modulus.reg.size;
	word *f = T+2*m_modulus.reg.size;
	word *g = T+3*m_modulus.reg.size;
	unsigned int bcLen=1, fgLen=m_modulus.reg.size;
	unsigned int k=0;

	SetWords(T, 0, 3*m_modulus.reg.size);
	b[0]=1;
	assert(a.reg.size <= m_modulus.reg.size);
	CopyWords(f, a.reg, a.reg.size);
	CopyWords(g, m_modulus.reg, m_modulus.reg.size);

	while (1)
	{
		word t=f[0];
		while (!t)
		{
			ShiftWordsRightByWords(f, fgLen, 1);
			if (c[bcLen-1])
				bcLen++;
			assert(bcLen <= m_modulus.reg.size);
			ShiftWordsLeftByWords(c, bcLen, 1);
			k+=WORD_BITS;
			t=f[0];
		}

		unsigned int i=0;
		while (t%2 == 0)
		{
			t>>=1;
			i++;
		}
		k+=i;

		if (t==1 && CountWords(f, fgLen)==1)
			break;

		if (i==1)
		{
			ShiftWordsRightByBits(f, fgLen, 1);
			t=ShiftWordsLeftByBits(c, bcLen, 1);
		}
		else
		{
			ShiftWordsRightByBits(f, fgLen, i);
			t=ShiftWordsLeftByBits(c, bcLen, i);
		}
		if (t)
		{
			c[bcLen] = t;
			bcLen++;
			assert(bcLen <= m_modulus.reg.size);
		}

		if (f[fgLen-1]==0 && g[fgLen-1]==0)
			fgLen--;

		if (f[fgLen-1] < g[fgLen-1])
		{
			swap(f, g);
			swap(b, c);
		}

		XorWords(f, g, fgLen);
		XorWords(b, c, bcLen);
	}

	while (k >= WORD_BITS)
	{
		word temp = b[0];
		// right shift b
		for (unsigned i=0; i+1<bitsToWords(m); i++)
			b[i] = b[i+1];
		b[bitsToWords(m)-1] = 0;

		if (t0%WORD_BITS)
		{
			b[t0/WORD_BITS-1] ^= temp << t0%WORD_BITS;
			b[t0/WORD_BITS] ^= temp >> (WORD_BITS - t0%WORD_BITS);
		}
		else
			b[t0/WORD_BITS-1] ^= temp;

		if (t1%WORD_BITS)
		{
			b[t1/WORD_BITS-1] ^= temp << t1%WORD_BITS;
			b[t1/WORD_BITS] ^= temp >> (WORD_BITS - t1%WORD_BITS);
		}
		else
			b[t1/WORD_BITS-1] ^= temp;

		k -= WORD_BITS;
	}

	if (k)
	{
		word temp = b[0] << (WORD_BITS - k);
		ShiftWordsRightByBits(b, bitsToWords(m), k);

		if (t0%WORD_BITS)
		{
			b[t0/WORD_BITS-1] ^= temp << t0%WORD_BITS;
			b[t0/WORD_BITS] ^= temp >> (WORD_BITS - t0%WORD_BITS);
		}
		else
			b[t0/WORD_BITS-1] ^= temp;

		if (t1%WORD_BITS)
		{
			b[t1/WORD_BITS-1] ^= temp << t1%WORD_BITS;
			b[t1/WORD_BITS] ^= temp >> (WORD_BITS - t1%WORD_BITS);
		}
		else
			b[t1/WORD_BITS-1] ^= temp;
	}

//	Element result((word)0, m);
	CopyWords(result.reg.ptr, b, result.reg.size);
	return result;
}

GF2NT::Element GF2NT::Multiply(const Element &a, const Element &b) const
{
//	Element result((word)0, m);
	unsigned int aSize = STDMIN(a.reg.size, result.reg.size);
	SetWords(result.reg.ptr, 0, result.reg.size);

	for (int i=m-1; i>=0; i--)
	{
		if (result[m-1])
		{
			ShiftWordsLeftByBits(result.reg.ptr, result.reg.size, 1);
			XorWords(result.reg.ptr, m_modulus.reg, result.reg.size);
		}
		else
			ShiftWordsLeftByBits(result.reg.ptr, result.reg.size, 1);

		if (b[i])
			XorWords(result.reg.ptr, a.reg, aSize);
	}

	if (m%WORD_BITS)
		result.reg.ptr[result.reg.size-1] = (word)Crop(result.reg[result.reg.size-1], m%WORD_BITS);
	return result;
}

GF2NT::Element GF2NT::Reduce(const Element &a) const
{
	SecBlock<word> b(a.reg.size);
	CopyWords(b, a.reg, a.reg.size);

	unsigned i;
	for (i=b.size-1; i>=bitsToWords(m); i--)
	{
		word temp = b[i];

		if (t0%WORD_BITS)
		{
			b[i-t0/WORD_BITS] ^= temp >> t0%WORD_BITS;
			b[i-t0/WORD_BITS-1] ^= temp << (WORD_BITS - t0%WORD_BITS);
		}
		else
			b[i-t0/WORD_BITS] ^= temp;

		if ((t0-t1)%WORD_BITS)
		{
			b[i-(t0-t1)/WORD_BITS] ^= temp >> (t0-t1)%WORD_BITS;
			b[i-(t0-t1)/WORD_BITS-1] ^= temp << (WORD_BITS - (t0-t1)%WORD_BITS);
		}
		else
			b[i-(t0-t1)/WORD_BITS] ^= temp;
	}

	if (i==bitsToWords(m)-1 && m%WORD_BITS)
	{
		word mask = ((word)1<<(m%WORD_BITS))-1;
		word temp = b[i] & ~mask;
		b[i] &= mask;

		b[i-t0/WORD_BITS] ^= temp >> t0%WORD_BITS;

		if ((t0-t1)%WORD_BITS)
		{
			b[i-(t0-t1)/WORD_BITS] ^= temp >> (t0-t1)%WORD_BITS;
			b[i-(t0-t1)/WORD_BITS-1] ^= temp << (WORD_BITS - (t0-t1)%WORD_BITS);
		}
		else
			b[i-(t0-t1)/WORD_BITS] ^= temp;
	}

	SetWords(result.reg.ptr, 0, result.reg.size);
//	Element result((word)0, m);
	CopyWords(result.reg.ptr, b, STDMIN(b.size, result.reg.size));
	return result;
}

class SquareMatrixMod2
{
public:
	SquareMatrixMod2 (unsigned m)
		: m(m), rows(new PolynomialMod2[m]) {}

	SquareMatrixMod2 (const SquareMatrixMod2 &matrix)
		: m(matrix.m), rows(new PolynomialMod2[matrix.m])
	{
		for (unsigned i=0; i<m; i++)
			rows[i] = matrix.rows[i];
	}

	~SquareMatrixMod2()
	{
		delete [] rows;
	}
		

	PolynomialMod2& operator[](unsigned i)
		{return rows[i];}

	const PolynomialMod2& operator[](unsigned i) const
		{return rows[i];}

	SquareMatrixMod2 MultiplicativeInverse() const
	{
		unsigned i, j;
		SquareMatrixMod2 A(*this);
		SquareMatrixMod2 Ainv(m);

		for (i=0; i<m; i++)
			Ainv[i] = PolynomialMod2::Monomial(i);

		for (i=0; i<m; i++)
		{
			for (j=i; j<m; j++)
				if (A[j][i])
				{
					swap(A[j], A[i]);
					swap(Ainv[j], Ainv[i]);
					break;
				}
			
			assert(A[i][i]);

			for (j=0; j<i; j++)
				if (A[j][i])
				{
					A[j] -= A[i];
					Ainv[j] -= Ainv[i];
				}

			for (j=i+1; j<m; j++)
				if (A[j][i])
				{
					A[j] -= A[i];
					Ainv[j] -= Ainv[i];
				}
		}

		return Ainv;
	}

	SquareMatrixMod2 Transpose() const
	{
		// flip the matrix along the diagnal
		SquareMatrixMod2 result(m);
		for (unsigned i=0; i<m; i++)
			for (unsigned j=0; j<m; j++)
				if ((*this)[i][j])
					result[j].SetBit(i);
		return result;
	}

	// this is actually multiplication by transpose of b
	friend PolynomialMod2 operator*(const PolynomialMod2 &a, const SquareMatrixMod2 &b)
	{
		PolynomialMod2 result((word)0, b.m);
		for (unsigned i=0; i<b.m; i++)
			if ((a&b[i]).Parity())
				result.SetBit(i);
		return result;
	}

	const unsigned int m;
	PolynomialMod2 *const rows;
};

GF2NO::GF2NO(unsigned m)
	: m(m), lambda(new unsigned int[m][2])
{
// an entry of 0 means no optimal normal basis
// an entry of 3 means both type I and type II ONB
static const byte ONB_TypeTable[] = {
0,0,3,2,1,2,2,0,0,2,1,2,1,0,2,0,0,0,3,0,0,0,0,2,0,0,2,0,1,2,2,0,0,2,0,2,1,0,0,2,
0,2,0,0,0,0,0,0,0,0,2,2,1,2,0,0,0,0,1,0,1,0,0,0,0,2,1,0,0,2,0,0,0,0,2,0,0,0,0,0,
0,2,1,2,0,0,2,0,0,2,2,0,0,0,0,2,0,0,2,2,1,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,2,
0,0,0,0,0,0,0,0,0,0,1,2,0,0,2,2,0,0,1,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,2,0,0,2,0,
0,0,1,0,0,0,0,0,0,0,0,0,1,2,2,0,0,0,1,2,1,0,0,2,0,0,2,0,0,2,0,2,0,0,2,0,1,0,0,0,
0,0,0,0,0,0,0,0,0,2,3,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1,0,0,0,2,2,0,2,0,0,0,0,0,2,
0,0,0,2,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,2,0,0,2,0,0,0,0,2,0,
0,2,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,2,0,0,0,2,0,0,2,0,0,2,0,0,0,0,0,0,1,0,0,0,
0,0,0,2,0,0,2,0,0,2,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,1,0,1,0,2,0,0,0,2,0,0,0,0,2,
0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,2,0,0,1,0,0,0,0,0,0,0,2,0,1,0,0,0,0,2,0,0,0,0,2,0,
0,0,0,0,0,0,0,0,0,0,2,2,0,2,2,0,0,0,1,2,1,0,0,0,0,0,2,0,0,2,0,2,0,0,0,0,0,0,2,0,
0,2,1,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,2,0,0,2,0,0,0,0,0,0,
0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,2,0,0,0,2,
0,0,1,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,1,0,0,2,0,2,1,0,0,0,0,0,0,0,2,0,1,0,2,0,
0,2,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,2,0,0,0,0,0,0,
0,0,0,0,0,0,2,0,0,0,0,2,1,0,2,2,0,0,3,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,2,
0,2,0,0,0,2,0,0,0,0,2,2,1,2,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,2,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,2,0,0,0,0,0,2,
0,0,0,2,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,0,2,0,0,2,0,0,0,0,0,2,1,0,0,0,
0,2,0,0,0,2,0,0,0,0,0,2,1,0,2,0,0,0,0,2,0,0,0,2,0,2,1,0,0,0,0,2,0,0,0,0,1,0,0,0,
0,0,0,2,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,1,0,1,0,0,2,0,2,2,0,0,0,0,0,
0,0,0,0,0,0,2,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,2,0,0,0,2,0,0,2,0,0,1,0,0,2,
0,0,1,0,0,0,0,0,0,0,0,2,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,0,
0,0,0,2,0,0,0,0,0,0,2,0,0,2,0,2,0,0,2,2,1,0,0,0,0,0,1,0,0,0,2,0,0,2,0,0,0,0,0,0,
0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,2,0,0,0,0,2,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,1,2,0,0,0,0,0,0,2,0,0,0,0,2,0,0,2,0,0,0,0,0,
0,2,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,1,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,1,0,2,0,0,0,0,0,1,0,2,2,
0,2,1,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,2,0,0,0,0,
0,0,0,0,0,0,2,0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,2,
0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,1,2,0,0,0,2,0,0,1,0,2,0,
0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,0,0,2,0,2,0,0,2,2,1,0,2,0,
0,0,1,0,0,0,0,0,0,2,1,0,0,0,0,2,0,0,0,0,1,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0,0,0,0,
0,0,0,2,0,0,0,0,0,2,0,2,0,0,0,0,0,0,2,0,0,2,0,0,0,0,2,0,0,2,0,0,0,2,0,2,0,0,0,2,
0,0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,
0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,2,1,0,0,0,2,0,0,0,0,0,0,0,0,2,
0,0,0,2,0,0,0,0,0,0,1,2,1,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,
0,2,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,2,0,0,0,0,0,2,0,0,0,2,0,2,0,0,0,0,0,0,2,0,
0,0,1,0,0,0,0,0,0,0,1,0,0,2,0,0,0,0,0,2,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,
0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,2,0,2,0,0,0,0,1,0,1,0,0,0,0,2,0,0,0,0,0,2,
0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,2,0,1,2,2,0,0,0,0,0,1,0,0,0,0,2,1,0,0,2,0,0,0,0,0,2,0,0,2,0,
0,0,0,2,0,0,2,0,0,2,0,0,0,2,0,0,0,0,2,2,0,0,0,0,0,2,1,0,0,0,2,2,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,
0,0,0,0,0,2,0,0,0,0,2,0,0,0,2,0,0,0,0,2,1,0,0,2,0,0,3,0,0,0,0,0,0,0,0,0,1,0,0,0,
0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,1,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,2,0,2,2,0,0,0,1,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,2,0,2,0,0,2,2,
0,2,0,0,0,2,0,0,0,0,0,0,1,2,0,0,0,0,1,0,0,0,0,2,0,0,1,0,0,0,0,0,0,0,2,0,1,0,0,0};

	if (m >= sizeof(ONB_TypeTable) || ONB_TypeTable[m]==0)
#ifdef THROW_EXCEPTIONS
		throw FieldNotAvailable();
#else
		abort();
#endif
		
	if (ONB_TypeTable[m]==1)
	{
		unsigned int i, twoexp = 1;
		unsigned int *log2 = new unsigned int[m+1];

		for (i=0; i<m; i++)
			lambda[i][0] = ((m/2+i)%m);

		for (i=0; i<m+1; i++) 
		{
			log2[twoexp] = i;
			twoexp = (twoexp << 1) % (m+1);
		}

		lambda[1][1] = m/2;
		lambda[m/2][1] = 1;
		lambda[log2[m/2+1]][1] = log2[m/2+1];

		for (i=2; i<=m/2; i++) 
		{
			lambda[log2[m-i+2]][1] = log2[i];
			lambda[log2[i]][1] = log2[m-i+2];
		}

		for (i=0; i<m; i++)
		{
			// scale lambda entries to save time in Multiply()
			lambda[i][0] *= bitsToWords(m);
			lambda[i][1] *= bitsToWords(m);
		}

		delete [] log2;
		return;
	}

	// use type II ONB if both type I and type II exist
	assert (ONB_TypeTable[m]==2 || ONB_TypeTable[m]==3);

	unsigned i, j, k;
	PolynomialMod2 f;

	PolynomialMod2 f0 = 1, f1 = 3;
	for (i=1; i<m; i++)
	{
		f = f0 + (f1 << 1);
		f0 = f1;
		f1 = f;
	}

	// construct matrix A whose i-th row is x^(2^i) mod f(x)
	SquareMatrixMod2 A(m);
	A[0] = 2;
	for (i=1; i<m; i++)
		A[i] = A[i-1].Squared()%f;

	// transpose Ainv for faster multiplication
	SquareMatrixMod2 Ainv = A.MultiplicativeInverse().Transpose();	

	// construct matrix T whose i-th row is (x*x^(2^i) mod f(x)) * Ainv;
	SquareMatrixMod2 T(m);
	for (i=0; i<m; i++)
		T[i] = ((A[i] << 1) % f) * Ainv;

	// construct the lambda matrix
	// the rows are compressed since we know only 2 bits are 1 in each row
	for (i=0; i<m; i++)
		for (j=0, k=0; j<m; j++)
			if (T[(m+j-i)%m][(m-i)%m])
			{
				assert(k<2);
				lambda[i][k++] = j*bitsToWords(m);	// scale j here to save time in Multiply()
			}
}

GF2NO::~GF2NO()
{
	delete [] lambda;
}

/*
GF2NO::Element GF2NO::Multiply(const GF2NO::Element &a, const GF2NO::Element &b) const
{
#define modm(x) (y=(x), (y < m ? y : y-m))

	unsigned int y;
	Element c((word)0, m);
	for (unsigned k=0; k<m; k++)
	{
		unsigned bit = a[k] & b[modm(lambda[0][0]+k)];
		for (unsigned i=1; i<m; i++)
			bit ^= a[modm(i+k)] & (b[modm(lambda[i][0]+k)] ^ b[modm(lambda[i][1]+k)]);
		if (bit)
			c.SetBit(k);
	}
	return c;
}
#endif
*/
/*
GF2NO::Element GF2NO::Multiply(const GF2NO::Element &x, const GF2NO::Element &b) const
{
	Element c((word)0, m);
	Element a(x);
	Element *bRotated = new Element[m];
	unsigned i, j;

	bRotated[0] = b;
	for (i=1; i<m; i++)
	{
		bRotated[i] = bRotated[i-1];
		bRotated[i].RotateRightByOne(m);
	}

	c = a & bRotated[lambda[0][0]];
	a.RotateRightByOne(m);

	for (i=1; i<m; i++)
	{
		Element &b1 = bRotated[lambda[i][0]];
		Element &b2 = bRotated[lambda[i][1]];
		for (j=0; j<c.reg.size; j++)
			c.reg[j] ^= a.reg[j] & (b1.reg[j] ^ b2.reg[j]);
		a.RotateRightByOne(m);
	}

	delete [] bRotated;
	return c;
}
*/

static void RotateRightOneBit(word *b, const word *a, unsigned size, unsigned hibit)
{
	word carry=0;
	const word *r=a+size;
	word *t=b+size;

	for (unsigned i=0; i<size; i++)
	{
		r--;
		t--;
		*t = (*r >> 1) | (carry << (WORD_BITS-1));
		carry = *r;
	}

	b[size-1] |= (carry&1) << hibit;
}

static void RotateRightOneWord(word *b, const word *a, unsigned size, unsigned bits)
{
	const word *r=a;
	word *t=b;
	unsigned i=size;

	while (--i)
	{
		*t = *(r+1);
		r++;
		t++;
	}

	if (bits)
	{
		*t = a[0] >> (WORD_BITS-bits);
		*(t-1) |= a[0] << bits;
	}
	else
		*t = a[0];
}

static void RotateRightOneWord(word *a, unsigned size, unsigned bits)
{
	word *r=a;
	const word u=*r;
	unsigned i=size;

	while (--i)
	{
		r++;
		*(r-1) = *(r);
	}

	if (bits)
	{
		*r = u >> (WORD_BITS-bits);
		*(r-1) |= u << bits;
	}
	else
		*r = u;
}

GF2NO::Element GF2NO::Multiply(const GF2NO::Element &a, const GF2NO::Element &b) const
{
	Element c((word)0, m);
	SecBlock<word> aRotated(c.reg.size*(WORD_BITS+m));
	word *const bRotated = aRotated+c.reg.size*WORD_BITS;
	unsigned i, j;
	const unsigned bits = m%32;

	// construct aRotated
	if (a.reg.size >= c.reg.size)
		CopyWords(aRotated, a.reg, c.reg.size);
	else
	{
		CopyWords(aRotated, a.reg, a.reg.size);
		SetWords(aRotated+a.reg.size, 0, c.reg.size-a.reg.size);
	}

	// construct bRotated
	if (b.reg.size >= c.reg.size)
		CopyWords(bRotated, b.reg, c.reg.size);
	else
	{
		CopyWords(bRotated, b.reg, b.reg.size);
		SetWords(bRotated+b.reg.size, 0, c.reg.size-b.reg.size);
	}

	unsigned bound = STDMIN(m,WORD_BITS)*c.reg.size;
	for (i=c.reg.size; i<bound; i+=c.reg.size)
	{
		unsigned hibit = (m-1)%WORD_BITS;
		RotateRightOneBit(aRotated+i, aRotated+i-c.reg.size, c.reg.size, hibit);
		RotateRightOneBit(bRotated+i, bRotated+i-c.reg.size, c.reg.size, hibit);
	}

	bound = m*c.reg.size;
	for (i=WORD_BITS*c.reg.size; i<bound; i+=c.reg.size)
		RotateRightOneWord(bRotated+i, bRotated+i-WORD_BITS*c.reg.size, c.reg.size, bits);

	word *b1 = bRotated + lambda[0][0], *b2;
	word *ar = aRotated, *const cr = c.reg;

	for (j=0; j<c.reg.size; j++)
		cr[j] ^= ar[j] & b1[j];

	bound = STDMIN(m, WORD_BITS);
	for (i=1; i<bound; i++)
	{
		ar = aRotated + c.reg.size*(i%WORD_BITS);
		b1 = bRotated + lambda[i][0];
		b2 = bRotated + lambda[i][1];
		for (j=0; j<c.reg.size; j++)
			cr[j] ^= ar[j] & (b1[j] ^ b2[j]);
	}

	for (; i<m; i++)
	{
		ar = aRotated + c.reg.size*(i%WORD_BITS);
		RotateRightOneWord(ar, c.reg.size, bits);
		b1 = bRotated + lambda[i][0];
		b2 = bRotated + lambda[i][1];
		for (j=0; j<c.reg.size; j++)
			cr[j] ^= ar[j] & (b1[j] ^ b2[j]);
	}

	return c;
}

static void RotateLeftBy(word *result, const word *source, unsigned int n, unsigned int m)
{
	assert(n<m);

	if (n==1)	// special case code for most frequent case
	{
		unsigned i = bitsToWords(m);
		word carry = source[i-1] >> ((m-1)%WORD_BITS);
		const word *r=source;
		word *t=result;
		while (i--)
		{
			*t = (*r << 1) | carry;
			carry = *r >> (WORD_BITS-1);
			r++;
			t++;
		}
		if (m%WORD_BITS)
			*(t-1) = (word)Crop(*(t-1), m%WORD_BITS);
		return;
	}

	unsigned i, p = m-n;
	if (p%WORD_BITS==0)
	{
		for (i=0; i<bitsToWords(n); i++)
			result[i] = source[i+p/WORD_BITS];
	}
	else
	{
		for (i=0; i+1<bitsToWords(n); i++)
		{
			result[i] = source[i+p/WORD_BITS] >> p%WORD_BITS;
			result[i] |= source[i+p/WORD_BITS+1] << (WORD_BITS-p%WORD_BITS);
		}
		result[i] = source[i+p/WORD_BITS] >> p%WORD_BITS;
		if ((n-1)%WORD_BITS+1 > (WORD_BITS-p%WORD_BITS))
			result[i] |= source[i+p/WORD_BITS+1] << (WORD_BITS-p%WORD_BITS);
	}

	if (n%WORD_BITS==0)
	{
		for (i=0; i<bitsToWords(p); i++)
			result[i+n/WORD_BITS] = source[i];
	}
	else
	{
		for (i=0; i+1<bitsToWords(p); i++)
		{
			result[i+n/WORD_BITS] |= source[i] << n%WORD_BITS;
			result[i+n/WORD_BITS+1] |= source[i] >> (WORD_BITS-n%WORD_BITS);
		}
		result[i+n/WORD_BITS] |= source[i] << n%WORD_BITS;
		if ((p-1)%WORD_BITS+1 > (WORD_BITS-n%WORD_BITS))
			result[i+n/WORD_BITS+1] |= source[i] >> (WORD_BITS-n%WORD_BITS);
	}

	if (m%WORD_BITS)
		result[bitsToWords(m)-1] = (word)Crop(result[bitsToWords(m)-1], m%WORD_BITS);
}

inline GF2NO::Element GF2NO::Square(const GF2NO::Element &a) const
{
	PolynomialMod2 result((word)0, m);
	assert(a.reg.size >= result.reg.size);
	RotateLeftBy(result.reg, a.reg, 1, m);
	return result;
}

inline GF2NO::Element GF2NO::RepeatedSquare(const GF2NO::Element &a, unsigned int times) const
{
	PolynomialMod2 result((word)0, m);
	assert(a.reg.size >= result.reg.size);
	RotateLeftBy(result.reg, a.reg, times, m);
	return result;
}

GF2NO::Element GF2NO::MultiplicativeInverse(const GF2NO::Element &a) const
{
	Element result=a;
	result.reg.CleanGrow(bitsToWords(m));
	unsigned int m1=m-1;

	for (signed i=BitPrecision(m1)-2; i>=0; i--)
	{
		unsigned int n = m1 >> i;
		if (n & 1)
			result = Multiply(Square(Multiply(RepeatedSquare(result, n/2), result)), a);
		else
			result = Multiply(result, RepeatedSquare(result, n/2));
	}
	return Square(result);
}
