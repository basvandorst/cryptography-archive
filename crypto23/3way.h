#ifndef THREEWAY_H
#define THREEWAY_H

#include "cryptlib.h"
#include "misc.h"

class ThreeWayEncryption : public BlockTransformation
{
public:
	enum {KEYLENGTH=12, BLOCKSIZE=12, ROUNDS=11};

	ThreeWayEncryption(const byte *userKey, unsigned rounds=ROUNDS);
	~ThreeWayEncryption();

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{ThreeWayEncryption::ProcessBlock(inoutBlock, inoutBlock);}

	unsigned int BlockSize() const {return BLOCKSIZE;}

private:
	word32 k[3];
	unsigned int rounds;
	SecBlock<word32> rc;
};

class ThreeWayDecryption : public BlockTransformation
{
public:
	enum {KEYLENGTH=12, BLOCKSIZE=12, ROUNDS=11};

	ThreeWayDecryption(const byte *userKey, unsigned rounds=ROUNDS);
	~ThreeWayDecryption();

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{ThreeWayDecryption::ProcessBlock(inoutBlock, inoutBlock);}

	unsigned int BlockSize() const {return BLOCKSIZE;}

private:
	word32 k[3];
	unsigned int rounds;
	SecBlock<word32> rc;
};

#endif
