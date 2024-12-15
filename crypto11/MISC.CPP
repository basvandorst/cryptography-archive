#include <memory.h>
#include "misc.h"

void xorbuf(byte *buf, const byte *mask, unsigned int count)
{
    while (count--)
        *buf++ ^= *mask++;
}

void byteReverse(word16 *out, const word16 *in, unsigned int i)
{
    i >>= 1;
    while (i--)
        out[i] = (in[i] << 8) | (in[i] >> 8);
}

void byteReverse(word32 *out, const word32 *in, unsigned int byteCount)
{
    word32 value;
    byteCount >>= 2;
    while( byteCount-- )
    {
        value = *in++;
        value = ( ( value & 0xFF00FF00L ) >> 8 ) |
                ( ( value & 0x00FF00FFL ) << 8 );
        *out++ = ( value << 16 ) | ( value >> 16 );
    }
}

int BytePrecision(unsigned long value)
{
    for (unsigned int i=sizeof(value); i; --i)
        if (value >> (i-1)*8)
            break;

    return i;
}

int BitPrecision(unsigned long value)
{
    for (unsigned int i=8*sizeof(value); i; i--)
        if (value >> (i-1))
            break;

    return i;
}

unsigned long Crop(unsigned long value, int size)
{
    return (value & ((1L << size) - 1));
}

