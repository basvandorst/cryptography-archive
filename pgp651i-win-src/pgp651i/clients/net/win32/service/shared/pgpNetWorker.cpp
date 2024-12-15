/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetWorker.cpp,v 1.16 1999/05/05 19:19:49 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>
#include <string>

#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpThreads.h"
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"

#include "pgpNetWorker.h"

// external globals
extern HANDLE	g_ExitEvent;
extern DWORD	g_platformID;

CPGPnetWorker::CPGPnetWorker()
{
	m_threadID = 0;
}

CPGPnetWorker::~CPGPnetWorker()
{
}

void
CPGPnetWorker::Go()
{
	 // create the thread with the StartProc
	if (PGPThreadCreate(&m_threadID,
						   0,
						   StartProc,
						   this) != kPGPError_NoErr) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to start thread");
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(), 
			0,
			__FILE__, 
			__LINE__);
	}
}

unsigned int
CPGPnetWorker::Run()
{
	// some default implementation of Run
	HANDLE lpHandles[1];
	DWORD ret;

	lpHandles[0] = g_ExitEvent;

	while (1) {
		ret = WaitForMultipleObjects(1, lpHandles, FALSE, INFINITE);
		if (ret == WAIT_TIMEOUT) 
			CPGPnetDebugLog::instance()->dbgOut("worker thread: wait timeout");
		else if (ret >= WAIT_ABANDONED_0) {
			switch (ret - WAIT_ABANDONED_0) {
			case 0:
				CPGPnetDebugLog::instance()->dbgOut(
					"worker thread: exit event abandoned");
				goto end;
				break;
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"worker thread: something abandoned");
				break;
			}
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				CPGPnetDebugLog::instance()->dbgOut(
					"worker thread: ExitEvent signaled");
				goto end;
				break;
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"worker thread: something signaled");
				break;
			}
		}
	}
end:
	return kPGPError_NoErr;
}

unsigned int __stdcall
CPGPnetWorker::StartProc(void *pv)
{
	CPGPnetWorker *pMT = static_cast<CPGPnetWorker*> (pv);

	return pMT->Run();
}
