#ifndef OAEP_H
#define OAEP_H

#include "cryptlib.h"
#include "pubkey.h"

template <class H>
class CompressorGenerator
{
public:
	CompressorGenerator(unsigned int k0, unsigned int k1)
		: k0(k0), k1(k1)
	{
		assert(k0 % 8 == 0);
		assert(k1 % 8 == 0);
	}

	void Generate(const byte *input, byte *output, unsigned int outputLength) const;
	void Compress(const byte *input, unsigned int inputLength, byte *output) const;

private:
	const unsigned int k0, k1;
};

template <class H, unsigned int K0 = 128>
class OAEP : public PaddingScheme
{
public:
	OAEP() : cg(K0, K0) {}
	~OAEP() {}

	unsigned int MaxInputLength(unsigned int paddedLength) const 
		{return paddedLength > 2+K0/8 ? paddedLength-2-K0/8 : 0;}

	void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const;
	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;

protected:
	void FixedUnpad(const byte *oaepBlock, unsigned int oaepBlockLength, byte *output) const;

	CompressorGenerator<H> cg;
};

// This next class only accepts inputs with input length == MaxInputLength(paddedLength)
// It is useful for SET.
template <class H, unsigned int K0 = 128>
class OAEP_FIXED : public OAEP<H, K0>
{
public:
	unsigned int MaxInputLength(unsigned int paddedLength) const
		{return paddedLength > 1+K0/8 ? paddedLength-1-K0/8 : 0;}

	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;
};

template <class H, unsigned int K0 = 128, unsigned int K1 = 128>
class PSSR : public PaddingScheme
{
public:
	PSSR() : cg(K0, K1) {}
	~PSSR() {}

	unsigned int MaxInputLength(unsigned int paddedLength) const 
		{return paddedLength > K0/8+K1/8+3 ? paddedLength-K0/8-K1/8-3 : 0;}

	void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const;
	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;

protected:
	CompressorGenerator<H> cg;
};

#endif
