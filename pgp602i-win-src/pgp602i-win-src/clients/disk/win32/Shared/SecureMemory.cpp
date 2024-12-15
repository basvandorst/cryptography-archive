//////////////////////////////////////////////////////////////////////////////
// SecureMemory.cpp
//
// Implementation of class SecureMemory.
//////////////////////////////////////////////////////////////////////////////

// $Id: SecureMemory.cpp,v 1.1.2.16 1998/08/04 02:10:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "SecureMemory.h"

#if defined(PGPDISK_MFC)

#include "DriverComm.h"
#include "Globals.h"

#endif	// PGPDISK_MFC


/////////////////////////////////////////////
// Class SecureMemory public member functions
/////////////////////////////////////////////

// The SecureMemory default constructor allocates a block of locked memory of
// the specified size and wipes it.

SecureMemory::SecureMemory(PGPUInt32 size)
{
	mSize = size;
	mPMem = NULL;

#if defined(PGPDISK_MFC)

	mInitErr = AllocLockedMem(mSize, &mPMem);

#elif defined(PGPDISK_95DRIVER)

	mPMem = HeapAllocate(mSize, NULL);

	if (IsNull(mPMem))
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);

#elif defined(PGPDISK_NTDRIVER)

	mPMem = (void *) new PGPUInt8[mSize];

	if (IsNull(mPMem))
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);

#endif	// PGPDISK_MFC

	if (mInitErr.IsntError())
	{
		pgpClearMemory(mPMem, mSize);
	}
}

// The SecureMemory destructor wipes and deallocates the locked memory
// allocated in the constructor.

SecureMemory::~SecureMemory()
{
	if (IsntNull(mPMem))
	{
		pgpClearMemory(mPMem, mSize);
		pgpDebugWhackMemory(mPMem, mSize);

#if defined(PGPDISK_MFC)
		FreeLockedMem(mPMem, mSize);
#elif defined(PGPDISK_95DRIVER)
		HeapFree(mPMem, NULL);
#elif defined(PGPDISK_NTDRIVER)
		delete[] (PGPUInt8 *) mPMem;
#endif	// PGPDISK_MFC

		mPMem = NULL;
	}
}

// GetPtr returns a pointer to the currently allocated memory block.

void * 
SecureMemory::GetPtr()
{
	return mPMem;
}

// GetSize returns the size of the currently allocated memory block.

PGPUInt32 
SecureMemory::GetSize()
{
	return mSize;
}
