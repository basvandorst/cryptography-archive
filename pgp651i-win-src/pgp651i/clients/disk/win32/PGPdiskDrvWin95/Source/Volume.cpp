//////////////////////////////////////////////////////////////////////////////
// Volume.cpp
//
// Implementation of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.cpp,v 1.5 1999/02/13 04:24:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrv.h"
#include "Globals.h"
#include "Volume.h"


///////////////////////////////////////
// Class Volume public member functions
///////////////////////////////////////

// The Class Volume constructor.

Volume::Volume()
{
	mMountState	= kVol_Unmounted;
	mLockState	= kLock_None;
	mDrive		= kInvalidDrive;

	mAttachedToLocalVol	= FALSE;
	mBBlock				= NULL; 

	mPDcb = NULL;

	mVolumeReq.isInUse = FALSE;
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
	DualErr		derr;
	PGPBoolean	hasOpenFiles;

	pgpAssert(Mounted());

	derr = Driver->HasOpenFiles(mDrive, &hasOpenFiles);

	if (derr.IsntError())
		return hasOpenFiles;
	else
		return TRUE;
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
	pgpAssert(Mounted());
	pgpAssertAddrValid(mPDcb, DCB);

	return ExtractBlockSize(mPDcb);
}

// GetTotalBlocks returns the total number of blocks on the volume.

PGPUInt64 
Volume::GetTotalBlocks()
{
	pgpAssert(Mounted());
	pgpAssertAddrValid(mPDcb, DCB);

	return MakeQWord(mPDcb->DCB_actual_sector_cnt[0], 
		mPDcb->DCB_actual_sector_cnt[0]);
}

// GetDcb returns a pointer to the DCB.

PDCB 
Volume::GetDcb()
{
	pgpAssert(Mounted());
	pgpAssertAddrValid(mPDcb, DCB);

	return mPDcb;
}

// GetDevParams16 will fill out the elements of DevParams16 structure for the
// mounted Volume by reading directly from the boot block.

DualErr 
Volume::GetDevParams16(DevParams16 *dp16)
{
	DualErr			derr;
	PBootSector16	pBS16;

	pgpAssertAddrValid(dp16, DevParams16);
	pgpAssertAddrValid(mBBlock, PGPUInt8);
	pgpAssert(Mounted());

	derr = Read(mBBlock, 0, 1);
	pBS16 = (PBootSector16) mBBlock;

	if (derr.IsntError())
	{
		pgpClearMemory(dp16, sizeof(DevParams16));

		dp16->dpDevType		= 0x05;				// we are a hard disk
		dp16->dpDevAttr		= 1;				// we are not removable
		dp16->dpCylinders	= (PGPUInt16) mPDcb->DCB_actual_cyl_cnt;
		dp16->dpMediaType	= pBS16->bsMedia;

		dp16->dpBytesPerSec	= pBS16->bsBytesPerSec;
		dp16->dpSecPerClust	= pBS16->bsSecPerClust;
		dp16->dpResSectors	= pBS16->bsResSectors;
		dp16->dpFats		= pBS16->bsFats;
		dp16->dpRootDirEnts	= pBS16->bsRootDirEnts;
		dp16->dpSectors		= pBS16->bsSectors;
		dp16->dpMedia		= pBS16->bsMedia;
		dp16->dpFatSecs		= pBS16->bsFatSecs;
		dp16->dpSecPerTrack	= pBS16->bsSecPerTrack;
		dp16->dpHeads		= pBS16->bsHeads;
		dp16->dpHiddenSecs	= pBS16->bsHiddenSecs;
		dp16->dpHugeSectors	= pBS16->bsHugeSectors;
	}

	return derr;
}

// GetDevParams32 will fill out the elements of DevParams32 structure for the
// mounted Volume by reading directly from the boot block.

