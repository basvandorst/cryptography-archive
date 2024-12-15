//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrvWinutils.cpp
//
// Contains utility functions for making interrupt calls to Windows/DOS.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrvWinutils.cpp,v 1.1.2.16.2.1 1998/09/01 00:31:46 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vtoolscp.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "CPGPdiskDrv.h"
#include "CPGPdiskDrvWinutils.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT


////////////
// Constants
////////////

// The following are file attribute flags returned by R0_GetFileAttributes.

const PGPUInt16 _A_NORMAL	= 0x0000;		// normal file
const PGPUInt16 _A_RDONLY	= 0x0001;		// read-only
const PGPUInt16 _A_HIDDEN	= 0x0002;		// hidden
const PGPUInt16 _A_SYSTEM	= 0x0004;		// system
const PGPUInt16 _A_ARCH		= 0x0020;		// special OS file (?)

// The following are from the DDK.

#define LOCKP_ALLOW_WRITES		0x01	// Bit 0 set - allow writes
#define LOCKP_FAIL_WRITES		0x00	// Bit 0 clear - fail writes
#define LOCKP_FAIL_MEM_MAPPING	0x02	// Bit 1 set - fail memory mappings
#define LOCKP_ALLOW_MEM_MAPPING	0x00	// Bit 1 clear - allow memory mappings
#define LOCKP_USER_MASK			0x03	// Mask for user lock flags
#define LOCKP_LOCK_FOR_FORMAT	0x04	// Level 0 lock for format


//////////////////////////////////////
// Functions for performing interrupts
//////////////////////////////////////

// DoWinInt simulates a DOS interrupt.

PGPBoolean 
CPGPdiskDrv::DoWinInt(PGPUInt8 interrupt, ALLREGS *pAllRegs)
{
	pgpAssertAddrValid(pAllRegs, ALLREGS);

	// Execute the interrupt.
	Exec_VxD_Int(interrupt, pAllRegs);

	return (pAllRegs->RFLAGS & 0x0001 ? FALSE : TRUE);
}


//////////////////////////
// Drive locking functions
//////////////////////////

// GetLockLevel returns the level of the lock on the given drive.

LockLevel 
CPGPdiskDrv::GetLockLevel(PGPUInt8 drive)
{
	PGPInt32	pLockType;
	PGPUInt32	pLockFlags, pLockOwner;

	pgpAssert(drive < kMaxDrives);

	IFSMgr_GetLockState(drive, (PGPUInt32 *) &pLockType, &pLockFlags, 
		&pLockOwner);

	switch (pLockType)
	{
	case -1:
		return kLock_None;
	case 0:
		return (pLockFlags & LOCKP_LOCK_FOR_FORMAT ? kLock_L0MR : kLock_L0);
	case 1:
		return kLock_L1;
	case 2:
		return kLock_L2;
	case 3:
		return kLock_L3;

	default:
		pgpAssert(FALSE);
		return kLock_None;
	}
}

// AcquireLogicalVolLock gets a lock of the specified level on the specified
// drive, increasing or decreasing the lock level as needed. It doesn't
// handle L0 locks.

