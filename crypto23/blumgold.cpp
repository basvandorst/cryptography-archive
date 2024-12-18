// blumgold.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "blumgold.h"
#include "asn.h"
#include "nbtheory.h"
#include "blumshub.h"

BlumGoldwasserPublicKey::BlumGoldwasserPublicKey(const Integer &n)
	: n(n), modulusLen(n.ByteCount())
{
}

BlumGoldwasserPublicKey::BlumGoldwasserPublicKey(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	n.BERDecode(seq);
	modulusLen = n.ByteCount();
}

void BlumGoldwasserPublicKey::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	n.DEREncode(seq);
}

unsigned int BlumGoldwasserPublicKey::MaxPlainTextLength(unsigned int cipherTextLength) const
{
	return cipherTextLength > modulusLen ? cipherTextLength - modulusLen : 0;
}

unsigned int BlumGoldwasserPublicKey::CipherTextLength(unsigned int plainTextLength) const
{
	return modulusLen + plainTextLength;
}

void BlumGoldwasserPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *input, unsigned int inputLen, byte *output)
{
	Integer seed(rng, 2, n-2);
	PublicBlumBlumShub bbs(n, seed);
	bbs.ProcessString(output+modulusLen, input, inputLen);
	bbs.modn.Square(bbs.current).Encode(output, modulusLen);
}

// *****************************************************************************
// private key operations:

BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(const Integer &n, const Integer &p, const Integer &q, const Integer &u)
	: BlumGoldwasserPublicKey(n),
	  p(p), q(q), u(u)
{
	assert(n == p*q);
	assert(u == EuclideanMultiplicativeInverse(p, q));
}

// generate a random private key
BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(RandomNumberGenerator &rng, unsigned int keybits)
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
	modulusLen = n.ByteCount();
}

BlumGoldwasserPrivateKey::BlumGoldwasserPrivateKey(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	n.BERDecode(seq);
	modulusLen = n.ByteCount();

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
	p.DEREncode(seq);
	q.DEREncode(seq);
	u.DEREncode(seq);
}

unsigned int BlumGoldwasserPrivateKey::Decrypt(const byte *input, unsigned int cipherTextLength, byte *output)
{
	if (cipherTextLength <= modulusLen)
		return 0;

	Integer xt(input, modulusLen);
	PublicBlumBlumShub bbs(n, Integer::Zero());
	unsigned int plainTextLength = cipherTextLength - modulusLen;
	unsigned int t = ((plainTextLength)*8 + bbs.maxBits-1) / bbs.maxBits;
	Integer dp = a_exp_b_mod_c((p+1)/4, t, p-1);
	Integer dq = a_exp_b_mod_c((q+1)/4, t, q-1);
	Integer xp = a_exp_b_mod_c(xt%p, dp, p);
	Integer xq = a_exp_b_mod_c(xt%q, dq, q);
	bbs.current = CRT(xp, p, xq, q, u);
	bbs.bitsLeft = bbs.maxBits;

	bbs.ProcessString(output, input+modulusLen, plainTextLength);
	return plainTextLength;
}
