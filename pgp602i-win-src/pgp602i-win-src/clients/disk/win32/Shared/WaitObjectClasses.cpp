//////////////////////////////////////////////////////////////////////////////
// WaitObjectClasses.cpp
//
// Synchronization object classes.
//////////////////////////////////////////////////////////////////////////////

// $Id: WaitObjectClasses.cpp,v 1.1.2.9 1998/08/04 02:10:36 nryan Exp $

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
#include "WaitObjectClasses.h"


//////////////////////////////////////////////////////////////////////////////
// NOTE:
//
// Unfortunately, synchronization primitive creation and use is so different
// across Win32, 95 driver mode and NT driver mode that it's impossible to
// provide a consistent calling interface to the following functions.
//////////////////////////////////////////////////////////////////////////////

#if defined(PGPDISK_MFC)

/////////////////////////////////////////////
// Class PGPdiskMutex public member functions
/////////////////////////////////////////////

// PGPdiskMutex constructs a PGPdiskMutex object.

PGPdiskMutex::PGPdiskMutex(PGPBoolean initOwnership, LPCSTR name)
{
	mMutexHandle = CreateMutex(NULL, initOwnership, name);

	if (IsNull(mMutexHandle))
	{
		mInitErr = DualErr(kPGDMinorError_MutexCreationFailed, 
			GetLastError());
	}
}

// ~PGPdiskMutex destroys a PGPdiskMutexObject.

PGPdiskMutex::~PGPdiskMutex()
{
	if (IsntNull(mMutexHandle))
	{
		CloseHandle(mMutexHandle);
		mMutexHandle = NULL;
	}
}

// Enter enters the mutex.

PGPUInt32 
PGPdiskMutex::Enter(PGPUInt32 timeout, PGPBoolean alertable)
{
	pgpAssert(IsntNull(mMutexHandle));

	return WaitForSingleObjectEx(mMutexHandle, timeout, alertable);
}

// Leave leaves the mutex.

void 
PGPdiskMutex::Leave()
{
	pgpAssert(IsntNull(mMutexHandle));

	ReleaseMutex(mMutexHandle);
}


/////////////////////////////////////////////////
// Class PGPdiskSemaphore public member functions
/////////////////////////////////////////////////

// PGPdiskSemaphore constructs a PGPdiskSemaphore object.
 
PGPdiskSemaphore::PGPdiskSemaphore(
	PGPUInt32	initCount, 
	PGPUInt32	maxCount, 
	LPCSTR		name)
{
	mSemHandle =  CreateSemaphore(NULL, initCount, maxCount, name);

	if (IsNull(mSemHandle))
	{
		mInitErr = DualErr(kPGDMinorError_SemaphoreCreationFailed, 
			GetLastError());
	}
}

// ~PGPdiskSemaphore destroys a PGPdiskSemaphore object.
 
PGPdiskSemaphore::~PGPdiskSemaphore()
{
	if (IsntNull(mSemHandle))
	{
		CloseHandle(mSemHandle);
		mSemHandle = NULL;
	}
}

// Waits waits on the semaphore.

PGPUInt32 
PGPdiskSemaphore::Wait(PGPUInt32 timeout, PGPBoolean alertable)
{
	pgpAssert(IsntNull(mSemHandle));

	return WaitForSingleObjectEx(mSemHandle, timeout, alertable);
}

// Signal signals the semaphore.

void 
PGPdiskSemaphore::Signal(PGPUInt32 count)
{
	pgpAssert(IsntNull(mSemHandle));

	ReleaseSemaphore(mSemHandle, count, NULL);
}


#elif defined(PGPDISK_95DRIVER)

/////////////////////////////////////////////
// Class PGPdiskMutex public member functions
/////////////////////////////////////////////

// PGPdiskMutex constructs a PGPdiskMutex object.

PGPdiskMutex::PGPdiskMutex(PGPUInt32 boost, PGPUInt32 flags)
{
	mMutexHandle = CreateMutex(boost, flags);

	if (IsNull(mMutexHandle))
		mInitErr = DualErr(kPGDMinorError_MutexCreationFailed);
}

// ~PGPdiskMutex destroys a PGPdiskMutexObject.

PGPdiskMutex::~PGPdiskMutex()
{
	if (IsntNull(mMutexHandle))
	{
		DestroyMutex(mMutexHandle);
		mMutexHandle = NULL;
	}
}

// Enter enters the mutex.

void 
PGPdiskMutex::Enter(PGPUInt32 flags)
{
	pgpAssert(IsntNull(mMutexHandle));

	EnterMutex(mMutexHandle, flags);
}

// Leave leaves the mutex.

void 
PGPdiskMutex::Leave()
{
	pgpAssert(IsntNull(mMutexHandle));

	LeaveMutex(mMutexHandle);
}


