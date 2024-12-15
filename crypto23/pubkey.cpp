#include "pch.h"
#include "pubkey.h"
#include "misc.h"

#define INSTANTIATE_PUBKEY_TEMPLATES_MACRO(E, S, F, I)	\
template class DecryptorTemplate<E, I>;\
template class EncryptorTemplate<E, F>;\
template class SignerTemplate<S, I>;\
template class VerifierTemplate<S, F>

template <class P, class T>
unsigned int CryptoSystemBaseTemplate<P, T>::MaxPlainTextLength() const
{
	return pad.MaxInputLength(PaddedBlockLength());
}

template <class P, class T>
unsigned int CryptoSystemBaseTemplate<P, T>::CipherTextLength() const
{
	return f.MaxImage().ByteCount();
}

template <class P, class T>
unsigned int DecryptorTemplate<P,T>::Decrypt(const byte *cipherText, byte *plainText)
{
	SecByteBlock paddedBlock(PaddedBlockLength());
	f.CalculateInverse(Integer(cipherText, CipherTextLength())).Encode(paddedBlock, PaddedBlockLength());
	return pad.Unpad(paddedBlock, PaddedBlockLength(), plainText);
}

template <class P, class T>
void EncryptorTemplate<P,T>::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
	assert(plainTextLength <= MaxPlainTextLength());

	SecByteBlock paddedBlock(PaddedBlockLength());
	pad.Pad(rng, plainText, plainTextLength, paddedBlock, PaddedBlockLength());
	f.ApplyFunction(Integer(paddedBlock, PaddedBlockLength())).Encode(cipherText, CipherTextLength());
}

template <class P, class T>
void SignerTemplate<P,T>::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLength, byte *signature)
{
	assert(messageLength <= MaxMessageLength());

	SecByteBlock paddedBlock(PaddedBlockLength());
	pad.Pad(rng, message, messageLength, paddedBlock, PaddedBlockLength());
	f.CalculateInverse(Integer(paddedBlock, PaddedBlockLength())).Encode(signature, SignatureLength());
}

template <class P, class T>
unsigned int VerifierTemplate<P,T>::Recover(const byte *signature, byte *recoveredMessage)
{
	SecByteBlock paddedBlock(PaddedBlockLength());
	f.ApplyFunction(Integer(signature, SignatureLength())).Encode(paddedBlock, PaddedBlockLength());
	return pad.Unpad(paddedBlock, PaddedBlockLength(), recoveredMessage);
}
