#ifndef MISC_H
#define MISC_H

#include "config.h"
#include <assert.h>
#include <string.h>

#ifdef AUTO_PTR_AVAILABLE
#include <utility>
#endif

#ifdef VECTOR_AVAILABLE
#include <vector>
#endif

inline unsigned int bitsToBytes(unsigned int bitCount)
{
	return ((bitCount+7)/(8));
}

inline unsigned int bytesToWords(unsigned int byteCount)
{
	return ((byteCount+WORD_SIZE-1)/WORD_SIZE);
}

inline unsigned int bitsToWords(unsigned int bitCount)
{
	return ((bitCount+WORD_BITS-1)/(WORD_BITS));
}

void xorbuf(byte *buf, const byte *mask, unsigned int count);

#ifndef __MINMAX_DEFINED
#define __MINMAX_DEFINED
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

template <class T> void swap (T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}

template <class T> inline T rotl(T x, unsigned int y)
{
    return ((x<<y) | (x>>(sizeof(T)*8-y)));
}

template <class T> inline T rotr(T x, unsigned int y)
{
    return ((x>>y) | (x<<(sizeof(T)*8-y)));
}

#if defined(_MSC_VER) || defined(__BCPLUSPLUS__)

#include <stdlib.h>
#define FAST_ROTATE

inline unsigned int rotl(unsigned short x, unsigned int y)
{
    return ((x<<y) | (x>>(16-y)));
}

inline unsigned int rotr(unsigned short x, unsigned int y)
{
    return ((x>>y) | (x<<(16-y)));
}

inline unsigned long rotl(unsigned long x, unsigned int y)
{
	return _lrotl(x, y);
}

inline unsigned long rotr(unsigned long x, unsigned int y)
{
	return _lrotr(x, y);
}
#endif // defined(_MSC_VER) || defined(__BCPLUSPLUS__)

inline word16 byteReverse(word16 value)
{
    return rotl(value, 8U);
}

inline word32 byteReverse(word32 value)
{
#ifdef FAST_ROTATE
	// 5 instructions with rotate instruction, 9 without
    return (rotr(value, 8) & 0xff00ff00) | (rotl(value, 8) & 0x00ff00ff);
#else
	// 6 instructions with rotate instruction, 8 without
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    return rotl(value, 16U);
#endif
}

#ifdef WORD64_AVAILABLE
inline word64 byteReverse(word64 value)
{
    value = ((value & 0xFF00FF00FF00FF00) >> 8) | ((value & 0x00FF00FF00FF00FF) << 8);
    value = ((value & 0xFFFF0000FFFF0000) >> 16) | ((value & 0x0000FFFF0000FFFF) << 16);
	return rotl(value, 32U);
}
#endif

template <class T> void byteReverse(T *out, const T *in, unsigned int byteCount)
{
	assert(byteCount%sizeof(T) == 0);
    byteCount /= sizeof(T);
	for (unsigned i=0; i<byteCount; i++)
		out[i] = byteReverse(in[i]);
}

#ifdef _MSC_VER
#define GETBYTE(x, y) (((byte *)&(x))[y])
#else
#define GETBYTE(x, y) (unsigned int)(((x)>>(8*y))&255)
#endif

unsigned int Parity(unsigned long);
unsigned int BytePrecision(unsigned long);
unsigned int BitPrecision(unsigned long);
unsigned long Crop(unsigned long, int size);

inline void SetWords(word *r, word a, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] = a;
}

inline void CopyWords(word *r, const word *a, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] = a[i];
}

inline void XorWords(word *r, const word *a, const word *b, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] = a[i] ^ b[i];
}

inline void XorWords(word *r, const word *a, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] ^= a[i];
}

inline void AndWords(word *r, const word *a, const word *b, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] = a[i] & b[i];
}

inline void AndWords(word *r, const word *a, unsigned int n)
{
	for (unsigned int i=0; i<n; i++)
		r[i] &= a[i];
}

