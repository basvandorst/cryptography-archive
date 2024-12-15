#ifndef RC5_H
#define RC5_H

#include "cryptlib.h"
#include "misc.h"

typedef word32 RC5_WORD;

class RC5Base : public BlockTransformation
{
public:
    // values of KEYLENGTH and ROUNDS are defaults only
    enum {KEYLENGTH=16, BLOCKSIZE = sizeof(RC5_WORD)*2, ROUNDS=12};  // == 8
    unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
    RC5Base(const byte *key, unsigned int keyLen, unsigned int rounds);

    static inline void PUTBLOCK(byte *block, word32 left, word32 right);
    static inline void GETBLOCK(const byte *block, word32 &left, word32 &right);

    const int r;                // number of rounds
    SecBlock<RC5_WORD> sTable;  // expanded key table
};

class RC5Encryption : public RC5Base
{
public:
    RC5Encryption(const byte *key, unsigned int keyLen=KEYLENGTH, unsigned int rounds=ROUNDS)
        : RC5Base(key, keyLen, rounds) {}

    void ProcessBlock(byte * inoutBlock)
        {RC5Encryption::ProcessBlock(inoutBlock, inoutBlock);}
    void ProcessBlock(const byte *inBlock, byte * outBlock);
};

class RC5Decryption : public RC5Base
{
public:
    RC5Decryption(const byte *key, unsigned int keyLen=KEYLENGTH, unsigned int rounds=ROUNDS)
        : RC5Base(key, keyLen, rounds) {}

    void ProcessBlock(byte * inoutBlock)
        {RC5Decryption::ProcessBlock(inoutBlock, inoutBlock);}
    void ProcessBlock(const byte *inBlock, byte * outBlock);
};

#endif

