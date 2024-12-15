//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterfaceComm.cpp
//
// Functions for driver/application communication.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterfaceComm.cpp,v 1.1.2.30 1998/07/06 08:57:42 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "Packets.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"

#include "CPGPdiskInterface.h"
#include "CPGPdiskInterfaceComm.h"
#include "Globals.h"
#include "KernelModeUtils.h"
#include "NtThreadSecurity.h"
#include "PGPdisk.h"


//////////////////////////////
// Packet processing functions
//////////////////////////////

// ProcessMountCallback mounts a PGPdisk in the context of our utility
// thread.

void 
CPGPdiskInterface::ProcessMountCallback(PGPUInt32 refData)
{
	DualErr		derr;
	HANDLE		token;
	PAD_Mount	pMNT;

	pMNT = (PAD_Mount) refData;
	pgpAssertAddrValid(pMNT, AD_Mount);

	// Make copy of security token of calling process.
	if (!IsImpersonationTokenSet())
		derr = SetImpersonationToken(Interface->mCallerProcId);

	if (derr.IsntError())
	{
		// Mount the PGPdisk.
		derr = Interface->MountPGPdisk(pMNT->path, pMNT->pContext, 
			pMNT->drive, pMNT->readOnly);
	}

	(* pMNT->header.pDerr) = derr;
}

// ProcessMount processes a kAD_Mount request.

DualErr 
CPGPdiskInterface::ProcessMount(PAD_Mount pMNT, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedContext, mappedPath, mappedOutDrive;

	mappedContext = mappedPath = mappedOutDrive = FALSE;

	// Check packet size.
	if (size < sizeof(AD_Mount))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check desired drive letter.
	if (derr.IsntError())
	{
		if (!IsLegalDriveNumber(pMNT->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Map embedded pointers.
	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pMNT->pContext, 
			sizeof(CipherContext));

		mappedContext = derr.IsntError();
	}

	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pMNT->path, 
			pMNT->sizePath);

		mappedPath = derr.IsntError();
	}

	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pMNT->pDrive, 
			sizeof(PGPUInt8));

		mappedOutDrive = derr.IsntError();
	}

	// Perform the mount.
	if (derr.IsntError())
	{
		// Save process ID of current process.
		mCallerProcId = (PGPUInt32) PsGetCurrentProcessId();

		// Perform the mount.
		derr = mUtilityThread.ScheduleSyncCallback(ProcessMountCallback, 
			(PGPUInt32) pMNT);
	}

	if (derr.IsntError())
	{
		derr = (* pMNT->header.pDerr);
	}

	// Find the drive of the mounted PGPdisk.
	if (derr.IsntError())
	{
		PGPdisk *pPGD = mPGPdisks.FindPGPdisk(pMNT->path);

		pgpAssertAddrValid(pPGD, PGPdisk);
		(* pMNT->pDrive) = pPGD->GetDrive();
	}

	// Unmap embedded pointers.
	if (mappedOutDrive)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pMNT->pDrive, 
			sizeof(PGPUInt8));
	}

	if (mappedPath)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pMNT->path, 
			pMNT->sizePath);
	}
	
	if (mappedContext)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pMNT->pContext, 
			sizeof(CipherContext));
	}

	return derr;
}


// ProcessUnmountCallback unmounts a PGPdisk in the context of our utility
// thread.

void 
CPGPdiskInterface::ProcessUnmountCallback(PGPUInt32 refData)
{
	DualErr		derr;
	PAD_Unmount	pUNMNT;

	pUNMNT = (PAD_Unmount) refData;
	pgpAssertAddrValid(pUNMNT, AD_Unmount);

	derr = Interface->UnmountPGPdisk(pUNMNT->drive, pUNMNT->isThisEmergency);

	(* pUNMNT->header.pDerr) = derr;
}

// ProcessUnmount processes a kAD_Unmount request.

