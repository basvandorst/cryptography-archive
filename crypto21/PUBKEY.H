#ifndef PUBKEY_H
#define PUBKEY_H

#include "integer.h"

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
	PublicKeyBaseTemplate() : f(*(T*)0) {assert(FALSE);}	// should never be called
	unsigned int PaddedBlockLength() const {return f.MaxPreimage().ByteCount();}

	T f;
};

// ********************************************************

template <class P, class T>
class CryptoSystemBaseTemplate : virtual public PK_CryptoSystem, virtual public PublicKeyBaseTemplate<T>
{
public:
    unsigned int MaxPlainTextLength() const {return pad.MaxInputLength(PaddedBlockLength());}
    unsigned int CipherTextLength() const {return f.MaxImage().ByteCount();}

	P pad;

protected:
	CryptoSystemBaseTemplate() {}
};

// ********************************************************

template <class P, class T>
class DecryptorTemplate : public PK_Decryptor, public CryptoSystemBaseTemplate<P, T>
{
public:
    unsigned int Decrypt(const byte *cipherText, byte *plainText);

protected:
	DecryptorTemplate() {}
};

template <class P, class T> unsigned int DecryptorTemplate<P,T>::Decrypt(const byte *cipherText, byte *plainText)
{
	SecByteBlock paddedBlock(PaddedBlockLength());
	f.CalculateInverse(Integer(cipherText, CipherTextLength())).Encode(paddedBlock, PaddedBlockLength());
	return pad.Unpad(paddedBlock, PaddedBlockLength(), plainText);
}

// ********************************************************

template <class P, class T>
class EncryptorTemplate : public PK_Encryptor, public CryptoSystemBaseTemplate<P, T>
{
public:
    void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);

protected:
	EncryptorTemplate() {}
};

template <class P, class T> void EncryptorTemplate<P,T>::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
	assert(plainTextLength <= MaxPlainTextLength());

	SecByteBlock paddedBlock(PaddedBlockLength());
	pad.Pad(rng, plainText, plainTextLength, paddedBlock, PaddedBlockLength());
	f.ApplyFunction(Integer(paddedBlock, PaddedBlockLength())).Encode(cipherText, CipherTextLength());
}

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

// ********************************************************

template <class P, class T>
class SignerTemplate : public PK_Signer, public SignatureSystemBaseTemplate<P, T>
{
public:
	void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLength, byte *signature);

protected:
	SignerTemplate() {}
};

template <class P, class T> void SignerTemplate<P,T>::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLength, byte *signature)
{
	assert(messageLength <= MaxMessageLength());

	SecByteBlock paddedBlock(PaddedBlockLength());
	pad.Pad(rng, message, messageLength, paddedBlock, PaddedBlockLength());
	f.CalculateInverse(Integer(paddedBlock, PaddedBlockLength())).Encode(signature, SignatureLength());
}

// ********************************************************

template <class P, class T>
class VerifierTemplate : public PK_Verifier, public SignatureSystemBaseTemplate<P, T>
{
public:
    boolean Verify(const byte *message, unsigned int messageLength, const byte *signature);

protected:
	VerifierTemplate() {}
};

template <class P, class T> boolean VerifierTemplate<P,T>::Verify(const byte *message, unsigned int messageLength, const byte *signature)
{
    assert(messageLength <= MaxMessageLength());

	SecByteBlock paddedBlock(PaddedBlockLength());
	f.ApplyFunction(Integer(signature, SignatureLength())).Encode(paddedBlock, PaddedBlockLength());
	SecByteBlock m(MaxMessageLength());
	unsigned int mLen = pad.Unpad(paddedBlock, PaddedBlockLength(), m);
    return (mLen==messageLength && memcmp(m, message, mLen)==0);
}

#endif
