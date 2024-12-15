// cryptlib.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTLIB_H
#define CRYPTLIB_H

// This file contains the declarations for the abstract base
// classes that provide a uniform interface to this library.

#include "config.h"

class CryptlibException
{
public:
	CryptlibException();
	CryptlibException(const char *message);
	CryptlibException(const CryptlibException &e);
	virtual ~CryptlibException();
	const char* what() const {return message ? message : "unknown exception";}

protected:
	char *message;
};

enum CipherDir {ENCRYPTION, DECRYPTION};

class BlockTransformation
{
public:
    virtual ~BlockTransformation() {}

    // For most BlockTransformation objects, pointers to input and output
    // should to be aligned on 32-bit word boundaries.
    virtual void ProcessBlock(byte *inoutBlock) =0;
    virtual void ProcessBlock(const byte *inBlock, byte *outBlock) =0;

    virtual unsigned int BlockSize() const =0;
};

class StreamCipher
{
public:
    virtual ~StreamCipher() {}

    virtual byte ProcessByte(byte input) =0;

    virtual void ProcessString(byte *outString, const byte *inString, unsigned int length);
    virtual void ProcessString(byte *inoutString, unsigned int length);
};

class RandomAccessStreamCipher : public virtual StreamCipher
{
public:
    virtual ~RandomAccessStreamCipher() {}
    virtual void Seek(unsigned long position) =0;
};

class RandomNumberGenerator
{
public:
    virtual ~RandomNumberGenerator() {}

    virtual byte GetByte() =0;

    // calls GetByte and returns the parity of the random byte
    virtual unsigned int GetBit();

    // get a random 32 bit word in the range min to max, inclusive
    virtual word32 GetLong(word32 min=0, word32 max=0xffffffffL);
    virtual word16 GetShort(word16 min=0, word16 max=0xffff)
        {return (word16)GetLong(min, max);}

    // calls GetByte size times
    virtual void GetBlock(byte *output, unsigned int size);
};

template <class T> void Shuffle(RandomNumberGenerator &rng, T *array, unsigned int size)
{
    while (--size)
        swap(array[size], array[(unsigned int)rng.GetLong(0, size)]);
}

class HashModule
{
public:
    virtual ~HashModule() {}

    virtual void Update(const byte *input, unsigned int length) =0;

    // Final() should reinitialize the module
    // so that a new digest can be calculated without creating another object
    virtual void Final(byte *digest) =0;

    virtual unsigned int DigestSize() const =0;

    virtual void CalculateDigest(byte *digest, const byte *input, int length)
		{Update(input, length); Final(digest);}
};

class MessageAuthenticationCode : public virtual HashModule
{
public:
	virtual ~MessageAuthenticationCode() {}

	virtual boolean Verify(const byte *mac);

    virtual boolean VerifyMAC(const byte *mac, const byte *input, int length)
		{Update(input, length); return Verify(mac);}
};

class BufferedTransformation
{
public:
    virtual ~BufferedTransformation() {}

    // returns number of bytes that is currently ready for output
    virtual unsigned long MaxRetrieveable() =0;

    // moves all of the buffered output to target as input
    virtual void TransferTo(BufferedTransformation &target);
    virtual unsigned int TransferTo(BufferedTransformation &target, unsigned int transferMax);

    virtual boolean Attachable() {return FALSE;}
    virtual void Detach(BufferedTransformation *) {}
    virtual void Attach(BufferedTransformation *) {}
    // calls InputFinished() for all attached objects
    virtual void Close() {InputFinished();}

    // input some bytes for processing
    virtual void Put(byte inByte) =0;
    virtual void Put(const byte *inString, unsigned int length) =0;
    virtual void InputFinished() {}

    void PutShort(word16 value, boolean highFirst=TRUE);
    void PutLong(word32 value, boolean highFirst=TRUE);

    // attempt to retrieve some processed bytes
    // both functions returns the number of bytes actually retrived,
    // which should be the lesser of MaxRetrieveable() and getMax
    virtual unsigned int Get(byte &outByte) =0;
    virtual unsigned int Get(byte *outString, unsigned int getMax) =0;

    int GetShort(word16 &value, boolean highFirst=TRUE);
    int GetLong(word32 &value, boolean highFirst=TRUE);

	virtual unsigned int Peek(byte &outByte) const =0;

    unsigned int Skip(unsigned int skipMax);
};

class PK_CryptoSystem
{
public:
    virtual ~PK_CryptoSystem() {}

    virtual unsigned int MaxPlainTextLength() const =0;
    virtual unsigned int CipherTextLength() const =0;
};

class PK_Encryptor : public virtual PK_CryptoSystem
{
public:
    virtual void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText) =0;
};

class PK_Decryptor : public virtual PK_CryptoSystem
{
public:
    // returns the length of plaintext
    virtual unsigned int Decrypt(const byte *cipherText, byte *plainText) =0;
};

class PK_SignatureSystem
{
public:
    virtual ~PK_SignatureSystem() {};

    virtual unsigned int MaxMessageLength() const =0;
    virtual unsigned int SignatureLength() const =0;
};

class PK_Signer : public virtual PK_SignatureSystem
{
public:
    virtual void Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature) =0;
};

class PK_Verifier : public virtual PK_SignatureSystem
{
public:
    // returns whether the signature is valid
    virtual boolean Verify(const byte *message, unsigned int messageLen, const byte *signature) =0;
};

class KeyAgreementProtocol
{
public:
	virtual ~KeyAgreementProtocol() {}

    virtual unsigned int PublicValueLength() const =0;
	virtual unsigned int AgreedKeyLength() const =0;

    virtual void Setup(RandomNumberGenerator &rng, byte *publicValue) =0;
    virtual void Agree(const byte *otherPublicValue, byte *agreedKey) const =0;
};

class PK_WithPrecomputation
{
public:
	virtual ~PK_WithPrecomputation() {}

	virtual void Precompute(unsigned int precomputationStorage) =0;
	virtual void LoadPrecomputation(BufferedTransformation &storedPrecomputation) =0;
	virtual void SavePrecomputation(BufferedTransformation &storedPrecomputation) const =0;
};

class PK_EncryptorWithPrecomputation : public PK_Encryptor, public virtual PK_WithPrecomputation
{
};

class PK_DecryptorWithPrecomputation : public PK_Decryptor, public virtual PK_WithPrecomputation
{
};

class PK_SignerWithPrecomputation : public PK_Signer, public virtual PK_WithPrecomputation
{
};

class PK_VerifierWithPrecomputation : public PK_Verifier, public virtual PK_WithPrecomputation
{
};

class KeyAgreementProtocolWithPrecomputation : public KeyAgreementProtocol, public virtual PK_WithPrecomputation
{
};

#endif