DualErr 
CPGPdiskDrv::AcquireLogicalVolLock(
	PGPUInt8	drive, 
	LockLevel	lock, 
	PGPUInt8	permissions)
{
	ALLREGS		allRegs;
	DualErr		derr;
	LockLevel	curLock;

	pgpAssert(lock != kLock_L0);
	pgpAssert(IsLegalDriveNumber(drive));

	// Make sure we aren't trying to alter a level-0 lock.
	curLock = GetLockLevel(drive);

	if (curLock == kLock_L0)
		derr = DualErr(kPGDMinorError_LockVolumeFailed);

	DebugOut("PGPdisk: Want lock %d on drive %u curlock %d", lock, drive, 
		curLock);

	// Since there is a special call to acquire a L1 lock, we do this first
	// before anything else.

	if (derr.IsntError())
	{
		if ((curLock == kLock_None) && (lock != kLock_None))
		{
			curLock = kLock_L1;

			allRegs.REAX	= 0x440D;				// function 440D
			allRegs.REBX	= 0x0100 + drive + 1;	// first get L1 lock
			allRegs.RECX	= 0x084A;				// (Lock Volume)
			allRegs.REDX	= permissions;			// locking permissions
			allRegs.RFLAGS	= 0x0001;				// carry cleared if error

			mLockInProgress = TRUE;
			mDriveBeingLocked = drive;

			if (!DoWinInt(0x21, &allRegs))
				derr = DualErr(kPGDMinorError_LockVolumeFailed, allRegs.REAX);

			mLockInProgress = FALSE;
		}
	}

	// Now we calculate how many lock levels we need to go up or down.
	if (derr.IsntError())
	{
		PGPUInt8	locksUp		= 0;
		PGPUInt8	locksDown	= 0;

		if (curLock != lock)
		{
			switch (curLock)
			{
			case kLock_L1:
				switch (lock)
				{
				case kLock_None:
					locksDown = 1;
					break;
				case kLock_L2:
					locksUp = 1;
					break;
				case kLock_L3:
					locksUp = 2;
					break;
				default:
					pgpAssert(FALSE);
					break;
				}
				break;

			case kLock_L2:
				switch (lock)
				{
				case kLock_None:
					locksDown = 2;
					break;
				case kLock_L1:
					locksDown = 1;
					break;
				case kLock_L3:
					locksUp = 1;
					break;
				default:
					pgpAssert(FALSE);
					break;
				}
				break;

			case kLock_L3:
				switch (lock)
				{
				case kLock_None:
					locksDown = 3;
					break;
				case kLock_L1:
					locksDown = 2;
					break;
				case kLock_L2:
					locksDown = 1;
					break;
				default:
					pgpAssert(FALSE);
					break;
				}
				break;

				default:
					pgpAssert(FALSE);
					break;
			}
		}

		// If we need to get locks, we do that here.
		while (derr.IsntError() && (locksUp != 0))
		{
			PGPUInt16 lockValue;

			locksUp--;

			if (curLock == kLock_L1)
			{
				curLock = kLock_L2;
				lockValue = 2;
			}
			else
			{
				curLock = kLock_L3;
				lockValue = 3;
			}

			allRegs.REAX	= 0x440D;				// function 440D
			allRegs.REBX	= (lockValue << 8) + drive + 1;	// get next lock
			allRegs.RECX	= 0x084A;				// (Lock Volume)
			allRegs.REDX	= 0;					// > L1s expect zero here
			allRegs.RFLAGS	= 0x0001;				// carry cleared if error

			mLockInProgress = TRUE;
			mDriveBeingLocked = drive;

			if (!DoWinInt(0x21, &allRegs))
				derr = DualErr(kPGDMinorError_LockVolumeFailed, allRegs.REAX);

			mLockInProgress = FALSE;
		}

		// If we need to release locks, we do that here.
		while (derr.IsntError() && (locksDown !=0))
		{
			locksDown--;

			if (curLock == kLock_L3)
				curLock = kLock_L2;
			else if (curLock == kLock_L2)
				curLock = kLock_L1;
			else if (curLock == kLock_L1)
				curLock = kLock_None;

			allRegs.REAX	= 0x440D;		// function 440D
			allRegs.REBX	= drive + 1;	// decrement lock level
			allRegs.RECX	= 0x086A;		// code 6A (Unlock Volume)
			allRegs.RFLAGS	= 0x0001;		// carry flag cleared on err

			if (!DoWinInt(0x21, &allRegs))
			{
				derr = DualErr(kPGDMinorError_UnlockVolumeFailed, 
					allRegs.REAX);
			}
		}
	}

	DebugOut("PGPdisk: Lock on drive %u is now %d", drive, 
		GetLockLevel(drive));

	return derr;
}

// GetFormatLockOnDrive locks the specified drive for formatting.

