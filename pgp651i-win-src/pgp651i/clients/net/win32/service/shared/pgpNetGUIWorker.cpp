/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetGUIWorker.cpp,v 1.41 1999/05/19 15:42:38 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>
#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetIPC.h"
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetKernelXChng.h"
#include "pgpNetQueueElement.h"
#include "pgpNetLogonUtils.h"
#include "pgpRWLockSignal.h"
#include "pgpNetCConfig.h"

#include "pgpNetGUIWorker.h"

// external globals
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_GUIQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_IKEQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_KernelQueue;
extern CPGPrwLockSignal<CPGPnetCConfig> *	g_pConfig;
extern CPGPrwLockSignal<CPGPnetDSAarray>	g_SAarray;

extern PGPCond_t							g_ExitEvent;
extern PGPContextRef						g_Context;


CPGPnetGUIWorker::CPGPnetGUIWorker()
	: m_winName(PGPNET_SERVICECOMMWINDOWNAME), m_bEnableStatus(FALSE)
{
}

CPGPnetGUIWorker::~CPGPnetGUIWorker()
{
}

LRESULT
CPGPnetGUIWorker::processGUIEvent(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case PGPNET_REQUESTCURRENTSTATUS:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_REQUESTCURRENTSTATUS");
		break;
	case PGPNET_ENABLESTATUSMESSAGES:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_ENABLESTATUSMESSAGES");
		m_bEnableStatus = TRUE;
		sendFullStatus();
		break;
	case PGPNET_DISABLESTATUSMESSAGES:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_DISABLESTATUSMESSAGES");
		m_bEnableStatus = FALSE;
		break;
	case PGPNET_ENABLELOGMESSAGES:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_ENABLELOGMESSAGES");
		CPGPnetAppLog::instance()->logToApp(TRUE);
		break;
	case PGPNET_DISABLELOGMESSAGES:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_DISABLELOGMESSAGES");
		CPGPnetAppLog::instance()->logToApp(FALSE);
		break;
	case PGPNET_CONFIGUPDATED:
	{
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_CONFIGUPDATED");

		HWND hWnd = FindWindow(PGPNET_SERVICEWINDOWNAME,
			PGPNET_SERVICEWINDOWNAME);
		PostMessage(hWnd, PGPNET_M_APPMESSAGE, 
					PGPNET_CONFIGUPDATED, lParam);

		break;
	}
	case PGPNET_CONFIGANDKEYRINGSUPDATED:
	{
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_CONFIGANDKEYRINGSUPDATED");

		HWND hWnd = FindWindow(PGPNET_SERVICEWINDOWNAME,
			PGPNET_SERVICEWINDOWNAME);
		PostMessage(hWnd, PGPNET_M_APPMESSAGE, 
					PGPNET_CONFIGANDKEYRINGSUPDATED, lParam);

		break;
	}
	case PGPNET_LOGON:
	{
		CPGPnetDebugLog::instance()->dbgOut("Received PGPNET_LOGON");

		HWND hWnd = FindWindow(PGPNET_SERVICEWINDOWNAME,
			PGPNET_SERVICEWINDOWNAME);
		PostMessage(hWnd, PGPNET_M_APPMESSAGE, PGPNET_LOGON, lParam);

		break;
	}
	case PGPNET_LOGOFF:
		CPGPnetDebugLog::instance()->dbgOut("Received PGPNET_LOGOFF");
		pgpNetClearPassphrases();
		// send the result to the tray app
		pgpNetSendLogonStatus(m_hWnd);
		break;
	case PGPNET_QUERYLOGONSTATUS:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_QUERYLOGONSTATUS");
		pgpNetSendLogonStatus(m_hWnd);
		break;
	case PGPNET_QUERYDRIVERSTATUS:
	{
		g_pConfig->startReading();
		PGPBoolean isDriverUp = g_pConfig->data().isDriverUp();
		PGPBoolean isDriverEnabled =
			g_pConfig->data().config()->bPGPnetEnabled;
		g_pConfig->stopReading();
		
		if (!isDriverUp)
			return PGPNET_DRIVERNOTAVAILABLE;
		
		return (isDriverEnabled) ? 
			PGPNET_DRIVERENABLED : PGPNET_DRIVERDISABLED;
		
		break;
	}
	case PGPNET_CLEARLOGFILE:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_QUERYDRIVERSTATUS");
		CPGPnetAppLog::instance()->clearLog();
		
		g_GUIQueue.startWriting();
		g_GUIQueue.data().push(
			CPGPnetQueueElement(0,0,PGPNET_CLEARLOGFILE,0));
		g_GUIQueue.stopWriting();
		g_GUIQueue.setEvent();

		break;
	case PGPNET_ATTEMPTIKE:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received PGPNET_ATTEMPTIKE");
		
		g_KernelQueue.startWriting();
		g_KernelQueue.data().push(
			CPGPnetQueueElement(0,0,PGPNET_ATTEMPTIKE,lParam));
		g_KernelQueue.stopWriting();
		g_KernelQueue.setEvent();
		break;
	default:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received Unknown message from App");
		break;
	}
	return 0;
}

