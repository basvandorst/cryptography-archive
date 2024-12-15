#ifndef BLOWFISH_H
#define BLOWFISH_H

#include "cryptlib.h"
#include "misc.h"

class Blowfish : public BlockTransformation
{
public:
    Blowfish(const byte *key_string, unsigned int keylength, CipherDir direction);

    void ProcessBlock(byte * inoutBlock)
        {Blowfish::ProcessBlock(inoutBlock, inoutBlock);}
    void ProcessBlock(const byte *inBlock, byte *outBlock);

    // value of KEYLENGTH is default only
    enum {KEYLENGTH=16, BLOCKSIZE=8, ROUNDS=16};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    void crypt_block(const word32 in[2], word32 out[2]) const;

    static const word32 p_init[ROUNDS+2];
    static const word32 s_init[4*256];
    SecBlock<word32> pbox, sbox;
};

class BlowfishEncryption : public Blowfish
{
public:
    BlowfishEncryption(byte *key_string, unsigned int keylength=KEYLENGTH)
        : Blowfish(key_string, keylength, ENCRYPTION) {}
};

class BlowfishDecryption : public Blowfish
{
public:
    BlowfishDecryption(byte *key_string, unsigned int keylength=KEYLENGTH)
        : Blowfish(key_string, keylength, DECRYPTION) {}
};

#endif

