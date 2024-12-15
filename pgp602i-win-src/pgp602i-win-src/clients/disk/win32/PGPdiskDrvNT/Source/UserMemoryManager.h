//////////////////////////////////////////////////////////////////////////////
// UserMemoryManager.h
//
// Declaration of class UserMemoryManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: UserMemoryManager.h,v 1.1.2.3 1998/07/06 08:57:58 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_UserMemoryManager_h	// [
#define Included_UserMemoryManager_h

#include "DualErr.h"

#include "LockableList.h"
#include "LookasideList.h"
#include "PGPdisk.h"


////////
// Types
////////

typedef struct LockedMemInfo
{
	LIST_ENTRY m_ListEntry;	// required by the list templates

	void		*pMem;		// pointer to locked memory
	void		*pSysMem;	// mapped to system memory
	PGPUInt32	nBytes;		// size of memory locked;
	PMDL		pMdl;		// MDL

} LockedMemInfo;

typedef LookasideList<LockedMemInfo> LockedMemInfoAllocator;
typedef LockableList<LockedMemInfo> LockedMemInfoList;


//////////////////////////
// Class UserMemoryManager
//////////////////////////

class UserMemoryManager
{
public:
	DualErr mInitErr;

				UserMemoryManager();
				~UserMemoryManager();

	DualErr		LockUserMem(void *pMem, PGPUInt32 nBytes);
	DualErr		MapUserMem(void **pMem, PGPUInt32 nBytes);

	DualErr		UnlockUnmapUserMem(void *pMem, PGPUInt32 nBytes);

private:
	LockedMemInfoAllocator	mLMIAllocator;		// locked mem info maker
	LockedMemInfoList		mLMIList;			// locked mem info list

	DualErr	InternalLockUserMem(void *pMem, PGPUInt32 nBytes, 
		LockedMemInfo **ppLMI);

	DualErr	InternalUnlockUnmapUserMem(void *pMem, PGPUInt32 nBytes);
};

#endif	// ] Included_UserMemoryManager_h
