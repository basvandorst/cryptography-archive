// blumgold.cpp - written and placed in the public domain by Wei Dai

#include "misc.h"
#include "asn.h"
#include "nbtheory.h"

#include "blumshub.h"
#include "blumgold.h"

BlumGoldwasserPublicKey::BlumGoldwasserPublicKey(const Integer &n, unsigned int maxPlainTextLength)
    : n(n), modulusLen(n.ByteCount()), maxPlainTextLen(maxPlainTextLength)
{
}

BlumGoldwasserPublicKey::BlumGoldwasserPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    n.BERDecode(seq);
	modulusLen = n.ByteCount();

	Integer maxL(seq);
	maxPlainTextLen = (unsigned int)maxL.ConvertToLong();
}

void BlumGoldwasserPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    n.DEREncode(seq);
	Integer(maxPlainTextLen).DEREncode(seq);
}

void BlumGoldwasserPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *input, unsigned int inputLen, byte *output)
{
    assert (inputLen <= MaxPlainTextLength());

    byte *const paddedBlock = output+modulusLen;

    // pad with non-zero random bytes
    for (unsigned i = 0; i < maxPlainTextLen-inputLen; i++)
        while ((paddedBlock[i] = rng.GetByte()) == 0);

    paddedBlock[maxPlainTextLen-inputLen] = 0;     // separator
    memcpy(paddedBlock+maxPlainTextLen-inputLen+1, input, inputLen);

	Integer seed(rng, 2, n-2);
	PublicBlumBlumShub bbs(n, seed);
	bbs.ProcessString(paddedBlock, maxPlainTextLen+1);
	bbs.modn.Square(bbs.current).Encode(output, modulusLen);
}

// *****************************************************************************
// private key operations:

BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(const Integer &n, const Integer &p, const Integer &q, const Integer &u, unsigned int maxPlainTextLength)
    : BlumGoldwasserPublicKey(n, maxPlainTextLength),
	  p(p), q(q), u(u)
{
	assert(n == p*q);
	assert(u == EuclideanMultiplicativeInverse(p, q));
}

// generate a random private key
BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(RandomNumberGenerator &rng, unsigned int keybits, unsigned int maxPlainTextLength)
{
	assert(keybits >= 16);
    // generate 2 random primes of suitable size
	if (keybits%2==0)
	{
		const Integer minP = Integer(182) << (keybits/2-8);
		const Integer maxP = Integer::Power2(keybits/2)-1;
		p.Randomize(rng, minP, maxP, Integer::BLUMINT);
		q.Randomize(rng, minP, maxP, Integer::BLUMINT);
	}
	else
	{
		const Integer minP = Integer::Power2((keybits-1)/2);
		const Integer maxP = Integer(181) << ((keybits+1)/2-8);
		p.Randomize(rng, minP, maxP, Integer::BLUMINT);
		q.Randomize(rng, minP, maxP, Integer::BLUMINT);
	}

	n = p*q;
	u = EuclideanMultiplicativeInverse(p, q);
	maxPlainTextLen = maxPlainTextLength;
	modulusLen = n.ByteCount();
}

BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    n.BERDecode(seq);
    modulusLen = n.ByteCount();

	Integer maxL(seq);
	maxPlainTextLen = (unsigned int)maxL.ConvertToLong();

    p.BERDecode(seq);
    q.BERDecode(seq);
	u.BERDecode(seq);

	assert(n == p*q);
	assert(u == EuclideanMultiplicativeInverse(p, q));
}

void BlumGoldwasserPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
	n.DEREncode(seq);
	Integer(maxPlainTextLen).DEREncode(seq);
    p.DEREncode(seq);
    q.DEREncode(seq);
	u.DEREncode(seq);
}

unsigned int BlumGoldwasserPrivateKey::Decrypt(const byte *input, byte *output)
{
	Integer xt(input, modulusLen);
	PublicBlumBlumShub bbs(n, Integer::ZERO);
	unsigned int t = ((maxPlainTextLen+1)*8 + bbs.maxBits-1) / bbs.maxBits;
	Integer dp = a_exp_b_mod_c((p+1)/4, t, p-1);
	Integer dq = a_exp_b_mod_c((q+1)/4, t, q-1);
	Integer xp = a_exp_b_mod_c(xt%p, dp, p);
	Integer xq = a_exp_b_mod_c(xt%q, dq, q);
	bbs.current = CRT(xp, p, xq, q, u);
    bbs.bitsLeft = bbs.maxBits;

    SecByteBlock paddedBlock(maxPlainTextLen+1);
	bbs.ProcessString(paddedBlock, input+modulusLen, maxPlainTextLen+1);

    // skip past the padding until we find the seperator
    unsigned i=0;
    while (i<maxPlainTextLen+1 && paddedBlock[i++]);
    assert(i==maxPlainTextLen+1 || paddedBlock[i-1]==0);

	unsigned int outputLen = maxPlainTextLen+1-i;
    memcpy (output, paddedBlock+i, outputLen);
    return outputLen;
}
