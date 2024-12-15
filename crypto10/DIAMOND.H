#ifndef DIAMOND_H
#define DIAMOND_H

#include "cryptlib.h"
#include "misc.h"
#include "crc.h"

class DiamondBase : public BlockTransformation
{
public:
    DiamondBase(const byte *key, unsigned int key_size, unsigned int rounds,
                CipherDir direction);

    enum {KEYLENGTH=16, BLOCKSIZE=16, ROUNDS=10};    // default keylength
    unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
    enum {ROUNDSIZE=4096};
    inline void substitute(int round, byte *y);

    const int numrounds;
    SecByteBlock s;         // Substitution boxes

    static inline void permute(byte *);
    static inline void ipermute(byte *);
    static const word32 permtable[9][256];
    static const word32 ipermtable[9][256];
};

class DiamondEncryption : public DiamondBase
{
public:
    DiamondEncryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
        : DiamondBase(key, key_size, rounds, ENCRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);
};

class DiamondDecryption : public DiamondBase
{
public:
    DiamondDecryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
        : DiamondBase(key, key_size, rounds, DECRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);
};

class DiamondLiteBase : public BlockTransformation
{
public:
    DiamondLiteBase(const byte *key, unsigned int key_size, unsigned int rounds,
                CipherDir direction);

    enum {KEYLENGTH=16, BLOCKSIZE=8, ROUNDS=8};    // default keylength
    unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
    enum {ROUNDSIZE=2048};
    inline void substitute(int round, byte *y);
    const int numrounds;
    SecByteBlock s;         // Substitution boxes

    static inline void permute(byte *);
    static inline void ipermute(byte *);
    static const word32 permtable[8][256];
    static const word32 ipermtable[8][256];
};

class DiamondLiteEncryption : public DiamondLiteBase
{
public:
    DiamondLiteEncryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
        : DiamondLiteBase(key, key_size, rounds, ENCRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);
};

class DiamondLiteDecryption : public DiamondLiteBase
{
public:
    DiamondLiteDecryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
        : DiamondLiteBase(key, key_size, rounds, DECRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);
};

#endif

