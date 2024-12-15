//////////////////////////////////////////////////////////////////////////////
// SharedMemory.h
//
// Declaration of class SharedMemory.
//////////////////////////////////////////////////////////////////////////////

// $Id: SharedMemory.h,v 1.1.2.6 1998/08/04 02:10:32 nryan Exp $

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
