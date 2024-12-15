/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Implementation of CService. This is a modified MS sample.

	$Id: pgpService.cpp,v 1.2.6.1 1999/06/14 03:22:39 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <stdio.h>
#include <string>

#include "pgpNetIPC.h"
#include "pgpNetDebugLog.h"
#include "pgpNetLogUtils.h"
#include "pgpService.h"

// externals
extern DWORD g_platformID;

// static variables
CService*	CService::m_pThis			= NULL;
std::string CService::m_szServiceName	= "PGPnetIKE";
std::string CService::m_szValueName		= "PGPnetIKE.EXE";
std::string CService::m_szDisplayName	= PGPNET_SERVICEWINDOWNAME;
PGPInt32	CService::m_iMajorVersion	= 1;
PGPInt32	CService::m_iMinorVersion	= 0;
HANDLE		CService::m_hEventSource	= 0;

typedef int (CALLBACK *REGPROC)(DWORD,DWORD);

CService::CService(const char* szServiceName, 
				   const char *szDisplayName)
{
	// copy the address of the current object so we can access it from
	// the static member callback functions. 
	// WARNING: This limits the application to only one CService object. 
	m_pThis = this;

	// Set the default service name and version
	m_szServiceName = szServiceName;
	m_szDisplayName = szDisplayName;

	m_iMajorVersion = 1;
	m_iMinorVersion = 0;
	m_hEventSource = NULL;

	// set up the initial service status 
	m_hServiceStatus = NULL;
	m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
	m_Status.dwCurrentState = SERVICE_STOPPED;
	m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	m_Status.dwWin32ExitCode = 0;
	m_Status.dwServiceSpecificExitCode = 0;
	m_Status.dwCheckPoint = 0;
	m_Status.dwWaitHint = 0;
	m_commandLine = FALSE;
	
	(void) PGPCondCreate(&m_StopEvent, 0);	// XXX error checking
}

CService::~CService()
{
	if (m_hEventSource) {
		DeregisterEventSource(m_hEventSource);
	}
	PGPCondDestroy(&m_StopEvent); // XXX error checking
}

//
// Default command line argument parsing
//
// Returns TRUE if it found an arg it recognised, FALSE if not
// Note: processing some arguments causes output to stdout to be generated.
//
PGPBoolean CService::ParseStandardArgs(int argc, char* argv[])
{
	// See if we have any command line args we recognise
	if (argc <= 1)
		return FALSE;
	
	if (_stricmp(argv[1], "-v") == 0) {
		return TRUE; // say we processed the argument
	} else if (_stricmp(argv[1], "-i") == 0) {
		// Request to install.
		if (!IsInstalled()) {
			// Try and install the copy that's running
			Install();
		}
		return TRUE; // say we processed the argument
	} else if (_stricmp(argv[1], "-u") == 0) {
		// Request to uninstall.
		if (IsInstalled()) {
			// Try and remove the copy that's installed
			Uninstall();
		}
		return TRUE; // say we processed the argument
	}
	// Don't recognise the args
	return FALSE;
}

//
// Install/uninstall routines
//
// Test if the service is currently installed
//
PGPBoolean CService::IsInstalled()
{
	PGPBoolean bResult = FALSE;

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		HKEY hKey = NULL;
		DWORD type = 0, size = 0;
		LONG ret;
		std::string szKey = 
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices";
	
		// check for existence of key
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			szKey.c_str(),
			0,
			KEY_ALL_ACCESS,
			&hKey);

		if (ret != ERROR_SUCCESS)
			return FALSE;
	
		// query value for our name
		ret = RegQueryValueEx(hKey,
			m_szValueName.c_str(),
			0,
			&type,
			0,
			&size);

		if (ret != ERROR_SUCCESS)
			return FALSE;
	
		// make sure that there is some data there
		if (size > 0)
			return TRUE;

		return FALSE;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		// Open the Service Control Manager
		SC_HANDLE hSCM = OpenSCManager(NULL,	// local machine
								   NULL,	// ServicesActive database
								   SC_MANAGER_ALL_ACCESS);	// full access
		if (hSCM) {
			// Try to open the service
			SC_HANDLE hService = OpenService(hSCM,
										 m_szServiceName.c_str(),
										 SERVICE_QUERY_CONFIG);
			if (hService) {
				bResult = TRUE;
				CloseServiceHandle(hService);
			}
			CloseServiceHandle(hSCM);
		}
	}
	return bResult;
}

