#ifndef BLUMGOLD_H
#define BLUMGOLD_H

#include "cryptlib.h"
#include "integer.h"

class BlumGoldwasserPublicKey : public PK_Encryptor
{
public:
    // you can use the default copy constructor to make a BlumGoldwasserPublicKey
    // out of a BlumGoldwasserPrivateKey
    BlumGoldwasserPublicKey(const Integer &n, unsigned int maxPlainTextLength=16);
    BlumGoldwasserPublicKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);

    unsigned int MaxPlainTextLength() const {return maxPlainTextLen;}
    unsigned int CipherTextLength() const {return modulusLen+maxPlainTextLen+1;}

protected:
    BlumGoldwasserPublicKey() {}

    Integer n;           // these are only modified in constructors
    unsigned int modulusLen, maxPlainTextLen;
};

class BlumGoldwasserPrivateKey : public BlumGoldwasserPublicKey, public PK_Decryptor
{
public:
    BlumGoldwasserPrivateKey(const Integer &n, const Integer &p, const Integer &q, const Integer &u, unsigned int maxPlainTextLength=16);
    // generate a random private key
    BlumGoldwasserPrivateKey(RandomNumberGenerator &rng, unsigned int keybits, unsigned int maxPlainTextLength=16);
    BlumGoldwasserPrivateKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    unsigned int Decrypt(const byte *cipherText, byte *plainText);

protected:
    Integer p, q, u;
};

#endif
