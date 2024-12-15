#ifndef GOST_H
#define GOST_H

#include "cryptlib.h"
#include "misc.h"

class GOST : public BlockTransformation
{
public:
	GOST(const byte *userKey, CipherDir);

	enum {KEYLENGTH=32, BLOCKSIZE=8};
	unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
	static void PrecalculateSTable();

	static const byte sBox[8][16];
	static bool sTableCalculated;
	static word32 sTable[4][256];

	SecBlock<word32> key;
};

class GOSTEncryption : public GOST
{
public:
	GOSTEncryption(const byte * userKey)
		: GOST (userKey, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{GOSTEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

class GOSTDecryption : public GOST
{
public:
	GOSTDecryption(const byte * userKey)
		: GOST (userKey, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{GOSTDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

#endif
