//////////////////////////////////////////////////////////////////////////////
// PGPdisk.cpp
//
// Implementation of class PGPdisk.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.cpp,v 1.9 1999/02/13 04:24:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "UtilityFunctions.h"

#include "Globals.h"
#include "PGPdisk.h"
#include "PlacementNew.h"


////////////////////////////////////////
// Class PGPdisk public member functions
////////////////////////////////////////

// The PGPdisk default constructor.

PGPdisk::PGPdisk() : VolFile(), mSmContextA(sizeof(CipherContext)), 
	mSmContextB(sizeof(CipherContext))
{
	mContextA	= NULL;
	mContextB	= NULL;

	mDataBuf = NULL;

	mPGPdiskReq.isInUse = FALSE;

	mInitErr = VolFile::mInitErr;

	if (mInitErr.IsntError())
	{
		mInitErr = mSmContextA.mInitErr;
	}

	if (mInitErr.IsntError())
	{
		mInitErr = mSmContextB.mInitErr;
	}

	// Get our data buffer.
	if (mInitErr.IsntError())
	{
		mInitErr = GetByteBuffer(kPGDBlocksPerOp*kDefaultBlockSize, 
			&mDataBuf);
	}

	if (mInitErr.IsntError())
	{
		// Use placement new.
		mContextA	= new (mSmContextA.GetPtr()) CipherContext;	
		mContextB	= new (mSmContextB.GetPtr()) CipherContext;
	}
}

// The PGPdisk destructor cleans up.

PGPdisk::~PGPdisk()
{
	if (Mounted())
		Unmount(TRUE);

	if (IsntNull(mContextB))
	{
		mContextB->~CipherContext();
		mContextB = NULL;
	}

	if (IsntNull(mContextA))
	{
		mContextA->~CipherContext();
		mContextA = NULL;
	}

	if (IsntNull(mDataBuf))
	{
		FreeByteBuffer(mDataBuf);
		mDataBuf = NULL;
	}
}

// BestGuessComparePaths uses a number of methods to determine if the
// specified file is the same as the current mounted PGPdisk.

PGPBoolean 
PGPdisk::BestGuessComparePaths(LPCSTR path)
{
	File		testFile;
	DualErr		derr;
	PGPBoolean	failedTest	= FALSE;
	PGPUInt32	i, numPGPdiskPathElems, numTestFilePathElems;
	PGPUInt64	PGPdiskLength, testFileLength;

	pgpAssert(Mounted());

	// First try the easy case.
	if (ComparePaths(path))
		return TRUE;

	// See if the number of path elements in each path is the same.
	numPGPdiskPathElems = 0;

	for (i=0; i<strlen(GetPath()); i++)
	{
		if ((GetPath()[i] == '\\') || (GetPath()[i] == '/'))
			numPGPdiskPathElems++;
	}

	numTestFilePathElems = 0;

	for (i=0; i<strlen(path); i++)
	{
		if ((path[i] == '\\') || (path[i] == '/'))
			numTestFilePathElems++;
	}

	if (numPGPdiskPathElems != numTestFilePathElems)
		failedTest = TRUE;

	// Make sure the files are the same size.
	if (!failedTest)
	{
		derr = GetLength(&PGPdiskLength);
	}

	if (derr.IsntError() && !failedTest)
	{
		derr = testFile.Open(path, kOF_MustExist | kOF_ReadOnly);
	}

	if (derr.IsntError() && !failedTest)
	{
		derr = testFile.GetLength(&testFileLength);
	}

	if (!failedTest && testFile.Opened())
	{
		testFile.Close();
	}

	if (derr.IsntError() && !failedTest)
	{
		if (PGPdiskLength != testFileLength)
			failedTest = TRUE;
	}

	// Check if the test file is in use (which must be the case with any
	// mounted PGPdisk.

	if (derr.IsntError() && !failedTest)
	{
		if (!Driver->IsFileInUse(path))
			failedTest = TRUE;
	}

	// Finally return false if there was an error.
	if (derr.IsError())
	{
		failedTest = TRUE;
	}

	return !failedTest;
}

// GetUniqueSessionId returns the unique session ID for the PGPdisk.