DualErr 
CPGPdiskInterface::ProcessUnmount(PAD_Unmount pUNMNT, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_Unmount))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		if (!IsLegalDriveNumber(pUNMNT->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the unmount.
	if (derr.IsntError())
	{
		derr = mUtilityThread.ScheduleSyncCallback(ProcessUnmountCallback, 
			(PGPUInt32) pUNMNT);
	}

	if (derr.IsntError())
	{
		derr = (* pUNMNT->header.pDerr);
	}

	return derr;
}

// ProcessQueryVersion processes a kAD_QueryVersion request.

DualErr 
CPGPdiskInterface::ProcessQueryVersion(PAD_QueryVersion pQV, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedVerBool	= FALSE;

	// Check packet size.
	if (size < sizeof(AD_QueryVersion))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Map embedded pointers.
	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pQV->pDriverVersion, 
			sizeof(PGPUInt32));

		mappedVerBool = derr.IsntError();
	}

	// Perform the query.
	if (derr.IsntError())
	{
		mPGPdiskAppVersion = pQV->appVersion;
		(* pQV->pDriverVersion) = kPGPdiskDriverVersion;
	}

	// Unmap embedded pointers.
	if (mappedVerBool)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQV->pDriverVersion, 
			sizeof(PGPUInt32));
	}

	return derr;
}

// ProcessQueryMounted processes a kAD_QueryMounted request.

DualErr 
CPGPdiskInterface::ProcessQueryMounted(PAD_QueryMounted pQM, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedIsDiskBool, mappedPath;

	mappedIsDiskBool = mappedPath = FALSE;

	// Check packet size.
	if (size < sizeof(AD_QueryMounted))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Map embedded pointers and check parameters.
	if (derr.IsntError())
	{
		if (pQM->trueIfUsePath)
		{
			derr = mUserMemManager.MapUserMem((void **) &pQM->path, 
				pQM->sizePath);

			mappedPath = derr.IsntError();
		}
		else if (!IsLegalDriveNumber(pQM->drive))
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pQM->pIsPGPdisk, 
			sizeof(PGPBoolean));

		mappedIsDiskBool = derr.IsntError();
	}

	// Perform the query.
	if (derr.IsntError())
	{
		if (pQM->trueIfUsePath)
		{
			if (IsntNull(Interface->mPGPdisks.FindPGPdisk(pQM->path)))
				(* pQM->pIsPGPdisk) = TRUE;
			else
				(* pQM->pIsPGPdisk) = FALSE;
		}
		else
		{
			if (IsntNull(Interface->mPGPdisks.FindPGPdisk(pQM->drive)))
				(* pQM->pIsPGPdisk) = TRUE;
			else
				(* pQM->pIsPGPdisk) = FALSE;
		}
	}

	// Unmap embedded pointers.
	if (mappedPath)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQM->path, pQM->sizePath);
	}

	if (mappedIsDiskBool)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQM->pIsPGPdisk, 
			sizeof(PGPBoolean));
	}

	return derr;
}

// ProcessQueryOpenFilesCallback queries about open files on a volume in the
// context of our utility thread.

void 
CPGPdiskInterface::ProcessQueryOpenFilesCallback(PGPUInt32 refData)
{
	DualErr				derr;
	PAD_QueryOpenFiles	pQOF;
	Volume				vol;

	pQOF = (PAD_QueryOpenFiles) refData;
	pgpAssertAddrValid(pQOF, AD_QueryOpenFiles);

	derr = vol.AttachLocalVolume(pQOF->drive);

	if (derr.IsntError())
	{
		(* pQOF->pHasOpenFiles) = vol.HasOpenFiles();
	}

	if (vol.AttachedToLocalVolume())
		vol.DetachLocalVolume();

	(* pQOF->header.pDerr) = derr;
}

// ProcessQueryOpenFiles processes a kAD_QueryOpenFiles request.

