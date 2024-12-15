#ifndef INTEGER_H
#define INTEGER_H

#include "cryptlib.h"
#include "misc.h"

class ostream;

class Integer
{
public:
    class DivideByZero : public CryptlibException 
	{
	public: 
		DivideByZero() : CryptlibException("Integer: division by zero") {}
	};

    enum Signedness {UNSIGNED, SIGNED};
	enum Sign {POSITIVE=0, NEGATIVE=1};
	enum RandomNumberType {ANY, ODD, PRIME, BLUMINT};

    Integer()
        : reg(2), sign(POSITIVE)
    {
        reg[0] = reg[1] = 0;
    }

    Integer(const Integer& t)
        : reg(RoundUp(t.WordCount())), sign(t.sign)
    {
        CopyWords(reg, t.reg, reg.size);
    }

    Integer(long value)
        : reg(2)
    {
        if (value >= 0)
            sign = POSITIVE;
        else
        {
            sign = NEGATIVE;
            value = -value;
        }
        reg[0] = word(value);
#ifdef __GNUC__
		reg[1] = 0;
#else
        reg[1] = sizeof(value)>WORD_SIZE ? word(value>>WORD_BITS) : 0;
#endif
    }

	long ConvertToLong() const
	{
		unsigned long value = reg[0];
#ifndef __GNUC__
		value += sizeof(value)>WORD_SIZE ? ((unsigned long)reg[1]<<WORD_BITS) : 0;
#endif
		value = Crop(value, 8*sizeof(value)-1);
		return sign==POSITIVE ? value : -long(value);
	}

    Integer(const char *str);

    // convert a big-endian byte array to Integer
    Integer(const byte *encodedInteger, unsigned int byteCount, Signedness s=UNSIGNED)
        {Decode(encodedInteger, byteCount, s);}

    Integer(const byte *BEREncodedInteger)
        {BERDecode(BEREncodedInteger);}

    Integer(BufferedTransformation &bt)
        {BERDecode(bt);}

    Integer(RandomNumberGenerator &rng, unsigned int bitcount)
        {Randomize(rng, bitcount);}

    Integer(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType=ANY)
        {Randomize(rng, min, max, rnType);}

	static Integer Power2(unsigned int e)
        {Integer r((word)0, bitsToWords(e+1)); r.SetBit(e); return r;}

    static const Integer ZERO, ONE;

    // encode Integer as a big-endian byte array, returns size of output
    unsigned int Encode(byte *output) const;
    // use this to make sure output size is exactly outputLen
    unsigned int Encode(byte *output, unsigned int outputLen) const;

    void Decode(const byte *input, unsigned int inputLen, Signedness=UNSIGNED);

    // encode Integer using Distinguished Encoding Rules, returns size of output
    unsigned int DEREncode(byte *output) const;
    unsigned int DEREncode(BufferedTransformation &bt) const;

    void BERDecode(const byte *input);
    void BERDecode(BufferedTransformation &bt);

    void Randomize(RandomNumberGenerator &rng, unsigned int bitcount);
    void Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max);
    void Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType);

	unsigned int WordCount() const;
    unsigned int ByteCount() const;
    unsigned int BitCount() const;

    Integer&  operator++();
    Integer&  operator--();
    boolean   operator!() const
        {return Negative() ? FALSE : (reg[0]==0 && WordCount()==0);}

    Integer&  operator=(const Integer& t)
    {
        reg.New(RoundUp(t.WordCount()));
        CopyWords(reg, t.reg, reg.size);
        sign = t.sign;
        return *this;
    }

    Integer&  operator+=(const Integer& t);
    Integer&  operator-=(const Integer& t);
    Integer&  operator*=(const Integer& t)  {return *this = *this*t;}
    Integer&  operator/=(const Integer& t)  {return *this = *this/t;}
    Integer&  operator%=(const Integer& t)  {return *this = *this%t;}
    Integer&  operator/=(word t)  {return *this = *this/t;}
    Integer&  operator%=(word t)  {return *this = *this%t;}

    Integer&  operator<<=(unsigned int);
    Integer&  operator>>=(unsigned int);

    // returns the n-th bit, n=0 being the least significant bit
    word operator[](unsigned int n) const
    {
        if (n/WORD_BITS >= reg.size)
            return 0;
        else
            return (reg[n/WORD_BITS] >> (n % WORD_BITS)) & 1;
    }

    void SetBit(unsigned int n)
    {
		reg.CleanGrow(RoundUp(bitsToWords(n+1)));
        reg[n/WORD_BITS] |= (word(1) << (n%WORD_BITS));
    }

    byte GetByte(unsigned int n) const
    {
        if (n/WORD_SIZE >= reg.size)
            return 0;
        else
            return byte(reg[n/WORD_SIZE] >> ((n%WORD_SIZE)*8));
    }

    friend Integer operator+(const Integer &a, const Integer &b);
    friend Integer operator-(const Integer &a, const Integer &b);
    friend Integer operator*(const Integer &a, const Integer &b);
    friend Integer operator/(const Integer &a, const Integer &b);
    friend Integer operator%(const Integer &a, const Integer &b);
    friend Integer operator/(const Integer &a, word b);
    friend word operator%(const Integer &a, word b);

