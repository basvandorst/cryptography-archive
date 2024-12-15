/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLprefs.c - handle PGP preferences dialogs
	

	$Id: CLprefs.c,v 1.44 1998/08/11 14:43:11 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"
#include <direct.h>

// project header files
#include "pgpclx.h"

// system header files
#include <commdlg.h>

#define MAXCACHETIMELIMIT		360000
#define MAXWIPEPASSES			32

#define PREFSEMAPHORENAME	("PGPprefsInstSem")

// default preferences
#define DEFAULT_ENCRYPTTOSELF			FALSE
#define DEFAULT_DECRYPTCACHEENABLE		TRUE
#define DEFAULT_DECRYPTCACHESECONDS		120
#define DEFAULT_SIGNCACHEENABLE			FALSE
#define DEFAULT_SIGNCACHESECONDS		120
#define DEFAULT_USEFASTKEYGEN			TRUE
#define	DEFAULT_WIPECONFIRM				TRUE
#define DEFAULT_MAILENCRYPTPGPMIME		FALSE
#define DEFAULT_WORDWRAPWIDTH			70
#define DEFAULT_WORDWRAPENABLE			TRUE
#define DEFAULT_MAILENCRYPTDEFAULT		FALSE
#define DEFAULT_MAILSIGNDEFAULT			FALSE
#define DEFAULT_AUTODECRYPT				FALSE
#define DEFAULT_KEYSERVERSYNCONUNKNOWN	FALSE
#define DEFAULT_KEYSERVERSYNCONADDNAME	FALSE
#define DEFAULT_KEYSERVERSYNCONKEYSIGN	FALSE
#define DEFAULT_KEYSERVERSYNCONREVOKE	FALSE
#define DEFAULT_KEYSERVERSYNCONVERIFY	FALSE
#define DEFAULT_PREFERREDALGORITHM		kPGPCipherAlgorithm_CAST5
#define DEFAULT_DISPLAYMARGVALIDITY		TRUE
#define DEFAULT_MARGVALIDISINVALID		TRUE
#define DEFAULT_WARNONADK				TRUE
#define DEFAULT_WIPEPASSES				1
#define DEFAULT_EXPORTCOMPATIBLE		TRUE

#define COMMENTBLOCK_MAX_LENGTH			60

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static PGPContextRef	ContextRef;

static HWND				hWndParent;
static HWND				hWndTree;
static PGPKeySetRef		keysetMain;
static PGPError			returnErr;
static BOOL				bNeedsCentering;
static BOOL				bPrefsWritten;
static BOOL				bReloadKeyring;
static BOOL				bKeyserverPrefsWritten;
static PGPPrefRef		PrefRefClient;
static PGPPrefRef		PrefRefAdmin;
static CHAR				szInitDir[MAX_PATH];

typedef struct {
	HWND		hwndDecryptDuration;
	SYSTEMTIME	stDecryptDuration;
	HWND		hwndSignDuration;
	SYSTEMTIME	stSignDuration;
} CACHEDURATIONSTRUCT, *PCACHEDURATIONSTRUCT;


static DWORD aGeneralIds[] = {			// Help IDs
    IDC_ENCRYPTTOSELF,		IDH_PGPCLPREF_ENCRYPTTOSELF, 
	IDC_USEFASTKEYGEN,		IDH_PGPCLPREF_USEFASTKEYGEN,
	IDC_DECRYPTCACHEENABLE,	IDH_PGPCLPREF_PASSCACHEENABLE,
	IDC_DECRYPTCACHETIME,	IDH_PGPCLPREF_PASSCACHETIME,
	IDC_SIGNCACHEENABLE,	IDH_PGPCLPREF_SIGNCACHEENABLE,
	IDC_SIGNCACHETIME,		IDH_PGPCLPREF_SIGNCACHETIME,
	IDC_COMMENTBLOCK,		IDH_PGPCLPREF_COMMENTBLOCK,
	IDC_WIPECONFIRM,		IDH_PGPCLPREF_WIPECONFIRM,
	IDC_NUMPASSES,			IDH_PGPCLPREF_NUMWIPEPASSES,
	IDC_NUMPASSESSPIN,		IDH_PGPCLPREF_NUMWIPEPASSESSPIN,
	IDC_NUMPASSESTEXT,		IDH_PGPCLPREF_NUMWIPEPASSES,
	0,0
};

static DWORD aFileIds[] = {			// Help IDs
    IDC_PRIVATEKEYRING,		IDH_PGPCLPREF_PRIVATEKEYRING, 
	IDC_SETPRIVATEKEYRING,	IDH_PGPCLPREF_BROWSEPRIVATEKEYRING,
    IDC_PUBLICKEYRING,		IDH_PGPCLPREF_PUBLICKEYRING, 
	IDC_SETPUBLICKEYRING,	IDH_PGPCLPREF_BROWSEPUBLICKEYRING,
	IDC_RNGSEEDFILE,		IDH_PGPCLPREF_RANDOMSEEDFILE,
	IDC_SETRNGSEEDFILE,		IDH_PGPCLPREF_BROWSERANDOMSEEDFILE,
	0,0
};

static DWORD aEmailIds[] = {			// Help IDs
	IDC_USEPGPMIME,		IDH_PGPCLPREF_USEPGPMIME,
	IDC_ENCRYPTBYDEFAULT,	IDH_PGPCLPREF_ENCRYPTBYDEFAULT,
	IDC_SIGNBYDEFAULT,		IDH_PGPCLPREF_SIGNBYDEFAULT,
	IDC_AUTODECRYPT,		IDH_PGPCLPREF_AUTODECRYPT,
	IDC_WRAPENABLE,			IDH_PGPCLPREF_WORDWRAPENABLE,
	IDC_WRAPNUMBER,			IDH_PGPCLPREF_WORDWRAPCOLUMN,
	0,0
};

