#ifndef BIGNUM_H
#define BIGNUM_H

#include <memory.h>

#include "cryptlib.h"
#include "misc.h"
#include "mpilib.h"
#include "mpiio.h"

enum RandomNumberType {ANY, ODD, PRIME, BLUMINT};

class MPIRegister : public SecBlock<unit>
{
public:
    MPIRegister() : SecBlock<unit>(MAX_UNIT_PRECISION) {}
};

class ostream;

// a class wrapper for Philip Zimmermann's MPI library
class bignum
{
public:
    /*  Various constructor functions. */
    bignum()
        {}

    bignum(unsigned long value);

    bignum(const char *str)
        {str2reg(reg, str);}

    enum Signedness{UNSIGNED, SIGNED};

    // convert a big-endian byte array to bignum
    bignum(const byte *encodedBignum, unsigned int byteCount, Signedness s=UNSIGNED)
        {Decode(encodedBignum, byteCount, s);}

    bignum(const byte *BEREncodedInteger)
        {BERDecode(BEREncodedInteger);}

    bignum(BufferedTransformation &bt)
        {BERDecode(bt);}

    bignum(RandomNumberGenerator &rng, unsigned int bitcount)
        {Randomize(rng, bitcount);}

    bignum(RandomNumberGenerator &rng, const bignum &min, const bignum &max, RandomNumberType rnType=ANY)
        {Randomize(rng, min, max, rnType);}

    bignum(const bignum& t)
        {memcpy(reg, t.reg, MAX_BYTE_PRECISION);}

    // encode bignum as a big-endian byte array, returns size of output
    unsigned int Encode(byte *output) const;
    // use this to make sure output size is exactly outputLen
    unsigned int Encode(byte *output, unsigned int outputLen) const;

    void Decode(const byte *input, unsigned int inputLen, Signedness=UNSIGNED);

    // encode bignum using Distinguished Encoding Rules, returns size of output
    unsigned int DEREncode(byte *output) const;
    unsigned int DEREncode(BufferedTransformation &bt) const;

    void BERDecode(const byte *input);
    void BERDecode(BufferedTransformation &bt);

    void Randomize(RandomNumberGenerator &rng, unsigned int bitcount);
    void Randomize(RandomNumberGenerator &rng, const bignum &min, const bignum &max);
    void Randomize(RandomNumberGenerator &rng, const bignum &min, const bignum &max, RandomNumberType rnType);

    unsigned int ByteCount() const
    {
        return countbytes(reg);
    }

    int BitCount() const
    {
        return countbits(reg);
    }

    /*  Various member unary operator functions. */

    bignum&  operator++()
    {
        mp_inc(reg);
        return *this;
    }

    bignum&  operator--()
    {
        mp_dec(reg);
        return *this;
    }

    int	     operator!() const
    {
        return testeq(reg.ptr, 0);
    }

    /*  Various member binary operator functions. */

    bignum&  operator=(const bignum& t)
    {
        memcpy(reg, t.reg, MAX_BYTE_PRECISION);
        return *this;
    }

    bignum&  operator+=(const bignum& t)
    {
        mp_add(reg, t.reg);
        return *this;
    }

    bignum&  operator-=(const bignum& t)
    {
        mp_sub(reg, t.reg);
        return *this;
    }

    bignum&  operator*=(const bignum& t)
    {
        *this = (*this) * t;
        return *this;
    }

    bignum&  operator/=(const bignum& t)
    {
        *this = (*this) / t;
        return *this;
    }

    bignum&  operator%=(const bignum& t)
    {
        *this = (*this) % t;
        return *this;
    }

    bignum&  operator<<=(unsigned int);
    bignum&  operator>>=(unsigned int);

    unsigned MaxBitPrecision() const {return MAX_BIT_PRECISION;}

    // returns the n-th bit, n=0 being the least significant bit
    int operator[](unsigned int n) const;

    /*  Various friend operator function. */

    friend bignum operator+(bignum a, const bignum &b)
    {
        return (a+=b);
    }

    friend bignum operator-(bignum a, const bignum &b)
    {
        return (a-=b);
    }

    friend bignum operator*(const bignum &a, const bignum &b);
    friend bignum operator/(const bignum &a, const bignum &b);
    friend bignum operator%(const bignum &a, const bignum &b);
    friend bignum operator/(const bignum &a, word16 b);
    friend word16 operator%(const bignum &a, word16 b);

//    friend bignum operator^(bignum, bignum);		// Bitwise exclusive or.
//    friend bignum operator&(bignum, bignum);		// Bitwise and.
//    friend bignum operator|(bignum, bignum);		// Bitwise or.

    friend bignum operator>>(bignum a, unsigned int n)
        {return (a>>=n);}

    friend bignum operator<<(bignum a, unsigned int n)
        {return (a<<=n);}

    void Negate() {mp_neg(reg);}

    friend bignum operator-(bignum a)
    {
        a.Negate();
        return a;
    }

    friend int    operator==(const bignum &a, const bignum &b)
    {
        return (memcmp(a.reg.ptr, b.reg.ptr, MAX_BYTE_PRECISION)==0);
    }

    friend int    operator!=(const bignum& a, const bignum& b)
    {
        return (memcmp(a.reg.ptr, b.reg.ptr, MAX_BYTE_PRECISION)!=0);
    }

    friend int    operator>(const bignum& a, const bignum& b)
    {
        return (mp_compare(a.reg, b.reg)>0);
    }

    friend int    operator>=(const bignum& a, const bignum& b)
    {
        return (mp_compare(a.reg, b.reg)>=0);
    }

    friend int	  operator<(const bignum& a, const bignum& b)
    {
        return (mp_compare(a.reg, b.reg)<0);
    }

    friend int	  operator<=(const bignum& a, const bignum& b)
    {
        return (mp_compare(a.reg, b.reg)<=0);
    }

    friend bignum a_times_b_mod_c(const bignum &x, const bignum& y, const bignum& m);
    friend bignum a_exp_b_mod_c(const bignum &x, const bignum& e, const bignum& m);

    class DivideErr {};

    friend void Divide(bignum &r, bignum &q,
                       const bignum &a, const bignum &d,
                       Signedness s=SIGNED);
    // POST: (a == d*q + r) && (0 <= r < abs(d))
    // note that the results are different from MPILIB's for negative numbers

    friend boolean Negative(const bignum &a)
    {
        return mp_tstminus(a.reg.ptr);
    }

    friend bignum Abs(bignum a)
    {
        mp_abs(a.reg.ptr);
        return a;
    }

    friend ostream& operator<<(ostream& out, const bignum &a);

    unit lsUnit()   {return lsunit(reg.ptr);}

private:
    MPIRegister reg;
};

#endif

