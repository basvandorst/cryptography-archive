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

	unsigned int MaxInputLength(unsigned int paddedLength) const 
		{return paddedLength > K0/8+K1/8+3 ? paddedLength-K0/8-K1/8-3 : 0;}

	void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const;
	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;

protected:
	CompressorGenerator<H> cg;
};

// ********************************************************

template <class H> 
void CompressorGenerator<H>::Generate(const byte *input, byte *output, unsigned int outputLength) const
{
	byte b=0;
	H h;
	while (outputLength > h.DigestSize())
	{
		h.Update(input, k1/8);
		h.Update(&b, 1);
		h.Final(output);
		b++;
		outputLength -= h.DigestSize();
		output += h.DigestSize();
	}

	SecByteBlock lastDigest(h.DigestSize());
	h.Update(input, k1/8);
	h.Update(&b, 1);
	h.Final(lastDigest);
	memcpy(output, lastDigest, outputLength);
}

template <class H> 
void CompressorGenerator<H>::Compress(const byte *input, unsigned int inputLength, byte *output) const
{
	H h;
	SecByteBlock digest(h.DigestSize());
	h.CalculateDigest(digest, input, inputLength);
	assert(h.DigestSize() >= k0/8);
	memcpy(output, digest+h.DigestSize()-k0/8, k0/8);
}

// ********************************************************

template <class H, unsigned int K0> 
void OAEP<H,K0>::Pad(RandomNumberGenerator &rng, const byte *input, unsigned int inputLength, byte *oaepBlock, unsigned int oaepBlockLength) const
{
    assert (inputLength <= MaxInputLength(oaepBlockLength));

    oaepBlock[0] = 1;

	const unsigned int Alen = oaepBlockLength-1-K0/8, Blen = K0/8;
	byte *const A = oaepBlock+1;	// masked data
	byte *const B = A+Alen;			// masked salt

	SecByteBlock salt(Blen);
	rng.GetBlock(salt, Blen);
	cg.Generate(salt, A, Alen);

    // pad from the right with 0xff
    for (unsigned int i = inputLength+1; i < Alen; i++)
		A[i] ^= 0xff;

	xorbuf(A, input, inputLength);
	cg.Compress(A, Alen, B);
	xorbuf(B, salt, Blen);
}

template <class H, unsigned int K0> 
void OAEP<H,K0>::FixedUnpad(const byte *oaepBlock, unsigned int oaepBlockLength, byte *t) const
{
	const unsigned int Alen = oaepBlockLength-1-K0/8, Blen = K0/8;
	const byte *const A = oaepBlock+1;	// masked data
	const byte *const B = A+Alen;		// masked salt

	SecByteBlock salt(Blen);
	cg.Compress(A, Alen, salt);
	xorbuf(salt, B, Blen);
	cg.Generate(salt, t, Alen);
	xorbuf(t, A, Alen);
}

template <class H, unsigned int K0> 
unsigned int OAEP<H,K0>::Unpad(const byte *oaepBlock, unsigned int oaepBlockLength, byte *output) const
{
	SecByteBlock t(oaepBlockLength-1-K0/8);
	FixedUnpad(oaepBlock, oaepBlockLength, t);

    // remove padding from the right
    unsigned i=oaepBlockLength-1-K0/8;
    while (i && t[--i]) 
		if (t[i] != 0xff)
			return 0;

	memcpy(output, t, i);
	return i;
}

template <class H, unsigned int K0> 
unsigned int OAEP_FIXED<H,K0>::Unpad(const byte *oaepBlock, unsigned int oaepBlockLength, byte *output) const
{
	FixedUnpad(oaepBlock, oaepBlockLength, output);
	return oaepBlockLength-1-K0/8;
}

// ********************************************************

template <class H, unsigned int K0, unsigned int K1> 
void PSSR<H,K0,K1>::Pad(RandomNumberGenerator &rng, const byte *input, unsigned int inputLength, byte *pssrBlock, unsigned int pssrBlockLength) const
{
    assert (inputLength <= MaxInputLength(pssrBlockLength));

	pssrBlock[0] = 1;

	const unsigned int Alen = K1/8, Clen = K0/8, Blen = pssrBlockLength-1-Alen-Clen;
	byte *const A = pssrBlock+1;	// hash(M || salt)
	byte *const B = A+Alen;			// masked message
	byte *const C = B+Blen;			// masked salt

	SecByteBlock salt(Clen);
	rng.GetBlock(salt, Clen);

	memcpy(B, input, inputLength);
	memcpy(C, salt, Clen);
	B[inputLength] = 0;
	memset(B+inputLength+1, 0xff, Blen-inputLength-1);	// pad from the right with 0xff
	cg.Compress(B, Blen+Clen, A);
	cg.Generate(A, B, Blen+Clen);
	xorbuf(B, input, inputLength);
	xorbuf(C, salt, Clen);
	for (unsigned i = inputLength+1; i<Blen; i++)		// pad it again
		B[i] ^= 0xff;
}

template <class H, unsigned int K0, unsigned int K1> 
unsigned int PSSR<H,K0,K1>::Unpad(const byte *pssrBlock, unsigned int pssrBlockLength, byte *output) const
{
    if (pssrBlock[0] != 1)
        return 0;

	const unsigned int Alen = K1/8, Clen = K0/8, Blen = pssrBlockLength-1-Alen-Clen;
	const byte *const A = pssrBlock+1;	// hash(M || salt)
	const byte *const B = A+Alen;		// masked message
	const byte *const C = B+Blen;		// masked salt

	SecByteBlock t(Alen+Blen+Clen);
	byte *const tB = t+Alen;

	cg.Generate(A, tB, Blen+Clen);
	xorbuf(tB, B, Blen+Clen);
	cg.Compress(tB, Blen+Clen, t);
	if (memcmp(t, A, Alen) != 0)
		return 0;
	
    // remove padding from the right
    unsigned i=Blen;
    while (i && tB[--i])
		if (tB[i] != 0xff)
			return 0;

	memcpy(output, tB, i);
	return i;
}

#endif
