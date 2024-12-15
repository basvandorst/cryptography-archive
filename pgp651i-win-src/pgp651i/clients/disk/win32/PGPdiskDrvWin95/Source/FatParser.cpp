//////////////////////////////////////////////////////////////////////////////
// FatParser.cpp
//
// Implementation of class FatParser.
//////////////////////////////////////////////////////////////////////////////

// $Id: FatParser.cpp,v 1.7 1999/02/13 04:24:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "FatParser.h"
#include "Globals.h"


////////////
// Constants
////////////

PGPUInt32	kFPFragArrayChunkSize	= 20;		// frags to allocate at once
PGPUInt32	kFPDataBufSize			= 2;		// # FAT secs at once
PGPUInt32	kFPMaxLoops				= 0x500000;	// max clusters to loop


//////////////////////////////////////////
// Class FatParser public member functions
//////////////////////////////////////////

// The FatParser path and Volume object overloaded constructor prepares for
// I/O to the specified file on the specified host.

FatParser::FatParser(LPCSTR path, PGPUInt8 hostDrive, PGPUInt32 bytesFile)
	 : FileSystemParser(path, hostDrive, bytesFile)
{
	pgpAssert(mHost.Mounted());
	pgpAssert(IsFatVolume(mFsId));

	mDataBuf		= NULL;
	mNumFragsInUse	= 0;
	mSizeFragArray	= 0;
	mFragArray		= NULL;

	mFatParserReq.isInUse = FALSE;

	mInitErr = FileSystemParser::mInitErr;

	// Note how we allocate space for one extra sector in the main sector
	// buffer. This is because FAT12 sectors can overflow sector boundaries.

	if (mInitErr.IsntError())
	{
		mInitErr = GetByteBuffer((kFPDataBufSize + 1) * mHost.GetBlockSize(), 
			&mDataBuf);
	}

	if (mInitErr.IsntError())
	{
		mInitErr = GetByteBuffer(kFPFragArrayChunkSize*sizeof(FileFrag), 
			(PGPUInt8 **) &mFragArray);

		mSizeFragArray = kFPFragArrayChunkSize;
	}

	if (mInitErr.IsntError())
	{
		mInitErr = GetFatData();
	}

	if (mInitErr.IsntError())
	{
		mInitErr = MakeFatFrags();
	}

#if PGP_DEBUG
	if (mInitErr.IsntError())
		DumpFrags();
#endif	// PGP_DEBUG
}

// The FatParser destructor is responsible for deallocating the list of frags
// and deleting the sector buffer.

FatParser::~FatParser()
{
	if (mFragArray)
		FreeByteBuffer((PGPUInt8 *) mFragArray);

	if (mDataBuf)
		FreeByteBuffer((PGPUInt8 *) mDataBuf);
}

// ReadAsync reads nBytes from the file at position pos (in bytes) 
// asynchronously.

void 
FatParser::ReadAsync(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPUInt16	blockSize;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	pgpAssertAddrValid(upInfo->callback, PGPUInt8);

	pgpAssertAddrValid(mFragArray, FileFrag);
	pgpAssert(mHost.Mounted());
	pgpAssert(mNumFragsInUse > 0);

	pgpAssert(!mFatParserReq.isInUse);
	mFatParserReq.isInUse = TRUE;

	blockSize = mHost.GetBlockSize();

	if (pos + nBytes > mBytesFile)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Prepare the request for execution.
	if (derr.IsntError())
	{
		PGPBoolean doneWithRequest;

		mFatParserReq.op = kFPOP_Read;
		mFatParserReq.upInfo = upInfo;

		mFatParserReq.downInfo.callback = FatParserCallback;
		mFatParserReq.downInfo.refData[0] = (PGPUInt32) this;

		// We start out by initializing the request so that all possible
		// pieces of the request are marked as completed.
		
		mFatParserReq.read.readHeader	= TRUE;
		mFatParserReq.read.readMiddle	= TRUE;
		mFatParserReq.read.readTail		= TRUE;

		// Now we analyze the request and determine what pieces must be
		// read so that the entire request can be marked as done.
		
		if (pos%blockSize > 0)
		{
			PGPUInt32 bytesInHeader;

			// The request has a header - that is, it doesn't begin on a
			// block boundary.
			
			bytesInHeader = min(nBytes, blockSize - (PGPUInt32) 
				(pos%blockSize));

			mFatParserReq.read.readHeader	= FALSE;
			mFatParserReq.read.bufHeader	= buf;
			mFatParserReq.read.posHeader	= pos;
			mFatParserReq.read.nBytesHeader	= bytesInHeader;

			buf		+= bytesInHeader;
			pos		+= bytesInHeader;
			nBytes	-= bytesInHeader;
		}

		if (nBytes >= blockSize)
		{
			PGPUInt32 bytesInMiddle;

			// The request has a middle - that is, a section that begins and
			// ends on a block boundary and is more than one block in length.
			
			bytesInMiddle = nBytes - nBytes%blockSize;

			mFatParserReq.read.readMiddle	= FALSE;
			mFatParserReq.read.bufMiddle	= buf;
			mFatParserReq.read.posMiddle	= pos;
			mFatParserReq.read.nBytesMiddle	= bytesInMiddle;

			buf		+= bytesInMiddle;
			pos		+= bytesInMiddle;
			nBytes	-= bytesInMiddle;
		}

		if (nBytes > 0)
		{
			// The request has a tail - that is, it doesn't end on a
			// block boundary.
			
			mFatParserReq.read.readTail		= FALSE;
			mFatParserReq.read.bufTail		= buf;
			mFatParserReq.read.posTail		= pos;
			mFatParserReq.read.nBytesTail	= nBytes;
		}

		// Execute the request.
		ExecuteRequest(&doneWithRequest);

		// Should always have stuff to do at this point. But if we don't, then
		// don't hang the computer by not calling back.

		if (doneWithRequest)
		{
			pgpAssert(FALSE);
			ScheduleAsyncCallback();
		}
	}

	// Callback on error.
	if (derr.IsError())
	{
		ScheduleAsyncCallback(derr);
	}
}

