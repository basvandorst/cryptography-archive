//////////////////////////////////////////////////////////////////////////////
// Volume.cpp
//
// Implementation of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.cpp,v 1.5 1998/12/14 18:58:55 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <Dbt.h>
#include <Shlobj.h>

#include "Required.h"
#include "DriverComm.h"
#include "UtilityFunctions.h"
#include "WindowsVersion.h"

#include "Globals.h"
#include "Volume.h"


////////////
// Constants
////////////

const PGPUInt32	kSHNotifySleepDelayMs = 500;
const PGPUInt64	kMaxBlocksDiskWeFormat = 
	(2047*kBytesPerMeg)/kDefaultBlockSize;

static LPCSTR	kVWin32DriverString	= "\\\\.\\VWIN32";
static LPCSTR	kVolumeOpenString	= "\\\\.\\%c:";

const PGPUInt32	kMaxBlocksPerWrite = 2000;
const PGPUInt32	kMinBlocksPerWrite = 10;

const PGPUInt32	kIdealNumWritesPerData = 10;


///////////////////////////////////////
// Class Volume public member functions
///////////////////////////////////////

// The Class Volume constructor.

Volume::Volume(PGPUInt8 drive)
{
	if (drive != kInvalidDrive)
		mMountState = kVol_Mounted;
	else
		mMountState = kVol_Unmounted;

	mLockState = kLock_None;
	mDrive = drive;

	mAttachedToLocalVol	= FALSE;

	mBlockSize = 0;
}

// The Volume destructor unmounted the volume if was mounted by us.

Volume::~Volume()
{
	DualErr derr;

	if (Mounted())
	{
		if (AttachedToLocalVolume())
			DetachLocalVolume();
	}
}

// GetDrive returns the drive number the volume is mounted on.

PGPUInt8 
Volume::GetDrive()
{
	return mDrive;
}

// GetBlockSize returns the block size of the mounted volume.

PGPUInt16 
Volume::GetBlockSize()
{
	pgpAssert(Mounted());

	return mBlockSize;
}

// GetTotalBlocks returns the total number of blocks on the volume.

PGPUInt64 
Volume::GetTotalBlocks()
{
	pgpAssert(Mounted());

	return mTotalBlocks;
}

// Mounted returns TRUE if the Volume is mounted.

PGPBoolean 
Volume::Mounted()
{
	return (mMountState == kVol_Mounted);
}

// Unmounted returns TRUE if the Volume is unmounted.

PGPBoolean 
Volume::Unmounted()
{
	return (mMountState == kVol_Unmounted);
}

// AttachedToLocalVolume returns TRUE if the Volume object is attached to a
// local volume, FALSE if not.

PGPBoolean 
Volume::AttachedToLocalVolume()
{
	return mAttachedToLocalVol;
}

// HasOpenFiles returns TRUE if the volume has open files, FALSE otherwise.

