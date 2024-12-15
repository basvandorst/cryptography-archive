//////////////////////////////////////////////////////////////////////////////
// FileSystemParser.cpp
//
// Implementation of class FileSystemParser.
//////////////////////////////////////////////////////////////////////////////

// $Id: FileSystemParser.cpp,v 1.1.2.5 1998/07/06 08:58:20 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "FileSystemParser.h"
#include "Globals.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


/////////////////////////////////////////////////
// Class FileSystemParser public member functions
/////////////////////////////////////////////////

// The FileSystemParser path and Volume object overloaded constructor sets
// the specified path and the specified host.

FileSystemParser::FileSystemParser(
	LPCSTR		path, 
	PGPUInt8	hostDrive, 
	PGPUInt32	bytesFile)
{
	pgpAssertStrValid(path);
	pgpAssert(IsLegalDriveNumber(hostDrive));

	mPath = NULL;
	mBytesFile = bytesFile;

	mInitErr = mHost.AttachLocalVolume(hostDrive);
	
	if (mInitErr.IsntError())
	{
		mInitErr = mHost.GetFsId(&mFsId);
	}

	if (mInitErr.IsntError())
	{
		mInitErr = SetPath(path);
	}
}

// GetPath returns a constant pointer to the pathname string.

LPCSTR FileSystemParser::GetPath()
{
	return (LPCSTR) mPath;
}

// SetPath sets the filename path variable.

DualErr 
FileSystemParser::SetPath(LPCSTR path)
{
	DualErr derr;

	ClearPath();

	derr = DupeString(path, &mPath);
	return derr;
}

// ClearPath clears the filename path variable.

void
FileSystemParser::ClearPath()
{
	if (mPath)
		delete[] mPath;

	mPath = NULL;
}
