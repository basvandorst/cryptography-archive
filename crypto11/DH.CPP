#include "nbtheory.h"
#include "asn.h"
#include "dh.h"

DH::DH(const bignum &p, const bignum &g)
    : p(p), g(g)
{
}

DH::DH(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(rng, pbits);
    p = pg.Prime();
    g = pg.Generator();
}

DH::DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
{
    PrimeAndGenerator pg(rng, pbits, qbits);
    p = pg.Prime();
    g = pg.Generator();
}

DH::DH(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    g.BERDecode(seq);
}

void DH::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
}

void DH::Setup(RandomNumberGenerator &rng, byte *publicValue)
{
    x.Randomize(rng, 2, p-2);
    bignum y = a_exp_b_mod_c(g, x, p);
    y.Encode(publicValue, OutputLength());
}

void DH::Agree(const byte *otherPublicValue, byte *out) const
{
    bignum otherPub(otherPublicValue, OutputLength());
    bignum agreedKey = a_exp_b_mod_c(otherPub, x, p);
    agreedKey.Encode(out, OutputLength());
}

