/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPcl.c - PGP ClientLib DLL 
	

	$Id: PGPcl.c,v 1.11.2.1 1998/09/04 15:42:19 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

// project header files
#include "pgpclx.h"
#include "treelist.h"

#include <zmouse.h>

#define IMPORTDLGCLASS	("PGPDLGCLASS")

// the following structure is used to get version information
// from COMCTL32.DLL.  This typedef is in the new system header
// file <SHWLAPI.H>.  Since not everyone may have this file, 
// I've taken the structure and put it here.
typedef struct _DllVersionInfo
{
	DWORD cbSize;
	DWORD dwMajorVersion;                   // Major version
	DWORD dwMinorVersion;                   // Minor version
	DWORD dwBuildNumber;                    // Build number
	DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

#pragma data_seg ("PGPcl_shared")
HWND g_hWndSplash = NULL;
#pragma data_seg ()

// DLL globals
HINSTANCE	g_hInst;
CHAR		g_szHelpFile[MAX_PATH];
UINT		g_uMouseWheelMessage = 0;

//	___________________________________________________
//
//	Register window class for selective import dialog
//	This allows setting the icon to the PGPkeys icon

static VOID
sRegisterSelectiveImportClass (VOID)
{
	WNDCLASSEX	wc;

	// class 0x8002 is the "hardwired" class of standard dialogs
	wc.cbSize = sizeof (WNDCLASSEX);
	GetClassInfoEx (NULL, MAKEINTRESOURCE(0x8002), &wc);

	wc.lpszClassName = IMPORTDLGCLASS;
	wc.hIcon = LoadIcon (g_hInst, MAKEINTRESOURCE(IDI_KEYICON));
	wc.hInstance = g_hInst;

	RegisterClassEx (&wc);
}

//	____________________________________________________________
//
//	check if proper version of COMCTL32.DLL is installed

#define MIN_MAJORVERSION	4
#define MIN_MINORVERSION	70

static PGPError  
sCheckComctl32Version (VOID) 
{
	PGPError		err			= kPGPError_NoErr;
	HINSTANCE		hComCtl;

	hComCtl = LoadLibrary ("comctl32.dll");

	if (hComCtl) {
 		FARPROC	pDllGetVersion;
  
		pDllGetVersion = GetProcAddress (hComCtl, "DllGetVersion");
   
		if (pDllGetVersion) {
			DLLVERSIONINFO	dvi;
			LONG			hr;		// this should be an HRESULT
      
			ZeroMemory (&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
   
			hr = (LONG)(*pDllGetVersion)(&dvi);
      
			if (!(hr & 0x01)) {		// HRESULT returns error in bit 0
				if (dvi.dwMajorVersion < MIN_MAJORVERSION)
					err = kPGPError_Win32_InvalidComctl32;
				else if (dvi.dwMajorVersion == MIN_MAJORVERSION) {
					if (dvi.dwMinorVersion < MIN_MINORVERSION)
						err = kPGPError_Win32_InvalidComctl32;
				}
			}
			else 
				err = kPGPError_Win32_InvalidComctl32;
		}
		else {
			err = kPGPError_Win32_InvalidComctl32;
		}

		FreeLibrary(hComCtl);
	}
	else 
		err = kPGPError_Win32_InvalidComctl32;


	return err;
}

//	____________________________________________________________
//
//	Dialog procedure for driver warning dialog

static BOOL CALLBACK 
sDriverWarnDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	switch(uMsg) {

	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND :
		switch(LOWORD (wParam)) {

		case IDOK : 
			if (IsDlgButtonChecked (hDlg, IDC_NOMOREWARNINGS) == BST_CHECKED)
				EndDialog (hDlg, 2);
			else
				EndDialog (hDlg, 1);
			break;

		case IDCANCEL :
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//________________________________________
//
//	code to start PGPmemlock driver in NT
//		driver must already have been copied to 
//		c:\winnt\system32\drivers directory and
//		appropriate registry entries created

#define DRIVERNAME				"PGPmemlock"
#define DRIVER_NO_ERROR			0
#define DRIVER_NOT_WINNT		1
#define DRIVER_ACCESS_ERROR		2
#define DRIVER_CREATE_FAIL		3
#define DRIVER_ALREADY_STARTED	4
#define DRIVER_MISC_ERROR		5

PGPError PGPclExport
PGPclStartMemLockDriver (VOID)
{
	PGPError		err			= kPGPError_UnknownError;
	SC_HANDLE		schSCMan	= NULL;
	SC_HANDLE		schServ		= NULL;
	DWORD			dwErr;
	BOOL			bRet;
	OSVERSIONINFO	osid;
	SERVICE_STATUS	ss;
	CHAR			szPath[MAX_PATH];

	// check if we're running under NT
	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	// no => just stop here
	if (osid.dwPlatformId != VER_PLATFORM_WIN32_NT) {
		err = DRIVER_NOT_WINNT;
		goto done;
	}

	// yes, open service control manager
	schSCMan = OpenSCManager (NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCMan == NULL) {
		dwErr = GetLastError();
		if (dwErr == ERROR_ACCESS_DENIED) {
			err = DRIVER_ACCESS_ERROR;
			goto done;
		}
		else {
			err = DRIVER_MISC_ERROR;
			goto done;
		}
	}
	// OK, success open of service control manager
	else {
		// try to open service
		schServ = OpenService (schSCMan, DRIVERNAME, 
						SERVICE_START|SERVICE_QUERY_STATUS);

		if (schServ == NULL) {
			// couldn't open service
			dwErr = GetLastError ();
			if (dwErr != ERROR_SERVICE_DOES_NOT_EXIST) {
				err = DRIVER_MISC_ERROR;
				goto done;
			}

			// try to create new service ...
			GetSystemDirectory (szPath, sizeof(szPath));
			if (szPath[lstrlen (szPath) -1] != '\\')
				lstrcat (szPath, "\\");
			lstrcat (szPath, "drivers\\");
			lstrcat (szPath, DRIVERNAME);
			lstrcat (szPath, ".sys");
			schServ = CreateService (schSCMan, DRIVERNAME, DRIVERNAME,
						SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
						SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
						szPath, NULL, NULL, NULL, NULL, NULL);

			if (schServ == NULL) {
				err = DRIVER_CREATE_FAIL;
				goto done;
			}
		}

		bRet = QueryServiceStatus (schServ, &ss);
		if (!bRet) {
			err = DRIVER_MISC_ERROR;
			goto done;
		}
		if (ss.dwCurrentState == SERVICE_STOPPED) {
			bRet = StartService (schServ, 0, NULL);
			if (!bRet) {
				dwErr = GetLastError ();
				err = DRIVER_MISC_ERROR;
				goto done;
			}
		}
		else {
			err = DRIVER_ALREADY_STARTED;
			goto done;
		}
	}
	err = DRIVER_NO_ERROR;

done :
	// cleanup service handle
	if (schServ)
		CloseServiceHandle (schServ);

	// clean up service control manager
	if (schSCMan)
		CloseServiceHandle (schSCMan);

	return err;
}


//	____________________________________________________________
//
//	check if page-locking driver is working by attempting to lock memory

static PGPError  
sCheckMemLockDriver (VOID) 
{
	PGPError		err			= kPGPError_NoErr;
	PGPContextRef	context		= kInvalidPGPContextRef;
	PGPPrefRef		prefs		= kInvalidPGPPrefRef;
	PGPBoolean		bWarn		= TRUE;
	PGPFlags		flags		= 0;
	LPVOID			pmem		= NULL;
	INT				iret		= 0;

	PGPclStartMemLockDriver ();

	err = PGPNewContext (kPGPsdkAPIVersion, &context); CKERR;

	pmem = PGPNewSecureData (PGPGetContextMemoryMgr (context), 32, 0);
	if (!pmem) {
		err = kPGPError_OutOfMemory;
		goto done;
	}

	flags = PGPGetMemoryMgrDataInfo (pmem);
	if (!(flags & kPGPMemoryMgrBlockInfo_NonPageable)) {
		// memory is pageable, thus the driver is not working
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (context), &prefs);
		PGPGetPrefBoolean (prefs, kPGPPrefWarnOnMemLockFailure, &bWarn);

		if (bWarn) {
			iret = DialogBox (g_hInst, MAKEINTRESOURCE(IDD_DRIVERWARNING),
							NULL, sDriverWarnDlgProc);
			switch (iret) {
			case 0 :
				err = kPGPError_UserAbort;
				break;

			case 2 :
				PGPSetPrefBoolean (prefs,kPGPPrefWarnOnMemLockFailure,FALSE);
			case 1 :
				err = kPGPError_NoErr;
				break;
			}
		}

		PGPclCloseClientPrefs (prefs, TRUE);
	}

done :
	if (pmem)
		PGPFreeData (pmem);
	if (PGPContextRefIsValid (context))
		PGPFreeContext (context);
 
	return err;
}

//	____________________________________________________________
//
//	initialize common controls

static VOID 
sInitControls (VOID)
{
	INITCOMMONCONTROLSEX	icex;
	HINSTANCE				hComCtl;


	// Ensure that the common control DLL is loaded.
	InitCommonControls ();

	// Ensure that the custom control DLL is loaded.
	InitTreeListControl ();

	hComCtl = LoadLibrary ("comctl32.dll");
	if (hComCtl) {
 		FARPROC	pInitCommonControlsEx;
  
		pInitCommonControlsEx = 
			GetProcAddress (hComCtl, "InitCommonControlsEx");
   
		if (pInitCommonControlsEx) {

			// Ensure that the extended common controls are loaded
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_BAR_CLASSES|ICC_DATE_CLASSES;
			(*pInitCommonControlsEx)(&icex);
		}

		FreeLibrary(hComCtl);
	}
}

//	____________________________________________________________
//
//	initialize DLL

PGPError PGPclExport 
PGPclInitLibrary (PGPContextRef context) 
{
	PGPError	err;

	err = sCheckComctl32Version ();
	if (IsPGPError (err)) 
		return err;

	err = sCheckMemLockDriver ();
	if (IsPGPError (err)) 
		return err;

	// register the mouse wheel message
	g_uMouseWheelMessage = RegisterWindowMessage (MSH_MOUSEWHEEL);

	sInitControls ();

	PGPInitPassphraseCaches (context);

	return err;
}

//	____________________________________________________________
//
//	close down DLL

PGPError PGPclExport 
PGPclCloseLibrary (VOID) 
{
	PGPClosePassphraseCaches ();

	return kPGPError_NoErr;
}

//	____________________________________________________________
//
//	get PGPkeys path from registry and append Help file name 

static VOID 
sGetHelpFilePath (VOID) 
{
	CHAR sz[128];

	PGPclGetPGPPath (g_szHelpFile, sizeof(g_szHelpFile));

	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (g_szHelpFile, sz);
}


//	____________________
//
//	DLL entry/exit point

BOOL WINAPI 
DllMain (
		 HINSTANCE hInstDll, 
		 DWORD dwReason, 
		 LPVOID lpvReserved) 
{

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstDll;
		sGetHelpFilePath ();
		sRegisterSelectiveImportClass ();
		break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}


