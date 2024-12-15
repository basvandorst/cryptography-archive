#ifndef BLUMSHUB_H
#define BLUMSHUB_H

#include "cryptlib.h"
#include "bignum.h"

class BlumBlumShub : public RandomNumberGenerator,
                     public RandomAccessStreamCipher
{
public:
    // Make sure key1 and key2 are both Blum integers at least 512 bits long,
    // and seed should be about as big as p*q
    BlumBlumShub(const bignum &key1, const bignum &key2, const bignum &seed);

    int GetBit();
    byte GetByte();

    byte ProcessByte(byte input)
        {return (input ^ GetByte());}

    void Seek(unsigned long index);

private:
    const bignum phi, n, x0;
    bignum current;
    const int maxBits;
    int bitsLeft;
};

#endif

