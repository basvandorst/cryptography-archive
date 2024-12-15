#ifndef VECTOR_H
#define VECTOR_H

#include <assert.h>

template <class T> struct vector
{
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

#endif