PGPBoolean
Volume::HasOpenFiles()
{
	DualErr		derr;
	PGPBoolean	hasOpenFiles;

	pgpAssert(Mounted());

	derr = AreFilesOpenOnDrive(mDrive, &hasOpenFiles);

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

// BrowseToVolume opens a view to the volume in an explorer window.

void 
Volume::BrowseToVolume()
{
	CString root;

	pgpAssert(Mounted());

	if (MakeRoot(mDrive, &root).IsntError())
	{
		ShellExecute(NULL, "explore", root, NULL, NULL, SW_SHOWNORMAL);
	}
}

// GetVolumeLabel retrieves the volume label associated with a mounted volume.

DualErr 
Volume::GetVolumeLabel(LPSTR label, PGPUInt32 size)
{
	DualErr	derr;
	CString root;

	pgpAssertAddrValid(label, LPSTR);
	pgpAssert(Mounted());

	derr = MakeRoot(mDrive, &root);

	if (derr.IsntError())
	{
		if (GetVolumeInformation(root, label, size, NULL, NULL, NULL, NULL, 
			0))
		{
			derr = DualErr(kPGDMinorError_GetVolumeInfoFailed, 
				GetLastError());
		}
	}

	return derr;
}

// SetVolumeLabel sets the 11-character label of the mounted volume.

DualErr 
Volume::SetVolumeLabel(LPCSTR label)
{
	DualErr	derr;
	CString	volName, root;

	pgpAssertStrValid(label);
	pgpAssert(Mounted());

	derr = CanonicalizeVolumeName(label, &volName);

	if (derr.IsntError())
	{
		derr = MakeRoot(mDrive, &root);
	}

	if (derr.IsntError())
	{
		if (!::SetVolumeLabel(root, volName))
		{
			derr = DualErr(kPGDMinorError_SetVolumeLabelFailed, 
				GetLastError());
		}
	}

	return derr;
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

	if (!App->IsVolumeValid(drive))
		derr = DualErr(kPGDMinorError_InvalidParameter);

	if (derr.IsntError())
	{
		mMountState	= kVol_Mounted;
		mLockState	= kLock_None;
		mDrive		= drive;

		mAttachedToLocalVol = TRUE;

		if (FillInVolInfo().IsError())
		{
			mBlockSize = kDefaultBlockSize;
			mTotalBlocks = 0;
		}
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

	mMountState	= kVol_Unmounted;
	mLockState	= kLock_None;
	mDrive		= kInvalidDrive;

	mAttachedToLocalVol = FALSE;
}

// LockVolumeForReadWrite locks the mounted volume for direct read/write
// access.

DualErr 
Volume::LockVolumeForReadWrite()
{
	DualErr derr;
	
	pgpAssert(Mounted());
	pgpAssert(!LockedForReadWrite() && !LockedForFormat());

	derr = LockUnlockVolume(mDrive, kLO_LockReadWrite);

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

	derr = LockUnlockVolume(mDrive, kLO_LockFormat);

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

	derr = LockUnlockVolume(mDrive, (LockedForReadWrite() ? 
		kLO_UnlockReadWrite : kLO_UnlockFormat));

	if (derr.IsntError())
	{
		mLockState = kLock_None;
	}

	return derr;
}

// Read reads blocks from the locked mounted volume.

DualErr 
Volume::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());
	pgpAssert(LockedForReadWrite() || LockedForFormat());

	derr = DirectDiskRead(mDrive, buf, nBlocks * GetBlockSize(), pos, 
		nBlocks);

	return derr;
}

// Write writes blocks to the locked mounted volume.

DualErr 
Volume::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());
	pgpAssert(LockedForReadWrite() || LockedForFormat());

	derr = DirectDiskWrite(mDrive, buf, nBlocks * GetBlockSize(), pos, 
		nBlocks);

	return derr;
}

// Format formats the mounted volume using our own algorithms.