DualErr 
Volume::GetDevParams32(DevParams32 *dp32)
{
	DualErr			derr;
	PBootSector32	pBS32;

	pgpAssert(Mounted());
	pgpAssertAddrValid(dp32, DevParams32);
	pgpAssertAddrValid(mBBlock, PGPUInt8);

	derr = Read(mBBlock, 0, 1);

	pBS32 = (PBootSector32) mBBlock;

	if (derr.IsntError())
	{
		pgpClearMemory(dp32, sizeof(DevParams32));

		dp32->dpDevType		= 0x05;			// we are a hard disk
		dp32->dpDevAttr		= 1;			// we are not removable
		dp32->dpCylinders	= (PGPUInt16) mPDcb->DCB_actual_cyl_cnt;
		dp32->dpMediaType	= pBS32->bsMedia;

		dp32->dpBytesPerSector		= pBS32->bsBytesPerSec;
		dp32->dpSectorsPerClust		= pBS32->bsSecPerClust;
		dp32->dpReservedSectors		= pBS32->bsResSectors;
		dp32->dpNumberOfFats		= pBS32->bsFats;
		dp32->dpRootEntries			= pBS32->bsRootDirEnts;
		dp32->dpTotalSectors		= pBS32->bsSectors;
		dp32->dpMediaDescriptor		= pBS32->bsMedia;
		dp32->dpSectorsPerFat		= pBS32->bsFatSecs;
		dp32->dpSectorsPerTrack		= pBS32->bsSecPerTrack;
		dp32->dpHeads				= pBS32->bsHeads;
		dp32->dpHiddenSectors		= pBS32->bsHiddenSecs;
		dp32->dpBigTotalSectors		= pBS32->bsHugeSectors;
		dp32->dpBigSectorsPerFat	= pBS32->bsBigSectorsPerFat;
		dp32->dpExtFlags			= pBS32->bsExtFlags;
		dp32->dpFS_Version			= pBS32->bsFS_Version;
		dp32->dpRootDirStrtClus		= pBS32->bsRootDirStrtClus;
		dp32->dpFsInfoSec			= pBS32->bsFsInfoSec;
		dp32->dpBkUpBootSec			= pBS32->bsBkUpBootSec;
	}

	return derr;
}

// GetMediaId fills in a MID structure with information about the volume.

DualErr 
Volume::GetMediaId(MID *pMid)
{
	DualErr		derr;
	PGPBoolean	foundRecognizedFileSys	= FALSE;

	pgpAssertAddrValid(pMid, MID);

	pgpAssert(Mounted());
	pgpAssertAddrValid(mBBlock, PGPUInt8);

	derr = Read(mBBlock, 0, 1);

	if (derr.IsntError())
	{
		LPSTR fsStr;

		pMid->midInfoLevel = 0;

		fsStr = (LPSTR) mBBlock + kFat12Offset;
		if (!strncmp(fsStr, "FAT12", 5))			// is it a FAT12 drive?
		{
			PBootSector12 pBS12 = (PBootSector12) mBBlock;

			foundRecognizedFileSys = TRUE;
			pMid->midSerialNum = pBS12->bsVolumeId;

			strncpy((char *) pMid->midVolLabel, pBS12->bsVolumeLabel, 11);
			strncpy((char *) pMid->midFileSysType, pBS12->bsFileSysType, 8);
		}

		fsStr = (LPSTR) mBBlock + kFat16Offset;
		if (!strncmp(fsStr, "FAT16", 5))			// is it a FAT16 drive?
		{
			PBootSector16 pBS16 = (PBootSector16) mBBlock;

			foundRecognizedFileSys = TRUE;
			pMid->midSerialNum = pBS16->bsVolumeId;

			strncpy((char *) pMid->midVolLabel, pBS16->bsVolumeLabel, 11);
			strncpy((char *) pMid->midFileSysType, pBS16->bsFileSysType, 8);
		}

		fsStr = (LPSTR) mBBlock + kFat32Offset;
		if (!strncmp(fsStr, "FAT32", 5))			// is it a FAT32 drive?
		{
			PBootSector32 pBS32 = (PBootSector32) mBBlock;

			foundRecognizedFileSys = TRUE;
			pMid->midSerialNum = pBS32->bsVolumeId;

			strncpy((char *) pMid->midVolLabel, pBS32->bsVolumeLabel, 11);
			strncpy((char *) pMid->midFileSysType, pBS32->bsFileSysType, 8);
		}

		// Special case for incorrectly formatted floppy drives. If the
		// filesystem is unknown but the media is a floppy, assume FAT12.

		if (!foundRecognizedFileSys)
		{
			PBootSector12 pBS12 = (PBootSector12) mBBlock;

			if (pBS12->bsMedia == kFloppyMediaId)
			{
				pMid->midSerialNum = pBS12->bsVolumeId;

				strncpy((char *) pMid->midVolLabel, pBS12->bsVolumeLabel, 11);
				strncpy((char *) pMid->midFileSysType, "FAT12   ", 8);
			}
		}
	}

	return derr;
}

