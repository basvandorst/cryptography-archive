#include "blumshub.h"

BlumBlumShub::BlumBlumShub(const bignum &p, const bignum &q, const bignum &seed)
    : phi((p-1)*(q-1)),
      n(p*q),
      x0(a_times_b_mod_c(seed, seed, n)),
      maxBits(BitPrecision(n.BitCount())-1)
{
    current = a_times_b_mod_c(x0, x0, n);
    bitsLeft = maxBits;
}

int BlumBlumShub::GetBit()
{
    if (bitsLeft==0)
    {
        current = a_times_b_mod_c(current, current, n);
        bitsLeft = maxBits;
    }

    return current[--bitsLeft];
}

byte BlumBlumShub::GetByte()
{
    byte b=0;
    for (int i=0; i<8; i++)
        b = (b << 1) | BlumBlumShub::GetBit();
    return b;
}

void BlumBlumShub::Seek(unsigned long index)
{
    bignum exp = a_exp_b_mod_c (2, ((index*8) / maxBits + 1), phi);
    current = a_exp_b_mod_c (x0, exp, n);
    bitsLeft = maxBits - int((index*8) % maxBits);
}