static DWORD aServerIds[] = {			// Help IDs
	IDC_NEWKEYSERVER,		IDH_PGPCLPREF_CREATESERVER,
	IDC_REMOVEKEYSERVER,	IDH_PGPCLPREF_REMOVESERVER,
	IDC_EDITKEYSERVER,		IDH_PGPCLPREF_EDITSERVER,
	IDC_SETASROOT,			IDH_PGPCLPREF_SETROOTSERVER,
	IDC_SYNCONUNKNOWN,		IDH_PGPCLPREF_SYNCONUNKNOWNKEYS,
	IDC_SYNCONADDNAME,		IDH_PGPCLPREF_SYNCONADDNAME,
	IDC_SYNCONKEYSIGN,		IDH_PGPCLPREF_SYNCONSIGN,
	IDC_SYNCONREVOKE,		IDH_PGPCLPREF_SYNCONREVOKE,
	IDC_SYNCONVERIFY,		IDH_PGPCLPREF_SYNCONVERIFY,
	IDC_KSTREELIST,			IDH_PGPCLPREF_SERVERLIST,
	0,0
};

static DWORD aAdvancedIds[] = {			// Help IDs
	IDC_PREFERREDALG,		IDH_PGPCLPREF_PREFERREDALG,
	IDC_ENABLECAST,			IDH_PGPCLPREF_ENABLECAST,
	IDC_ENABLE3DES,			IDH_PGPCLPREF_ENABLE3DES,
	IDC_ENABLEIDEA,			IDH_PGPCLPREF_ENABLEIDEA,
	IDC_DISPMARGVALIDITY,	IDH_PGPCLPREF_DISPMARGVALIDITY,
	IDC_MARGVALIDISINVALID,	IDH_PGPCLPREF_WARNONMARGINAL,
	IDC_WARNONADK,			IDH_PGPCLPREF_WARNONADK,
	IDC_EXPORTCOMPATIBLE,	IDH_PGPCLPREF_EXPORTCOMPATIBLE,
	IDC_EXPORTCOMPLETE,		IDH_PGPCLPREF_EXPORTCOMPLETE,
    0,0 
}; 

//	______________________________________________
//
//  Get standard client preferences file reference