PGPUInt64 
PGPdisk::GetUniqueSessionId()
{
	pgpAssert(Mounted());

	return mUniqueSessionId;
}

// FlipContexts flips the key bits on each context.

void 
PGPdisk::FlipContexts()
{
	pgpAssertAddrValid(mContextA, CipherContext);
	pgpAssertAddrValid(mContextB, CipherContext);
	pgpAssert(Mounted());

	mContextA->FlipContext();
	mContextB->FlipContext();
}

// ValidateContexts makes sure the cipher procs aren't damaged.

DualErr 
PGPdisk::ValidateContexts()
{
	DualErr derr;

	pgpAssertAddrValid(mContextA, CipherContext);
	pgpAssertAddrValid(mContextB, CipherContext);
	pgpAssert(Mounted());

	derr = mContextA->ValidateContext();

	if (derr.IsntError())
	{
		derr = mContextB->ValidateContext();
	}

	return derr;
}

// Mount reads in the PGPdisk-specific header and asks VolFile to continue
// with the mount, passing along header and disk size information.

DualErr 
PGPdisk::Mount(
	LPCSTR			path, 
	CipherContext	*context, 
	PDCB			pDcb, 
	PGPUInt8		drive, 
	PGPBoolean		mountReadOnly)
{
	DualErr				derr;
	File				diskFile;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(context, CipherContext);

	// Mark the PGPdisk as mounted.
	if (!mountReadOnly)
		SetPGPdiskMountedFlag(path, TRUE);

	// Read the session ID.
	mUniqueSessionId = GetPGPdiskUniqueSessionId(path);

	// Read in the header.
	derr = ReadPGPdiskFileMainHeader(path, &fileHeader);
	readHeader = derr.IsntError();

	// Initialize the contexts and flip context B.
	if (derr.IsntError())
	{
		pgpAssertAddrValid(mContextA, CipherContext);
		pgpAssertAddrValid(mContextB, CipherContext);

		(* mContextA) = (* mContextB) = (* context);

		mContextB->FlipContext();
	}

	// Finish the mount by calling down.
	if (derr.IsntError())
	{
		derr = VolFile::Mount(path, fileHeader->numHeaderBlocks, 
			fileHeader->numDataBlocks, pDcb, drive, mountReadOnly);
	}

	if (derr.IsError())
	{
		if (!mountReadOnly)
			SetPGPdiskMountedFlag(path, FALSE);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) fileHeader);

	return derr;
}

// Unmount unmounts a mounted PGPdisk.

DualErr 
PGPdisk::Unmount(PGPBoolean isThisEmergency)
{
	DualErr		derr;
	LPSTR		pathCopy;
	PGPBoolean	dupedPath, wasReadOnly;

	pgpAssert(Mounted());

	dupedPath = FALSE;
	wasReadOnly = IsReadOnly();

	// Copy path.
	derr = DupeString(GetPath(), &pathCopy);
	dupedPath = derr.IsntError();

	// Unmount the PGPdisk volume.
	if (derr.IsntError())
	{
		derr = VolFile::Unmount(isThisEmergency);
	}

	if (derr.IsntError())
	{
		// Mark as not mounted.
		if (!wasReadOnly)
			SetPGPdiskMountedFlag(pathCopy, FALSE);

		// Unflip the contexts and destroy them.
		(mContextA->IsFlipped() ? mContextA->FlipContext() : 
			mContextB->FlipContext());

		mContextA->DestroyContext();
		mContextB->DestroyContext();
	}

	if (dupedPath)
		delete[] pathCopy;

	return derr;
}

// ReadAsync performs a read request on the mounted PGPdisk asynchronously.

