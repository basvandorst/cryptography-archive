/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Implementation of the CPGPnetMainWorker class.

	$Id: pgpNetMainWorker.cpp,v 1.21.4.2 1999/06/14 23:04:05 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <tchar.h>

#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpOptionList.h"
#include "pgpUserInterface.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "PGPcl.h."
#include "pgpRandomPool.h"

// PGPnet includes
#include "pgpNetProcess.h"
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetIKEmanager.h"
#include "pgpNetMainWorker.h"
#include "pgpNetIPC.h"
#include "pgpService.h"
#include "pgpNetQueueElement.h"
#include "pgpNetLogonUtils.h"
#include "pgpRWLockSignal.h"
#include "pgpNetCConfig.h"
#include "resource.h"

// global external
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_IKEQueue;

extern CPGPrwLockSignal<CPGPnetCConfig>		*g_pConfig;
extern PGPCond_t							g_ShutdownRequest;
extern PGPContextRef						g_Context;
extern DWORD								g_platformID;
extern PGPBoolean							g_isWAN;

static PNCONFIG s_OldConfig;

LRESULT CALLBACK
CPGPnetMainWorker::winProc(HWND hwnd,
						   UINT uMsg,
						   WPARAM wParam,
						   LPARAM lParam)
{
	CPGPnetMainWorker *me =
		static_cast<CPGPnetMainWorker*>((void*)GetWindowLong(
			hwnd,
			GWL_USERDATA));
	
	switch (uMsg) {
	case WM_ENDSESSION:
		if (wParam) {
			if (lParam == ENDSESSION_LOGOFF)
				CService::CtrlHandler(CTRL_LOGOFF_EVENT);
			else
				CService::CtrlHandler(CTRL_SHUTDOWN_EVENT);
		}
		return 0;
		break;
	case WM_POWERBROADCAST:
		switch (wParam) {
		case PBT_APMQUERYSUSPEND:
			return TRUE;
			break;
		case PBT_APMSUSPEND:
			// start suspending, may be sent without querys
			CPGPnetDebugLog::instance()->dbgOut("Got PBT_APMSUSPEND");
			CService::CtrlHandler(PBT_APMSUSPEND);
			break;
		case PBT_APMRESUMESUSPEND:
			// resume operations. does not require a suspend
			CPGPnetDebugLog::instance()->dbgOut("Got PBT_APMRESUMESUSPEND");
			CService::CtrlHandler(PBT_APMRESUMESUSPEND);
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case PGPNET_M_APPMESSAGE:
		switch (wParam) {
		case PGPNET_CONFIGUPDATED:
		case PGPNET_CONFIGANDKEYRINGSUPDATED:
		{
			PGPError	err		= kPGPError_NoErr;

			// save config in s_OldConfig
			g_pConfig->startReading();
			pgpCopyMemory(
				g_pConfig->data().config(), 
				&s_OldConfig,
				sizeof(s_OldConfig));
			g_pConfig->stopReading();

			g_pConfig->startWriting();
			g_pConfig->data().reloadConfiguration();
			if (wParam == PGPNET_CONFIGANDKEYRINGSUPDATED)
			{
				err = g_pConfig->data().reloadKeyring();
			}
			g_pConfig->stopWriting();

			g_pConfig->startReading();
			// propogateConfig sends configuration to Kernel & IKE
			g_pConfig->data().propogateConfig();
			g_pConfig->stopReading();

			if (IsPGPError(err)) {
				CPGPnetAppLog::instance()->logServiceEvent(
					kPGPnetSrvcError_NoAuthKey,
					0,
					0,
					__FILE__,
					__LINE__);
				pgpNetClearPassphrases();
				pgpNetSendLogonStatus(me->hWnd());
			}
			else if (!pgpMemoryEqual(s_OldConfig.expkeyidPGPAuthKey,
				g_pConfig->data().config()->expkeyidPGPAuthKey,
				kPGPMaxExportedKeyIDSize) || 
				!pgpMemoryEqual(s_OldConfig.expkeyidX509AuthKey,
				g_pConfig->data().config()->expkeyidX509AuthKey,
				kPGPMaxExportedKeyIDSize)) 
			{	
				CPGPnetDebugLog::instance()->dbgOut(
					"MainWorker: Asking for passphrase"
					"because CONFIGUPDATED");
				pgpNetPromptForPassphrase(HWND(lParam), me->hWnd(), TRUE);
				pgpNetSendLogonStatus(me->hWnd());
			}
			break;
		}
		case PGPNET_LOGON:
			CPGPnetDebugLog::instance()->dbgOut(
				"MainWorker: Asking for passphrase"
				"because PGPNET_LOGON");
			pgpNetPromptForPassphrase(HWND(lParam), me->hWnd(), FALSE);
			pgpNetSendLogonStatus(me->hWnd());
			break;
		case PGPNET_LOGOFF:
			CPGPnetDebugLog::instance()->dbgOut(
				"MainWorker: SYSTEM Logoff");
			CPGPnetDebugLog::instance()->dbgOut(
				"MainWorker: User %s logging off",
				me->currentUser());
			g_pConfig->startReading();
			PGPBoolean bCache;
			bCache = g_pConfig->data().config()->bCachePassphrases;
			g_pConfig->stopReading();
			if (!bCache)
				pgpNetClearPassphrases();
			me->interactiveUser(FALSE);
			break;
		default:
			CPGPnetDebugLog::instance()->dbgOut(
				"Got unknown app message: %d", wParam);
			break;
		}
		break;
	default:
		if (me && uMsg == me->uReloadKeyringMsg()) {
			CPGPnetDebugLog::instance()->dbgOut("Got ReloadKeyrinMsg");
			// need to reload keyring
			g_pConfig->startWriting();
			PGPError err = g_pConfig->data().reloadKeyring();
			g_pConfig->stopWriting();
		
			if (IsPGPError(err)) {
				CPGPnetAppLog::instance()->logServiceEvent(
					kPGPnetSrvcError_NoAuthKey,
					0,
					0,
					__FILE__,
					__LINE__);
				pgpNetClearPassphrases();
				pgpNetSendLogonStatus(me->hWnd());
			}
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CPGPnetMainWorker::CPGPnetMainWorker(const char *displayName,
									 HINSTANCE hInst)
	: m_displayName(displayName), m_hInstance(hInst)
{
	(void) PGPCondCreate(&m_logonEvent, 0);	// XXX check error
}

CPGPnetMainWorker::~CPGPnetMainWorker()
{
	PGPCondDestroy(&m_logonEvent);

	if (g_platformID == VER_PLATFORM_WIN32_NT)
		RegCloseKey(m_hKey);
}

PGPInt32
CPGPnetMainWorker::registerClass(HINSTANCE hinstance) 
{ 
	WNDCLASSEX wcx; 
	
	// Fill in the window class structure with parameters 
	// that describe the main window. 
	
	wcx.cbSize = sizeof(wcx);				// size of structure 
	wcx.style = CS_HREDRAW | CS_VREDRAW;	// redraw if size changes 
	wcx.lpfnWndProc = CPGPnetMainWorker::winProc;	
	wcx.cbClsExtra = 0;						// no extra class memory 
	wcx.cbWndExtra = 0;						// no extra window memory 
	wcx.hInstance = hinstance;				// handle of instance 
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// predefined app. icon 
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);	// predefined arrow 
	wcx.hbrBackground = NULL;				// white background brush 
	wcx.lpszMenuName =  NULL;				// name of menu resource 
	wcx.lpszClassName = m_displayName.c_str();	// name of window class 
	wcx.hIconSm = NULL;
	
	// Register the window class. 
	return RegisterClassEx(&wcx); 
}

PGPBoolean
CPGPnetMainWorker::createMyWindow(HINSTANCE hinstance) 
{ 
	// Create the main window. 
	
	m_hWnd = CreateWindow(m_displayName.c_str(),	// name of window class 
						  m_displayName.c_str(),	// title-bar string 
						  WS_OVERLAPPEDWINDOW,	// top-level window 
						  CW_USEDEFAULT,	// default horizontal position 
						  CW_USEDEFAULT,	// default vertical position 
						  CW_USEDEFAULT,	// default width 
						  CW_USEDEFAULT,	// default height 
						  (HWND) NULL,		// no owner window 
						  (HMENU) NULL,		// use class menu 
						  hinstance,		// handle of application instance 
						  (LPVOID) NULL);	// no window-creation data 
	
	if (!m_hWnd) {
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(), 
			0,
			__FILE__,
			__LINE__);
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to create window '%s'",
			m_displayName);
		return FALSE;
	}
	
	SetLastError(0);
	PGPInt32 err = SetWindowLong(m_hWnd, GWL_USERDATA, (long)this);
	PGPInt32 lasterr = GetLastError();
	if (err == 0 && lasterr != 0) {
		CPGPnetAppLog::instance()->logSysEvent(lasterr, 
			0,
			__FILE__, 
			__LINE__);
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to set window '%s' data",
			m_displayName);
		return FALSE;
	}

	CPGPnetAppLog::instance()->sendingWnd(m_hWnd);
	
	return TRUE; 
}

