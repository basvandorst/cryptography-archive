// eccrypto.h - written and placed in the public domain by Wei Dai

#ifndef ECCRYPTO_H
#define ECCRTPTO_H

#include "cryptlib.h"
#include "ecbasic.h"

template <class EC> class ECPublicKey : public PK_EncryptorWithPrecomputation, public PK_VerifierWithPrecomputation
{
public:
    ECPublicKey(const EC &E, const EC::Point &P, const EC::Point &Q, const Integer &orderP)
		: ec(E), P(P), Q(Q), n(orderP), 
		  Ppc(E, P, ExponentBitLength(), 1), Qpc(E, Q, ExponentBitLength(), 1)
	{
		// can't put this in the initialization list because of a compiler bug in MSVC 2.2
		l = ec.GetField().MaxElementByteLength();
		f = n.ByteCount();
	}

/*
	// TODO: these are not implemented yet because there is no standard way to encoding EC keys
    ECPublicKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;
*/

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);
    boolean Verify(const byte *message, unsigned int messageLen, const byte *signature);

    unsigned int MaxPlainTextLength() const {return l-2;}
    unsigned int CipherTextLength() const {return 3*l+1;}
	// message length for signature is unlimited, but only message digests should be signed
    unsigned int MaxMessageLength() const {return 0xffff;}
    unsigned int SignatureLength() const {return 2*f;}

	const EC::Point& BasePoint() const {return P;}
	const EC::Point& PublicPoint() const {return Q;}

protected:
	unsigned ExponentBitLength() const {return n.BitCount();}

	const EC ec;
    const EC::Point P;
	EC::Point Q;	// should be const, but ECPrivateKey constructor needs to modify it
	const Integer n;
	ECPrecomputation<EC> Ppc, Qpc;
	unsigned int l, f;
};

template <class EC> class ECPrivateKey : public ECPublicKey<EC>, public PK_Decryptor, public PK_SignerWithPrecomputation
{
public:
    ECPrivateKey(const EC &E, const EC::Point &P, const EC::Point &Q, const Integer &orderP, const Integer &d)
		: ECPublicKey<EC>(E, P, Q, orderP), d(d) {}
    // generate a random private key
    ECPrivateKey(RandomNumberGenerator &rng, const EC &E, const EC::Point &P, const Integer &orderP)
		: ECPublicKey<EC>(E, P, P, orderP), d(rng, 2, n-2, Integer::ANY) 
	{
		Q = Ppc.Multiply(d);
		Qpc.Precompute(Q, ExponentBitLength(), 1);
	}

/*
	// TODO: these are not implemented yet because there is no standard way to encoding EC keys
    ECPrivateKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;
*/

    unsigned int Decrypt(const byte *cipherText, byte *plainText);
    void Sign(RandomNumberGenerator &, const byte *message, unsigned int messageLen, byte *signature);

protected:
    Integer d;
};

// Elliptic Curve Key Establishment Protocol
template <class EC> class ECKEP : public KeyAgreementProtocolWithPrecomputation, private ECPrivateKey<EC>
{
public:
	ECKEP(const ECPrivateKey<EC> &myPrivKey, const ECPublicKey<EC> &otherPubKey)
		: ECPrivateKey<EC>(myPrivKey), QB(otherPubKey.PublicPoint()) {assert(ec.Equal(P, otherPubKey.BasePoint()));}

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

    unsigned int PublicValueLength() const {return 2*l+1;}
	unsigned int AgreedKeyLength() const {return l;}

    void Setup(RandomNumberGenerator &rng, byte *publicValue);
    void Agree(const byte *otherPublicValue, byte *agreedKey) const;

protected:
	EC::Point QB;
	Integer k, x;
};

// ******************************************************************

inline Integer ConvertToInteger(const PolynomialMod2 &x)
{
	unsigned int l = x.ByteCount();
	SecByteBlock temp(l);
	x.Encode(temp, l);
	return Integer(temp, l);
}

inline Integer ConvertToInteger(const Integer &x)
{
	return x;
}

// ******************************************************************

template <class EC> void ECPublicKey<EC>::Precompute(unsigned int precomputationStorage)
{
	Ppc.Precompute(P, ExponentBitLength(), precomputationStorage);
	Qpc.Precompute(Q, ExponentBitLength(), precomputationStorage);
}

template <class EC> void ECPublicKey<EC>::LoadPrecomputation(BufferedTransformation &bt)
{
	Ppc.Load(bt);
	Qpc.Load(bt);
}

template <class EC> void ECPublicKey<EC>::SavePrecomputation(BufferedTransformation &bt) const
{
	Ppc.Save(bt);
	Qpc.Save(bt);
}

