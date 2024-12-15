//////////////////////////////////////////////////////////////////////////////
// File.cpp
//
// Implementation of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.cpp,v 1.1.2.26 1998/08/07 01:20:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "File.h"
#include "KernelModeUtils.h"
#include "NtThreadSecurity.h"
#include "Volume.h"


/////////////////////////////////////
// Class File public member functions
/////////////////////////////////////

// The Class File default constructor.

File::File()
{
	mHandle	= NULL;
	mPath	= NULL;

	mBlockBuf = NULL;
}

// The destructor for class File will close the file.

File::~File()
{
	DualErr	derr;

	if (Opened())
	{
		derr = Close();
		pgpAssert(derr.IsntError());
	}

	ClearPath();
}

// IsHostNetworked returns TRUE if the file's host drive is remote.

PGPBoolean 
File::IsHostNetworked()
{
	pgpAssert(IsPathSet());

	return IsNetworkedPath(mPath);
}

// IsPathSet returns TRUE if a path is set, FALSE otherwise.

PGPBoolean 
File::IsPathSet()
{
	return (IsntNull(mPath));
}

// IsReadOnly returns TRUE if the file has been opened in read-only mode,
// FALSE otherwise.

PGPBoolean 
File::IsReadOnly()
{
	pgpAssert(Opened());

	return mIsReadOnly;
}

// Opened returns TRUE if the file is opened, FALSE if not.

PGPBoolean 
File::Opened()
{
	return IsntNull(mHandle);
}

// GetLength returns the length of the file in bytes.

DualErr 
File::GetLength(PGPUInt64 *pLength)
{
	DualErr						derr;
	FILE_STANDARD_INFORMATION	fileInfo;
	IO_STATUS_BLOCK				ioStatus;
	NTSTATUS					status;

	pgpAssertAddrValid(pLength, PGPUInt32);
	pgpAssert(Opened());

	status = ZwQueryInformationFile(mHandle, &ioStatus, &fileInfo, 
		sizeof(fileInfo), FileStandardInformation);

	if (!NT_SUCCESS(status))
		derr = DualErr(kPGDMinorError_ZwQueryInfoFileFailed, status);

	if (derr.IsntError())
	{
		(* pLength) = fileInfo.EndOfFile.QuadPart;
	}

	return derr;
}

// SetLength sets the length of the open file to the specified value.

DualErr 
File::SetLength(PGPInt64 length)
{
	DualErr							derr;
	FILE_END_OF_FILE_INFORMATION	eofInfo;
	IO_STATUS_BLOCK					ioStatus;
	NTSTATUS						status;

	pgpAssert(Opened());

	eofInfo.EndOfFile.QuadPart = length;

	status = ZwSetInformationFile(mHandle, &ioStatus, &eofInfo, 
		sizeof(eofInfo), FileEndOfFileInformation);

	if (!NT_SUCCESS(status))
	{
		derr = DualErr(kPGDMinorError_ZwSetInfoFileFailed, status);
	}

	return derr;
}

// GetLocalHostDrive returns the drive number of the volume that hosts this
// file, but only if it is a local host.

PGPUInt8
File::GetLocalHostDrive()
{
	pgpAssert(!IsUNCPath(mPath));

	return DriveLetToNum(mPath[0]);
}

// GetPath returns a constant pointer to the pathname string.

LPCSTR 
File::GetPath()
{
	return (LPCSTR) mPath;
}

// SetPath sets the filename path variable.

DualErr 
File::SetPath(LPCSTR path)
{
	DualErr derr;

	pgpAssert(!Opened());

	ClearPath();
	derr = DupeString(path, &mPath);

	return derr;
}

// ClearPath clears the filename path variable.

void 
File::ClearPath()
{
	pgpAssert(!Opened());

	if (mPath)
		delete[] mPath;

	mPath = NULL;
}

// ComparePaths compares the passed pathname with the path of the opened File
// object, returning TRUE if they refer to the same file.

PGPBoolean 
File::ComparePaths(LPCSTR path)
{
	pgpAssert(Opened());
	pgpAssertStrValid(mPath);
	pgpAssertStrValid(path);

	return (_stricmp(mPath, path) ? FALSE : TRUE);
}

