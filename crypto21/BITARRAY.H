#ifndef BITARRAY_H
#define BITARRAY_H

#include "cryptlib.h"
#include "misc.h"

class ostream;

class BitArray
{
public:
    BitArray(unsigned int size=0)
        : size(size), array(new byte[(size+7)/8])
    {
        ClearAll();
    }

    BitArray(const BitArray &original)
        : size(original.size), array(new byte[(size+7)/8])
    {
        memcpy(array, original.array, (size+7)/8);
    }

    BitArray(BufferedTransformation &BEREncoded);

    ~BitArray()
    {
        delete [] array;
    }

    void BERDecode(BufferedTransformation &in);
    void DEREncode(BufferedTransformation &out);

    BitArray & operator=(const BitArray &original)
    {
        Resize(original.size);
        memcpy(array, original.array, (size+7)/8);
        return *this;
    }

    boolean operator==(const BitArray &target)
    {
        return (size==target.size && !memcmp(array, target.array, (size+7)/8));
    }

    boolean operator!=(const BitArray &target)
    {
        return !(operator==(target));
    }

    class Element;

    Element operator[](unsigned int index)
    {
        return Element(*this, index);
    }

    unsigned int Size()
    {
        return size;
    }

    void Resize(unsigned int newsize)
    {
        if (size!=newsize)
        {
            size=newsize;
            delete [] array;
            array = new byte[(size+7)/8];
        }
        ClearAll();
    }

    int Get(unsigned int index) const
    {
        assert (index<size);
        return (array[index/8] & (1 << (7-(index % 8)))) != 0;
    }

    void Clear(unsigned int index)
    {
        assert (index<size);
        array[index/8] &= ~(1 << (7-(index % 8)));
    }

    void Set(unsigned int index, int value=1)
    {
        assert (index<size);
        if (value)  // set bit
            array[index/8] |= 1 << (7-(index % 8));
        else        // clear bit
            array[index/8] &= ~(1 << (7-(index % 8)));
    }

    void ClearAll()
    {
        memset(array, 0, (size+7)/8);
    }

    void SetAll(int value=1)
    {
        memset(array, value ? 0xff : 0, (size+7)/8);
        if (size%8)
            array[(size+7)/8 - 1] &= ~((1 << (8-(size%8)))-1);
    }

    unsigned int Count();

    friend void swapBitArray (BitArray &a, BitArray &b)
    {
        swap(a.array, b.array);
        swap(a.size, b.size);
    }

    void swapElement(unsigned int a, unsigned int b)
    {
        int tmp=Get(a);
        Set(a, Get(b));
        Set(b, tmp);
    }

    friend ostream& operator<<(ostream& out, const BitArray &a);

private:
    unsigned int size;
    byte * array;

    class Element
    {
    public:
        Element(BitArray &parent, unsigned int index)
            : parent(parent), index(index) {}
        operator int()
            {return parent.Get(index);}
        operator =(int value)
            {parent.Set(index, value); return *this;}
    private:
        BitArray &parent;
        const unsigned int index;
    };
};

#endif

