//////////////////////////////////////////////////////////////////////////////
// VolFile.cpp
//
// Implementation of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.cpp,v 1.9 1999/02/13 04:24:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "FatUtils.h"
#include "UtilityFunctions.h"

#include "VolFile.h"


////////////////////////////////////////
// Class VolFile public member functions
////////////////////////////////////////

// The Class VolFile default constructor.

VolFile::VolFile() : File(), Volume()
{
	mIOEnabled = TRUE;
	mUnmountPending = FALSE;

	mIsInUse = FALSE;
	mUpInfo = NULL;

	mBBlock = NULL;

	// Get our data buffer.
	mInitErr = GetByteBuffer(kDefaultBlockSize, &mBBlock);
}

// The VolFile destructor unmounts the VolFile.

VolFile::~VolFile()
{
	if (Mounted())
		Unmount();

	if (IsntNull(mBBlock))
	{
		FreeByteBuffer(mBBlock);
		mBBlock = NULL;
	}
}

// IsIOEnabled returns TRUE if I/O to this VolFile is enabled,
// false otherwise.

PGPBoolean 
VolFile::IsIOEnabled()
{
	return mIOEnabled;
}

// IsUnmountPending returns TRUE if this VolFile is unmounting, FALSE
// otherwise.

PGPBoolean 
VolFile::IsUnmountPending()
{
	return mUnmountPending;
}

// EnableIO enables or disables I/O to this VolFile.

void 
VolFile::EnableIO(PGPBoolean isEnabled)
{
	pgpAssert(Mounted());

	mIOEnabled = isEnabled;
}

// MarkUnmountPending marks or unmarks a VolFile as being unmounted.

void 
VolFile::MarkUnmountPending(PGPBoolean isUnmountPending)
{
	pgpAssert(Mounted());

	mUnmountPending = isUnmountPending;
}

// GetDevParams16 will fill out the elements of DevParams16 structure for the
// mounted VolFile.

DualErr 
VolFile::GetDevParams16(DevParams16 *dp16)
{
	DualErr		derr;
	FileSysId	fsId;

	pgpAssert(Mounted());
	pgpAssertAddrValid(dp16, DevParams16);

	// Get the existing file system ID.
	derr = GetFsId(&fsId);

	if (derr.IsntError())
	{
		// If it appears to be already formatted, take the device params
		// direct from the bootblock, else invent our own.

		if (IsFatVolume(fsId))
		{
			derr = Volume::GetDevParams16(dp16);
		}
		else
		{
			FatData		fat;
			Geometry	geom;
			PGPUInt64	megsDisk;

			megsDisk = (mBlocksDisk * kDefaultBlockSize) / kBytesPerMeg;
			fat.fdFsId = (megsDisk < 2 ? kFS_FAT12 : kFS_FAT16);

			InitFatData(&fat, mBlocksDisk);
			CalcGeometry(mBlocksDisk, kDefaultBlockSize, &geom);

			pgpClearMemory(dp16, sizeof(DevParams16));

			dp16->dpDevType		= 0x05;				// we are a hard disk
			dp16->dpDevAttr		= 1;				// we are not removable
			dp16->dpCylinders	= geom.geCyls;
			dp16->dpMediaType	= kMediaByte;

			dp16->dpBytesPerSec	= kDefaultBlockSize;
			dp16->dpSecPerClust	= (PGPUInt8) fat.fdSpc;
			dp16->dpResSectors	= fat.fdReservedSecs;
			dp16->dpFats		= fat.fdFatCount;
			dp16->dpRootDirEnts	= fat.fdRootDirEnts;
			dp16->dpSectors		= (PGPUInt16) (megsDisk < 32 ? 
				mBlocksDisk : 0);
			dp16->dpMedia		= kMediaByte;
			dp16->dpFatSecs		= (PGPUInt16) fat.fdFatSize;
			dp16->dpSecPerTrack	= geom.geSpt;
			dp16->dpHeads		= geom.geHeads;
			dp16->dpHiddenSecs	= 0;
			dp16->dpHugeSectors	= (PGPUInt32) 
									(megsDisk >= 32 ? mBlocksDisk : 0);
		}
	}

	return derr;
}

// GetDevParams32 will fill out the elements of DevParams16 structure for the
// mounted VolFile.