// GetFsId returns the ID of the filesystem of the mounted volume by reading
// the volume's bootsector and then seeing if it can find an identifier
// describing a known filesystem.

DualErr 
Volume::GetFsId(FileSysId *fsId)
{
	DualErr	derr;
	MID		mid;

	pgpAssertAddrValid(fsId, FileSysId);

	pgpAssert(Mounted());
	pgpAssertAddrValid(mBBlock, PGPUInt8);

	(* fsId) = kFS_Unknown;

	derr = GetMediaId(&mid);

	if (derr.IsntError())
	{
		LPSTR fsStr = (char *) mid.midFileSysType;

		if (!strncmp(fsStr, "FAT12", 5))		// is it a FAT12 drive?
			(* fsId) = kFS_FAT12;

		if (!strncmp(fsStr, "FAT16", 5))		// is it a FAT16 drive?
			(* fsId) = kFS_FAT16;

		if (!strncmp(fsStr, "FAT32", 5))		// is it a FAT32 drive?
			(* fsId) = kFS_FAT32;
	}

	return derr;
}

// AttachLocalVolume initializes this Volume object for access to an already
// mounted volume on the local computer.

DualErr 
Volume::AttachLocalVolume(PGPUInt8 drive)
{
	DualErr	derr;
	PDCB	pDcb;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(Unmounted());
	pgpAssert(!AttachedToLocalVolume());

	// Find the DCB by making a system call.
	if (!(pDcb = IspGetDcb(drive)))
		derr = DualErr(kPGDMinorError_IspGetDcbFailed);

	if (derr.IsntError())
	{
		derr = InitMountedVars(drive, pDcb);
		mAttachedToLocalVol = derr.IsntError();
	}

	return derr;
}

// DetachLocalVolume marks this Volume object as no longer being associated
// with a local volume.

void 
Volume::DetachLocalVolume()
{
	pgpAssert(Mounted());
	pgpAssert(AttachedToLocalVolume());

	mAttachedToLocalVol = FALSE;
	CleanUpMountedVars();
}

// Mount() mounts the volume on the specified drive letter using the
// specified DCB.

DualErr 
Volume::Mount(PDCB pDcb, PGPUInt8 drive)
{
	DualErr		derr;
	PGPBoolean	assocedDcb	= FALSE;

	pgpAssertAddrValid(pDcb, DCB);
	pgpAssert(Unmounted());
	pgpAssert(!AttachedToLocalVolume());

	// If no preferred drive letter is specified, or if the drive is in use, 
	// we make a system call to get the next free drive letter.

	if ((drive < 2) || (drive >= kMaxDrives) || (IsntNull(IspGetDcb(drive))))
	{
		drive = pDcb->DCB_cmn.DCB_drive_lttr_equiv = 
			IspDriveLetterPick(mPDcb, NULL);

		if (drive == kInvalidDrive)
			derr = DualErr(kPGDMinorError_IspDriveLetterPickFailed);
	}	

	// Then we make another call to associate this driver letter with our
	// DCB, thereby mounting the volume.

	if (derr.IsntError())
	{
		if (!IspAssociateDcb(pDcb, drive, NULL))
			derr = DualErr(kPGDMinorError_IspAssociateDcbFailed);

		assocedDcb = derr.IsntError();
	}

	// Initialize some variables for mounted drives.
	if (derr.IsntError())
	{
		derr = InitMountedVars(drive, pDcb);
	}

	// Clean up in case of error.
	if (derr.IsError())
	{
		if (assocedDcb)
			IspDisassociateDcb(drive);
	}

	return derr;
}

// Unmount unmounts a mounted Volume.

