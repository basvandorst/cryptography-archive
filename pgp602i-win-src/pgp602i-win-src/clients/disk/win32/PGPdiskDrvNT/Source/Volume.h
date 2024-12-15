//////////////////////////////////////////////////////////////////////////////
// Volume.h
//
// Declaration of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.h,v 1.1.2.15 1998/07/06 08:58:00 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Volume_h	// [
#define Included_Volume_h

#include <ntdddisk.h>
#include "DualErr.h"

#include "CPGPdiskDriver.h"
#include "FSCTLCodes.h"


///////////////
// Class Volume
///////////////

// Class Volume can be used to represent any local volume on the system. It
// can also be used to mount and unmount new volumes.

class Volume
{
public:
					Volume();
	virtual			~Volume();

	PGPBoolean		Mounted();
	PGPBoolean		Unmounted();
	PGPBoolean		AttachedToLocalVolume();
	PGPBoolean		HasOpenFiles();

	PGPBoolean		LockedForReadWrite();
	PGPBoolean		LockedForFormat();

	PGPUInt8		GetDrive();
	PGPUInt16		GetBlockSize();
	PGPUInt64		GetTotalBlocks();
	PDEVICE_OBJECT	GetDeviceObject();

	DualErr			AttachLocalVolume(PGPUInt8 drive);
	void			DetachLocalVolume();

	DualErr			Mount(LPCSTR deviceName, PGPUInt8 drive = kInvalidDrive, 
						PGPBoolean mountReadOnly = FALSE);
	DualErr			Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr			LockVolumeForReadWrite();
	DualErr			LockVolumeForFormat();
	DualErr			UnlockVolume();

	DualErr			Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks);
	DualErr			Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks);

protected:
	PDEVICE_OBJECT	mDeviceObject;	// device object for volume
	PGPUInt32		mDevExtInfo;	// value to put in device extension

private:
	enum MountState	{kVol_Mounted, kVol_Unmounted};
	enum LockState	{kLock_None, kLock_ReadWrite, kLock_Format};

	MountState		mMountState;	// holds mount status of volume
	LockState		mLockState;		// lock status of the volume
	PGPUInt8		mDrive;			// mounted drive number

	PGPBoolean		mAttachedToLocalVol;	// attached to local volume?

	PDEVICE_OBJECT	mVHDeviceObject;	// device object for handle
	PFILE_OBJECT	mVHFileObject;		// file object for handle
	HANDLE			mVolumeHandle;		// handle to volume

	KUstring		mDeviceName;	// device name
	KUstring		mLinkName;		// link name

	DISK_GEOMETRY	mGeometry;		// geometry of drive

	PGPBoolean		VolumeHandleOpened();
	DualErr			GetDriveGeometry();

	DualErr			OpenVolumeHandle();
	void			CloseVolumeHandle();

	DualErr			SendIOCTLRequest(PGPUInt8 majorFunc, PGPUInt8 minorFunc, 
						PGPUInt32 ioctlCode, PVOID inBuf = NULL, 
						PGPUInt32 sizeInBuf = 0, PVOID outBuf = NULL, 
						PGPUInt32 sizeOutBuf = 0);

	DualErr			SendUserFSCTLRequest(PGPUInt32 fsctlCode);
};

#endif	// ] Included_Volume_h