DualErr 
CPGPdiskDrv::GetFormatLockOnDrive(PGPUInt8 drive)
{
	ALLREGS		allRegs;
	DualErr		derr;
	PGPUInt32	locksTaken	= 0;

	pgpAssert(IsLegalDriveNumber(drive));

	// Make sure we aren't already locked;
	if (GetLockLevel(drive) != kLock_None)
		derr = DualErr(kPGDMinorError_LockVolumeFailed);

	// First get a 'normal' level-0 lock.
	if (derr.IsntError())
	{
		allRegs.REAX	= 0x440D;				// function 440D
		allRegs.REBX	= 0x0000 + drive + 1;	// L0 lock
		allRegs.RECX	= 0x084A;				// (Lock Volume)
		allRegs.REDX	= 0;					// must be 0
		allRegs.RFLAGS	= 0x0001;				// carry set if error

		mLockInProgress = TRUE;
		mDriveBeingLocked = drive;

		if (!DoWinInt(0x21, &allRegs))
			derr = DualErr(kPGDMinorError_LockVolumeFailed, allRegs.REAX);
		else
			locksTaken++;

		mLockInProgress = FALSE;
	}

	// Now get a 'more restrictive' level-0 lock.
	if (derr.IsntError())
	{
		allRegs.REAX	= 0x440D;				// function 440D
		allRegs.REBX	= 0x0000 + drive + 1;	// L0 lock
		allRegs.RECX	= 0x084A;				// (Lock Volume)
		allRegs.REDX	= 4;					// must be 4
		allRegs.RFLAGS	= 0x0001;				// carry set if error

		mLockInProgress = TRUE;
		mDriveBeingLocked = drive;

		if (!DoWinInt(0x21, &allRegs))
			derr = DualErr(kPGDMinorError_LockVolumeFailed, allRegs.REAX);
		else
			locksTaken++;

		mLockInProgress = FALSE;
	}

	// Undo what we did on error.
	if (derr.IsError())
	{
		while (locksTaken-- > 0)
		{
			allRegs.REAX	= 0x440D;		// function 440D
			allRegs.REBX	= drive + 1;	// decrement lock level
			allRegs.RECX	= 0x086A;		// code 6A (Unlock Volume)
			allRegs.RFLAGS	= 0x0001;		// carry flag set on error

			DoWinInt(0x21, &allRegs);
		}
	}

	return derr;
}

// ReleaseFormatLockOnDrive release a formatting lock on the specified drive.

DualErr 
CPGPdiskDrv::ReleaseFormatLockOnDrive(PGPUInt8 drive)
{
	ALLREGS		allRegs;
	DualErr		derr;
	PGPUInt32	locksTaken	= 2;

	pgpAssert(IsLegalDriveNumber(drive));

	// Make sure we are already locked;
	if (GetLockLevel(drive) != kLock_L0MR)
		derr = DualErr(kPGDMinorError_UnlockVolumeFailed);

	// Release locks.
	while (derr.IsntError() && (locksTaken-- > 0))
	{
		allRegs.REAX	= 0x440D;		// function 440D
		allRegs.REBX	= drive + 1;	// decrement lock level
		allRegs.RECX	= 0x086A;		// code 6A (Unlock Volume)
		allRegs.RFLAGS	= 0x0001;		// carry flag set on error

		if (!DoWinInt(0x21, &allRegs))
			derr = DualErr(kPGDMinorError_UnlockVolumeFailed, allRegs.REAX);
	}

	return derr;
}


/////////////////////////
// File utility functions
/////////////////////////

// IsFileInUseByReader returns TRUE if someone has opened the specified file
// with read access, FALSE otherwise.

PGPBoolean 
CPGPdiskDrv::IsFileInUseByReader(LPCSTR path)
{
	File	existingFile;
	DualErr	derr;

	pgpAssertStrValid(path);

	derr = existingFile.Open(path, 
		kOF_ReadOnly | kOF_DenyRead | kOF_MustExist);

	if (existingFile.Opened())
	{
		existingFile.Close();
	}

	return derr.IsError();
}

