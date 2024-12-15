//////////////////////////////////////////////////////////////////////////////
// PGPdisk.h
//
// Declaration of class PGPdisk.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdisk.h,v 1.3 1998/12/14 18:59:44 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdisk_h	// [
#define Included_PGPdisk_h

#include "DualErr.h"
#include "GenericCallback.h"
#include "PGPdiskFileFormat.h"

#include "CipherContext.h"

#include "PGPdiskWorkerThread.h"
#include "VolFile.h"


////////////////
// Class PGPdisk
////////////////

// Class PGPdisk represents a PGPdisk file. In the future it will handle
// encryption and decryption of the data area but for now it just handles
// parsing of the file header.

class PGPdisk : public VolFile
{
public:
	DualErr	mInitErr;

							PGPdisk();
							~PGPdisk();

	PGPBoolean				BestGuessComparePaths(LPCSTR path);
	PGPUInt64				GetUniqueSessionId();
	PGPdiskWorkerThread *	GetThread();

	void					FlipContexts();
	DualErr					ValidateContexts();

	DualErr					Mount(LPCSTR path, CipherContext *pContext, 
								PGPUInt8 drive = kInvalidDrive, 
								PGPBoolean mountReadOnly = FALSE);
	DualErr						Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr					Read(PGPUInt8 *buf, PGPUInt64 pos, 
								PGPUInt32 nBytes);
	DualErr					Write(PGPUInt8 *buf, PGPUInt64 pos, 
								PGPUInt32 nBytes);

private:
	CipherContext		*mContextA;			// cipher context A
	CipherContext		*mContextB;			// cipher context B

	PGPdiskWorkerThread	mWorkerThread;		// thread for PGPdisk

	PGPUInt64			mUniqueSessionId;	// unique session ID
	PGPUInt8			*mDataBuf;			// data buffer

	PGPdiskMutex		mContextMutex;		// mutex for context use

	SecureMemory		mSmContextA;		// secure mem for context A
	SecureMemory		mSmContextB;		// secure mem for context B

	void	SmartCipherBlocks(PGPUInt64 startBlockIndex, PGPUInt32 nBlocks, 
				const void *src, void *dest, CipherOp op);
};

#endif	// ] Included_PGPdisk_h
