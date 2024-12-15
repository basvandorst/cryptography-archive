// arc2.cpp - this code comes from an anonymous Usenet post
// any modifications are placed in the public domain by Wei Dai

#include "pch.h"
#include "arc4.h"

NAMESPACE_BEGIN(CryptoPP)

ARC4::ARC4(const byte *key_data_ptr, unsigned int key_data_len)
    : state(256)
{
    byte index1;
    byte index2;
    int counter;

    for (counter = 0; counter < 256; counter++)
        state[counter] = (byte) counter;
    x = 0;
    y = 0;
    index1 = 0;
    index2 = 0;
    for (counter = 0; counter < 256; counter++)
	{
        index2 = (key_data_ptr[index1++] + state[counter] + index2);
		std::swap(state[counter], state[index2]);
		if (index1 >= key_data_len) index1 = 0;
    }
}

ARC4::~ARC4()
{
    x=0;
    y=0;
}

byte ARC4::GetByte()
{
    x++;
    y += state[x];
	std::swap(state[x], state[y]);
    return (state[(state[x] + state[y]) & 255]);
}

byte ARC4::ProcessByte(byte input)
{
    x++;
    y += state[x];
	std::swap(state[x], state[y]);
    return input ^ (state[(state[x] + state[y]) & 255]);
}

void ARC4::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
    byte *const s=state;
    while(length--)
    {
        byte a = s[++x];
        y += a;
        byte b = s[y];
        s[y] = a;
        a += b;
        s[x] = b;
        *outString++ = *inString++ ^ s[a];
    }
}

void ARC4::ProcessString(byte *inoutString, unsigned int length)
{
    byte *const s=state;
    while(length--)
    {
        byte a = s[++x];
        y += a;
        byte b = s[y];
        s[y] = a;
        a += b;
        s[x] = b;
        *inoutString++ ^= s[a];
    }
}

NAMESPACE_END