PGPBoolean
CPGPnetMainWorker::isInteractiveShellRunning()
{
	CPGPnetDebugLog::instance()->dbgOut("Getting interactive shell status");

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		return FALSE;	// XXX default to FALSE, until detection is possible
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		if (m_shell.empty())
			return FALSE;
	
		TASK_LIST tlist[256];

		CPGPnetDebugLog::instance()->dbgOut("Getting task list");
		// get all the running processes
		PGPUInt32 numTasks = GetTaskList(tlist, 256);

		CPGPnetDebugLog::instance()->dbgOut("Looking for shell process");
		// Look for the shell process
		for (PGPUInt32 i=0; i<numTasks; i++) {
			if (strnicmp(m_shell.c_str(), tlist[i].ProcessName, m_shell.size())
				== 0) {
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

// XXX This function doesn't work under W95
void 
CPGPnetMainWorker::processRegistryChange(HKEY hKey, HANDLE hEvent)
{
	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS)
		return;

	DWORD dwBytes = 0;
	PGPInt32 ret = 0;

	ret = RegQueryValueEx(hKey,"DefaultUserName", 0, 0, 0, &dwBytes);

	char *szData = new char[dwBytes];
	ret = RegQueryValueEx(
		hKey,
		"DefaultUserName", 
		0, 
		0, 
		reinterpret_cast<unsigned char*>(szData), 
		&dwBytes);

	if (ret == ERROR_SUCCESS && 
		(_strnicmp(m_currentUser.c_str(), szData, m_currentUser.size())
		 != 0)) {
		m_currentUser = szData;
		CPGPnetDebugLog::instance()->dbgOut(
			"ProcessRegistryChange: %s logon",
			m_currentUser.c_str());
	}
	delete [] szData;
   
	// somebody is logging in, set interactiveUser to TRUE
	// and ask for passphrase if needed
	m_bInteractiveUser = TRUE;

	CPGPnetDebugLog::instance()->dbgOut("OK, asking for passphrase because REGISTRY");

	// Are we caching the passphrase??
	g_pConfig->startReading();
	PGPBoolean bCache = g_pConfig->data().config()->bCachePassphrases;
	PGPBoolean bValid = g_pConfig->data().isValidPassphrases();
	g_pConfig->stopReading();

	if (!bCache || !bValid)
		pgpNetPromptForPassphrase(0, m_hWnd, TRUE);

	pgpNetSendLogonStatus(m_hWnd);
   
	CPGPnetDebugLog::instance()->dbgOut(
		"Shell %s running, DefaultUserName=%s", 
		m_bInteractiveUser ? "is" : "is not", 
		m_currentUser.c_str());
}

unsigned int
CPGPnetMainWorker::Run()
{
	PGPInt32 ret = 0;

	if (!registerClass(m_hInstance))
		return FALSE;
	
	if (!createMyWindow(m_hInstance))
		return FALSE;

	m_uReloadKeyringMsg = RegisterWindowMessage(RELOADKEYRINGMSG);

	//
	// try to get the default shell and user name from under
	// the winlogon registry key
	//

	if (g_platformID == VER_PLATFORM_WIN32_NT) {
		// none of this works under W95
		char szKey[] =
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey,0,KEY_READ,&m_hKey);
		if (ret == ERROR_SUCCESS) {
			DWORD dwSize = 0;
			char *szData = 0;
		
			ret = RegQueryValueEx(m_hKey, "Shell", 0, 0, 0, &dwSize);
			szData = new char[dwSize]; // XXX memory check
			ret = RegQueryValueEx(m_hKey,
				"Shell",
				0,
				0,
				reinterpret_cast<unsigned char*>(szData),
				&dwSize);
			if (ret == ERROR_SUCCESS)
				m_shell = szData;

			delete [] szData;

			// get the size of data
			ret = RegQueryValueEx(m_hKey,"DefaultUserName",0,0,0,&dwSize);
			szData = new char[dwSize];	// XXX memory check
			ret = RegQueryValueEx(m_hKey,
				"DefaultUserName",
				0,
				0,
				reinterpret_cast<unsigned char*>(szData),
				&dwSize);
			if (ret == ERROR_SUCCESS)
				m_currentUser = szData;

			delete [] szData;
		}

		ret = RegNotifyChangeKeyValue(
			m_hKey,
			FALSE,
			REG_NOTIFY_CHANGE_LAST_SET,
			m_logonEvent, 
			TRUE);
	
		if (ret != ERROR_SUCCESS) {
			CPGPnetDebugLog::instance()->dbgOut("Unable to call RegNotifyChangeKeyValue");
			CPGPnetAppLog::instance()->logSysEvent(ret, 0, __FILE__, __LINE__);
		}
	}

	m_bInteractiveUser = isInteractiveShellRunning();
	
	CPGPnetDebugLog::instance()->dbgOut("Shell %s running, DefaultUserName=%s", 
			 m_bInteractiveUser ? "is" : "is not", 
			 m_currentUser.c_str());

#if _DEBUG
	ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
#endif
	
	CPGPnetDebugLog::instance()->dbgOut("Telling the IKE manager to Go");
	
	// want to prompt for the key to user
	// Only want to prompt if there is an interactive user running
	if (m_bInteractiveUser) {
		CPGPnetDebugLog::instance()->dbgOut("OK, asking for passphrase because Service Start");
		pgpNetPromptForPassphrase(0, m_hWnd, TRUE);
		pgpNetSendLogonStatus(m_hWnd);
	}

	const PGPInt16 numHandles = 2;
	HANDLE lpHandles[numHandles];
	lpHandles[0] = g_ShutdownRequest;
	lpHandles[1] = m_logonEvent;
	
	MSG msg;
	PGPBoolean running = TRUE;
	while (running) {
		// Read all of the messages in this next loop, 
		// removing each message as we read it.
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			// If it's a quit message, we're out of here.
			switch (msg.message) {
			case WM_QUIT:
				running = FALSE;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				goto end;
				break;
			default:
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} // End of PeekMessage while loop.
		
		PGPUInt32 ret = MsgWaitForMultipleObjects(numHandles,
												  lpHandles, 
												  FALSE, 
												  INFINITE, 
												  QS_ALLINPUT);
		switch (ret) {
		case WAIT_OBJECT_0 + numHandles:	// windows message
			continue;
			break;
		case WAIT_OBJECT_0:					// exit event
			CPGPnetDebugLog::instance()->dbgOut("Received ShutdownRequest");
			PostQuitMessage(0);
			break;
		case WAIT_OBJECT_0 + 1:	// winlogon registry event - NT only
			if (g_platformID == VER_PLATFORM_WIN32_NT) {
				if (!m_bInteractiveUser) {
					CPGPnetDebugLog::instance()->dbgOut(
						"Received Registry Event");
					processRegistryChange(m_hKey, lpHandles[1]);
				}
				RegNotifyChangeKeyValue(
					m_hKey,
					FALSE,
					REG_NOTIFY_CHANGE_LAST_SET, 
					lpHandles[1],
					TRUE);
			}
			break;
		default:
			CPGPnetDebugLog::instance()->dbgOut("Something bad happened");
			PostQuitMessage(0);
			break;
		}
	}
	
end:
	PGPCondSignal(&g_ShutdownRequest);

	return kPGPError_NoErr;
}
