#include "misc.h"
#include "bignum.h"
#include "nbtheory.h"
#include "asn.h"

#include <iostream.h>
#include <memory.h>
#include <assert.h>

// this variable is just a hack to automaticly set the global precision
int init_global_precision = (set_precision(MAX_UNIT_PRECISION), 0);

bignum::bignum(unsigned long value)
{
    unit * r = reg;
    unitfill0(r, global_precision);
    make_lsbptr(r, global_precision);
    for (int i=0; i<sizeof(value); i+=BYTES_PER_UNIT)
        *post_higherunit(r) = unit(value >> i*8);
}

void bignum::Decode(const byte *from, unsigned int frombytes, Signedness s)
{
	const int unsigned tobytes = MAX_BYTE_PRECISION;
    const byte filler = ((s==SIGNED) && (*from & 0x80)) ? 0xff : 0;

	if (tobytes > frombytes) {
		memset(reg, filler, tobytes - frombytes);
		memcpy((byte *)reg.ptr + tobytes - frombytes, from, frombytes);
	} else {
		memcpy(reg, from + frombytes - tobytes, tobytes);
	}
#ifdef LITTLE_ENDIAN
	hiloswap((byte *)reg.ptr, tobytes);
#endif
}

unsigned int bignum::Encode(byte *to) const
{
    const boolean negative = Negative(*this);
    const byte filler = negative ? 0xff : 0;
    unsigned int i=0;

#ifdef LITTLE_ENDIAN
    byte *const end = (byte *) reg.ptr;
    for (const byte *p = end+MAX_BYTE_PRECISION-1; p>end; --p)
        if (*p != filler)
            break;
#else
    byte *const end = (byte *) reg.ptr + MAX_BYTE_PRECISION - 1;
    for (const byte *p = (byte *) reg.ptr; p<end; ++p)
        if (*p != filler)
            break;
#endif

    if (((*p & 0x80) && !negative) || (!(*p & 0x80) && negative))
        to[i++] = filler;

    to[i++] = *p;

    while (p != end)
#ifdef LITTLE_ENDIAN
        to[i++] = *--p;
#else
        to[i++] = *++p;
#endif
    return i;
}

unsigned int bignum::Encode(byte *to, unsigned int tobytes) const
{
	const unsigned int frombytes = MAX_BYTE_PRECISION;
    const byte filler = Negative(*this) ? 0xff : 0;

#ifndef LITTLE_ENDIAN
	if (tobytes > frombytes) {
		memset(to, filler, tobytes-frombytes);
		memcpy(to + tobytes - frombytes, (byte *)reg.ptr, frombytes);
	} else {
		memcpy(to, (byte *)reg.ptr + frombytes - tobytes, tobytes);
	}
#else
	if (tobytes > frombytes) {
		memcpy(to, reg, frombytes);
		memset(to + frombytes, filler, tobytes-frombytes);
	} else {
		memcpy(to, reg, tobytes);
	}
	hiloswap(to, tobytes);
#endif
    return tobytes;
}

unsigned int bignum::DEREncode(byte *output) const
{
    unsigned int i=0;
    output[i++] = INTEGER;
    SecByteBlock buf(MAX_BYTE_PRECISION);
    const unsigned int bc = Encode(buf);
    i += DERLengthEncode(bc, output+i);
    memcpy(output+i, buf, bc);
    return i+bc;
}

unsigned int bignum::DEREncode(BufferedTransformation &bt) const
{
    bt.Put(INTEGER);
    SecByteBlock buf(MAX_BYTE_PRECISION);
    const unsigned int bc = Encode(buf);
    unsigned int lengthBytes = DERLengthEncode(bc, bt);
    bt.Put(buf, bc);
    return 1+lengthBytes+bc;
}

