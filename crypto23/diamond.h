#ifndef DIAMOND_H
#define DIAMOND_H

#include "cryptlib.h"
#include "misc.h"
#include "crc.h"

class Diamond2Base : public BlockTransformation
{
public:
	Diamond2Base(const byte *key, unsigned int key_size, unsigned int rounds,
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
#ifdef DIAMOND_USE_PERMTABLE
	static const word32 permtable[9][256];
	static const word32 ipermtable[9][256];
#endif
};

class Diamond2Encryption : public Diamond2Base
{
public:
	Diamond2Encryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
		: Diamond2Base(key, key_size, rounds, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock);
};

class Diamond2Decryption : public Diamond2Base
{
public:
	Diamond2Decryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
		: Diamond2Base(key, key_size, rounds, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock);
};

class Diamond2LiteBase : public BlockTransformation
{
public:
	Diamond2LiteBase(const byte *key, unsigned int key_size, unsigned int rounds,
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
#ifdef DIAMOND_USE_PERMTABLE
	static const word32 permtable[8][256];
	static const word32 ipermtable[8][256];
#endif
};

class Diamond2LiteEncryption : public Diamond2LiteBase
{
public:
	Diamond2LiteEncryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
		: Diamond2LiteBase(key, key_size, rounds, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock);
};

class Diamond2LiteDecryption : public Diamond2LiteBase
{
public:
	Diamond2LiteDecryption(const byte *key, unsigned int key_size=KEYLENGTH, unsigned int rounds=ROUNDS)
		: Diamond2LiteBase(key, key_size, rounds, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock);
	void ProcessBlock(byte * inoutBlock);
};

#endif