// WriteAsync writes nBytes to the file at position pos (in bytes)
// asynchronously.

void 
FatParser::WriteAsync(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPUInt16	blockSize;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	pgpAssertAddrValid(upInfo->callback, PGPUInt8);

	pgpAssertAddrValid(mFragArray, FileFrag);
	pgpAssert(mHost.Mounted());
	pgpAssert(mNumFragsInUse > 0);

	pgpAssert(!mFatParserReq.isInUse);
	mFatParserReq.isInUse = TRUE;

	blockSize = mHost.GetBlockSize();

	if (pos + nBytes > mBytesFile)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Prepare the request for execution.
	if (derr.IsntError())
	{
		PGPBoolean doneWithRequest;

		mFatParserReq.op = kFPOP_Write;
		mFatParserReq.upInfo = upInfo;

		mFatParserReq.downInfo.callback = FatParserCallback;
		mFatParserReq.downInfo.refData[0] = (PGPUInt32) this;

		// We start out by initializing the request so that all possible
		// pieces of the request are marked as completed.
		
		mFatParserReq.write.readHeader	= TRUE;
		mFatParserReq.write.wroteHeader	= TRUE;
		mFatParserReq.write.wroteMiddle	= TRUE;
		mFatParserReq.write.readTail	= TRUE;
		mFatParserReq.write.wroteTail	= TRUE;

		// Now we analyze the request and determine what pieces must be
		// written so that the entire request can be marked as done.
		
		if (pos%blockSize > 0)
		{
			PGPUInt32 bytesInHeader;

			// The request has a header - that is, it doesn't begin on a
			// block boundary.
			
			bytesInHeader = min(nBytes, blockSize - (PGPUInt32) 
				(pos%blockSize));

			mFatParserReq.write.readHeader		= FALSE;
			mFatParserReq.write.wroteHeader		= FALSE;
			mFatParserReq.write.bufHeader		= buf;
			mFatParserReq.write.posHeader		= pos;
			mFatParserReq.write.nBytesHeader	= bytesInHeader;

			buf		+= bytesInHeader;
			pos		+= bytesInHeader;
			nBytes	-= bytesInHeader;
		}

		if (nBytes >= blockSize)
		{
			PGPUInt32 bytesInMiddle;

			// The request has a middle - that is, a section that begins and
			// ends on a block boundary and is more than one block in length.
			
			bytesInMiddle = nBytes - nBytes%blockSize;

			mFatParserReq.write.wroteMiddle		= FALSE;
			mFatParserReq.write.bufMiddle		= buf;
			mFatParserReq.write.posMiddle		= pos;
			mFatParserReq.write.nBytesMiddle	= bytesInMiddle;

			buf		+= bytesInMiddle;
			pos		+= bytesInMiddle;
			nBytes	-= bytesInMiddle;
		}

		if (nBytes > 0)
		{
			// The request has a tail - that is, it doesn't end on a
			// block boundary.
			
			mFatParserReq.write.readTail	= FALSE;
			mFatParserReq.write.wroteTail	= FALSE;
			mFatParserReq.write.bufTail		= buf;
			mFatParserReq.write.posTail		= pos;
			mFatParserReq.write.nBytesTail	= nBytes;
		}

		// Execute the request.
		ExecuteRequest(&doneWithRequest);

		// Should always have stuff to do at this point. But if we don't, then
		// don't hang the computer by not calling back.

		if (doneWithRequest)
		{
			pgpAssert(FALSE);
			ScheduleAsyncCallback();
		}
	}

	// Callback on error;
	if (derr.IsError())
	{
		ScheduleAsyncCallback(derr);
	}
}

// ReadSync reads nBytes from the file at position pos (in bytes)
// synchronously.

