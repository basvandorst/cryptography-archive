//////////////////////////////////////////////////////////////////////////////
// File.h
//
// Declaration of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.h,v 1.1.2.12 1998/08/08 01:22:05 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_File_h	// [
#define Included_File_h

#include "DualErr.h"


////////////
// Constants
////////////

// These are the flags for File::Open.
const PGPUInt16	kOF_MustExist		= 0x0001;
const PGPUInt16	kOF_NoTrunc			= 0x0002;
const PGPUInt16	kOF_Trunc			= 0x0004;
const PGPUInt16	kOF_ReadOnly		= 0x0008;
const PGPUInt16	kOF_DenyRead		= 0x0010;
const PGPUInt16	kOF_DenyWrite		= 0x0020;


/////////////
// Class File
/////////////

// File represents a file on disk. File objects allow the user to attach a
// pathname to the object and open the specified path at different times.

class File
{
public:
				File();
	virtual		~File();

	PGPBoolean	IsHostNetworked();
	PGPBoolean	IsPathSet();
	PGPBoolean	Opened();

	PGPUInt8	GetLocalHostDrive();

	LPCSTR		GetPath();
	void		ClearPath();
	DualErr		SetPath(LPCSTR path);

	PGPBoolean	ComparePaths(LPCSTR path);

	DualErr		GetLength(PGPUInt64 *pLength);
	DualErr		SetLength(PGPInt64 length);

	DualErr		Open(LPCSTR path = NULL, PGPUInt16 flags = kOF_NoTrunc);
	DualErr		Close();

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes);

private:
	CString		mPath;				// pathname
	HANDLE		mHandle;			// win32 file handle
	PGPBoolean	mIsHostNetworked;	// is the host a networked drive?
};

#endif	// ] Included_File_h
