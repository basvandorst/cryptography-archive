#ifndef ELGAMAL_H
#define ELGAMAL_H

#include "bignum.h"

class ElGamalPrivateKey;

class ElGamalPublicKey : public PK_Encryptor, public PK_Verifier
{
public:
    ElGamalPublicKey(const bignum &p, const bignum &g, const bignum &y);
    ElGamalPublicKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);
    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxPlainTextLength() const {return min(255U, modulusLen-2);}
    unsigned int CipherTextLength() const {return 2*modulusLen;}
    unsigned int MaxMessageLength() const {return modulusLen-1;}
    unsigned int SignatureLength() const {return 2*modulusLen;}

protected:
    void RawEncrypt(const bignum &k, const bignum &m, bignum &a, bignum &b) const;
    boolean RawVerify(const bignum &m, const bignum &a, const bignum &b) const;

private:
    friend ElGamalPrivateKey;
    ElGamalPublicKey() {}

    bignum p, g, y;
    unsigned int modulusLen;
};

class ElGamalPrivateKey : public ElGamalPublicKey,
                          public PK_Decryptor, public PK_Signer
{
public:
    ElGamalPrivateKey(const bignum &p, const bignum &g, const bignum &y, const bignum &x);
    ElGamalPrivateKey(RandomNumberGenerator &rng, unsigned int pbits);
    ElGamalPrivateKey(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);
    // generate a random private key, given p and g
    ElGamalPrivateKey(RandomNumberGenerator &rng, const bignum &p, const bignum &g);

    ElGamalPrivateKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    unsigned int Decrypt(const byte *cipherText, byte *plainText);
    void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature);

    unsigned int MaxPlainTextLength() const {return min(255U, modulusLen-2);}
    unsigned int CipherTextLength() const {return 2*modulusLen;}
    unsigned int MaxMessageLength() const {return modulusLen-1;}
    unsigned int SignatureLength() const {return 2*modulusLen;}

protected:
    void RawSign(const bignum &k, const bignum &m, bignum &a, bignum &b) const;
    void RawDecrypt(const bignum &a, const bignum &b, bignum &m) const;

private:
    bignum x;
};

#endif

