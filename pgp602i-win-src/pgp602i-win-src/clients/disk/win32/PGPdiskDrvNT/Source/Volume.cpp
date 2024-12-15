//////////////////////////////////////////////////////////////////////////////
// Volume.cpp
//
// Implementation of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.cpp,v 1.1.2.28.2.1 1998/10/21 07:12:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDriver.h"
#include "Globals.h"
#include "KernelModeUtils.h"
#include "PGPdiskRequestQueue.h"
#include "Volume.h"


///////////////////////////////////////
// Class Volume public member functions
///////////////////////////////////////

// The Class Volume constructor.

Volume::Volume()
{
	mDevExtInfo = (PGPUInt32) this;

	mMountState	= kVol_Unmounted;
	mLockState	= kLock_None;
	mDrive		= kInvalidDrive;

	mAttachedToLocalVol = FALSE;

	mDeviceObject = NULL;

	mVHDeviceObject	= NULL;
	mVHFileObject	= NULL;
	mVolumeHandle	= NULL;
}

// The Volume destructor unmounted the volume if was mounted by us.

Volume::~Volume()
{
	DualErr derr;

	if (Mounted())
	{
		if (AttachedToLocalVolume())
		{
			DetachLocalVolume();
		}
		else
		{
			derr = Unmount();
			pgpAssert(derr.IsntError());
		}
	}
}

// Mounted returns TRUE if the volume is mounted, FALSE if not.

PGPBoolean 
Volume::Mounted()
{
	return (mMountState == kVol_Mounted);
}

// Unmounted returns TRUE if the volume is unmounted, FALSE if not.

PGPBoolean 
Volume::Unmounted()
{
	return (mMountState == kVol_Unmounted);
}

// HasOpenFiles returns TRUE if the volume has open files, FALSE otherwise.

PGPBoolean
Volume::HasOpenFiles()
{
	DualErr derr;

	pgpAssert(Mounted());

	// If lock fails, then the volume has open files.
	derr = LockVolumeForReadWrite();

	if (LockedForReadWrite())
		UnlockVolume();

	return derr.IsError();
}

// LockedForReadWrite returns TRUE if the volume is mounted and locked for
// read/write access, FALSE otherwise.

PGPBoolean 
Volume::LockedForReadWrite()
{
	return (Mounted() && (mLockState == kLock_ReadWrite));
}

// LockedForReadWrite returns TRUE if the volume is mounted and locked for
// format access, FALSE otherwise.

PGPBoolean 
Volume::LockedForFormat()
{
	return (Mounted() && (mLockState == kLock_Format));
}

// AttachedToLocalVolume returns TRUE if the Volume object is attached to a
// local volume, FALSE if not.

PGPBoolean 
Volume::AttachedToLocalVolume()
{
	return mAttachedToLocalVol;
}

// GetDrive returns the drive number of the volume.

PGPUInt8 
Volume::GetDrive()
{
	pgpAssert(Mounted());

	return mDrive;
}

// GetBlockSize returns the block size of the volume.

PGPUInt16 
Volume::GetBlockSize()
{
	DualErr derr;

	pgpAssert(Mounted());

	derr = GetDriveGeometry();

	if (derr.IsntError())
		return (PGPUInt16) mGeometry.BytesPerSector;
	else
		return kDefaultBlockSize;
}

// GetTotalBlocks returns the total number of blocks on the volume.

PGPUInt64 
Volume::GetTotalBlocks()
{
	DualErr derr;

	pgpAssert(Mounted());

	derr = GetDriveGeometry();

	if (derr.IsntError())
	{
		return (mGeometry.Cylinders.QuadPart * mGeometry.TracksPerCylinder * 
			mGeometry.SectorsPerTrack);
	}
	else
	{
		return 0;
	}
}

// GetDeviceObject returns the device object associated with the volume.

PDEVICE_OBJECT 
Volume::GetDeviceObject()
{
	pgpAssert(Mounted());

	return mDeviceObject;
}

