//////////////////////////////////////////////////////////////////////////////
// File.cpp
//
// Implementation of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.cpp,v 1.1.2.16.2.1 1998/09/01 00:31:36 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "UtilityFunctions.h"

#include "File.h"
#include "Globals.h"


/////////////////////////////////////
// Class File public member functions
/////////////////////////////////////

// The Class File default constructor.

File::File()
{
	mHandle = INVALID_HANDLE_VALUE;
}

// The Class File destructor.

File::~File()
{
	if (Opened())
		Close();

	ClearPath();
}

// IsHostNetworked returns TRUE if the file's host drive is remote.

PGPBoolean 
File::IsHostNetworked()
{
	return mIsHostNetworked;
}

// IsPathSet returns TRUE if a path is set, FALSE otherwise.

PGPBoolean 
File::IsPathSet()
{
	return (!mPath.IsEmpty());
}

// Opened returns TRUE if the file is open, FALSE otherwise.

PGPBoolean 
File::Opened()
{
	return (mHandle != INVALID_HANDLE_VALUE);
}

// GetLocalHostDrive returns the drive number of the volume that hosts this
// file, but only if it is a local host.

PGPUInt8
File::GetLocalHostDrive()
{
	pgpAssert(!IsHostNetworked());

	return DriveLetToNum(mPath[0]);
}

// GetPath returns a constant pointer to to the path of the attached pathname.

LPCSTR 
File::GetPath()
{
	return (LPCSTR) mPath;
}

// SetPath sets the attached filename to 'path' without opening the file.

