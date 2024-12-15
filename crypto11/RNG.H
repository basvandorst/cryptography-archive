#ifndef RNG_H
#define RNG_H

#include "cryptlib.h"

// linear congruential generator
// originally by William S. England

// do not use for cryptographic purposes

class LC_RNG : public RandomNumberGenerator
{
public:
    LC_RNG(word32 init_seed)
        : seedptr((byte *)&seed) {seed=init_seed;}

    byte GetByte();

private:
    word32 seed;
    byte *const seedptr;

    static const word32 m;
    static const word32 q;
    static const word16 a;
    static const word16 r;
};

// RNG derived from ANSI X9.17 Appendix C

class X917RNG : public RandomNumberGenerator
{
public:
    // cipher will be deleted by destructor
    X917RNG(BlockTransformation *cipher, const byte *seed);
    ~X917RNG();

    byte GetByte();

private:
    BlockTransformation *const cipher;
    const int S;            // blocksize of cipher
    byte *const dtbuf;      // buffer for enciphered timestamp
    byte *const randseed;
    byte *const randbuf;
    int randbuf_counter;    // # of unused bytes left in randbuf
};

#endif

