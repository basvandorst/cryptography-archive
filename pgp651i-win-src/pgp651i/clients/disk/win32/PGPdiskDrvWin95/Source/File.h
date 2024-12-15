//////////////////////////////////////////////////////////////////////////////
// File.h
//
// Declaration of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.h,v 1.8 1998/12/14 19:00:14 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_File_h	// [
#define Included_File_h

#include "DualErr.h"
#include "FatUtils.h"
#include "GenericCallback.h"

#include "FatParser.h"
#include "Volume.h"


////////////
// Constants
////////////

// These are the flags for File::Open.
const PGPUInt16	kOF_MustExist			= 0x0001;
const PGPUInt16	kOF_ReadOnly			= 0x0002;
const PGPUInt16	kOF_EnableDirectDisk	= 0x0004;
const PGPUInt16	kOF_DenyRead			= 0x0008;
const PGPUInt16	kOF_DenyWrite			= 0x0010;

// Access mode for the file.
enum PGDFileAccessMode {kAM_Default, kAM_API, kAM_DirectDisk};


/////////////
// Class File
/////////////

// This custom File class provides for read/write access to files on drives
// using either the normal Win32 API file access functions (the
// IFSMgr_Ring0_FileIO functions) or by direct access to the volume using the
// driver's own filesystem format (FAT drives only for now). FAT volumes are
// opened by default in direct access mode, unless specified otherwise
// through a flag to 'Open' (not recommended because it will cause crashes).
// Direct access is done by passing read and write calls to a file-system
// parser object.

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

	DualErr		GetLocalHostFileSysId(FileSysId *fsId);
	PGPUInt8	GetLocalHostDrive();

	LPCSTR		GetPath();
	DualErr		SetPath(LPCSTR path);
	void		ClearPath();

	PGPBoolean	ComparePaths(LPCSTR path);

	DualErr		Open(LPCSTR path = NULL, PGPUInt16 flags = NULL);
	DualErr		Close();

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);

private:
	PGDFileAccessMode	mMode;				// access mode of the open file

	PGPBoolean			mOpened;			// is file opened?
	PGPBoolean			mUseInt21;			// use int21 interface?

	PGPBoolean			mMustExist;			// must file exist?
	PGPBoolean			mIsReadOnly;		// opened in read-only mode?
	PGPBoolean			mDenyRead;			// deny read sharing?
	PGPBoolean			mDenyWrite;			// deny write sharing?

	HANDLE				mR0Handle;			// Ring-0 file handle for the file
	PGPUInt16			mInt21Handle;		// handle for int21 calls

	LPSTR				mPath;				// pathname to the file
	FatParser			*mPFatParser;		// Parser for FAT volumes

	PGPBoolean			mIsInUse;			// object in use for async calls?
	GenericCallbackInfo *mUpInfo;			// for faking async calls

	DualErr					GetFileHandle(PGPBoolean mustExist = TRUE, 
								PGPBoolean readOnly = FALSE, 
								PGPBoolean denyRead = FALSE, 
								PGPBoolean denyWrite = FALSE);

	DualErr					ReleaseFileHandle();

	DualErr					GetR0FileHandle();
	DualErr					ReleaseR0FileHandle();

	DualErr					GetInt21FileHandle();
	DualErr					ReleaseInt21FileHandle();

	void					ScheduleAsyncCallback(GenericCallbackInfo *upInfo, 
								DualErr derr = DualErr::NoError);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_File_h
