//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterfaceVolumes.cpp
//
// Functions for PGPdisk volume mounting and unmounting.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterfaceVolumes.cpp,v 1.5 1999/03/31 23:51:07 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"
#include "Packets.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"

#include "CPGPdiskInterface.h"
#include "CPGPdiskInterfaceVolumes.h"
#include "Globals.h"
#include "KernelModeUtils.h"
#include "PGPdisk.h"


//////////////////////////////////////
// Mount and unmount utility functions
//////////////////////////////////////

// MountPGPdisk mounts the PGPdisk file specified by 'path' and returns the
// drive number it was mounted on.

DualErr 
CPGPdiskInterface::MountPGPdisk(
	LPCSTR			path, 
	CipherContext	*pContext, 
	PGPUInt8		drive, 
	PGPBoolean		mountReadOnly)
{
	DualErr		derr;
	PGPBoolean	createdPGPdisk	= FALSE;
	PGPdisk		*pPGD;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pContext, CipherContext);

	// We must first check if we have space for another PGPdisk.
 	if (mPGPdisks.GetNumPGPdisks() >= kMaxMountedPGPdisks)
		derr = DualErr(kPGDMinorError_MaxPGPdisksMounted);

	// Search for any pre-existing PGPdisks corresponding to the path.
	if (derr.IsntError())
	{
		if (mPGPdisks.FindPGPdisk(path))
			derr = DualErr(kPGDMinorError_PGPdiskAlreadyMounted);
	}

	// Validate the PGPdisk.
	if (derr.IsntError())
	{
		derr = ValidatePGPdisk(path);
	}

	// If the PGPdisk is opened by someone else with write access, fail.
	if (derr.IsntError())
	{
		if (IsFileInUseByWriter(path))
			derr = DualErr(kPGDMinorError_PGPdiskAlreadyInUse);
	}

	// Create a new PGPdisk object.
	if (derr.IsntError())
	{
		pPGD = new PGPdisk;

		if (IsNull(pPGD))
			derr = DualErr(kPGDMinorError_OutOfMemory);

		createdPGPdisk = derr.IsntError();
	}

	if (derr.IsntError())
	{
		derr = pPGD->mInitErr;
	}

	// Attempt our mount.
	if (derr.IsntError())
	{
		derr = pPGD->Mount(path, pContext, drive, mountReadOnly);
	}

	// Attach the object to the global list.
	if (derr.IsntError())
	{
		mPGPdisks.AddPGPdisk(pPGD);
	}

	// Cleanup if an error occurred.
	if (derr.IsError())
	{
		if (createdPGPdisk)
		{
			if (pPGD->Mounted())
				pPGD->Unmount(TRUE);

			delete pPGD;
		}
	}

	return derr;
}

// UnmountPGPdisk unmounts the PGPdisk specified by 'drive'.

DualErr 
CPGPdiskInterface::UnmountPGPdisk(PGPUInt8 drive, PGPBoolean isThisEmergency)
{
	DualErr		derr;
	PGPBoolean	removedPGPdisk, unmountedChildren;
	PGPdisk		*pPGD;
	PGPUInt32	i	= 0;

	removedPGPdisk = unmountedChildren = FALSE;

	pgpAssert(IsLegalDriveNumber(drive));

	// Verify that the PGPdisk exists and is mounted.
	if (IsNull(pPGD = mPGPdisks.FindPGPdisk(drive)))
		derr = DualErr(kPGDMinorError_PGPdiskNotMounted);

	// If this PGPdisk has other PGPdisks mounted on it, we must perform a
	// depth-first unmount of all those PGPdisks first.

	while (derr.IsntError())
	{
		PGPdisk *pClient;

		if (!(pClient = Interface->mPGPdisks.EnumPGPdisks(i++)))
			break;

		if (!IsUNCPath(pClient->GetPath()) &&
			(pClient->GetLocalHostDrive() == drive))
		{
			i = 0;
			derr = UnmountPGPdisk(pClient->GetDrive(), isThisEmergency);

			if (isThisEmergency)
				derr = DualErr::NoError;

			unmountedChildren = derr.IsntError();
		}
	}

	// Check if the PGPdisk has open files.
	if (derr.IsntError())
	{
		// If we unmounted any children, we may have to wait a little while
		// for the system to flush its cache. Try about 10 times.

		if (unmountedChildren)
		{
			i = 0;

			while (pPGD->HasOpenFiles() && (i++ < kMaxCheckOpenFilesAttempts))
			{
				LARGE_INTEGER	ticks	= 
					RtlConvertLongToLargeInteger(-500 * 10000);

				KeDelayExecutionThread(KernelMode, FALSE, &ticks);
			}
		}

		if (pPGD->HasOpenFiles())
			derr = DualErr(kPGDMinorError_FilesOpenOnDrive);

		if (isThisEmergency)
			derr = DualErr::NoError;
	}

	// Remove the object from the global list and attempt the unmount.
	if (derr.IsntError())
	{
		mPGPdisks.RemovePGPdisk(pPGD);
		removedPGPdisk = TRUE;

		derr = pPGD->Unmount(isThisEmergency);
	}

	// Delete the PGPdisk object.
	if (derr.IsntError())
	{
		delete pPGD;
	}

	// Cleanup if an error occurred;
	if (derr.IsError())
	{
		if (removedPGPdisk && pPGD->Mounted())
			mPGPdisks.AddPGPdisk(pPGD);
	}

	return derr;
}

// UnmountAllPGPdisks unmounts all PGPdisks in the system.

DualErr 
CPGPdiskInterface::UnmountAllPGPdisks(PGPBoolean isThisEmergency)
{
	DualErr		derr, storedDerr;
	PGPUInt8	drive;
	PGPUInt32	i	= 0;
	PGPdisk		*pPGD;

	// Note how we enumerate on position 0, and increase this only when we
	// find a drive we can't unmount for whatever reason. This is because
	// if a drive can't be unmounted it won't be removed, and thus 'sticks'
	// at position 0, 1, etc.

	while (pPGD = mPGPdisks.EnumPGPdisks(i))
	{
		pgpAssert(pPGD->Mounted());
		drive = pPGD->GetDrive();

		// Now attempt the unmount.
		if (derr.IsntError())
		{
			derr = UnmountPGPdisk(drive, isThisEmergency);
		}

		// Remember only the first error we see.
		if (derr.IsError())
		{
			if (storedDerr.IsntError())
			{
				storedDerr = derr;
			}

			derr = DualErr::NoError;
			i++;
		}
	}

	return storedDerr;
}
