// bitarray.cpp - written and placed in the public domain by Wei Dai

#include "bitarray.h"
#include "asn.h"

#include <iostream.h>

BitArray::BitArray(BufferedTransformation &bt)
{
    size = 0;
    array = new byte[size];
    BERDecode(bt);
}

void BitArray::BERDecode(BufferedTransformation &bt)
{
    // TODO: decode constructed encoding as well
    byte b;
    if (!bt.Get(b) || b != BIT_STRING)
        BERDecodeError();

    unsigned int bc;
    BERLengthDecode(bt, bc);

    byte unusedBits;
    if (!bt.Get(unusedBits) || unusedBits >= 8)
        BERDecodeError();
    --bc;

    Resize(bc*8 - unusedBits);

    if (bt.Get(array, bc) < bc)
        BERDecodeError();

    if (unusedBits) // remove padding
        array[(size+7)/8 - 1] &= ~((1 << (8-(size%8)))-1);
}

void BitArray::DEREncode(BufferedTransformation &bt)
{
    bt.Put(BIT_STRING);
    DERLengthEncode((size+7)/8 + 1, bt);
    bt.Put(((size+7)/8)*8 - size);  // number of unused bytes
    bt.Put(array, (size+7)/8);
}

unsigned int BitArray::Count()
{
    unsigned int count=0;
    for (unsigned int i=0; i<(size+7)/8; i++)
    {
        byte x=array[i];
        if (x)
            do
                count++;
            while (0 != (x = x&(x-1)));
    }
    return count;
}

ostream& operator<<(ostream& out, const BitArray &a)
{
    for (unsigned int i=0; i<a.size; i++)
        out << a.Get(i);

    return out;
}

