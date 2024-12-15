/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Implementation of the CPGPnetIKEservice class. This class handles most
	of the work of the service (see ServiceMain() in the base class, and see
	Run() here).

	$Id: pgpNetIKEservice.cpp,v 1.9.4.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>

// PGP includes
#include "pgpErrors.h"
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetIKEmanager.h"
#include "pgpNetIKEservice.h"
#include "pgpNetIPC.h"
#include "resource.h"

// global external
extern PGPCond_t g_ExitEvent;
extern PGPCond_t g_ShutdownRequest;
extern PGPCond_t g_SuspendEvent;

CPGPnetIKEservice::CPGPnetIKEservice(const char *serviceName,
									 const char *displayName,
									 HINSTANCE hInst)
	: CService(serviceName, displayName), m_bSuspend(FALSE)
{
	m_pIKEmanager = new CPGPnetIKEmanager(displayName, hInst);
}

CPGPnetIKEservice::~CPGPnetIKEservice()
{
	delete m_pIKEmanager;
}

PGPBoolean
CPGPnetIKEservice::OnInit()
{	
	PGPInt32 ret = 0;

	return TRUE;
}

void 
CPGPnetIKEservice::OnShutdown() 
{ 
	CPGPnetDebugLog::instance()->dbgOut("Service shutdown request");
	PGPCondSignal(&g_ShutdownRequest);
	m_pIKEmanager->WaitForWorkerFinish();
}

void
CPGPnetIKEservice::OnLogoff()
{
	CPGPnetDebugLog::instance()->dbgOut("Service signaled for logoff");
	HWND hWnd = FindWindow(PGPNET_SERVICEWINDOWNAME, PGPNET_SERVICEWINDOWNAME);
	PostMessage(hWnd, PGPNET_M_APPMESSAGE, PGPNET_LOGOFF, 0);
}

void
CPGPnetIKEservice::OnResume()
{
	if (m_bSuspend) {
		m_bSuspend = FALSE;
		
		// Resume IKE and Kernel threads
		m_pIKEmanager->GoResume();
	}
}

void CPGPnetIKEservice::OnSuspend()
{
	m_bSuspend = TRUE;
	PGPCondSignal(&g_SuspendEvent);

	m_pIKEmanager->WaitForSuspendFinish();
	ResetEvent(g_SuspendEvent);
}

//
// The main function. The service ends when this function quits
//
void
CPGPnetIKEservice::Run()
{	
	debugMsg("Telling the IKE manager to Go");
	m_pIKEmanager->Go();
	
	const PGPInt16 numHandles = 2;
	HANDLE lpHandles[numHandles];
	lpHandles[0] = m_StopEvent;
	lpHandles[1] = g_ExitEvent;
	
	PGPBoolean running = TRUE;
	while (running) {
		// Read all of the messages in this next loop, 
		// removing each message as we read it.
		PGPUInt32 ret = WaitForMultipleObjects(numHandles,
			lpHandles, 
			FALSE, 
			INFINITE);
		if (ret >= WAIT_ABANDONED_0) {
			running = FALSE;
			goto end;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				debugMsg("Received Stop Event from service control manager");
				running = FALSE;
				goto end;
				break;
			case 1:
				debugMsg("Received Exit Event from Window closing");
				running = FALSE;
				goto end;
				break;
			default:
				break;
			}
		}
	}
	
end:
	PGPCondSignal(&g_ShutdownRequest);
	m_pIKEmanager->WaitForWorkerFinish();
}