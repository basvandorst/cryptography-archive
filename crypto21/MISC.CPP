// misc.cpp - written and placed in the public domain by Wei Dai

#include "misc.h"

void xorbuf(byte *buf, const byte *mask, unsigned int count)
{
	if (((unsigned int)buf | (unsigned int)mask | count) % WORD_SIZE == 0)
		XorWords((word *)buf, (const word *)mask, count/WORD_SIZE);
	else
	{
		while (count--)
			*buf++ ^= *mask++;
	}
}

unsigned int Parity(unsigned long value)
{
	for (unsigned int i=8*sizeof(value)/2; i>0; i/=2)
		value ^= value >> i;
	return (unsigned int)value&1;
}

unsigned int BytePrecision(unsigned long value)
{
	unsigned int i;
    for (i=sizeof(value); i; --i)
        if (value >> (i-1)*8)
            break;

    return i;
}

unsigned int BitPrecision(unsigned long value)
{
	unsigned int i;
    for (i=8*sizeof(value); i; i--)
        if (value >> (i-1))
            break;

    return i;
}

unsigned long Crop(unsigned long value, int size)
{
	if (size < 8*sizeof(value))
    	return (value & ((1L << size) - 1));
	else
		return value;
}