DualErr 
CPGPdiskInterface::ProcessQueryOpenFiles(
	PAD_QueryOpenFiles	pQOF, 
 	PGPUInt32			size)
{
	DualErr		derr;
	PGPBoolean	mappedOpenBool	= FALSE;

	// Check packet size.
	if (size < sizeof(AD_QueryOpenFiles))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Map embedded pointers.
	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pQOF->pHasOpenFiles, 
			sizeof(PGPBoolean));

		mappedOpenBool = derr.IsntError();
	}

	// Check drive letter.
	if (derr.IsntError())
	{
		if (!IsLegalDriveNumber(pQOF->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the query.
	if (derr.IsntError())
	{
		derr = mUtilityThread.ScheduleSyncCallback(
			ProcessQueryOpenFilesCallback, (PGPUInt32) pQOF);
	}

	if (derr.IsntError())
	{
		derr = (* pQOF->header.pDerr);
	}

	// Unmap embedded pointers.
	if (mappedOpenBool)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQOF->pHasOpenFiles, 
			sizeof(PGPBoolean));
	}

	return derr;
}

// ProcessChangePrefs processes a DA_ChangePrefs request.

DualErr 
CPGPdiskInterface::ProcessChangePrefs(PAD_ChangePrefs pCP, PGPUInt32 size)
{
	DualErr derr;

	File dude;

	if (dude.Opened())
		dude.Close();

	// Check packet size.
	if (size < sizeof(AD_ChangePrefs))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check params.
	if (derr.IsntError())
	{
		if ((pCP->unmountTimeout == 0) || 
			(pCP->unmountTimeout > kMaxUnmountTimeout))
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	// Update preferences.
	if (derr.IsntError())
	{
		mAutoUnmount = pCP->autoUnmount;
		mUnmountTimeout = pCP->unmountTimeout;
	}

	return derr;
}

// ProcessLockUnlockMem processes a DA_LockUnlockMem request.

DualErr 
CPGPdiskInterface::ProcessLockUnlockMem(
	PAD_LockUnlockMem	pLUM, 
 	PGPUInt32			size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_LockUnlockMem))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Lock/unlock the user memory.
	if (derr.IsntError())
	{
		if (pLUM->trueForLock)
		{
			derr = Interface->mUserMemManager.LockUserMem(pLUM->pMem, 
				pLUM->nBytes);
		}
		else
		{
			derr = Interface->mUserMemManager.UnlockUnmapUserMem(pLUM->pMem, 
				pLUM->nBytes);
		}
	}

	return derr;
}

// ProcessGetPGPdiskInfo processes a kAD_GetPGPdiskInfo request.

DualErr 
CPGPdiskInterface::ProcessGetPGPdiskInfo(
	PAD_GetPGPdiskInfo	pGPI, 
 	PGPUInt32			size)
{
	DualErr		derr;
	PGPBoolean	mappedArray	= FALSE;
	PGPUInt32	arraySize, i;

	// Check packet size.
	if (size < sizeof(AD_GetPGPdiskInfo))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Map embedded pointers.
	if (derr.IsntError())
	{
		arraySize = pGPI->arrayElems * sizeof(PGPdiskInfo);

		derr = mUserMemManager.MapUserMem((void **) &pGPI->pPDIArray, 
			arraySize);

		mappedArray = derr.IsntError();
	}

	// Fill in the array entries.
	if (derr.IsntError())
	{
		for (i = 0; i < pGPI->arrayElems; i++)
		{
			PGPdisk		*pPGD;
			PGPdiskInfo	*pPDI;

			pPDI = &pGPI->pPDIArray[i];

			if (IsNull(pPGD = mPGPdisks.EnumPGPdisks(i)))
			{
				pPDI->drive = kInvalidDrive;
			}
			else
			{
				pPDI->drive = pPGD->GetDrive();
				pPDI->sessionId = pPGD->GetUniqueSessionId();

				strcpy(pPDI->path, pPGD->GetPath());
			}
		}
	}

	// Unmap embedded pointers.
	if (mappedArray)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pGPI->pPDIArray, 
			arraySize);
	}

	return derr;
}