DualErr 
Volume::Unmount(PGPBoolean isThisEmergency)
{
	DualErr		derr;
	LockLevel	oldLock;
	PGPBoolean	hasOpenFiles, lockedDown;

	pgpAssert(Mounted());
	pgpAssert(!AttachedToLocalVolume());

	hasOpenFiles = lockedDown = FALSE;

	if (!isThisEmergency)
	{
		// Check for open files.
		derr = Driver->HasOpenFiles(mDrive, &hasOpenFiles);

		if (derr.IsntError())
		{
			if (hasOpenFiles)
				derr = DualErr(kPGDMinorError_FilesOpenOnDrive);
		}

		if (derr.IsntError())
		{
			// Flush the volume.
			VolFlush(mDrive, NULL);

			// Get a lock on the drive.
			oldLock = Driver->GetLockLevel(mDrive);
			derr = Driver->AcquireLogicalVolLock(mDrive, kLock_L3);

			lockedDown = derr.IsntError();
		}
	}

	if (derr.IsntError())
	{
		// Some people tell me this should be done to prevent hangs. (?)
		NotifyVolumeRemoval(GetDrive());

		// We make a system call to disassociate the mounted DCB with its
		// drive letter, thereby unmounting the volume.

		if (!IspDisassociateDcb(GetDrive()))
			derr = DualErr(kPGDMinorError_IspDisassocDcbFailed);
	}

	if (derr.IsntError())
	{
		CleanUpMountedVars();
	}

	if (derr.IsError())
	{
		if (lockedDown)
			Driver->AcquireLogicalVolLock(mDrive, oldLock);
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

	derr = Driver->AcquireLogicalVolLock(mDrive, kLock_L3);

	if (derr.IsntError())
	{
		mLockState = kLock_ReadWrite;
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

	derr = Driver->GetFormatLockOnDrive(mDrive);

	if (derr.IsntError())
	{
		mLockState = kLock_Format;
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

	if (LockedForReadWrite())
		derr = Driver->AcquireLogicalVolLock(mDrive, ::kLock_None);
	else
		derr = Driver->ReleaseFormatLockOnDrive(mDrive);

	if (derr.IsntError())
	{
		mLockState = kLock_None;
	}

	return derr;
}

// Read reads 'nBlocks' sectors from the logical mounted volume from sector
// position 'pos'.

DualErr 
Volume::Read(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBlocks, 
	GenericCallbackInfo	*upInfo)
{
	pgpAssertAddrValid(buf, PGPUInt8);

	return DiskAccess(IOR_READ, buf, pos, nBlocks, upInfo);
}

// Write reads 'nBlocks' sectors from the logical mounted volume from sector
// position 'pos'.

DualErr 
Volume::Write(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBlocks, 
	GenericCallbackInfo	*upInfo)
{
	pgpAssertAddrValid(buf, PGPUInt8);

	return DiskAccess(IOR_WRITE, buf, pos, nBlocks, upInfo);
}


////////////////////////////////////////
// Class Volume private member functions
////////////////////////////////////////

// InitMountedVars initializes some variables that describe a mounted volume
// and/or can be used to perform I/O on that volume.

DualErr 
Volume::InitMountedVars(PGPUInt8 drive, PDCB pDcb)
{
	DualErr derr;

	pgpAssert(IsLegalDriveNumber(drive));
	pgpAssert(Unmounted());
	pgpAssertAddrValid(pDcb, DCB);

	// Allocate a buffer for bootblock and devparam requests.
	if (derr.IsntError())
	{
		derr = GetByteBuffer(ExtractBlockSize(pDcb), (PGPUInt8 **) &mBBlock);
	}

	if (derr.IsntError())
	{
		mMountState = kVol_Mounted;
		mDrive = drive;

		mPDcb = pDcb;
	}

	return derr;
}

// CleanUpMountedVars cleans up those variables we initialized in
// InitMountedVars.

void 
Volume::CleanUpMountedVars()
{
	pgpAssert(Mounted());
	pgpAssertAddrValid(mBBlock, PGPUInt8);

	mMountState = kVol_Unmounted;

	FreeByteBuffer(mBBlock);

	mDrive = kInvalidDrive;

	mBBlock = NULL;
	mPDcb = NULL;
}

// DiskAccess performs either an IOR_READ or an IOR_WRITE operation on the
// mounted volume.

DualErr 
Volume::DiskAccess(
	PGPUInt16			func, 
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBlocks, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PIOP		pIop;
	PIOR		pIor;
	PGPBoolean	useAsync;
	PGPUInt16	offset, size;
	PVRP		pVrp;

	pgpAssertAddrValid(buf, PGPUInt8);

	pgpAssert((func == IOR_READ) || (func == IOR_WRITE));

	// Is this request asynchronous?
	useAsync = IsntNull(upInfo);

	pgpAssert(Mounted());
	pgpAssertAddrValid(mPDcb, DCB);
	pgpAssert(!mVolumeReq.isInUse);

	// Prepare the request.
	mVolumeReq.isInUse = TRUE;
	
	if (useAsync)
	{
		mVolumeReq.upInfo = upInfo;
		mVolumeReq.downInfo.refData[0] = (PGPUInt32) this;
	}

	// Make sure we check that the VRP field in the DCB exists; the system
	// has a habit of nullifying it when an I/O error occurs.

	if (!(pVrp = (PVRP) mPDcb->DCB_cmn.DCB_vrp_ptr))
		derr = DualErr(kPGDMinorError_InvalidVRP);

	// Create our IOP/IOR.
	if (derr.IsntError())
	{
		size = (PGPUInt16) pVrp->VRP_max_req_size + 
			pVrp->VRP_max_sgd*sizeof(SGD);
		offset = (PGPUInt16) pVrp->VRP_delta_to_ior;

		pIop = IspCreateIop(size, offset, ISP_M_FL_INTERRUPT_TIME);

		if (!pIop)
			derr = DualErr(kPGDMinorError_IorAllocationFailed);
	}

	// Now we fill in the fields of our IOR structure prior to sending the
	// I/O request to the IOS subsystem.

	if (derr.IsntError())
	{
		pIor = &pIop->IOP_ior;
		pIop->IOP_timer_orig = pIop->IOP_timer = kDefaultIopTimeOut;

		mVolumeReq.pIor = pIor;

		pIor->IOR_next		= NULL;					// must be NULL
		pIor->IOR_func		= func;					// IOR_READ or IOR_WRITE
		pIor->IOR_status	= 0;					// result code goes here
		pIor->IOR_flags		= IORF_VERSION_002		// means we are using IOR
							| IORF_HIGH_PRIORITY			// high priority
							| IORF_LOGICAL_START_SECTOR		// in partition
							| IORF_DOUBLE_BUFFER;			// copy buffer

		// We have to set the start address, transfer count, and buffer fields
		// with the passed information.

		pIor->IOR_start_addr[0]		= GetLowDWord(pos);		// low sector
		pIor->IOR_start_addr[1]		= GetHighDWord(pos);	// high sector
		pIor->IOR_xfer_count		= nBlocks;				// # secs
		pIor->IOR_buffer_ptr		= (PGPUInt32) buf;		// buffer
		pIor->IOR_private_client	= kPGPdiskIopMagic;		// comes from us
		pIor->IOR_req_vol_handle	= (PGPUInt32) pVrp;		// vrp pointer
		pIor->IOR_sgd_lin_phys		= (PGPUInt32) (pIor + 1);	// SGDs here
		pIor->IOR_num_sgds			= 0;					// system fills in
		pIor->IOR_vol_designtr		= GetDrive();

		// Initialize the callback info.
		if (useAsync)
		{
			pIor->IOR_callback = (CMDCPLT) VolumeCallback;
			pIor->IOR_req_req_handle = (PGPUInt32) &mVolumeReq.downInfo;
		}
		else
		{
			pIor->IOR_flags |= IORF_SYNC_COMMAND;
			pIor->IOR_callback = NULL;
		}

		// The IlbIoCriteria routine must be run on the IOR in order to let
		// the system massage some of the fields in the IOR into a form that
		// it likes.

		if (!IlbIoCriteria(pIor))
			DebugOut("PGPdisk: IlbIoCriteria in DiskAccess failed");

		DebugOut(
			"PGPdisk: Vol Access IOR %X func %u pos %u nBlocks %u drive %u", 
			pIor, func, (PGPUInt32) pos, nBlocks, GetDrive());

		// Send the request to the IOS for processing synchronously.
		__asm push edi
		IOS_SendCommand(pIor, mPDcb);
		__asm pop edi

		DebugOut("PGPdisk: Exiting Vol Access IOR %X", pIor);
	}

	// If we are sync and called down the IOP we need to cleanup now
	// since there is no callback.

	if (derr.IsntError())
	{
		if (!useAsync)
		{
			derr = CleanUpRequest();	// extract derr from IOR
			mVolumeReq.isInUse = FALSE;
		}
	}

	// If an error calling down the IOP, we need to callback if we were
	// called async.

	if (derr.IsError())
	{
		if (useAsync)
		{
			CleanUpRequest();
			ScheduleAsyncCallback(derr);
		}
	}

	return derr;
}

// CleanUpRequest performs housekeeping after a request has been completed.

DualErr 
Volume::CleanUpRequest()
{
	DualErr	derr;
	PIOR	pIor;

	pgpAssert(mVolumeReq.isInUse);
	pIor = mVolumeReq.pIor;

	// Convert possible error value to Windows standard.
	if (pIor->IOR_status >= IORS_ERROR_DESIGNTR)
	{
		if (pIor->IOR_func == IOR_READ)
		{
			derr = DualErr(kPGDMinorError_IorReadFailure, 
				IOSMapIORSToI21(pIor->IOR_status));
		}
		else
		{
			derr = DualErr(kPGDMinorError_IorWriteFailure, 
				IOSMapIORSToI21(pIor->IOR_status));
		}
	}

	// De-allocate the IOR if one was allocated.
	if (pIor)
	{
		PGPUInt16	offset;
		PVRP		pVrp;

		pVrp = (PVRP) mPDcb->DCB_cmn.DCB_vrp_ptr;
		
		// Hopefully our volume didn't go away.
		if (pVrp)
		{
			pgpAssertAddrValid(pVrp, VRP);

			offset = (PGPUInt16) pVrp->VRP_delta_to_ior;
			IspDeallocMem((PGPUInt8 *) pIor - offset);
		}
	}

	return derr;
}

// VolumeCallback is called by the system after it has finished processing
// the IOP we gave it. We extract the address of the Volume object in question
// and pass the callback to it.

void 
Volume::VolumeCallback(GenericCallbackInfo *downInfo)
{
	Volume *pVol;

	pgpAssertAddrValid(downInfo, GenericCallbackInfo);

	pVol = (Volume *) downInfo->refData[0];
	pgpAssertAddrValid(pVol, Volume);

	pVol->VolumeCallbackAux();
}

// VolumeCallbackAux is called by the static callback function
// 'VolumeCallback' so we don't have to type 'pVol' before every reference to
// an object member or method. (Not needed here but coded so we are parallel
// to higher classes such as FatParser and PGPdisk in this regard.)

void 
Volume::VolumeCallbackAux()
{
	DualErr derr;

	pgpAssert(mVolumeReq.isInUse);

	// Cleanup.
	derr = CleanUpRequest();

	// Call up.
	ScheduleAsyncCallback(derr);
}

// ScheduleAsyncCallback schedules a windows event that calls our function
// that will call the asynchronous request up.

void 
Volume::ScheduleAsyncCallback(DualErr derr)
{
	static RestrictedEvent_THUNK callbackThunk;

	pgpAssertAddrValid(mVolumeReq.upInfo, GenericCallbackInfo);
	mVolumeReq.upInfo->derr = derr;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) this, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID __stdcall 
Volume::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	Volume *pVol;

	pVol = (Volume *) Refdata;
	pgpAssertAddrValid(pVol, Volume);

	pgpAssert(pVol->mVolumeReq.isInUse);
	pVol->mVolumeReq.isInUse = FALSE;

	pgpAssertAddrValid(pVol->mVolumeReq.upInfo, GenericCallbackInfo);
	pVol->mVolumeReq.upInfo->callback(pVol->mVolumeReq.upInfo);
}