void bignum::BERDecode(const byte *input)
{
    if (*input++ != INTEGER)
        BERDecodeError();
    int bc;
    if (!(*input & 0x80))
        bc = *input++;
    else
    {
        int lengthBytes = *input++ & 0x7f;
        if (lengthBytes > 2)
            BERDecodeError();
        bc = *input++;
        if (lengthBytes > 1)
            bc = (bc << 8) | *input++;
    }
    if (bc > units2bytes(MAX_UNIT_PRECISION))
        BERDecodeError();
    Decode(input, bc, SIGNED);
}

void bignum::BERDecode(BufferedTransformation &bt)
{
    byte b;
    if (!bt.Get(b) || b != INTEGER)
        BERDecodeError();

    unsigned int bc;
    BERLengthDecode(bt, bc);
    if (bc > units2bytes(MAX_UNIT_PRECISION))
        BERDecodeError();

    SecByteBlock buf(MAX_BYTE_PRECISION);

    if (bc != bt.Get(buf, bc))
        BERDecodeError();
    Decode(buf, bc, SIGNED);
}

void bignum::Randomize(RandomNumberGenerator &rng, unsigned int nbits)
{
    const unsigned int nbytes = nbits/8 + 1;
    assert(nbytes <= MAX_BYTE_PRECISION);

    SecByteBlock buf(nbytes);
    rng.GetBlock(buf, nbytes);
    buf[0] = (byte)Crop(buf[0], nbits % 8);
    Decode(buf, nbytes, UNSIGNED);
}

void bignum::Randomize(RandomNumberGenerator &rng, const bignum &min, const bignum &max)
{
    bignum range = max - min;
    const unsigned int nbits = range.BitCount();

    do
    {
        Randomize(rng, nbits);
    }
    while (*this > range);

    *this += min;
}

void bignum::Randomize(RandomNumberGenerator &rng, const bignum &min, const bignum &max, RandomNumberType rnType)
{
    while (1)   // loop until a suitable rn has been generated
    {
        Randomize(rng, min, max);

        switch (rnType)
        {
            case ANY:
                return;

            case ODD:
                lsunit(reg.ptr) |= 1;
                return;

            case PRIME:
            case BLUMINT:
                if (NextPrime(*this, max, rnType==BLUMINT))
                    return;
        }
    }
}

ostream& operator<<(ostream& out, const bignum &a)
{
    // Get relevant conversion specifications from ostream.
    long f = out.flags() & ios::basefield;	// Get base digits.
    int base;
    switch(f)
    {
    	case ios::dec :	base = 10;
    		        break;
    	case ios::oct :	base = 8;
    		        break;
    	case ios::hex :	base = 16;
    		        break;
    	default :	base = 10;
    }

    int owidth = out.width(0);
    char fill_char = out.fill();

    char s[MAX_BIT_PRECISION/2];
    reg2str(s, a.reg, base);
    for (int i=strlen(s); i<owidth; i++)
        out << fill_char;
    return (out << s);
}

bignum& bignum::operator<<=(unsigned int n)
{
    if (n)
    {
        unsigned int shiftUnits = n / UNITSIZE;
        unsigned int shiftBits = n % UNITSIZE;

        unit *ptr = msbptr(reg.ptr, MAX_UNIT_PRECISION);

        for (unsigned int i=shiftUnits; i<(MAX_UNIT_PRECISION-1); i++)
        {
            *ptr = (*(ptr-tohigher(shiftUnits)) << shiftBits) |
                   (*(ptr-tohigher(shiftUnits+1)) >> (UNITSIZE-shiftBits));
            pre_lowerunit(ptr);
        }

        if (shiftUnits < MAX_UNIT_PRECISION)
        {
            *ptr = (*(ptr-tohigher(shiftUnits)) << shiftBits);
            pre_lowerunit(ptr);
        }

        for (i=0; i<min(shiftUnits, (unsigned)MAX_UNIT_PRECISION); i++)
            *post_lowerunit(ptr) = 0;
    }
    return *this;
}

