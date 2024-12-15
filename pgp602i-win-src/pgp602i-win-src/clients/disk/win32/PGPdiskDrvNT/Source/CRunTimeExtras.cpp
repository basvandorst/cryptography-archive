//////////////////////////////////////////////////////////////////////////////
// CRunTimeExtras.cpp
//
// Definitions of extra C run-time functions we define.
//////////////////////////////////////////////////////////////////////////////

// $Id: CRunTimeExtras.cpp,v 1.1.2.6 1998/06/11 08:53:57 nryan Exp $

// Portions Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>
#include "CRunTimeExtras.h"


///////////////////////////////
// C run time library functions
///////////////////////////////

extern "C" 
void
__cdecl 
free(void *memBlock)
{
	delete[] memBlock;
}

extern "C" 
void * 
__cdecl 
malloc(size_t size)
{
	unsigned char *newBlock;

	newBlock = new unsigned char[size + sizeof(size_t)];

	if (newBlock == NULL)
		return NULL;

	((size_t *) newBlock)[0] = size;

	return (newBlock + sizeof(size_t));
}

extern "C" 
void * 
__cdecl 
realloc(void *oldBlock, size_t size)
{
	size_t			oldSize;
	unsigned char	*actualOldBlock;
	void			*newBlock;
	
	if (oldBlock == NULL)
		return malloc(size);

	actualOldBlock = ((unsigned char *) oldBlock) - sizeof(size_t);
	oldSize = ((size_t *) actualOldBlock)[0];

	if (size == 0)
	{
		free(actualOldBlock);
		return NULL;
	}

	newBlock = malloc(size);

	if (newBlock == NULL)
		return NULL;

	memmove(newBlock, oldBlock, (size < oldSize ? size : oldSize));

	return newBlock;
}