DualErr 
Volume::Format()
{
	DualErr		derr;
	FatData		fat;
	PGPBoolean	allocedBlockBuf, lockedForFormat;
	PGPUInt8	*blockBuf;
	PGPUInt64	megsDisk;

	pgpAssert(Mounted());
	pgpAssert(!LockedForReadWrite() || !LockedForFormat());

	allocedBlockBuf = lockedForFormat = FALSE;
	megsDisk = (GetTotalBlocks() * kDefaultBlockSize) / kBytesPerMeg;

	// Can only format drives with standard block sizes.
	if (GetBlockSize() != kDefaultBlockSize)
		derr = DualErr(kPGDMinorError_CantFormatDrive);

	// Too big for format?
	if (derr.IsntError())
	{
		if (GetBlockSize() > kMaxBlocksDiskWeFormat)
			derr = DualErr(kPGDMinorError_DiskTooBigToFormat);
	}

	// Get block buffer.
	if (derr.IsntError())
	{
		derr = GetByteBuffer(kDefaultBlockSize, &blockBuf);
		allocedBlockBuf = derr.IsntError();
	}

	// Lock the volume for format.
	if (derr.IsntError())
	{
		derr = LockVolumeForFormat();
		lockedForFormat = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Initialize FAT data.
		fat.fdFsId = kFS_FAT16;

		if (megsDisk < 2)
		{
			fat.fdFsId = kFS_FAT12;
		}
		else if ((megsDisk >= kMinFat32Megs) && 
			IsWin95OSR2CompatibleMachine())
		{
			fat.fdFsId = kFS_FAT32;
		}

		InitFatData(&fat, GetBlockSize());

		derr = ClearBlocks(0, fat.fdFirstSecData);
	}

	// Write out the FAT data structures.
	if (derr.IsntError())
	{
		BigFatBootFSInfo	bfInfo;
		BootSector12		bb12;
		BootSector16		bb16;
		BootSector32		bb32;
		PGPUInt32			fat16Sig;
		PGPUInt64			pos;

		pgpAssert(sizeof(bb12) == kDefaultBlockSize);
		pgpAssert(sizeof(bb16) == kDefaultBlockSize);
		pgpAssert(sizeof(bb32) == kDefaultBlockSize);

		pgpClearMemory(blockBuf, kDefaultBlockSize);

		pos = 0;

		switch (fat.fdFsId)
		{

		case kFS_FAT12:
			// Init the boot block.
			InitFAT12BootBlock(GetBlockSize(), &fat, &bb12);

			// Write the boot block.
			derr = Write((PGPUInt8 *) &bb12, pos, 1);

			// Write the first FAT.
			if (derr.IsntError())
			{
				pgpCopyMemory((PGPUInt8 *) &kFat12Sig, blockBuf, 
					sizeof(kFat12Sig));

				pos += fat.fdReservedSecs;
				derr = Write(blockBuf, pos, 1);
			}

			// Write the second FAT.
			if (derr.IsntError())
			{
				pos += fat.fdFatSize;
				derr = Write(blockBuf, pos, 1);
			}
			break;

		case kFS_FAT16:
			// Init the boot block.
			InitFAT16BootBlock(GetBlockSize(), &fat, &bb16);

			// Decide on a FAT signature.
			fat16Sig = (megsDisk < 16 ? kUnder16MbFat16Sig : 
				kOver16MbFat16Sig);

			// Write the boot block.
			derr = Write((PGPUInt8 *) &bb16, pos, 1);

			// Write the first FAT.
			if (derr.IsntError())
			{
				pgpCopyMemory((PGPUInt8 *) &fat16Sig, blockBuf, 
					sizeof(fat16Sig));

				pos += fat.fdReservedSecs;
				derr = Write(blockBuf, pos, 1);
			}

			// Write the second FAT.
			if (derr.IsntError())
			{
				pos += fat.fdFatSize;
				derr = Write(blockBuf, pos, 1);
			}
			break;

		case kFS_FAT32:
			// Init the boot block.
			InitFAT32BootBlock(GetBlockSize(), &fat, &bb32, &bfInfo);

			// Write the boot block.
			derr = Write((PGPUInt8 *) &bb32, pos, 1);

			// Write the BigFatBootInfo structure.
			if (derr.IsntError())
			{
				pgpCopyMemory((PGPUInt8 *) &bfInfo, blockBuf, 
					sizeof(bfInfo));

				pos += bb32.bsFsInfoSec;
				derr = Write(blockBuf, pos, 1);
			}

			if (derr.IsntError())
			{
				PGPUInt32 threeClusts[3];

				threeClusts[0] = kFat32Clust1;
				threeClusts[1] = kFat32Clust2;
				threeClusts[2] = kFat32Clust3;

				pgpClearMemory(blockBuf, kDefaultBlockSize);
				pgpCopyMemory((PGPUInt8 *) &threeClusts, blockBuf, 
					sizeof(threeClusts));

				// Write the first FAT.
				pos = fat.fdReservedSecs;
				derr = Write(blockBuf, pos, 1);

				// Write the second FAT.
				if (derr.IsntError())
				{
					pos += fat.fdFatSize;
					derr = Write(blockBuf, pos, 1);
				}
			}
			break;

		default:
			pgpAssert(FALSE);
			break;
		}
	}

	// Unlock the volume.
	if (lockedForFormat)
	{
		UnlockVolume();
	}

	// Free our block buffer.
	if (allocedBlockBuf)
	{
		FreeByteBuffer(blockBuf);
	}

	return derr;
}

// Mount asks the driver to mount a volume. If 'pMNT' exists, it must use
// this packet passed to it from a derived class, instead of its own packet.
// This allows extensions to be made to the MNT structure without breaking
// base classes.