DualErr 
FatParser::ReadSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt16	blockSize;
	PGPUInt32	bufPos, i, blocksToRead, sizeChunk;
	PGPUInt64	blockVolume;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertAddrValid(mFragArray, FileFrag);
	pgpAssert(mHost.Mounted());
	pgpAssert(mNumFragsInUse > 0);
	pgpAssert(nBytes > 0);

	blockSize = mHost.GetBlockSize();

	if (pos + nBytes > mBytesFile)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// The position (pos) and size (nBytes) variables are in bytes, but we can
	// only read from a volume in sectors. What we have to do is handle the
	// request in at most three separate pieces - a head, middle, and end.

	if (derr.IsntError())
	{
		bufPos = 0;

		if (pos%blockSize > 0)				// is there a head?
		{
			PGPUInt32 headOffset, headSize;

			i = FindFragmentIndex(pos);

			// Now read in the head.
			//
			// blockVolume = sector on disk containing the head.
			// headOffset = byte offset into blockVolume where the head starts
			// headSize = size of the head, in bytes

			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);
			headOffset	= (PGPUInt32) pos%blockSize;
			headSize	= min(nBytes, blockSize - headOffset);

			derr = mHost.Read(mDataBuf, blockVolume, 1);

			if (derr.IsntError())
			{
				pgpCopyMemory(mDataBuf + headOffset, buf, headSize);

				bufPos	= headSize;
				pos		+= headSize;
				nBytes	-= headSize;
			}
		}
	}

	// Read in the middle. Since the middle may be in any number of fragments
	// on the disk, we have to use a while loop.

	if (derr.IsntError())
	{
		blocksToRead = nBytes/blockSize;

		while (derr.IsntError() && (blocksToRead > 0))	// is there a middle?
		{
			i = FindFragmentIndex(pos);

			// Determine exactly how many sectors we can read before we must
			// move onto the next file fragment.
			//
			// blockVolume = sector on disk where the current read begins
			// sizeChunk = the most sectors we can read in this iteration

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);
			sizeChunk = min(blocksToRead, (PGPUInt32) 
				((mFragArray[i].fileEnd - pos) / blockSize) + 1);

			derr = mHost.Read(buf + bufPos, blockVolume, sizeChunk);

			if (derr.IsntError())
			{
				bufPos			+= sizeChunk * blockSize;
				nBytes			-= sizeChunk * blockSize;
				pos				+= sizeChunk * blockSize;
				blocksToRead	-= sizeChunk;
			}
		}
	}
	
	// Process the tail.
	if (derr.IsntError())
	{
		if (nBytes)							// is there a tail?	
		{
			i = FindFragmentIndex(pos);

			// This read is very simple because we know the tail begins on a
			// sector boundary.
			//
			// secsDisk = the sectors in disk containing the tail.

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);
			derr = mHost.Read(mDataBuf, blockVolume, 1);

			if (derr.IsntError())
				pgpCopyMemory(mDataBuf, buf + bufPos, nBytes);
		}
	}

	return derr;
}

// WriteSync reads nBytes from the file at position pos (in bytes)
// synchronously.

DualErr 
FatParser::WriteSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt16	blockSize;
	PGPUInt32	bufPos, i, blocksToWrite, sizeChunk;
	PGPUInt64	blockVolume;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertAddrValid(mFragArray, FileFrag);
	pgpAssert(mHost.Mounted());
	pgpAssert(mNumFragsInUse > 0);
	pgpAssert(nBytes > 0);

	blockSize = mHost.GetBlockSize();

	if (pos + nBytes > mBytesFile)
		derr = DualErr(kPGDMinorError_OOBFileRequest);

	// Process the head.
	if (derr.IsntError())
	{
		bufPos = 0;

		if (pos%blockSize > 0)				// is there a head?
		{
			PGPUInt32 headOffset, headSize;

			i = FindFragmentIndex(pos);

			// Write out the head.
			//
			// blockVolume = sector on disk containing the head
			// headOffset = byte offset into secsDisk where the head starts
			// headSize = size of the head, in bytes

			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);
			headOffset	= (PGPUInt32) pos%blockSize;
			headSize	= min(nBytes, blockSize - headOffset);

			derr = mHost.Read(mDataBuf, blockVolume, 1);

			if (derr.IsntError())
			{
				pgpCopyMemory(buf, mDataBuf + headOffset, headSize);
				derr = mHost.Write(mDataBuf, blockVolume, 1);
			}

			if (derr.IsntError())
			{
				bufPos	= headSize;
				pos		+= headSize;
				nBytes	-= headSize;
			}
		}
	}

	// Write out the middle. Since the middle may be in any number of
	// fragments on the disk, we have to use a while loop.

	if (derr.IsntError())
	{
		blocksToWrite = nBytes / blockSize;

		while (derr.IsntError() && (blocksToWrite > 0))	// is there a middle?
		{
			i = FindFragmentIndex(pos);

			// Determine exactly how many sectors we can write before we must
			// move onto the next file fragment.
			//
			// blockVolume = sector on disk where the current write begins
			// sizeChunk = the most sectors we can write in this iteration

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);
			sizeChunk = min(blocksToWrite, (PGPUInt32) 
				((mFragArray[i].fileEnd - pos) / blockSize) + 1);

			// Now we perform the write from the output buffer to disk.

			derr = mHost.Write(buf + bufPos, blockVolume, sizeChunk);

			if (derr.IsntError())
			{
				bufPos			+= sizeChunk * blockSize;
				nBytes			-= sizeChunk * blockSize;
				pos				+= sizeChunk * blockSize;
				blocksToWrite	-= sizeChunk;
			}
		}
	}

	// Process the tail.
	if (derr.IsntError())
	{
		if (nBytes)							// is there a tail?	
		{
			i = FindFragmentIndex(pos);

			// Write out the tail.
			//
			// blockVolume = sector on disk containing the tail
			// headSize = size of the tail, in bytes

			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);
			derr = mHost.Read(mDataBuf, blockVolume, 1);

			if (derr.IsntError())
			{
				pgpCopyMemory(buf, mDataBuf, nBytes);
				derr = mHost.Write(mDataBuf, blockVolume, 1);
			}
		}
	}

	return derr;
}

// Read reads nBytes from the file at position pos (in bytes). It uses the
// previously initialized member variable mFragArray to map locations in the
// file to locations on disk, since a file on a FAT volume may be spread out
// in many fragments.

DualErr 
FatParser::Read(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	if (upInfo)
		ReadAsync(buf, pos, nBytes, upInfo);
	else
		derr = ReadSync(buf, pos, nBytes);

	return derr;
}

// Write writes nBytes to the file at position pos (in bytes). It uses the
// previously initialized member variable mPFF to map locations in the file
// to locations on disk, since a file on a FAT volume may be spread out in
// many fragments.

