#ifndef DH_H
#define DH_H

#include "nbtheory.h"

class DH : public PK_WithPrecomputation<KeyAgreementProtocol>
{
public:
	DH(const Integer &p, const Integer &g);
	DH(RandomNumberGenerator &rng, unsigned int pbits);

	DH(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	unsigned int PublicValueLength() const {return p.ByteCount();}
	unsigned int AgreedKeyLength() const {return p.ByteCount();}
	// for compatibility with version 1
	unsigned int OutputLength() const {return p.ByteCount();}

	void Setup(RandomNumberGenerator &rng, byte *publicValue);
	void Agree(const byte *otherPublicValue, byte *agreedKey) const;

	const Integer &Prime() const {return p;}
	const Integer &Generator() const {return g;}

private:
	unsigned int ExponentBitLength() const;

	Integer p, g, x;
	ModExpPrecomputation gpc;
};

#endif
