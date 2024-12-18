// blowfish.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "blowfish.h"

Blowfish::Blowfish(const byte *key_string, unsigned int keylength, CipherDir dir)
	: pbox(ROUNDS+2), sbox(4*256)
{
	unsigned i, j=0, k;
	word32 data, dspace[2] = {0, 0};

	memcpy(pbox, p_init, sizeof(p_init));
	memcpy(sbox, s_init, sizeof(s_init));

	// Xor key string into encryption key vector
	for (i=0 ; i<ROUNDS+2 ; ++i)
	{
		data = 0 ;
		for (k=0 ; k<4 ; ++k )
			data = (data << 8) | key_string[j++ % keylength];
		pbox[i] ^= data;
	}

	crypt_block(dspace, pbox);

	for (i=0; i<ROUNDS; i+=2)
		crypt_block(pbox+i, pbox+i+2);

	crypt_block(pbox+ROUNDS, sbox);

	for (i=0; i<4*256-2; i+=2)
		crypt_block(sbox+i, sbox+i+2);

	if (dir==DECRYPTION)
		for (i=0; i<(ROUNDS+2)/2; i++)
			std::swap(pbox[i], pbox[ROUNDS+1-i]);
}

// this version is only used to make pbox and sbox
void Blowfish::crypt_block(const word32 in[2], word32 out[2]) const
{
	word32 left = in[0];
	word32 right = in[1];

	const word32 *const s=sbox;
	const word32 *p=pbox;

	left ^= p[0];

	for (unsigned i=0; i<ROUNDS/2; i++)
	{
		right ^= (((s[GETBYTE(left,3)] + s[256+GETBYTE(left,2)])
			  ^ s[2*256+GETBYTE(left,1)]) + s[3*256+GETBYTE(left,0)])
			  ^ p[2*i+1];

		left ^= (((s[GETBYTE(right,3)] + s[256+GETBYTE(right,2)])
			 ^ s[2*256+GETBYTE(right,1)]) + s[3*256+GETBYTE(right,0)])
			 ^ p[2*i+2];
	}

	right ^= p[ROUNDS+1];

	out[0] = right;
	out[1] = left;
}

void Blowfish::ProcessBlock(const byte *in, byte *out)
{
#ifdef LITTLE_ENDIAN
	word32 left = byteReverse(*(word32 *)in);
	word32 right = byteReverse(*(word32 *)(in+4));
#else
	word32 left = *(word32 *)in;
	word32 right = *(word32 *)(in+4);
#endif

	const word32 *const s=sbox;
	const word32 *p=pbox;

	left ^= p[0];

	for (unsigned i=0; i<ROUNDS/2; i++)
	{
		right ^= (((s[GETBYTE(left,3)] + s[256+GETBYTE(left,2)])
			  ^ s[2*256+GETBYTE(left,1)]) + s[3*256+GETBYTE(left,0)])
			  ^ p[2*i+1];

		left ^= (((s[GETBYTE(right,3)] + s[256+GETBYTE(right,2)])
			 ^ s[2*256+GETBYTE(right,1)]) + s[3*256+GETBYTE(right,0)])
			 ^ p[2*i+2];
	}

	right ^= p[ROUNDS+1];

#ifdef LITTLE_ENDIAN
	*(word32 *)out = byteReverse(right);
	*(word32 *)(out+4) = byteReverse(left);
#else
	*(word32 *)out = right;
	*(word32 *)(out+4) = left;
#endif
}