bignum& bignum::operator>>=(unsigned int n)
{
    if (n)
    {
        unsigned int shiftUnits = n / UNITSIZE;
        unsigned int shiftBits = n % UNITSIZE;

        unit *ptr = lsbptr(reg.ptr, MAX_UNIT_PRECISION);

        for (unsigned int i=shiftUnits; i<(MAX_UNIT_PRECISION-1); i++)
        {
            *ptr = (*(ptr+tohigher(shiftUnits)) >> shiftBits) |
                   (*(ptr+tohigher(shiftUnits+1)) << (UNITSIZE-shiftBits));
            pre_higherunit(ptr);
        }

        if (shiftUnits < MAX_UNIT_PRECISION)
        {
            *ptr = (*(ptr+tohigher(shiftUnits)) >> shiftBits);
            pre_higherunit(ptr);
        }

        for (i=0; i<min(shiftUnits, (unsigned)MAX_UNIT_PRECISION); i++)
            *post_higherunit(ptr) = 0;
    }
    return *this;
}

int bignum::operator[](unsigned int index) const
{
#ifdef LITTLE_ENDIAN
    unit value = reg[index/UNITSIZE];
#else
    unit value = reg[MAX_UNIT_PRECISION - 1 - index/UNITSIZE];
#endif
    return int((value >> (index % UNITSIZE)) & 1);
}

bignum operator*(const bignum &a, const bignum &b)
{
    bignum result;

    // mp_mult is very slow for negative numbers
    if (Negative(a))
    {
        if (Negative(b))
            mp_mult(result.reg, (-a).reg, (-b).reg);
        else
        {
            mp_mult(result.reg, (-a).reg, b.reg);
            mp_neg(result.reg);
        }
    }
    else
    {
        if (Negative(b))
        {
            mp_mult(result.reg, a.reg, (-b).reg);
            mp_neg(result.reg);
        }
        else
            mp_mult(result.reg, a.reg, b.reg);
    }

    return result;
}

bignum operator/(const bignum &a, const bignum &b)
{
    bignum remainder, quotient;
    Divide(remainder, quotient, a, b);
    return quotient;
}

bignum operator%(const bignum &a, const bignum &b)
{
    bignum remainder, quotient;
    Divide(remainder, quotient, a, b);
    return remainder;
}

bignum operator/(const bignum &a, word16 b)
{
    bignum quotient;
    mp_shortdiv(quotient.reg, a.reg, b);
    return quotient;
}

word16 operator%(const bignum &a, word16 b)
{
    bignum quotient;
    return mp_shortdiv(quotient.reg, a.reg, b);
}

bignum a_times_b_mod_c(const bignum &x, const bignum& y, const bignum& m)
{
    bignum result;
    stage_modulus(m.reg);
    mp_modmult(result.reg, x.reg, y.reg);
    modmult_burn();
    return result;
}

bignum a_exp_b_mod_c(const bignum &x, const bignum& e, const bignum& m)
{
    bignum result;
    mp_modexp(result.reg, x.reg, e.reg, m.reg);
    return result;
}

void Divide(bignum &remainder, bignum &quotient,
                   const bignum &dividend, const bignum &divisor,
                   bignum::Signedness s)
{
    int status;

    if (s==bignum::SIGNED)
    {
        boolean negative = FALSE;
        bignum tempDividend(dividend);
        bignum tempDivisor(divisor);

        if (Negative(dividend))
        {
        	mp_neg(tempDividend.reg);
            negative = (!negative);
        }

        if (Negative(divisor))
        {
        	mp_neg(tempDivisor.reg);
            negative = (!negative);
        }

        status = mp_udiv(remainder.reg, quotient.reg, tempDividend.reg, tempDivisor.reg);

        if (negative)
        {
            quotient.Negate();
            if (!!remainder)
            {
                --quotient;
                remainder.Negate();
                remainder += tempDivisor;
            }
        }
    }
    else
        status = mp_udiv(remainder.reg, quotient.reg, dividend.reg, divisor.reg);

#ifdef THROW_EXCEPTIONS
    if (status < 0)
        throw bignum::DivideErr();
#endif
    assert(status==0);
}

