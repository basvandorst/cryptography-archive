#include "pch.h"
#include "eccrypto.h"
#include "ec2n.h"
#include "ecp.h"

template class ECPublicKey<EC2N>;
template class ECPublicKey<ECP>;
template class ECPrivateKey<EC2N>;
template class ECPrivateKey<ECP>;
template class ECKEP<EC2N>;
template class ECKEP<ECP>;

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

template <class EC> ECPublicKey<EC>::ECPublicKey(const EC &E, const Point &P, const Point &Q, const Integer &orderP)
	: ec(E), P(P), Q(Q), n(orderP), 
	  Ppc(E, P, ExponentBitLength(), 1), Qpc(E, Q, ExponentBitLength(), 1)
{
	l = ec.GetField().MaxElementByteLength();
	f = n.ByteCount();
}

template <class EC> ECPublicKey<EC>::~ECPublicKey()
{
}

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
	Point kP = Ppc.Multiply(k);
	Point kQ = Qpc.Multiply(k);

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

template <class EC> bool ECPublicKey<EC>::Verify(const byte *message, unsigned int messageLen, const byte *signature)
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

template <class EC> ECPrivateKey<EC>::~ECPrivateKey()
{
}

template <class EC> void ECPrivateKey<EC>::Randomize(RandomNumberGenerator &rng)
{
	d.Randomize(rng, 2, n-2, Integer::ANY);
	Q = Ppc.Multiply(d);
	Qpc.Precompute(Q, ExponentBitLength(), 1);
}

template <class EC> unsigned int ECPrivateKey<EC>::Decrypt(const byte *cipherText, byte *plainText)
{
	if (cipherText[0]!=0)	// TODO: no support for point compression yet
		return 0;

	FieldElement kPx(cipherText+1, l);
	FieldElement kPy(cipherText+l+1, l);
	Point kP(kPx, kPy);
	Point kQ(ec.Multiply(d, kP));

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

template <class EC> ECKEP<EC>::~ECKEP()
{
}

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
	Point kP = Ppc.Multiply(k);
	publicValue[0] = 0;
	kP.x.Encode(publicValue+1, l);
	x.Decode(publicValue+1, l);
	kP.y.Encode(publicValue+l+1, l);
}

template <class EC> void ECKEP<EC>::Agree(const byte *otherPublicValue, byte *agreedKey) const
{
	FieldElement Rx(otherPublicValue+1, l);
	FieldElement Ry(otherPublicValue+l+1, l);
	Point R(Rx, Ry);
	Integer s = k+d*x*ConvertToInteger(Q.x) % n;
	Integer r = s*ConvertToInteger(Rx)*ConvertToInteger(QB.x) % n;
	ec.CascadeMultiply(s, R, r, QB).x.Encode(agreedKey, l);
}