// Open opens the file specified by 'path' with the flags specified by
// 'flags'.

DualErr 
File::Open(LPCSTR path, PGPUInt16 flags)
{
	DualErr		derr;
	KUstring	uniPath;
	NTSTATUS	status;
	PGPBoolean	allocedBlockBuf, denyRead, denyWrite, mustExist;

	allocedBlockBuf = denyRead = denyWrite = mustExist = mWeSetPath = FALSE;

	mustExist		= (flags & kOF_MustExist	? TRUE : FALSE);
	mIsReadOnly		= (flags & kOF_ReadOnly		? TRUE : FALSE);
 	denyRead		= (flags & kOF_DenyRead		? TRUE : FALSE);
	denyWrite		= (flags & kOF_DenyWrite	? TRUE : FALSE);
	mNoBuffering	= (flags & kOF_NoBuffering	? TRUE : FALSE);

	// Set the path.
	if (IsntNull(path))
	{
		pgpAssertStrValid(path);

		derr = SetPath(path);
		mWeSetPath = derr.IsntError();
	}

	// Convert path to unicode.
	if (derr.IsntError())
	{
		if (IsUNCPath(mPath))
		{
			derr = AssignToUni(&uniPath, mPath + 2);

			if (derr.IsntError())
			{
				derr = PrependToUni(&uniPath, kNTUNCLinkPathPrefix);
			}
		}
		else
		{
			derr = AssignToUni(&uniPath, mPath);

			if (derr.IsntError())
			{
				derr = PrependToUni(&uniPath, kNTLinkPathPrefix);
			}
		}
	}

	// Get block size of host if needed.
	if (derr.IsntError())
	{
		if (mNoBuffering)
		{
			if (!IsHostNetworked())
			{
				Volume vol;

				derr = vol.AttachLocalVolume(DriveFromPath(mPath));

				if (derr.IsntError())
				{
					mBlockSize = vol.GetBlockSize();

					// Can't be greater than PAGE_SIZE.
					if (mBlockSize > PAGE_SIZE)
						derr = DualErr(kPGDMinorError_BlockSizeNotSupported);
				}

				if (vol.AttachedToLocalVolume())
					vol.DetachLocalVolume();
			}
			else
			{
				mBlockSize = kDefaultBlockSize;
			}
		}
	}

	// Create block buffer if needed.
	if (derr.IsntError())
	{
		if (mNoBuffering)
		{
			derr = GetByteBuffer(PAGE_SIZE, (PGPUInt8 **) &mBlockBuf);
			allocedBlockBuf = derr.IsntError();
		}
	}

	if (derr.IsntError())
	{
		ACCESS_MASK			accessMask;
		IO_STATUS_BLOCK		ioStatus;
		OBJECT_ATTRIBUTES	objAttribs;
		PGPUInt32			createDisp, createOpts, shareAccess;

		if (IsHostNetworked())
			ImpersonateToken();

		// Initialize object attributes
		InitializeObjectAttributes(&objAttribs, uniPath, 
			OBJ_CASE_INSENSITIVE, NULL, NULL);

		// Set create options.
		createOpts = FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT;

		if (mNoBuffering)
			createOpts |= FILE_NO_INTERMEDIATE_BUFFERING;

		// Set access flags.
		accessMask = SYNCHRONIZE | FILE_READ_ACCESS | FILE_WRITE_ACCESS;

		if (mIsReadOnly)
			accessMask &= ~FILE_WRITE_ACCESS;

		// Set share flags.
		shareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;

		if (denyRead)
			shareAccess &= ~FILE_SHARE_READ;

		if (denyWrite)
			shareAccess &= ~FILE_SHARE_WRITE;

		// Set the create disposition.
		if (mustExist)
			createDisp = FILE_OPEN;
		else
			createDisp = FILE_OPEN_IF;

		// Open the file.
		status = ZwCreateFile(&mHandle, accessMask, &objAttribs, &ioStatus, 
			0, FILE_ATTRIBUTE_NORMAL, shareAccess, createDisp, createOpts, 
			NULL, 0);

		if (!NT_SUCCESS(status))
			derr = DualErr(kPGDMinorError_ZwCreateFileFailed, status);

		if (IsHostNetworked())
			RevertToSelf();
	}

	if (derr.IsError())
	{
		if (allocedBlockBuf)
		{
			FreeByteBuffer(mBlockBuf);
		}

		if (mWeSetPath)
		{
			ClearPath();
			mWeSetPath = FALSE;
		}
	}

	return derr;
}

