//////////////////////////////////////////////////////////////////////////////
// File.cpp
//
// Implementation of class File.
//////////////////////////////////////////////////////////////////////////////

// $Id: File.cpp,v 1.1.2.16.2.2 1998/09/01 01:18:16 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "File.h"
#include "Globals.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


CONTEXTHANDLE procContext;

/////////////////////////////////////
// Class File public member functions
/////////////////////////////////////

// The Class File default constructor.

File::File()
{
	mMode = kAM_Default;
	mOpened = mIsReadOnly = mUseInt21 = FALSE;

	mR0Handle = NULL;
	mInt21Handle = NULL;

	mPath = NULL;
	mPFatParser = NULL;

	mIsInUse = FALSE;
	mUpInfo = NULL;
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
	pgpAssert(Opened());
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

// Opened returns TRUE if the file is opened, FALSE if not

PGPBoolean 
File::Opened()
{
	return mOpened;
}

// GetLength returns the length of the file in bytes.

DualErr 
File::GetLength(PGPUInt64 *pLength)
{
	DualErr		derr;
	PGPUInt16	pError;
	PGPUInt32	length;

	pgpAssertAddrValid(pLength, PGPUInt32);
	pgpAssert(Opened());

	if (mUseInt21)
	{
		derr = Driver->Int21GetFileLength(mInt21Handle, &length);
	}
	else
	{
		length = R0_GetFileSize(mR0Handle, &pError);

		if (pError)
			derr = DualErr(kPGDMinorError_R0GetFileSize, pError);
	}

	if (derr.IsntError())
	{
		(* pLength) = length;
	}

	return derr;
}

// SetLength sets the length of the open file to the specified value.

DualErr 
File::SetLength(PGPInt64 length)
{
	DualErr derr;

	// Unable to do this in Ring-0, assert.
	pgpAssert(FALSE);

	return derr;
}

// GetLocalHostFileSysId returns an identifier describing the filesystem type
// of the file's host drive.

DualErr
File::GetLocalHostFileSysId(FileSysId *fsId)
{
	DualErr		derr;
	PGPUInt8	hostDrive;
	Volume		hostVol;

	pgpAssertAddrValid(fsId, FileSysId);
	pgpAssert(Opened());

	hostDrive = GetLocalHostDrive();
	pgpAssert(IsLegalDriveNumber(hostDrive));

	derr = hostVol.AttachLocalVolume(hostDrive);

	if (derr.IsntError())
	{
		derr = hostVol.GetFsId(fsId);
	}

	if (hostVol.AttachedToLocalVolume())
	{
		hostVol.DetachLocalVolume();
	}

	return derr;
}

// GetLocalHostDrive returns the drive number of the volume that hosts this
// file, but only if it is a local host.

PGPUInt8
File::GetLocalHostDrive()
{
	pgpAssert(Opened());
	pgpAssert(!IsHostNetworked());

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
	pgpAssert(IsPathSet());
	pgpAssertStrValid(mPath);
	pgpAssertStrValid(path);

	return (stricmp(mPath, path) ? FALSE : TRUE);
}

// Open opens the file specified by 'path' with the flags specified by
// 'flags'.
//
// API access means that read and write calls to the file will be done using
// the system-provided Ring-0 file access calls. Direct disk access means that
// the API calls will be bypassed and the file will be read and written
// directly on disk. This means the driver must parse the volume's FAT table
// by itself, and it does this by creating and using a file-system parser
// object for the specific filesystem of the host drive.

DualErr 
File::Open(LPCSTR path, PGPUInt16 flags)
{
	DualErr		derr;
	PGPBoolean	denyRead, denyWrite, mustExist, enableDirectDisk;
	PGPBoolean	madeParser, readOnly, weSetPath;

	madeParser = weSetPath = FALSE;

	pgpAssert(!Opened());

	mustExist			= (flags & kOF_MustExist		? TRUE : FALSE);
	readOnly			= (flags & kOF_ReadOnly			? TRUE : FALSE);
	enableDirectDisk	= (flags & kOF_EnableDirectDisk	? TRUE : FALSE);
 	denyRead			= (flags & kOF_DenyRead			? TRUE : FALSE);
	denyWrite			= (flags & kOF_DenyWrite		? TRUE : FALSE);

	// Set the path.
	if (IsntNull(path))
	{
		pgpAssertStrValid(path);

		derr = SetPath(path);
		weSetPath = derr.IsntError();
	}

	// Grab the file handle.
	if (derr.IsntError())
	{
		pgpAssert(IsPathSet());
		derr = GetFileHandle(mustExist, readOnly, denyRead, denyWrite);

		mOpened = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Open in API or direct-disk mode?
		if (!enableDirectDisk || IsHostNetworked())
		{
			// API automatically on networked or forced-API drives.
			mMode = kAM_API;
		}
		else
		{
			FileSysId fsId;

			// Otherwise determine if host is FAT. Don't fail on error.
			GetLocalHostFileSysId(&fsId);
			mMode = (IsFatVolume(fsId) ? kAM_DirectDisk : kAM_API);
		}

		// Perform special initialization for direct-disk access.
		if (mMode == kAM_DirectDisk)
		{
			PGPUInt64 bytesFile;

			// Must flush dirty buffers to the PGPdisk file now.
			if (!IsHostNetworked())
				VolFlush(GetLocalHostDrive(), NULL);

			derr = GetLength(&bytesFile);

			// Create the FAT-parser.
			if (derr.IsntError())
			{
				if (!(mPFatParser = new FatParser(GetPath(), 
					GetLocalHostDrive(), (PGPUInt32) bytesFile)))
				{
					derr = DualErr(kPGDMinorError_OutOfMemory);
				}

				madeParser = derr.IsntError();
			}

			if (derr.IsntError())
			{
				derr = mPFatParser->mInitErr;
			}
		}
	}

	// Lots of cleanup to do in case of an error.
	if (derr.IsError())
	{
		if (madeParser)
		{
			delete mPFatParser;
			mPFatParser = NULL;
		}

		if (mOpened)
		{
			ReleaseFileHandle();
			mOpened = FALSE;
		}

		if (weSetPath)
			ClearPath();
	}

	return derr;
}

// Close closes a previously opened File object. It releases the R0 file
// handle, and deletes the parser (if any).

DualErr 
File::Close()
{
	DualErr		derr;
	PGPBoolean	wasHostNetworked;
	PGPUInt8	hostDrive;

	pgpAssert(Opened());

	if (IsHostNetworked())
	{
		wasHostNetworked = TRUE;
	}
	else
	{
		wasHostNetworked = FALSE;
		hostDrive = GetLocalHostDrive();
	}

	derr = ReleaseFileHandle();

	if (derr.IsntError())
	{
		if (mPFatParser)
		{
			delete mPFatParser;
			mPFatParser = NULL;
		}

		mOpened = FALSE;

		// We must flush data to local host drives to force the systemwide 
		// FSD buffers to refill themselves with good data.

		if (!wasHostNetworked)
			VolFlush(hostDrive, VOL_DISCARD_CACHE);

		ClearPath();
	}

	return derr;
}

// Read reads nBytes from the opened file at position pos (in bytes).

DualErr 
File::Read(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPBoolean	useAsync;
	PGPUInt16	pError;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Opened());
	pgpAssert(nBytes > 0);
	pgpAssert(pos <= 0xFFFFFFFF);

	useAsync = IsntNull(upInfo);

	// If the file was opened in kAM_API mode, read directly from the file.
	// Else, call down to the filesystem-specific parser.

	if (mMode == kAM_API)
	{
		if (mUseInt21)
		{
			derr = Driver->Int21ReadFile(mInt21Handle, buf, (PGPUInt32) pos, 
				nBytes);
		}
		else
		{
			R0_ReadFile(FALSE, mR0Handle, (PVOID) buf, nBytes, 
				(PGPUInt32) pos, &pError);

			if (pError)
				derr = DualErr(kPGDMinorError_R0ReadFailed, pError);
		}

		if (useAsync)
			ScheduleAsyncCallback(upInfo, derr);
	}
	else
	{
		DualErr readErr;

		pgpAssertAddrValid(mPFatParser, FatParser);
		readErr = mPFatParser->Read(buf, (PGPUInt32) pos, nBytes, upInfo);

		if (!useAsync)
			derr = readErr;
	}

	return derr;
}