inline word ShiftWordsLeftByBits(word *r, unsigned int n, unsigned int shiftBits)
{
	assert (shiftBits<WORD_BITS);
	word u, carry=0;
	if (shiftBits)
		for (unsigned int i=0; i<n; i++)
	    {
	        u = r[i];
	        r[i] = (u << shiftBits) | carry;
	        carry = u >> (WORD_BITS-shiftBits);
	    }
	return carry;
}

inline word ShiftWordsRightByBits(word *r, unsigned int n, unsigned int shiftBits)
{
	assert (shiftBits<WORD_BITS);
	word u, carry=0;
	if (shiftBits)
		for (int i=n-1; i>=0; i--)
	    {
	        u = r[i];
	        r[i] = (u >> shiftBits) | carry;
	        carry = u << (WORD_BITS-shiftBits);
	    }
	return carry;
}

inline void ShiftWordsLeftByWords(word *r, unsigned int n, unsigned int shiftWords)
{
	if (n && shiftWords)
	{
		for (unsigned i=n-1; i>=shiftWords; i--)
			r[i] = r[i-shiftWords];
		SetWords(r, 0, min(n, shiftWords));
	}
}

inline void ShiftWordsRightByWords(word *r, unsigned int n, unsigned int shiftWords)
{
	if (n && shiftWords)
	{
		for (unsigned i=0; i<n-shiftWords; i++)
			r[i] = r[i+shiftWords];
		SetWords(r+n-shiftWords, 0, min(n, shiftWords));
	}
}

// ********************************************************

#ifdef SECALLOC_DEFAULT
#define SecAlloc(type, number) (new type[(number)])
#define SecFree(ptr, number) (memset((ptr), 0, (number)*sizeof(*(ptr))), delete [] (ptr))
#else
#define SecAlloc(type, number) (new type[(number)])
#define SecFree(ptr, number) (delete [] (ptr))
#endif

template <class T> struct SecBlock
{
    SecBlock(unsigned int size=0)
        : size(size) {ptr = SecAlloc(T, size);}
	SecBlock(const SecBlock<T> &t)
		: size(t.size) {ptr = SecAlloc(T, size); CopyFrom(t);}
	SecBlock(const T *t, unsigned int size)
		: size(size) {ptr = SecAlloc(T, size); memcpy(ptr, t, size*sizeof(T));}
    ~SecBlock()
        {SecFree(ptr, size);}

#if defined(__GNUC__) || defined(__BCPLUSPLUS__)
    operator const void *() const
        {return ptr;}
	operator void *()
		{return ptr;}
#endif

    operator const T *() const
        {return ptr;}
	operator T *()
		{return ptr;}

#ifndef _MSC_VER
    T *operator +(unsigned int offset)
        {return ptr+offset;}
    const T *operator +(unsigned int offset) const
        {return ptr+offset;}
    T& operator[](unsigned int index)
		{assert(index<size); return ptr[index];}
    const T& operator[](unsigned int index) const
        {assert(index<size); return ptr[index];}
#endif

	void CopyFrom(const SecBlock<T> &t)
	{
		New(t.size);
		memcpy(ptr, t.ptr, size*sizeof(T));
	}

	SecBlock& operator=(const SecBlock<T> &t)
	{
		CopyFrom(t);
		return *this;
	}

	void New(unsigned int newSize)
	{
		if (newSize != size)
		{
			T *newPtr = SecAlloc(T, newSize);
			SecFree(ptr, size);
			ptr = newPtr;
			size = newSize;
		}
	}

	void CleanNew(unsigned int newSize)
	{
		if (newSize != size)
		{
			T *newPtr = SecAlloc(T, newSize);
			SecFree(ptr, size);
			ptr = newPtr;
			size = newSize;
		}
		memset(ptr, 0, size*sizeof(T));
	}

	void Grow(unsigned int newSize)
	{
		if (newSize > size)
		{
			T *newPtr = SecAlloc(T, newSize);
			memcpy(newPtr, ptr, size*sizeof(T));
			SecFree(ptr, size);
			ptr = newPtr;
			size = newSize;
		}
	}

	void CleanGrow(unsigned int newSize)
	{
		if (newSize > size)
		{
			T *newPtr = SecAlloc(T, newSize);
			memcpy(newPtr, ptr, size*sizeof(T));
			memset(newPtr+size, 0, (newSize-size)*sizeof(T));
			SecFree(ptr, size);
			ptr = newPtr;
			size = newSize;
		}
	}

    friend void swap(SecBlock<T> &a, SecBlock<T> &b)
        {swap(a.size, b.size); swap(a.ptr, b.ptr);}

    unsigned int size;
    T *ptr;
};