//    friend Integer operator^(Integer, Integer);		// Bitwise exclusive or.
//    friend Integer operator&(Integer, Integer);		// Bitwise and.
//    friend Integer operator|(Integer, Integer);		// Bitwise or.

    friend Integer operator>>(Integer a, unsigned int n)
        {return (a>>=n);}

    friend Integer operator<<(Integer a, unsigned int n)
        {return (a<<=n);}

    void Negate();

    friend Integer operator-(Integer a)
    {
        a.Negate();
        return a;
    }

    int Compare(const Integer& a) const;

    boolean operator==(const Integer &t) const
    {
        return (Compare(t)==0);
    }

    boolean operator!=(const Integer& t) const
    {
        return (Compare(t)!=0);
    }

    boolean operator>(const Integer& t) const
    {
        return (Compare(t)>0);
    }

    boolean operator>=(const Integer& t) const
    {
        return (Compare(t)>=0);
    }

    boolean operator<(const Integer& t) const
    {
        return (Compare(t)<0);
    }

    boolean operator<=(const Integer& t) const
    {
        return (Compare(t)<=0);
    }

    Integer Double() const {return *this << 1;}
    Integer Square() const;
	Integer SquareRoot() const;
	boolean IsSquare() const;

    friend Integer a_times_b_mod_c(const Integer &x, const Integer& y, const Integer& m)
        {return x*y%m;}
    friend Integer a_exp_b_mod_c(const Integer &x, const Integer& e, const Integer& m);

    // returns r and q such that (a == d*q + r) && (0 <= r < abs(d))
    friend void Divide(Integer &r, Integer &q, const Integer &a, const Integer &d);
    friend word ShortDivide(Integer &q, const Integer &a, word d);

	friend Integer EuclideanMultiplicativeInverse(const Integer &a, const Integer &n);

    boolean Negative() const	{return sign == NEGATIVE;}
    boolean Positive() const	{return sign == POSITIVE;}
    Integer AbsoluteValue() const
        {if (Positive()) return *this; else return operator-(*this);}

    friend void swap(Integer &a, Integer &b)
        {swap(a.reg, b.reg); swap(a.sign, b.sign);}

    friend ostream& operator<<(ostream& out, const Integer &a);

private:
    friend class ModularArithmetics;
    friend class MontgomeryRepresentation;
    friend class HalfMontgomeryRepresentation;

    Integer(word value, unsigned int length)
        : reg(RoundUp(length)), sign(POSITIVE)
    {
		reg[0] = value;
		SetWords(reg+1, 0, reg.size-1);
    }

    int PositiveCompare(const Integer &t) const;
	friend void PositiveAdd(Integer &sum, const Integer &a, const Integer &b);
	friend void PositiveSubtract(Integer &diff, const Integer &a, const Integer &b);
	friend void PositiveMultiply(Integer &product, const Integer &a, const Integer &b);
	friend void PositiveDivide(Integer &remainder, Integer &quotient, const Integer &dividend, const Integer &divisor);

	static unsigned int RoundUp(unsigned int n)
	{
        static const unsigned int table[] = {2, 2, 2, 4, 4, 8, 8, 8, 8};
        if (n<=8)
            return table[n];
        else if (n<=16)
            return 16;
        else if (n<=32)
            return 32;
        else if (n<=64)
            return 64;
		else return 1U << BitPrecision(n-1);
	}

    SecWordBlock reg;
    Sign sign;
};