// Write writes nBytes from the opened file at position pos (in bytes).

DualErr 
File::Write(
	PGPUInt8			*buf, 
	PGPUInt64			pos, 
	PGPUInt32			nBytes, 
	GenericCallbackInfo	*upInfo)
{
	DualErr		derr;
	PGPBoolean	useAsync;
	PGPUInt16	pError;

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssert(Opened());
	pgpAssert(!mIsReadOnly);
	pgpAssert(nBytes > 0);
	pgpAssert(pos <= 0xFFFFFFFF);

	useAsync = IsntNull(upInfo);

	// If the file was opened in kAM_API mode, write directly to the file.
	// Else, call down to the filesystem-specific parser.

	if (mMode == kAM_API)
	{
		if (mUseInt21)
		{
			derr = Driver->Int21WriteFile(mInt21Handle, buf, (PGPUInt32) pos, 
				nBytes);
		}
		else
		{
			R0_WriteFile(FALSE, mR0Handle, (PVOID) buf, nBytes, 
				(PGPUInt32) pos, &pError);

			if (pError)
				derr = DualErr(kPGDMinorError_R0ReadFailed, pError);
		}

		if (useAsync)
			ScheduleAsyncCallback(upInfo, derr);
	}
	else
	{
		DualErr writeErr;

		pgpAssertAddrValid(mPFatParser, FatParser);
		writeErr = mPFatParser->Write(buf, (PGPUInt32) pos, nBytes, upInfo);

		if (!useAsync)
			derr = writeErr;
	}

	return derr;
}