PGPError PGPclExport
PGPclOpenClientPrefs (
		PGPMemoryMgrRef	memMgr, 
		PGPPrefRef*		pPrefRef) 
{
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PGPError		err;
	PFLFileSpecRef	FileRef;
	PGPPrefRef		ClientPrefRef;
	
	err = PGPclGetPGPPath (szPath, sizeof(szPath));
	if (IsPGPError (err)) {
		GetWindowsDirectory (szPath, sizeof(szPath));
		if (szPath[lstrlen(szPath)-1] != '\\') lstrcat (szPath, "\\");
	}

	LoadString (g_hInst, IDS_CLIENTPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	PFLNewFileSpecFromFullPath (memMgr, szPath, &FileRef);

	err = PGPOpenPrefFile (FileRef, clientDefaults, clientDefaultsSize, 
			&ClientPrefRef);
	if ((err == kPGPError_FileOpFailed) ||
		(err == kPGPError_FileNotFound)) {
		err = PFLFileSpecCreate (FileRef);
		pgpAssert (IsntPGPError (err));
		err = PGPOpenPrefFile(FileRef, clientDefaults, clientDefaultsSize,
				&ClientPrefRef);
		pgpAssert (IsntPGPError (err));
	}
	PFLFreeFileSpec (FileRef);

	if (IsPGPError (err)) {
		if (PGPRefIsValid (ClientPrefRef)) 
			PGPClosePrefFile (ClientPrefRef);
	}
	else {	
		*pPrefRef = ClientPrefRef;
	}

	return err;
}

//	______________________________________________
//
//  Get standard admin preferences file reference

PGPError PGPclExport
PGPclOpenAdminPrefs (
		PGPMemoryMgrRef	memMgr,
		PGPPrefRef*		pPrefRef, 
		BOOL			bLoadDefaults) 
{
	PGPError		err					= kPGPError_NoErr;
	PGPPrefRef		AdminPrefRef		= kInvalidPGPPrefRef;
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PFLFileSpecRef	FileRef;
	DWORD			dw;

	err = PGPclGetPGPPath (szPath, sizeof(szPath));
	if (IsPGPError (err)) {
		GetWindowsDirectory (szPath, sizeof(szPath));
		if (szPath[lstrlen(szPath)-1] != '\\') lstrcat (szPath, "\\");
	}

	LoadString (g_hInst, IDS_ADMINPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	// check for file existence
	dw = GetFileAttributes (szPath);
	if ((dw == 0xFFFFFFFF) || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
		if (!bLoadDefaults)
			err = kPGPError_FileNotFound;
	}

	if (IsntPGPError (err)) {
		PFLNewFileSpecFromFullPath (memMgr, szPath, &FileRef);

		err = PGPOpenPrefFile (FileRef, adminDefaults, adminDefaultsSize,
				&AdminPrefRef);

		PFLFreeFileSpec (FileRef);
	}

	if (IsPGPError (err)) {
		PGPclMessageBox (NULL, IDS_CAPTIONERROR, IDS_NOADMINFILE,
									MB_OK|MB_ICONSTOP);
		if (PGPRefIsValid (AdminPrefRef)) 
			PGPClosePrefFile (AdminPrefRef);
	}
	else {	
		*pPrefRef = AdminPrefRef;
	}

	return err;
}

//	_______________________________________
//
//  Save and close down client preferences file

PGPError PGPclExport
PGPclCloseClientPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave) 
{
	PGPError		err;

	if (bSave) {
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	err = PGPClosePrefFile (PrefRef);
	pgpAssert (IsntPGPError(err));

	return err;
}

//	_______________________________________
//
//  Save and close down admin preferences file

PGPError PGPclExport
PGPclCloseAdminPrefs (
		PGPPrefRef	PrefRef, 
		BOOL		bSave) 
{
	PGPError		err;

	if (bSave) {
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	err = PGPClosePrefFile (PrefRef);
	pgpAssert (IsntPGPError(err));

	return err;
}

//	____________________________
//
//  Check for illegal file names

static BOOL 
sIsFileNameOK (LPSTR sz) 
{
	UINT	uLen;
	DWORD	dwAttrib;

	uLen = lstrlen (sz);

	if (uLen < 4) return FALSE;
	if ((sz[1] != ':') && (sz[1] != '\\')) return FALSE;
	if ((sz[1] == ':') && (sz[2] != '\\')) return FALSE;
	if (strcspn (sz, "/*?""<>|") != uLen) return FALSE;
	if (sz[uLen-1] == ':') return FALSE;
	if (sz[uLen-1] == '\\') return FALSE;

	dwAttrib = GetFileAttributes (sz);
	if (dwAttrib == 0xFFFFFFFF) return TRUE;	// file doesn't exist
	if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) return FALSE;
	if (dwAttrib & FILE_ATTRIBUTE_SYSTEM) return FALSE;

	return TRUE;
}

//	_____________________________________________________
//
//  verify that file or backup of file exists.  
//	if not, replace with default file name in install dir
//	if no install dir found, use Windows dir

static VOID
sVerifyFileExists (LPSTR lpszFile, PGPBoolean KeyRing)
{
	CHAR	sz[MAX_PATH];
	DWORD	dwAttrib;
	CHAR*	p;

	dwAttrib = GetFileAttributes (lpszFile);
	if ((dwAttrib == 0xFFFFFFFF) || 
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		lstrcpy (sz, lpszFile);
		p = strrchr (sz, '\\');
		if (p) p++;
		else p = sz;

		if (IsPGPError (PGPclGetPGPPath (lpszFile, MAX_PATH))) 
			GetWindowsDirectory (lpszFile, MAX_PATH);
		else
		{
			if(KeyRing)
			{
				// We don't really want install directory.. we want 
				// PGP Keyrings subdirectory. Otherwise we'll just 
				// take windows like everything else -wjb

				CHAR	sz2[128];

				if (lpszFile[lstrlen (lpszFile) -1] != '\\') 
					lstrcat (lpszFile, "\\");

				LoadString (g_hInst, IDS_KEYRINGFOLDER, sz2, sizeof(sz2));
				lstrcat (lpszFile, sz2);

				_mkdir (lpszFile); // just in case user deleted it
			}
		}

		if (lpszFile[lstrlen (lpszFile) -1] != '\\') 
			lstrcat (lpszFile, "\\");
		lstrcat (lpszFile, p);
	}
}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sFilePropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szRing[MAX_PATH];
	CHAR			szTemp[MAX_PATH];
	CHAR			szFilter[128];
	CHAR			szTitle[128];
	CHAR			szDefExt[8];
	CHAR*			p;
	HWND			hParent;
	RECT			rc;
	PGPError		err;
	PGPFileSpecRef	fileRef;
	LPSTR			lpsz;
	HANDLE			hfile;
	
	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_PublicKeyring,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, TRUE);
			SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_PrivateKeyring,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, TRUE);
			SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_RandomSeedFile,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			sVerifyFileExists (szTemp, FALSE);
			SetDlgItemText (hDlg, IDC_RNGSEEDFILE, szTemp);
		}

		szInitDir[0] = '\0';

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aFileIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aFileIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_SETPRIVATEKEYRING :
			GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_PRIVKEYRINGFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_PRIVKEYCAPTION, szTitle, 
								sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPRIVRINGEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;

		case IDC_SETPUBLICKEYRING :
			GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_PUBKEYRINGFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_PUBKEYCAPTION, szTitle, sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPUBRINGEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;

		case IDC_SETRNGSEEDFILE :
			GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, sizeof(szFile));
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				szFile[0] = '\0';
			}
			else {
				if (szInitDir[0])
					szFile[0] = '\0';
			}
			LoadString (g_hInst, IDS_SEEDFILEFILTER, szFilter, 
								sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			LoadString (g_hInst, IDS_SEEDFILECAPTION, szTitle, 
								sizeof(szTitle));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hDlg;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle        = szTitle;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFSEEDFILEEXT, szDefExt, 
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile);
				if (OpenFileName.nFileOffset > 0) 
					szFile[OpenFileName.nFileOffset-1] = '\0';
				lstrcpy (szInitDir, szFile);
			}
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {

		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_FILEDIALOG); 
			break;

		case PSN_APPLY :
			// get current private keyring file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_PrivateKeyring, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) {
					if (!sIsFileNameOK (szFile)) {
						PGPclMessageBox (GetParent (hDlg), 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						PGPFreeFileSpec (fileRef);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) return TRUE;

					// create file if it doesn't exist 
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PrivateKeyring, fileRef);
					if (!PGPclErrorBox (hDlg, err)) 
						bReloadKeyring = TRUE;
				}
				PGPFreeFileSpec (fileRef);
			}

			// get current public keyring file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_PublicKeyring, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) {
					if (!sIsFileNameOK (szFile)) {
						PGPclMessageBox (GetParent (hDlg), 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						PGPFreeFileSpec (fileRef);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) return TRUE;

					// create file if it doesn't exist
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PublicKeyring, fileRef);
					if (!PGPclErrorBox (hDlg, err)) 
						bReloadKeyring = TRUE;
				}
				PGPFreeFileSpec (fileRef);
			}

			// get RNG seed file
			err = PGPsdkPrefGetFileSpec (ContextRef, 
							kPGPsdkPref_RandomSeedFile, &fileRef);
			if (IsntPGPError (err) && fileRef) {
				PGPGetFullPathFromFileSpec (fileRef, &lpsz);
				lstrcpy (szRing, lpsz);
				PGPFreeData (lpsz);
				// compare with selected file and set pref if different
				GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, 
									sizeof(szFile));
				// create file if it doesn't exist -wjb
				hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
				CloseHandle (hfile);

				if (lstrcmpi (szFile, szRing) != 0) {
					if (!sIsFileNameOK (szFile)) {
						PGPclMessageBox (GetParent (hDlg), 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						PGPFreeFileSpec (fileRef);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPclErrorBox (hDlg, err)) return TRUE;

					// create file if it doesn't exist
					hfile = CreateFile (szFile, GENERIC_READ|GENERIC_WRITE, 
								0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 
								NULL);
					CloseHandle (hfile);

					PGPclErrorBox (hDlg, PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_RandomSeedFile, fileRef));
				}
				PGPFreeFileSpec (fileRef);
			}

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

			if (bReloadKeyring) {
				// reset warn flag
				PGPSetPrefBoolean (PrefRefClient, 
					kPGPPrefWarnOnReadOnlyKeyRings, (PGPBoolean)TRUE);
			}
			return TRUE;
            break;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;
			break;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}

	}
	return FALSE;

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sEncryptPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	HWND					hParent;
	RECT					rc;
	UINT					u, uSec, uEncrypt, uSign;
	PGPBoolean				b;
	CHAR					sz[64];
	PCACHEDURATIONSTRUCT	pcds;

	switch (uMsg) {

	case WM_INITDIALOG:
		
		pcds = clAlloc (sizeof(CACHEDURATIONSTRUCT));
		SetWindowLong (hDlg, GWL_USERDATA, (LONG)pcds);
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// initialize decrypt to self
		b = DEFAULT_ENCRYPTTOSELF;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefEncryptToSelf, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_UNCHECKED);

		// create and initialize decryption date/time picker control
		GetWindowRect (GetDlgItem (hDlg, IDC_DECRYPTCACHETIME), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndDecryptDuration = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
							 DTS_TIMEFORMAT,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_DECRYPTCACHETIME, 
							 g_hInst, NULL);
		SendMessage (pcds->hwndDecryptDuration, DTM_SETFORMAT, 0, 
						(LPARAM)"HH' : 'mm' : 'ss");
		SetWindowPos (pcds->hwndDecryptDuration, 
				GetDlgItem (hDlg, IDC_DECRYPTCACHEENABLE),
				0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		
		GetSystemTime (&(pcds->stDecryptDuration));
		uSec = DEFAULT_DECRYPTCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefDecryptCacheSeconds, &uSec);
		u = uSec / 3600;
		pcds->stDecryptDuration.wHour = u;

		uSec -= (u * 3600);
		u = uSec / 60;
		pcds->stDecryptDuration.wMinute = u;

		uSec -= (u * 60);
		pcds->stDecryptDuration.wSecond = uSec;

		pcds->stDecryptDuration.wMilliseconds = 0;

		SendMessage (pcds->hwndDecryptDuration, DTM_SETSYSTEMTIME, GDT_VALID, 
						(LPARAM)&(pcds->stDecryptDuration));

		// initialize decrypt cache on/off
		b = DEFAULT_DECRYPTCACHEENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefDecryptCacheEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_DECRYPTCACHEENABLE, BST_CHECKED);
			EnableWindow (pcds->hwndDecryptDuration, TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_DECRYPTCACHEENABLE, BST_UNCHECKED);
			EnableWindow (pcds->hwndDecryptDuration, FALSE);
		}


		// create and initialize sign date/time picker control
		GetWindowRect (GetDlgItem (hDlg, IDC_SIGNCACHETIME), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndSignDuration = CreateWindowEx (0, DATETIMEPICK_CLASS,
                             "DateTime",
                             WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|
							 DTS_TIMEFORMAT,
                             rc.left, rc.top, 
							 rc.right-rc.left, rc.bottom-rc.top, 
							 hDlg, (HMENU)IDC_SIGNCACHETIME, 
							 g_hInst, NULL);
		SendMessage (pcds->hwndSignDuration, DTM_SETFORMAT, 0, 
						(LPARAM)"HH' : 'mm' : 'ss");
		SetWindowPos (pcds->hwndSignDuration, 
				GetDlgItem (hDlg, IDC_SIGNCACHEENABLE),
				0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		
		GetSystemTime (&(pcds->stSignDuration));
		uSec = DEFAULT_DECRYPTCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefSignCacheSeconds, &uSec);
		u = uSec / 3600;
		pcds->stSignDuration.wHour = u;

		uSec -= (u * 3600);
		u = uSec / 60;
		pcds->stSignDuration.wMinute = u;

		uSec -= (u * 60);
		pcds->stSignDuration.wSecond = uSec;

		pcds->stSignDuration.wMilliseconds = 0;

		SendMessage (pcds->hwndSignDuration, DTM_SETSYSTEMTIME, GDT_VALID, 
						(LPARAM)&(pcds->stSignDuration));

		// initialize sign cache on/off
		b = DEFAULT_SIGNCACHEENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefSignCacheEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_CHECKED);
			EnableWindow (pcds->hwndSignDuration, TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_UNCHECKED);
			EnableWindow (pcds->hwndSignDuration, FALSE);
		}

		// comment block text
		SendDlgItemMessage (hDlg, IDC_COMMENTBLOCK, EM_SETLIMITTEXT, 
							COMMENTBLOCK_MAX_LENGTH, 0);
		sz[0] = '\0';