DualErr 
FatParser::Write(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	if (upInfo)
		WriteAsync(buf, pos, nBytes, upInfo);
	else
		derr = WriteSync(buf, pos, nBytes);

	return derr;
}


///////////////////////////////////////////
// Class FatParser private member functions
///////////////////////////////////////////

// CalcDiskSector calculates the sector on disk containing a certain location
// in a file, given that location and a pointer to a single FileFrag.

PGPUInt64 
FatParser::CalcDiskSector(
	PGPUInt64	filePos, 
	PFileFrag	pFF, 
	PGPUInt16	blockSize)
{
	PGPUInt64	diskPos;

	pgpAssertAddrValid(pFF, FileFrag);
	pgpAssert(filePos >= pFF->fileBegin);
	pgpAssert(filePos <= pFF->fileEnd);

	diskPos = pFF->diskBegin + (filePos - pFF->fileBegin);

	return (diskPos / blockSize);
}

// ExpandFragArray allocated memory for more file fragments.

DualErr 
FatParser::ExpandFragArray()
{
	DualErr		derr;
	PFileFrag	temp;

	// Get memory for more fragments.
	derr = GetByteBuffer(
		(mSizeFragArray + kFPFragArrayChunkSize) * sizeof(FileFrag), 
		(PGPUInt8 **) &temp);

	if (derr.IsntError())
	{
		// Copy the old fragments to the new memory.
		pgpCopyMemory(mFragArray, temp, mSizeFragArray*sizeof(FileFrag));

		// Delete the old memory.
		FreeByteBuffer((PGPUInt8 *) mFragArray);

		mFragArray = temp;
		mSizeFragArray += kFPFragArrayChunkSize;
	}

	return derr;
}

// FindFragmentIndex will find and return the index of the file fragment that
// contains the file position specified.

PGPUInt32 
FatParser::FindFragmentIndex(PGPUInt64 pos)
{
	PGPBoolean	foundFrag	= FALSE;
	PGPUInt32	i, j, x;

	pgpAssertAddrValid(mFragArray, FileFrag);
	pgpAssert(mSizeFragArray > 0);

	i = 0;
	j = mNumFragsInUse - 1;

	// Binary search.
	do
	{
		x = (i + j)/2;

		if ((pos >= mFragArray[x].fileBegin) && 
			(pos <= mFragArray[x].fileEnd))
		{
			foundFrag = TRUE;
			break;
		}
		else
		{
			(pos > mFragArray[x].fileEnd ? i = x + 1: j = x - 1);
		}
	}
	while (j >= i);

	if (!foundFrag)
	{
		pgpDebugMsg("PGPdisk: FindFragment recieved an OOB position");
		return 0xFFFFFFFF;
	}
	else
	{
		return x;
	}
}

// InitFileFrag initializes a file fragment. It sets the beginning (and ending,
// since it is zero length) file and disk parameters according to the passed
// values.

void 
FatParser::InitFileFrag(
	PFileFrag	pFF, 
	PGPUInt64	fileByte, 
	PGPUInt64	diskByte)
{
	pgpAssertAddrValid(pFF, FileFrag);

	pFF->fileBegin	= pFF->fileEnd	= fileByte;
	pFF->diskBegin	= pFF->diskEnd	= diskByte;
	pFF->next		= pFF->prev		= NULL;
}

#if PGP_DEBUG

// DumpFrags is a debugging procedure that will print out a list of the
// fragments in a file to the debugger.

void 
FatParser::DumpFrags()
{
	PGPUInt16	blockSize;
	PGPUInt64	secBegin, secEnd;

	blockSize = mHost.GetBlockSize();
	DebugOut("\nPGPdisk: Frags Dump for %s:\n", GetPath());

	if (mNumFragsInUse == 0)
	{
		DebugOut("\nPGPdisk: No frags to dump\n");
	}
	else
	{
		for (PGPUInt32 i=0; i < mNumFragsInUse; i++)
		{
			DebugOut("PGPdisk: Frag #%u", i);

			secBegin	= mFragArray[i].fileBegin/blockSize;
			secEnd		= mFragArray[i].fileEnd/blockSize;

			DebugOut("PGPdisk: File begin: %u to file end: %u", secBegin, 
				secEnd);

			secBegin	= (PGPUInt32) (mFragArray[i].diskBegin/blockSize);
			secEnd		= (PGPUInt32) (mFragArray[i].diskEnd/blockSize);

			DebugOut("PGPdisk: Disk begin: %u to disk end: %u", secBegin, 
				secEnd);
		}
	
		DebugOut("\nPGPdisk: Frag Dump completed\n");
	}
}

#endif	// PGP_DEBUG

// GetFatData acquires data specific to the FAT filesystem of the host drive
// and fills in the data member 'mFat'.