PGPBoolean
CService::Install()
{
	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		// Get the executable file path
		char szFilePath[_MAX_PATH];
		GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

		HKEY hKey = NULL;
		std::string szKey = 
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices";
		DWORD disp;
	
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						 szKey.c_str(),
						 0, 
						 0, 
						 REG_OPTION_NON_VOLATILE, 
						 KEY_ALL_ACCESS, 
						 0, 
						 &hKey, 
						 &disp) != ERROR_SUCCESS)
			return FALSE;
	
		int err = RegSetValueEx(hKey,
			m_szValueName.c_str(),
			0,
			REG_EXPAND_SZ,
			(CONST BYTE*)szFilePath,
			strlen(szFilePath) + 1);     
	
		RegCloseKey(hKey);
	
		if (err != ERROR_SUCCESS)
			return FALSE;
	
		return TRUE;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
	// Open the Service Control Manager
	SC_HANDLE hSCM = OpenSCManager(NULL,	// local machine
								   NULL,	// ServicesActive database
								   SC_MANAGER_ALL_ACCESS);	// full access
	if (!hSCM)
		return FALSE;

	// Get the executable file path
	char szFilePath[_MAX_PATH];
	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	// Create the service
	SC_HANDLE hService = CreateService(
		hSCM,
		m_szServiceName.c_str(),
		m_szDisplayName.c_str(),
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START, 
		SERVICE_ERROR_NORMAL,
		szFilePath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	if (!hService) {
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	// make registry entries to support logging messages
	// Add the source name as a subkey under the Application
	// key in the EventLog service portion of the registry.
	std::string szKey;
	HKEY hKey = NULL;
	szKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
	szKey += m_szServiceName;

	if (RegCreateKey(HKEY_LOCAL_MACHINE, szKey.c_str(), &hKey) != ERROR_SUCCESS) {
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return FALSE;
	}
	
	// Add the Event ID message-file name to the 'EventMessageFile' subkey.
	RegSetValueEx(hKey,
				  "EventMessageFile",
				  0,
				  REG_EXPAND_SZ, 
				  (CONST BYTE*)szFilePath,
				  strlen(szFilePath) + 1);     

	// Set the supported types flags.
	DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
		EVENTLOG_INFORMATION_TYPE;
	RegSetValueEx(hKey,
				  "TypesSupported",
				  0,
				  REG_DWORD,
				  (CONST BYTE*)&dwData,
				  sizeof(DWORD));
	RegCloseKey(hKey);
	
	LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, m_szServiceName.c_str());
	
	// tidy up
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return TRUE;
	}
	return FALSE;
}

PGPBoolean
CService::Uninstall()
{
	PGPBoolean bResult = FALSE;

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		PGPBoolean err = TRUE;
		HKEY hKey = NULL;
		std::string szKey =
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices";
	
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey.c_str(), 0, KEY_ALL_ACCESS, &hKey)
			!= ERROR_SUCCESS)
			return FALSE;
	
		if (RegDeleteValue(hKey, m_szValueName.c_str()) != ERROR_SUCCESS)
			err = FALSE;
	
		RegCloseKey(hKey);

		return err;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		// Open the Service Control Manager
		SC_HANDLE hSCM = OpenSCManager(NULL,	// local machine
								   NULL,	// ServicesActive database
								   SC_MANAGER_ALL_ACCESS);	// full access
		if (!hSCM) return FALSE;
	
		SC_HANDLE hService = OpenService(hSCM,
									 m_szServiceName.c_str(),
									 DELETE);
		if (hService) {
			if (DeleteService(hService)) {
				LogEvent(EVENTLOG_INFORMATION_TYPE,
						EVMSG_REMOVED,
						m_szServiceName.c_str());
				bResult = TRUE;
			} else {
				LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, m_szServiceName.c_str());
			}
			CloseServiceHandle(hService);
		}
	
		CloseServiceHandle(hSCM);
	}
	return bResult;
}

//
// Logging functions
//
// This function makes an entry into the application event log
//
void
CService::LogEvent(WORD wType, DWORD dwID,
					 const char* pszS1,
					 const char* pszS2,
					 const char* pszS3)
{
	if (g_platformID == VER_PLATFORM_WIN32_NT) {
		const char* ps[3];
		ps[0] = pszS1;
		ps[1] = pszS2;
		ps[2] = pszS3;

		int iStr = 0;
		for (int i = 0; i < 3; i++) {
			if (ps[i] != NULL) iStr++;
		}
	
		// Check the event source has been registered and if
		// not then register it now
		if (!m_hEventSource) {
			m_hEventSource = RegisterEventSource(NULL,	 // local machine
											 m_szServiceName.c_str()); // source name
		}
	
		if (m_hEventSource) {
			ReportEvent(m_hEventSource,
						wType,
						0,
						dwID,
						NULL, // sid
						iStr,
						0,
						ps,
						NULL);
		}
	}
}