#if PGP_BUSINESS_SECURITY
		PGPGetPrefStringBuffer (PrefRefAdmin, kPGPPrefComments, 
								sizeof(sz), sz);
#endif
		if (sz[0]) 
			EnableWindow (GetDlgItem (hDlg, IDC_COMMENTBLOCK), FALSE);
		else 
			PGPGetPrefStringBuffer (PrefRefClient, kPGPPrefComment, 
									sizeof(sz), sz);
		SetDlgItemText (hDlg, IDC_COMMENTBLOCK, sz);

		b = DEFAULT_WIPECONFIRM;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWarnOnWipe, &b);
		if (b) CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_UNCHECKED);

		// initialize fast key gen preference
		b = DEFAULT_USEFASTKEYGEN;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefFastKeyGen, &b);
		if (b) CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_UNCHECKED);

		// initialize wipe preferences
		SendDlgItemMessage (hDlg, IDC_NUMPASSESSPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAXWIPEPASSES, 1));

		u = DEFAULT_WIPEPASSES;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefFileWipePasses, &u);
		SendDlgItemMessage (hDlg, IDC_NUMPASSESSPIN, UDM_SETPOS,
				0, (LPARAM)MAKELONG (u, 0));

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aGeneralIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aGeneralIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {

		case IDC_DECRYPTCACHEENABLE :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg, 
						IDC_DECRYPTCACHEENABLE) == BST_CHECKED) 
				EnableWindow (pcds->hwndDecryptDuration, TRUE);
			else 
				EnableWindow (pcds->hwndDecryptDuration, FALSE);
			break;

		case IDC_SIGNCACHEENABLE :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hDlg,
						IDC_SIGNCACHEENABLE) == BST_CHECKED) 
				EnableWindow (pcds->hwndSignDuration, TRUE);
			else 
				EnableWindow (pcds->hwndSignDuration, FALSE);
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_ENCRYPTDIALOG); 
			break;

		case PSN_APPLY :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			SendMessage (pcds->hwndDecryptDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stDecryptDuration));
			uEncrypt = pcds->stDecryptDuration.wHour * 3600;
			uEncrypt += (pcds->stDecryptDuration.wMinute * 60);
			uEncrypt += pcds->stDecryptDuration.wSecond;

			SendMessage (pcds->hwndSignDuration, DTM_GETSYSTEMTIME, 0, 
						(LPARAM)&(pcds->stSignDuration));
			uSign = pcds->stSignDuration.wHour * 3600;
			uSign += (pcds->stSignDuration.wMinute * 60);
			uSign += pcds->stSignDuration.wSecond;

			if ((uEncrypt > MAXCACHETIMELIMIT)||(uSign > MAXCACHETIMELIMIT)) {
				PGPclMessageBox (hDlg, IDS_CAPTION, 
					IDS_INVALIDCACHEVALUE, MB_OK|MB_ICONHAND);
				SetWindowLong (hDlg, DWL_MSGRESULT, 
					PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTTOSELF) == BST_CHECKED)
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
								kPGPPrefEncryptToSelf, b));
			
			u = 0;
			b = FALSE;
			if (uEncrypt > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_DECRYPTCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDecryptCacheEnable, b));
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefDecryptCacheSeconds, uEncrypt));
			if (!b) u |= PGPCL_DECRYPTIONCACHE;

			b = FALSE;
			if (uSign > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_SIGNCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefSignCacheEnable, b));
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefSignCacheSeconds, uSign));
			if (!b) u |= PGPCL_SIGNINGCACHE;

			if (u != 0)
				PGPclNotifyPurgePassphraseCache (u, 0);
			
			if (!GetDlgItemText (hDlg, IDC_COMMENTBLOCK, sz, sizeof(sz))) {
				sz[0] = '\0';
			}
			PGPclErrorBox (hDlg, PGPSetPrefString (PrefRefClient,
									kPGPPrefComment, sz));
			
			if (IsDlgButtonChecked (hDlg, IDC_USEFASTKEYGEN) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefFastKeyGen, b));
            
			if (IsDlgButtonChecked (hDlg, IDC_WIPECONFIRM) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnWipe, b));

			u = GetDlgItemInt (hDlg, IDC_NUMPASSES, NULL, FALSE);
			if (u < 1) u = 1;
			if (u > MAXWIPEPASSES) u = MAXWIPEPASSES;
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
									kPGPPrefFileWipePasses, u));

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sEmailPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	HWND		hParent;
	RECT		rc;
	PGPBoolean	b;
	UINT		u;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}
	
		b = DEFAULT_MAILENCRYPTPGPMIME;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailEncryptPGPMIME, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_UNCHECKED);

		u = DEFAULT_WORDWRAPWIDTH;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefWordWrapWidth, &u);
		SetDlgItemInt (hDlg, IDC_WRAPNUMBER, u, FALSE);

		b = DEFAULT_WORDWRAPENABLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWordWrapEnable, &b);
		if (b) {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_CHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), FALSE);
		}

		b = DEFAULT_MAILENCRYPTDEFAULT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailEncryptDefault, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_UNCHECKED);

		b = DEFAULT_MAILSIGNDEFAULT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMailSignDefault, &b);
		if (b) CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_UNCHECKED);

		b = DEFAULT_AUTODECRYPT;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefAutoDecrypt, &b);
		if (b) CheckDlgButton (hDlg, IDC_AUTODECRYPT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_AUTODECRYPT, BST_UNCHECKED);

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aEmailIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aEmailIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_WRAPENABLE :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED)
				EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), TRUE);
			else
				EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), FALSE);
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_EMAILDIALOG); 
			break;

		case PSN_APPLY :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefWordWrapEnable, b));
			
			if (b) {
				u = GetDlgItemInt (hDlg, IDC_WRAPNUMBER, NULL, FALSE);
				if (u == 0) {
					PGPclMessageBox (hDlg, IDS_CAPTION, 
										IDS_INVALIDWRAP, MB_OK|MB_ICONHAND);
					SetWindowLong (hDlg, DWL_MSGRESULT, 
										PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}
				PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient,
										kPGPPrefWordWrapWidth, u));
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTPGPMIME) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailEncryptPGPMIME, b));
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailSignPGPMIME, b));
						
			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTBYDEFAULT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailEncryptDefault, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_SIGNBYDEFAULT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailSignDefault, b));
		
			if (IsDlgButtonChecked (hDlg, IDC_AUTODECRYPT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefAutoDecrypt, b));
			
			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return PSNRET_NOERROR;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}

