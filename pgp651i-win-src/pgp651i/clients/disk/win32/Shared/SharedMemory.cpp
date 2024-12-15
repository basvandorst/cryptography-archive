//////////////////////////////////////////////////////////////////////////////
// SharedMemory.cpp
//
// Implementation of class SharedMemory.
//////////////////////////////////////////////////////////////////////////////

// $Id: SharedMemory.cpp,v 1.3 1998/12/14 19:01:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "SharedMemory.h"


/////////////////////////////////////////////
// Class SharedMemory public member functions
/////////////////////////////////////////////

// The SharedMemory constructor.

SharedMemory::SharedMemory()
{
	mSharedMemHandle = NULL;
	mPMem = NULL;
}

// The SharedMemory destructor.

SharedMemory::~SharedMemory()
{
	DualErr derr;

	if (IsMemAttached())
	{
		derr = Detach();
		pgpAssert(derr.IsntError());
	}
}

// IsMemAttached returns TRUE if shared memory is attached, FALSE otherwise.

PGPBoolean 
SharedMemory::IsMemAttached()
{
	return IsntNull(mSharedMemHandle);
}

// GetMemPointer returns a pointer to the attached shared memory.

void * 
SharedMemory::GetMemPointer()
{
	pgpAssert(IsMemAttached());

	return mPMem;
}

// GetMemName returns a constant pointer to the name of the shared memory.

LPCSTR 
SharedMemory::GetMemName()
{
	pgpAssert(IsMemAttached());

	return mSharedMemName;
}

// Attach attaches already existing shared memory to the object.

DualErr 
SharedMemory::Attach(LPCSTR memName)
{
	DualErr		derr;
	PGPBoolean	openedMapping	= FALSE;

	pgpAssertStrValid(memName);
	pgpAssert(!IsMemAttached());

	// Try to open the shared memory object.
	mSharedMemHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memName);

	if (IsNull(mSharedMemHandle))
		derr = DualErr(kPGDMinorError_OpenFileMappingFailed, GetLastError());

	openedMapping = derr.IsntError();

	// If succeeded, map a view of the shared memory.
	if (derr.IsntError())
	{
		mPMem = MapViewOfFile(mSharedMemHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (IsNull(mPMem))
		{
			derr = DualErr(kPGDMinorError_MapViewOfFileFailed, 
				GetLastError());
		}
	}

	if (derr.IsntError())
	{
		mSharedMemName = memName;
	}

	if (derr.IsError())
	{
		if (openedMapping)
		{
			CloseHandle(mSharedMemHandle);
			mSharedMemHandle = NULL;
		}
	}

	return derr;
}

// Detach detaches shared memory we previously attached.

DualErr 
SharedMemory::Detach()
{
	DualErr derr;

	pgpAssert(IsMemAttached());

	if (!UnmapViewOfFile(mPMem))
		derr = DualErr(kPGDMinorError_UnmapViewOfFileFailed, GetLastError());

	mPMem = NULL;

	if (!CloseHandle(mSharedMemHandle))
		derr = DualErr(kPGDMinorError_CloseHandleFailed, GetLastError());

	mSharedMemHandle = NULL;

	return derr;
}

// CreateAndAttach creates new shared memory and attaches it to the object.

DualErr 
SharedMemory::CreateAndAttach(LPCSTR memName, PGPUInt32 memSize)
{
	DualErr		derr;
	PGPBoolean	createdMapping	= FALSE;

	pgpAssertStrValid(memName);

	// Create the file mapping for the memory.
	mSharedMemHandle = CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL, 
		PAGE_READWRITE, 0, memSize, memName);

	if (IsntNull(mSharedMemHandle))
	{
		// Make sure this memory doesn't already exist.
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			derr = DualErr(kPGDMinorError_CreateFileMappingFailed, 
				GetLastError());

			CloseHandle(mSharedMemHandle);
			mSharedMemHandle = NULL;
		}
	}
	else
	{
		derr = DualErr(kPGDMinorError_CreateFileMappingFailed, 
			GetLastError());
	}

	createdMapping = derr.IsntError();

	// If succeeded, map a view of the shared memory.
	if (derr.IsntError())
	{
		mPMem = MapViewOfFile(mSharedMemHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (IsNull(mPMem))
		{
			derr = DualErr(kPGDMinorError_MapViewOfFileFailed, 
				GetLastError());
		}
	}

	if (derr.IsntError())
	{
		mSharedMemName = memName;
	}

	if (derr.IsError())
	{
		if (createdMapping)
		{
			CloseHandle(mSharedMemHandle);
			mSharedMemHandle = NULL;
		}
	}

	return derr;
}