// Close closes a previously opened File object.

DualErr 
File::Close()
{
	DualErr		derr;
	NTSTATUS	status;

	pgpAssert(Opened());

	status = ZwClose(mHandle);

	if (!NT_SUCCESS(status))
		derr = DualErr(kPGDMinorError_ZwCloseFailed, status);

	if (derr.IsntError())
	{
		mHandle = NULL;

		if (mNoBuffering)
			FreeByteBuffer(mBlockBuf);

		if (mWeSetPath)
		{
			ClearPath();
			mWeSetPath = FALSE;
		}
	}

	return derr;
}

// Read reads nBytes from the opened file at position pos (in bytes).

DualErr 
File::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr derr;

	pgpAssert(Opened());

	if (IsHostNetworked())
		ImpersonateToken();
	
	if (mNoBuffering)
		derr = ReadNoBuf(buf, pos, nBytes);
	else
		derr = ReadBuf(buf, pos, nBytes);

	if (IsHostNetworked())
		RevertToSelf();

	return derr;
}

// Write writes nBytes from the opened file at position pos (in bytes).

DualErr 
File::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr derr;

	pgpAssert(Opened());
	pgpAssert(!mIsReadOnly);

	if (IsHostNetworked())
		ImpersonateToken();

	if (derr.IsntError())
	{
		if (mNoBuffering)
			derr = WriteNoBuf(buf, pos, nBytes);
		else
			derr = WriteBuf(buf, pos, nBytes);
	}

	if (IsHostNetworked())
		RevertToSelf();

	return derr;
}


//////////////////////////////////////
// Class File private member functions
//////////////////////////////////////

// ReadBuf reads nBytes from the opened file at position pos (in bytes),
// assuming the file is NOT opened in no-buffering mode.

DualErr 
File::ReadBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus;
	LARGE_INTEGER	bigPos;
	NTSTATUS		status;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);
	pgpAssert(Opened());

	bigPos.QuadPart = pos;

	status = ZwReadFile(mHandle, NULL, NULL, NULL, &ioStatus, buf, nBytes, 
		&bigPos, NULL);

	if (!NT_SUCCESS(status))
		derr = DualErr(kPGDMinorError_ZwReadFileFailed, status);

	return derr;
}

// WriteBuf writes nBytes from the opened file at position pos (in bytes), 
// assuming the file is NOT opened in no-buffering mode.

DualErr 
File::WriteBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus;
	LARGE_INTEGER	bigPos;
	NTSTATUS		status;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);

	pgpAssert(Opened());
	pgpAssert(!mIsReadOnly);

	bigPos.QuadPart = pos;

	status = ZwWriteFile(mHandle, NULL, NULL, NULL, &ioStatus, buf, nBytes, 
		&bigPos, NULL);

	if (!NT_SUCCESS(status))
		derr = DualErr(kPGDMinorError_ZwWriteFileFailed, status);

	return derr;
}

// ReadNoBuf reads nBytes from the opened file at position pos (in bytes),
// assuming the file IS opened in no-buffering mode.

