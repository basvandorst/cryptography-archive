//////////////////////////////////////////////////////////////////////////////
// FatUtils.cpp
//
// Utility functions for processing FAT filesystem data.
//////////////////////////////////////////////////////////////////////////////

// $Id: FatUtils.cpp,v 1.1.2.16 1998/08/04 02:10:11 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "FatUtils.h"
#include "UtilityFunctions.h"


//////////////////////////////////
// Disk Geometry Utility Functions
//////////////////////////////////

// CalcGeometry return fake geometry information for a drive.

void 
CalcGeometry(PGPUInt64 blocksDisk, PGPUInt16 blockSize, Geometry *geom)
{
	pgpAssertAddrValid(geom, Geometry);
	pgpAssert(blocksDisk > 0);
	pgpAssert(blockSize > 0);

	geom->geHeads		= kMaxHeads - 1;
	geom->geCyls		= kMaxCyls - 1;
	geom->geSpt			= kMaxSpt - 1;
	geom->geBlockSize	= blockSize;
	geom->geSecsDisk	= blocksDisk;
}


////////////////////////////////
// General FAT Utility Functions
////////////////////////////////

// CalcFat16Spc calculates the sectors per cluster required for a FAT16
// volume of the given size in blocks.

PGPUInt16 
CalcFat16Spc(PGPUInt64 blocksData, PGPUInt16 blockSize)
{
	PGPUInt64 megsData = blocksData * blockSize/kBytesPerMeg;

	if (megsData < 16)
		return 8;
	else if (megsData < 128)
		return 4;
	else if (megsData < 256)
		return 8;
	else if (megsData < 512)
		return 16;
	else if (megsData < 1024)
		return 32;
	else 
		return 64;
}

// CalcFat32Spc calculates the sectors per cluster required for a FAT32
// volume of the given size in blocks.

PGPUInt16 
CalcFat32Spc(PGPUInt64 blocksData, PGPUInt16 blockSize)
{
	PGPUInt64 megsData = blocksData * blockSize/kBytesPerMeg;

	// This function is not reliable for sizes above 8 gigabytes as Microsoft
	// hasn't appeared to have told us yet what clusters sizes should be in
	// volumes above 8 gigs.

	if (megsData < 8192)
		return 8;
	else
		return 32;
}

// CalcFatSize will calculate the size of the FAT tables for a volume.
//
// On entry the fdBlockSize, fdFatCount, fdReservedSecs, and fdSPC members
// must be specified.
//
// On exit fdFatSize will have been filled in.

