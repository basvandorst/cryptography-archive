#ifndef LUC_H
#define LUC_H

#include "pkcspad.h"
#include "integer.h"

class LUCFunction : virtual public TrapdoorFunction
{
public:
	LUCFunction(const Integer &n, const Integer &e) : n(n), e(e) {}
	LUCFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer ApplyFunction(const Integer &x) const;
	Integer MaxPreimage() const {return n-1;}
	Integer MaxImage() const {return n-1;}

protected:
	LUCFunction() {}	// to be used only by InvertableLUCFunction
	Integer n, e;	// these are only modified in constructors
};

class InvertableLUCFunction : public LUCFunction, public InvertableTrapdoorFunction
{
public:
	InvertableLUCFunction(const Integer &n, const Integer &e,
						  const Integer &p, const Integer &q, const Integer &u);
	// generate a random private key
	InvertableLUCFunction(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17);
	InvertableLUCFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer CalculateInverse(const Integer &x) const;

protected:
	Integer p, q, u;
};

class LUCPrivateKey : public DecryptorTemplate<PKCS_EncryptionPaddingScheme, InvertableLUCFunction>,
					  public SignerTemplate<PKCS_SignaturePaddingScheme, InvertableLUCFunction>
{
public:
	LUCPrivateKey(const Integer &n, const Integer &e, const Integer &p, const Integer &q, const Integer &u)
		: PublicKeyBaseTemplate<InvertableLUCFunction>(
			InvertableLUCFunction(n, e, p, q, u)) {}

	LUCPrivateKey(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17)
		: PublicKeyBaseTemplate<InvertableLUCFunction>(
			InvertableLUCFunction(rng, keybits, eStart)) {}

	LUCPrivateKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<InvertableLUCFunction>(bt) {}
};

class LUCPublicKey : public EncryptorTemplate<PKCS_EncryptionPaddingScheme, LUCFunction>,
					 public VerifierTemplate<PKCS_SignaturePaddingScheme, LUCFunction>
{
public:
	LUCPublicKey(const Integer &n, const Integer &e)
		: PublicKeyBaseTemplate<LUCFunction>(LUCFunction(n, e)) {}

	LUCPublicKey(const LUCPrivateKey &priv)
		: PublicKeyBaseTemplate<LUCFunction>(priv.GetTrapdoorFunction()) {}

	LUCPublicKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<LUCFunction>(bt) {}
};

class LUCELG_Encryptor : public PK_FixedLengthEncryptor
{
public:
	LUCELG_Encryptor(const Integer &p, const Integer &g, const Integer &y);
	LUCELG_Encryptor(BufferedTransformation &bt);

	void DEREncode(BufferedTransformation &bt) const;

	void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);

	unsigned int MaxPlainTextLength() const {return STDMIN(255U, modulusLen-3);}
	unsigned int CipherTextLength() const {return 2*modulusLen;}

protected:
	LUCELG_Encryptor() {}
	void RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const;
	unsigned int ExponentBitLength() const;

	Integer p, g, y;
	unsigned int modulusLen;
};

class LUCELG_Decryptor : public LUCELG_Encryptor, public PK_FixedLengthDecryptor
{
public:
	LUCELG_Decryptor(const Integer &p, const Integer &g, const Integer &y, const Integer &x);
	LUCELG_Decryptor(RandomNumberGenerator &rng, unsigned int pbits);
	// generate a random private key, given p and g
	LUCELG_Decryptor(RandomNumberGenerator &rng, const Integer &p, const Integer &g);

	LUCELG_Decryptor(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	unsigned int Decrypt(const byte *cipherText, byte *plainText);

protected:
	void RawDecrypt(const Integer &a, const Integer &b, Integer &m) const;

	Integer x;
};

class LUCELG_Verifier : public PK_Verifier
{
public:
	LUCELG_Verifier(const Integer &p, const Integer &q, const Integer &g, const Integer &y);
	LUCELG_Verifier(BufferedTransformation &bt);

	void DEREncode(BufferedTransformation &bt) const;

	bool Verify(const byte *message, unsigned int messageLen, const byte *signature);

	// message length for signature is unlimited, but only message digests should be signed
	unsigned int MaxMessageLength() const {return 0xffff;}
	unsigned int SignatureLength() const {return pLen+qLen;}

protected:
	LUCELG_Verifier() {}
	bool RawVerify(const Integer &m, const Integer &a, const Integer &b) const;

	Integer p, q, g, y;
	unsigned int pLen, qLen;
};

class LUCELG_Signer : public LUCELG_Verifier, public PK_Signer
{
public:
	LUCELG_Signer(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x);
	LUCELG_Signer(RandomNumberGenerator &rng, unsigned int pbits);
	// generate a random private key, given p, q and g
	LUCELG_Signer(RandomNumberGenerator &rng, const Integer &p, const Integer &q, const Integer &g);

	LUCELG_Signer(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature);

protected:
	void RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &a, Integer &b) const;

	Integer x;
};

class LUCDIF : public KeyAgreementProtocol
{
public:
	LUCDIF(const Integer &p, const Integer &g);
	LUCDIF(RandomNumberGenerator &rng, unsigned int pbits);

	LUCDIF(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	unsigned int PublicValueLength() const {return p.ByteCount();}
	unsigned int AgreedKeyLength() const {return p.ByteCount();}

	void Setup(RandomNumberGenerator &rng, byte *publicValue);
	void Agree(const byte *otherPublicValue, byte *agreedKey) const;

private:
	unsigned int ExponentBitLength() const;

	Integer p, g, x;
};

#endif