// IsFileInUseByWriter returns TRUE if someone has opened the specified file
// with write access, FALSE otherwise.

PGPBoolean 
CPGPdiskDrv::IsFileInUseByWriter(LPCSTR path)
{
	File	existingFile;
	DualErr	derr;

	pgpAssertStrValid(path);

	derr = existingFile.Open(path, 
		kOF_ReadOnly | kOF_DenyWrite | kOF_MustExist);

	if (existingFile.Opened())
	{
		existingFile.Close();
	}

	return derr.IsError();
}

// IsFileInUseByWriter returns TRUE if someone has opened the specified file,
// FALSE otherwise.

PGPBoolean 
CPGPdiskDrv::IsFileInUse(LPCSTR path)
{
	pgpAssertStrValid(path);

	return (IsFileInUseByReader(path) || IsFileInUseByWriter(path));
}

// IsFileValid returns TRUE if the specified file exists.

PGPBoolean 
CPGPdiskDrv::IsFileValid(LPCSTR path)
{
	WORD attribs, error;

	pgpAssertStrValid(path);

	if (R0_GetFileAttributes((LPSTR) path, &attribs, &error))
		return TRUE;
	else
		return FALSE;
}

// GetFirstClustFile performs an IOCTL call to ask the system for the index
// of the first cluster corresponding to the given file on the given FAT
// drive.

DualErr 
CPGPdiskDrv::GetFirstClustFile(LPCSTR path, PGPUInt32 *firstClust)
{
	ALLREGS		allRegs;
	DualErr		derr;
	LockLevel	oldLock;
	PGPBoolean	lockedDown	= FALSE;
	PGPUInt8	drive;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(firstClust, PGPBoolean);

	drive = DriveLetToNum(path[0]);
	oldLock = GetLockLevel(drive);

	allRegs.REAX	= 0x440D;			// function 440D
	allRegs.REBX	= BCS_WANSI;		// character set of path
	allRegs.RECX	= 0x0871;			// minor code 71 (Get First Cluster)
	allRegs.REDX	= (PGPUInt32) path;	// file path
	allRegs.RFLAGS	= 0x0001;			// carry flag cleared on error
	
	AcquireLogicalVolLock(drive, kLock_L3);
	lockedDown = TRUE;

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_GetFirstClustFailed, allRegs.REAX);

	if (lockedDown)
		AcquireLogicalVolLock(drive, oldLock);

	if (derr.IsntError())
	{
		(* firstClust) = MakeLong((PGPUInt16) allRegs.REAX, (PGPUInt16) allRegs.REDX);
	}
	
	return derr;
}

// HasOpenFiles returns TRUE if the specified drive has open file handles,
// FALSE otherwise.

DualErr 
CPGPdiskDrv::HasOpenFiles(PGPUInt8 drive, PGPBoolean *hasOpenFiles)
{
	ALLREGS		allRegs;
	static char	path[kMaxStringSize];
	DualErr		derr;

	pgpAssertAddrValid(hasOpenFiles, PGPBoolean);
	pgpAssert(IsLegalDriveNumber(drive));

	allRegs.REAX	= 0x440D;			// function 440D
	allRegs.REBX	= drive + 1;		// drive to enumerate on
	allRegs.RECX	= 0x086D;			// minor code 6D (Enum Open Files)
	allRegs.REDX	= (PGPUInt32) path;	// file path
	allRegs.RESI	= 0;				// set index to 0 to get first file
	allRegs.REDI	= 0;				// enumerate all kinds of files (0)
	allRegs.RFLAGS	= 0x0001;			// carry flag cleared on error

	// It is OK for this function to return an error since this is its
	// documented behavior if there are no open files on the drive.

	DoWinInt(0x21, &allRegs);

	// Note that FALSE is returned if the carry flag is sent, since this
	// means that there are no open files on the drive.

	if (derr.IsntError())
	{
		(* hasOpenFiles) = ((allRegs.RFLAGS & 0x0001) ? FALSE : TRUE);
	}
	
	return derr;
}


