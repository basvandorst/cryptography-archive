#ifndef IDEA_H
#define IDEA_H

#include "cryptlib.h"
#include "misc.h"

class IDEA : public BlockTransformation
{
public:
    IDEA(const byte *userKey, CipherDir dir);

    void ProcessBlock(byte * inoutBlock)
        {IDEA::ProcessBlock(inoutBlock, inoutBlock);}
    void ProcessBlock(const byte *inBlock, byte * outBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;};

private:
    void EnKey(const byte *);
    void DeKey();
    SecBlock<word16> key;
};

class IDEAEncryption : public IDEA
{
public:
    IDEAEncryption(const byte * userKey)
        : IDEA (userKey, ENCRYPTION) {}
};

class IDEADecryption : public IDEA
{
public:
    IDEADecryption(const byte * userKey)
        : IDEA (userKey, DECRYPTION) {}
};

#endif