// ProcessLockUnlockVolCallback locks or unlocks a volume in the context of
// our utility thread.

void 
CPGPdiskInterface::ProcessLockUnlockVolCallback(PGPUInt32 refData)
{
	DualErr				derr;
	PAD_LockUnlockVol	pLUV;
	Volume				vol;

	pLUV = (PAD_LockUnlockVol) refData;
	pgpAssertAddrValid(pLUV, AD_LockUnlockVol);

	derr = vol.AttachLocalVolume(pLUV->drive);

	if (derr.IsntError())
	{
		switch (pLUV->lockOp)
		{
		case kLO_LockReadWrite:
			derr = vol.LockVolumeForReadWrite();
			break;

		case kLO_LockFormat:
			derr = vol.LockVolumeForFormat();
			break;

		case kLO_UnlockReadWrite:
		case kLO_UnlockFormat:
			derr = vol.UnlockVolume();
			break;

		default:
			derr = DualErr(kPGDMinorError_InvalidParameter);
			break;
		}
	}

	if (vol.AttachedToLocalVolume())
		vol.DetachLocalVolume();

	(* pLUV->header.pDerr) = derr;
}

// ProcessLockUnlockVol processes a kAD_LockUnlockVol request.

// *************************************************************************
// NOTE: Implementation of this function is currently broken because the
// lock goes away when 'vol' goes out of context. Will be fixed in v2.0.
// *************************************************************************

