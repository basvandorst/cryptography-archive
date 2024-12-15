//////////////////////////////////////////////////////////////////////////////
// WaitObjectClasses.h
//
// Declarations for WaitObjectClasses.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: WaitObjectClasses.h,v 1.1.2.8 1998/08/04 02:10:37 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_WaitObjectClasses_h	// [
#define Included_WaitObjectClasses_h

#include "DualErr.h"


//////////////////////////////////////////////////////////////////////////////
// NOTE:
//
// Unfortunately, synchronization primitive creation and use is so different
// across Win32, 95 driver mode and NT driver mode that it's impossible to
// provide a consistent calling interface to the following functions.
//////////////////////////////////////////////////////////////////////////////

// The following classes are thread-safe.

////////////
// Constants
////////////

const PGPUInt32 kInfiniteKernelSemLimit = 0x7FFFFFFF;


#if defined(PGPDISK_MFC)

/////////////////////
// Class PGPdiskMutex
/////////////////////

class PGPdiskMutex
{
public:
	DualErr mInitErr;

				PGPdiskMutex(PGPBoolean initOwnership = FALSE, 
					LPCSTR name = NULL);
				~PGPdiskMutex();

	PGPUInt32	Enter(PGPUInt32 timeout = INFINITE, 
					PGPBoolean alertable = FALSE);
	void		Leave();

private:
	HANDLE mMutexHandle;
};


/////////////////////////
// Class PGPdiskSemaphore
/////////////////////////

class PGPdiskSemaphore
{
public:
	DualErr mInitErr;

				PGPdiskSemaphore(PGPUInt32 initCount = 0, PGPUInt32 maxCount = 1, 
					LPCSTR name = NULL);
				~PGPdiskSemaphore();

	PGPUInt32	Wait(PGPUInt32 timeout = INFINITE, 
					PGPBoolean alertable = FALSE);
	void		Signal(PGPUInt32 count = 1);

private:
	HANDLE mSemHandle;
};


#elif defined(PGPDISK_95DRIVER)

/////////////////////
// Class PGPdiskMutex
/////////////////////

class PGPdiskMutex
{
public:
	DualErr mInitErr;

			PGPdiskMutex(PGPUInt32 boost = 0, PGPUInt32 flags = NULL);
			~PGPdiskMutex();

	void	Enter(PGPUInt32 flags = BLOCK_ENABLE_INTS | BLOCK_SVC_INTS);
	void	Leave();

private:
	MUTEXHANDLE mMutexHandle;
};


/////////////////////////
// Class PGPdiskSemaphore
/////////////////////////

class PGPdiskSemaphore
{
public:
	DualErr mInitErr;

			PGPdiskSemaphore(PGPUInt32 tokenCount = 1);
			~PGPdiskSemaphore();

	void	Wait(PGPUInt32 flags = BLOCK_ENABLE_INTS | BLOCK_SVC_INTS);
	void	Signal();

private:
	SEMHANDLE mSemHandle;
};


#elif defined(PGPDISK_NTDRIVER)

#define INFINITE 0xFFFFFFFF

/////////////////////
// Class PGPdiskMutex
/////////////////////

class PGPdiskMutex
{
public:
	DualErr mInitErr;

				PGPdiskMutex(PGPUInt32 level);
				~PGPdiskMutex();

	PGPUInt32	Enter(PGPUInt32 timeout = INFINITE, 
					PGPBoolean alertable = FALSE);
	void		Leave(PGPBoolean wait = FALSE);

private:
	PKMUTEX mMutexPointer;
};


/////////////////////////
// Class PGPdiskSemaphore
/////////////////////////

class PGPdiskSemaphore
{
public:
	DualErr mInitErr;

				PGPdiskSemaphore(PGPUInt32 count = 0, PGPUInt32 limit = 1);
				~PGPdiskSemaphore();

	PGPUInt32	Wait(PGPUInt32 timeout = INFINITE, 
					PGPBoolean alertable = FALSE);
	void		Signal(PGPUInt32 count = 1, PGPBoolean wait = FALSE);

private:
	PKSEMAPHORE mSemPointer;
};

#endif // PGPDISK_MFC

#endif // ] Included_WaitObjectClasses_h
