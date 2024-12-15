//////////////////////////////////////////////////////////////////////////////
// VolFile.h
//
// Declaration of class VolFile.
//////////////////////////////////////////////////////////////////////////////

// $Id: VolFile.h,v 1.1.2.12 1998/07/06 08:58:28 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_VolFile_h	// [
#define Included_VolFile_h

#include "DualErr.h"
#include "GenericCallback.h"

#include "File.h"
#include "Volume.h"


////////////////
// Class VolFile
////////////////

// Class VolFile objects represent volumes associated with a file on a host
// drive. These files can be mounted and unmounted at will, and all I/O
// requests will be routed to the appropriate location on these files just
// like they were hard disks themselves.

class VolFile : public File, public Volume
{
public:
	DualErr mInitErr;

				VolFile();
	virtual		~VolFile();

	PGPBoolean	IsUnmountPending();
	PGPBoolean	IsIOEnabled();

	void		EnableIO(PGPBoolean isEnabled);
	void		MarkUnmountPending(PGPBoolean isPending);

	DualErr		GetDevParams16(DevParams16 *dp16);
	DualErr		GetDevParams32(DevParams32 *dp32);

	DualErr		Mount(LPCSTR path, PGPUInt64 blocksHeader, 
					PGPUInt64 blocksDisk, PDCB pDcb, 
					PGPUInt8 drive = kInvalidDrive, 
					PGPBoolean readOnly = FALSE);
	DualErr		Unmount(PGPBoolean isThisEmergency = FALSE);

	DualErr		Read(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);
	DualErr		Write(PGPUInt8 *buf, PGPUInt64 pos, PGPUInt32 nBytes, 
					GenericCallbackInfo *upInfo = NULL);

private:
	PGPBoolean			mIOEnabled;			// I/O to this drive enabled?
	PGPBoolean			mUnmountPending;	// Unmount pending?

	PGPUInt8			*mBBlock;			// anti-crash bb cache
	PGPUInt64			mBlocksDisk;		// size of volume
	PGPUInt64			mBlocksHeader;		// size of file header

	PGPBoolean			mIsInUse;			// object in use for async calls?
	GenericCallbackInfo *mUpInfo;			// for faking async calls

	void					SetupDcb(PDCB pDcb, 
								PGPBoolean mountReadOnly = FALSE);

	void					ScheduleAsyncCallback(GenericCallbackInfo *upInfo, 
								DualErr derr = DualErr::NoError);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_VolFile_h
