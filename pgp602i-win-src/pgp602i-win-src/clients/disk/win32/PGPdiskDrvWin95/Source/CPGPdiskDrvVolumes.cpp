//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvVolumes.cpp
//
// Contains high-level functions for working with PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvVolumes.cpp,v 1.2.2.17 1998/07/06 08:58:12 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrvVolumes.h"
#include "CPGPdiskDrvWinutils.h"
#include "Globals.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


//////////////////////////////////////
// Mount and unmount utility functions
//////////////////////////////////////

// MountPGPdisk mounts the PGPdisk file specified by 'path' and returns the
// drive number it was mounted on.

DualErr 
CPGPdiskDrv::MountPGPdisk(
	LPCSTR			path, 
	CipherContext	*context, 
	PGPUInt8		drive, 
	PGPBoolean		mountReadOnly)
{
	DualErr		derr;
	PDCB		newDcb;
	PGPBoolean	addedPGPdisk, claimedDcb, createdPGPdiskObject, isPathLooped;
	PGPdisk		*pPGD;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(context, CipherContext);

	addedPGPdisk = claimedDcb = createdPGPdiskObject = isPathLooped = FALSE;

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
		if (!(pPGD = new PGPdisk()))
			derr = DualErr(kPGDMinorError_OutOfMemory);

		createdPGPdiskObject = derr.IsntError();
	}
	
	if (derr.IsntError())
	{
		derr = pPGD->mInitErr;
	}

	// We now claim a new DCB.
	if (derr.IsntError())
	{
		newDcb = mDcbs.ClaimDcb();

		if (!newDcb)
			derr = DualErr(kPGDMinorError_DcbClaimFailed);

		claimedDcb = derr.IsntError();
	}
	
	// Attempt our mount.
	if (derr.IsntError())
	{
		derr = pPGD->Mount(path, context, newDcb, drive, mountReadOnly);
	}

	if (derr.IsntError())
	{
		// Attach the object to the global list.
		mPGPdisks.AddPGPdisk(pPGD);
		addedPGPdisk = TRUE;

		// Hook an ejection filter to the host.
		if (!IsUNCPath(pPGD->GetPath()))
			derr = HookEjectionFilter(pPGD->GetLocalHostDrive());
	}

	// Cleanup if an error occurred.
	if (derr.IsError())
	{
		if (addedPGPdisk)
		{
			mPGPdisks.RemovePGPdisk(pPGD);
		}

		if (createdPGPdiskObject)
		{
			if (pPGD->Mounted())
				pPGD->Unmount();

			delete pPGD;
		}

		if (claimedDcb)
		{
			mDcbs.ReleaseDcb(newDcb);
		}
	}

	return derr;
}

// UnmountPGPdisk unmounts the PGPdisk specified by 'drive'.

DualErr 
CPGPdiskDrv::UnmountPGPdisk(PGPUInt8 drive, PGPBoolean isThisEmergency)
{
	DualErr		derr;
	PDCB		oldDcb;
	PGPBoolean	removedPGPdisk	= FALSE;
	PGPdisk		*pPGD;
	PGPUInt32	i	= 0;

	pgpAssert(IsLegalDriveNumber(drive));

	// First we verify that the PGPdisk exists and is mounted.
	if (IsNull(pPGD = mPGPdisks.FindPGPdisk(drive)))
		derr = DualErr(kPGDMinorError_PGPdiskNotMounted);

	// If IO is disabled to the drive, make this an emergency automatically.
	if (derr.IsntError())
	{
		if (!pPGD->IsIOEnabled())
			isThisEmergency = TRUE;
	}

	// If I/O to this drive is in process, don't unmount. This OVERRIDES
	// 'isThisEmergency'.

	if (derr.IsntError())
	{
		if (pPGD->IsUnmountPending())
			derr = DualErr(kPGDMinorError_FailSilently);
		else if (mIopProcessor.IsDriveBusy(drive))
			derr = DualErr(kPGDMinorError_PGPdiskIsBusy);
		else
			pPGD->MarkUnmountPending(TRUE);
	}

	// If this PGPdisk has other PGPdisks mounted on it, we must perform a
	// depth-first unmount of all those PGPdisks first.

	while (derr.IsntError())
	{
		PGPdisk *pClient;

		if (!(pClient = Driver->mPGPdisks.EnumPGPdisks(i++)))
			break;

		if (!IsUNCPath(pClient->GetPath()) &&
			(pClient->GetLocalHostDrive() == drive))
		{
			i = 0;
			derr = UnmountPGPdisk(pClient->GetDrive(), isThisEmergency);
		}
	}

	// Check if the PGPdisk has open files.
	if (derr.IsntError())
	{
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

		oldDcb = pPGD->GetDcb();
		derr = pPGD->Unmount(isThisEmergency);
	}

	// Release the previously claimed DCB.
	if (derr.IsntError())
	{
		mDcbs.ReleaseDcb(oldDcb);
		delete pPGD;
	}

	// Cleanup if an error occurred;
	if (derr.IsError())
	{
		pPGD->MarkUnmountPending(FALSE);

		if (removedPGPdisk && pPGD->Mounted())
			mPGPdisks.AddPGPdisk(pPGD);
	}

	return derr;
}

// UnmountAllPGPdisks unmounts all PGPdisks in the system.

DualErr 
CPGPdiskDrv::UnmountAllPGPdisks(PGPBoolean isThisEmergency)
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

// UnmountAllPGPdisksCallback is scheduled to unmount all PGPdisk at a safe
// time.

VOID 
__stdcall 
CPGPdiskDrv::UnmountAllPGPdisksCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	DualErr		derr;
	PGPBoolean	warnIfUnmountsFail	= (PGPBoolean) Refdata;

	derr = Driver->UnmountAllPGPdisks(FALSE);

	if (derr.IsError() && warnIfUnmountsFail)
	{
		Driver->ReportError(kPGDMajorError_PGPdiskUnmountAllFailed, derr);
	}
}

// UnmountAllPGPdisksSafely schedules a callback to unmount all PGPdisks at a
// safe time.

void 
CPGPdiskDrv::UnmountAllPGPdisksSafely(PGPBoolean warnIfUnmountsFail)
{
	static RestrictedEvent_THUNK callbackThunk;

	Call_Restricted_Event(0, NULL, PEF_WAIT_NOT_CRIT, 
		(PVOID) warnIfUnmountsFail, UnmountAllPGPdisksCallback, 0, 
		&callbackThunk);
}
