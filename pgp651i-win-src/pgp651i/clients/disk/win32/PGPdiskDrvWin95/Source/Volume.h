//////////////////////////////////////////////////////////////////////////////
// Volume.h
//
// Declaration of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.h,v 1.4 1998/12/14 19:00:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Volume_h	// [
#define Included_Volume_h

#include "DualErr.h"
#include "FatUtils.h"
#include "GenericCallback.h"


////////////
// Constants
////////////

const PGPUInt32	kDefaultIopTimeOut	= 15;			// IOP time out in secs
const PGPUInt32	kPGPdiskIopMagic	= 0xAB42F61D;	// IOP came from us


////////
// Types
////////

#define IOR_event _ureq._IOR_requestor_usage[0]

// VolumeRequestInfo holds info for a specific request.

typedef struct VolumeRequestInfo
{
	PGPBoolean isInUse;		// this object in use?

	GenericCallbackInfo	downInfo;	// info for call down
	GenericCallbackInfo	*upInfo;	// info for call up

	PIOR pIor;				// IOR

} VolumeRequestInfo;


///////////////
// Class Volume
///////////////

// Class Volume can be used to represent any local volume on the system. If
// the volume is not already mounted, it cannot be mounted until a DCB is
// attached to it (DCBs are allocated for the driver by the system upon
// startup). It does no DCB initialization itself; that is for the client
// or for derived classes to do.

class Volume
{
public:
				Volume();
	virtual		~Volume();

	PGPBoolean	Mounted();
	PGPBoolean	Unmounted();
	PGPBoolean	AttachedToLocalVolume();
	PGPBoolean	HasOpenFiles();

	PGPBoolean	LockedForReadWrite();
	PGPBoolean	LockedForFormat();

	PGPUInt8	GetDrive();
	PGPUInt16	GetBlockSize();
	PGPUInt64	GetTotalBlocks();
	PDCB		GetDcb();

	DualErr		GetDevParams16(DevParams16 *dp16);
	DualErr		GetDevParams32(DevParams32 *dp32);
	DualErr		GetMediaId(MID *pMid);
	DualErr		GetFsId(FileSysId *fsId);

	DualErr		AttachLocalVolume(PGPUInt8 drive);
	void		DetachLocalVolume();

	DualErr		Mount(PDCB pDcb, PGPUInt8 drive = kInvalidDrive);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr		LockVolumeForReadWrite();
	DualErr		LockVolumeForFormat();
	DualErr		UnlockVolume();

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks, 
					GenericCallbackInfo *upInfo = NULL);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks, 
					GenericCallbackInfo *upInfo = NULL);

protected:
	PDCB mPDcb;		// pointer to the DCB for the volume

private:
	enum MountState	{kVol_Mounted, kVol_Unmounted};
	enum LockState	{kLock_None, kLock_ReadWrite, kLock_Format};

	MountState			mMountState;	// holds mount status of volume
	LockState			mLockState;		// lock status of the volume
	PGPUInt8			mDrive;			// mounted drive number

	PGPBoolean			mAttachedToLocalVol;	// attached to a local volume?
	PGPUInt8			*mBBlock;		// boot block buffer for DevParams

	VolumeRequestInfo	mVolumeReq;		// request info

	DualErr					InitMountedVars(PGPUInt8 drive, PDCB pDcb);
	void					CleanUpMountedVars();

	DualErr					DiskAccess(PGPUInt16 func, PGPUInt8 *buf, 
								PGPUInt64 pos, PGPUInt32 nBlocks, 
								GenericCallbackInfo *upInfo = NULL);

	DualErr					CleanUpRequest();

	static void				VolumeCallback(GenericCallbackInfo *downInfo);
	void					VolumeCallbackAux();

	void					ScheduleAsyncCallback(
								DualErr derr = DualErr::NoError);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_Volume_h
