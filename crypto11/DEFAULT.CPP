#include "default.h"
#include <time.h>
#include <string.h>

const int MASH_ITERATIONS=200;

// The purpose of this function Mash() is to take an arbitrary length input
// string and *deterministicly* produce an arbitrary length output string such
// that (1) it looks random, (2) no information about the input is
// deducible from it, and (3) it contains as much entropy as it can hold, or
// the amount of entropy in the input string, whichever is smaller.

void Mash(byte *const in, word16 inLen, byte *out, word16 outLen, int iterations)
{
    unsigned int bufSize = (outLen-1+DefaultHashModule::DIGESTSIZE-((outLen-1)%DefaultHashModule::DIGESTSIZE));

    // ASSERT: bufSize == (the smallest multiple of DIGESTSIZE that is >= outLen)

    byte b[2];
    SecByteBlock buf(bufSize);
    SecByteBlock outBuf(bufSize);
    DefaultHashModule hash;

    for(int i=0; i<outLen; i+=DefaultHashModule::DIGESTSIZE)
    {
        b[0] = (byte) i >> 8;
        b[1] = (byte) i;
        hash.Update(b, 2);
        hash.Update(in, inLen);
        hash.Final((byte *)outBuf+i);
    }

    while (iterations-- > 1)
    {
        memcpy(buf, outBuf, bufSize);
        for (i=0; i<bufSize; i+=DefaultHashModule::DIGESTSIZE)
        {
            b[0] = (byte) i >> 8;
            b[1] = (byte) i;
            hash.Update(b, 2);
            hash.Update(buf, bufSize);
            hash.Final((byte *)outBuf+i);
        }
    }

    memcpy(out, outBuf, outLen);
}

DefaultBlockCipherBase::DefaultBlockCipherBase(const char *passphrase, const byte *salt, CipherDir dir)
{
    int passphraseLength = strlen(passphrase);
    SecByteBlock temp(passphraseLength+SALTLENGTH);
    memcpy(temp, passphrase, passphraseLength);
    memcpy(temp+passphraseLength, salt, SALTLENGTH);

    // mash passphrase and salt together into key and IV
    keyIV = SecAlloc(byte, KEYLENGTH + BLOCKSIZE);
    Mash(temp, passphraseLength + SALTLENGTH, keyIV, KEYLENGTH+BLOCKSIZE, MASH_ITERATIONS);
    if (dir==ENCRYPTION)
        ecb = new Default_ECB_Encryption(keyIV);
    else
        ecb = new Default_ECB_Decryption(keyIV);
}

DefaultBlockCipherBase::~DefaultBlockCipherBase()
{
    delete ecb;
}

DefaultBlockEncryption::DefaultBlockEncryption(const char *passphrase, const byte *salt)
    : DefaultBlockCipherBase(passphrase, salt, ENCRYPTION),
      CBCEncryption(*ecb, keyIV+KEYLENGTH)
{
    SecFree(keyIV, KEYLENGTH + BLOCKSIZE);
}

DefaultBlockDecryption::DefaultBlockDecryption(const char *passphrase, const byte *salt)
    : DefaultBlockCipherBase(passphrase, salt, DECRYPTION),
      CBCDecryption(*ecb, keyIV+KEYLENGTH)
{
    SecFree(keyIV, KEYLENGTH + BLOCKSIZE);
}

DefaultEncryptor::DefaultEncryptor(const char *passphrase, BufferedTransformation *outQueue)
    : DefaultBlockEncryption(passphrase, GenerateSalt(passphrase)),
      BlockEncryptionFilter(*this, outQueue)
{
    outQueue->Put(salt, SALTLENGTH);

    SecByteBlock keyCheck(DefaultHashModule::DIGESTSIZE);
    DefaultHashModule hash;

    hash.Update((byte *)passphrase, strlen(passphrase));
    hash.Update(salt, SALTLENGTH);
    hash.Final(keyCheck);

    assert(BLOCKSIZE <= DefaultHashModule::DIGESTSIZE);
    Put(keyCheck, BLOCKSIZE);

    SecFree(salt, DefaultHashModule::DIGESTSIZE);
}

byte *DefaultEncryptor::GenerateSalt(const char *passphrase)
{
    assert(DefaultHashModule::DIGESTSIZE >= SALTLENGTH);
    salt = SecAlloc(byte, DefaultHashModule::DIGESTSIZE);

    // use hash(passphrase | time | clock) as salt
    DefaultHashModule hash;
    hash.Update((byte *)passphrase, strlen(passphrase));
    time_t t=time(0);
    hash.Update((byte *)&t, sizeof(t));
    clock_t c=clock();
    hash.Update((byte *)&c, sizeof(c));
    hash.Final(salt);

    return salt;
}

DefaultDecryptor::DefaultDecryptor(const char *p, BufferedTransformation *outQueue)
    : Filter(outQueue),
      passphrase(SecAlloc(char, strlen(p)+1)),
      salt(SecAlloc(byte, SALTLENGTH)),
      keyCheck(SecAlloc(byte, BLOCKSIZE))
{
    strcpy(passphrase, p);
    state = WAITING_FOR_KEYCHECK;
    count = 0;
}

DefaultDecryptor::~DefaultDecryptor()
{
    if (state != WAITING_FOR_KEYCHECK)
        delete cipher;
}

void DefaultDecryptor::Put(byte inByte)
{
    if (state==WAITING_FOR_KEYCHECK)
    {
        assert (count < SALTLENGTH+BLOCKSIZE);

        if (count < SALTLENGTH)
            salt[count]=inByte;
        else
            keyCheck[count-SALTLENGTH]=inByte;

        if (++count == SALTLENGTH+BLOCKSIZE)
            CheckKey();
    }
    else
        outQueue->Put(inByte);
}

void DefaultDecryptor::Put(const byte *inString, unsigned int length)
{
    while (state==WAITING_FOR_KEYCHECK && length--)
        Put(*inString++);

    if (length)
        outQueue->Put(inString, length);
}

void DefaultDecryptor::CheckKey()
{
    SecByteBlock check(max((unsigned int)2*BLOCKSIZE, (unsigned int)DefaultHashModule::DIGESTSIZE));

    DefaultHashModule hash;
    hash.Update((byte *)passphrase, strlen(passphrase));
    hash.Update(salt, SALTLENGTH);
    hash.Final(check);

    cipher = new DefaultBlockDecryption(passphrase, salt);
    cipher->ProcessBlock(keyCheck, check+BLOCKSIZE);

    if (memcmp(check, check+BLOCKSIZE, BLOCKSIZE))
        state = KEY_BAD;
    else
        state = KEY_GOOD;

    outQueue = new BlockDecryptionFilter(*cipher, outQueue);
}