//	_____________________________________________________
//
//  Initialize preferred algorithm combo box on basis of
//	enabled algorithms

static VOID
sInitPreferredAlgComboBox (HWND hDlg, UINT uInit) {

	INT		iIdx, iPrevAlg;
	CHAR	sz[32];
	UINT	uIndex;

	if (uInit) iPrevAlg = uInit;
	else {
		iIdx = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETCURSEL, 0, 0);	
		if (iIdx != CB_ERR) 
			iPrevAlg = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETITEMDATA, iIdx, 0);
		else 
			iPrevAlg = kPGPCipherAlgorithm_None;
	}
	SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_RESETCONTENT, 0, 0);

	iIdx = -1;

	// put item into combobox for each enabled algorithm
	if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) {
		LoadString (g_hInst, IDS_CAST, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_CAST5);
		if (iPrevAlg == kPGPCipherAlgorithm_CAST5) iIdx = uIndex;
	}

	if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) {
		LoadString (g_hInst, IDS_IDEA, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_IDEA);
		if (iPrevAlg == kPGPCipherAlgorithm_IDEA) iIdx = uIndex;
	}

	if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) {
		LoadString (g_hInst, IDS_3DES, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_3DES);
		if (iPrevAlg == kPGPCipherAlgorithm_3DES) iIdx = uIndex;
	}

	if (iIdx < 0) iIdx = 0;
	SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETCURSEL, iIdx, 0);

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sAdvancedPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	HWND				hParent;
	RECT				rc;
	PGPBoolean			b;
	UINT				u, uNumAlgs;
	INT					iIdx;
	PGPCipherAlgorithm*	pAlgs;
	PGPError			err;
	PGPSize				size;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// enable checkboxes of those ciphers supported by SDK
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_CAST5);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLECAST), TRUE);
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_3DES);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLE3DES), TRUE);
		err = 
			PGPclCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_IDEA);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLEIDEA), TRUE);
		
		// check boxes of enabled algorithms
		PGPGetPrefData (PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  &size, &pAlgs);
		uNumAlgs = size / sizeof(PGPCipherAlgorithm);

		for (u=0; u<uNumAlgs; u++) {
			err = PGPclCheckSDKSupportForCipherAlg (pAlgs[u]);
			if (IsntPGPError (err)) {
				switch (pAlgs[u]) {
				case kPGPCipherAlgorithm_CAST5 :
					CheckDlgButton (hDlg, IDC_ENABLECAST, BST_CHECKED);
					break;

				case kPGPCipherAlgorithm_3DES :
					CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_CHECKED);
					break;

				case kPGPCipherAlgorithm_IDEA :
					CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_CHECKED);
					break;
				}
			}
		}
		if (pAlgs) PGPDisposePrefData (PrefRefClient, pAlgs);

		// initialize preferred algorithm
		u = DEFAULT_PREFERREDALGORITHM;
		PGPGetPrefNumber (PrefRefClient, kPGPPrefPreferredAlgorithm, &u);
		sInitPreferredAlgComboBox (hDlg, u);

		// initialize trust model preferences
		b = DEFAULT_DISPLAYMARGVALIDITY;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefDisplayMarginalValidity,&b);
		if (b) CheckDlgButton (hDlg, IDC_DISPMARGVALIDITY, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_DISPMARGVALIDITY, BST_UNCHECKED);

		b = DEFAULT_MARGVALIDISINVALID;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefMarginalIsInvalid, &b);
		if (b) CheckDlgButton (hDlg, IDC_MARGVALIDISINVALID, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_MARGVALIDISINVALID, BST_UNCHECKED);

		b = DEFAULT_WARNONADK;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWarnOnADK, &b);
		if (b) CheckDlgButton (hDlg, IDC_WARNONADK, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WARNONADK, BST_UNCHECKED);

		// initialize export format
		b = DEFAULT_EXPORTCOMPATIBLE;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefExportKeysCompatible, &b);
		if (b) 
			CheckRadioButton (hDlg, IDC_EXPORTCOMPATIBLE, 
							IDC_EXPORTCOMPLETE, IDC_EXPORTCOMPATIBLE);
		else 
			CheckRadioButton (hDlg, IDC_EXPORTCOMPATIBLE, 
							IDC_EXPORTCOMPLETE, IDC_EXPORTCOMPLETE);

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aAdvancedIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aAdvancedIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_ENABLECAST :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLE3DES :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLEIDEA :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_CHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_UNCHECKED);
				sInitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPclMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;
		}
		break;

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
				IDH_PGPCLPREF_ADVANCEDDIALOG); 
			break;

		case PSN_APPLY :
			// get preferred algorithm
			iIdx = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
										CB_GETCURSEL, 0, 0);	
			if (iIdx != CB_ERR) 
				u = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
									CB_GETITEMDATA, iIdx, 0);
			else 
				u = kPGPCipherAlgorithm_None;
			PGPclErrorBox (hDlg, PGPSetPrefNumber (PrefRefClient, 
										kPGPPrefPreferredAlgorithm, u));
			
			// save enabled ciphers
			pAlgs = clAlloc (3 * sizeof(PGPCipherAlgorithm));
			pAlgs[0] = u;	// first element is preferred

			if (pAlgs[0] == kPGPCipherAlgorithm_None) u = 0;
			else u = 1;

			if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_CAST5) 
					pAlgs[u++] = kPGPCipherAlgorithm_CAST5;
			}
			if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_3DES) 
					pAlgs[u++] = kPGPCipherAlgorithm_3DES;
			}
			if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED) {
				if (pAlgs[0] != kPGPCipherAlgorithm_IDEA) 
					pAlgs[u++] = kPGPCipherAlgorithm_IDEA;
			}
			PGPSetPrefData (PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  u * sizeof(PGPCipherAlgorithm), pAlgs);
			clFree (pAlgs);

			// save trust model preferences
			if (IsDlgButtonChecked (
							hDlg, IDC_DISPMARGVALIDITY) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDisplayMarginalValidity, b));
			if (IsDlgButtonChecked (
							hDlg, IDC_MARGVALIDISINVALID) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefMarginalIsInvalid, b));

			if (IsDlgButtonChecked (
							hDlg, IDC_WARNONADK) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnADK, b));

			// save export format preferences
			if (IsDlgButtonChecked (
							hDlg, IDC_EXPORTCOMPATIBLE) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefExportKeysCompatible, b));

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

			return PSNRET_NOERROR;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			returnErr = kPGPError_UserAbort;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}


