//////////////////////////////////////////////////////////////////////////////
// FileSystemParser.h
//
// Declaration of class FileSystemParser.
//////////////////////////////////////////////////////////////////////////////

// $Id: FileSystemParser.h,v 1.1.2.7 1998/07/06 08:58:21 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_FileSystemParser_h	// [
#define Included_FileSystemParser_h

#include "DualErr.h"
#include "GenericCallback.h"
#include "FatUtils.h"

#include "Volume.h"


/////////////////////////
// Class FileSystemParser
/////////////////////////

// The purpose of a FileSystemParser object is to perform direct access (reads
// and writes) to a file on a disk with a particular filesystem. This access
// is done not using the standard system file access functions, but by reading
// and parsing the sectors off of the disk raw. For every filesystem you wish
// to handle, derive it from this base class. This class is Ring-0
// thread-safe.

class FileSystemParser
{
public:
	DualErr	mInitErr;

			FileSystemParser(LPCSTR path, PGPUInt8 hostDrive, 
				PGPUInt32 bytesFile);
	virtual	~FileSystemParser() {ClearPath();};

	LPCSTR	GetPath();
	DualErr	SetPath(LPCSTR path);
	void	ClearPath();

	DualErr	Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *gcInfo = NULL) 
			{return DualErr::NoError;};

	DualErr	Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
				GenericCallbackInfo *gcInfo = NULL)
			{return DualErr::NoError;};

protected:
	FileSysId	mFsId;			// filesystem of the host drive
	LPSTR		mPath;			// path of file being accessed
	PGPUInt32	mBytesFile;		// size of file
	Volume		mHost;			// Volume object for file's host
};

#endif	// ] Included_FileSystemParser_h