template <class EC> void ECPublicKey<EC>::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
    assert (plainTextLength <= MaxPlainTextLength());

	Integer k(rng, 2, n-2, Integer::ANY);
	EC::Point kP = Ppc.Multiply(k);
	EC::Point kQ = Qpc.Multiply(k);

	cipherText[0] = 0;
	kP.x.Encode(cipherText+1, l);
	kP.y.Encode(cipherText+l+1, l);
	kQ.x.Encode(cipherText+2*l+1, l);

	SecByteBlock paddedBlock(l-1);
    // pad with non-zero random bytes
    for (unsigned i = 0; i < l-2-plainTextLength; i++)
        while ((paddedBlock[i] = rng.GetByte()) == 0);
	paddedBlock[l-2-plainTextLength] = 0;
	memcpy(paddedBlock+l-1-plainTextLength, plainText, plainTextLength);
    xorbuf(cipherText+2*l+2, paddedBlock, l-1);
}

template <class EC> boolean ECPublicKey<EC>::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
	assert (messageLen <= MaxMessageLength());

	Integer e(message, messageLen);
	Integer r(signature, f);
	Integer s(signature+f, f);

	// convert kP.x into a Integer
	Integer x = ConvertToInteger(Ppc.CascadeMultiply(s, Qpc, r).x);
	return !!r && r == (x+e)%n;
}

// ******************************************************************

/*
template <class EC> ECPrivateKey<EC>::ECPrivateKey(const EC &E, const EC::Point &P, const EC::Point &Q, const Integer &orderP, const Integer &d)
	: ECPublicKey<EC>(E, P, Q, orderP), d(d)
{
}

template <class EC> ECPrivateKey<EC>::ECPrivateKey(RandomNumberGenerator &rng, const EC &E, const EC::Point &P, const Integer &orderP)
	: ECPublicKey<EC>(E, P, P, orderP), d(rng, 2, n-2, Integer::ANY)
{
	Q = ec.Multiply(d, P);
}
*/

template <class EC> unsigned int ECPrivateKey<EC>::Decrypt(const byte *cipherText, byte *plainText)
{
	if (cipherText[0]!=0)	// TODO: no support for point compression yet
		return 0;

	EC::FieldElement kPx(cipherText+1, l);
	EC::FieldElement kPy(cipherText+l+1, l);
	EC::Point kP(kPx, kPy);
	EC::Point kQ(ec.Multiply(d, kP));

	SecByteBlock paddedBlock(l-1);
    kQ.x.Encode(paddedBlock, l-1);
    xorbuf(paddedBlock, cipherText+2*l+2, l-1);

	unsigned i;
	// remove padding
	for (i=0; i<l-1; i++)
		if (paddedBlock[i] == 0)			// end of padding reached
		{
			i++;
			break;
		}

	memcpy(plainText, paddedBlock+i, l-1-i);
	return l-1-i;
}

template <class EC> void ECPrivateKey<EC>::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature)
{
	Integer r, s;

	do
	{
		Integer k(rng, 2, n-2, Integer::ANY);
		// convert kP.x into an Integer
		Integer x = ConvertToInteger(Ppc.Multiply(k).x);
		Integer e(message, messageLen);
		r = (x+e)%n;
		s = (k-d*r)%n;
	} while (!r);

	r.Encode(signature, f);
	s.Encode(signature+f, f);
}

// ******************************************************************

template <class EC> void ECKEP<EC>::Precompute(unsigned int precomputationStorage)
{
	Ppc.Precompute(P, ExponentBitLength(), precomputationStorage);
}

template <class EC> void ECKEP<EC>::LoadPrecomputation(BufferedTransformation &bt)
{
	Ppc.Load(bt);
}

template <class EC> void ECKEP<EC>::SavePrecomputation(BufferedTransformation &bt) const
{
	Ppc.Save(bt);
}

template <class EC> void ECKEP<EC>::Setup(RandomNumberGenerator &rng, byte *publicValue)
{
	k.Randomize(rng, 2, n-2, Integer::ANY);
	EC::Point kP = Ppc.Multiply(k);
	publicValue[0] = 0;
	kP.x.Encode(publicValue+1, l);
	x.Decode(publicValue+1, l);
	kP.y.Encode(publicValue+l+1, l);
}

template <class EC> void ECKEP<EC>::Agree(const byte *otherPublicValue, byte *agreedKey) const
{
	EC::FieldElement Rx(otherPublicValue+1, l);
	EC::FieldElement Ry(otherPublicValue+l+1, l);
	EC::Point R(Rx, Ry);
	Integer s = k+d*x*ConvertToInteger(Q.x) % n;
	Integer r = s*ConvertToInteger(Rx)*ConvertToInteger(QB.x) % n;
	ec.CascadeMultiply(s, R, r, QB).x.Encode(agreedKey, l);
}

#endif