void 
PGPdisk::ReadAsync(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	PGPBoolean doneWithRequest;

	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	pgpAssertAddrValid(upInfo->callback, PGPUInt8);
	pgpAssertAddrValid(buf, PGPUInt8);

	pgpAssert(Mounted());

	pgpAssert(!mPGPdiskReq.isInUse);
	mPGPdiskReq.isInUse = TRUE;

	// Prepare the request for execution.
	mPGPdiskReq.op = kPGDOP_Read;
	mPGPdiskReq.upInfo = upInfo;

	mPGPdiskReq.downInfo.callback = PGPdiskCallback;
	mPGPdiskReq.downInfo.refData[0] = (PGPUInt32) this;

	// We start out by initializing the request so that all possible
	// pieces of the request are marked as completed.

	mPGPdiskReq.read.readHeader	= TRUE;
	mPGPdiskReq.read.readMiddle	= TRUE;
	mPGPdiskReq.read.readTail	= TRUE;

	// Now we analyze the request and determine what pieces must be
	// read so that the entire request can be marked as done.

	if (pos%kDefaultBlockSize > 0)
	{
		PGPUInt32 bytesInHeader;

		// The request has a header - that is, it doesn't begin on a
		// block boundary.

		bytesInHeader = min(nBytes, kDefaultBlockSize - 
			(PGPUInt32) (pos%kDefaultBlockSize));

		mPGPdiskReq.read.readHeader		= FALSE;
		mPGPdiskReq.read.bufHeader		= buf;
		mPGPdiskReq.read.posHeader		= pos;
		mPGPdiskReq.read.nBytesHeader	= bytesInHeader;

		buf		+= bytesInHeader;
		pos		+= bytesInHeader;
		nBytes	-= bytesInHeader;
	}

	if (nBytes >= kDefaultBlockSize)
	{
		PGPUInt32 bytesInMiddle;

		// The request has a middle - that is, a section that begins and
		// ends on a block boundary and is more than one block in length.

		bytesInMiddle = nBytes - nBytes%kDefaultBlockSize;

		mPGPdiskReq.read.readMiddle		= FALSE;
		mPGPdiskReq.read.bufMiddle		= buf;
		mPGPdiskReq.read.posMiddle		= pos;
		mPGPdiskReq.read.nBytesMiddle	= bytesInMiddle;

		buf		+= bytesInMiddle;
		pos		+= bytesInMiddle;
		nBytes	-= bytesInMiddle;
	}

	if (nBytes > 0)
	{
		// The request has a tail - that is, it doesn't end on a
		// block boundary.

		mPGPdiskReq.read.readTail	= FALSE;
		mPGPdiskReq.read.bufTail	= buf;
		mPGPdiskReq.read.posTail	= pos;
		mPGPdiskReq.read.nBytesTail	= nBytes;
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

// WriteAsync performs a write request on the mounted PGPdisk asynchronously.

void 
PGPdisk::WriteAsync(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	PGPBoolean doneWithRequest;

	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	pgpAssertAddrValid(upInfo->callback, PGPUInt8);
	pgpAssertAddrValid(buf, PGPUInt8);

	pgpAssert(Mounted());

	pgpAssert(!mPGPdiskReq.isInUse);
	mPGPdiskReq.isInUse = TRUE;

	// Prepare the request for execution.
	mPGPdiskReq.op = kPGDOP_Write;
	mPGPdiskReq.upInfo = upInfo;

	mPGPdiskReq.downInfo.callback = PGPdiskCallback;
	mPGPdiskReq.downInfo.refData[0] = (PGPUInt32) this;

	// We start out by initializing the request so that all possible
	// pieces of the request are marked as completed.

	mPGPdiskReq.write.readHeader	= TRUE;
	mPGPdiskReq.write.wroteHeader	= TRUE;
	mPGPdiskReq.write.wroteMiddle	= TRUE;
	mPGPdiskReq.write.readTail		= TRUE;
	mPGPdiskReq.write.wroteTail		= TRUE;

	// Now we analyze the request and determine what pieces must be
	// written so that the entire request can be marked as done.

	if (pos%kDefaultBlockSize > 0)
	{
		PGPUInt32 bytesInHeader;

		// The request has a header - that is, it doesn't begin on a
		// block boundary.

		bytesInHeader = min(nBytes, kDefaultBlockSize - 
			(PGPUInt32) (pos%kDefaultBlockSize));

		mPGPdiskReq.write.readHeader	= FALSE;
		mPGPdiskReq.write.wroteHeader	= FALSE;
		mPGPdiskReq.write.bufHeader		= buf;
		mPGPdiskReq.write.posHeader		= pos;
		mPGPdiskReq.write.nBytesHeader	= bytesInHeader;

		buf		+= bytesInHeader;
		pos		+= bytesInHeader;
		nBytes	-= bytesInHeader;
	}

	if (nBytes >= kDefaultBlockSize)
	{
		PGPUInt32 bytesInMiddle;

		// The request has a middle - that is, a section that begins and
		// ends on a block boundary and is more than one block in length.

		bytesInMiddle = nBytes - nBytes%kDefaultBlockSize;

		mPGPdiskReq.write.wroteMiddle	= FALSE;
		mPGPdiskReq.write.bufMiddle		= buf;
		mPGPdiskReq.write.posMiddle		= pos;
		mPGPdiskReq.write.nBytesMiddle	= bytesInMiddle;

		buf		+= bytesInMiddle;
		pos		+= bytesInMiddle;
		nBytes	-= bytesInMiddle;
	}

	if (nBytes > 0)
	{
		// The request has a tail - that is, it doesn't end on a
		// block boundary.

		mPGPdiskReq.write.readTail		= FALSE;
		mPGPdiskReq.write.wroteTail		= FALSE;
		mPGPdiskReq.write.bufTail		= buf;
		mPGPdiskReq.write.posTail		= pos;
		mPGPdiskReq.write.nBytesTail	= nBytes;
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

// ReadSync performs a read request on the mounted PGPdisk synchronously.

DualErr 
PGPdisk::ReadSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	blocksChunk, blocksToRead, bufPos;
	PGPUInt64	blockPGPdisk;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	// The position (pos) and size (nBytes) variables are in bytes, but we
	// can only perform encryption in blocks. What we have to do is handle
	// the request in at most three separate pieces - a head, middle, and
	// end.

	bufPos = 0;

	if (pos%kDefaultBlockSize > 0)	// is there a head?
	{
		PGPUInt32 headOffset, headSize;

		// Now read in the head.
		//
		// blockPGPdisk = block on PGPdisk containing the head.
		// headOffset = byte offset into blockPGPdisk where head starts
		// headSize = size of the head, in bytes

		blockPGPdisk	= pos/kDefaultBlockSize;
		headOffset		= (PGPUInt32) (pos%kDefaultBlockSize);
		headSize		= min(nBytes, kDefaultBlockSize - headOffset);

		derr = VolFile::Read(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
			kDefaultBlockSize);

		if (derr.IsntError())
		{
			SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
				kCipherOp_Decrypt);

			pgpCopyMemory(mDataBuf + headOffset, buf, headSize);

			bufPos	= headSize;
			pos		+= headSize;
			nBytes	-= headSize;
		}
	}

	// Read in the middle, in 'kPGDBlocksPerOp' chunks.
	if (derr.IsntError())
	{
		blocksToRead = nBytes/kDefaultBlockSize;

		while (derr.IsntError() && (blocksToRead > 0))	// is there a middle?
		{
			// Determine exactly how many blocks to read this iteration.
			//
			// blockPGPdisk = block on the PGPdisk where this read begins
			// blocksChunk = the most blocks we can read in this iteration

			blockPGPdisk	= pos/kDefaultBlockSize;
			blocksChunk		= min(blocksToRead, kPGDBlocksPerOp);

			derr = VolFile::Read(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
				blocksChunk*kDefaultBlockSize);

			if (derr.IsntError())
			{
				SmartCipherBlocks(blockPGPdisk, blocksChunk, mDataBuf, 
					buf + bufPos, kCipherOp_Decrypt);

				bufPos			+= blocksChunk * kDefaultBlockSize;
				nBytes			-= blocksChunk * kDefaultBlockSize;
				pos				+= blocksChunk * kDefaultBlockSize;
				blocksToRead	-= blocksChunk;
			}
		}
	}
	
	// Read in the tail.
	if (derr.IsntError())
	{
		if (nBytes)					// is there a tail?	
		{
			// This read is very simple because we know the tail begins on a
			// block boundary.
			//
			// blockPGPdisk = the block in the PGPdisk containing the tail.

			blockPGPdisk = pos/kDefaultBlockSize;

			derr = VolFile::Read(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
				kDefaultBlockSize);

			if (derr.IsntError())
			{
				SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
					kCipherOp_Decrypt);

				pgpCopyMemory(mDataBuf, buf + bufPos, nBytes);
			}
		}
	}

	return derr;
}

// WriteSync performs a write request on the mounted PGPdisk synchronously.

DualErr 
PGPdisk::WriteSync(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	blocksChunk, blocksToWrite, bufPos;
	PGPUInt64	blockPGPdisk;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	// The position (pos) and size (nBytes) variables are in bytes, but we
	// can only perform encryption in blocks. What we have to do is handle
	// the request in at most three separate pieces - a head, middle, and
	// end.

	bufPos = 0;

	if (pos%kDefaultBlockSize > 0)	// is there a head?
	{
		PGPUInt32 headOffset, headSize;

		// Now write the head.
		//
		// blockPGPdisk = block on PGPdisk containing the head.
		// headOffset = byte offset into blockPGPdisk where head starts
		// headSize = size of the head, in bytes

		blockPGPdisk	= pos/kDefaultBlockSize;
		headOffset		= (PGPUInt32) (pos%kDefaultBlockSize);
		headSize		= min(nBytes, kDefaultBlockSize - headOffset);

		derr = VolFile::Read(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
			kDefaultBlockSize);

		if (derr.IsntError())
		{
			SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
				kCipherOp_Decrypt);

			pgpCopyMemory(buf, mDataBuf + headOffset, headSize);

			SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
				kCipherOp_Encrypt);

			derr = VolFile::Write(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
				kDefaultBlockSize);
		}

		if (derr.IsntError())
		{
			bufPos	= headSize;
			pos		+= headSize;
			nBytes	-= headSize;
		}
	}

	// Write the middle, in 'kPGDBlocksPerOp' chunks.
	if (derr.IsntError())
	{
		blocksToWrite = nBytes/kDefaultBlockSize;

		while (derr.IsntError() && (blocksToWrite > 0))	// is there a middle?
		{
			// Determine exactly how many blocks to write this iteration.
			//
			// blockPGPdisk = block on the PGPdisk where this write begins
			// blocksChunk = the most blocks we can write in this iteration

			blockPGPdisk	= pos/kDefaultBlockSize;
			blocksChunk		= min(blocksToWrite, kPGDBlocksPerOp);

			SmartCipherBlocks(blockPGPdisk, blocksChunk, buf + bufPos, 
				mDataBuf, kCipherOp_Encrypt);

			derr = VolFile::Write(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
				blocksChunk*kDefaultBlockSize);

			if (derr.IsntError())
			{
				bufPos			+= blocksChunk * kDefaultBlockSize;
				nBytes			-= blocksChunk * kDefaultBlockSize;
				pos				+= blocksChunk * kDefaultBlockSize;
				blocksToWrite	-= blocksChunk;
			}
		}
	}
	
	// Write in the tail.
	if (derr.IsntError())
	{
		if (nBytes)					// is there a tail?	
		{
			// This read is very simple because we know the tail begins on a
			// block boundary.
			//
			// blockPGPdisk = the block in the PGPdisk containing the tail.

			blockPGPdisk = pos/kDefaultBlockSize;

			derr = VolFile::Read(mDataBuf, blockPGPdisk*kDefaultBlockSize, 
				kDefaultBlockSize);

			if (derr.IsntError())
			{
				SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
					kCipherOp_Decrypt);

				pgpCopyMemory(buf, mDataBuf, nBytes);

				SmartCipherBlocks(blockPGPdisk, 1, mDataBuf, mDataBuf, 
					kCipherOp_Encrypt);

				derr = VolFile::Write(mDataBuf, 
					blockPGPdisk*kDefaultBlockSize, kDefaultBlockSize);
			}
		}
	}

	return derr;
}