//	_____________________________________________________
//
//  Message processing function for property sheet dialog

static LRESULT WINAPI 
sKeyserverPropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	HWND		hParent;
	RECT		rc;
	PGPBoolean	b;

	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}
		hWndTree = CLCreateKeyserverTreeList (PrefRefClient, 
										ContextRef, hDlg, keysetMain);
		SetWindowPos (hWndTree, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		// initialize "Sync On ..." checkboxes
		b = DEFAULT_KEYSERVERSYNCONUNKNOWN;
		PGPGetPrefBoolean (PrefRefClient, 
										kPGPPrefKeyServerSyncUnknownKeys, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONADDNAME;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnAdd, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONADDNAME, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONADDNAME, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONKEYSIGN;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnKeySign, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONKEYSIGN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONKEYSIGN, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONREVOKE;
		PGPGetPrefBoolean (PrefRefClient, 
								kPGPPrefKeyServerSyncOnRevocation, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONREVOKE, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONREVOKE, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONVERIFY;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnVerify, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONVERIFY, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONVERIFY, BST_UNCHECKED);

		return TRUE;

	case WM_KEYDOWN :
	case WM_SYSKEYDOWN :
	case WM_KEYUP :
	case WM_CHAR :
		if (GetFocus () == hWndTree)
			SendMessage (hWndTree, uMsg, wParam, lParam);
		break;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aServerIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aServerIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDM_NEWSERVER :
		case IDC_NEWKEYSERVER :
			CLNewKeyserver (hDlg, hWndTree);
			break;

		case IDM_REMOVESERVER :
		case IDC_REMOVEKEYSERVER :
			CLRemoveKeyserver (hDlg, hWndTree);
			break;

		case IDM_EDITSERVER :
		case IDC_EDITKEYSERVER :
			CLEditKeyserver (hDlg, hWndTree);
			break;

		case IDM_SETASROOT :
		case IDC_SETASROOT :
			CLSetKeyserverAsRoot (hDlg, hWndTree);
			break;

		}
		break;

	case WM_NOTIFY :
		if (((NMHDR FAR *) lParam)->hwndFrom == hWndTree) {
			CLProcessKeyserverTreeList (hDlg, hWndTree, wParam, lParam);
		}
		else {
			switch (((NMHDR FAR *) lParam)->code) {
			case PSN_SETACTIVE :
				break;
	
			case PSN_HELP :
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
					IDH_PGPCLPREF_KEYSERVERDIALOG); 
				break;
	
			case PSN_APPLY :	
				CLSaveKeyserverPrefs (PrefRefClient, hDlg, hWndTree);
				CLDestroyKeyserverTreeList (hWndTree);

				// get and save checkbox values
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONUNKNOWN)==BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncUnknownKeys, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONADDNAME) ==
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnAdd, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONKEYSIGN) ==
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnKeySign, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONREVOKE) == 
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnRevocation, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONVERIFY) == 
															BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPclErrorBox (hDlg, PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnVerify, b));
				
				bKeyserverPrefsWritten = TRUE;

				returnErr = kPGPError_NoErr;
	            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
	            return PSNRET_NOERROR;
	
			case PSN_KILLACTIVE :
	            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
	            return TRUE;
	
			case PSN_RESET :
				CLDestroyKeyserverTreeList (hWndTree);
				returnErr = kPGPError_UserAbort;
	            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
				break;
			}
		}
	}

	return FALSE;

}