DualErr 
CPGPdiskInterface::ProcessLockUnlockVol(
	PAD_LockUnlockVol	pLUV, 
	PGPUInt32			size)
{
	DualErr	derr;
	Volume	vol;

	// Check packet size.
	if (size < sizeof(AD_LockUnlockVol))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		if (!IsLegalDriveNumber(pLUV->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Fail for security reasons if volume isn't a PGPdisk.
	if (derr.IsntError())
	{
		if (IsNull(mPGPdisks.FindPGPdisk(pLUV->drive)))
			derr = DualErr(kPGDMinorError_AccessDeniedToVolume);
	}

	// Perform the lock/unlock.
	if (derr.IsntError())
	{
		derr = mUtilityThread.ScheduleSyncCallback(
			ProcessLockUnlockVolCallback, (PGPUInt32) pLUV);
	}

	if (derr.IsntError())
	{
		derr = (* pLUV->header.pDerr);
	}

	return derr;
}

// ProcessReadWriteVolCallback reads from or writes to a volume in the
// context of our utility thread.

void 
CPGPdiskInterface::ProcessReadWriteVolCallback(PGPUInt32 refData)
{
	DualErr				derr;
	PAD_ReadWriteVol	pRWV;
	Volume				vol;

	pRWV = (PAD_ReadWriteVol) refData;
	pgpAssertAddrValid(pRWV, AD_ReadWriteVol);

	derr = vol.AttachLocalVolume(pRWV->drive);

	// Fail for security reasons if volume isn't a PGPdisk.
	if (derr.IsntError())
	{
		if (IsNull(Interface->mPGPdisks.FindPGPdisk(pRWV->drive)))
			derr = DualErr(kPGDMinorError_AccessDeniedToVolume);
	}
	
	if (derr.IsntError())
	{
		if (pRWV->trueIfRead)
			derr = vol.Read(pRWV->buf, pRWV->pos, pRWV->nBlocks);
		else
			derr = vol.Write(pRWV->buf, pRWV->pos, pRWV->nBlocks);
	}

	if (vol.AttachedToLocalVolume())
		vol.DetachLocalVolume();

	(* pRWV->header.pDerr) = derr;
}

// ProcessReadWriteVol processes a kAD_ReadWriteVol request.

DualErr 
CPGPdiskInterface::ProcessReadWriteVol(PAD_ReadWriteVol pRWV, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedBuf	= FALSE;

	// Check packet size.
	if (size < sizeof(AD_ReadWriteVol))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		if (!IsLegalDriveNumber(pRWV->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Map embedded pointers.
	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pRWV->buf, 
			pRWV->bufSize);

		mappedBuf = derr.IsntError();
	}

	// Perform the read/write.
	if (derr.IsntError())
	{
		derr = mUtilityThread.ScheduleSyncCallback(
			ProcessReadWriteVolCallback, (PGPUInt32) pRWV);
	}

	if (derr.IsntError())
	{
		derr = (* pRWV->header.pDerr);
	}

	// Unmap embedded pointers.
	if (mappedBuf)
		mUserMemManager.UnlockUnmapUserMem((void *) pRWV->buf, pRWV->bufSize);

	return derr;
}

// ProcessQueryVolInfoCallback gets info about a volume in the context of our
// utility thread.

void 
CPGPdiskInterface::ProcessQueryVolInfoCallback(PGPUInt32 refData)
{
	DualErr				derr;
	PAD_QueryVolInfo	pQVI;
	Volume				vol;

	pQVI = (PAD_QueryVolInfo) refData;
	pgpAssertAddrValid(pQVI, AD_QueryVolInfo);

	derr = vol.AttachLocalVolume(pQVI->drive);

	if (derr.IsntError())
	{
		(* pQVI->pBlockSize) = vol.GetBlockSize();
		(* pQVI->pTotalBlocks) = vol.GetTotalBlocks();
	}

	if (vol.AttachedToLocalVolume())
		vol.DetachLocalVolume();

	(* pQVI->header.pDerr) = derr;
}

// ProcessQueryVolInfo processes a kAD_QueryVolInfo request.

DualErr 
CPGPdiskInterface::ProcessQueryVolInfo(PAD_QueryVolInfo pQVI, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedBlockSize, mappedTotalBlocks;

	mappedBlockSize = mappedTotalBlocks = FALSE;

	// Check packet size.
	if (size < sizeof(AD_QueryVolInfo))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pQVI, AD_QueryVolInfo);

		if (!IsLegalDriveNumber(pQVI->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Map embedded pointers.
	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pQVI->pBlockSize, 
			sizeof(PGPUInt16));

		mappedBlockSize = derr.IsntError();
	}

	if (derr.IsntError())
	{
		derr = mUserMemManager.MapUserMem((void **) &pQVI->pTotalBlocks, 
			sizeof(PGPUInt64));

		mappedTotalBlocks = derr.IsntError();
	}

	// Perform the query.
	if (derr.IsntError())
	{
		derr = mUtilityThread.ScheduleSyncCallback(
			ProcessQueryVolInfoCallback, (PGPUInt32) pQVI);
	}

	if (derr.IsntError())
	{
		derr = (* pQVI->header.pDerr);
	}

	// Unmap embedded pointers.
	if (mappedTotalBlocks)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQVI->pTotalBlocks, 
			sizeof(PGPUInt64));
	}

	if (mappedBlockSize)
	{
		mUserMemManager.UnlockUnmapUserMem((void *) pQVI->pBlockSize, 
			sizeof(PGPUInt16));
	}

	return derr;
}

// ProcessNotifyUserLogoff processes a kAD_NotifyUserLogoff request.

DualErr
CPGPdiskInterface::ProcessNotifyUserLogoff(
	PAD_NotifyUserLogoff	pNUL, 
	PGPUInt32				size)
{
	DualErr derr;

	// Begin attempts every second to unmount all PGPdisks.
	mUnmountAllMode = TRUE;

	return derr;
}

// ProcessADPacket is called to process an AD (application to driver) packet.