DualErr 
File::SetPath(LPCSTR path)
{
	DualErr derr;

	pgpAssertStrValid(path);
	pgpAssert(!Opened());

	try
	{
		mIsHostNetworked = IsNetworkedPath(path);

		ClearPath();
		mPath = path;
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	return derr;
}

// ClearPath frees the attached pathname.

void 
File::ClearPath()
{
	pgpAssert(!Opened());

	mPath.Empty();
}

// ComparePaths returns TRUE if the passed pathname represents the same file
// as the attached pathname.

PGPBoolean 
File::ComparePaths(LPCSTR path)
{
	pgpAssertStrValid(path);

	return (mPath.CompareNoCase(path) == 0);
}

// GetLength returns the length of the file in bytes.

DualErr 
File::GetLength(PGPUInt64 *pLength)
{
	DualErr			derr;
	ULARGE_INTEGER	liLength;
	PGPBoolean		weOpened	= FALSE;

	pgpAssertAddrValid(pLength, PGPUInt64);
	pgpAssert(IsPathSet());

	if (!Opened())
	{
		derr = Open(NULL, kOF_MustExist | kOF_ReadOnly);
		weOpened = derr.IsntError();
	}

	if (derr.IsntError())
	{
		liLength.LowPart = GetFileSize(mHandle, &liLength.HighPart);

		if ((liLength.LowPart == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
			derr = DualErr(kPGDMinorError_GetFileSizeFailed, GetLastError());
	}

	if (weOpened)
		Close();

	if (derr.IsntError())
	{
		(* pLength) = liLength.QuadPart;
	}

	return derr;
}

// SetLength sets the length of the open file to the specified value.

DualErr 
File::SetLength(PGPInt64 length)
{
	DualErr			derr;
	LARGE_INTEGER	liLength;

	pgpAssert(Opened());

	liLength.QuadPart = length;

	liLength.LowPart = SetFilePointer(mHandle, liLength.LowPart, 
		&liLength.HighPart, FILE_BEGIN);
	
	if ((liLength.LowPart == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
		derr = DualErr(kPGDMinorError_SetFilePosFailed, GetLastError());

	if (derr.IsntError())
	{
		if (SetEndOfFile(mHandle) == 0)
			derr = DualErr(kPGDMinorError_SetEOFFailed, GetLastError());
	}

	return derr;
}

// Open will attach 'path' to the File object and open the specified file
// with the specified flags.

DualErr 
File::Open(LPCSTR path, PGPUInt16 flags)
{
	DualErr		derr;
	PGPBoolean	denyRead, denyWrite, mustExist, openReadOnly, weOpened;
	PGPBoolean	truncateFile, weSetPath;
	PGPUInt32	accessFlags, openFlags, shareFlags;

	weOpened = weSetPath = FALSE;

	pgpAssert(!Opened());

	mustExist		= (flags & kOF_MustExist	? TRUE : FALSE);
	openReadOnly	= (flags & kOF_ReadOnly		? TRUE : FALSE);
	truncateFile	= (flags & kOF_Trunc		? TRUE : FALSE);
 	denyRead		= (flags & kOF_DenyRead		? TRUE : FALSE);
	denyWrite		= (flags & kOF_DenyWrite	? TRUE : FALSE);

	if (path)
	{
		derr = SetPath(path);
		weSetPath = derr.IsntError();
	}

	if (derr.IsntError())
	{
		shareFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;

		if (openReadOnly)
			accessFlags	= GENERIC_READ;
		else
			accessFlags = GENERIC_READ | GENERIC_WRITE;

		if (denyRead)
			shareFlags &= ~FILE_SHARE_READ;

		if (denyWrite)
			shareFlags &= ~FILE_SHARE_WRITE;

		if (mustExist)
			openFlags = OPEN_EXISTING;
		else
			openFlags = OPEN_ALWAYS;

		mHandle = CreateFile(mPath, accessFlags, shareFlags, NULL, 
			openFlags, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		if (mHandle == INVALID_HANDLE_VALUE)
			derr = DualErr(kPGDMinorError_FileOpenFailed, GetLastError());

		weOpened = derr.IsntError();
	}

	if (derr.IsntError())
	{
		if (truncateFile)
			derr = SetLength(0);
	}

	if (derr.IsError())
	{
		if (weOpened)
			Close();

		if (weSetPath)
			ClearPath();
	}

	return derr;
}

// Close closes an opened file.

DualErr 
File::Close()
{
	DualErr	derr;

	pgpAssert(Opened());

	if (!CloseHandle(mHandle))
		derr = DualErr(kPGDMinorError_CloseHandleFailed, GetLastError());

	if (derr.IsntError())
	{
		mHandle = INVALID_HANDLE_VALUE;
	}

	return derr;
}

// Read reads 'nBytes' bytes from position 'pos' into buffer 'buf'.

DualErr 
File::Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	LARGE_INTEGER	li;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Opened());

	li.QuadPart = pos;

	li.LowPart = SetFilePointer(mHandle, li.LowPart, &li.HighPart, 
		FILE_BEGIN);
	
	if ((li.LowPart == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
		derr = DualErr(kPGDMinorError_SetFilePosFailed, GetLastError());

	if (derr.IsntError())
	{
		unsigned long numRead;

		if (!ReadFile(mHandle, buf, nBytes, &numRead, NULL))
			derr = DualErr(kPGDMinorError_FileReadFailed, GetLastError());
	}

	return derr;
}

// Write writes 'nBytes' bytes from buffer 'buf' into position 'pos'.

DualErr 
File::Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes)
{
	DualErr			derr;
	LARGE_INTEGER	li;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Opened());

	li.QuadPart = pos;

	li.LowPart = SetFilePointer(mHandle, li.LowPart, &li.HighPart, 
		FILE_BEGIN);
	
	if ((li.LowPart == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
		derr = DualErr(kPGDMinorError_SetFilePosFailed, GetLastError());

	if (derr.IsntError())
	{
		unsigned long numWritten;

		if (!WriteFile(mHandle, buf, nBytes, &numWritten, NULL))
			derr = DualErr(kPGDMinorError_FileWriteFailed, GetLastError());
	}

	return derr;
}
