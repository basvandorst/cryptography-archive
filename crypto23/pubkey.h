#ifndef PUBKEY_H
#define PUBKEY_H

#include "cryptlib.h"
#include <assert.h>

class Integer;

class TrapdoorFunction
{
public:
	virtual ~TrapdoorFunction() {}

	virtual Integer ApplyFunction(const Integer &x) const =0;
	virtual Integer MaxPreimage() const =0;
	virtual Integer MaxImage() const =0;
};

class InvertableTrapdoorFunction : virtual public TrapdoorFunction
{
public:
	virtual Integer CalculateInverse(const Integer &x) const =0;
};

class PaddingScheme
{
public:
	virtual ~PaddingScheme() {}

	virtual unsigned int MaxInputLength(unsigned int paddedLength) const =0;

	virtual void Pad(RandomNumberGenerator &rng, const byte *raw, unsigned int inputLength, byte *padded, unsigned int paddedLength) const =0;
	// returns length of raw
	virtual unsigned int Unpad(const byte *padded, unsigned int paddedLength, byte *raw) const =0;
};

// ********************************************************

template <class T>
class PublicKeyBaseTemplate
{
public:
	PublicKeyBaseTemplate(const T &f) : f(f) {}
	PublicKeyBaseTemplate(BufferedTransformation &bt) : f(bt) {}
	void DEREncode(BufferedTransformation &bt) const {f.DEREncode(bt);}

	const T & GetTrapdoorFunction() const {return f;}

protected:
	// a hack to avoid having to write constructors for non-concrete derived classes
	PublicKeyBaseTemplate() : f(*(T*)0) {assert(false);}	// should never be called
	unsigned int PaddedBlockLength() const {return f.MaxPreimage().ByteCount();}

	T f;
};

// ********************************************************

template <class P, class T>
class CryptoSystemBaseTemplate : virtual public PK_FixedLengthCryptoSystem, virtual public PublicKeyBaseTemplate<T>
{
public:
	unsigned int MaxPlainTextLength() const;
	unsigned int CipherTextLength() const;

	P pad;

protected:
	CryptoSystemBaseTemplate() {}
};

template <class P, class T>
class DecryptorTemplate : public PK_FixedLengthDecryptor, public CryptoSystemBaseTemplate<P, T>
{
public:
	~DecryptorTemplate() {}
	unsigned int Decrypt(const byte *cipherText, byte *plainText);

protected:
	DecryptorTemplate() {}
};

template <class P, class T>
class EncryptorTemplate : public PK_FixedLengthEncryptor, public CryptoSystemBaseTemplate<P, T>
{
public:
	~EncryptorTemplate() {}
	void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);

protected:
	EncryptorTemplate() {}
};

// ********************************************************

template <class P, class T>
class SignatureSystemBaseTemplate : virtual public PK_SignatureSystem, virtual public PublicKeyBaseTemplate<T>
{
public:
	unsigned int MaxMessageLength() const {return pad.MaxInputLength(PaddedBlockLength());}
	unsigned int SignatureLength() const {return f.MaxImage().ByteCount();}

	P pad;

protected:
	SignatureSystemBaseTemplate() {}
};

template <class P, class T>
class SignerTemplate : public PK_Signer, public SignatureSystemBaseTemplate<P, T>
{
public:
	~SignerTemplate() {}
	void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLength, byte *signature);

protected:
	SignerTemplate() {}
};

template <class P, class T>
class VerifierTemplate : public PK_VerifierWithRecovery, public SignatureSystemBaseTemplate<P, T>
{
public:
	~VerifierTemplate() {}
	unsigned int Recover(const byte *signature, byte *recoveredMessage);

protected:
	VerifierTemplate() {}
};

#endif