// AttachLocalVolume initializes this Volume object for access to an already
// mounted volume on the local computer.

DualErr 
Volume::AttachLocalVolume(PGPUInt8 drive)
{
	DualErr derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(!Mounted());
	pgpAssert(!AttachedToLocalVolume());

	if (LockedForReadWrite() || LockedForFormat())
		UnlockVolume();

	mMountState	= kVol_Mounted;
	mLockState	= kLock_None;
	mDrive		= drive;

	mAttachedToLocalVol = TRUE;

	return derr;
}

// DetachLocalVolume marks this Volume object as no longer being associated
// with a local volume.

void 
Volume::DetachLocalVolume()
{
	pgpAssert(Mounted());
	pgpAssert(AttachedToLocalVolume());

	if (VolumeHandleOpened())
		CloseVolumeHandle();

	mMountState	= kVol_Unmounted;
	mLockState	= kLock_None;
	mDrive		= kInvalidDrive;

	mAttachedToLocalVol = FALSE;
}

// Mount creates a device and links it to the specified drive letter.

DualErr 
Volume::Mount(LPCSTR deviceName, PGPUInt8 drive, PGPBoolean mountReadOnly)
{
	DualErr		derr;
	KUstring	uniDeviceName, drivePath;
	NTSTATUS	status;
	PGPBoolean	createdDevice, foundDrive;
	PGPUInt8	i;

	pgpAssertStrValid(deviceName);
	pgpAssert(Unmounted());
	pgpAssert(!AttachedToLocalVolume());

	createdDevice = foundDrive = FALSE;

	// If preferred drive letter is specified, is it available?
	if (IsLegalDriveNumber(drive))
	{
		derr = MakePathToDrive(drive, &drivePath);

		if (derr.IsntError())
		{
			foundDrive = !IsValidDeviceName(drivePath);
		}
	}

	// Find a drive letter we can use.
	if (derr.IsntError() && !foundDrive)
	{
		for (i = 2; i < kMaxDrives; i++)
		{
			derr = MakePathToDrive(i, &drivePath);

			if (derr.IsntError())
			{
				if (!IsValidDeviceName(drivePath))
				{
					foundDrive = TRUE;
					drive = i;
					break;
				}
			}
			
			if (derr.IsError())
				break;
		}
	}

	// Die if no free drive found.
	if (derr.IsntError())
	{
		if (!foundDrive)
			derr = DualErr(kPGDMinorError_NoDriveLettersFree);
	}

	// Prepare device name.
	if (derr.IsntError())
	{
		derr = AssignToUni(&uniDeviceName, deviceName);
	}

	// Prepend device prefix.
	if (derr.IsntError())
	{
		derr = PrependToUni(&uniDeviceName, kNTDevicePathPrefix);
	}

	// Append driver letter.
	if (derr.IsntError())
	{
		char driveLet[2];

		driveLet[0] = DriveNumToLet(drive);
		driveLet[1] = '\0';

		derr = AppendToUni(&uniDeviceName, driveLet);
	}

	// Create a new device object.
	if (derr.IsntError())
	{
		PGPUInt32 attribs = FILE_VIRTUAL_VOLUME;

		if (mountReadOnly)
			attribs |= FILE_READ_ONLY_DEVICE;

		status = IoCreateDevice(Interface->mPGPdiskDriver->DriverObject(), 
			0, uniDeviceName, FILE_DEVICE_DISK, attribs, FALSE, 
			&mDeviceObject);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoCreateDeviceFailed, status);

		createdDevice = derr.IsntError();
	}

	// Assign the link name.
	if (derr.IsntError())
	{
		derr = AssignToUni(&mLinkName, drivePath);
	}

	// Create the link.
	if (derr.IsntError())
	{
		mDeviceObject->Flags |= DO_DIRECT_IO;
		mDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

		mDeviceObject->DeviceExtension = (void *) mDevExtInfo;

		status = IoCreateSymbolicLink(mLinkName, uniDeviceName);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoCreateSymbolicLinkFailed, status);
	}

	// Initialize variables on success.
	if (derr.IsntError())
	{
		mMountState	= kVol_Mounted;
		mLockState	= kLock_None;
		mDrive		= drive;

		mAttachedToLocalVol = FALSE;
	}

	// Cleanup on error.
	if (derr.IsError())
	{
		if (createdDevice)
		{
			IoDeleteDevice(mDeviceObject);
			mDeviceObject = NULL;
		}
	}

	return derr;
}