DualErr 
Volume::Mount(PGPBoolean mountReadOnly, PAD_Mount useThisPMNT)
{
	AD_Mount	MNT, *pMNT;
	DualErr		derr;
	PGPUInt8	drive;

	pMNT = (useThisPMNT ? useThisPMNT : &MNT);

	pgpAssertAddrValid(pMNT, AD_Mount);
	pgpAssert(Unmounted());

	// Initialize the fields we are responsible for.
	pMNT->readOnly	= mountReadOnly;		// TRUE if read-only
	pMNT->pDrive	= &drive;				// will be drive mounted on

	// Send the packet to the driver.
	derr = SendMountRequest(pMNT);

	if (derr.IsntError())
	{
		CString root;

		mDrive = (* pMNT->pDrive);
		mMountState = kVol_Mounted;

		// Tell the system about the new volume.
		if (MakeRoot(mDrive, &root).IsntError())
		{
			BroadcastDriveMessage(mDrive, DBT_DEVICEARRIVAL);
			SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, root, NULL);
		}

		if (FillInVolInfo().IsError())
		{
			mBlockSize = kDefaultBlockSize;
			mTotalBlocks = 0;
		}
	}

	return derr;
}

// Unmount asks the driver to unmount a volume. If 'pUMNT' exists, it must
// use this packet passed to it from a derived class, instead of its own
// packet. This allows extensions to be made to the UMNT structure without
// breaking base classes.

DualErr 
Volume::Unmount(PGPBoolean isThisEmergency, PAD_Unmount useThisPUNMNT)
{
	AD_Unmount	UNMNT, *pUNMNT;
	DualErr		derr;

	pUNMNT = (useThisPUNMNT ? useThisPUNMNT : &UNMNT);

	pgpAssertAddrValid(pUNMNT, AD_Unmount);
	pgpAssert(Mounted());

	// Tell the system the volume is going away.
	if (IsWinNT4CompatibleMachine())
	{
		BroadcastDriveMessage(mDrive, DBT_DEVICEREMOVEPENDING);
	}

	// Initialize the fields we are responsible for.
	pUNMNT->drive = mDrive;						// drive number to unmount
	pUNMNT->isThisEmergency = isThisEmergency;	// emergency unmount?

	// Send the packet to the driver.
	derr = SendUnmountRequest(pUNMNT);

	if (derr.IsntError())
	{
		CString root;

		// Tell the system the volume has gone away.
		if (MakeRoot(mDrive, &root).IsntError())
		{
			SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH, root, NULL);
		}

		mDrive = kInvalidDrive;
		mMountState = kVol_Unmounted;
	}

	return derr;
}


//////////////////////////////////////////
// Class Volume protected member functions
//////////////////////////////////////////

// CreateReasonableWriteBuffer creates a buffer of a size that's reasonable
// given that it will be used to write out data of size 'blocksData'.

DualErr 
Volume::CreateReasonableWriteBuffer(
	PGPUInt32 blocksData, 
	PGPUInt8 **buf, 
	PGPUInt32 *pBlocksSizeBuf)
{
	DualErr		derr;
	PGPUInt32	blocksSizeBuf;

	pgpAssert(blocksData > 0);
	pgpAssertAddrValid(buf, PGPUInt8 *);
	pgpAssertAddrValid(pBlocksSizeBuf, PGPUInt32);

	// Calculate the ideal size of the buffer to create.
	blocksSizeBuf = blocksData/kIdealNumWritesPerData;

	if (blocksSizeBuf < kMinBlocksPerWrite)
	{
		blocksSizeBuf = kMinBlocksPerWrite;
	}
	else if (blocksSizeBuf > kMaxBlocksPerWrite)
	{
		blocksSizeBuf = kMaxBlocksPerWrite;
	}

	// Create the largest buffer we can that's closest to our ideal size.
	while (TRUE)
	{
		derr = GetByteBuffer(blocksSizeBuf*kDefaultBlockSize, buf);

		if (derr.IsntError())
		{
			(* pBlocksSizeBuf) = blocksSizeBuf;
			break;
		}
		else
		{
			blocksSizeBuf /= 2;

			if (blocksSizeBuf < kMinBlocksPerWrite)
				break;
		}
	}

	return derr;
}


////////////////////////////////////////
// Class Volume private member functions
////////////////////////////////////////

// FillInVolInfo asks the driver for some extra volume information.

DualErr 
Volume::FillInVolInfo()
{
	pgpAssert(Mounted());

	return QueryVolInfo(mDrive, &mBlockSize, &mTotalBlocks);
}

// ClearBlocks will clear the specified blocks of the Volume.

