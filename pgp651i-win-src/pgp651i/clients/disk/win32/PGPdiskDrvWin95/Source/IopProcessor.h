//////////////////////////////////////////////////////////////////////////////
// IopProcessor.h
//
// Declaration of class IopProcessor.
//////////////////////////////////////////////////////////////////////////////

// $Id: IopProcessor.h,v 1.4 1998/12/14 19:00:18 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_IopProcessor_h	// [
#define Included_IopProcessor_h

#include "DualErr.h"
#include "GenericCallback.h"
#include "Wrappers.h"


////////////
// Constants
////////////

const PGPUInt32 kIopProcMaxMinis = 20;	// max minirequests in an IOP


////////
// Types
////////

// MiniRequestWrapper is used in conjunction with IopRequestInfo to keep
// track of which pieces of which request have been processed so far.

typedef struct MiniRequestWrapper
{
	PGPBoolean	isInUse;	// TRUE if this object is in use

	PGPUInt8	*buf;		// the I/O buffer
	PGPUInt64	pos;		// transfer position
	PGPUInt32	nBytes;		// size of transfer

} MiniRequestWrapper;

// IopRequestInfo holds information about a certain I/O request in progress.

typedef struct IopRequestInfo
{
	PGPBoolean	isBusy;		// TRUE if this object is in use
	PGPBoolean	useAsync;	// process asynchronously?

	PIOP	pIop;	// the IOP itself
	PIOR	pIor;	// points to IOR in the IOP

	GenericCallbackInfo	downInfo;	// information for and from class PGPdisk
	MiniRequestWrapper	miniReqs[kIopProcMaxMinis];	// pieces of the request

} IopRequestInfo;


/////////////////////
// Class IopProcessor
/////////////////////

// An object of class IopProcessor takes an IOP and processes it
// asynchronously. It makes sure each SGD in a read/write request has been
// processed before calling back the entire IOP. This class is Ring-0 thread-
// safe.

class IopProcessor
{
public:
				IopProcessor();
				~IopProcessor();

	PGPBoolean	IsDriveBusy(PGPUInt8 drive);
	void		SetDriveBusy(PGPUInt8 drive, PGPBoolean isBusy);

	PGPBoolean	IsIOAsync();
	void		SetIOAsync(PGPBoolean isIOAsync);

	void		ProcessIop(PIOP pIop);

private:
	IopRequestInfo	mIopReqs[kMaxDrives];	// requests by drive #
	PGPBoolean		mUseAsyncIO;			// use async I/O?

	static void				ScheduleIorReadWriteDispatch(PGPUInt8 drive);

	static void				IorGenIoctlDispatcher(PIOP pIop);
	static VOID __stdcall	IorReadWriteDispatcher(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);

	void					ProcessIorGenIoctl(PIOP pIop);
	void					ProcessIorReadWrite(PIOP pIop, 
								PGPBoolean useAsync = TRUE);

	DualErr					ExecuteRequest(PGPUInt8 drive, 
								PGPBoolean *doneWithRequest);

	static void				IorReadWriteCallback(
								GenericCallbackInfo *downInfo);
	void					IorReadWriteCallbackAux(PGPUInt8 drive);

	void					ScheduleAsyncCallback(PIOP pIop, 
								PGPUInt16 status);
	static VOID __stdcall	AsyncExecuteCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};

#endif	// ] Included_IopProcessor_h