// Unmount unmounts a mounted Volume.

DualErr 
Volume::Unmount(PGPBoolean isThisEmergency)
{
	DualErr		derr;
	NTSTATUS	status;

	pgpAssert(Mounted());
	pgpAssert(!AttachedToLocalVolume());

	if (LockedForReadWrite() || LockedForFormat())
		UnlockVolume();

	// Lock the volume.
	derr = LockVolumeForReadWrite();

	// Dismount the volume.
	if (derr.IsntError())
	{
		derr = SendUserFSCTLRequest(FSCTL_DISMOUNT_VOLUME);
	}

	if (isThisEmergency)
		derr = DualErr::NoError;

	// Delete the symbolic link.
	if (derr.IsntError())
	{
		status = IoDeleteSymbolicLink(mLinkName);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoDeleteSymbolicLinkFailed, status);

		if (isThisEmergency)
			derr = DualErr::NoError;
	}

	// Delete the device object and finish up.
	if (derr.IsntError())
	{
		if (!isThisEmergency || (isThisEmergency && LockedForReadWrite()))
			UnlockVolume();

		mDeviceObject->DeviceExtension = NULL;

		IoDeleteDevice(mDeviceObject);
		mDeviceObject = NULL;

		mMountState	= kVol_Unmounted;
		mLockState	= kLock_None;
		mDrive		= kInvalidDrive;
	}

	if (derr.IsError())
	{
		if (LockedForReadWrite())
			UnlockVolume();
	}

	return derr;
}

// LockVolumeForReadWrite locks the mounted volume for direct read/write
// access.

DualErr 
Volume::LockVolumeForReadWrite()
{
	DualErr derr;
	
	pgpAssert(Mounted());
	pgpAssert(!LockedForReadWrite() && !LockedForFormat());

	derr = OpenVolumeHandle();
	
	if (derr.IsntError())
	{
		derr = SendUserFSCTLRequest(FSCTL_LOCK_VOLUME);
	}

	if (derr.IsntError())
	{
		mLockState = kLock_ReadWrite;
	}

	if (derr.IsError())
	{
		if (VolumeHandleOpened())
			CloseVolumeHandle();
	}

	return derr;
}

// LockVolumeForFormat locks the mounted volume for formatting.

DualErr 
Volume::LockVolumeForFormat()
{
	DualErr derr;
	
	pgpAssert(Mounted());
	pgpAssert(!LockedForReadWrite() && !LockedForFormat());

	derr = OpenVolumeHandle();

	if (derr.IsntError())
	{
		derr = SendUserFSCTLRequest(FSCTL_LOCK_VOLUME);
	}

	if (derr.IsntError())
	{
		mLockState = kLock_Format;
	}

	if (derr.IsError())
	{
		if (VolumeHandleOpened())
			CloseVolumeHandle();
	}

	return derr;
}

// UnlockVolume removes any outstanding locks on the volume;

DualErr 
Volume::UnlockVolume()
{
	DualErr derr;

	pgpAssert(Mounted());
	pgpAssert(LockedForReadWrite() || LockedForFormat());

	derr = SendUserFSCTLRequest(FSCTL_UNLOCK_VOLUME);
	pgpAssert(derr.IsntError());

	if (derr.IsntError())
	{
		mLockState = kLock_None;
		CloseVolumeHandle();
	}

	return derr;
}

// Read reads 'nBlocks' sectors from the logical mounted volume from sector
// position 'pos'.

