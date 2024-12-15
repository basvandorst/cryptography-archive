#ifndef DH_H
#define DH_H

#include "bignum.h"

class DH
{
public:
    DH(const bignum &p, const bignum &g);
    DH(RandomNumberGenerator &rng, unsigned int pbits);
    DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);

    DH(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    // this is the length of both publicValue and agreedKey
    unsigned int OutputLength() const {return p.ByteCount();}

    void Setup(RandomNumberGenerator &rng, byte *publicValue);
    void Agree(const byte *otherPublicValue, byte *agreedKey) const;

private:
    bignum p, g, x;
};

#endif