DualErr 
FatParser::GetFatData()
{
	DualErr	derr;

	pgpAssert(mHost.Mounted());

	mFat.fdBlockSize = mHost.GetBlockSize();
	mFat.fdFsId = mFsId;

	switch (mFsId)
	{

	case kFS_FAT12:
	case kFS_FAT16:
	{
		DevParams16	dp16;

		derr = mHost.GetDevParams16(&dp16);

		if (derr.IsntError())
		{
			mFat.fdActiveFat	= 0;					// always 0
			mFat.fdFatCount		= dp16.dpFats;			// number of FATs
			mFat.fdReservedSecs	= dp16.dpResSectors;	// reserved sectors
			mFat.fdRootDirEnts	= dp16.dpRootDirEnts;	// root dir entries
			mFat.fdSpc			= dp16.dpSecPerClust;	// sectors per cluster
			mFat.fdFatSize		= dp16.dpFatSecs;		// sectors per FAT
			mFat.fdFirstSecFat	= dp16.dpResSectors;	// FATs begin > rsrvd

			CalcFatDataSec(&mFat);
		}
		break;
	}

	case kFS_FAT32:
	{
		DevParams32 dp32;
		PGPUInt8	activeFat;
		PGPUInt32	flags;

		derr = mHost.GetDevParams32(&dp32);

		if (derr.IsntError())
		{
			// Check for mirrored FAT tables in FAT32.
			flags = dp32.dpExtFlags;
			if (flags & BGBPB_F_NoFATMirror)
				activeFat = 0;
			else
				activeFat = (PGPUInt8) (flags & BGBPB_F_ActiveFATMsk);
		
			mFat.fdActiveFat	= activeFat;				// # of active FAT
			mFat.fdFatCount		= dp32.dpNumberOfFats;		// number of FATs
			mFat.fdReservedSecs	= dp32.dpReservedSectors;	// reserved seecs
			mFat.fdSpc			= dp32.dpSectorsPerClust;	// sects per clust
			mFat.fdFatSize		= dp32.dpBigSectorsPerFat;	// secs per FAT
			mFat.fdFirstSecFat	= dp32.dpReservedSectors;	// FATs > reserved

			CalcFatDataSec(&mFat);
		}
		break;
	}

	default:
		pgpAssert(FALSE);
		break;
	}

	// Finally we make a system call to determine the index of the first
	// cluster in the FAT table for our file.

	if (derr.IsntError())
	{
	    derr = Driver->GetFirstClustFile(mPath, &mFat.fdFirstClustFile);
	}

	return derr;
}

// ParseNextCluster is a helper function for MakeFatFrags. Given a set of
// information about a file and its associated FAT table, and the index of
// a cluster in that FAT table, it reads in the next clusters and expands
// the file's fragment list as appropriate.

DualErr 
FatParser::ParseNextCluster(ParseNextClusterInfo *pPNCI)
{
	DualErr		derr;
	PGPUInt64	blockVolume;

	pgpAssertAddrValid(pPNCI, ParseNextClusterInfo);

	// Sanity checks.

	if ((pPNCI->curClust > pPNCI->totalClusts) || 
		((++pPNCI->overFlow) > kFPMaxLoops))
	{
		derr = DualErr(kPGDMinorError_CorruptFat);
	}

	// Check if we are still within the current fragment - that is, if the
	// next cluster is 1 greater than the current cluster. If not, create a
	// new file fragment.

	if (derr.IsntError())
	{
		if (pPNCI->nextClust == (pPNCI->curClust + 1))	// within fragment?
		{
			// We are not at the end of a fragment. First increment curClust
			// by 1, and expand our fragment by the size (in bytes) of 1
			// cluster.

			pPNCI->curClust++;
			mFragArray[pPNCI->i].fileEnd += pPNCI->spc * pPNCI->blockSize;
			mFragArray[pPNCI->i].diskEnd += pPNCI->spc * pPNCI->blockSize;

			// Next update the offset to point to the new current cluster's
			// data (which will hold the next cluster's index). If the offset
			// runs over our buffer size, we have to read in the next buffer.

			pPNCI->bitOffset += pPNCI->bitsClust;

			if (pPNCI->bitOffset >= 
				(PGPUInt32) kFPDataBufSize * pPNCI->blockSize * kBitsPerByte)
			{
				GetClusterInfo(pPNCI->firstSecFat, pPNCI->curClust, 
					pPNCI->bitsClust, pPNCI->blockSize, &pPNCI->clustSec, 
					&pPNCI->bitOffset);

				derr = mHost.Read(mDataBuf, pPNCI->clustSec, 
					kFPDataBufSize + 1);
			}
		}
		else
		{
			PGPUInt64 oldSector;

			// OK, we are at the end of a fragment. Set curClust to the index
			// of the new cluster (which can be any valid cluster index), and
			// expand the current fragment.

			pPNCI->curClust = pPNCI->nextClust;

			mFragArray[pPNCI->i].fileEnd += 
				(pPNCI->spc * pPNCI->blockSize) - 1;

			mFragArray[pPNCI->i].diskEnd += 
				(pPNCI->spc * pPNCI->blockSize) - 1;

			oldSector = pPNCI->clustSec;

			// Find the new FAT sector.
			GetClusterInfo(pPNCI->firstSecFat, pPNCI->curClust, 
				pPNCI->bitsClust, pPNCI->blockSize, &pPNCI->clustSec, 
				&pPNCI->bitOffset);

			// If this lies in a different sector, read it in.
			if (pPNCI->clustSec != oldSector)
			{
				derr = mHost.Read(mDataBuf, pPNCI->clustSec, 
					kFPDataBufSize + 1);
			}

			// Finally we calculate which sector on disk the new curClust
			// refers to, and then we init a new fragment

			if (derr.IsntError())
			{
				blockVolume = ClusterToSector(pPNCI->curClust, 
					pPNCI->firstSecData, pPNCI->spc, pPNCI->clustBias);

				// See if we have to allocate a new frag.
				if (pPNCI->i + 1 == mSizeFragArray)
					derr = ExpandFragArray();
			}

			// Init a new file fragment.
			if (derr.IsntError())
			{
				pPNCI->i++;
				mNumFragsInUse++;

				InitFileFrag(&mFragArray[pPNCI->i], 
					mFragArray[pPNCI->i - 1].fileEnd + 1, 
					blockVolume * pPNCI->blockSize);
			}
		}
	}

	// Extract the next cluster.
	pPNCI->nextClust = ExtractCluster(mDataBuf, pPNCI->bitOffset, 
		pPNCI->curClust, mFsId);

	return derr;
}

