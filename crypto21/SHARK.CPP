// shark.cpp - written and placed in the public domain by Wei Dai

#include "shark.h"

#ifdef WORD64_AVAILABLE

#include "modes.h"

// multiply two elements of GF(2^8)
word SHARKBase::GF256Mul(word a, word b)
{
	word result = 0;

	for (unsigned int i=0; i<8; i++)
	{
		result <<= 1;
		if (result & 0x100)
			result ^= 0x1f5;

		b <<= 1;
		if (b & 0x100)
			result ^= a;
	}
	return result & 0xff;
}

word64 SHARKBase::Transform(word64 a)
{
	word64 result=0;
	for (unsigned int i=0; i<8; i++)
		for(unsigned int j=0; j<8; j++) 
			result ^= word64(GF256Mul(iG[i][j], word(a>>(56-8*j)))) << (56-8*i);
	return result;
}

void SHARKBase::InitEncryptionRoundKeys(const byte *key, unsigned int keyLen, unsigned int rounds, word64 *roundkeys)
{
	// concatenate key enought times to fill a
	for (unsigned int i=0; i<(rounds+1)*8; i++)
		((byte *)roundkeys)[i] = key[i%keyLen];

	SHARKEncryption e;
	byte IV[8] = {0,0,0,0,0,0,0,0};
	CFBEncryption cfb(e, IV);

	cfb.ProcessString((byte *)roundkeys, (rounds+1)*8);

#ifdef LITTLE_ENDIAN
	byteReverse(roundkeys, roundkeys, (rounds+1)*8);
#endif

	roundkeys[rounds] = Transform(roundkeys[rounds]);
}

// construct an SHARKEncryption object with fixed round keys, to be used to initialize actual round keys
SHARKEncryption::SHARKEncryption()
	: SHARKBase(ROUNDS)
{
	for (unsigned int i=0; i<ROUNDS; i++)
		roundkeys[i] = cbox[0][i];

	roundkeys[ROUNDS] = Transform(cbox[0][ROUNDS]);

#ifdef LITTLE_ENDIAN
	roundkeys[0] = byteReverse(roundkeys[0]);
	roundkeys[ROUNDS] = byteReverse(roundkeys[ROUNDS]);
#endif
}

SHARKEncryption::SHARKEncryption(const byte *key, unsigned int keyLen, unsigned int rounds)
	: SHARKBase(rounds)
{
	InitEncryptionRoundKeys(key, keyLen, rounds, roundkeys);

#ifdef LITTLE_ENDIAN
	roundkeys[0] = byteReverse(roundkeys[0]);
	roundkeys[rounds] = byteReverse(roundkeys[rounds]);
#endif
}

void SHARKEncryption::ProcessBlock(const byte *in, byte *out)
{
	word64 tmp = *(word64 *)in ^ roundkeys[0];

#ifdef LITTLE_ENDIAN
	tmp = cbox[0][GETBYTE(tmp, 0)] ^ cbox[1][GETBYTE(tmp, 1)] 
		^ cbox[2][GETBYTE(tmp, 2)] ^ cbox[3][GETBYTE(tmp, 3)] 
		^ cbox[4][GETBYTE(tmp, 4)] ^ cbox[5][GETBYTE(tmp, 5)] 
		^ cbox[6][GETBYTE(tmp, 6)] ^ cbox[7][GETBYTE(tmp, 7)]
		^ roundkeys[1];
#else
	tmp = cbox[0][GETBYTE(tmp, 7)] ^ cbox[1][GETBYTE(tmp, 6)] 
		^ cbox[2][GETBYTE(tmp, 5)] ^ cbox[3][GETBYTE(tmp, 4)] 
		^ cbox[4][GETBYTE(tmp, 3)] ^ cbox[5][GETBYTE(tmp, 2)] 
		^ cbox[6][GETBYTE(tmp, 1)] ^ cbox[7][GETBYTE(tmp, 0)]
		^ roundkeys[1];
#endif

	for(unsigned int i=2; i<rounds; i++) 
	{
		tmp = cbox[0][GETBYTE(tmp, 7)] ^ cbox[1][GETBYTE(tmp, 6)] 
			^ cbox[2][GETBYTE(tmp, 5)] ^ cbox[3][GETBYTE(tmp, 4)] 
			^ cbox[4][GETBYTE(tmp, 3)] ^ cbox[5][GETBYTE(tmp, 2)] 
			^ cbox[6][GETBYTE(tmp, 1)] ^ cbox[7][GETBYTE(tmp, 0)]
			^ roundkeys[i];
	}

	out[0] = sbox[GETBYTE(tmp, 7)];
	out[1] = sbox[GETBYTE(tmp, 6)];
	out[2] = sbox[GETBYTE(tmp, 5)];
	out[3] = sbox[GETBYTE(tmp, 4)];
	out[4] = sbox[GETBYTE(tmp, 3)];
	out[5] = sbox[GETBYTE(tmp, 2)];
	out[6] = sbox[GETBYTE(tmp, 1)];
	out[7] = sbox[GETBYTE(tmp, 0)];

    *(word64 *)out ^= roundkeys[rounds];
}

