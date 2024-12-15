#ifndef DSA_H
#define DSA_H

#include "cryptlib.h"
#include "nbtheory.h"

const int MIN_DSA_PRIME_LENGTH = 512;
const int MAX_DSA_PRIME_LENGTH = 1024;

// both seedLength and primeLength are in bits, but seedLength should
// be a multiple of 8
boolean GenerateDSAPrimes(byte *seed, unsigned int seedLength, int &counter,
                          Integer &p, unsigned int primeLength, Integer &q);

class DSAPublicKey : public PK_VerifierWithPrecomputation
{
public:
    // you can use the default copy constructor to make a DSAPublicKey out of a DSAPrivateKey
    DSAPublicKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y);
	DSAPublicKey(BufferedTransformation &storedKey);

    void DEREncode(BufferedTransformation &bt) const;

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxMessageLength() const {return 20;}
    unsigned int SignatureLength() const {return 40;}

protected:
    DSAPublicKey() {}
    boolean RawVerify(const Integer &h, const Integer &r, const Integer &s) const;
    friend boolean DSAValidate();

    Integer p, q, g, y;           // these are only modified in constructors
	ModExpPrecomputation gpc, ypc;
};

class DSAPrivateKey : public DSAPublicKey, public PK_SignerWithPrecomputation
{
public:
    DSAPrivateKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x);

    // generate a random private key
    DSAPrivateKey(RandomNumberGenerator &rng, int keybits);
    // generate a random private key, given p, q, and g
    DSAPrivateKey(RandomNumberGenerator &rng, const Integer &p, const Integer &q, const Integer &g);
    DSAPrivateKey(BufferedTransformation &storedKey);

    void DEREncode(BufferedTransformation &bt) const;

    void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature);

protected:
    void RawSign(const Integer &k, const Integer &h, Integer &r, Integer &s) const;
    friend boolean DSAValidate();

    Integer x;
};

#endif