DualErr 
Volume::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus; 
	KEVENT			event; 
	KIrp			pIrp;
	LARGE_INTEGER	liPos;
	NTSTATUS		status;
	PGPBoolean		openedVolHandle	= FALSE;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	// Open handle to the volume.
	if (!VolumeHandleOpened())
	{
		derr = OpenVolumeHandle();
		openedVolHandle = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Initialize the event.
		KeInitializeEvent(&event, NotificationEvent, FALSE);

		// Build the request.
		liPos.QuadPart = pos * GetBlockSize();

		pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, mVHDeviceObject, 
			buf, nBlocks * GetBlockSize(), &liPos, &event, &ioStatus);

		if (IsNull((PIRP) pIrp))
			derr = DualErr(kPGDMinorError_IoBuildSynchFsdRequestFailed);
	}

	// Send the request.
	if (derr.IsntError())
	{
		pIrp.NextStackLocation()->FileObject = mVHFileObject;

		status = IoCallDriver(mVHDeviceObject, pIrp);
 
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = ioStatus.Status;
		}

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoCallDriverFailed, status);
	}

	// Close volume handle.
	if (openedVolHandle)
		CloseVolumeHandle();

	return derr;
}

// Write reads 'nBlocks' sectors from the logical mounted volume from sector
// position 'pos'.

DualErr 
Volume::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus; 
	KEVENT			event; 
	KIrp			pIrp;
	LARGE_INTEGER	liPos;
	NTSTATUS		status;
	PGPBoolean		openedVolHandle	= FALSE;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	// Open handle to the volume.
	if (!VolumeHandleOpened())
	{
		derr = OpenVolumeHandle();
		openedVolHandle = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Initialize the event.
		KeInitializeEvent(&event, NotificationEvent, FALSE);

		// Build the request.
		liPos.QuadPart = pos * GetBlockSize();

		pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, mVHDeviceObject, 
			buf, nBlocks * GetBlockSize(), &liPos, &event, &ioStatus);

		if (IsNull((PIRP) pIrp))
			derr = DualErr(kPGDMinorError_IoBuildIOCTLRequestFailed);
	}

	// Send the request.
	if (derr.IsntError())
	{
		pIrp.NextStackLocation()->FileObject = mVHFileObject;

		status = IoCallDriver(mVHDeviceObject, pIrp);
 
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = ioStatus.Status;
		}

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoCallDriverFailed, status);
	}

	// Close volume handle.
	if (openedVolHandle)
		CloseVolumeHandle();

	return derr;
}


////////////////////////////////////////
// Class Volume private member functions
////////////////////////////////////////

// VolumeHandleOpened returns TRUE if a handle to the volume has been opened,
// FALSE otherwise.

PGPBoolean 
Volume::VolumeHandleOpened()
{
	return (IsntNull(mVolumeHandle));
}

// GetDriveGeometry gets the geometry of the specified drive.

DualErr
Volume::GetDriveGeometry()
{
	DualErr		derr;
	PGPBoolean	openedVolHandle;

	pgpAssert(Mounted());

	if (!VolumeHandleOpened())
	{
		derr = OpenVolumeHandle();
		openedVolHandle = derr.IsntError();
	}

	// Send down the request.
	if (derr.IsntError())
	{
		derr = SendIOCTLRequest(IRP_MJ_DEVICE_CONTROL, 0, 
			IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &mGeometry, 
			sizeof(mGeometry));
	}

	if (openedVolHandle)
		CloseVolumeHandle();

	return derr;
}

// OpenVolumeHandle opens a handle to the actual volume and references the
// associated file object.