SHARKDecryption::SHARKDecryption(const byte *key, unsigned int keyLen, unsigned int rounds)
	: SHARKBase(rounds)
{
	InitEncryptionRoundKeys(key, keyLen, rounds, roundkeys);

	unsigned int i;

	// transform encryption round keys into decryption round keys
	for (i=0; i<rounds/2; i++)
		swap(roundkeys[i], roundkeys[rounds-i]);

	for (i=1; i<rounds; i++)
		roundkeys[i] = Transform(roundkeys[i]);

#ifdef LITTLE_ENDIAN
	roundkeys[0] = byteReverse(roundkeys[0]);
	roundkeys[rounds] = byteReverse(roundkeys[rounds]);
#endif
}

void SHARKDecryption::ProcessBlock(const byte *in, byte *out)
{
	word64 tmp = *(word64 *)in ^ roundkeys[0];

#ifdef LITTLE_ENDIAN
	tmp = cbox[0][GETBYTE(tmp, 0)] ^ cbox[1][GETBYTE(tmp, 1)] 
		^ cbox[2][GETBYTE(tmp, 2)] ^ cbox[3][GETBYTE(tmp, 3)] 
		^ cbox[4][GETBYTE(tmp, 4)] ^ cbox[5][GETBYTE(tmp, 5)] 
		^ cbox[6][GETBYTE(tmp, 6)] ^ cbox[7][GETBYTE(tmp, 7)]
		^ roundkeys[1];
#else
	tmp = cbox[0][GETBYTE(tmp, 7)] ^ cbox[1][GETBYTE(tmp, 6)] 
		^ cbox[2][GETBYTE(tmp, 5)] ^ cbox[3][GETBYTE(tmp, 4)] 
		^ cbox[4][GETBYTE(tmp, 3)] ^ cbox[5][GETBYTE(tmp, 2)] 
		^ cbox[6][GETBYTE(tmp, 1)] ^ cbox[7][GETBYTE(tmp, 0)]
		^ roundkeys[1];
#endif

	for(unsigned int i=2; i<rounds; i++) 
	{
		tmp = cbox[0][GETBYTE(tmp, 7)] ^ cbox[1][GETBYTE(tmp, 6)] 
			^ cbox[2][GETBYTE(tmp, 5)] ^ cbox[3][GETBYTE(tmp, 4)] 
			^ cbox[4][GETBYTE(tmp, 3)] ^ cbox[5][GETBYTE(tmp, 2)] 
			^ cbox[6][GETBYTE(tmp, 1)] ^ cbox[7][GETBYTE(tmp, 0)]
			^ roundkeys[i];
	}

	out[0] = sbox[GETBYTE(tmp, 7)];
	out[1] = sbox[GETBYTE(tmp, 6)];
	out[2] = sbox[GETBYTE(tmp, 5)];
	out[3] = sbox[GETBYTE(tmp, 4)];
	out[4] = sbox[GETBYTE(tmp, 3)];
	out[5] = sbox[GETBYTE(tmp, 2)];
	out[6] = sbox[GETBYTE(tmp, 1)];
	out[7] = sbox[GETBYTE(tmp, 0)];

    *(word64 *)out ^= roundkeys[rounds];
}

#endif // WORD64_AVAILABLE
