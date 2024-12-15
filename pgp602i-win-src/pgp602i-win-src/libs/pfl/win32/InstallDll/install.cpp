/*------------------------------------------
   browse32.c -- MyGetOpenFileName in DLL.
  ------------------------------------------*/

#include <windows.h>
#include <windef.h>
#include <shlobj.h>
#include <ole2.h>

// Standard initialization function in 32-bit DLLs.
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return (1);
}

extern "C" 
{
int PGPclStartMemLockDriver (VOID);
BOOL WINAPI MyGetOpenFileName32(HWND, LPSTR, LPSTR, LPSTR, LPSTR);
BOOL WINAPI MyBrowseForFolder32 (HWND, LPSTR);
UINT CALLBACK CenterOpenFileName (HWND, UINT, WPARAM, LPARAM);
INT CALLBACK CenterBrowseForFolder (HWND, UINT, LPARAM, LPARAM);
}
//Struct for OPENFILENAME 
static OPENFILENAME ofn;
//Struct for SHBrowseForFolder
static BROWSEINFO bi;
BOOL bResult;

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

			//szClientInstaller = (char *) lParam;
			//SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALL),
			//	szClientInstaller);

			break;
		}
	}
	return TRUE;
}

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
			
			//Error Code Formating
			//if (error == 0)
			//{
			//	LPVOID lpMessageBuffer;
			//	FormatMessage(
			//	FORMAT_MESSAGE_ALLOCATE_BUFFER |
			//	FORMAT_MESSAGE_FROM_SYSTEM,
			//	NULL,
			//	GetLastError(),
			//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			//	(LPTSTR) &lpMessageBuffer,
			//	0,
			//	NULL );

				// display this string
 			//	MessageBox(NULL, (char*)lpMessageBuffer, "",
 			//  MB_OK|MB_ICONWARNING);

				// Free the buffer allocated by the system
			//	LocalFree( lpMessageBuffer );
			//}

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
		}
	return TRUE;
	}

//________________________________________
//
//		Initialize function for
//		MyGetOpenFileName32.

void MyGetOpenFileNameInit ()
	{
	UINT uiMsg;
	HWND hdlg;
	WPARAM wParam;
	LPARAM lParam;

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