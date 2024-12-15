/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	This is the main program file containing the entry point.
	
	$Id: main.cpp,v 1.16.4.1 1999/06/14 03:22:37 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <string>
#include <queue>

// PGP includes
#include "pgpBase.h"
#include "pgpPubTypes.h"
#include "pgpRMWOLock.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpRandomPool.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetLogonUtils.h"
#include "pgpNetIKEservice.h"
#include "pgpNetQueueElement.h"
#include "pgpRWLockSignal.h"
#include "pgpNetCConfig.h"
#include "pgpNetIPC.h"
#include "pgpNetDSAarray.h"

// C includes
PGP_BEGIN_C_DECLARATIONS
#include "pgpNetLog.h"
#include "pgpNetPaths.h"
PGP_END_C_DECLARATIONS

// globals
CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_IKEQueue;
CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_UDPQueue;
CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_KernelQueue;
CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_GUIQueue;

CPGPrwLockSignal<CPGPnetDSAarray>					g_SAarray;
CPGPrwLockSignal<CPGPnetCConfig>					*g_pConfig;

PGPCond_t				g_localIPEvent;
PGPUInt32				g_localIP;
PGPCond_t				g_ExitEvent;
PGPCond_t				g_ShutdownRequest;
PGPCond_t				g_SuspendEvent;
PGPContextRef			g_Context = kInvalidPGPContextRef;
PGPnetLogRef			g_logRef = 0;

DWORD					g_platformID = VER_PLATFORM_WIN32_NT;
PGPBoolean				g_isWAN = FALSE;

// local statics
static void sConvertToArgs(int *, char ***, LPSTR);
static CPGPnetIKEservice *IKEservice;

void
sConvertToArgs(int *argc, char ***argv, LPSTR lpCL)
{
	int count = 0;
	std::string cline, word; 
	int start = 0, found = 0;
	
	*argc = 1;
	*argv = 0;
	
	cline = lpCL;
	// look for a word
	while ((found = cline.find_first_not_of(" \t", start)) != -1) {
		// found one
		count++;
		// move to end of word
		if ((start = cline.find_first_of(" \t", found)) == -1)
			start = cline.length();	// must be the last word
	}
	
	if (count == 0)
		return;
	
	*argc = count+1;
	*argv = new char*[count+1];
	count = 1;
	start = 0;

	while ((found = cline.find_first_not_of(" \t", start)) != -1) {
		// move to end of word
		start = cline.find_first_of(" \t", found);
		if (start == -1)
			start = cline.length();

		word = cline.substr(found, start);
		(*argv)[count] = new char[word.length()+1];
		strncpy((*argv)[count], word.c_str(), word.length()+1);
		count++;
	}
}

PGPBoolean
sCheckWAN()
{
	// look for name of driver in registry
	char *tmpString = 0;
	DWORD dwSize = 0;
	PGPBoolean ret = FALSE;

	HKEY hKey;
	std::string sKey = 
		"SOFTWARE\\Network Associates\\PGP\\PGPnet";

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
	if (ret == ERROR_SUCCESS) {
		ret = RegQueryValueEx(hKey, "CardType", 0, 0, 0, &dwSize);
		tmpString = new char[dwSize];
		ret = RegQueryValueEx(hKey,
			"CardType",
			0,
			0,
			(PGPByte*)tmpString,
			&dwSize);

		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS) {
			goto end;
		}
	} else {
		goto end;
	}

	if (stricmp(tmpString, "NDISWAN") == 0) {
		ret = TRUE;
	} else if (stricmp(tmpString, "NDIS") == 0) {
		ret = FALSE;
	}

end:
	delete [] tmpString;
	return ret;
}