//
// Service startup and registration
//
PGPBoolean
CService::StartService()
{

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		ServiceMain(0,0);
		return TRUE;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		SERVICE_TABLE_ENTRY st[] = 
			{
				{const_cast<char*>(m_szServiceName.c_str()), ServiceMain},
				{NULL, NULL}
			};
	
		debugMsg("Calling StartServiceCtrlDispatcher()");

		PGPBoolean b = StartServiceCtrlDispatcher(st);

		debugMsg("Returned from StartServiceCtrlDispatcher()");
		return b;
	}
	return FALSE;
}

//
// static member function (callback)
//
void
CService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	// Get a pointer to the C++ object
	CService* pService = CService::m_pThis;

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		pService->debugMsg("Entering CNTService::ServiceMain()");

		// Start the initialisation
		if (pService->Initialize()) {
			// Do the real work. 
			// When the Run function returns, the service has stopped.
			pService->debugMsg("Calling Run()");
			pService->Run();
		}

		pService->debugMsg("Leaving CNTService::ServiceMain()");
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		pService->debugMsg("Entering CService::ServiceMain()");

		if (!pService->commandLine()) {
			// Register the control request handler
			pService->m_Status.dwCurrentState = SERVICE_START_PENDING;
			pService->m_hServiceStatus =
				RegisterServiceCtrlHandler(pService->m_szServiceName.c_str(), Handler);
	
			if (pService->m_hServiceStatus == NULL) {
				pService->LogEvent(EVENTLOG_ERROR_TYPE,
								EVMSG_CTRLHANDLERNOTINSTALLED);
				return;
			}
		}

		// Start the initialisation
		if (pService->Initialize()) {
			// Do the real work. 
			// When the Run function returns, the service has stopped.
			pService->m_Status.dwWin32ExitCode = 0;
			pService->m_Status.dwCheckPoint = 0;
			pService->m_Status.dwWaitHint = 0;
			pService->debugMsg("Calling pService->Run()");
			pService->Run();
		}

		// Tell the service manager we are stopped
		pService->SetStatus(SERVICE_STOPPED);

		pService->debugMsg("Leaving CService::ServiceMain()");
	}
}

//
// status functions
//
void
CService::SetStatus(DWORD dwState)
{
	if (!m_commandLine) {
		debugMsg("CService::SetStatus(%lu, %lu)", m_hServiceStatus, dwState);

		m_Status.dwCurrentState = dwState;
		SetServiceStatus(m_hServiceStatus, &m_Status);
	}
}

//
// Service initialization
//
PGPBoolean CService::Initialize()
{
	CPGPnetDebugLog::instance()->dbgOut("Entering CService::Initialize()");
	
	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		// Call RegisterServiceProcess
		HMODULE hnd;
		REGPROC fproc;
		
		hnd = LoadLibrary("kernel32.dll");
		if (!hnd) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Unable to register service: no handle to kernel32");
			return FALSE;
		}
		
		fproc = (REGPROC) GetProcAddress(hnd, "RegisterServiceProcess");
		if (!fproc) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Unable to register service: "
				"RegisterServiceProcess not found");
			return FALSE;
		}
		
		int err = (*fproc)(NULL, 1);
		
		if (!err) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Unable to register service: "
				"RegisterServiceProcess failed");
			return FALSE;
		}
		(void) FreeLibrary(hnd);

		// Perform the actual initialization
		PGPBoolean bResult = OnInit(); 
	
		CPGPnetDebugLog::instance()->dbgOut("Leaving CService::Initialize()");

		return bResult;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {

		// Start the initialization
		SetStatus(SERVICE_START_PENDING);

		debugMsg("Entering CService::Initialize()");

		// Perform the actual initialization
		PGPBoolean bResult = OnInit(); 

		// Set final state
		m_Status.dwWin32ExitCode = GetLastError();
		m_Status.dwCheckPoint = 0;
		m_Status.dwWaitHint = 0;
	
		if (!bResult) {
			LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
			SetStatus(SERVICE_STOPPED);
			return FALSE;    
		}

		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);
		SetStatus(SERVICE_RUNNING);

		debugMsg("Leaving CService::Initialize()");

		return TRUE;
	}
	return FALSE;
}