class ModularArithmetics
{
public:
	typedef Integer Element;

	ModularArithmetics(const Integer &modulus)
		: modulus(modulus), result((word)0, modulus.reg.size) {}

	ModularArithmetics(const ModularArithmetics &ma)
		: modulus(ma.modulus), result((word)0, modulus.reg.size) {}

	const Integer& Modulus() const {return modulus;}

    virtual Integer ConvertIn(const Integer &a) const
        {return a%modulus;}

    virtual Integer ConvertOut(const Integer &a) const
		{return a;}

	boolean Equal(const Integer &a, const Integer &b) const
		{return a==b;}

	Integer Identity() const
		{return Integer::ZERO;}

	Integer Add(const Integer &a, const Integer &b) const;

	void Accumulate(Integer &a, const Integer &b) const;

	Integer Inverse(const Integer &a) const;

	Integer Subtract(const Integer &a, const Integer &b) const;

	void Reduce(Integer &a, const Integer &b) const;

	Integer Double(const Integer &a) const
		{return Add(a, a);}

	virtual Integer MultiplicativeIdentity() const
		{return (word)1;}

	virtual Integer Multiply(const Integer &a, const Integer &b) const
		{return a*b%modulus;}

	virtual Integer Square(const Integer &a) const
		{return a.Square()%modulus;}

	virtual boolean IsUnit(const Integer &a) const;

	virtual Integer MultiplicativeInverse(const Integer &a) const;

	Integer Divide(const Integer &a, const Integer &b) const
		{return Multiply(a, MultiplicativeInverse(b));}

    virtual Integer Exponentiate(const Integer &a, const Integer &e) const;

	virtual Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const;

	unsigned int MaxElementBitLength() const
		{return (modulus-1).BitCount();}

	unsigned int MaxElementByteLength() const
		{return (modulus-1).ByteCount();}

protected:
	Integer modulus, result;
};

// do modular arithmetics in Montgomery representation for increased speed
class MontgomeryRepresentation : public ModularArithmetics
{
public:
	MontgomeryRepresentation(const Integer &modulus);	// modulus must be odd

    Integer ConvertIn(const Integer &a) const
        {return (a<<(WORD_BITS*modulus.reg.size))%modulus;}

    Integer ConvertOut(const Integer &a) const;

	Integer MultiplicativeIdentity() const
		{return Integer::Power2(WORD_BITS*modulus.reg.size)%modulus;}

	Integer Multiply(const Integer &a, const Integer &b) const;

	Integer Square(const Integer &a) const;

	Integer MultiplicativeInverse(const Integer &a) const;

    Integer Exponentiate(const Integer &a, const Integer &e) const;

	Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const;

private:
	Integer u;
    SecWordBlock workspace;
};

// another alternative representation, e-mail me if you figure out how it works :)
class HalfMontgomeryRepresentation : public ModularArithmetics
{
public:
	HalfMontgomeryRepresentation(const Integer &modulus);	// modulus must be odd

    Integer ConvertIn(const Integer &a) const
        {return (a<<(WORD_BITS*modulus.reg.size/2))%modulus;}

    Integer ConvertOut(const Integer &a) const;

	Integer MultiplicativeIdentity() const
		{return Integer::Power2(WORD_BITS*modulus.reg.size/2)%modulus;}

	Integer Multiply(const Integer &a, const Integer &b) const;

	Integer Square(const Integer &a) const;

	Integer MultiplicativeInverse(const Integer &a) const;

    Integer Exponentiate(const Integer &a, const Integer &e) const;

	Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const;

private:
	Integer v, u;
    SecWordBlock workspace;
};

class GFP : public ModularArithmetics
{
public:
    GFP(const Integer &modulus)
		: ModularArithmetics(modulus) {}

	boolean IsUnit(const Integer &a) const
        {return !!a;}

	Integer FieldSize() const
		{return modulus;}
};

#endif
