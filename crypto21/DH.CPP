// dh.cpp - written and placed in the public domain by Wei Dai

#include "nbtheory.h"
#include "asn.h"
#include "dh.h"

DH::DH(const Integer &p, const Integer &g)
    : p(p), g(g), gpc(p, g, ExponentBitLength(), 1)
{
}

DH::DH(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(1, rng, pbits);
    p = pg.Prime();
    g = pg.Generator();
	gpc.Precompute(p, g, ExponentBitLength(), 1);
}

DH::DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
{
    PrimeAndGenerator pg(1, rng, pbits, qbits);
    p = pg.Prime();
    g = pg.Generator();
	gpc.Precompute(p, g, ExponentBitLength(), 1);
}

DH::DH(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    g.BERDecode(seq);
	gpc.Precompute(p, g, ExponentBitLength(), 1);
}

void DH::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
}

void DH::Precompute(unsigned int precomputationStorage)
{
	gpc.Precompute(p, g, ExponentBitLength(), precomputationStorage);
}

void DH::LoadPrecomputation(BufferedTransformation &bt)
{
	gpc.Load(p, bt);
}

void DH::SavePrecomputation(BufferedTransformation &bt) const
{
	gpc.Save(bt);
}

void DH::Setup(RandomNumberGenerator &rng, byte *publicValue)
{
    x.Randomize(rng, ExponentBitLength());
    Integer y = gpc.Exponentiate(x);
    y.Encode(publicValue, OutputLength());
}

void DH::Agree(const byte *otherPublicValue, byte *out) const
{
    Integer otherPub(otherPublicValue, OutputLength());
    Integer agreedKey = a_exp_b_mod_c(otherPub, x, p);
    agreedKey.Encode(out, OutputLength());
}

unsigned int DH::ExponentBitLength() const
{
	return 2*DiscreteLogWorkFactor(p.BitCount());
}