DualErr 
Volume::ClearBlocks(PGPUInt32 startBlock, PGPUInt32 endBlock)
{
	DualErr		derr;
	PGPBoolean	allocedBlanks	= FALSE;
	PGPUInt8	*blanks;
	PGPUInt32	blocksBuf;

	pgpAssert(Mounted());
	pgpAssert(LockedForFormat());

	// Allocate our block buffer.
	derr = CreateReasonableWriteBuffer(endBlock - startBlock, &blanks, 
		&blocksBuf);

	allocedBlanks = derr.IsntError();

	// Clear the blocks.
	if (derr.IsntError())
	{
		PGPUInt32 blocksThisTime;

		pgpClearMemory(blanks, blocksBuf*kDefaultBlockSize);

		for (PGPUInt32 i = startBlock; i <= endBlock; i += blocksBuf)
		{
			blocksThisTime = min(blocksBuf, endBlock - i + 1);

			derr = Write(blanks, i, blocksThisTime);

			if (derr.IsError())
			{
				break;
			}
		}
	}

	// Free our block buffer.
	if (allocedBlanks)
		FreeByteBuffer(blanks);

	return derr;
}

// InitFAT12BootBlock fills a FAT12 boot block structure.

void 
Volume::InitFAT12BootBlock(
	PGPUInt64		blocksDisk, 
	FatData			*fat, 
	BootSector12	*bb12)
{
	Geometry	geom;
	PartEntry	part;
	PGPUInt64	megsDisk;

	pgpAssertAddrValid(fat, FatData);
	pgpAssertAddrValid(bb12, BootSector12);
	pgpAssert(blocksDisk > 0);

	pgpClearMemory(bb12, sizeof(BootSector12));

	megsDisk = (blocksDisk*kDefaultBlockSize) / kBytesPerMeg;
	CalcGeometry(blocksDisk, kDefaultBlockSize, &geom);

	// First we fill in the fields of the kBootBlock structure so our VolFile
	// will look like a real hard disk to Windows when it is mounted.

	bb12->bsJump[0]	= 0xEB;		// fill in jmp instruction to look real
	bb12->bsJump[1]	= 0x3C;
	bb12->bsJump[2]	= 0x90;

	pgpCopyMemory(kDefaultOEMName, bb12->bsOemName, 8);	// can be anything

	bb12->bsBytesPerSec		= kDefaultBlockSize;	// bytes per sector
	bb12->bsSecPerClust		= 1;					// sectors per cluster
	bb12->bsResSectors		= fat->fdReservedSecs;	// reserved sectors
	bb12->bsFats			= fat->fdFatCount;		// number of FATs
	bb12->bsRootDirEnts		= fat->fdRootDirEnts;	// entries in root dir
	bb12->bsSectors			= (PGPUInt16) geom.geSecsDisk;
	bb12->bsMedia			= kMediaByte;			// 0xF8 for hard disks
	bb12->bsFatSecs			= (PGPUInt16) fat->fdFatSize;	// secs per FAT
	bb12->bsSecPerTrack		= geom.geSpt;			// sectors per track
	bb12->bsHeads			= geom.geHeads;			// number of heads
	bb12->bsHiddenSecs		= 0;					// no hidden sectors
	bb12->bsHugeSectors		= 0;
	bb12->bsDriveNumber		= kHardDriveId;			// 0x80 since hard drive
	bb12->bsBootSignature	= kFirstBootSig;		// 0x29 needs to go here
	bb12->bsVolumeId		= (PGPUInt32) &bb12;	// address as volume ID
	bb12->bsSignature		= kSecondBootSig;		// boot sector signature

	pgpCopyMemory(kDefaultVolLabel, bb12->bsVolumeLabel, 11);	// vol name
	pgpCopyMemory(kFat12IdStr, bb12->bsFileSysType, 8);			// FS type

	// Fill in our local partition entry structure with the necessary values
	// to simulate a single partition covering the entire disk.

	part.peBootable			= kHardDriveId;			// 0x80 for bootable vols
	part.peBeginHead		= 0;					// starts at vector 0
	part.peBeginSector		= 1;
	part.peBeginCylinder	= 0;
	part.peFileSystem		= kFat12PartId;
	part.peEndHead			= geom.geHeads;			// end at this vector
	part.peEndSector		= geom.geSpt;
	part.peEndCylinder		= geom.geCyls;
	part.peStartSector		= 1;					// no bias to start
	part.peSectors			= (PGPUInt32) geom.geSecsDisk;	// total sectors

	// Copy the partition information into the first entry.
	bb12->bsPartEnts[0] = part;		// simple memberwise copy
}

