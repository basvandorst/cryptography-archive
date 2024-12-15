//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvComm.cpp
//
// Functions for driver/application communication.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvComm.cpp,v 1.1.2.28 1998/07/06 08:58:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrvComm.h"
#include "Globals.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


//////////////////////////////
// Packet processing functions
//////////////////////////////

// ProcessMount processes a kAD_Mount request.

DualErr 
CPGPdiskDrv::ProcessMount(PAD_Mount pMNT, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_Mount))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check desired drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pMNT, AD_Mount);

		if (!IsLegalDriveNumber(pMNT->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the mount.
	if (derr.IsntError())
	{
		pgpAssertStrValid(pMNT->path);
		pgpAssertAddrValid(pMNT->pContext, CipherContext);

		derr = MountPGPdisk(pMNT->path, pMNT->pContext, pMNT->drive, 
			pMNT->readOnly);
	}

	if (derr.IsntError())
	{
		PGPdisk *pPGD = mPGPdisks.FindPGPdisk(pMNT->path);

		pgpAssertAddrValid(pPGD, PGPdisk);
		(* pMNT->pDrive) = pPGD->GetDrive();
	}

	return derr;
}

// ProcessUnmount processes a kAD_Unmount request.

DualErr 
CPGPdiskDrv::ProcessUnmount(PAD_Unmount pUNMNT, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_Unmount))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pUNMNT, AD_Unmount);

		if (!IsLegalDriveNumber(pUNMNT->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the unmount.
	if (derr.IsntError())
	{
		derr = UnmountPGPdisk(pUNMNT->drive, pUNMNT->isThisEmergency);
	}
	
	return derr;
}

// ProcessQueryVersion processes a kAD_QueryVersion request.

DualErr 
CPGPdiskDrv::ProcessQueryVersion(PAD_QueryVersion pQV, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_QueryVersion))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Perform the query.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pQV, AD_QueryVersion);
		pgpAssertAddrValid(pQV->pDriverVersion, PGPBoolean);

		mPGPdiskAppVersion = pQV->appVersion;
		(* pQV->pDriverVersion) = kPGPdiskDriverVersion;
	}

	return derr;
}

// ProcessQueryMounted processes a kAD_QueryMounted request.

DualErr 
CPGPdiskDrv::ProcessQueryMounted(PAD_QueryMounted pQM, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_QueryMounted))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check parameters.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pQM, AD_QueryMounted);
		pgpAssertAddrValid(pQM->pIsPGPdisk, PGPBoolean);

		if (pQM->trueIfUsePath)
		{
			pgpAssertStrValid(pQM->path);
		}
		else if (!IsLegalDriveNumber(pQM->drive))
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	// Perform the query.
	if (derr.IsntError())
	{
		if (pQM->trueIfUsePath)
		{
			if (IsntNull(Driver->mPGPdisks.FindPGPdisk(pQM->path)))
				(* pQM->pIsPGPdisk) = TRUE;
			else
				(* pQM->pIsPGPdisk) = FALSE;
		}
		else
		{
			if (IsntNull(Driver->mPGPdisks.FindPGPdisk(pQM->drive)))
				(* pQM->pIsPGPdisk) = TRUE;
			else
				(* pQM->pIsPGPdisk) = FALSE;
		}
	}

	return derr;
}

// ProcessQueryOpenFiles processes a kAD_QueryOpenFiles request.

DualErr 
CPGPdiskDrv::ProcessQueryOpenFiles(PAD_QueryOpenFiles pQOF, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_QueryOpenFiles))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pQOF, AD_QueryMounted);

		if (!IsLegalDriveNumber(pQOF->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the query.
	if (derr.IsntError())
	{
		Volume vol;

		// Prepare for the read/write.
		derr = vol.AttachLocalVolume(pQOF->drive);

		if (derr.IsntError())
		{
			(* pQOF->pHasOpenFiles) = vol.HasOpenFiles();
		}

		if (vol.AttachedToLocalVolume())
			vol.DetachLocalVolume();
	}

	return derr;
}

// ProcessChangePrefs processes a DA_ChangePrefs request.

DualErr 
CPGPdiskDrv::ProcessChangePrefs(PAD_ChangePrefs pCP, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_ChangePrefs))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check params.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pCP, AD_ChangePrefs);

		if ((pCP->unmountTimeout == 0) || 
			(pCP->unmountTimeout > kMaxUnmountTimeout))
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	// Update preferences.
	if (derr.IsntError())
	{
		mAutoUnmount	= pCP->autoUnmount;
		mUnmountTimeout	= pCP->unmountTimeout;
	}

	return derr;
}

// ProcessLockUnlockMem processes a DA_LockUnlockMem request.

DualErr 
CPGPdiskDrv::ProcessLockUnlockMem(PAD_LockUnlockMem pLUM, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_LockUnlockMem))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Lock the memory
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pLUM, AD_LockUnlockMem);
		pgpAssertAddrValid(pLUM->pMem, PGPUInt8);

		if (pLUM->trueForLock)
			derr = LockUserBuffer(pLUM->pMem, pLUM->nBytes);
		else
			derr = UnlockUserBuffer(pLUM->pMem, pLUM->nBytes);
	}

	return derr;
}

// ProcessGetPGPdiskInfo processes a kAD_GetPGPdiskInfo request.