//	______________________________________
//
//	Check for previously running instances

static HANDLE 
sShowExistingPropertySheet (
		LPSTR lpszSemName, 
		LPSTR lpszWindowTitle) 
{
    HANDLE	hSem;
    HWND	hWndMe;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, lpszSemName);

    // Close handle and return NULL if existing semaphore was opened.
    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
        CloseHandle(hSem);
        hWndMe = FindWindow ((LPCTSTR)32770L, lpszWindowTitle);
        if (hWndMe) {
			SetForegroundWindow (hWndMe);
		}
        return NULL;
    }

    // If new semaphore was created, return FALSE.
    return hSem;
}
 
//	____________________________
//
//  Create property sheet dialog

PGPError PGPclExport 
PGPclPreferences (
		PGPContextRef	Context, 
		HWND			hWnd, 
		INT				startsheet,
		PGPKeySetRef	keyset) 
{
    PROPSHEETPAGE	psp[5];
    PROPSHEETHEADER psh;
	UINT			uNumPages = 0;
	CHAR			szTitle[128];
	HANDLE			hSemaphore;
	PGPError		err;

	//assume no KeyServer page
	uNumPages = (sizeof(psp) / sizeof(PROPSHEETPAGE)); 

	// get title of property sheet
	LoadString (g_hInst, IDS_PROPCAPTION, szTitle, sizeof(szTitle));

	// if preferences dialog already exists, move it to the foreground
	hSemaphore = sShowExistingPropertySheet (PREFSEMAPHORENAME, szTitle);
	if (!hSemaphore) return kPGPError_Win32_AlreadyOpen;

	hWndParent = hWnd;
	keysetMain = keyset;

	if (Context != NULL)
		ContextRef = Context;
	else
		PGPNewContext (kPGPsdkAPIVersion, &ContextRef);

	PGPsdkLoadDefaultPrefs (ContextRef);

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[0].hInstance = g_hInst;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PREF0);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = sEncryptPropDlgProc;
	psp[0].pszTitle = (LPSTR)IDS_TITLE0;
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[1].hInstance = g_hInst;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PREF1);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = sFilePropDlgProc;
	psp[1].pszTitle = (LPSTR)IDS_TITLE1;
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[2].hInstance = g_hInst;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PREF2);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = sEmailPropDlgProc;
	psp[2].pszTitle = (LPSTR)IDS_TITLE2;
	psp[2].lParam = 0;
	psp[2].pfnCallback = NULL;

	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[3].hInstance = g_hInst;
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PREF3);
	psp[3].pszIcon = NULL;
	psp[3].pfnDlgProc = sKeyserverPropDlgProc;
	psp[3].pszTitle = (LPSTR)IDS_TITLE3;
	psp[3].lParam = 0;
	psp[3].pfnCallback = NULL;

	psp[4].dwSize = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[4].hInstance = g_hInst;
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PREF4);
	psp[4].pszIcon = NULL;
	psp[4].pfnDlgProc = sAdvancedPropDlgProc;
	psp[4].pszTitle = (LPSTR)IDS_TITLE4;
	psp[4].lParam = 0;
	psp[4].pfnCallback = NULL;
	
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = hWndParent;
	psh.hInstance = g_hInst;
	psh.pszIcon = NULL;
	psh.pszCaption = (LPSTR) szTitle;
	psh.nPages = uNumPages;
	psh.nStartPage = startsheet;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback = NULL;

	bNeedsCentering = TRUE;
	bReloadKeyring = FALSE;
	bKeyserverPrefsWritten = FALSE;

	err = kPGPError_NoErr;
#if PGP_BUSINESS_SECURITY
#if PGP_ADMIN_BUILD
	err = PGPclOpenAdminPrefs (
			PGPGetContextMemoryMgr (ContextRef),
			&PrefRefAdmin, 
			TRUE);
#else
	err = PGPclOpenAdminPrefs (
			PGPGetContextMemoryMgr (ContextRef),
			&PrefRefAdmin, 
			FALSE);
#endif
#endif

	if (IsntPGPError (err)) {
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (ContextRef),
									&PrefRefClient);
		PropertySheet(&psh);

		PGPsdkSavePrefs (ContextRef);
		PGPclCloseClientPrefs (PrefRefClient, TRUE);
	}
#if PGP_BUSINESS_SECURITY
	PGPclCloseAdminPrefs (PrefRefAdmin, FALSE);
#endif

	if (bReloadKeyring) {
		PGPclNotifyKeyringChanges (0);
	}

	if (bKeyserverPrefsWritten) {
		PGPclNotifyKeyserverPrefsChanges (0);	
	}

	PGPclNotifyPrefsChanges (0);

	if (Context == NULL)
		PGPFreeContext(ContextRef);

	CloseHandle (hSemaphore);

    return returnErr;
}
 


