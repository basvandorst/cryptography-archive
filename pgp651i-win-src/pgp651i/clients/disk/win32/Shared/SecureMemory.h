//////////////////////////////////////////////////////////////////////////////
// SecureMemory.h
//
// Declaration of class SecureMemory.
//////////////////////////////////////////////////////////////////////////////

// $Id: SecureMemory.h,v 1.4 1998/12/14 19:01:22 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SecureMemory_h	// [
#define Included_SecureMemory_h

#include "DualErr.h"


/////////////////////
// Class SecureMemory
/////////////////////

// This class is thread-safe.

class SecureMemory
{
public:
	DualErr mInitErr;

	SecureMemory(PGPUInt32 size);
	~SecureMemory();

	PGPUInt32	GetSize();		// returns the size of the allocated memory
	void		*GetPtr();		// returns a pointer to the allocated memory

private:
	PGPUInt32	mSize;			// size of the memory
	void		*mPMem;			// pointer to the locked memory
};

#endif	// ] Included_SecureMemory_h
