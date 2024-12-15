//////////////////////////////////////////////////////////////////////////////
// UserMemoryManager.cpp
//
// Definition of class UserMemoryManager.
//////////////////////////////////////////////////////////////////////////////

// $Id: UserMemoryManager.cpp,v 1.4 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"
#include "UserMemoryManager.h"


////////////
// Constants
////////////

const PGPUInt16 kPGPdiskDefaultLMIDepth = 32;


//////////////////////////////////////////////////
// Class UserMemoryManager public member functions
//////////////////////////////////////////////////

// The UserMemoryManager default constructor.

UserMemoryManager::UserMemoryManager() 
	: mLMIAllocator(kPGPdiskDefaultLMIDepth)
{
	mInitErr = mLMIAllocator.mInitErr;
}

// The UserMemoryManager destructor.

UserMemoryManager::~UserMemoryManager()
{
}

// LockUserMem locks memory passed in from User-Mode.

DualErr 
UserMemoryManager::LockUserMem(void *pMem, PGPUInt32 nBytes)
{
	DualErr			derr;
	LockedMemInfo	*pLMI;

	derr = InternalLockUserMem(pMem, nBytes, &pLMI);

	return derr;
}

// MapUserMem locks memory passed in from User-Mode and maps it to a system
// address

DualErr 
UserMemoryManager::MapUserMem(void **pMem, PGPUInt32 nBytes)
{
	DualErr			derr;
	LockedMemInfo	*pLMI;

	// Lock the memory.
	derr = InternalLockUserMem((* pMem), nBytes, &pLMI);

	// Return mapped address.
	if (derr.IsntError())
	{
		(* pMem) = pLMI->pSysMem;
	}

	return derr;
}

// UnlockUnmapUserMem unlocks user memory previously locked and mapped.

DualErr 
UserMemoryManager::UnlockUnmapUserMem(void *pMem, PGPUInt32 nBytes)
{
	DualErr derr;

	derr = InternalUnlockUnmapUserMem(pMem, nBytes);

	return derr;
}


///////////////////////////////////////////////////
// Class UserMemoryManager private member functions
///////////////////////////////////////////////////

// InternalLockUserMem locks and maps user memory.

DualErr 
UserMemoryManager::InternalLockUserMem(
	void			*pMem, 
	PGPUInt32		nBytes, 
	LockedMemInfo	**ppLMI)
{
	DualErr			derr;
	LockedMemInfo	*pLMI;
	PGPBoolean		allocedLMI, allocedMdl;

	allocedLMI = allocedMdl = FALSE;

	pgpAssertAddrValid(ppLMI, LockedMemInfo);

	// Get memory for locked mem info structure.
	derr = mLMIAllocator.Allocate(&pLMI);
	allocedLMI = derr.IsntError();

	// Allocate and initialize an MDL.
	if (derr.IsntError())
	{
		pLMI->pMem		= pMem;
		pLMI->nBytes	= nBytes;
		pLMI->pMdl		= IoAllocateMdl(pMem, nBytes, FALSE, FALSE, NULL); 

		if (IsNull(pLMI->pMdl))
			derr = DualErr(kPGDMinorError_IoAllocateMdlFailed);

		allocedMdl = derr.IsntError();
	}

	// Try to lock down the client's area.
	if (derr.IsntError())
	{
		__try 
		{
			MmProbeAndLockPages(pLMI->pMdl, UserMode, IoModifyAccess);
		} 
		__except(EXCEPTION_EXECUTE_HANDLER) 
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	// Add the memory info to the global list.
	if (derr.IsntError())
	{
		pLMI->pSysMem = MmGetSystemAddressForMdl(pLMI->pMdl);

		mLMIList.PublicLock();
		mLMIList.InsertHead(pLMI);
		mLMIList.PublicUnlock();
	}

	if (derr.IsError())
	{
		if (allocedLMI)
			mLMIAllocator.Free(pLMI);

		if (allocedMdl)
			IoFreeMdl(pLMI->pMdl);
	}

	if (derr.IsntError())
	{
		(* ppLMI) = pLMI;
	}

	return derr;	
}

// InternalUnlockUnmapUserMem unlocks user memory previously locked and
// mapped.

DualErr 
UserMemoryManager::InternalUnlockUnmapUserMem(void *pMem, PGPUInt32 nBytes)
{
	DualErr			derr;
	LockedMemInfo	*pLMI;
	PGPBoolean		foundMemInfo	= FALSE;
	PGPUInt32		i;

	mLMIList.PublicLock();

	pLMI = mLMIList.Head();

	// Find the info structure for this memory.
	for (i = 0; i < mLMIList.Count(); i++)
	{
		if (IsntNull(pLMI))
		{
			// Check BOTH original user address and mapped system address.
			if (((pLMI->pMem == pMem) || (pLMI->pSysMem == pMem)) && 
				(pLMI->nBytes == nBytes))
			{
				foundMemInfo = TRUE;
				break;
			}
			else
			{
				pLMI = mLMIList.Next(pLMI);
			}
		}
		else
		{
			foundMemInfo = FALSE;
			break;
		}
	}

	if (!foundMemInfo)
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Unlock the memory and free the list entry.
	if (derr.IsntError())
	{
		MmUnlockPages(pLMI->pMdl);
		IoFreeMdl(pLMI->pMdl);

		mLMIList.Remove(pLMI);
		mLMIAllocator.Free(pLMI);
	}

	mLMIList.PublicUnlock();

	return derr;	
}
