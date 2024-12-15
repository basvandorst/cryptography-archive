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

    enum {KEYLENGTH=16, BLOCKSIZE=8, ROUNDS=8};
    unsigned int BlockSize() const {return BLOCKSIZE;};

private:
    void EnKey(const byte *);
    void DeKey();
    SecBlock<word> key;

#ifdef IDEA_LARGECACHE
	static inline void LookupMUL(unsigned int &a, word16 b);
	void LookupKeyLogs();
	static void BuildLogTables();
	static boolean tablesBuilt;
	static word16 log[0x10000], antilog[0x10000];
#endif
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