typedef SecBlock<byte> SecByteBlock;
typedef SecBlock<word> SecWordBlock;

// ********************************************************

#ifndef AUTO_PTR_AVAILABLE
template<class T> class auto_ptr 
{
public:
	auto_ptr(T *p = 0): pointee(p) {}

	auto_ptr(auto_ptr<T>& rhs): pointee(rhs.release()) {}

	~auto_ptr() { delete pointee; }

	auto_ptr<T>& operator=(auto_ptr<T>& rhs)
	{
		if (this != &rhs) reset(rhs.release());
		return *this;
	}

	T& operator*() const { return *pointee; }

	T* operator->() const { return pointee; }

	T* get() const { return pointee; }

	T* release()
	{
		T *oldPointee = pointee;
		pointee = 0;
		return oldPointee;
	} 

	void reset(T *p = 0) { delete pointee; pointee = p; }

private:
	T *pointee;
};
#endif // AUTO_PTR_AVAILABLE

// ********************************************************

#ifndef VECTOR_AVAILABLE
template <class T> class vector
{
public:
    vector(unsigned int size=0)
        : size(size) {ptr = new T[size];}
    vector(const vector<T>& x);
    ~vector()
        {delete [] ptr;}

    T& operator[](unsigned int index)
		{assert(index<size); return ptr[index];}
    const T& operator[](unsigned int index) const
        {assert(index<size); return ptr[index];}

	T* begin()
		{return ptr;}
	const T* begin() const
		{return ptr;}

	void reserve(unsigned int n);
	vector<T>& operator=(const vector<T>& x);

private:
    unsigned int size;
    T *ptr;
};

template <class T> vector<T>::vector(const vector<T>& x) 
	: size(x.size)
{
	ptr = new T[size];
#ifdef THROW_EXCEPTIONS
	try
	{
#endif
		for (unsigned i=0; i<size; i++) 
			ptr[i] = x[i];
#ifdef THROW_EXCEPTIONS
	}
	catch (...)
	{
		delete [] ptr;
		throw;
	}
#endif
}

template <class T> vector<T>& vector<T>::operator=(const vector<T>& x)
{
	if (size < x.size) 
	{
		delete [] ptr;
		size = x.size;
		ptr = new T[size];
	}

	for (unsigned int i=0; i<x.size; i++)
		ptr[i] = x[i];

	return *this;
}

template <class T> void vector<T>::reserve(unsigned int n)
{
	if (size < n) 
	{
		T *tmp = new T[n];
#ifdef THROW_EXCEPTIONS
		try
		{
#endif
			for (unsigned int i=0; i<size; i++)
				tmp[i] = ptr[i];
#ifdef THROW_EXCEPTIONS
		}
		catch (...)
		{
			delete [] tmp;
			throw;
		}
#endif
		delete [] ptr;
		size = n;
		ptr = tmp;
	}
}
#endif // VECTOR_AVAILABLE

template <class T> class vector_auto_ptrs
{
public:
    vector_auto_ptrs(unsigned int size=0)
        : size(size) {ptr = new auto_ptr<T>[size];}
    ~vector_auto_ptrs()
        {delete [] ptr;}

    auto_ptr<T>& operator[](unsigned int index)
		{assert(index<size); return ptr[index];}
    const auto_ptr<T>& operator[](unsigned int index) const
        {assert(index<size); return ptr[index];}

private:
    unsigned int size;
    auto_ptr<T> *ptr;
};

#endif // MISC_H
