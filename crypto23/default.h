#ifndef DEFAULT_H
#define DEFAULT_H

#include "sha.h"
#include "des.h"
#include "modes.h"
#include "filters.h"

typedef DES_EDE_Encryption Default_ECB_Encryption;
typedef DES_EDE_Decryption Default_ECB_Decryption;
typedef SHA DefaultHashModule;

class DefaultBlockCipherBase
{
public:
	enum {SALTLENGTH=8, BLOCKSIZE=Default_ECB_Encryption::BLOCKSIZE};

protected:
	DefaultBlockCipherBase(const char *, const byte *, CipherDir);

	enum {KEYLENGTH=Default_ECB_Encryption::KEYLENGTH};
	byte *keyIV;
	member_ptr<BlockTransformation> ecb;
};

class DefaultBlockEncryption : public DefaultBlockCipherBase, public CBCEncryption
{
public:
	DefaultBlockEncryption(const char *passphrase, const byte *salt);
};

class DefaultBlockDecryption : public DefaultBlockCipherBase, public CBCDecryption
{
public:
	DefaultBlockDecryption(const char *passphrase, const byte *salt);
};

class DefaultEncryptor : public Filter
{
public:
	DefaultEncryptor(const char *passphrase, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);

private:
	enum {SALTLENGTH=DefaultBlockEncryption::SALTLENGTH,
		  BLOCKSIZE=DefaultBlockEncryption::BLOCKSIZE};

	member_ptr<DefaultBlockEncryption> cipher;
};

class DefaultDecryptor : public Filter
{
public:
	DefaultDecryptor(const char *passphrase, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);

	enum State {WAITING_FOR_KEYCHECK, KEY_GOOD, KEY_BAD};
	State CurrentState() const {return state;}

private:
	void CheckKey();

	enum {SALTLENGTH=DefaultBlockDecryption::SALTLENGTH,
		  BLOCKSIZE=DefaultBlockDecryption::BLOCKSIZE};
	State state;
	SecBlock<char> passphrase;
	SecByteBlock salt, keyCheck;
	int count;
	member_ptr<DefaultBlockDecryption> cipher;
};

#endif
