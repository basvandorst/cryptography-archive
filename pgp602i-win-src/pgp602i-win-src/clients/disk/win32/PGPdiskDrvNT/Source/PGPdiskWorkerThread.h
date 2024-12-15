/////////////////////////////////////////////////////////////////////////////////
// PGPdiskWorkerThread.h
//
// Declaration of class PGPdiskWorkerThread.
/////////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskWorkerThread.h,v 1.1.2.17 1998/07/28 21:07:48 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskWorkerThread_h	// [
#define Included_PGPdiskWorkerThread_h

#include "DualErr.h"
#include "WaitObjectClasses.h"

#include "PGPdiskRequestQueue.h"


////////////////////////////
// Class PGPdiskWorkerThread
////////////////////////////

class PGPdiskWorkerThread : private KSystemThread
{
public:
	DualErr mInitErr;

							PGPdiskWorkerThread();
							~PGPdiskWorkerThread();

	PGPBoolean				IsThreadRunning();
	PGPdiskRequestQueue *	GetRequestQueue();

	DualErr					StartThread();
	void					KillThread();

	DualErr					QueueIrpForProcessing(PIRP pIrp);

	DualErr					ScheduleAsyncCallback(PEZCALLBACK callback, 
								PGPUInt32 refData = NULL);
	DualErr					ScheduleSyncCallback(PEZCALLBACK callback, 
								PGPUInt32 refData = NULL);

private:
	PGPdiskRequestQueue	mRequestQueue;		// queue for requests
	PGPdiskSemaphore	mIoSem;				// signalled to indicate I/O

	PGPBoolean			mExitThread;		// exit at next opportunity
	PGPBoolean			mThreadRunning;		// thread running?

	MEMBER_THREAD(PGPdiskWorkerThread, ProcessIo);

	PGPdisk *	GetPGPdiskFromIrp(KIrp I);

	void		OpenAndCloseToPreventHang(PGPdisk *pPGD);

	NTSTATUS	ProcessIrpMjRead(KIrp I);
	NTSTATUS	ProcessIrpMjWrite(KIrp I);
	NTSTATUS	ProcessIrpMjCleanup(KIrp I);
	NTSTATUS	ProcessIrpMjDeviceControl(KIrp I);

	void		ProcessIrp(PRQRequest *pPRQ, PGPBoolean *irpNeedsCompletion);
	void		ProcessAsyncCallback(PRQRequest *pPRQ);
	void		ProcessSyncCallback(PRQRequest *pPRQ);
};

#endif	// ] Included_PGPdiskWorkerThread_h