//////////////////////////////////////
// Class File private member functions 
//////////////////////////////////////

// GetFileHandle opens a file handle.

DualErr 
File::GetFileHandle(
	PGPBoolean	mustExist, 
	PGPBoolean	readOnly, 
	PGPBoolean	denyRead, 
	PGPBoolean	denyWrite)
{
	DualErr derr;

	pgpAssertStrValid(mPath);

	mUseInt21 = FALSE;

	mMustExist	= mustExist;
	mIsReadOnly	= readOnly;
	mDenyRead	= denyRead;
	mDenyWrite	= denyWrite;

	// First try to get a Ring0-file-IO handle.
	derr = GetR0FileHandle();
/*
	// If failure, fall back upon int21 file handle.
	if (derr.IsError())
	{
		derr = GetInt21FileHandle();

		// If this attempt succeeds, we must open upon every read or write
		// for now. Close the int21 file handle.

		if (derr.IsntError())
		{
			mUseInt21 = TRUE;
			ReleaseInt21FileHandle();
		}
	}
*/
	return derr;
}

// ReleaseFileHandle closes the file handle opened earlier.

DualErr 
File::ReleaseFileHandle()
{
	DualErr derr;

	if (mUseInt21)
		derr = ReleaseInt21FileHandle();
	else
		derr = ReleaseR0FileHandle();

	return derr;
}

// GetR0FileHandle opens a Ring-0 file-IO file handle.

