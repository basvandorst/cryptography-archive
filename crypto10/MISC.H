#ifndef MISC_H
#define MISC_H

#include "config.h"
#include <assert.h>
#include <memory.h>

#ifdef SECALLOC_DEFAULT
#define SecAlloc(type, number) (new type[(number)])
#define SecFree(ptr, number) (memset((ptr), 0, (number)*sizeof(*(ptr))), delete [] (ptr))
#else
#define SecAlloc(type, number) (new type[(number)])
#define SecFree(ptr, number) (delete [] (ptr))
#endif

template <class T> struct SecBlock
{
public:
    SecBlock(unsigned int size)
        : size(size), ptr(SecAlloc(T, size)) {}
    ~SecBlock()
        {SecFree(ptr, size);}

    operator T *() const
        {return ptr;}
        
#ifndef _MSC_VER
    T *operator +(unsigned int offset)
        {return ptr+offset;}
    T& operator[](unsigned int index)
        {assert(index<size); return ptr[index];}
    const T& operator[](unsigned int index) const
        {assert(index<size); return ptr[index];}
#endif
                
    const unsigned int size;
    T *const ptr;
};

typedef SecBlock<byte> SecByteBlock;

void xorbuf(byte *buf, const byte *mask, unsigned int count);

void byteReverse(word16 *out, const word16 *in, unsigned int byteCount);
void byteReverse(word32 *out, const word32 *in, unsigned int byteCount);

inline word16 Invert(const word16 value)
{
    return (value << 8) | (value >> 8);
}

inline word32 Invert(const word32 value)
{
    word32 work = ((value & 0xFF00FF00L) >> 8) | ((value & 0x00FF00FFL) << 8);
    return (work << 16) | (work >> 16);
}

#ifndef __MINMAX_DEFINED
// skip these if the compiler provides them
template <class T> inline T min (const T t1, const T t2)
{
    return (t1 < t2 ? t1 : t2);
}

template <class T> inline T max (const T t1, const T t2)
{
    return (t1 > t2 ? t1 : t2);
}
#endif

template <class T> inline void swap (T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}

// these seem to be MUCH faster than the rotate functions
// provided by Borland C++ 4.5
template <class T> inline T rotl(T x, unsigned int y)
{
    return ((x<<y) | (x>>(sizeof(T)*8-y)));
}

template <class T> inline T rotr(T x, unsigned int y)
{
    return ((x>>y) | (x<<(sizeof(T)*8-y)));
}

int BytePrecision(unsigned long);
int BitPrecision(unsigned long);
unsigned long Crop(unsigned long, int size);

#endif

