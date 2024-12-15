//////////////////////////////////////////////////////////////////////////////
// SharedMemory.h
//
// Declaration of class SharedMemory.
//////////////////////////////////////////////////////////////////////////////

// $Id: SharedMemory.h,v 1.3 1998/12/14 19:01:30 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_SharedMemory_h	// [
#define Included_SharedMemory_h

#include "DualErr.h"


/////////////////////
// Class SharedMemory
/////////////////////

class SharedMemory
{
public:
	SharedMemory();
	~SharedMemory();

	PGPBoolean	IsMemAttached();
	void *		GetMemPointer();
	LPCSTR		GetMemName();

	DualErr		Attach(LPCSTR memName);
	DualErr		Detach();

	DualErr		CreateAndAttach(LPCSTR memName, PGPUInt32 memSize);

private:
	CString		mSharedMemName;
	HANDLE		mSharedMemHandle;
	void		*mPMem;
};

#endif // Included_SharedMemory_h
