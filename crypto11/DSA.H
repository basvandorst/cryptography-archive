#ifndef DSA_H
#define DSA_H

#include "cryptlib.h"
#include "bignum.h"

const int MIN_DSA_PRIME_LENGTH = 512;
const int MAX_DSA_PRIME_LENGTH = 1024;

// both seedLength and primeLength are in bits, but seedLength should
// be a multiple of 8
boolean GenerateDSAPrimes(byte *seed, unsigned int seedLength, int &counter,
                          bignum &p, unsigned int primeLength, bignum &q);

class DSAPrivateKey;

class DSAPublicKey : public PK_Verifier
{
public:
    // you can use the default copy constructor to make a DSAPublicKey
    // out of a DSAPrivateKey
    DSAPublicKey(const bignum &p, const bignum &q,
                 const bignum &g, const bignum &y);
    DSAPublicKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxMessageLength() const {return 20;}
    unsigned int SignatureLength() const {return 40;}

protected:
    boolean RawVerify(const bignum &h, const bignum &r, const bignum &s) const;
    friend boolean DSAValidate();

private:
    friend DSAPrivateKey;
    DSAPublicKey() {}

    bignum p;           // these are only modified in constructors
    bignum q;
    bignum g;
    bignum y;
};

class DSAPrivateKey : public DSAPublicKey, public PK_Signer
{
public:
    DSAPrivateKey(const bignum &p, const bignum &q, const bignum &g, const bignum &y, const bignum &x);

    // generate a random private key
    DSAPrivateKey(RandomNumberGenerator &rng, int keybits);
    // generate a random private key, given p, q, and g
    DSAPrivateKey(RandomNumberGenerator &rng, const bignum &p, const bignum &q, const bignum &g);

    DSAPrivateKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature);

    unsigned int MaxMessageLength() const {return 20;}
    unsigned int SignatureLength() const {return 40;}

protected:
    void RawSign(const bignum &k, const bignum &h, bignum &r, bignum &s) const;
    friend boolean DSAValidate();

private:
    bignum x;
};

#endif