// Read performs a read request on the mounted PGPdisk.

	DualErr 
PGPdisk::Read(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	DebugOut("PGPdisk: Reading PGPdisk %u at pos %u nBlocks %u", GetDrive(), 
		(PGPUInt32) (pos/kDefaultBlockSize), 
		(PGPUInt32) (nBytes/kDefaultBlockSize));

	if (upInfo)
		ReadAsync(buf, pos, nBytes, upInfo);
	else
		derr = ReadSync(buf, pos, nBytes);

	return derr;
}

// Write performs a write request on the mounted PGPdisk.

DualErr 
PGPdisk::Write(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	DebugOut("PGPdisk: Writing PGPdisk %u at pos %u nBlocks %u", GetDrive(), 
		(PGPUInt32) (pos/kDefaultBlockSize), 
		(PGPUInt32) (nBytes/kDefaultBlockSize));

	if (upInfo)
		WriteAsync(buf, pos, nBytes, upInfo);
	else
		derr = WriteSync(buf, pos, nBytes);

	return derr;
}


/////////////////////////////////////////
// Class PGPdisk private member functions
/////////////////////////////////////////

// SmartCipherBlocks performs the given cipher operation using whichever
// cipher context isn't flipped.

void 
PGPdisk::SmartCipherBlocks(
	PGPUInt64	startBlockIndex, 
	PGPUInt32	nBlocks, 
	const void	*src, 
	void		*dest, 
	CipherOp	op)
{
	pgpAssertAddrValid(mContextA, CipherContext);
	pgpAssertAddrValid(mContextB, CipherContext);
			
	if (mContextA->IsFlipped())
		mContextB->CipherBlocks(startBlockIndex, nBlocks, src, dest, op);
	else
		mContextA->CipherBlocks(startBlockIndex, nBlocks, src, dest, op);
}

