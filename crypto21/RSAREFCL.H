#ifndef RSAREFCL_H
#define RSAREFCL_H

#include "cryptlib.h"

#ifdef USE_RSAREF

#ifdef USE_BIGNUM_WITH_RSAREF
#include "integer.h"
#endif

#include "misc.h"

extern "C" {
#include <global.h>
#include <rsaref.h>
//#include <rsa.h>
}

/*
class RSAREFPrivateKey;

class RSAREFPublicKey
{
public:
#ifdef USE_BIGNUM_WITH_RSAREF
    RSAREFPublicKey(const Integer &n, const Integer &e);
#endif
    RSAREFPublicKey(const RSAREFPrivateKey &);
    RSAREFPublicKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    // encrypt and decrypt in PKCS #1 format
    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);
    unsigned int Decrypt(const byte *cipherText, byte *plainText);

    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxPlainTextLength() const {return CipherTextLength()-11;}
    unsigned int CipherTextLength() const {return (key->bits + 7) / 8;}
    unsigned int MaxMessageLength() const {return MaxPlainTextLength();}
    unsigned int SignatureLength() const {return CipherTextLength();}

#ifdef USE_BIGNUM_WITH_RSAREF
    Integer Exponent() const {return Integer(key->exponent, MAX_RSA_MODULUS_LEN);}
    Integer Modulus() const {return Integer(key->modulus, MAX_RSA_MODULUS_LEN);}
#endif

private:
    SecBlock<R_RSA_PUBLIC_KEY> keyBlock;
    R_RSA_PUBLIC_KEY *const key;
};

class RSAREFPrivateKey
{
public:
#ifdef USE_BIGNUM_WITH_RSAREF
    RSAREFPrivateKey(const Integer &n, const Integer &e, const Integer &d,
                     const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u);
#endif
    // generate a random private key
    RSAREFPrivateKey(RandomNumberGenerator &rng, int keybits, boolean useFermat4=FALSE);
    RSAREFPrivateKey(BufferedTransformation &bt);

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

    unsigned int MaxPlainTextLength() const {return CipherTextLength()-11;}
    unsigned int CipherTextLength() const {return (key->bits + 7) / 8;}
    unsigned int MaxMessageLength() const {return MaxPlainTextLength();}
    unsigned int SignatureLength() const {return CipherTextLength();}

    friend RSAREFPublicKey;    // allow RSAREFPublicKey to grab n and e

private:
    SecBlock<R_RSA_PRIVATE_KEY> keyBlock;
    R_RSA_PRIVATE_KEY *const key;
};
*/

class RSAREF_DH : public KeyAgreementProtocol
{
public:
#ifdef USE_BIGNUM_WITH_RSAREF
    RSAREF_DH(const Integer &p, const Integer &g);
#endif
    // generate random parameters, pbits is length of prime, qbits is length
    // of subprime
    RSAREF_DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits);
    RSAREF_DH(BufferedTransformation &bt);
    ~RSAREF_DH();

    void DEREncode(BufferedTransformation &bt) const;

    unsigned int PublicValueLength() const {return params.primeLen;}
    unsigned int AgreedKeyLength() const {return params.primeLen;}
	// for compatibility with version 1
    unsigned int OutputLength() const {return params.primeLen;}

    void Setup(RandomNumberGenerator &rng, byte *publicValue);
    void Agree(const byte *otherPublicValue, byte *agreedKey) const;

private:
    unsigned int BERDecode(BufferedTransformation &bt);
    R_DH_PARAMS params;
    SecByteBlock privateValue;
};

#endif  // USE_RSAREF

#endif  // RSAREFCL_H

