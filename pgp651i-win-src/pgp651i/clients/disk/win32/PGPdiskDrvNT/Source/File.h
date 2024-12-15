//////////////////////////////////////////////////////////////////////////////
// File.h
//
// Declaration of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.h,v 1.3 1998/12/14 18:59:37 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_File_h	// [
#define Included_File_h

#include "DualErr.h"


////////////
// Constants
////////////

// These are the flags for File::Open.
const PGPUInt16	kOF_MustExist	= 0x0001;
const PGPUInt16	kOF_ReadOnly	= 0x0002;
const PGPUInt16	kOF_DenyRead	= 0x0004;
const PGPUInt16	kOF_DenyWrite	= 0x0008;
const PGPUInt16	kOF_NoBuffering	= 0x00010;


/////////////
// Class File
/////////////

// Class File must process all requests in its own separate thread due to the
// fact that handles are only valid within a single thread context.

class File
{
public:
				File();
	virtual		~File();

	PGPBoolean	IsHostNetworked();
	PGPBoolean	IsPathSet();
	PGPBoolean	IsReadOnly();
	PGPBoolean	Opened();

	DualErr		GetLength(PGPUInt64 *pLength);
	DualErr		SetLength(PGPInt64 length);

	PGPUInt8	GetLocalHostDrive();

	LPCSTR		GetPath();
	DualErr		SetPath(LPCSTR path);
	void		ClearPath();

	PGPBoolean	ComparePaths(LPCSTR path);

	DualErr		Open(LPCSTR path = NULL, PGPUInt16 flags = NULL);
	DualErr		Close();

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

private:
	HANDLE		mHandle;			// handle for the file
	LPSTR		mPath;				// pathname to the file

	PGPBoolean	mNoBuffering;		// opened with no buffering?
	PGPUInt16	mBlockSize;			// block size of host
	PGPUInt8	*mBlockBuf;			// block buffer

	PGPBoolean	mIsReadOnly;		// opened in read-only mode?
	PGPBoolean	mWeSetPath;			// set path in Open()?

	DualErr		ReadBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		WriteBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

	DualErr		ReadNoBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		WriteNoBuf(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
};

#endif	// ] Included_File_h