DualErr 
File::ReadNoBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus;
	LARGE_INTEGER	bigPos;
	NTSTATUS		status;
	PGPUInt32		blocksInMiddle, bufPos;
	PGPUInt64		blockFile;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);

	pgpAssert(Opened());
	pgpAssert(mNoBuffering);

	// The position (pos) and size (nBytes) variables are in bytes, but we
	// are reading in blocks. What we have to do is handle the request in at
	// most three separate pieces - a head, middle, and end.

	bufPos = 0;

	if (pos%mBlockSize > 0)			// is there a head?
	{
		PGPUInt32 headOffset, headSize;

		// Now read in the head.
		//
		// blockFile = block in the file containing the head.
		// headOffset = byte offset into blockFile where head starts
		// headSize = size of the head, in bytes

		blockFile	= pos/mBlockSize;
		headOffset	= (PGPUInt32) (pos%mBlockSize);
		headSize	= min(nBytes, mBlockSize - headOffset);

		derr = ReadBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);

		if (derr.IsntError())
		{
			pgpCopyMemory(mBlockBuf + headOffset, buf, headSize);

			bufPos	= headSize;
			pos		+= headSize;
			nBytes	-= headSize;
		}
	}

	// Read in the middle.
	if (derr.IsntError())
	{
		blocksInMiddle = nBytes/mBlockSize;

		if (blocksInMiddle > 0)		// is there a middle?
		{
			// Read in all of the middle.
			//
			// blockFile = block in the PGPdisk where this read begins

			blockFile = pos/mBlockSize;

			derr = ReadBuf(buf + bufPos, blockFile*mBlockSize, 
				blocksInMiddle*mBlockSize);

			if (derr.IsntError())
			{
				bufPos	+= blocksInMiddle * mBlockSize;
				nBytes	-= blocksInMiddle * mBlockSize;
				pos		+= blocksInMiddle * mBlockSize;
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
			// blockFile = block in the file containing the tail.

			blockFile = pos/mBlockSize;

			derr = ReadBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);

			if (derr.IsntError())
			{
				pgpCopyMemory(mBlockBuf, buf + bufPos, nBytes);
			}
		}
	}

	return derr;
}

// WriteNoBuf writes nBytes from the opened file at position pos (in bytes).
// assuming the file IS opened in no-buffering mode.

DualErr 
File::WriteNoBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	IO_STATUS_BLOCK	ioStatus;
	LARGE_INTEGER	bigPos;
	NTSTATUS		status;
	PGPUInt32		blocksInMiddle, bufPos;
	PGPUInt64		blockFile;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(nBytes > 0);

	pgpAssert(Opened());
	pgpAssert(!mIsReadOnly);
	pgpAssert(mNoBuffering);

	// The position (pos) and size (nBytes) variables are in bytes, but we
	// are writing in blocks. What we have to do is handle the request in at
	// most three separate pieces - a head, middle, and end.

	bufPos = 0;

	if (pos%mBlockSize > 0)		// is there a head?
	{
		PGPUInt32 headOffset, headSize;

		// Now write out the head.
		//
		// blockFile = block in the file file containing the head.
		// headOffset = byte offset into blockFile where head starts
		// headSize = size of the head, in bytes

		blockFile	= pos/mBlockSize;
		headOffset	= (PGPUInt32) (pos%mBlockSize);
		headSize	= min(nBytes, mBlockSize - headOffset);

		derr = ReadBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);

		if (derr.IsntError())
		{
			pgpCopyMemory(buf, mBlockBuf + headOffset, headSize);

			derr = WriteBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);
			
			bufPos	= headSize;
			pos		+= headSize;
			nBytes	-= headSize;
		}
	}

	// Write out the middle.
	if (derr.IsntError())		// is there a middle?
	{
		blocksInMiddle = nBytes/mBlockSize;

		if (blocksInMiddle > 0)
		{
			// Write out all of the middle.
			//
			// blockFile = block in the file where this write begins

			blockFile = pos/mBlockSize;

			derr = WriteBuf(buf + bufPos, blockFile*mBlockSize, 
				blocksInMiddle*mBlockSize);

			if (derr.IsntError())
			{
				bufPos	+= blocksInMiddle * mBlockSize;
				nBytes	-= blocksInMiddle * mBlockSize;
				pos		+= blocksInMiddle * mBlockSize;
			}
		}
	}
	
	// Write out the tail.
	if (derr.IsntError())
	{
		if (nBytes)				// is there a tail?	
		{
			// This write is very simple because we know the tail begins on a
			// block boundary.
			//
			// blockFile = the block in the file containing the tail.

			derr = ReadBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);

			if (derr.IsntError())
			{
				pgpCopyMemory(buf, mBlockBuf, nBytes);

				derr = WriteBuf(mBlockBuf, blockFile*mBlockSize, mBlockSize);
			}
		}
	}

	return derr;
}