DualErr 
VolFile::GetDevParams32(DevParams32 *dp32)
{
	DualErr		derr;
	FileSysId	fsId;

	pgpAssert(Mounted());
	pgpAssertAddrValid(dp32, DevParams32);

	// Get the existing file system ID.
	derr = GetFsId(&fsId);

	if (derr.IsntError())
	{
		// If it appears to be already formatted, take the device params
		// direct from the bootblock, else invent our own.

		if (IsFatVolume(fsId))
		{
			derr = Volume::GetDevParams32(dp32);
		}
		else
		{
			FatData		fat;
			Geometry	geom;
			PGPUInt64	megsDisk;

			megsDisk = (mBlocksDisk * kDefaultBlockSize) / kBytesPerMeg;
			fat.fdFsId = (megsDisk < 2 ? kFS_FAT12 : kFS_FAT16);

			InitFatData(&fat, mBlocksDisk);
			CalcGeometry(mBlocksDisk, kDefaultBlockSize, &geom);

			pgpClearMemory(dp32, sizeof(DevParams32));

			dp32->dpDevType		= 0x05;			// we are a hard disk
			dp32->dpDevAttr		= 1;			// we are not removable
			dp32->dpCylinders	= geom.geCyls;
			dp32->dpMediaType	= kMediaByte;

			dp32->dpBytesPerSector		= kDefaultBlockSize;
			dp32->dpSectorsPerClust		= (PGPUInt8) fat.fdSpc;
			dp32->dpReservedSectors		= fat.fdReservedSecs;
			dp32->dpNumberOfFats		= fat.fdFatCount;
			dp32->dpRootEntries			= fat.fdRootDirEnts;
			dp32->dpTotalSectors		= (PGPUInt16) 
				(megsDisk < 32 ? mBlocksDisk : 0);
			dp32->dpMediaDescriptor		= kMediaByte;
			dp32->dpSectorsPerFat		= (PGPUInt16) fat.fdFatSize;
			dp32->dpSectorsPerTrack		= geom.geSpt;
			dp32->dpHeads				= geom.geHeads;
			dp32->dpHiddenSectors		= 0;
			dp32->dpBigTotalSectors		= (PGPUInt32) (megsDisk >= 32 ? 
											mBlocksDisk : 0);
		}
	}

	return derr;
}

// Mount will mount the volume file specified by 'path'.

DualErr 
VolFile::Mount(
	LPCSTR		path, 
	PGPUInt64	blocksHeader, 
	PGPUInt64	blocksDisk, 
	PDCB		pDcb, 
	PGPUInt8	drive, 
	PGPBoolean	mountReadOnly)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssert(blocksDisk > 0);
	pgpAssertAddrValid(pDcb, PDCB);

	pgpAssert(Unmounted());

	// Initialize our data members.
	mBlocksDisk		= blocksDisk;
	mBlocksHeader	= blocksHeader;

	// Prepare the DCB.
	SetupDcb(pDcb, mountReadOnly);

	// Open the file.
	derr = Open(path, kOF_MustExist | kOF_DenyWrite | kOF_EnableDirectDisk | 
		(mountReadOnly ? kOF_ReadOnly : NULL));

	// Fill the bootblock cache.
	if (derr.IsntError())
	{
		derr = File::Read(mBBlock, blocksHeader*kDefaultBlockSize, 
			kDefaultBlockSize);
	}

	// Attempt the mount.
	if (derr.IsntError())
	{
		derr = Volume::Mount(pDcb, drive);
	}

	// Cleanup in case of error.
	if (derr.IsError())
	{
		if (Opened())
			Close();
	}

	return derr;
}

// Unmount unmounts a mounted volume file. It calls down to Volume::Unmount
// to do the job, then closes the file.

DualErr 
VolFile::Unmount(PGPBoolean isThisEmergency)
{
	DualErr derr;

	pgpAssert(Mounted());

	derr = Volume::Unmount(isThisEmergency);

	if (derr.IsntError())
	{
		DualErr afterFact = Close();
		pgpAssert(afterFact.IsntError());
	}

	return derr;
}

// Read performs a read request on the mounted volume.

DualErr 
VolFile::Read(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPBoolean	useAsync;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);
	pgpAssert(Mounted());

	useAsync = IsntNull(upInfo);

	if (pos + nBytes > mBlocksDisk * kDefaultBlockSize)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Verify IO is enabled to this drive.
	if (derr.IsntError())
	{
		if (!IsIOEnabled())
			derr = DualErr(kPGDMinorError_IOFlagViolation);
	}

	// We now check if the system is reading the boot block only. In this
	// case, we need to read from our cache because touching the disk may
	// result in a hang.

	if (derr.IsntError())
	{
		if ((pos == 0) && (nBytes == kDefaultBlockSize))
		{
			pgpCopyMemory(mBBlock, buf, kDefaultBlockSize);

			if (useAsync)
				ScheduleAsyncCallback(upInfo, derr);
		}
		else
		{
			DualErr readErr;

			readErr = File::Read(buf, 
				pos + mBlocksHeader*kDefaultBlockSize, nBytes, upInfo);

			if (!useAsync)
				derr = readErr;
		}
	}

	if (useAsync && derr.IsError())
	{
		ScheduleAsyncCallback(upInfo, derr);
	}

	return derr;
}