DualErr 
CPGPdiskDrv::ProcessGetPGPdiskInfo(PAD_GetPGPdiskInfo pGPI, PGPUInt32 size)
{
	DualErr		derr;
	PGPUInt32	i;

	// Check packet size.
	if (size < sizeof(AD_GetPGPdiskInfo))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Fill in the array entries.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pGPI, AD_GetPGPdiskInfo);

		for (i = 0; i < pGPI->arrayElems; i++)
		{
			PGPdisk		*pPGD;
			PGPdiskInfo	*pPDI;

			pPDI = &pGPI->pPDIArray[i];
			pgpAssertAddrValid(pPDI, PGPdiskInfo);

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

	return derr;
}

// ProcessLockUnlockVol processes a kAD_LockUnlockVol request.

DualErr 
CPGPdiskDrv::ProcessLockUnlockVol(PAD_LockUnlockVol pLUV, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_LockUnlockVol))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pLUV, AD_LockUnlockVol);

		if (!IsLegalDriveNumber(pLUV->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the lock/unlock.
	if (derr.IsntError())
	{
		Volume vol;

		// Prepare for the read/write.
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

			if (vol.AttachedToLocalVolume())
				vol.DetachLocalVolume();
		}
	}

	return derr;
}

// ProcessReadWriteVol processes a kAD_ReadWriteVol request.

DualErr 
CPGPdiskDrv::ProcessReadWriteVol(PAD_ReadWriteVol pRWV, PGPUInt32 size)
{
	DualErr derr;

	// Check packet size.
	if (size < sizeof(AD_ReadWriteVol))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	// Check drive letter.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pRWV, AD_ReadWriteVol);

		if (!IsLegalDriveNumber(pRWV->drive))
			derr = DualErr(kPGDMinorError_InvalidParameter);
	}

	// Perform the read/write.
	if (derr.IsntError())
	{
		Volume vol;

		// Prepare for the read/write.
		derr = vol.AttachLocalVolume(pRWV->drive);

		// Perform the read/write.
		if (derr.IsntError())
		{
			if (pRWV->trueIfRead)
				derr = vol.Read(pRWV->buf, pRWV->pos, pRWV->nBlocks);
			else
				derr = vol.Write(pRWV->buf, pRWV->pos, pRWV->nBlocks);
		}

		if (vol.AttachedToLocalVolume())
			vol.DetachLocalVolume();
	}

	return derr;
}

// ProcessQueryVolInfo processes a kAD_QueryVolInfo request.

DualErr 
CPGPdiskDrv::ProcessQueryVolInfo(PAD_QueryVolInfo pQVI,	PGPUInt32 size)
{
	DualErr derr;

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

	// Perform the query.
	if (derr.IsntError())
	{
		Volume vol;

		pgpAssertAddrValid(pQVI->pBlockSize, PGPUInt16);
		pgpAssertAddrValid(pQVI->pTotalBlocks, PGPUInt16);

		derr = vol.AttachLocalVolume(pQVI->drive);

		if (derr.IsntError())
		{
			(* pQVI->pBlockSize) = vol.GetBlockSize();
			(* pQVI->pTotalBlocks) = vol.GetTotalBlocks();
		}

		if (vol.AttachedToLocalVolume())
			vol.DetachLocalVolume();
	}

	return derr;
}

// ProcessNotifyUserLogoff processes a kAD_NotifyUserLogoff request.

DualErr
CPGPdiskDrv::ProcessNotifyUserLogoff(
	PAD_NotifyUserLogoff	pNUL, 
	PGPUInt32				size)
{
	DualErr derr;

	// Begin attempts every second to unmount all PGPdisks.
	mUnmountAllMode = TRUE;

	return derr;
}

// ProcessADPacket is called to process an AD (application to driver) packet.

PGPUInt32 
CPGPdiskDrv::ProcessADPacket(PADPacketHeader pPacket, PGPUInt32 size)
{
	DualErr derr;

	// Validate the packet.
	if (IsNull(pPacket))
	{
		return ERROR_INVALID_PARAMETER;
	}
	if (size < sizeof(ADPacketHeader))
	{
		return ERROR_INSUFFICIENT_BUFFER;
	}
	else if (pPacket->magic != kPGPdiskADPacketMagic)
	{
		return ERROR_INVALID_PARAMETER;
	}

	// Was there an error during driver initialization?
	if (derr.IsntError())
	{
		derr = Driver->mInitErr;
	}

	// Process the request.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(pPacket, ADPacketHeader);
		DebugOut("PGPdisk: %s received\n", GetADPacketName(pPacket->code));

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

	(* pPacket->pDerr) = derr;

	return ERROR_SUCCESS;
}


///////////////////////
// Dispatcher functions
///////////////////////

// OnW32DeviceIoControl is a member function of PGPdiskDevice that has been
// overridden to handle DeviceIoControl calls. These calls are made by the
// application in order to pass messages and/or packets on to the driver.

DWORD 
CPGPdiskDrv::OnW32DeviceIoControl(PIOCTLPARAMS p)
{
	PGPUInt32 result;

	switch (p->dioc_IOCtlCode)
	{
	case DIOC_OPEN:						// called when handle is opened
		result = DEVIOCTL_NOERROR;
		break;

	case DIOC_CLOSEHANDLE:				// called when handle is closed
		result = DEVIOCTL_NOERROR;
		break;

	case IOCTL_PGPDISK_SENDPACKET:		// called to send packet to driver
		result = ProcessADPacket((PADPacketHeader) p->dioc_InBuf, 
			p->dioc_cbInBuf);
		break;

	default:
		result = ERROR_INVALID_PARAMETER;
		break;
	}

	return result;
}