// MakeFATFrags constructs the file fragment array for the File being opened
// on its FAT host drive with direct disk access mode. This procedure reads
// directly from the FAT table of the host drive.

DualErr 
FatParser::MakeFatFrags()
{
	DualErr		derr;
	PGPUInt8	bitsClust, clustBias;
	PGPUInt16	blockSize, spc;
	PGPUInt32	bitOffset, clustSec, curClust, fatSize, firstSecFat;
	PGPUInt32	firstClustFile, firstSecFile, firstSecData, i, maxClust;
	PGPUInt32	minClust, totalClusts;

	pgpAssertAddrValid(mDataBuf, PGPUInt8);
	pgpAssert(mNumFragsInUse == 0);

	// First fill in some filesystem specific data.
	switch (mFsId)
	{
	case kFS_FAT12:
		bitsClust	= kBitsFat12Clust;
		clustBias	= kFat12ClustBias;
		maxClust	= kMaxFat12Clust;
		minClust	= kMinFat12Clust;
		break;

	case kFS_FAT16:
		bitsClust	= kBitsFat16Clust;
		clustBias	= kFat16ClustBias;
		maxClust	= kMaxFat16Clust;
		minClust	= kMinFat16Clust;
		break;

	case kFS_FAT32:
		bitsClust	= kBitsFat32Clust;
		clustBias	= kFat32ClustBias;
		maxClust	= kMaxFat32Clust;
		minClust	= kMinFat32Clust;
		break;

	default:
		pgpAssert(FALSE);
		break;
	}

	// Then fill in the data specific to this drive.
	blockSize		= mFat.fdBlockSize;
	fatSize			= mFat.fdFatSize;
	spc				= mFat.fdSpc;		
	firstSecData	= mFat.fdFirstSecData;
	firstClustFile	= mFat.fdFirstClustFile;
	firstSecFat		= mFat.fdFirstSecFat + mFat.fdActiveFat*fatSize;

	totalClusts = TotalClusters(fatSize, bitsClust, blockSize);

	DebugOut("PGPdisk: blockSize is %u", blockSize);
	DebugOut("PGPdisk: fatSize is %u", fatSize);
	DebugOut("PGPdisk: spc is %u", spc);
	DebugOut("PGPdisk: firstSecData is %u", firstSecData);
	DebugOut("PGPdisk: firstClustFile is %u", firstClustFile);
	DebugOut("PGPdisk: firstSecFat is %u", firstSecFat);

	pgpAssert(firstSecFat < firstSecData);
	pgpAssert(firstClustFile <= totalClusts);

	// Now we will calculate the index of the first sector on disk holding our
	// file's contents.

	firstSecFile = ClusterToSector(firstClustFile, firstSecData, spc, 
		clustBias);

	pgpAssert(firstSecFile >= firstSecData);

	// Init the first file fragment structure for the file.
	if (derr.IsntError())
	{
		i = 0;
		mNumFragsInUse = 1;

		InitFileFrag(&mFragArray[i], 0, (PGPUInt64) firstSecFile*blockSize);
	}

	// 'curClust' will keep track of the index of the current cluster we are
	// looking at. 'clustSec' and 'bitOffset' keep track of the sector in the
	// FAT table where this cluster is located, and the offset (in bits) into
	// that sector where the cluster begins.

	if (derr.IsntError())
	{
		curClust = firstClustFile;

		GetClusterInfo(firstSecFat, curClust, bitsClust, blockSize, &clustSec, 
			&bitOffset);

		// Before parsing the FAT table we read the current sector into a
		// buffer. WE MUST read in the extra sector because FAT12 clusters can
		// overrun the boundary.

		derr = mHost.Read(mDataBuf, clustSec, kFPDataBufSize + 1);
	}

	if (derr.IsntError())
	{
		ParseNextClusterInfo PNCI;

		PNCI.blockSize	= blockSize;
		PNCI.clustBias	= clustBias;
		PNCI.spc		= spc;

		PNCI.bitsClust		= bitsClust;
		PNCI.bitOffset		= bitOffset;
		PNCI.clustSec		= clustSec;
		PNCI.curClust		= curClust;
		PNCI.firstSecData	= firstSecData;
		PNCI.firstSecFat	= firstSecFat;
		PNCI.i				= i;
		PNCI.nextClust		= 
			ExtractCluster(mDataBuf, bitOffset, curClust, mFsId);
		PNCI.overFlow		= 0;
		PNCI.totalClusts	= totalClusts;

		// We now construct the file fragment list by parsing the FAT table.
		// Beginning with the first cluster of the file, we iterate over all
		// remaining clusters until finished.
		//
		// As long as the cluster two numbers, it refers to a valid cluster
		// index; else it means the file has ended.

		while (derr.IsntError() && (PNCI.nextClust >= minClust) && 
			(PNCI.nextClust <= maxClust))
		{
			derr = ParseNextCluster(&PNCI);
		}

		i = PNCI.i;
	}

	// The file has been completely parsed.
	if (derr.IsntError())
	{
		mFragArray[i].fileEnd	+= (spc*blockSize) - 1;
		mFragArray[i].diskEnd	+= (spc*blockSize) - 1;

		// Does combined size of the fragments agree with the actual file size?
		if ((mFragArray[i].fileEnd < (mBytesFile - spc*blockSize)) ||
			(mFragArray[i].fileEnd > (mBytesFile + spc*blockSize)))
		{
			derr = DualErr(kPGDMinorError_CorruptFat);
		}
	}

	if (derr.IsError())
	{
		mNumFragsInUse = 0;
	}

	return derr;
}