// InitFAT16BootBlock fills a FAT16 boot block structure.

void 
Volume::InitFAT16BootBlock(
	PGPUInt64		blocksDisk, 
	FatData			*fat, 
	BootSector16	*bb16)
{			
	Geometry	geom;
	PartEntry	part;
	PGPUInt64	megsDisk;

	pgpAssertAddrValid(fat, FatData);
	pgpAssertAddrValid(bb16, BootSector16);
	pgpAssert(blocksDisk > 0);

	pgpClearMemory(bb16, sizeof(BootSector16));

	megsDisk = (blocksDisk*kDefaultBlockSize) / kBytesPerMeg;
	CalcGeometry(blocksDisk, kDefaultBlockSize, &geom);

	// First we fill in the fields of the kBootBlock structure so our VolFile
	// will look like a real hard disk to Windows95 when it is mounted.

	bb16->bsJump[0]	= 0xEB;		// fill in jmp instruction to look real
	bb16->bsJump[1]	= 0x3C;
	bb16->bsJump[2]	= 0x90;

	pgpCopyMemory(kDefaultOEMName, bb16->bsOemName, 8);	// can be anything

	bb16->bsBytesPerSec		= kDefaultBlockSize;	// bytes per sector
	bb16->bsSecPerClust		= (PGPUInt8) fat->fdSpc;	// secs per cluster
	bb16->bsResSectors		= fat->fdReservedSecs;	// reserved sectors
	bb16->bsFats			= fat->fdFatCount;		// number of FATs
	bb16->bsRootDirEnts		= fat->fdRootDirEnts;	// entries in root dir
	bb16->bsSectors			= (PGPUInt16) (megsDisk<32 ? geom.geSecsDisk : 0);
	bb16->bsMedia			= kMediaByte;			// 0xF8 for hard disks
	bb16->bsFatSecs			= (PGPUInt16) fat->fdFatSize;	// secs per FAT
	bb16->bsSecPerTrack		= geom.geSpt;			// sectors per track
	bb16->bsHeads			= geom.geHeads;			// number of heads
	bb16->bsHiddenSecs		= 0;					// no hidden sectors
	bb16->bsHugeSectors		= (PGPUInt32) (megsDisk >= 32 ? 
								geom.geSecsDisk : 0);
	bb16->bsDriveNumber		= kHardDriveId;			// 0x80 since hard drive
	bb16->bsBootSignature	= kFirstBootSig;		// 0x29 needs to go here
	bb16->bsVolumeId		= (PGPUInt32) &bb16;	// address as volume ID
	bb16->bsSignature		= kSecondBootSig;		// boot sector signature

	pgpCopyMemory(kDefaultVolLabel, bb16->bsVolumeLabel, 11);	// vol name
	pgpCopyMemory(kFat16IdStr, bb16->bsFileSysType, 8);			// FS type
	
	// Fill in our local partition entry structure with the necessary values
	// to simulate a single partition covering the entire disk.

	part.peBootable			= kHardDriveId;			// 0x80 for bootable vols
	part.peBeginHead		= 0;					// starts at vector 0
	part.peBeginSector		= 1;
	part.peBeginCylinder	= 0;
	part.peFileSystem		= (megsDisk >= 32 ? kBigFat16PartId : 
								kSmallFat16PartId);
	part.peEndHead			= geom.geHeads;			// end at this vector
	part.peEndSector		= geom.geSpt;
	part.peEndCylinder		= geom.geCyls;
	part.peStartSector		= 1;					// no bias to start
	part.peSectors			= (PGPUInt32) geom.geSecsDisk;	// total sectors
	
	// Copy the partition information into the first entry.
	bb16->bsPartEnts[0] = part;		// simple memberwise copy
}

// InitFAT32BootBlock fills a FAT32 boot block structure. It also fills in a
// FAT32 BigFatBootFSInfo structure.