DualErr	
Volume::OpenVolumeHandle()
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus;
	KUstring		drivePath;
	NTSTATUS		status;
	PGPBoolean		openedVol, reffedFileObj;

	openedVol = reffedFileObj = FALSE;

	pgpAssert(!VolumeHandleOpened());

	// Get path to volume.
	derr = MakePathToDrive(mDrive, &drivePath);

	// Get handle to volume;
	if (derr.IsntError())
	{
		OBJECT_ATTRIBUTES objectAttributes;

		InitializeObjectAttributes(&objectAttributes, drivePath, 
			OBJ_CASE_INSENSITIVE, NULL, NULL);

		status = ZwCreateFile(&mVolumeHandle, 
			SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE, 
			&objectAttributes, &ioStatus, NULL, 0, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, 
			FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_ZwCreateFileFailed, status);

		openedVol = derr.IsntError();
	}

	// Get file object from this handle.
	if (derr.IsntError())
	{
		status = ObReferenceObjectByHandle(mVolumeHandle, FILE_READ_DATA, 
			NULL, KernelMode, (void **) &mVHFileObject, NULL);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_ObRefObjByHandleFailed, status);

		reffedFileObj = derr.IsntError();
	}

	// Initialize variables on success.
	if (derr.IsntError())
	{
		mVHDeviceObject = IoGetRelatedDeviceObject(mVHFileObject);
	}

	// Cleaup on error.
	if (derr.IsError())
	{
		if (reffedFileObj)
		{
			ObDereferenceObject(mVHFileObject);
			mVHFileObject = NULL;
			mVHDeviceObject = NULL;
		}

		if (openedVol)
		{
			ZwClose(mVolumeHandle);
			mVolumeHandle = NULL;
		}
	}

	return derr;
}

// CloseVolumeHandle closes a handle opened with OpenVolumeHandle.

void 
Volume::CloseVolumeHandle()
{
	pgpAssert(VolumeHandleOpened());

	// Dereference file object pointer taken before.
	ObDereferenceObject(mVHFileObject);
	mVHFileObject = NULL;
	mVHDeviceObject = NULL;

	// Close volume handle.
	ZwClose(mVolumeHandle);

	mVolumeHandle = NULL;
}

// SendIOCTLRequest sends an IOCTL request to the mounted volume.

DualErr 
Volume::SendIOCTLRequest(
	PGPUInt8	majorFunc, 
	PGPUInt8	minorFunc, 
	PGPUInt32	ioctlCode, 
	PVOID		inBuf, 
	PGPUInt32	sizeInBuf, 
	PVOID		outBuf, 
	PGPUInt32	sizeOutBuf)
{
	DualErr				derr;
	KEVENT				event;
	IO_STATUS_BLOCK		ioStatus;
	KIrp				pIrp;
	NTSTATUS			status;

	pgpAssert(Mounted());
	pgpAssert(VolumeHandleOpened());

	// Initialize the event.
	KeInitializeEvent(&event, NotificationEvent, FALSE);

	// Build the request.
	pIrp = IoBuildDeviceIoControlRequest(ioctlCode, mVHDeviceObject, inBuf, 
		sizeInBuf, outBuf, sizeOutBuf, FALSE, &event, &ioStatus);

	if (IsNull((PIRP) pIrp))
		derr = DualErr(kPGDMinorError_IoBuildIOCTLRequestFailed);

	// Call down the request.
	if (derr.IsntError())
	{
		pIrp.MajorFunction(NEXT) = majorFunc;
		pIrp.MinorFunction(NEXT) = minorFunc;

		pIrp.NextStackLocation()->FileObject = mVHFileObject;
		pIrp.IoctlCode(NEXT) = ioctlCode;

		status = IoCallDriver(mVHDeviceObject, pIrp);

		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
			status = ioStatus.Status;
		}

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_IoCallDriverFailed, status);
	}

	return derr;
}

// SendUserFSCTLRequest is a wrapper around SendIOCTLRequest.

DualErr 
Volume::SendUserFSCTLRequest(PGPUInt32 fsctlCode)
{
	pgpAssert(Mounted());
	pgpAssert(VolumeHandleOpened());

	return SendIOCTLRequest(IRP_MJ_FILE_SYSTEM_CONTROL, 
		IRP_MN_USER_FS_REQUEST, fsctlCode);
}
