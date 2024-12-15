//////////////////////////////////////////////////////////////////////////////
// Volume.h
//
// Declaration of class Volume.
//////////////////////////////////////////////////////////////////////////////

// $Id: Volume.h,v 1.1.2.11 1998/07/06 08:57:34 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Volume_h	// [
#define Included_Volume_h

#include "FatUtils.h"
#include "Packets.h"


///////////////
// Class Volume
///////////////

// Class Volume represent a local volume. This class's purpose is to serve as
// one of the two base classes for class VolFile.

class Volume
{
public:
				Volume(PGPUInt8 drive = kInvalidDrive);
	virtual		~Volume();

	PGPUInt8	GetDrive();
	PGPUInt16	GetBlockSize();
	PGPUInt64	GetTotalBlocks();

	PGPBoolean	Mounted();
	PGPBoolean	Unmounted();
	PGPBoolean	AttachedToLocalVolume();
	PGPBoolean	HasOpenFiles();

	PGPBoolean	LockedForReadWrite();
	PGPBoolean	LockedForFormat();

	void		BrowseToVolume();

	DualErr		GetVolumeLabel(LPSTR label, PGPUInt32 size);
	DualErr		SetVolumeLabel(LPCSTR label);

	DualErr		AttachLocalVolume(PGPUInt8 drive);
	void		DetachLocalVolume();

	DualErr		LockVolumeForReadWrite();
	DualErr		LockVolumeForFormat();
	DualErr		UnlockVolume();

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBlocks);

	DualErr		Format();

	DualErr		Mount(PGPBoolean mountReadOnly = FALSE, 
					PAD_Mount useThisPMNT = NULL);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE, 
					PAD_Unmount useThisPUNMT = NULL);

protected:
	DualErr		CreateReasonableWriteBuffer(PGPUInt32 blocksData, 
					PGPUInt8 **buf, PGPUInt32 *pBlocksSizeBuf);

private:
	enum LockState	{kLock_None, kLock_ReadWrite, kLock_Format};
	enum MountState	{kVol_Mounted, kVol_Unmounted};

	MountState	mMountState;		// mount status of the volume
	LockState	mLockState;			// lock status of the volume
	PGPUInt8	mDrive;				// drive # of the volume

	PGPBoolean	mAttachedToLocalVol;	// attached to local volume?
	PGPUInt16	mBlockSize;			// block size
	PGPUInt64	mTotalBlocks;		// block size

	DualErr	FillInVolInfo();

	DualErr ClearBlocks(PGPUInt32 startBlock, PGPUInt32 endBlock);

	void	InitFAT12BootBlock(PGPUInt64 blocksDisk, FatData *fat, 
				BootSector12 *bb12);
	void	InitFAT16BootBlock(PGPUInt64 blocksDisk, FatData *fat, 
				BootSector16 *bb16);
	void	InitFAT32BootBlock(PGPUInt64 blocksDisk, FatData *fat, 
				BootSector32 *bb32, BigFatBootFSInfo *bfInfo);
};

#endif	// ] Included_Volume_h
