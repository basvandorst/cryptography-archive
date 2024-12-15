//____________________________________________________________________________
//	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
//	All rights reserved.
//	
//	install.c -- install library.
//  Author: Philip Nathan
//
//Id: install.c,v 1.1 1999/02/10 00:06:08 philipn Exp $_______________________

#include <windows.h>
#include "install.h"
#include <windef.h>
#include <shlobj.h>
#include <ole2.h>
#include <winsvc.h>
#include <winerror.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <aclapi.h>
#include <io.h>
#include <sys\stat.h>
#include "prototype.h"


// Standard initialization function in 32-bit DLLs.
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return (1);
}

static SECURITY_DESCRIPTOR fileSecurityDescriptor;
static SECURITY_DESCRIPTOR directorySecurityDescriptor;

//Struct for OPENFILENAME 
static OPENFILENAME ofn;
//Struct for SHBrowseForFolder
static BROWSEINFO bi;

//Globals
BOOL bResult;
BOOL bRc;
enum WinVersion WvRc;


//________________________________________
//
//		Callback function to hook into 
//		dialogs and position them centered.

UINT CALLBACK CenterOpenFileName (HWND hdlg,UINT uiMsg,
		WPARAM wParam,LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_INITDIALOG:
		{
			RECT rc;

			// center dialog on screen
			GetWindowRect(GetParent(hdlg), &rc);
			SetWindowPos(GetParent(hdlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

			break;
		}
	}
	return TRUE;
}

//________________________________________
//
//	
//		

INT CALLBACK CenterBrowseForFolder (HWND hdlg,UINT uiMsg,
		LPARAM lParam,LPARAM lpData)
{
int 	error;

	switch(uiMsg)
	{
		case BFFM_INITIALIZED:
		{
			RECT rc;

			// center dialog on screen
			error = GetWindowRect(hdlg, &rc);
			//if (error == 0)
			//	MessageBox(NULL, "ERROR", "Test", MB_OK);

			error = SetWindowPos(hdlg, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			break;
		}
	}
	return TRUE;
}


//________________________________________
//
//		Function to supply us with a 95/NT
//		looking Browse dialog for browseing
//		to folders in Installshield

BOOL WINAPI MyBrowseForFolder32 (HWND hwnd, LPSTR lpszTitle)
{
	LPMALLOC pMalloc;
	LPITEMIDLIST pidlInstallDir;
	char szInstallDir[MAX_PATH];
	int nDirLength;
	SHGetMalloc(&pMalloc);

	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szInstallDir;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = 0;
	bi.lpfn = CenterBrowseForFolder;
	bi.lParam = 0;
	
	pidlInstallDir = SHBrowseForFolder(&bi);

	if (pidlInstallDir != NULL)
	{
		SHGetPathFromIDList(pidlInstallDir, lpszTitle);
		nDirLength = strlen(lpszTitle);
		
		pMalloc->Free(pidlInstallDir);
		return 1;
	}
	else
		return 0;
}


//________________________________________
//
//		Initialize function for
//		MyGetOpenFileName32.

void MyGetOpenFileNameInit ()
	{
	static char *szFilter[] = {"All Files", "*.*", ""} ;

	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = NULL ;
	ofn.hInstance         = NULL ;
	ofn.lpstrFilter       = szFilter [0] ;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter    = 0 ;
	ofn.nFilterIndex      = 0 ;
	ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
	ofn.nMaxFile          = _MAX_PATH ;
	ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT ;
	ofn.lpstrInitialDir   = NULL ;
	ofn.lpstrTitle        = NULL ;
	ofn.Flags             = 0 ;             // Set in Open and Close functions
	ofn.nFileOffset       = 0 ;
	ofn.nFileExtension    = 0 ;
	ofn.lpstrDefExt       = NULL ;
	ofn.lCustData         = 0L ;
	ofn.lpfnHook          = CenterOpenFileName;
	ofn.lpTemplateName    = NULL ;

	//MessageBox(NULL, "In MyGetOpenFileNameInit", "Test", MB_OK);
	}

//________________________________________
//
//		Function to supply us with a 95/NT
//		looking Browse dialog for browseing
//		to files in Installshield.

BOOL WINAPI MyGetOpenFileName32(HWND hwnd, LPSTR lpstrFileFilter, \
								LPSTR lpstrFileName, LPSTR lpstrDlgTitle, \
								LPSTR lpstrExt)
     {
     MyGetOpenFileNameInit();

     if (hwnd)
		ofn.hwndOwner = hwnd;

	 if (lpstrFileFilter[0] != '\0')
     	ofn.lpstrFilter = lpstrFileFilter ;

     ofn.lpstrFile = lpstrFileName ;

     if (lpstrDlgTitle[0] != '\0')
     	ofn.lpstrTitle = lpstrDlgTitle ;

     ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER;

     if (lpstrExt[0] != '\0')
     	ofn.lpstrDefExt = lpstrExt ;

	 OleInitialize(NULL); 

     bResult = GetOpenFileName (&ofn) ;
		
	 OleUninitialize();

	 return bResult;
     }


//________________________________________
//
//		code to start PGPmemlock driver in NT
//		driver must already have been copied to 
//		c:\winnt\system32\drivers directory.

#define DRIVERNAME				"PGPmemlock"
#define DRIVER_NO_ERROR			0
#define DRIVER_NOT_WINNT		1
#define DRIVER_ACCESS_ERROR		2
#define DRIVER_CREATE_FAIL		3
#define DRIVER_ALREADY_STARTED	4
#define DRIVER_MISC_ERROR		5
#define UNKNOWN_ERROR			6

 int PGPclStartMemLockDriver (VOID)
{
	int				err         = UNKNOWN_ERROR;
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

#if PGPCERTD
	//________________________________________
	//
	//		custom DLL functions, used during 
	//		uninstallation,
	//		
			
	int		UninstInitialize	(HWND hwndDlg, HANDLE HInstance, LONG lReserved)
	{
		#define PATHTOCERTDKEY "SOFTWARE\\Network Associates\\PGP Certificate Server"

		LPSTR PathToPrvRing;
		LPSTR PathToPubRing;
		LPSTR PathToCfgFile;
		DWORD dwType;
		DWORD dwSize;
		LPBYTE pValue		= NULL;
		HKEY hMainKey		= HKEY_LOCAL_MACHINE;
		HKEY hOpenKey		= NULL;
		char* Name = "AppPath";	

		//Set all -master files back to READ and WRITE 
		PathToPrvRing = (char*)malloc (_MAX_PATH);
		PathToPubRing = (char*)malloc (_MAX_PATH);
		PathToCfgFile = (char*)malloc (_MAX_PATH);

		if (RegOpenKeyEx(hMainKey, PATHTOCERTDKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
						== ERROR_SUCCESS)
		{
			dwSize = _MAX_PATH;
			pValue = (LPBYTE) calloc(sizeof(BYTE), (DWORD)dwSize);
			
			RegQueryValueEx(hOpenKey, Name, NULL, &dwType, pValue, &dwSize);
			RegCloseKey (hOpenKey);	
		}

		memcpy (PathToPrvRing, pValue, dwSize);
		memcpy (PathToPubRing, pValue, dwSize);
		memcpy (PathToCfgFile, pValue, dwSize);

		strcat (PathToPrvRing, "\\etc\\PGPcertd-secring-master.skr");
		strcat (PathToPubRing, "\\etc\\PGPcertd-pubring-master.pkr");
		strcat (PathToCfgFile, "\\etc\\pgpcertd-master.cfg");

		_chmod (PathToPrvRing, _S_IREAD | _S_IWRITE);
		_chmod (PathToPubRing, _S_IREAD | _S_IWRITE);
		_chmod (PathToCfgFile, _S_IREAD | _S_IWRITE);

		free (PathToPrvRing);
		free (PathToPubRing);
		free (PathToCfgFile);
		free (pValue);

		//Remove IIS 4 entries
		CreateIISVDir (0);

		//Delete Repd and Certd services
		DelSrv(0);
		DelSrv(1);
		return 1;
	}

	void	UninstUnInitialize	(HWND hwndDlg, HANDLE HInstance, LONG lReserved)
	{
		#define ntmemlockdriver "\\Drivers\\PGPmemlock.sys"

		LPSTR lpBuffer;
		LPSTR lpBufferb;
		LPSTR PathtoFile;
		BOOL bIsNT = FALSE;

		//delete the start menu folder, installshield can leave this behind
		lpBuffer = (char*)malloc (_MAX_PATH);
		lpBufferb = (char*)malloc (_MAX_PATH);
		GetWindowsDirectory(lpBuffer, _MAX_PATH);
		strcat (lpBuffer, "\\Profiles\\All Users\\Start Menu\\Programs\\PGP Certificate Server");
		strcpy (lpBufferb, lpBuffer);
		strcat (lpBufferb, "\\Documentation");
		RemoveDirectory(lpBufferb);
		RemoveDirectory(lpBuffer);
		free(lpBuffer);
		free(lpBufferb);
		RefreshStartMenu ();

		//If memlock was removed, delete reg. entries, otherwise start it.
		PathtoFile = (char*)malloc (MAX_PATH);
		GetSystemDirectory(PathtoFile, MAX_PATH);
		strcat (PathtoFile, ntmemlockdriver);

		if (_access(PathtoFile, 00) != 0)
		{
			//Memlock doesnt exist, remove keys.	
			RegDeleteKey(
						HKEY_LOCAL_MACHINE,         
						"SYSTEM\\CurrentControlSet\\Services\\PGPmemlock\\Enum"
						);
			RegDeleteKey(
						HKEY_LOCAL_MACHINE,         
						"SYSTEM\\CurrentControlSet\\Services\\PGPmemlock"
						);
		}
		else
		{
			//Memlock driver exists, start it.
			PGPclStartMemLockDriver ();
		}

		free(PathtoFile);
	}
#else
	//STUBS
	//________________________________________
	//
	//		custom DLL functions, used during 
	//		uninstallation,
	//	
	int		UninstInitialize	(HWND hwndDlg, HANDLE HInstance, LONG lReserved)
	{
		return 0;
	}

	void	UninstUnInitialize	(HWND hwndDlg, HANDLE HInstance, LONG lReserved)
	{	
	}
#endif

/*_____________________________________________________________________________
 * 	CreateAndWait
 */
BOOL CreateAndWait (char* CommandLine, LPSTR CmdExe, LPSTR ERRORSTRING)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO			si;
	int					error;
	

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	strcat(CommandLine, CmdExe);

	GetShortPathName(CommandLine, CommandLine, strlen(CommandLine) + 1);

	#if PGPDEBUG	
		MessageBox (GetFocus(), CommandLine, "DEBUGMODE", 0 | MB_ICONHAND);
	#endif

	error = CreateProcess(NULL, CommandLine, NULL, NULL,
						FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

	if (error == 0)
	{
		MessageBox (GetFocus(), ERRORSTRING, "ERROR", 0 | MB_ICONERROR);
		return FALSE;		
	}/*if*/

	/* wait at this line until launched program ends */
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return TRUE;
}/*end*/

//________________________________________
//
//		Code to stop a running service so it		
//		can be removed
//

INT StopService (LPSTR ServiceName)
{
SERVICE_STATUS STATUS;
SC_HANDLE hSCManager;
SC_HANDLE hService;
int iReturn;

hSCManager = NULL;
hService = NULL;
iReturn = false;

hSCManager = OpenSCManager	(// pointer to machine name string
							NULL,
							// pointer to database name string
							NULL,  
							// type of access	
							SC_MANAGER_ALL_ACCESS   
							);


hService = OpenService	(// handle to service control manager db
						hSCManager,
						// pointer to name of service to start
						ServiceName, 
						SERVICE_ALL_ACCESS  // type of access to service
						);

iReturn = ControlService(// handle to service
						hService,
						// control code  LPSERVICE_STATUS lpServiceStatus
						SERVICE_CONTROL_STOP, 
						// pointer to service status structure		
						&STATUS				
						);


CloseServiceHandle (hService);
CloseServiceHandle (hSCManager);

return iReturn;
}

//________________________________________
//
//		Code to refresh the startmenu so 		 
//		changes are reflected
//

void RefreshStartMenu ()
{
	IMalloc *pMalloc;

	if(SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		LPITEMIDLIST pidl;

		if(SUCCEEDED(SHGetSpecialFolderLocation((HWND)NULL, CSIDL_STARTMENU, &pidl)))
		{
			SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidl, NULL);
			pMalloc -> Free(pidl);
		}
		pMalloc -> Release();
	}
}

//________________________________________
//
//		Code to remove repd and certd		 
//		services
//
void DelSrv (int arg)
{
	#define	PGP_SERVICE_NAME "PGP Certificate Server"
	#define PGP_REPDSERVICE_NAME "PGP Replication Engine"

	SC_HANDLE service, scm;
	int	ok;
	SERVICE_STATUS stat;
	BOOL	doCertd = FALSE;
	BOOL	doRepd = FALSE;

		
	/* Initialize syslog/event error handling. */
//	ErrorLogInit("pgpcertd", g->eErrorLogLevel, g->iPort, 1);
	switch (arg) {
		case 0:	/* Run as a command line app. */
			doCertd = TRUE;
			break;
		case 1:	/* Run as a command line app. */
			doRepd = TRUE;	
			break;
		break;
	}

	if ( (doRepd == FALSE) && (doCertd == FALSE) )
	{
		MessageBox( NULL, "bad parm delsrv", "DelSrv",MB_OK | MB_ICONERROR); 
		return;
	}

	//fprintf(stdout, "Starting...\n");
	scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	//if (!scm)
	//	errorHandler ("Error in OpenScManager", GetLastError());

	/* Uninstall PGPcertd service
	 */
	if (doCertd == TRUE)
	{
		// Get the service handle
		service = OpenService(scm, PGP_SERVICE_NAME, SERVICE_ALL_ACCESS | DELETE);
		if (!service)
		{
		//	MessageBox( NULL, "Error opening Service Control Manager.",
		//			"",MB_OK | MB_ICONERROR);
		return;
		}
			
		// Stop the service
		ok = QueryServiceStatus (service, &stat);
		//if (!ok)
		//	errorHandler ("Error in QueryServiceStatus of PGPcertd", GetLastError());
			
		if (stat.dwCurrentState != SERVICE_STOPPED)
		{
			//sprintf(stdout, "Stopping Service...\n");
			ok = ControlService(service, SERVICE_CONTROL_STOP, &stat);
			//if (!ok)
			//	errorHandler ("Error in ControlService of PGPcertd", GetLastError());
			Sleep(500);	
		}

		ok = DeleteService( service);
		//if (!ok)
		//	errorHandler("Error in Delete Service of PGPcertd", GetLastError());
		//else
		//{
		//	Sleep(1);
		//}
			
		CloseServiceHandle (service);	
	}

	/* Uninstall PGPrepd service
	 */
	if (doRepd == TRUE)
	{
		// Get the service handle
		service = OpenService(scm, PGP_REPDSERVICE_NAME, SERVICE_ALL_ACCESS | DELETE);
		//if (!service)
		//	errorHandler ("Error in OpenService of PGPrepd", GetLastError());
			
		// Stop the service
		ok = QueryServiceStatus (service, &stat);
		//if (!ok)
		//	errorHandler ("Error in QueryServiceStatus of PGPrepd", GetLastError());
			
		if (stat.dwCurrentState != SERVICE_STOPPED)
		{
			//fprintf(stdout, "Stopping Service...\n");
			ok = ControlService(service, SERVICE_CONTROL_STOP, &stat);
			//if (!ok)
			//	errorHandler ("Error in ControlService of PGPrepd", GetLastError());
			Sleep(500);	
		}

		ok = DeleteService( service);
		if (!ok)
			Sleep(1);
		else
		{
			Sleep(1);
		}
			
		CloseServiceHandle (service);	
	}

	CloseServiceHandle (scm);
	return;
	//fprintf(stdout, "Ending...\n");
}

void AddSrv (int arg)
{
	#define PGP_REPDEXEC_NAME "PGPrepd.exe"
	#define PGP_CERTDEXEC_NAME "PGPcertd.exe"

	SC_HANDLE service, scm;
    BYTE Filename[512];	// pointer to buffer for module path 
	char *pChr;
	BOOL	doCertd = FALSE;
	BOOL	doRepd = FALSE;


	/* Initialize syslog/event error handling. */
//	ErrorLogInit("pgpcertd", g->eErrorLogLevel, g->iPort, 1);

	switch (arg) {
		case 0:	/* Run as a command line app. */
			doCertd = TRUE;	
			break;
		case 1:	/* Run as a command line app. */
			doRepd = TRUE;	
			break;
		break;
	}

	if ( (doRepd == FALSE) && (doCertd == FALSE) )
	{
		MessageBox( NULL, "bad parm addsrv", "AddSrv",MB_OK | MB_ICONERROR);
		return;
	}

	//fprintf(stdout, "Starting...\n");
	scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (!scm)
	{
		//MessageBox( NULL, "Error opening Service Control Manager.",
		//			"",MB_OK | MB_ICONERROR);
		return;
	}
		
 
	/* Install PGPcertd as a service
	 */
	if (doCertd == TRUE)
	{
		memset(Filename, 0, sizeof(Filename));
		GetModuleFileName(NULL,(char*)Filename,512);
  		if (pChr = strrchr((char*)Filename, '\\'))
			*pChr = '\0';
		strcat((char*)Filename, "\\");
		strcat((char*)Filename, PGP_CERTDEXEC_NAME);

		/* Enable the application's GUI, have the service start manually 
		 */
		service = CreateService( scm, PGP_SERVICE_NAME, PGP_SERVICE_NAME, SERVICE_ALL_ACCESS,
								SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
								SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, (char*)Filename,
								NULL, NULL, NULL, NULL, NULL);
		if (!service)
		{
			MessageBox( NULL, "Can't create Service.(It may already exist)",
						"",MB_OK | MB_ICONERROR);
			return;
		}
		CloseServiceHandle (service);
	}

	/* Install PGPrepd as a service
	 */
	if (doRepd == TRUE)
	{
		memset(Filename, 0, sizeof(Filename));
		GetModuleFileName(NULL, (char*)Filename,512);
   		if (pChr = strrchr((char*)Filename, '\\'))
			*pChr = '\0';
		strcat((char*)Filename, "\\");
		strcat((char*)Filename, PGP_REPDEXEC_NAME);

		/* Enable the application's GUI, have the service start manually 
		 */
		service = CreateService( scm, PGP_REPDSERVICE_NAME, PGP_REPDSERVICE_NAME, SERVICE_ALL_ACCESS,
								SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
								SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, (char*)Filename,
								NULL, NULL, NULL, NULL, NULL);
		if (!service)
			Sleep(1);
		else
		{
			Sleep(1);
		}
			
		CloseServiceHandle (service);
	}


	CloseServiceHandle (scm);
	return;
	
}

//________________________________________
//
//		ErrorHandler routine		 
//		
//
void errorHandler (char *s, DWORD err)
{
	char *errtext;

	errtext = (char*)malloc (strlen (s) + 25);

	sprintf(errtext, "%s error number= %d\n", s, err);

	MessageBox( NULL, errtext, "err",MB_OK | MB_ICONERROR); 
	free (errtext);
	return;	
}

//________________________________________
//
//		Code to start a stopped service				
//

INT StartServ (LPSTR ServiceName)
{
SERVICE_STATUS STATUS;
SC_HANDLE hSCManager;
SC_HANDLE hService;
int iReturn;

hSCManager = NULL;
hService = NULL;
iReturn = false;

/*#define SERVICE_STOPPED                0x00000001
#define SERVICE_START_PENDING          0x00000002
#define SERVICE_STOP_PENDING           0x00000003
#define SERVICE_RUNNING                0x00000004
#define SERVICE_CONTINUE_PENDING       0x00000005
#define SERVICE_PAUSE_PENDING          0x00000006
#define SERVICE_PAUSED                 0x00000007*/

hSCManager = OpenSCManager	(// pointer to machine name string
							NULL,
							// pointer to database name string
							NULL,  
							// type of access	
							SC_MANAGER_ALL_ACCESS   
							);

hService = OpenService	(// handle to service control manager db
						hSCManager,
						// pointer to name of service to start
						ServiceName, 
						// type of access to service
						SERVICE_ALL_ACCESS    
						);
			StartService (// handle to service control manager db
						hService,
						// number of sevice arguments
						0,
						// ServiceArgsVector
						NULL);

while( QueryServiceStatus (hService, &STATUS) )
{
	if ((STATUS.dwCurrentState) != (SERVICE_START_PENDING))
		break;
	Sleep (1250);
}


CloseServiceHandle (hService);
CloseServiceHandle (hSCManager);

if(STATUS.dwCurrentState == SERVICE_RUNNING)
	return 1;
else
	return 0;
}

#if PGPCERTD
/* Find out the account or group name even if renamed or internationalized. */
static BOOL GetAccountName(char *szAcctName, DWORD cbAcctName, SID_IDENTIFIER_AUTHORITY *sia,
    BYTE subauthorityCount, DWORD subauthority)
{
    CHAR szDomainName[81];
    DWORD cbDomainName = sizeof szDomainName - 1;
    PSID pSID;
    SID_NAME_USE eSNU;
    if (AllocateAndInitializeSid(sia, subauthorityCount, subauthority, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &pSID)) 
    {
        if (LookupAccountSid(NULL, pSID, szAcctName, &cbAcctName, szDomainName, &cbDomainName, &eSNU))
        {
            FreeSid(pSID);
            return TRUE;
        }
    }
    return FALSE;
}

/* A reusable function for finding out the administrators group name even if
   renamed or internationalized. This routine allocates static storage for the
   name. This is necessary in order to use the name with BuildExplicitAccessWithName. */
char *GetAdminGroupName()
{
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    static char name[81];

    if (!GetAccountName(name, sizeof name - 1, &ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID))
        return NULL;
    return name;
}

/* A reusable function for finding out the system account name even if
   renamed or internationalized. This routine allocates static storage for the
   name. This is necessary in order to use the name with BuildExplicitAccessWithName. */
char *GetSystemAcctName()
{
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    static char name[81];

    if (!GetAccountName(name, sizeof name - 1, &ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID))
        return NULL;
    return name;
}

/* A reusable function for finding out the creator owner name even if
   renamed or internationalized. This routine allocates static storage for the
   name. This is necessary in order to use the name with BuildExplicitAccessWithName. */
char *GetCreatorOwnerName()
{
    SID_IDENTIFIER_AUTHORITY creatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    static char name[81];

    if (!GetAccountName(name, sizeof name - 1, &creatorAuthority, 1, SECURITY_CREATOR_OWNER_RID))
        return NULL;
    return name;
}

static BOOL TraverseAndSecureDirectoryTree(char* szRootDir)
{
    HANDLE hndFileFound;
    WIN32_FIND_DATA fdFileFound;
    BOOL bReturn = FALSE;
    char szCurDir[_MAX_PATH + 1];
    char szNewDir[_MAX_PATH + 1];

    if (strcmp(szRootDir, "..") == 0 || strcmp(szRootDir, ".") == 0)
        return TRUE;

    // Save the current directory so that we can restore it eventually
    if (GetCurrentDirectory(_MAX_PATH, szCurDir) == 0)
        return FALSE;

    // Start us off by making the specified root directory our working root
    if (SetCurrentDirectory(szRootDir) == FALSE)
        goto Return;

    if (GetCurrentDirectory(_MAX_PATH, szNewDir) == 0)
        goto Return;

    /* We could use SetNamedSecurityInfo here instead, which would be simpler,
       but it runs amazingly slow for some reason. */
    if (!SetFileSecurity(szNewDir, DACL_SECURITY_INFORMATION, &directorySecurityDescriptor))
        goto Return;

    hndFileFound = FindFirstFile("*.*", &fdFileFound);

    while (hndFileFound != INVALID_HANDLE_VALUE)
    {
        if (fdFileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!TraverseAndSecureDirectoryTree(fdFileFound.cFileName))
                goto Return;
        }
        else 
            if (!SetFileSecurity(fdFileFound.cFileName, DACL_SECURITY_INFORMATION, &fileSecurityDescriptor))
                goto Return;

        if (FindNextFile(hndFileFound, &fdFileFound) == FALSE)
            break;
    }

    // We made it this far, so we're cleaning up after successful run
    bReturn = TRUE;
    FindClose(hndFileFound);

Return:
    // Restore the initially current directory
    if (SetCurrentDirectory(szCurDir)  == FALSE)
        return FALSE;

    return bReturn;
}

/* This is intended as a reusable function for setting the security of a
   directory and all of its subdirectories and files. The security for
   directories is specified by dirAccesses, for files by fileAccesses. The
   caller must build using BuildExplicitAccessWithName or other means. */
BOOL SecureDirectoryTree(char *szDirRoot, EXPLICIT_ACCESS *dirAccesses, DWORD dirAccessesCount,
    EXPLICIT_ACCESS *fileAccesses, DWORD fileAccessesCount)
{
    BOOL wasSuccessful;
    PACL fileAcl;
    PACL directoryAcl;

    /* Create an ACL for files and for directories from the two explicit access
       structures. */
    if (SetEntriesInAcl(fileAccessesCount, fileAccesses, NULL, &fileAcl) != ERROR_SUCCESS)
        return FALSE;
    if (SetEntriesInAcl(dirAccessesCount, dirAccesses, NULL, &directoryAcl) != ERROR_SUCCESS)
    {
        LocalFree(fileAcl);
        return FALSE;
    }
    if (!InitializeSecurityDescriptor(&fileSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
        return FALSE;
    if (!SetSecurityDescriptorDacl(&fileSecurityDescriptor, TRUE, fileAcl, FALSE))
        return FALSE;
    if (!InitializeSecurityDescriptor(&directorySecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
        return FALSE;
    if (!SetSecurityDescriptorDacl(&directorySecurityDescriptor, TRUE, directoryAcl, FALSE))
        return FALSE;

    /* As the directories are traversed, set the directories and files to the
       desired acceses. */
    wasSuccessful = TraverseAndSecureDirectoryTree(szDirRoot);
    LocalFree(fileAcl);
    LocalFree(directoryAcl);
    return wasSuccessful;
}

__declspec(dllexport)
BOOL CALLBACK SetPermissions(char* szDirRoot)
{
    DWORD dwDummy;
    char szFileSystem[41];
    DWORD cbFS = 40;
    char szDrive[] = "x:\\";
    EXPLICIT_ACCESS fileAccesses[3];
    EXPLICIT_ACCESS dirAccesses[3];
    char *name;

    // Don't do this for "FAT" or "FAT32" file systems.
    szDrive[0] = szDirRoot[0];
    if ( szDrive[0] != '\\' ) 
        {
        if (GetVolumeInformation(szDrive, NULL, 0, NULL, &dwDummy, &dwDummy, szFileSystem, cbFS) == FALSE)
            return FALSE;
        if (strnicmp(szFileSystem, "FAT", strlen("FAT")) == 0)
            return TRUE;
        };

    /* Get the names of the account and group names to set access for, even if
       if they are renamed or internationalized. Build explicit access
       structures for each of them to define their permissions. */
    name = GetAdminGroupName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&fileAccesses[0], name, GENERIC_ALL, SET_ACCESS, NO_INHERITANCE);
    BuildExplicitAccessWithName(&dirAccesses[0], name, GENERIC_ALL, SET_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    name = GetSystemAcctName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&fileAccesses[1], name, GENERIC_ALL, SET_ACCESS, NO_INHERITANCE);
    BuildExplicitAccessWithName(&dirAccesses[1], name, GENERIC_ALL, SET_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    name = GetCreatorOwnerName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&fileAccesses[2], name, GENERIC_ALL, SET_ACCESS, NO_INHERITANCE);
    BuildExplicitAccessWithName(&dirAccesses[2], name, GENERIC_ALL, SET_ACCESS, SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    return SecureDirectoryTree(szDirRoot, dirAccesses, 3, fileAccesses, 3);
}

static PACL regAcl;
static HKEY rootKey;

static BOOL TraverseAndSecureKeys(LPTSTR keyName)
{
    HKEY key;
    DWORD index;
    BOOL wasSuccessful;
    LONG retCode;
    TCHAR subKeyName[MAX_PATH + 1];
    TCHAR nextKeyName[MAX_PATH + 1];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        return FALSE;
    __try
    {
        wasSuccessful = SetSecurityInfo(key, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, NULL, NULL,
            regAcl, NULL) == ERROR_SUCCESS;
        if (!wasSuccessful)
            __leave;
        for (index = 0; ; ++index)
        {
            retCode = RegEnumKey(key, index, subKeyName, sizeof subKeyName);
            if (retCode == ERROR_NO_MORE_ITEMS)
                __leave;
            wasSuccessful = retCode == ERROR_SUCCESS;
            if (!wasSuccessful)
                __leave;
            strcpy(nextKeyName, "");
            strncat(nextKeyName, keyName, sizeof nextKeyName);
            strncat(nextKeyName, "\\", sizeof nextKeyName);
            strncat(nextKeyName, subKeyName, sizeof nextKeyName);
            wasSuccessful = TraverseAndSecureKeys(nextKeyName);
            if (!wasSuccessful)
                __leave;
        }
    }
    __finally
    {
        RegCloseKey(key);
    }
    return wasSuccessful;
}

/* This is intended as a reusable function for setting the security of a
   registry key and all of its sub keys. The security is specified by accesses,
   which the caller must build using BuildExplicitAccessWithName or other
   means. */
BOOL SecureRegistryTree(HKEY aRootKey, char *keyName, EXPLICIT_ACCESS *accesses, DWORD accessesCount)
{
    BOOL wasSuccessful;

    /* Create an ACL that contains the access permissions. */
    if (SetEntriesInAcl(accessesCount, accesses, NULL, &regAcl) != ERROR_SUCCESS)
        return FALSE;
    /* Traverse the reg key tree, setting all key's security to regAcl. */
    rootKey = aRootKey;
    wasSuccessful = TraverseAndSecureKeys(keyName);
    LocalFree(regAcl);
    return wasSuccessful;
}

__declspec(dllexport)
BOOL CALLBACK SetRegPermissions(DWORD rootKeyAsDWord, char* keyName)
{
    EXPLICIT_ACCESS accesses[3];
    char *name;

    /* Build an explicit access structure for admin group. */
    name = GetAdminGroupName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&accesses[0], name, KEY_ALL_ACCESS, SET_ACCESS,
        SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    /* Build an explicit access structure for system account. */
    name = GetSystemAcctName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&accesses[1], name, KEY_ALL_ACCESS, SET_ACCESS,
        SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    /* Build an explicit access structure for creator owner. */
    name = GetCreatorOwnerName();
    if (name == NULL)
        return FALSE;
    BuildExplicitAccessWithName(&accesses[2], name, KEY_ALL_ACCESS, SET_ACCESS,
        SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    /* Traverse the reg key tree, setting all key's security. */
    return SecureRegistryTree((HKEY)rootKeyAsDWord, keyName, accesses, 3);
}
#else
__declspec(dllexport)
BOOL CALLBACK SetRegPermissions(DWORD rootKeyAsDWord, char* keyName)
{
    return 0;
}

__declspec(dllexport)
BOOL CALLBACK SetPermissions(char* szDirRoot)
{
    return 0;
}
#endif