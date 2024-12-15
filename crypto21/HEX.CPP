// hex.cpp - written and placed in the public domain by Wei Dai

#include "hex.h"

const byte HexEncoder::vec[17] = "0123456789ABCDEF";

void HexEncoder::Put(const byte *inString, unsigned int length)
{
    while (length--)
        HexEncoder::Put(*inString++);
}

void HexDecoder::Put(const byte *inString, unsigned int length)
{
    while (length--)
        HexDecoder::Put(*inString++);
}