void 
CalcFatSize(FatData *fat, PGPUInt64 blocksDisk)
{
	PGPUInt32	bitsPerClust, bitsPerBlock, bitsPerSpc, numClusts;
	PGPUInt64	bitsAvail;

	pgpAssertAddrValid(fat, FatData);
	pgpAssert(IsFatVolume(fat->fdFsId));
	pgpAssert(fat->fdBlockSize > 0);
	pgpAssert(fat->fdFatCount > 0);
	pgpAssert(fat->fdSpc > 0);

	switch (fat->fdFsId)
	{
	case kFS_FAT12:
		bitsPerClust = kBitsFat12Clust;
		break;

	case kFS_FAT16:
		bitsPerClust = kBitsFat16Clust;
		break;

	case kFS_FAT32:
		bitsPerClust = kBitsFat32Clust;
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	// Bits per block = bytes per block * bits per byte.
	bitsPerBlock = fat->fdBlockSize * kBitsPerByte;

	// Bits available for FAT tables = (# of blocks on disk - # of reserved
	// sectors) * bits per block.

	bitsAvail = (blocksDisk - fat->fdReservedSecs) * bitsPerBlock;

	// Bits per SPC = # of spc * bits per block.
	bitsPerSpc = fat->fdSpc * bitsPerBlock;

	// # of clusters = available bits / (bits per SPC + (# fats * bits per
	// fat table cluster)).

	numClusts = (PGPUInt32) 
		(bitsAvail / (bitsPerSpc + (fat->fdFatCount * bitsPerClust)));

	// Blocks per FAT = ceiling of (# of clusters * bits per cluster) divided
	// by (bits per block);

	fat->fdFatSize = (PGPUInt32) CeilDiv((numClusts * bitsPerClust), 
		bitsPerBlock);
}

// CalcFatDataSec will calculate and return the index of the first sector on
// disk of the data area of a FAT drive.
//
// On entry the fdBlockSize, fdFatCount, fdReservedSecs, fdSPC, 
// fdRootDirEnts (if non-FAT32), and fdFatSize members must be specified.
//
// On exit fdFirstSecData will have been filled in.

void 
CalcFatDataSec(FatData *fat)
{
	PGPUInt32 bytesDirEnt, secsRootDir;

	pgpAssertAddrValid(fat, FatData);
	pgpAssert(IsFatVolume(fat->fdFsId));
	pgpAssert(fat->fdBlockSize > 0);
	pgpAssert(fat->fdFatCount > 0);
	pgpAssert(fat->fdSpc > 0);
	pgpAssert(fat->fdFatSize > 0);

	switch (fat->fdFsId)
	{
	case kFS_FAT12:
	case kFS_FAT16:
		// Size of the root directory = ceiling of (# of root directory
		// entries * the size of each entry) divided by (# bytes per sector).

		bytesDirEnt = (fat->fdFsId == kFS_FAT12 ? kBytesFat12DirEnt : 
			kBytesFat16DirEnt);

		// First sector of data area = (# of reserved sectors) plus (# of FAT
		// tables times size of each table in sectors) plus (sectors in the
		// root directory).

		secsRootDir = (PGPUInt32) 
			CeilDiv((fat->fdRootDirEnts * bytesDirEnt), 
			fat->fdBlockSize);

		fat->fdFirstSecData = fat->fdReservedSecs + 
			(fat->fdFatCount * fat->fdFatSize) + secsRootDir;
		break;

	case kFS_FAT32:
		// First sector of data area = (# of reserved sectors) plus (# of FAT
		// tables times size of each table in sectors).

		fat->fdFirstSecData = fat->fdReservedSecs + 
			(fat->fdFatCount*fat->fdFatSize);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// InitFatData will initialize a FatData structure with default values. On
// entrance, the fdFsId member must be filled in.

void 
InitFatData(FatData *fat, PGPUInt64 blocksDisk)
{
	pgpAssertAddrValid(fat, FatData);
	pgpAssert(IsFatVolume(fat->fdFsId));

	switch (fat->fdFsId)
	{

	case kFS_FAT12:
		fat->fdBlockSize	= kDefaultBlockSize;
		fat->fdActiveFat	= 0;
		fat->fdFatCount		= kDefaultFatCount;
		fat->fdReservedSecs	= kDefaultFat12Reserved;
		fat->fdRootDirEnts	= kDefaultRootDirEnts;
		fat->fdSpc			= 1;
		break;

	case kFS_FAT16:
		fat->fdBlockSize	= kDefaultBlockSize;
		fat->fdActiveFat	= 0;
		fat->fdFatCount		= kDefaultFatCount;
		fat->fdReservedSecs	= kDefaultFat16Reserved;
		fat->fdRootDirEnts	= kDefaultRootDirEnts;
		fat->fdSpc			= CalcFat16Spc(blocksDisk, fat->fdBlockSize);
		break;

	case kFS_FAT32:
		fat->fdBlockSize	= kDefaultBlockSize;
		fat->fdActiveFat	= kDefaultActiveFat;
		fat->fdFatCount		= kDefaultFatCount;
		fat->fdReservedSecs	= kDefaultFat32Reserved;
		fat->fdSpc			= CalcFat32Spc(blocksDisk, fat->fdBlockSize);
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	CalcFatSize(fat, blocksDisk);
	CalcFatDataSec(fat);
}


/////////////////////////////////////
// Cluster Specific Utility Functions
/////////////////////////////////////

// ClusterToSector returns the sector number corresponding to the first sector
// that the given cluster points to. The bias is a small constant subtracted
// from the cluster index to account for some reserved non-data clusters at
// the beginning of every FAT table.
//
// First sector referenced by cluster = the first sector of the data area +
// the adjusted index of the cluster * the number of sectors per cluster.

PGPUInt32 
ClusterToSector(
	PGPUInt32 clustNum, 
	PGPUInt32 firstSecData, 
	PGPUInt16 spc, 
	PGPUInt16 bias)
{
	pgpAssert(firstSecData > 0);
	pgpAssert(spc > 0);

	return firstSecData + ((clustNum - bias) * spc);
}

// ExtractCluster returns the value stored at the cluster that is bitOffset
// bits into the buffer mSecBuf (which contains sectors from a FAT table),
// representing the cluster index curClust.

PGPUInt32 
ExtractCluster(
	PGPUInt8	*buf, 
	PGPUInt32	bitOffset, 
	PGPUInt32	curClust, 
	FileSysId	fsId)
{
	PGPUInt8 a, b;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(IsFatVolume(fsId));

	switch (fsId)
	{
	case kFS_FAT12:
		a = buf[bitOffset/kBitsPerByte];				// get byte 1
		b = buf[bitOffset/kBitsPerByte + 1];			// get byte 2

		if (curClust % 2)
			return (MakeWord(a, b) >> 4);
		else
			return (MakeWord(a, b) & 0x0FFF);

	case kFS_FAT16:
		return ((PGPUInt16*) buf)[bitOffset/kBitsPerWord];

	case kFS_FAT32:
		return ((PGPUInt32*) buf)[bitOffset/kBitsPerDWord] & kFat32ClustMask;


	default:
		pgpAssert(FALSE);
		return 0;
	}

	return 0;
}

// GetClusterSector takes as input:
//
// fFatSec = the number of the first sector of a FAT table,
// clustNum = the index of a cluster in that table,
// bitsClust = the size of each cluster in bits.
//
// As output, it sets the value of these variables (passed by reference):
//
// clustSec = the number of the sector (within the FAT table) containing
// bitOffset = the offset of the cluster in that particular sector (in bits).

void 
GetClusterInfo(
	PGPUInt32	firstFatSec, 
	PGPUInt32	clustNum, 
	PGPUInt32	bitsClust, 
	PGPUInt16	blockSize, 
	PGPUInt32	*clustSec, 
	PGPUInt32	*bitOffset)
{
	pgpAssertAddrValid(clustSec, PGPUInt32);
	pgpAssertAddrValid(bitOffset, PGPUInt32);
	pgpAssert(firstFatSec > 0);
	pgpAssert(bitsClust > 0);

	(* clustSec) = firstFatSec + 
		((clustNum*bitsClust)/kBitsPerByte/blockSize);

	(* bitOffset) = (PGPUInt16) 
		((clustNum*bitsClust)%(blockSize*kBitsPerByte));
}

// TotalClusters returns the total number of clusters in a FAT table of size
// 'fatSize' (in sectors) with clusters of size 'bitsClust' (in bits) and
// blocks of size 'blockSize' (in bytes).

PGPUInt32 
TotalClusters(
	PGPUInt32	fatSize, 
	PGPUInt32	bitsClust, 
	PGPUInt16	blockSize)
{
	pgpAssert(fatSize > 0);
	pgpAssert(bitsClust > 0);
	pgpAssert(blockSize > 0);

	return (fatSize * blockSize * kBitsPerByte) / bitsClust;
}