//////////////////////////////
// Int21 file access functions
//////////////////////////////

// Int21OpenFile opens a file using the Int21 interface.

DualErr 
CPGPdiskDrv::Int21OpenFile(
	LPCSTR		path, 
	PGPUInt16	mode, 
	PGPUInt16	attribs, 
	PGPUInt16	action, 
	PGPUInt16	*pHandle)
{
	ALLREGS	allRegs;
	DualErr	derr;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pHandle, PGPUInt16);

	mode |= OPEN_FLAGS_NOCRITERR;

	allRegs.REAX	= 0x716C;			// function 716C
	allRegs.REBX	= mode;				// sharing and mode flags
	allRegs.RECX	= attribs;			// attribute flags
	allRegs.REDX	= action;			// action to take
	allRegs.RESI	= (PGPUInt32) path;	// file path
	allRegs.REDI	= 0;
	allRegs.RFLAGS	= 0x0001;			// carry flag cleared on error
	
	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0OpenFailed, allRegs.REAX);

	if (derr.IsntError())
	{
		(* pHandle) = (PGPUInt16) allRegs.REAX;
	}
	
	return derr;
}

// Int21CloseFile closes a file using the Int21 interface.

DualErr 
CPGPdiskDrv::Int21CloseFile(PGPUInt16 handle)
{
	ALLREGS	allRegs;
	DualErr	derr;

	allRegs.REAX	= 0x3E00;		// function 3E
	allRegs.REBX	= handle;		// handle to close
	allRegs.RFLAGS	= 0x0001;		// carry flag cleared on error

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0CloseFailed, allRegs.REAX);
	
	return derr;
}

// Int21GetFileLength gets the length of a file.

DualErr 
CPGPdiskDrv::Int21GetFileLength(PGPUInt16 handle, PGPUInt32 *pLength)
{
	ALLREGS		allRegs;
	DualErr		derr;

	allRegs.REAX	= 0x4202;		// function 42
	allRegs.REBX	= handle;		// file handle
	allRegs.RECX	= 0;
	allRegs.REDX	= 0;
	allRegs.RFLAGS	= 0x0001;		// carry flag cleared on error

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0GetFileSize, allRegs.REAX);

	if (derr.IsntError())
	{
		(* pLength) = (allRegs.REDX << 16) | (allRegs.REAX & 0xFFFF);
	}

	return derr;
}

// Int21SetFilePos sets the position in a file.

DualErr 
CPGPdiskDrv::Int21SetFilePos(PGPUInt16 handle, PGPUInt32 pos)
{
	ALLREGS		allRegs;
	DualErr		derr;

	allRegs.REAX	= 0x4200;		// function 42
	allRegs.REBX	= handle;		// file handle
	allRegs.RECX	= pos >> 16;	// high word of pos
	allRegs.REDX	= pos & 0xFFFF;	// low word of pos
	allRegs.RFLAGS	= 0x0001;		// carry flag cleared on error

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0SetFilePosFailed, allRegs.REAX);
	
	return derr;
}

// Int21ReadFileAux is a helper function for Int21ReadFile.

DualErr 
CPGPdiskDrv::Int21ReadFileAux(
	PGPUInt16	handle, 
	PGPUInt8	*buf, 
	PGPUInt16	nBytes)
{
	ALLREGS	allRegs;
	DualErr	derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	allRegs.REAX	= 0x3F00;			// function 3F
	allRegs.REBX	= handle;			// file handle
	allRegs.RECX	= nBytes;			// number of bytes to read
	allRegs.REDX	= (PGPUInt32) buf;	// buffer
	allRegs.RFLAGS	= 0x0001;			// carry flag cleared on error

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0ReadFailed, allRegs.REAX);

	return derr;
}

// Int21WriteFileAux is a helper function for Int21WriteFile.

