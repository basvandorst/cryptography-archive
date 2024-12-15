//////////////////////////////////////////////////////////////////////////////
// DriverComm.h
//
// Declarations for DriverComm.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: DriverComm.h,v 1.1.2.14 1998/08/04 02:10:06 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_DriverComm_h	// [
#define Included_DriverComm_h

#include "DualErr.h"
#include "Packets.h"


/////////////////////
// Exported functions
/////////////////////

PGPBoolean	IsDriverOpen();
DualErr		OpenPGPdiskDriver();
DualErr		ClosePGPdiskDriver();

DualErr		CheckDriverVersion();
DualErr		NotifyUserLogoff();

DualErr		SetDriverPrefs(PGPBoolean autoUnmount, 
				PGPUInt32 autoUnmountTimeout);

DualErr		GetPGPdiskInfo(PGPdiskInfo *pPDIArray, PGPUInt32 elemsArray);
DualErr		IsFileAPGPdisk(LPCSTR path, PGPBoolean *isFileAPGPdisk);
DualErr		IsVolumeAPGPdisk(PGPUInt8 drive, PGPBoolean *isVolumeAPGPdisk);

DualErr		SendMountRequest(PAD_Mount pMNT);
DualErr		SendUnmountRequest(PAD_Unmount pUNMNT);

DualErr		AreFilesOpenOnDrive(PGPUInt8 drive, PGPBoolean *areFilesOpen);

DualErr		LockUnlockVolume(PGPUInt8 drive, LockOp lockOp);
DualErr		DirectDiskRead(PGPUInt8 drive, PGPUInt8 *buf, PGPUInt32 bufSize, 
				PGPUInt64 pos, PGPUInt32 nBlocks);
DualErr		DirectDiskWrite(PGPUInt8 drive, PGPUInt8 *buf, PGPUInt32 bufSize, 
				PGPUInt64 pos, PGPUInt32 nBlocks);
DualErr		QueryVolInfo(PGPUInt8 drive, PGPUInt16 *pBlockSize, 
				PGPUInt64 *pTotalBlocks);

DualErr		AllocLockedMem(PGPUInt32 nBytes, void **pPMem);
DualErr		FreeLockedMem(void *pMem, PGPUInt32 nBytes);

#endif	// ] Included_DriverComm_h
