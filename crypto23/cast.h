#ifndef CAST_H
#define CAST_H

#include "cryptlib.h"
#include "misc.h"

class CAST128 : public BlockTransformation
{
public:
	enum {KEYLENGTH=16, BLOCKSIZE=8};
	unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
	// keylength should be between 5 and 16
	CAST128(const byte *userKey, unsigned int keylength);

	static const word32 S[8][256];

	bool reduced;
	SecBlock<word32> K;
};

class CAST128Encryption : public CAST128
{
public:
	CAST128Encryption(const byte *userKey, unsigned int keylength=KEYLENGTH)
		: CAST128(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{CAST128Encryption::ProcessBlock(inoutBlock, inoutBlock);}
};

class CAST128Decryption : public CAST128
{
public:
	CAST128Decryption(const byte *userKey, unsigned int keylength=KEYLENGTH)
		: CAST128(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock)
		{CAST128Decryption::ProcessBlock(inoutBlock, inoutBlock);}
};

#endif
