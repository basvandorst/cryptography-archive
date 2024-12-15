// eccrypto.h - written and placed in the public domain by Wei Dai

#ifndef ECCRYPTO_H
#define ECCRTPTO_H

#include "cryptlib.h"
#include "integer.h"

/* The following classes are explicitly instantiated in eccrypto.cpp

	ECPublicKey<EC2N>;
	ECPublicKey<ECP>;
	ECPrivateKey<EC2N>;
	ECPrivateKey<ECP>;
	ECKEP<EC2N>;
	ECKEP<ECP>;
*/

template <class T> class EcPrecomputation;

typedef PK_WithPrecomputation<PK_FixedLengthEncryptor> PKWPFLE;
typedef PK_WithPrecomputation<PK_Verifier> PKWPV;
typedef PK_WithPrecomputation<PK_Signer> PKWPS;

template <class EC> class ECPublicKey : public PKWPFLE, public PKWPV
{
public:
	typedef typename EC::Point Point;

	ECPublicKey(const EC &E, const Point &P, const Point &Q, const Integer &orderP);
	~ECPublicKey();

/*
	// TODO: these are not implemented yet because there is no standard way to encoding EC keys
	ECPublicKey(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;
*/

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);
	bool Verify(const byte *message, unsigned int messageLen, const byte *signature);

	unsigned int MaxPlainTextLength() const {return l-2;}
	unsigned int CipherTextLength() const {return 3*l+1;}
	// message length for signature is unlimited, but only message digests should be signed
	unsigned int MaxMessageLength() const {return 0xffff;}
	unsigned int SignatureLength() const {return 2*f;}

	const Point& BasePoint() const {return P;}
	const Point& PublicPoint() const {return Q;}

protected:
	unsigned ExponentBitLength() const {return n.BitCount();}

	EC ec;
	Point P;
	Point Q;
	Integer n;
	EcPrecomputation<EC> Ppc, Qpc;
	unsigned int l, f;
};

template <class EC> class ECPrivateKey : public ECPublicKey<EC>, public PK_FixedLengthDecryptor, public PKWPS
{
public:
	typedef typename EC::Point Point;

	ECPrivateKey(const EC &E, const Point &P, const Point &Q, const Integer &orderP, const Integer &d)
		: ECPublicKey<EC>(E, P, Q, orderP), d(d) {}
	// generate a random private key
	ECPrivateKey(RandomNumberGenerator &rng, const EC &E, const Point &P, const Integer &orderP)
		: ECPublicKey<EC>(E, P, P, orderP) {Randomize(rng);}
	~ECPrivateKey();

/*
	// TODO: these are not implemented yet because there is no standard way to encoding EC keys
	ECPrivateKey(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;
*/

	unsigned int Decrypt(const byte *cipherText, byte *plainText);
	void Sign(RandomNumberGenerator &, const byte *message, unsigned int messageLen, byte *signature);

protected:
	typedef typename EC::FieldElement FieldElement;
	void Randomize(RandomNumberGenerator &rng);
	Integer d;
};

// Elliptic Curve Key Establishment Protocol
template <class EC> class ECKEP : public PK_WithPrecomputation<KeyAgreementProtocol>, private ECPrivateKey<EC>
{
public:
	typedef typename EC::Point Point;

	ECKEP(const ECPrivateKey<EC> &myPrivKey, const ECPublicKey<EC> &otherPubKey)
		: ECPrivateKey<EC>(myPrivKey), QB(otherPubKey.PublicPoint()) {assert(ec.Equal(P, otherPubKey.BasePoint()));}
	~ECKEP();

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	unsigned int PublicValueLength() const {return 2*l+1;}
	unsigned int AgreedKeyLength() const {return l;}

	void Setup(RandomNumberGenerator &rng, byte *publicValue);
	void Agree(const byte *otherPublicValue, byte *agreedKey) const;

protected:
	Point QB;
	Integer k, x;
};

#endif