void 
Volume::InitFAT32BootBlock(
	PGPUInt64			blocksDisk, 
	FatData				*fat, 
	BootSector32		*bb32, 
	BigFatBootFSInfo	*bfInfo)
{			
	Geometry	geom;
	PartEntry	part;
	PGPUInt64	bytesData, megsDisk;

	pgpAssertAddrValid(fat, FatData);
	pgpAssertAddrValid(bb32, BootSector32);
	pgpAssertAddrValid(bfInfo, BigFatBootFSInfo);
	pgpAssert(blocksDisk > 0);

	bytesData = blocksDisk * kDefaultBlockSize;
	megsDisk = (blocksDisk*kDefaultBlockSize)/kBytesPerMeg;

	CalcGeometry(blocksDisk, kDefaultBlockSize, &geom);

	pgpClearMemory(bb32, sizeof(BootSector32));
	pgpClearMemory(bfInfo, sizeof(BigFatBootFSInfo));

	// First we fill in the fields of the kBootBlock structure so our VolFile
	// will look like a real hard disk to Windows95 when it is mounted.

	bb32->bsJump[0]	= 0xEB;		// fill in jmp instruction to look real
	bb32->bsJump[1]	= 0x3C;
	bb32->bsJump[2]	= 0x90;

	pgpCopyMemory(kDefaultOEMName, bb32->bsOemName, 8);	// can be anything

	bb32->bsBytesPerSec		= kDefaultBlockSize;	// bytes per sector
	bb32->bsSecPerClust		= (PGPUInt8) fat->fdSpc;	// sectors per cluster
	bb32->bsResSectors		= fat->fdReservedSecs;	// reserved sectors
	bb32->bsFats			= fat->fdFatCount;		// number of FATs
	bb32->bsRootDirEnts		= 0;					// 0 in FAT32
	bb32->bsSectors			= 0;					// FAT32 must be > 255 Mb
	bb32->bsMedia			= kMediaByte;			// 0xF8 for hard disks
	bb32->bsFatSecs			= 0;					// 0 in FAT32
	bb32->bsSecPerTrack		= geom.geSpt;			// sectors per track
	bb32->bsHeads			= geom.geHeads;			// number of heads
	bb32->bsHiddenSecs		= 0;					// no hidden sectors
	bb32->bsHugeSectors		= (PGPUInt32) geom.geSecsDisk;

	bb32->bsBigSectorsPerFat	= fat->fdFatSize;	// total sectors per FAT
	bb32->bsExtFlags		= NULL;					// extended flags
	bb32->bsFS_Version		= NULL;					// filesystem version
	bb32->bsRootDirStrtClus	= kDefaultRootDirStart;	// first clust of root dir
	bb32->bsFsInfoSec		= kDefaultBigFatStart;	// sector of Bigfat
	bb32->bsBkUpBootSec		= -1;					// no backup boot sec

	bb32->bsDriveNumber		= kHardDriveId;			// 0x80 since hard drive
	bb32->bsBootSignature	= kFirstBootSig;		// 0x29 needs to go here
	bb32->bsVolumeId		= (PGPUInt32) &bb32;	// use address as ID
	bb32->bsSignature		= kSecondBootSig;		// boot sector signature

	pgpCopyMemory(kDefaultVolLabel, bb32->bsVolumeLabel, 11);	// vol name
	pgpCopyMemory(kFat32IdStr, bb32->bsFileSysType, 8);			// FS type
	
	// Fill in our local partition entry structure with the necessary values
	// to simulate a single partition covering the entire disk.

	part.peBootable			= 0x80;					// 0x80 for bootable vols
	part.peBeginHead		= 0;					// starts at vector 0
	part.peBeginSector		= 1;
	part.peBeginCylinder	= 0;
	part.peFileSystem		= kFat32PartId;
	part.peEndHead			= geom.geHeads;			// end at this vector
	part.peEndSector		= geom.geSpt;
	part.peEndCylinder		= geom.geCyls;
	part.peStartSector		= 1;					// no bias to start
	part.peSectors			= (PGPUInt32) geom.geSecsDisk;	// total sectors
	
	// Copy the partition information into the first entry.
	bb32->bsPartEnts[0] = part;		// simple memberwise copy

	// Finally initialize the BigFatBootInfo structure.
	bfInfo->bfSecSig				= kBigFatSecSig;
	bfInfo->bfFSInf_Sig				= kBigFatSig;
	bfInfo->bfFSInf_next_free_clus	= kDefaultRootDirStart;
	bfInfo->bsSignature				= kSecondBootSig;

	bfInfo->bfFSInf_free_clus_cnt = (PGPUInt32)
		(bytesData / (fat->fdSpc * kDefaultBlockSize) + 2);
}