DualErr 
File::GetR0FileHandle()
{
	DualErr		derr;
	PGPUInt8	action, pAction;
	PGPUInt16	mode, pError;

	pgpAssertStrValid(mPath);

	if (mIsReadOnly)
		mode = OPEN_ACCESS_READONLY;
	else
		mode = OPEN_ACCESS_READWRITE;

	if (mDenyRead && mDenyWrite)
		mode |= OPEN_SHARE_DENYREADWRITE;
	else if (mDenyRead)
		mode |= OPEN_SHARE_DENYREAD;
	else if (mDenyWrite)
		mode |= OPEN_SHARE_DENYWRITE;
	else
		mode |= OPEN_SHARE_DENYNONE;

	if (mMustExist)
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_FAIL;
	else
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_CREATE;

	mR0Handle = R0_OpenCreateFile(FALSE, (LPSTR) GetPath(), mode, 
		ATTR_NORMAL, action, NULL, &pError, &pAction);

	if (pError > 0)
	{
		derr = DualErr(kPGDMinorError_R0OpenFailed, pError);
	}

	return derr;
}

// ReleaseR0FileHandle closes the file handle opened earlier.

DualErr 
File::ReleaseR0FileHandle()
{
	DualErr		derr;
	PGPUInt16	pError;

	R0_CloseFile(mR0Handle, &pError);

	if (pError)
		derr = DualErr(kPGDMinorError_R0CloseFailed, pError);

	if (derr.IsntError())
	{
		mR0Handle = NULL;
	}

	return derr;
}

// GetInt21FileHandle acquires an int21 file handle.

DualErr 
File::GetInt21FileHandle()
{
	DualErr		derr;
	PGPUInt8	action;
	PGPUInt16	mode;

	pgpAssertStrValid(mPath);

	if (mIsReadOnly)
		mode = OPEN_ACCESS_READONLY;
	else
		mode = OPEN_ACCESS_READWRITE;

	if (mDenyRead && mDenyWrite)
		mode |= OPEN_SHARE_DENYREADWRITE;
	else if (mDenyRead)
		mode |= OPEN_SHARE_DENYREAD;
	else if (mDenyWrite)
		mode |= OPEN_SHARE_DENYWRITE;
	else
		mode |= OPEN_SHARE_DENYNONE;

	if (mMustExist)
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_FAIL;
	else
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_CREATE;

	derr = Driver->Int21OpenFile(GetPath(), mode, ATTR_NORMAL, action, 
		&mInt21Handle);

	return derr;
}

// ReleaseInt21FileHandle closes an int21 file handle.

DualErr 
File::ReleaseInt21FileHandle()
{
	DualErr derr;

	derr = Driver->Int21CloseFile(mInt21Handle);

	if (derr.IsntError())
	{
		mInt21Handle = NULL;
	}

	return derr;
}

// ScheduleAsyncCallback schedules a windows event that calls our function
// that will call the asynchronous request up.

void 
File::ScheduleAsyncCallback(GenericCallbackInfo *upInfo, DualErr derr)
{
	static RestrictedEvent_THUNK callbackThunk;

	pgpAssert(!mIsInUse);
	mIsInUse = TRUE;

	pgpAssertAddrValid(upInfo, GenericCallbackInfo);
	mUpInfo = upInfo;
	mUpInfo->derr = derr;

	Call_Restricted_Event(0, NULL, PEF_ALWAYS_SCHED, (PVOID) this, 
		AsyncExecuteCallback, 0, &callbackThunk);
}

// AsyncExecuteCallback was scheduled by 'ScheduleAsyncCallback' for the
// purpose of calling back up the asynchronous request we received.

VOID 
__stdcall 
File::AsyncExecuteCallback(
	VMHANDLE		hVM, 
	THREADHANDLE	hThread, 
	PVOID			Refdata, 
	PCLIENT_STRUCT	pRegs)
{
	File *pFile;

	pFile = (File *) Refdata;
	pgpAssertAddrValid(pFile, File);

	pgpAssert(pFile->mIsInUse);
	pFile->mIsInUse = FALSE;

	pgpAssertAddrValid(pFile->mUpInfo, GenericCallbackInfo);
	pFile->mUpInfo->callback(pFile->mUpInfo);
}