unsigned int
CPGPnetGUIWorker::Run()
{
	PGPUInt32 ret = 0;

	if (!registerClass(0))
		return FALSE;
	
	if (!createMyWindow(0))
		return FALSE;

	// Do not show the window, since we want to be invisible. 
#if _DEBUG	
	//::ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
#endif

	const PGPInt16 numHandles = 2;
	HANDLE lpHandles[numHandles];
	lpHandles[0] = g_ExitEvent;
	lpHandles[1] = g_GUIQueue.event();

	MSG msg;
	PGPBoolean running = TRUE;
	while (running) {
		// Read all of the messages in this next loop, 
		// removing each message as we read it.
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			// If it's a quit message, we're out of here.
			if (msg.message == WM_QUIT) {
				running = FALSE;
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				goto end;
			}
			// Otherwise, dispatch the message.
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		} // End of PeekMessage while loop.

		ret = ::MsgWaitForMultipleObjects(numHandles,
										  lpHandles, 
										  FALSE, 
										  INFINITE,
										  QS_ALLINPUT);
		if (ret >= WAIT_ABANDONED_0) {
			CPGPnetDebugLog::instance()->dbgOut(
				"GUIWorker: something abandoned");
			::PostQuitMessage(0);
		} else if (ret == WAIT_OBJECT_0 + numHandles) {
			continue;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				CPGPnetDebugLog::instance()->dbgOut(
					"GUIWorker: ShutdownRequest signaled");
				::PostQuitMessage(0);
				break;
			case 1:
				CPGPnetDebugLog::instance()->dbgOut(
					"GUIWorker: GUIQueue event signaled");
				processQueue();
				break;
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"GUIWorker: something signaled");
				break;
			}
		}
	}
	
end:
	return 0;
}

PGPInt32
CPGPnetGUIWorker::registerClass(HINSTANCE hinstance) 
{ 
	WNDCLASSEX wcx; 
	
	// Fill in the window class structure with parameters 
	// that describe the main window. 
	
	wcx.cbSize = sizeof(wcx);				// size of structure 
	wcx.style = CS_HREDRAW | CS_VREDRAW;	// redraw if size changes 
	wcx.lpfnWndProc = CPGPnetGUIWorker::winProc; // points to window proc
	wcx.cbClsExtra = 0;						// no extra class memory 
	wcx.cbWndExtra = 0;						// no extra window memory 
	wcx.hInstance = hinstance;				// handle of instance 
	wcx.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);	// predefined app. icon 
	wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);	// predefined arrow 
	wcx.hbrBackground = NULL;				// white background brush 
	wcx.lpszMenuName =  NULL;				// name of menu resource 
	wcx.lpszClassName = m_winName.c_str();	// name of window class 
	wcx.hIconSm = NULL;
	
	// Register the window class. 
	return ::RegisterClassEx(&wcx); 
}

PGPBoolean
CPGPnetGUIWorker::createMyWindow(HINSTANCE hinstance) 
{ 
	// Create the gui worker window. 
	
	m_hWnd = CreateWindow(m_winName.c_str(),	// name of window class 
						  m_winName.c_str(),	// title-bar string 
						  WS_OVERLAPPEDWINDOW,	// top-level window 
						  CW_USEDEFAULT,		// default horizontal pos
						  CW_USEDEFAULT,		// default vertical pos
						  CW_USEDEFAULT,		// default width 
						  CW_USEDEFAULT,		// default height 
						  (HWND) NULL,			// no owner window 
						  (HMENU) NULL,			// use class menu 
						  hinstance,			// handle of app instance 
						  (LPVOID) NULL);		// no window-creation data 
	
	if (!m_hWnd) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to create window '%s'",
			PGPNET_SERVICECOMMWINDOWNAME);
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		return FALSE;
	}

	SetLastError(0);
	PGPInt32 err = SetWindowLong(m_hWnd, GWL_USERDATA, (long)this);
	PGPInt32 lasterr = GetLastError();
	if (err == 0 && lasterr != 0) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to set window '%s' data",
			PGPNET_SERVICECOMMWINDOWNAME);
		CPGPnetAppLog::instance()->logSysEvent(lasterr, 
			0,
			__FILE__,
			__LINE__);
		return FALSE;
	}
	
	return TRUE; 
}

