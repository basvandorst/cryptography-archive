//////////////////////////////////////////////////////////////////////////////
// VolFile.cpp
//
// Implementation of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.cpp,v 1.4 1999/03/31 23:51:08 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#define	__w64
#include <vdw.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "VolFile.h"


////////////////////////////////////////
// Class VolFile public member functions
////////////////////////////////////////

// The Class VolFile default constructor.

VolFile::VolFile() : File(), Volume()
{
	mDevExtInfo = (PGPUInt32) this;
}

// The VolFile destructor unmounts the VolFile.

VolFile::~VolFile()
{
	if (Mounted())
		Unmount();
}

// GetGeometry returns fake geometry for the drive.

void 
VolFile::GetGeometry(PDISK_GEOMETRY pGeom)
{
	pgpAssertAddrValid(pGeom, DISK_GEOMETRY);

	pGeom->MediaType			= FixedMedia;
	pGeom->Cylinders.QuadPart	= mBlocksDisk;
	pGeom->TracksPerCylinder	= 1;
	pGeom->SectorsPerTrack		= 1;
	pGeom->BytesPerSector		= kDefaultBlockSize;
}

// GetPartitionInfo returns fake partition information for the drive.

void 
VolFile::GetPartitionInfo(PPARTITION_INFORMATION pPartInfo)
{
	pgpAssertAddrValid(pPartInfo, PARTITION_INFORMATION);

	pPartInfo->StartingOffset			= RtlConvertUlongToLargeInteger(0);
	pPartInfo->PartitionLength.QuadPart	= mBlocksDisk * kDefaultBlockSize;
	pPartInfo->HiddenSectors			= 0;
	pPartInfo->PartitionNumber			= 1;
	pPartInfo->PartitionType			= PARTITION_ENTRY_UNUSED;
	pPartInfo->BootIndicator			= FALSE;
	pPartInfo->RecognizedPartition		= TRUE;
	pPartInfo->RewritePartition			= FALSE;
}

// Mount will mount the volume file specified by 'path'.

DualErr 
VolFile::Mount(
	LPCSTR		path, 
	LPCSTR		deviceName, 
	PGPUInt64	blocksHeader, 
	PGPUInt64	blocksDisk, 
	PGPUInt8	drive, 
	PGPBoolean	mountReadOnly)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssertStrValid(deviceName);

	pgpAssert(blocksDisk > 0);
	pgpAssert(Unmounted());

	// Initialize our data members.
	mBlocksDisk		= blocksDisk;
	mBlocksHeader	= blocksHeader;

	// Open the file.
	derr = Open(path, kOF_MustExist | kOF_DenyWrite | kOF_NoBuffering | 
		(mountReadOnly ? kOF_ReadOnly : NULL));

	// Attempt the mount.
	if (derr.IsntError())
	{
		derr = Volume::Mount(deviceName, drive, mountReadOnly);
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
VolFile::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);
	pgpAssert(Mounted());

	// Out of bounds?
	if (pos + nBytes > mBlocksDisk * kDefaultBlockSize)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Call down the request.
	if (derr.IsntError())
	{
		derr = File::Read(buf, pos + mBlocksHeader*kDefaultBlockSize, nBytes);
	}

	return derr;
}

// Write performs a write request on the mounted volume. Note how Write
// accounts for the bias introduced by the size of the volume file header.

DualErr 
VolFile::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);
	pgpAssert(Mounted());

	// Out of bounds?
	if (pos + nBytes > mBlocksDisk * kDefaultBlockSize)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Call down the request.
	if (derr.IsntError())
	{
		derr = File::Write(buf, pos + mBlocksHeader*kDefaultBlockSize, 
			nBytes);
	}

	return derr;
}