/////////////////////////////////////////////////
// Class PGPdiskSemaphore public member functions
/////////////////////////////////////////////////

// PGPdiskSemaphore constructs a PGPdiskSemaphore object.
 
PGPdiskSemaphore::PGPdiskSemaphore(PGPUInt32 tokenCount)
{
	mSemHandle =  Create_Semaphore(tokenCount);

	if (IsNull(mSemHandle))
		mInitErr = DualErr(kPGDMinorError_SemaphoreCreationFailed);
}

// ~PGPdiskSemaphore destroys a PGPdiskSemaphore object.
 
PGPdiskSemaphore::~PGPdiskSemaphore()
{
	if (IsntNull(mSemHandle))
	{
		Destroy_Semaphore(mSemHandle);
		mSemHandle = NULL;
	}
}

// Waits waits on the semaphore.

void 
PGPdiskSemaphore::Wait(PGPUInt32 flags)
{
	pgpAssert(IsntNull(mSemHandle));

	Wait_Semaphore(mSemHandle, flags);
}

// Signal signals the semaphore.

void 
PGPdiskSemaphore::Signal()
{
	pgpAssert(IsntNull(mSemHandle));

	Signal_Semaphore(mSemHandle);
}


#elif defined(PGPDISK_NTDRIVER)

/////////////////////////////////////////////
// Class PGPdiskMutex public member functions
/////////////////////////////////////////////

// PGPdiskMutex constructs a PGPdiskMutex object.

PGPdiskMutex::PGPdiskMutex(PGPUInt32 level)
{
	mMutexPointer = new KMUTEX;

	if (IsNull(mMutexPointer))
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);

	if (mInitErr.IsntError())
	{
		KeInitializeMutex(mMutexPointer, level);
	}
}

// ~PGPdiskMutex destroys a PGPdiskMutexObject.

PGPdiskMutex::~PGPdiskMutex()
{
	if (IsntNull(mMutexPointer))
	{
		delete mMutexPointer;
		mMutexPointer = NULL;
	}
}

// Enter enters the mutex.

PGPUInt32 
PGPdiskMutex::Enter(PGPUInt32 timeout, PGPBoolean alertable)
{
	pgpAssertAddrValid(mMutexPointer, KMUTEX);

	if (timeout == INFINITE)
	{
		return KeWaitForMutexObject(mMutexPointer, Executive, KernelMode, 
			alertable, NULL);
	}
	else
	{
		LARGE_INTEGER largeTimeout;

		largeTimeout.QuadPart = timeout;

		return KeWaitForMutexObject(mMutexPointer, Executive, KernelMode, 
			alertable, &largeTimeout);
	}
}

// Leave leaves the mutex.

void 
PGPdiskMutex::Leave(PGPBoolean wait)
{
	pgpAssertAddrValid(mMutexPointer, KMUTEX);

	KeReleaseMutex(mMutexPointer, wait);
}


/////////////////////////////////////////////////
// Class PGPdiskSemaphore public member functions
/////////////////////////////////////////////////

// PGPdiskSemaphore constructs a PGPdiskSemaphore object.
 
PGPdiskSemaphore::PGPdiskSemaphore(PGPUInt32 count, PGPUInt32 limit)
{
	mSemPointer = new KSEMAPHORE;

	if (IsNull(mSemPointer))
		mInitErr = DualErr(kPGDMinorError_OutOfMemory);

	if (mInitErr.IsntError())
	{
		KeInitializeSemaphore(mSemPointer, count, limit);
	}
}

// ~PGPdiskSemaphore destroys a PGPdiskSemaphore object.
 
PGPdiskSemaphore::~PGPdiskSemaphore()
{
	if (IsntNull(mSemPointer))
	{
		delete mSemPointer;
		mSemPointer = NULL;
	}
}

// Waits waits on the semaphore.

PGPUInt32 
PGPdiskSemaphore::Wait(PGPUInt32 timeout, PGPBoolean alertable)
{
	pgpAssertAddrValid(mSemPointer, KSEMAPHORE);

	if (timeout == INFINITE)
	{
		return KeWaitForSingleObject(mSemPointer, Executive, KernelMode, 
			alertable, NULL);
	}
	else
	{
		LARGE_INTEGER largeTimeout;

		largeTimeout.QuadPart = timeout;

		return KeWaitForSingleObject(mSemPointer, Executive, KernelMode, 
			alertable, &largeTimeout);
	}
}

// Signal signals the semaphore.

void 
PGPdiskSemaphore::Signal(PGPUInt32 count, BOOLEAN wait)
{
	pgpAssertAddrValid(mSemPointer, KSEMAPHORE);

	KeReleaseSemaphore(mSemPointer, 0, count, wait);
}

#endif // PGPDISK_MFC