int WINAPI 
WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lpCL, int nCS)
{
	PGPError err = kPGPError_NoErr;
	PGPBoolean cmdLine = FALSE;
	PGPUInt32 i = 0;
	int argc = 0, ret = 0;
	char **argv = 0;
	BOOL fSuccess; 
 
	sConvertToArgs(&argc, &argv, lpCL);

	// look for -c to set command line mode (also sets debug mode)
    if ((argc > 1) && (_stricmp(argv[1], "-c") == 0)) {
		cmdLine = TRUE;
	} else {
		cmdLine = FALSE;
	}

	CPGPnetDebugLog::Initialize(cmdLine);

	CPGPnetDebugLog::instance()->dbgOut("Ok, Parsing arguments");

	// Parse for standard arguments (install, uninstall, version etc.)
	if (CService::ParseStandardArgs(argc, argv))
		goto end;

	CPGPnetDebugLog::instance()->dbgOut("Initializing SDK");

	// Initialize the sdk library
	err = PGPNewContext(kPGPsdkAPIVersion, &g_Context);
	if (IsPGPError(err)) {
		g_Context = kInvalidPGPContextRef;
		goto end;
	}

	CPGPnetDebugLog::instance()->dbgOut("Creating PGPnet log file");

	// Create the PGPnet log file
	PFLFileSpecRef	filespecLog;
	CHAR			szFile[MAX_PATH];
	
	err = PGPnetGetLogFileFullPath(szFile, sizeof(szFile));
	if (IsPGPError (err)) {
		CPGPnetDebugLog::instance()->dbgOut("Fatal Error creating PGPnet log file");
		goto end;
	}
	err = PFLNewFileSpecFromFullPath(
		PGPGetContextMemoryMgr(g_Context), szFile, &filespecLog);
	if (IsPGPError (err)) {
		CPGPnetDebugLog::instance()->dbgOut("Fatal Error creating PGPnet log file");
		goto end;
	}
#if PGP_DEBUG
	err = PGPnetOpenLogFile(filespecLog, TRUE, FALSE, &g_logRef);
#else
	err = PGPnetOpenLogFile(filespecLog, TRUE, TRUE, &g_logRef);
#endif
	if (IsPGPError (err)) {
		CPGPnetDebugLog::instance()->dbgOut("Fatal Error creating PGPnet log file");
		goto end;
	}

	PFLFreeFileSpec(filespecLog);

	// Give the logRef to the AppLog singleton
	CPGPnetAppLog::instance()->logRef(g_logRef);

	if (PGPCondCreate2(&g_ExitEvent, 0) != kPGPError_NoErr) {
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		goto end;
	}

	if (PGPCondCreate2(&g_ShutdownRequest, 0) != kPGPError_NoErr) {
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		goto end;
	}

	if (PGPCondCreate(&g_localIPEvent, 0) != kPGPError_NoErr) {
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		goto end;
	}

	
	if (PGPCondCreate2(&g_SuspendEvent, 0) != kPGPError_NoErr) {
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		goto end;
	}

	g_localIP = 0;

	CPGPnetDebugLog::instance()->dbgOut("OS check");

	// Check to make sure that we are running on NT only
	OSVERSIONINFO vinfo;
	vinfo.dwOSVersionInfoSize = sizeof(vinfo);
	if (!GetVersionEx(&vinfo)) {
		CPGPnetAppLog::instance()->logSysEvent(
			GetLastError(),
			0,
			__FILE__,
			__LINE__);
		goto end;
	}

	g_platformID = vinfo.dwPlatformId;

	// Are we securing the WAN adapter??
	g_isWAN = sCheckWAN();

	CPGPnetDebugLog::instance()->dbgOut("Reading config");

	g_pConfig = new CPGPrwLockSignal<CPGPnetCConfig>;

	// read in the PGPnet configuration
	g_pConfig->startWriting();
	err = g_pConfig->data().loadConfiguration();
	g_pConfig->stopWriting();
	if (IsPGPError(err)) {
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		CPGPnetAppLog::instance()->logServiceEvent(
			kPGPnetSrvcError_ReadConfig, 
			0,
			0,
			__FILE__, 
			__LINE__);
		
		goto end;
	}

	CPGPnetDebugLog::instance()->dbgOut("Init Logon");

	pgpNetLogonInit();

	CPGPnetDebugLog::instance()->dbgOut("Creating service");

	IKEservice = new CPGPnetIKEservice(
		"PGPnetIKE",
		PGPNET_SERVICEWINDOWNAME,
		hI);

	CPGPnetDebugLog::instance()->dbgOut("Setting console control handler");
	fSuccess = SetConsoleCtrlHandler( 
		(PHANDLER_ROUTINE) CService::CtrlHandler,	// handler function 
		TRUE);										// add to list 
	if (!fSuccess) {
		CPGPnetDebugLog::instance()->dbgOut("Unable to set handler: %d", GetLastError());
		goto end; 
	}

	CPGPnetDebugLog::instance()->dbgOut("Checking mode");

	// look for -c to set command line mode
    if (cmdLine) {
		IKEservice->commandLine(TRUE);
		IKEservice->ServiceMain(0, 0);
		goto end;
	}

	CPGPnetDebugLog::instance()->dbgOut("Service starting.");
	IKEservice->StartService();
	
end:
    // When we get here, the service has been stopped
	if (IKEservice)
		ret = IKEservice->exitStatus();
	delete IKEservice;

	PGPCondDestroy(&g_ExitEvent);
	PGPCondDestroy(&g_ShutdownRequest);
	PGPCondDestroy(&g_SuspendEvent);
	pgpNetLogonCleanup();

	delete g_pConfig;

	if (g_logRef)
		PGPnetCloseLogFile(g_logRef);
	
	if (g_Context)
		PGPFreeContext(g_Context);
	
	// need to free argv
	for (i = 1; i < argc; i++)
		delete argv[i];
	delete [] argv;
	
    return ret;
}