// ExecuteRequest executes the next 'piece' of the current request. If there
// nothing left to execute, 'doneWithRequest' is set to TRUE, otherwise FALSE.

void 
PGPdisk::ExecuteRequest(PGPBoolean *doneWithRequest)
{
	PGPUInt8	*buf;
	PGPUInt32	nBytes;
	PGPUInt64	pos;

	pgpAssertAddrValid(doneWithRequest, PGPBoolean);
	(* doneWithRequest) = FALSE;

	pgpAssert(mPGPdiskReq.isInUse);

	// Look for the next piece of the request and process it. Read requests
	// can consist of reading the header, the tail, or any number of pieces
	// in the middle. Write requests are similar except header and tail
	// processing must be done in two parts.

	switch (mPGPdiskReq.op)
	{

	case kPGDOP_Read:
		if (!mPGPdiskReq.read.readHeader)
		{
			// Read in the header block.

			pos = mPGPdiskReq.read.posHeader;

			VolFile::Read(mDataBuf, pos - pos%kDefaultBlockSize, 
				kDefaultBlockSize, &mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.read.readMiddle)
		{
			PGPUInt32 bytesToRead;

			// Read in as much of the middle as we can. Leave the rest for
			// later.

			pos			= mPGPdiskReq.read.posMiddle;
			nBytes		= mPGPdiskReq.read.nBytesMiddle;
			bytesToRead	= min(nBytes, kPGDBlocksPerOp*kDefaultBlockSize);

			VolFile::Read(mDataBuf, pos, bytesToRead, &mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.read.readTail)
		{
			// Read in the tail block.
			pos = mPGPdiskReq.read.posTail;

			VolFile::Read(mDataBuf, pos, kDefaultBlockSize, 
				&mPGPdiskReq.downInfo);
		}
		else
		{
			(* doneWithRequest) = TRUE;
		}
		break;

	case kPGDOP_Write:
		if (!mPGPdiskReq.write.readHeader)
		{
			// Read in the header block. This must be done because we can't
			// encrypt in units smaller than one block. Once we've read in
			// the header, we modify it with data from the input buffer,
			// encrypt the whole block, and write it out.

			pos = mPGPdiskReq.write.posHeader;

			VolFile::Read(mDataBuf, pos - pos%kDefaultBlockSize, 
				kDefaultBlockSize, &mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.write.wroteHeader)
		{
			// We read in the header block, now modify it with the changed
			// data from the input buffer, encrypt it, and write it out.

			buf		= mPGPdiskReq.write.bufHeader;
			pos		= mPGPdiskReq.write.posHeader;
			nBytes	= mPGPdiskReq.write.nBytesHeader;

			pgpAssertAddrValid(buf, PGPUInt8);
			pgpCopyMemory(buf, mDataBuf + pos%kDefaultBlockSize, nBytes);

			SmartCipherBlocks(pos/kDefaultBlockSize, 1, mDataBuf, mDataBuf, 
				kCipherOp_Encrypt);

			VolFile::Write(mDataBuf, pos - pos%kDefaultBlockSize, 
				kDefaultBlockSize, &mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.write.wroteMiddle)
		{
			PGPUInt32 bytesToWrite;

			// Encrypt and write out as much of the middle as we can. Leave
			// the rest for later.

			buf		= mPGPdiskReq.write.bufMiddle;
			pos		= mPGPdiskReq.write.posMiddle;
			nBytes	= mPGPdiskReq.write.nBytesMiddle;

			bytesToWrite = min(nBytes, kPGDBlocksPerOp*kDefaultBlockSize);

			SmartCipherBlocks(pos/kDefaultBlockSize, 
				bytesToWrite/kDefaultBlockSize, buf, mDataBuf, 
				kCipherOp_Encrypt);

			VolFile::Write(mDataBuf, pos, bytesToWrite, 
				&mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.write.readTail)
		{
			// Read in the tail block. This must be done because we can't
			// encrypt in units smaller than one block. Once we've read in
			// the tail, we modify it with data from the input buffer,
			// encrypt the whole block, and write it out.

			pos = mPGPdiskReq.write.posTail;

			VolFile::Read(mDataBuf, pos - pos%kDefaultBlockSize, 
				kDefaultBlockSize, &mPGPdiskReq.downInfo);
		}
		else if (!mPGPdiskReq.write.wroteTail)
		{
			// We read in the tail block, now modify it with the changed
			// data from the input buffer, encrypt it, and write it out.

			buf		= mPGPdiskReq.write.bufTail;
			pos		= mPGPdiskReq.write.posTail;
			nBytes	= mPGPdiskReq.write.nBytesTail;

			pgpAssertAddrValid(buf, PGPUInt8);
			pgpCopyMemory(buf, mDataBuf, nBytes);

			SmartCipherBlocks(pos/kDefaultBlockSize, 1, mDataBuf, mDataBuf, 
				kCipherOp_Encrypt);

			VolFile::Write(mDataBuf, pos, kDefaultBlockSize, 
				&mPGPdiskReq.downInfo);
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

// PGPdiskCallback is called as a callback by the routines who executed our
// read and write requests. We extract the address of the PGPdisk object in
// question and pass the callback to it.

void 
PGPdisk::PGPdiskCallback(GenericCallbackInfo *downInfo)
{
	PGPdisk *pPGD;

	pgpAssertAddrValid(downInfo, GenericCallbackInfo);

	pPGD = (PGPdisk *) downInfo->refData[0];
	pgpAssertAddrValid(pPGD, PGPdisk);

	pPGD->PGPdiskCallbackAux();
}

// PGPdiskCallbackAux is called by the static callback function
// 'PGPdiskCallback' so we don't have to type 'pPGD' before every reference
// to an object member or method.

void 
PGPdisk::PGPdiskCallbackAux()
{
	DualErr		derr;
	PGPBoolean	doneWithRequest;
	PGPUInt8	*buf;
	PGPUInt32	nBytes;
	PGPUInt64	pos;

	pgpAssert(mPGPdiskReq.isInUse);

	derr = mPGPdiskReq.downInfo.derr;

	if (derr.IsntError())
	{
		// Before passing the request to 'ExecuteRequest' we perform some pre-
		// processing on the data manipulated by the piece of the request that
		// was just processed.

		switch (mPGPdiskReq.op)
		{

		case kPGDOP_Read:
			if (!mPGPdiskReq.read.readHeader)
			{
				// We read in the header block. Decrypt and copy just what the
				// caller needs into his buffer.

				mPGPdiskReq.read.readHeader = TRUE;

				buf		= mPGPdiskReq.read.bufHeader;
				pos		= mPGPdiskReq.read.posHeader;
				nBytes	= mPGPdiskReq.read.nBytesHeader;

				pgpAssertAddrValid(buf, PGPUInt8);

				SmartCipherBlocks(pos/kDefaultBlockSize, 1, mDataBuf, 
					mDataBuf, kCipherOp_Decrypt);

				pgpCopyMemory(mDataBuf + pos%kDefaultBlockSize, buf, nBytes);
			}
			else if (!mPGPdiskReq.read.readMiddle)
			{
				PGPUInt32 bytesRead;

				// We read in a piece of the middle. Decrypt it, copy it into
				// the output buffer, and if there is still more middle left
				// then adjust the variables.

				buf		= mPGPdiskReq.read.bufMiddle;
				pos		= mPGPdiskReq.read.posMiddle;
				nBytes	= mPGPdiskReq.read.nBytesMiddle;

				pgpAssertAddrValid(buf, PGPUInt8);

				bytesRead = min(nBytes, kPGDBlocksPerOp*kDefaultBlockSize);

				SmartCipherBlocks(pos/kDefaultBlockSize, 
					bytesRead/kDefaultBlockSize, mDataBuf, buf, 
					kCipherOp_Decrypt);

				if (nBytes - bytesRead > 0)
				{
					mPGPdiskReq.read.bufMiddle		+= bytesRead;
					mPGPdiskReq.read.posMiddle		+= bytesRead;
					mPGPdiskReq.read.nBytesMiddle	-= bytesRead;
				}
				else
				{
					mPGPdiskReq.read.readMiddle = TRUE;
				}
			}
			else if (!mPGPdiskReq.read.readTail)
			{
				// We read in the tail block. Decrypt and copy just what the
				// caller needs into his buffer.

				mPGPdiskReq.read.readTail = TRUE;

				buf		= mPGPdiskReq.read.bufTail;
				pos		= mPGPdiskReq.read.posTail;
				nBytes	= mPGPdiskReq.read.nBytesTail;

				pgpAssertAddrValid(buf, PGPUInt8);

				SmartCipherBlocks(pos/kDefaultBlockSize, 1, mDataBuf, 
					mDataBuf, kCipherOp_Decrypt);

				pgpCopyMemory(mDataBuf, buf, nBytes);
			}
			break;

		case kPGDOP_Write:
			if (!mPGPdiskReq.write.readHeader)
			{
				// No pre-processing needed here.
				mPGPdiskReq.write.readHeader = TRUE;
			}
			else if (!mPGPdiskReq.write.wroteHeader)
			{
				// No pre-processing needed here.
				mPGPdiskReq.write.wroteHeader = TRUE;
			}
			else if (!mPGPdiskReq.write.wroteMiddle)
			{
				PGPUInt32 bytesWritten;

				// We wrote out a piece of the middle. If there is still more
				// middle left to write out then adjust the variables.

				buf		= mPGPdiskReq.write.bufMiddle;
				pos		= mPGPdiskReq.write.posMiddle;
				nBytes	= mPGPdiskReq.write.nBytesMiddle;

				pgpAssertAddrValid(buf, PGPUInt8);

				bytesWritten = min(nBytes, kPGDBlocksPerOp*kDefaultBlockSize);

				if (nBytes - bytesWritten > 0)
				{
					mPGPdiskReq.write.bufMiddle		+= bytesWritten;
					mPGPdiskReq.write.posMiddle		+= bytesWritten;
					mPGPdiskReq.write.nBytesMiddle	-= bytesWritten;
				}
				else
				{
					mPGPdiskReq.write.wroteMiddle = TRUE;
				}
			}
			else if (!mPGPdiskReq.write.readTail)
			{
				// No pre-processing needed here.
				mPGPdiskReq.write.readTail = TRUE;
			}
			else if (!mPGPdiskReq.write.wroteTail)
			{
				// No pre-processing needed here.
				mPGPdiskReq.write.wroteTail = TRUE;
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
PGPdisk::ScheduleAsyncCallback(DualErr derr)
{
	static RestrictedEvent_THUNK callbackThunk;

	pgpAssertAddrValid(mPGPdiskReq.upInfo, GenericCallbackInfo);
	mPGPdiskReq.upInfo->derr = derr;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) this, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID 
__stdcall 
PGPdisk::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	PGPdisk *pPGD;

	pPGD = (PGPdisk *) Refdata;
	pgpAssertAddrValid(pPGD, PGPdisk);

	pgpAssert(pPGD->mPGPdiskReq.isInUse);
	pPGD->mPGPdiskReq.isInUse = FALSE;

	pgpAssertAddrValid(pPGD->mPGPdiskReq.upInfo, GenericCallbackInfo);
	pPGD->mPGPdiskReq.upInfo->callback(pPGD->mPGPdiskReq.upInfo);
}