// ExecuteRequest executes the next 'piece' of the current request. If there
// nothing left to execute, 'doneWithRequest' is set to TRUE, otherwise FALSE.

void 
FatParser::ExecuteRequest(PGPBoolean *doneWithRequest)
{
	PGPUInt8	*buf;
	PGPUInt16	blockSize;
	PGPUInt32	i, nBytes, sizeChunk;
	PGPUInt64	blockVolume, pos;

	pgpAssertAddrValid(doneWithRequest, PGPBoolean);
	(* doneWithRequest) = FALSE;

	pgpAssert(mFatParserReq.isInUse);

	// Look for the next piece of the request and process it. Read requests
	// can consist of reading the header, the tail, or any number of pieces
	// in the middle. Write requests are similar except header and tail
	// processing must be done in two parts.
	
	switch (mFatParserReq.op)
	{

	case kFPOP_Read:
		blockSize = mHost.GetBlockSize();

		if (!mFatParserReq.read.readHeader)
		{
			// Read in the header block.

			pos = mFatParserReq.read.posHeader;
			i = FindFragmentIndex(pos);

			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);

			mHost.Read(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.read.readMiddle)
		{
			// Read in as much of the middle as we can. Leave the rest for
			// later.

			buf		= mFatParserReq.read.bufMiddle;
			pos		= mFatParserReq.read.posMiddle;
			nBytes	= mFatParserReq.read.nBytesMiddle;
			i		= FindFragmentIndex(pos);

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);
			sizeChunk = min(nBytes/blockSize, (PGPUInt32) 
				((mFragArray[i].fileEnd - pos) / blockSize) + 1);

			mFatParserReq.read.sizeCurrentRead = sizeChunk*blockSize;

			mHost.Read(buf, blockVolume, sizeChunk, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.read.readTail)
		{
			// Read in the tail block.

			pos = mFatParserReq.read.posTail;
			i = FindFragmentIndex(pos);

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);

			mHost.Read(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else
		{
			(* doneWithRequest) = TRUE;
		}
		break;

	case kFPOP_Write:
		blockSize = mHost.GetBlockSize();

		if (!mFatParserReq.write.readHeader)
		{
			// Read in the header block. This must be done because we can't
			// write in units smaller than one block. Once we've read in
			// the header, we modify it with data from the input buffer,
			// and write it out.
			
			pos = mFatParserReq.write.posHeader;
			i = FindFragmentIndex(pos);

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);

			mHost.Read(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.write.wroteHeader)
		{
			// We read in the header block, now modify it with the changed
			// data from the input buffer and write it out.
			
			buf			= mFatParserReq.write.bufHeader;
			pos			= mFatParserReq.write.posHeader;
			nBytes		= mFatParserReq.write.nBytesHeader;
			i			= FindFragmentIndex(pos);
			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);

			pgpAssertAddrValid(buf, PGPUInt8);
			pgpCopyMemory(buf, mDataBuf + pos%blockSize, nBytes);
			
			mHost.Write(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.write.wroteMiddle)
		{
			// Encrypt and write out as much of the middle as we can. Leave
			// the rest for later.

			buf			= mFatParserReq.write.bufMiddle;
			pos			= mFatParserReq.write.posMiddle;
			nBytes		= mFatParserReq.write.nBytesMiddle;
			i			= FindFragmentIndex(pos);

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);
			sizeChunk = min(nBytes/blockSize, (PGPUInt32) 
				((mFragArray[i].fileEnd - pos) / blockSize) + 1);

			mFatParserReq.write.sizeCurrentWrite = sizeChunk*blockSize;

			mHost.Write(buf, blockVolume, sizeChunk, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.write.readTail)
		{
			// Read in the tail block. This must be done because we can't
			// write in units smaller than one block. Once we've read in
			// the tail, we modify it with data from the input buffer,
			// and write it out.
			
			pos = mFatParserReq.write.posTail;
			i = FindFragmentIndex(pos);

			blockVolume = CalcDiskSector(pos, &mFragArray[i], blockSize);

			mHost.Read(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else if (!mFatParserReq.write.wroteTail)
		{
			// We read in the tail block, now modify it with the changed
			// data from the input buffer and write it out.

			buf			= mFatParserReq.write.bufTail;
			nBytes		= mFatParserReq.write.nBytesTail;
			pos			= mFatParserReq.write.posTail;
			i			= FindFragmentIndex(pos);
			blockVolume	= CalcDiskSector(pos, &mFragArray[i], blockSize);

			pgpAssertAddrValid(buf, PGPUInt8);
			pgpCopyMemory(buf, mDataBuf + pos%blockSize, nBytes);
			
			mHost.Write(mDataBuf, blockVolume, 1, &mFatParserReq.downInfo);
		}
		else
		{
			(* doneWithRequest) = TRUE;
		}
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// FatParserCallback is called as a callback by the routines who executed our
// read and write requests. We extract the address of the FatParser object in
// question and pass the callback to it.

void 
FatParser::FatParserCallback(GenericCallbackInfo *downInfo)
{
	FatParser *pFP;

	pgpAssertAddrValid(downInfo, GenericCallbackInfo);

	pFP = (FatParser *) downInfo->refData[0];
	pgpAssertAddrValid(pFP, FatParser);

	pFP->FatParserCallbackAux();
}

// FatParserCallbackAux is called by the static callback function
// 'FatParserCallback' so we don't have to type 'pFP' before every reference
// to an object member or method.

void 
FatParser::FatParserCallbackAux()
{
	DualErr		derr;
	PGPBoolean	doneWithRequest;
	PGPUInt8	*buf;
	PGPUInt16	blockSize;
	PGPUInt32	nBytes;
	PGPUInt64	pos;

	pgpAssert(mFatParserReq.isInUse);
	blockSize = mHost.GetBlockSize();	

	derr = mFatParserReq.downInfo.derr;

	if (derr.IsntError())
	{
		// Before passing the request to 'ExecuteRequest' we perform some pre-
		// processing on the data manipulated by the piece of the request that
		// was just processed.

		switch (mFatParserReq.op)
		{

		case kFPOP_Read:
			if (!mFatParserReq.read.readHeader)
			{			
				// We read in the header block. Copy just what the caller needs
				// into his buffer.
				
				mFatParserReq.read.readHeader = TRUE;

				buf		= mFatParserReq.read.bufHeader;
				pos		= mFatParserReq.read.posHeader;
				nBytes	= mFatParserReq.read.nBytesHeader;

				pgpAssertAddrValid(buf, PGPUInt8);

				pgpCopyMemory(mDataBuf + pos%blockSize, buf, nBytes);
			}
			else if (!mFatParserReq.read.readMiddle)
			{
				PGPUInt32 bytesRead;

				// We read in a piece of the middle. Copy it into the output
				// buffer, and if there is still more middle left then adjust
				// the variables.
				
				buf			= mFatParserReq.read.bufMiddle;
				pos			= mFatParserReq.read.posMiddle;
				nBytes		= mFatParserReq.read.nBytesMiddle;
				bytesRead	= mFatParserReq.read.sizeCurrentRead;

				pgpAssertAddrValid(buf, PGPUInt8);

				if (nBytes - bytesRead > 0)
				{
					mFatParserReq.read.bufMiddle		+= bytesRead;
					mFatParserReq.read.posMiddle		+= bytesRead;
					mFatParserReq.read.nBytesMiddle		-= bytesRead;
				}
				else
				{
					mFatParserReq.read.readMiddle = TRUE;
				}
			}
			else if (!mFatParserReq.read.readTail)
			{
				// We read in the tail block. Copy just what the caller needs
				// into his buffer.

				mFatParserReq.read.readTail = TRUE;

				buf = mFatParserReq.read.bufTail;
				nBytes = mFatParserReq.read.nBytesTail;

				pgpAssertAddrValid(buf, PGPUInt8);

				pgpCopyMemory(mDataBuf, buf, nBytes);
			}
			break;

		case kFPOP_Write:
			if (!mFatParserReq.write.readHeader)
			{
				// No pre-processing needed here.
				mFatParserReq.write.readHeader = TRUE;
			}
			else if (!mFatParserReq.write.wroteHeader)
			{
				// No pre-processing needed here.
				mFatParserReq.write.wroteHeader = TRUE;
			}
			else if (!mFatParserReq.write.wroteMiddle)
			{
				PGPUInt32 bytesWritten;

				// We wrote out a piece of the middle. Copy it into the output
				// buffer, and if there is still more middle left then adjust
				// the variables.
				
				buf				= mFatParserReq.write.bufMiddle;
				pos				= mFatParserReq.write.posMiddle;
				nBytes			= mFatParserReq.write.nBytesMiddle;
				bytesWritten	= mFatParserReq.write.sizeCurrentWrite;

				pgpAssertAddrValid(buf, PGPUInt8);

				if (nBytes - bytesWritten > 0)
				{
					mFatParserReq.write.bufMiddle		+= bytesWritten;
					mFatParserReq.write.posMiddle		+= bytesWritten;
					mFatParserReq.write.nBytesMiddle	-= bytesWritten;
				}
				else
				{
					mFatParserReq.write.wroteMiddle = TRUE;
				}
			}
			else if (!mFatParserReq.write.readTail)
			{
				// No pre-processing needed here.
				mFatParserReq.write.readTail = TRUE;
			}
			else if (!mFatParserReq.write.wroteTail)
			{
				// No pre-processing needed here.
				mFatParserReq.write.wroteTail = TRUE;
			}
			break;

		default:
			pgpAssert(FALSE);
			break;
		}

		// Perform further processing on this request, if needed.
		ExecuteRequest(&doneWithRequest);
	}

	// Callback if we're done or on error.
	if (doneWithRequest || derr.IsError())
	{
		ScheduleAsyncCallback(derr);
	}
}

// ScheduleAsyncCallback schedules a windows event that calls our function
// that will call the asynchronous request up.

void 
FatParser::ScheduleAsyncCallback(DualErr derr)
{
	static RestrictedEvent_THUNK callbackThunk;

	pgpAssertAddrValid(mFatParserReq.upInfo, GenericCallbackInfo);
	mFatParserReq.upInfo->derr = derr;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) this, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID 
__stdcall 
FatParser::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	FatParser *pFP;

	pFP = (FatParser *) Refdata;
	pgpAssertAddrValid(pFP, FatParser);

	pgpAssert(pFP->mFatParserReq.isInUse);
	pFP->mFatParserReq.isInUse = FALSE;

	pgpAssertAddrValid(pFP->mFatParserReq.upInfo, GenericCallbackInfo);
	pFP->mFatParserReq.upInfo->callback(pFP->mFatParserReq.upInfo);
}