NTSTATUS 
CPGPdiskInterface::ProcessADPacket(PADPacketHeader pPacket, PGPUInt32 size)
{
	DualErr		derr;
	PGPBoolean	mappedDerr	= FALSE;

	// Validate the packet. Note that the packet has already been buffered in
	// non-paged memory by the system.

	if (IsNull(pPacket))
	{
		return STATUS_INVALID_PARAMETER;
	}
	if (size < sizeof(ADPacketHeader))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}
	else if (pPacket->magic != kPGPdiskADPacketMagic)
	{
		return STATUS_INVALID_PARAMETER;
	}

	DebugOut("PGPdisk: %s received\n", GetADPacketName(pPacket->code));

	// Map embedded error pointer.
	derr = mUserMemManager.MapUserMem((void **) &pPacket->pDerr, 
		sizeof(DualErr));

	mappedDerr = derr.IsntError();

	// Was there an error during driver initialization?
	if (derr.IsntError())
	{
		derr = mPGPdiskDriver->mInitErr;
	}

	// Process the query.
	if (derr.IsntError())
	{
		switch (pPacket->code)
		{
		case kAD_Mount:				// mount a PGPdisk
			derr = ProcessMount((PAD_Mount) pPacket, size);
			break;

		case kAD_Unmount:			// unmount a PGPdisk
			derr = ProcessUnmount((PAD_Unmount) pPacket, size);
			break;

		case kAD_QueryVersion:		// exchange version numbers
			derr = ProcessQueryVersion((PAD_QueryVersion) pPacket, size);
			break;

		case kAD_QueryMounted:		// ask if drive/path is PGPdisk
			derr = ProcessQueryMounted((PAD_QueryMounted) pPacket, size);
			break;

		case kAD_QueryOpenFiles:	// app asks if drive has open files
			derr = ProcessQueryOpenFiles((PAD_QueryOpenFiles) pPacket, size);
			break;

		case kAD_ChangePrefs:		// app changes preferences
			derr = ProcessChangePrefs((PAD_ChangePrefs) pPacket, size);
			break;

		case kAD_LockUnlockMem:		// app wants to lock or unlock memory
			derr = ProcessLockUnlockMem((PAD_LockUnlockMem) pPacket, size);
			break;

		case kAD_GetPGPdiskInfo:	// fill in array with PGPdisk info
			derr = ProcessGetPGPdiskInfo((PAD_GetPGPdiskInfo) pPacket, size);
			break;

		case kAD_LockUnlockVol:		// lock/unlock volume
			derr = ProcessLockUnlockVol((PAD_LockUnlockVol) pPacket, size);
			break;

		case kAD_ReadWriteVol:		// read/write to volume
			derr = ProcessReadWriteVol((PAD_ReadWriteVol) pPacket, size);
			break;

		case kAD_QueryVolInfo:		// get extra info about a volume
			derr = ProcessQueryVolInfo((PAD_QueryVolInfo) pPacket, size);
			break;

		case kAD_NotifyUserLogoff:	// user logging off, unmount PGPdisks
			derr = ProcessNotifyUserLogoff((PAD_NotifyUserLogoff) pPacket, 
				size);
			break;

		default:
			derr = DualErr(kPGDMinorError_InvalidParameter);
			break;
		}
	}

	// Unmap embedded error pointer.
	if (mappedDerr)
	{
		(* pPacket->pDerr) = derr;

		mUserMemManager.UnlockUnmapUserMem(pPacket->pDerr, sizeof(DualErr));

		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_INVALID_PARAMETER;
	}
}


///////////////////////
// Dispatcher functions
///////////////////////

// DeviceControl handles communications from the app.

NTSTATUS 
CPGPdiskInterface::DeviceControl(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	switch (I.IoctlCode())
	{
	case IOCTL_PGPDISK_SENDPACKET:

		status = ProcessADPacket((PADPacketHeader) I.IoctlBuffer(), 
			I.IoctlInputBufferSize());
		break;

	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	return I.Complete(status);
}
