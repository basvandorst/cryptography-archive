#ifndef DEFAULT_H
#define DEFAULT_H

#include "shs.h"
#include "des.h"
#include "modes.h"
#include "filters.h"
#include "queue.h"

typedef DES_EDE_Encryption Default_ECB_Encryption;
typedef DES_EDE_Decryption Default_ECB_Decryption;
typedef SHS DefaultHashModule;

class DefaultBlockCipherBase
{
public:
    enum {SALTLENGTH=8, BLOCKSIZE=Default_ECB_Encryption::BLOCKSIZE};

protected:
    DefaultBlockCipherBase(const char *, const byte *, CipherDir);
    ~DefaultBlockCipherBase();

    enum {KEYLENGTH=Default_ECB_Encryption::KEYLENGTH};
    byte *keyIV;
    BlockTransformation *ecb;
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

class DefaultEncryptor : private DefaultBlockEncryption,
                         public BlockEncryptionFilter
{
public:
    DefaultEncryptor(const char *passphrase, BufferedTransformation *outQueue=new ByteQueue);

private:
    byte *GenerateSalt(const char *passphrase);
    byte *salt;
};

class DefaultDecryptor : public Filter
{
public:
    DefaultDecryptor(const char *passphrase, BufferedTransformation *outQueue=new ByteQueue);
    ~DefaultDecryptor();

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);

    enum State {WAITING_FOR_KEYCHECK, KEY_GOOD, KEY_BAD};
    State CurrentState() const {return state;}

private:
    void CheckKey();

    enum {SALTLENGTH=DefaultBlockDecryption::SALTLENGTH,
          BLOCKSIZE=DefaultBlockDecryption::BLOCKSIZE};
    State state;
    char *const passphrase;
    byte *const salt;
    byte *const keyCheck;
    int count;
    DefaultBlockDecryption *cipher;
};

#endif