DualErr 
CPGPdiskDrv::Int21WriteFileAux(
	PGPUInt16	handle, 
	PGPUInt8	*buf, 
	PGPUInt16	nBytes)
{
	ALLREGS	allRegs;
	DualErr	derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	allRegs.REAX	= 0x4000;			// function 40
	allRegs.REBX	= handle;			// file handle
	allRegs.RECX	= nBytes;			// number of bytes to write
	allRegs.REDX	= (PGPUInt32) buf;	// buffer
	allRegs.RFLAGS	= 0x0001;			// carry flag cleared on error

	if (!DoWinInt(0x21, &allRegs))
		derr = DualErr(kPGDMinorError_R0WriteFailed, allRegs.REAX);
	
	return derr;
}

// Int21ReadFile reads from a file.

DualErr 
CPGPdiskDrv::Int21ReadFile(
	PGPUInt16	handle, 
	PGPUInt8	*buf, 
	PGPUInt32	pos, 
	PGPUInt32	nBytes)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	// Set the file position.
	derr = Int21SetFilePos(handle, pos);

	// We must read in chunks.
	if (derr.IsntError())
	{
		PGPUInt32 bufPos, bytesLeft;

		bufPos = 0;
		bytesLeft = nBytes;

		while (derr.IsntError() && (bytesLeft > 0))
		{
			PGPUInt16 bytesToRead;
			
			bytesToRead = (PGPUInt16) min(0x7000, nBytes - bufPos);
			derr = Int21ReadFileAux(handle, buf + bufPos, bytesToRead);

			if (derr.IsntError())
			{
				bufPos += bytesToRead;
				bytesLeft -= bytesToRead;
			}
		}
	}
	
	return derr;
}

// Int21WriteFile writes to a file.

DualErr 
CPGPdiskDrv::Int21WriteFile(
	PGPUInt16	handle, 
	PGPUInt8	*buf, 
	PGPUInt32	pos, 
	PGPUInt32	nBytes)
{
	DualErr derr;

	pgpAssertAddrValid(buf, PGPUInt8);

	// Set the file position.
	derr = Int21SetFilePos(handle, pos);

	// We must write in chunks.
	if (derr.IsntError())
	{
		PGPUInt32 bufPos, bytesLeft;

		bufPos = 0;
		bytesLeft = nBytes;

		while (derr.IsntError() && (bytesLeft > 0))
		{
			PGPUInt16 bytesToWrite;
			
			bytesToWrite = (PGPUInt16) min(0x7000, nBytes - bufPos);
			derr = Int21WriteFileAux(handle, buf + bufPos, bytesToWrite);

			if (derr.IsntError())
			{
				bufPos += bytesToWrite;
				bytesLeft -= bytesToWrite;
			}
		}
	}
	
	return derr;
}


///////////////////
// Memory functions
///////////////////

// LockUserBuffer locks memory passed in from Ring-3.

DualErr 
CPGPdiskDrv::LockUserBuffer(void *pMem, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	numPages, page;

	pgpAssertAddrValid(pMem, VoidAlign);

	page = ((PGPUInt32) pMem) >> kWin32Power2PageSize;
	numPages = ((nBytes - 1) >> kWin32Power2PageSize) + 1;

	if (!::LinPageLock(page, numPages, NULL))
	{
		derr = DualErr(kPGDMinorError_LinPageLockFailed);
	}

	return derr;
}

// UnlockUserBuffer unlocks memory passed in from Ring-3.

DualErr 
CPGPdiskDrv::UnlockUserBuffer(void *pMem, PGPUInt32 nBytes)
{
	DualErr		derr;
	PGPUInt32	numPages, page;

	pgpAssertAddrValid(pMem, VoidAlign);

	page = ((PGPUInt32) pMem) >> kWin32Power2PageSize;
	numPages = ((nBytes - 1) >> kWin32Power2PageSize) + 1;

	if (!::LinPageUnLock(page, numPages, NULL))
	{
		derr = DualErr(kPGDMinorError_LinPageUnlockFailed);
	}

	return derr;
}
