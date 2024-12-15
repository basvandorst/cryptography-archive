//////////////////////////////////////////////////////////////////////////////
// DriverComm.cpp
//
// Driver communication functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: DriverComm.cpp,v 1.1.2.17 1998/08/04 02:10:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#else
#error Define PGPDISK_MFC.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "DriverComm.h"
#include "PGPdiskVersion.h"
#include "UtilityFunctions.h"
#include "WindowsVersion.h"


//////////
// Globals
//////////

HANDLE DriverHandle = INVALID_HANDLE_VALUE;


//////////////////////
// Low-level functions
//////////////////////

// IsDriverOpen returns TRUE if the PGPdisk driver is open, FALSE otherwise.

PGPBoolean 
IsDriverOpen()
{
	return (DriverHandle != INVALID_HANDLE_VALUE);
}

// OpenPGPdiskDriver opens a handle to the PGPdisk driver.

DualErr 
OpenPGPdiskDriver()
{
	DualErr derr;

	if (!IsDriverOpen())
	{
		DriverHandle = CreateFile(kPGPdiskDriverName, 
			GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (DriverHandle == INVALID_HANDLE_VALUE)
			derr = DualErr(kPGDMinorError_DriverNotInstalled);
	}

	return derr;
}

// ClosePGPdiskDriver closes a handle to the PGPdisk driver.

DualErr 
ClosePGPdiskDriver()
{
	DualErr derr;

	if (IsDriverOpen())
	{
		if (!::CloseHandle(DriverHandle))
			derr = DualErr(kPGDMinorError_CloseHandleFailed);
	}

	if (derr.IsntError())
	{
		DriverHandle = INVALID_HANDLE_VALUE;
	}

	return derr;
}

// SendPacket sends a packet to the driver using DeviceIoControl.

DualErr 
SendPacket(PADPacketHeader pPacket, PGPUInt16 code, PGPUInt32 packetSize)
{
	DualErr			derr;
	unsigned long	nBytesReturned;

	pgpAssertAddrValid(pPacket, ADPacketHeader);
	pgpAssert(IsDriverOpen());

	// Initialize the packet header.
	pPacket->magic	= kPGPdiskADPacketMagic;
	pPacket->code	= code;
	pPacket->pDerr	= &derr;

	// Send the packet.
	if (!(DeviceIoControl(DriverHandle, IOCTL_PGPDISK_SENDPACKET, pPacket, 
		packetSize, NULL, NULL, &nBytesReturned, NULL)))
	{
		derr = DualErr(kPGDMinorError_DriverCommFailure, GetLastError());
	}

	return derr;
}


////////////////////////////////////
// Version and preferences functions
////////////////////////////////////

// CheckDriverVersion makes sure the correct version of the driver is running.

DualErr 
CheckDriverVersion()
{
	AD_QueryVersion	QV;
	DualErr			derr;
	PGPUInt32		driverVersion;

	pgpAssert(IsDriverOpen());

	// Fill in and send the version query packet.
	QV.appVersion = kPGPdiskAppVersion;
	QV.pDriverVersion = &driverVersion;

	derr = SendPacket((PADPacketHeader) &QV, kAD_QueryVersion, sizeof(QV));

	// Check for version incompatiblity.
	if (derr.IsntError())
	{
		if (driverVersion != kCompatDriverVersion)
			derr = DualErr(kPGDMinorError_DriverIsIncompatVersion);
	}

	return derr;
}

// NotifyUserLogoff tells the driver the current user is logging off. It will
// then continually attempt to unmount all PGPdisks.

DualErr 
NotifyUserLogoff()
{
	AD_NotifyUserLogoff	NUL;
	DualErr				derr;

	// Send the packet.
	derr = SendPacket((PADPacketHeader) &NUL, kAD_NotifyUserLogoff, 
		sizeof(NUL));

	return derr;
}

// SetDriverPrefs informs the driver of the state of the application's
// preferences.

DualErr 
SetDriverPrefs(PGPBoolean autoUnmount, PGPUInt32 autoUnmountTimeout)
{
	AD_ChangePrefs	CP;
	DualErr			derr;

	CP.autoUnmount		= autoUnmount;
	CP.unmountTimeout	= autoUnmountTimeout;

	// Send the packet.
	derr = SendPacket((PADPacketHeader) &CP, kAD_ChangePrefs, sizeof(CP));

	return derr;
}


/////////////////////////////
// PGPdisk-specific functions
/////////////////////////////

// GetPGPdiskInfo asks the driver for info on all mounted PGPdisks.

DualErr 
GetPGPdiskInfo(PGPdiskInfo *pPDIArray, PGPUInt32 elemsArray)
{
	AD_GetPGPdiskInfo	GPI;
	DualErr				derr;

	pgpAssertAddrValid(pPDIArray, PGPdiskInfo);
	pgpAssert(IsDriverOpen());

	// Fill in and send the GetPGPdiskInfo packet.
	GPI.arrayElems	= elemsArray;
	GPI.pPDIArray	= pPDIArray;

	derr = SendPacket((PADPacketHeader) &GPI, kAD_GetPGPdiskInfo, 
		sizeof(GPI));

	return derr;
}


// IsFileAPGPdisk asks the driver if the specified file is a mounted PGPdisk.

DualErr 
IsFileAPGPdisk(LPCSTR path, PGPBoolean *isFileAPGPdisk)
{
	DualErr			derr;
	AD_QueryMounted	QM;

	pgpAssertAddrValid(isFileAPGPdisk, PGPBoolean);
	pgpAssertStrValid(path);

	QM.trueIfUsePath	= TRUE;
	QM.path				= path;
	QM.sizePath			= strlen(path) + 1;
	QM.pIsPGPdisk		= isFileAPGPdisk;

	derr = SendPacket((PADPacketHeader) &QM, kAD_QueryMounted, sizeof(QM));

	return derr;
}

// IsVolumeAPGPdisk asks the driver if the specified volume is a PGPdisk.

DualErr 
IsVolumeAPGPdisk(PGPUInt8 drive, PGPBoolean *isVolumeAPGPdisk)
{
	DualErr			derr;
	AD_QueryMounted	QM;

	pgpAssertAddrValid(isVolumeAPGPdisk, PGPBoolean);
	pgpAssert(IsLegalDriveNumber(drive));

	QM.trueIfUsePath	= FALSE;
	QM.drive			= drive;
	QM.pIsPGPdisk		= isVolumeAPGPdisk;

	derr = SendPacket((PADPacketHeader) &QM, kAD_QueryMounted, sizeof(QM));

	return derr;
}

// SendMountRequest sends an already filled in mount packet to the driver.

DualErr 
SendMountRequest(PAD_Mount pMNT)
{
	DualErr derr;

	pgpAssertAddrValid(pMNT, AD_Mount);
	derr = SendPacket((PADPacketHeader) pMNT, kAD_Mount, sizeof(AD_Mount));

	return derr;
}

// SendUnmountRequest sends an already filled in unmount packet to the
// driver.

DualErr 
SendUnmountRequest(PAD_Unmount pUNMNT)
{
	DualErr derr;

	pgpAssertAddrValid(pUNMNT, AD_Unmount);

	derr = SendPacket((PADPacketHeader) pUNMNT, kAD_Unmount, 
		sizeof(AD_Unmount));

	return derr;
}


/////////////////////////////
// Volume/directory functions
/////////////////////////////

// AreFilesOpenOnDrive returns TRUE if the given drive has open file handles, 
// FALSE otherwise. It needs to ask the driver to get this information.

DualErr 
AreFilesOpenOnDrive(PGPUInt8 drive, PGPBoolean *areFilesOpen)
{
	AD_QueryOpenFiles	QOF;
	DualErr				derr;

	pgpAssertAddrValid(areFilesOpen, PGPBoolean);
	pgpAssert(IsLegalDriveNumber(drive));

	// Fill it in and send the packet to the driver.
	QOF.drive			= drive;
	QOF.pHasOpenFiles	= areFilesOpen;

	derr = SendPacket((PADPacketHeader) &QOF, kAD_QueryOpenFiles, 
		sizeof(QOF));

	return derr;
}

// LockUnlockVolume locks or unlocks a mounted volume.

DualErr 
LockUnlockVolume(PGPUInt8 drive, LockOp lockOp)
{
	AD_LockUnlockVol	LUV;
	DualErr				derr;

	pgpAssert(IsLegalDriveNumber(drive));

	// Prepare the lock request.
	LUV.drive	= drive;
	LUV.lockOp	= lockOp;

	// Send the request to the driver.
	derr = SendPacket((PADPacketHeader) &LUV, kAD_LockUnlockVol, sizeof(LUV));

	return derr;
}

// DirectDiskRead reads blocks from a locked mounted volume.

DualErr 
DirectDiskRead(
	PGPUInt8	drive, 
	PGPUInt8	*buf, 
	PGPUInt32	bufSize, 
	PGPUInt64	pos, 
	PGPUInt32	nBlocks)
{
	AD_ReadWriteVol	RWV;
	DualErr			derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssertAddrValid(buf, PGPUInt8);

	// Prepare the read request.
	RWV.trueIfRead	= TRUE;
	RWV.drive		= drive;
	RWV.buf			= buf;
	RWV.bufSize		= bufSize;
	RWV.pos			= pos;
	RWV.nBlocks		= nBlocks;

	// Send the request to the driver.
	derr = SendPacket((PADPacketHeader) &RWV, kAD_ReadWriteVol, sizeof(RWV));

	return derr;
}

// DirectDiskWrite writes blocks to a locked mounted volume.

DualErr 
DirectDiskWrite(
	PGPUInt8	drive, 
	PGPUInt8	*buf, 
	PGPUInt32	bufSize, 
	PGPUInt64	pos, 
	PGPUInt32	nBlocks)
{
	AD_ReadWriteVol	RWV;
	DualErr			derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssertAddrValid(buf, PGPUInt8);

	// Prepare the read request.
	RWV.trueIfRead	= FALSE;
	RWV.drive		= drive;
	RWV.buf			= buf;
	RWV.bufSize		= bufSize;
	RWV.pos			= pos;
	RWV.nBlocks		= nBlocks;

	// Send the request to the driver.
	derr = SendPacket((PADPacketHeader) &RWV, kAD_ReadWriteVol, sizeof(RWV));

	return derr;
}

// QueryVolInfo gets some extra volume information from the driver.

DualErr 
QueryVolInfo(PGPUInt8 drive, PGPUInt16 *pBlockSize, PGPUInt64 *pTotalBlocks)
{
	AD_QueryVolInfo	QVI;
	DualErr			derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssertAddrValid(pBlockSize, PGPUInt16);
	pgpAssertAddrValid(pTotalBlocks, PGPUInt64);

	// Prepare the request.
	QVI.drive			= drive;
	QVI.pBlockSize		= pBlockSize;
	QVI.pTotalBlocks	= pTotalBlocks;

	// Send the request to the driver.
	derr = SendPacket((PADPacketHeader) &QVI, kAD_QueryVolInfo, sizeof(QVI));

	return derr;
}


///////////////////
// Memory functions
///////////////////

// AllocLockedMem allocates and returns a block of locked memory of the
// specified size (rounded up to the nearest page boundary).

DualErr 
AllocLockedMem(PGPUInt32 nBytes, void **pPMem)
{
	DualErr		derr;
	PGPBoolean	allocedMem	= FALSE;
	void		*pLockedMem;

	pgpAssertAddrValid(pPMem, VoidAlign);

	// Allocate the memory.
	pLockedMem = VirtualAlloc(NULL, nBytes, MEM_COMMIT, PAGE_READWRITE);

	if (IsNull(pLockedMem))
		derr = DualErr(kPGDMinorError_OutOfMemory);

	allocedMem = derr.IsntError();

	// Ask the driver to lock it.
	if (derr.IsntError())
	{
		AD_LockUnlockMem LUM;

		LUM.pMem		= pLockedMem;
		LUM.nBytes		= nBytes;
		LUM.trueForLock	= TRUE;

		derr = SendPacket((PADPacketHeader) &LUM, kAD_LockUnlockMem, 
			sizeof(LUM));
	}

	if (derr.IsntError())
	{
		(* pPMem) = pLockedMem;
	}

	if (derr.IsError())
	{
		if (allocedMem)
			VirtualFree(pLockedMem, 0, MEM_RELEASE);
	}

	return derr;
}

// FreeLockedMem frees a block of locked memory previously allocated by
// AllocLockedMem.

DualErr 
FreeLockedMem(void *pMem, PGPUInt32 nBytes)
{
	AD_LockUnlockMem	LUM;
	DualErr				derr;

	pgpAssertAddrValid(pMem, VoidAlign);

	// Ask the driver to unlock the memory.
	LUM.pMem		= pMem;
	LUM.nBytes		= nBytes;
	LUM.trueForLock	= FALSE;

	derr = SendPacket((PADPacketHeader) &LUM, kAD_LockUnlockMem, sizeof(LUM));

	// Free the memory.
	if (derr.IsntError())
	{
		if (!::VirtualFree(pMem, 0, MEM_RELEASE))
			derr = DualErr(kPGDMinorError_VirtualFreeFailed);
	}

	return derr;
}