LRESULT CALLBACK
CPGPnetGUIWorker::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)    
{
	CPGPnetGUIWorker *me = static_cast<CPGPnetGUIWorker*>(
		(void*)::GetWindowLong(hwnd, GWL_USERDATA));

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case PGPNET_M_APPMESSAGE:
		return me->processGUIEvent(wParam, lParam);
		break;
	case WM_COPYDATA:
		return me->processCopyData(wParam, lParam);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void
CPGPnetGUIWorker::processQueue()
{
	while (!g_GUIQueue.data().empty()) {
		g_GUIQueue.startReading();
		// pull one thing off;
		CPGPnetQueueElement queueElem = g_GUIQueue.data().front();
		g_GUIQueue.stopReading();

		g_GUIQueue.startWriting();
		g_GUIQueue.data().pop();
		g_GUIQueue.stopWriting();

		switch (queueElem.type()) {
		case OID_PGP_NEWSA:
		{
			const PGPikeSA *pSA = 0;
			pgpCopyMemory(queueElem.data(), &pSA, sizeof(pSA));

			g_SAarray.startWriting();
			if (!g_SAarray.data().addSA(&pSA)) {
				CPGPnetAppLog::instance()->logPGPEvent(
					kPGPError_OutOfMemory,
					pSA->ipAddress,
					__FILE__,
					__LINE__);
			}
			g_SAarray.stopWriting();

			if (m_bEnableStatus) {
				sendToApp(PGPNET_STATUSNEWSA, sizeof(PGPikeSA), pSA);
			}
			break;
		}
		case OID_PGP_SADIED:
			if (queueElem.size() != sizeof(PGPipsecSPI)) {
				CPGPnetDebugLog::instance()->dbgOut("Wrong Size SADIED");
				pgpAssert(TRUE);
				return;
			}

			if (m_bEnableStatus) {
				sendToApp(PGPNET_STATUSREMOVESA,
					sizeof(PGPipsecSPI),
					queueElem.data());	// send to app
			}
			break;
		case PGPNET_CLEARLOGFILE:
			sendToApp(PGPNET_CLEARLOGFILE);
			break;
		case PGPNET_PROMISCUOUSHOSTADDED:
			sendToApp(PGPNET_PROMISCUOUSHOSTADDED);
			break;
		default:
			break;
		}
	}

	g_GUIQueue.resetEvent();
}

void
CPGPnetGUIWorker::sendFullStatus()
{
	if (!m_bEnableStatus)
		return;

	g_SAarray.startReading();
	PGPUInt32 numSAs = g_SAarray.data().numSAs();
	g_SAarray.stopReading();

	if (numSAs == 0)
		return;

	PGPikeSA *flatBuf = 0;

	g_SAarray.startReading();
	sendToApp(PGPNET_STATUSFULL,
		numSAs * sizeof(PGPikeSA),
		g_SAarray.data().flatBuffer(&flatBuf));
	g_SAarray.stopReading();

	delete [] flatBuf;
}

void
CPGPnetGUIWorker::sendToApp(PGPUInt32 dwData, PGPUInt32 cbData, const void *lpData)
{
	if (!m_bEnableStatus)
		return;

	HWND hWnd = FindWindow(PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE);
	if (hWnd) {
		COPYDATASTRUCT cds;

		cds.dwData = dwData;
		cds.cbData = cbData;
		cds.lpData = const_cast<void*>(lpData);

		SendMessage(hWnd, 
			WM_COPYDATA, 
			(WPARAM) m_hWnd, 
			(LPARAM) &cds);
	}
}

void
CPGPnetGUIWorker::sendToApp(WPARAM wParam, LPARAM lParam /* =0 */)
{
	HWND hWnd = FindWindow(PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE);
	if (hWnd) {
		SendMessage(hWnd, 
			PGPNET_M_APPMESSAGE, 
			(WPARAM) wParam, 
			(LPARAM) lParam);
	}
}

LRESULT
CPGPnetGUIWorker::processCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT) lParam;

	switch (pcds->dwData) {
	case PGPNET_STATUSREMOVESA:
	{
		if (pcds->cbData != sizeof(PGPipsecSPI)) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Wrong size parameter for STATUSREMOVESA");
			return 0;
		}

		g_SAarray.startReading();
		PGPikeSA* pSA = g_SAarray.data().findSA(pcds->lpData, pcds->cbData);
		g_SAarray.stopReading();
		if (!pSA)
			return 0;

		// put a copy of the *pointer* onto IKE queue
		g_IKEQueue.startWriting();
		g_IKEQueue.data().push(CPGPnetQueueElement(
			sizeof(PGPikeSA*),
			&pSA,
			kPGPike_MT_SADied,
			pSA->ipAddress));
		g_IKEQueue.stopWriting();
		g_IKEQueue.setEvent();
		break;
	}
	default:
		CPGPnetDebugLog::instance()->dbgOut(
			"Received unknown WM_COPYDATA");
		break;
	}
	return 0;
}
