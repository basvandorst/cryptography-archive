#ifndef RSA_H
#define RSA_H

#include "cryptlib.h"
#include "bignum.h"

class RSAPublicKey : public PK_Encryptor, public PK_Decryptor,
                     public PK_Verifier
{
public:
    // you can use the default copy constructor to make a RSAPublicKey
    // out of a RSAPrivateKey
    RSAPublicKey(const bignum &n, const bignum &e);
    RSAPublicKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    // encrypt and decrypt in PKCS #1 format
    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);
    unsigned int Decrypt(const byte *cipherText, byte *plainText);
    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxPlainTextLength() const {return modulusLen-11;}
    unsigned int CipherTextLength() const {return modulusLen;}
    unsigned int MaxMessageLength() const {return modulusLen-11;}
    unsigned int SignatureLength() const {return modulusLen;}

    const bignum& Exponent() const {return e;}
    const bignum& Modulus() const {return n;}

protected:
    void RawEncrypt(const bignum &in, bignum &out) const;

private:
    friend class RSAPrivateKey;
    RSAPublicKey() {}

    bignum n;           // these are only modified in constructors
    bignum e;
    unsigned int modulusLen;
};

class RSAPrivateKey : public RSAPublicKey, public PK_Signer
{
public:
    RSAPrivateKey(const bignum &n, const bignum &e, const bignum &d,
                  const bignum &p, const bignum &q, const bignum &dp, const bignum &dq, const bignum &u);
    // generate a random private key
    RSAPrivateKey(RandomNumberGenerator &rng, int keybits, bignum eStart=17);
    RSAPrivateKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    // encrypt and decrypt in PKCS #1 format
    void Encrypt(const byte *plainText, unsigned int plainTextLength, byte *cipherText);
    unsigned int Decrypt(const byte *cipherText, byte *plainText);

    void Encrypt(RandomNumberGenerator &, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
        {Encrypt(plainText, plainTextLength, cipherText);}

    void Sign(const byte *message, unsigned int messageLen, byte *signature)
        {Encrypt(message, messageLen, signature);}

    void Sign(RandomNumberGenerator &, const byte *message, unsigned int messageLen, byte *signature)
        {Encrypt(message, messageLen, signature);}

    unsigned int MaxMessageLength() const {return modulusLen-11;}
    unsigned int SignatureLength() const {return modulusLen;}

protected:
    void RawEncrypt(const bignum &in, bignum &out) const;

private:
    bignum d;   // these are modified only in constructors
    bignum p;   // p < q
    bignum q;
    bignum dp;
    bignum dq;
    bignum u;
};

#endif