// Write performs a write request on the mounted volume. Note how Write
// accounts for the bias introduced by the size of the volume file header.

DualErr 
VolFile::Write(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPBoolean	useAsync;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);
	pgpAssert(Mounted());

	useAsync = IsntNull(upInfo);

	if (pos + nBytes > mBlocksDisk * kDefaultBlockSize)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Verify IO is enabled to this drive.
	if (derr.IsntError())
	{
		if (!IsIOEnabled())
			derr = DualErr(kPGDMinorError_IOFlagViolation);
	}

	// If the system is writing the bootblock, stick it in our cache.
	if (derr.IsntError())
	{
		DualErr writeErr;

		if ((pos == 0) && (nBytes >= kDefaultBlockSize))
		{
			pgpCopyMemory(buf, mBBlock, kDefaultBlockSize);
		}

		writeErr = File::Write(buf, 
			pos + mBlocksHeader*kDefaultBlockSize, nBytes, upInfo);

		if (!useAsync)
			derr = writeErr;
	}

	if (useAsync && derr.IsError())
	{
		ScheduleAsyncCallback(upInfo, derr);
	}

	return derr;
}


/////////////////////////////////////////
// Class VolFile private member functions
/////////////////////////////////////////

// SetupDcb initializes the DCB for the VolFile object prior to mounting.

void 
VolFile::SetupDcb(PDCB pDcb, PGPBoolean mountReadOnly)
{
	Geometry geom;

	pgpAssertAddrValid(pDcb, PDCB);

	CalcGeometry(mBlocksDisk, kDefaultBlockSize, &geom);

	if (!mountReadOnly)
	{
		pDcb->DCB_cmn.DCB_device_flags |= DCB_DEV_WRITEABLE;
		pDcb->DCB_bdd.DCB_BDD_flags |= BDF_WRITEABLE;
	}

	pDcb->DCB_cmn.DCB_apparent_blk_shift	= SimpleLog2(geom.geBlockSize);
	pDcb->DCB_actual_sector_cnt[0]			= GetLowDWord(geom.geSecsDisk);
	pDcb->DCB_actual_sector_cnt[1]			= GetHighDWord(geom.geSecsDisk);
	pDcb->DCB_actual_blk_size				= geom.geBlockSize;
	pDcb->DCB_actual_head_cnt				= geom.geHeads;
	pDcb->DCB_actual_cyl_cnt				= geom.geCyls;
	pDcb->DCB_actual_spt					= geom.geSpt;

	pDcb->DCB_bdd.DCB_apparent_sector_cnt[0]	= GetLowDWord(geom.geSecsDisk);
	pDcb->DCB_bdd.DCB_apparent_sector_cnt[1]	= GetHighDWord(geom.geSecsDisk);
	pDcb->DCB_bdd.DCB_apparent_blk_size			= geom.geBlockSize;
	pDcb->DCB_bdd.DCB_apparent_head_cnt			= geom.geHeads;
	pDcb->DCB_bdd.DCB_apparent_cyl_cnt			= geom.geCyls;
	pDcb->DCB_bdd.DCB_apparent_spt				= geom.geSpt;
}

// ScheduleAsyncCallback schedules a windows event that calls our function
// that will call the asynchronous request up.

void 
VolFile::ScheduleAsyncCallback(GenericCallbackInfo *upInfo, DualErr derr)
{
	static RestrictedEvent_THUNK callbackThunk;

	pgpAssert(!mIsInUse);
	mIsInUse = TRUE;

	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	mUpInfo = upInfo;
	mUpInfo->derr = derr;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) this, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID 
__stdcall 
VolFile::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	VolFile *pVF;

	pVF = (VolFile *) Refdata;
	pgpAssertAddrValid(pVF, VolFile);

	pgpAssert(pVF->mIsInUse);
	pVF->mIsInUse = FALSE;

	pgpAssertAddrValid(pVF->mUpInfo, GenericCallbackInfo);
	pVF->mUpInfo->callback(pVF->mUpInfo);
}
