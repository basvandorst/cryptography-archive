//////////////////////////////////////////////////////////////////////////////
// PGPdisk.cpp
//
// Implementation of class PGPdisk.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.cpp,v 1.1.4.29 1998/07/30 00:50:24 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "UtilityFunctions.h"

#include "Globals.h"
#include "KernelModeUtils.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

static LPCSTR kPGPdiskDeviceName = "PGPdiskVolume";

const PGPUInt32 kPGDBlocksPerOp = 32;	// cipher this many blocks at a time


////////////////////////////////////////
// Class PGPdisk public member functions
////////////////////////////////////////

// The PGPdisk default constructor.

PGPdisk::PGPdisk() : VolFile(), mContextMutex(0), 
	mSmContextA(sizeof(CipherContext)), mSmContextB(sizeof(CipherContext))
{
	mDevExtInfo = (PGPUInt32) this;

	mContextA	= NULL;
	mContextB	= NULL;

	mInitErr = mWorkerThread.mInitErr;

	if (mInitErr.IsntError())
	{
		mInitErr = mSmContextA.mInitErr;
	}

	if (mInitErr.IsntError())
	{
		mInitErr = mSmContextB.mInitErr;
	}

	if (mInitErr.IsntError())
	{
		PGPUInt32 sizeBuf;

		// Use placement new.
		mContextA	= new (mSmContextA.GetPtr()) CipherContext;	
		mContextB	= new (mSmContextB.GetPtr()) CipherContext;

		// Get our data buffer. MUST be PAGE_SIZE or greater so that we get
		// it page-aligned for non-buffered file access calls.

		sizeBuf = kPGDBlocksPerOp * kDefaultBlockSize;

		if (sizeBuf < PAGE_SIZE)
			sizeBuf = PAGE_SIZE;

		mInitErr = GetByteBuffer(sizeBuf, (PGPUInt8 **) &mDataBuf);
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
		if (IsFileInUse(path))
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

// GetThread returns the PGPdisk's worker thread.

PGPdiskWorkerThread * 
PGPdisk::GetThread()
{
	return &mWorkerThread;
}

// FlipContexts flips the key bits on each context.

void 
PGPdisk::FlipContexts()
{
	pgpAssertAddrValid(mContextA, CipherContext);
	pgpAssertAddrValid(mContextB, CipherContext);
	pgpAssert(Mounted());

	mContextMutex.Enter();

	mContextA->FlipContext();
	mContextB->FlipContext();

	mContextMutex.Leave();
}

// ValidateContexts makes sure the cipher procs aren't damaged.

DualErr 
PGPdisk::ValidateContexts()
{
	DualErr derr;

	pgpAssertAddrValid(mContextA, CipherContext);
	pgpAssertAddrValid(mContextB, CipherContext);
	pgpAssert(Mounted());

	mContextMutex.Enter();

	derr = mContextA->ValidateContext();

	if (derr.IsntError())
	{
		derr = mContextB->ValidateContext();
	}

	mContextMutex.Leave();

	return derr;
}

// Mount reads in the PGPdisk-specific header and asks VolFile to continue
// with the mount, passing along header and disk size information.

DualErr 
PGPdisk::Mount(
	LPCSTR			path, 
	CipherContext	*pContext, 
	PGPUInt8		drive, 
	PGPBoolean		mountReadOnly)
{
	DualErr				derr;
	File				diskFile;
	KUstring			deviceName;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*fileHeader;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pContext, CipherContext);

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

		(* mContextA) = (* mContextB) = (* pContext);

		mContextB->FlipContext();
	}

	// Start the worker thread.
	if (derr.IsntError())
	{
		derr = mWorkerThread.StartThread();
	}
	
	// Finish the mount by calling down.
	if (derr.IsntError())
	{
		derr = VolFile::Mount(path, kPGPdiskDeviceName, 
			fileHeader->numHeaderBlocks, fileHeader->numDataBlocks, drive, 
			mountReadOnly);
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

		// Stop the thread.
		mWorkerThread.KillThread();

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

// Read performs a read request on the mounted PGPdisk.

DualErr 
PGPdisk::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	blocksChunk, blocksToRead, bufPos;
	PGPUInt64	blockPGPdisk;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	DebugOut("PGPdisk: Reading PGPdisk %u at pos %u nBlocks %u\n", 
		GetDrive(), (PGPUInt32) (pos/kDefaultBlockSize), 
		(PGPUInt32) (nBytes/kDefaultBlockSize));

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

// Write performs a write request on the mounted PGPdisk.

DualErr 
PGPdisk::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	blocksChunk, blocksToWrite, bufPos;
	PGPUInt64	blockPGPdisk;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Mounted());

	DebugOut("PGPdisk: Writing PGPdisk %u at pos %u nBlocks %u\n", 
		GetDrive(), (PGPUInt32) (pos/kDefaultBlockSize), 
		(PGPUInt32) (nBytes/kDefaultBlockSize));

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
			
	mContextMutex.Enter();

	if (mContextA->IsFlipped())
		mContextB->CipherBlocks(startBlockIndex, nBlocks, src, dest, op);
	else
		mContextA->CipherBlocks(startBlockIndex, nBlocks, src, dest, op);

	mContextMutex.Leave();
}
