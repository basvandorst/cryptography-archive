//////////////////////////////////////////////////////////////////////////////
// PGPdisk.h
//
// Declaration of class PGPdisk.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.h,v 1.1.2.12.2.4 1998/10/25 23:31:32 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdisk_h	// [
#define Included_PGPdisk_h

#include "DualErr.h"
#include "PGPdiskFileFormat.h"
#include "SecureString.h"

#include "PGPdiskRandomPool.h"
#include "VolFile.h"


////////////////
// Class PGPdisk
////////////////

// Class PGPdisk represents a PGPdisk volume file.

class PGPdisk : public VolFile
{
public:
	DualErr mInitErr;

			PGPdisk(LPCSTR path = NULL, PGPUInt8 drive = kInvalidDrive, 
				PGPUInt64 sessionId = kInvalidSessionId);
			~PGPdisk() { };

	PGPBoolean	BestGuessComparePaths(LPCSTR path);

	PGPUInt64	GetUniqueSessionId();
	void		ReadInUniqueSessionId(LPCSTR path);
	DualErr		MakeUniqueSessionId(LPCSTR path);

	DualErr		Mount(LPCSTR path, SecureString *passphrase, 
					PGPUInt8 drive = kInvalidDrive, 
					PGPBoolean mountReadOnly = FALSE);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr		CreatePGPdisk(LPCSTR path, PGPUInt64 blocksDisk, 
					PGPdiskEncryptionAlgorithm algorithm, 
					SecureString *passphrase, PGPdiskRandomPool *randomPool, 
					PGPUInt8 drive = kInvalidDrive);

	DualErr		ConvertPGPdisk(LPCSTR path, SecureString *passphrase, 
					PGPdiskRandomPool *randomPool);

	DualErr		HighLevelFormatPGPdisk();

	void		SetProgressBarInfo(HWND hWnd, PGPBoolean *pCancelFlag = NULL);

private:
	HWND		mProgressBarHwnd;		// handle of progress bar
	PGPBoolean	*mPCancelFlag;			// points to user cancel indicator
	PGPUInt64	mUniqueSessionId;		// unique session ID

	PGPBoolean	HasUserCanceled();
	void		SetProgressBarPos(PGPUInt32 nPos);
	void		SetProgressBarRange(PGPUInt32 nMin, PGPUInt32 nMax);

	DualErr		CheckFileSystemCompatibility(LPCSTR path, 
					PGPBoolean mountReadOnly, CipherContext *pContext);

	DualErr		InitHeaderWithNewSessionKey(PGPdiskFileHeader *fileHeader, 
					PGPdiskEncryptionAlgorithm algorithm, 
					SecureString *passphrase, PGPdiskRandomPool *randomPool);

	DualErr		InitPGPdiskFileHeader(PGPdiskFileHeader *fileHeader, 
					PGPUInt64 blocksDisk, 
					PGPdiskEncryptionAlgorithm algorithm, 
					SecureString *passphrase, PGPdiskRandomPool *randomPool, 
					PGPUInt8 drive = kInvalidDrive);

	DualErr		FirstTimeEncryptDiskFile(PGPdiskFileHeader *fileHeader, 
					SecureString *passphrase);
	DualErr		FirstTimeWriteHeaderDiskFile(PGPdiskFileHeader* fileHeader);

	DualErr		GetOldSessionKey(LPCSTR path, PGPdiskFileHeader *fileHeader, 
					SecureString *passphrase, CASTKey *oldSessionKey);

	DualErr		ConvertBlocks(PGPdiskFileHeader *fileHeader, 
					PGPdiskCASTConvertInfo *pCCI, 
					const CASTKey *oldSessionKey, 
					const PassphraseSalt *oldSalt, 
					const CASTKey *newSessionKey, 
					const PassphraseSalt *newSalt, 
					PGPUInt32 startingBlock, PGPUInt32 numBlocks);
};

#endif	// ] Included_PGPdisk_h
