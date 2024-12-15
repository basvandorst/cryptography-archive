#ifndef PKCSPAD_H
#define PKCSPAD_H

#include "cryptlib.h"
#include "pubkey.h"

class PKCS_EncryptionPaddingScheme : public PaddingScheme
{
public:
	unsigned int MaxInputLength(unsigned int paddedLength) const {return paddedLength-11;}

	void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const;
	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;
};

class PKCS_SignaturePaddingScheme : public PaddingScheme
{
public:
	unsigned int MaxInputLength(unsigned int paddedLength) const {return paddedLength-11;}

	void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const;
	unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const;
};

#endif