//
// main function to do the real work of the service
//
// This function performs the main work of the service. 
// When this function returns the service has stopped.
//
void
CService::Run()
{
	debugMsg("Entering CService::Run()");

	PGPBoolean running = TRUE;

	while (running) {
		PGPUInt32 ret = WaitForSingleObject(m_StopEvent, 10000);
		switch (ret) {
		case WAIT_TIMEOUT:
			debugMsg("Sleeping...");
			break;
		case WAIT_OBJECT_0:
			running = FALSE;
			debugMsg("Stop Event signaled");
			break;
		case WAIT_ABANDONED:
			running = FALSE;
			debugMsg("Stop event abandoned?");
			break;
		case WAIT_FAILED:
			running = FALSE;
			debugMsg("Wait for stop event failed?");
			break;
		default:
			running = FALSE;
			break;
		}
	}
			
	// nothing more to do
	debugMsg("Leaving CService::Run()");
}

//
// Control request handlers
// static member function (callback) to handle commands from the
// service control manager
//
void
CService::Handler(DWORD dwOpcode)
{
	// Get a pointer to the object
	CService* pService = CService::m_pThis;
	
	pService->debugMsg("CService::Handler(%lu)", dwOpcode);

	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:	// 1
		pService->SetStatus(SERVICE_STOP_PENDING);
		pService->OnStop();
		PGPCondSignal(&pService->m_StopEvent);
		pService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
		break;
		
	case SERVICE_CONTROL_PAUSE: // 2
		pService->OnPause();
		break;
		
	case SERVICE_CONTROL_CONTINUE: // 3
		pService->OnContinue();
		break;
		
	case SERVICE_CONTROL_INTERROGATE: // 4
		pService->OnInterrogate();
		break;
		
	case SERVICE_CONTROL_SHUTDOWN: // 5
		pService->OnShutdown();
		break;
		
	default:
		if (dwOpcode >= SERVICE_CONTROL_USER) {
			if (!pService->OnUserControl(dwOpcode)) {
				pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
			}
		} else {
			pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
		}
		break;
	}

	// Report current status
	pService->debugMsg("Updating status (%lu, %lu)",
					   pService->m_hServiceStatus,
					   pService->m_Status.dwCurrentState);
}

BOOL 
CService::CtrlHandler(DWORD fdwCtrlType) 
{
	// Get a pointer to the object
	CService* pService = CService::m_pThis;
	
    switch (fdwCtrlType) 
    { 
		case CTRL_LOGOFF_EVENT: 
 			CPGPnetDebugLog::instance()->dbgOut("Got CTRL_LOGOFF_EVENT");
			pService->OnLogoff();
			return FALSE;
			break;
        case CTRL_SHUTDOWN_EVENT:
			pService->debugMsg("Got CTRL_SHUTDOWN_EVENT");
			pService->OnShutdown();
			return FALSE;
		case PBT_APMSUSPEND:
			pService->OnSuspend();
			return FALSE;
			break;
		case PBT_APMRESUMESUSPEND:
			pService->OnResume();
			return FALSE;
			break;
        default:
            return FALSE; 
    } 
} 

//
// Called when the service is first initialized
//
PGPBoolean
CService::OnInit()
{
	debugMsg("CService::OnInit()");
	return TRUE;
}

//
// Called when the service control manager wants to stop the service
//
void
CService::OnStop()
{
	debugMsg("CService::OnStop()");
}

//
// called when the service is interrogated
//
void
CService::OnInterrogate()
{
	debugMsg("CService::OnInterrogate()");
}

//
// called when the service is paused
//
void
CService::OnPause()
{
	debugMsg("CService::OnPause()");
}

//
// called when the service is continued
//
void
CService::OnContinue()
{
	debugMsg("CService::OnContinue()");
}

//
// called when the service is shut down
//
void
CService::OnShutdown()
{
	debugMsg("CService::OnShutdown()");
}

void
CService::OnLogoff()
{
	debugMsg("CService::OnLogoff()");
}

//
// called when the service gets a user control message
//
PGPBoolean
CService::OnUserControl(DWORD dwOpcode)
{
	debugMsg("CService::OnUserControl(%8.8lXH)", dwOpcode);
	return FALSE; // say not handled
}

void
CService::debugMsg(const char *fmt, ...)
{
	if (CPGPnetDebugLog::isDebug()) {
		va_list args;
		va_start(args, fmt);
		CPGPnetDebugLog::instance()->vdbgOut(fmt, args);
		va_end(args);
	}
}
