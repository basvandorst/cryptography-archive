/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	CDprefs.c - handle PGP preferences dialogs
	

	$Id: CDprefs.c,v 1.49 1997/10/21 13:36:28 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"
#include <commdlg.h>

#define MAXCACHETIMELIMIT		 360000

#define SEMAPHORENAME	("PGPprefsInstSem")

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
#define DEFAULT_PREFERREDALGORITHM		kPGPCipherAlgorithm_CAST5
#define DEFAULT_DISPLAYMARGVALIDITY		TRUE
#define DEFAULT_MARGVALIDISINVALID		TRUE
#define DEFAULT_WARNONADK				TRUE

#define COMMENTBLOCK_MAX_LENGTH			60

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
extern HWND g_hWndPreferences;

// local globals
static PFLContextRef	ClientContextRefPref = NULL;
static UINT				uClientContextRefCount = 0;
static PGPPrefRef		ClientPrefRef;

static PFLContextRef	AdminContextRefPref = NULL;
static UINT				uAdminContextRefCount = 0;
static PGPPrefRef		AdminPrefRef;

static PGPContextRef	ContextRef;

static HWND				hWndParent;
static HWND				hWndTree;
static PGPError			returnErr;
static BOOL				bNeedsCentering;
static BOOL				bReloadKeyring;
static PGPPrefRef		PrefRefClient;
static PGPPrefRef		PrefRefAdmin;

typedef struct {
	HWND	hwndDecryptDuration;
	HWND	hwndSignDuration;
} CACHEDURATIONSTRUCT, *PCACHEDURATIONSTRUCT;


static DWORD aIds[] = {			// Help IDs
    IDC_ENCRYPTTOSELF,		IDH_PGPCDPREF_ENCRYPTTOSELF, 
	IDC_DECRYPTCACHEENABLE,	IDH_PGPCDPREF_PASSCACHEENABLE,
	IDC_SIGNCACHEENABLE,	IDH_PGPCDPREF_SIGNCACHEENABLE,
	IDC_COMMENTBLOCK,		IDH_PGPCDPREF_COMMENTBLOCK,
	IDC_USEFASTKEYGEN,		IDH_PGPCDPREF_USEFASTKEYGEN,
	IDC_WIPECONFIRM,		IDH_PGPCDPREF_WIPECONFIRM,

    IDC_PRIVATEKEYRING,		IDH_PGPCDPREF_PRIVATEKEYRING, 
	IDC_SETPRIVATEKEYRING,	IDH_PGPCDPREF_SETPRIVATEKEYRING,
    IDC_PUBLICKEYRING,		IDH_PGPCDPREF_PUBLICKEYRING, 
	IDC_SETPUBLICKEYRING,	IDH_PGPCDPREF_SETPUBLICKEYRING,
	IDC_RNGSEEDFILE,		IDH_PGPCDPREF_RANDOMSEEDFILE,
	IDC_SETRNGSEEDFILE,		IDH_PGPCDPREF_SETRANDOMSEEDFILE,

	IDC_ENCRYPTPGPMIME,		IDH_PGPCDPREF_ENCRYPTPGPMIME,
	IDC_WRAPENABLE,			IDH_PGPCDPREF_WORDWRAPENABLE,
	IDC_WRAPNUMBER,			IDH_PGPCDPREF_WORDWRAPCOLUMN,
	IDC_ENCRYPTBYDEFAULT,	IDH_PGPCDPREF_ENCRYPTBYDEFAULT,
	IDC_SIGNBYDEFAULT,		IDH_PGPCDPREF_SIGNBYDEFAULT,
	IDC_AUTODECRYPT,		IDH_PGPCDPREF_AUTODECRYPT,

	IDC_NEWKEYSERVER,		IDH_PGPCDPREF_CREATESERVER,
	IDC_REMOVEKEYSERVER,	IDH_PGPCDPREF_REMOVESERVER,
	IDC_SETDEFAULTKEYSERVER,IDH_PGPCDPREF_SETDEFAULTSERVER,
	IDC_SYNCONUNKNOWN,		IDH_PGPCDPREF_SYNCONUNKNOWNKEYS,
	IDC_SYNCONADDNAME,		IDH_PGPCDPREF_SYNCONADDNAME,
	IDC_SYNCONKEYSIGN,		IDH_PGPCDPREF_SYNCONSIGN,
	IDC_SYNCONREVOKE,		IDH_PGPCDPREF_SYNCONREVOKE,
	IDC_KSTREELIST,			IDH_PGPCDPREF_SERVERLIST,

	IDC_ENABLECAST,			IDH_PGPCDPREF_ENABLECAST,
	IDC_ENABLE3DES,			IDH_PGPCDPREF_ENABLE3DES,
	IDC_ENABLEIDEA,			IDH_PGPCDPREF_ENABLEIDEA,
	IDC_PREFERREDALG,		IDH_PGPCDPREF_PREFERREDALG,
	IDC_DISPMARGVALIDITY,	IDH_PGPCDPREF_DISPMARGVALIDITY,
	IDC_MARGVALIDISINVALID,	IDH_PGPCDPREF_WARNONMARGINAL,
	IDC_WARNONADK,			IDH_PGPCDPREF_WARNONADK,
    0,0 
}; 

//	______________________________________________
//
//  Get standard client preferences file reference

PGPError PGPcdExport
PGPcomdlgOpenClientPrefs (PGPPrefRef* pPrefRef) 
{
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PGPError		err;
	PFLFileSpecRef	FileRef;

	if (ClientContextRefPref) {
		uClientContextRefCount++;
		*pPrefRef = ClientPrefRef;
		return kPGPError_NoErr;
	}

	PFLNewContext (&ClientContextRefPref);
	uClientContextRefCount = 1;

	err = PGPcomdlgGetPGPPath (szPath, sizeof(szPath));
	if (IsPGPError (err)) {
		GetWindowsDirectory (szPath, sizeof(szPath));
		if (szPath[lstrlen(szPath)-1] != '\\') lstrcat (szPath, "\\");
	}

	LoadString (g_hInst, IDS_CLIENTPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	PFLNewFileSpecFromFullPath (ClientContextRefPref, szPath, &FileRef);

	err = PGPOpenPrefFile (FileRef, &ClientPrefRef);
	if ((err == kPGPError_FileOpFailed) ||
		(err == kPGPError_FileNotFound)) {
		err = PFLFileSpecCreate (FileRef);
		pgpAssert (IsntPGPError (err));
		err = PGPOpenPrefFile(FileRef, &ClientPrefRef);
		pgpAssert (IsntPGPError (err));
		if (IsntPGPError (err)) {
			err = PGPLoadClientDefaults (ClientPrefRef);
			pgpAssert (IsntPGPError(err));
			err = PGPSavePrefFile (ClientPrefRef);
			pgpAssert (IsntPGPError(err));
		}
	}
	PFLFreeFileSpec (FileRef);

	if (IsPGPError (err)) {
		if (PGPRefIsValid (ClientPrefRef)) 
			PGPClosePrefFile (ClientPrefRef);

		if (PGPRefIsValid (ClientContextRefPref)) 
			PFLFreeContext (ClientContextRefPref);

		ClientContextRefPref = NULL;
	}
	else {	
		*pPrefRef = ClientPrefRef;
		uClientContextRefCount = 1;
	}

	return err;
}

//	______________________________________________
//
//  Get standard admin preferences file reference

PGPError PGPcdExport
PGPcomdlgOpenAdminPrefs (PGPPrefRef* pPrefRef, BOOL bLoadDefaults) 
{
	CHAR			szPath[MAX_PATH];
	CHAR			sz[64];
	PGPError		err;
	PFLFileSpecRef	FileRef;

	if (AdminContextRefPref) {
		uAdminContextRefCount++;
		*pPrefRef = AdminPrefRef;
		return kPGPError_NoErr;
	}

	PFLNewContext (&AdminContextRefPref);

	err = PGPcomdlgGetPGPPath (szPath, sizeof(szPath));
	if (IsPGPError (err)) {
		GetWindowsDirectory (szPath, sizeof(szPath));
		if (szPath[lstrlen(szPath)-1] != '\\') lstrcat (szPath, "\\");
	}

	LoadString (g_hInst, IDS_ADMINPREFSFILE, sz, sizeof(sz));
	lstrcat (szPath, sz);

	PFLNewFileSpecFromFullPath (AdminContextRefPref, szPath, &FileRef);

	err = PGPOpenPrefFile (FileRef, &AdminPrefRef);
	if ((err == kPGPError_FileOpFailed) ||
		(err == kPGPError_FileNotFound)) {
		if (bLoadDefaults) {
			err = PFLFileSpecCreate (FileRef);
			pgpAssert (IsntPGPError (err));
			err = PGPOpenPrefFile(FileRef, &AdminPrefRef);
			pgpAssert (IsntPGPError (err));
			if (IsntPGPError (err)) {
				err = PGPLoadAdminDefaults (AdminPrefRef);
				pgpAssert (IsntPGPError(err));
				err = PGPSavePrefFile (AdminPrefRef);
				pgpAssert (IsntPGPError(err));
			}
		}
	}
	PFLFreeFileSpec (FileRef);

	if (IsPGPError (err)) {
		PGPcomdlgMessageBox (NULL, IDS_CAPTIONERROR, IDS_NOADMINFILE,
									MB_OK|MB_ICONSTOP);
		if (PGPRefIsValid (AdminPrefRef)) 
			PGPClosePrefFile (AdminPrefRef);

		if (PGPRefIsValid (AdminContextRefPref)) 
			PFLFreeContext (AdminContextRefPref);

		AdminContextRefPref = NULL;
	}
	else {	
		*pPrefRef = AdminPrefRef;
		uAdminContextRefCount = 1;
	}

	return err;
}

//	_______________________________________
//
//  Save and close down client preferences file

PGPError PGPcdExport
PGPcomdlgCloseClientPrefs (PGPPrefRef PrefRef, BOOL bSave) 
{
	PGPError		err;

	if (bSave) {
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	if (--uClientContextRefCount == 0) {
		err = PGPClosePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
		err = PFLFreeContext (ClientContextRefPref);
		pgpAssert (IsntPGPError(err));
		ClientContextRefPref = NULL;
	}

	return kPGPError_NoErr;
}

//	_______________________________________
//
//  Save and close down admin preferences file

PGPError PGPcdExport
PGPcomdlgCloseAdminPrefs (PGPPrefRef PrefRef, BOOL bSave) 
{
	PGPError		err;

	if (bSave) {
		err = PGPSavePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
	}

	if (--uAdminContextRefCount == 0) {
		err = PGPClosePrefFile (PrefRef);
		pgpAssert (IsntPGPError(err));
		err = PFLFreeContext (AdminContextRefPref);
		pgpAssert (IsntPGPError(err));
		AdminContextRefPref = NULL;
	}

	return kPGPError_NoErr;
}

//	____________________________
//
//  Check for illegal file names

BOOL 
IsFileNameOK (LPSTR sz) 
{
	UINT	uLen;
	DWORD	dwAttrib;

	uLen = lstrlen (sz);

	if (uLen < 4) return FALSE;
	if ((sz[1] != ':') && (sz[1] != '\\')) return FALSE;
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

VOID
VerifyFileExists (LPSTR lpszFile)
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

		if (IsPGPError (PGPcomdlgGetPGPPath (lpszFile, MAX_PATH))) 
			GetWindowsDirectory (lpszFile, MAX_PATH);
		if (lpszFile[lstrlen (lpszFile) -1] != '\\') 
			lstrcat (lpszFile, "\\");
		lstrcat (lpszFile, p);
	}
}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

LRESULT WINAPI 
FilePropDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szRing[MAX_PATH];
	CHAR			szInitDir[MAX_PATH];
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
	PGPKeySetRef	keysetDummy;
	
	switch (uMsg) {

	case WM_INITDIALOG:
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			g_hWndPreferences = hParent;
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

			VerifyFileExists (szTemp);
			SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_PrivateKeyring,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			VerifyFileExists (szTemp);
			SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szTemp);
		}

		err = PGPsdkPrefGetFileSpec (ContextRef, kPGPsdkPref_RandomSeedFile,
									&fileRef);
		if (IsntPGPError (err) && fileRef) {
			PGPGetFullPathFromFileSpec (fileRef, &lpsz);
			lstrcpy (szTemp, lpsz);
			PGPFreeData (lpsz);
			PGPFreeFileSpec (fileRef);

			VerifyFileExists (szTemp);
			SetDlgItemText (hDlg, IDC_RNGSEEDFILE, szTemp);
		}
		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_SETPRIVATEKEYRING :
			GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, sizeof(szFile));
			lstrcpy (szInitDir, "");
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				lstrcpy (szFile, "");
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
			}
			break;

		case IDC_SETPUBLICKEYRING :
			GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, sizeof(szFile));
			lstrcpy (szInitDir, "");
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				lstrcpy (szFile, "");
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
			}
			break;

		case IDC_SETRNGSEEDFILE :
			GetDlgItemText (hDlg, IDC_RNGSEEDFILE, szFile, sizeof(szFile));
			lstrcpy (szInitDir, "");
			if (GetFileAttributes (szFile) & FILE_ATTRIBUTE_DIRECTORY) {
				lstrcpy (szInitDir, szFile);
				lstrcpy (szFile, "");
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
				IDH_PGPCDPREF_FILEDIALOG); 
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
				if (lstrcmpi (szFile, szRing) != 0) {
					if (!IsFileNameOK (szFile)) {
						PGPcomdlgMessageBox (g_hWndPreferences, 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPcomdlgErrorMessage (err)) return TRUE;
					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PrivateKeyring, fileRef);
					if (!PGPcomdlgErrorMessage (err)) 
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
				if (lstrcmpi (szFile, szRing) != 0) {
					if (!IsFileNameOK (szFile)) {
						PGPcomdlgMessageBox (g_hWndPreferences, 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPcomdlgErrorMessage (err)) return TRUE;
					err = PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_PublicKeyring, fileRef);
					if (!PGPcomdlgErrorMessage (err)) 
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
				if (lstrcmpi (szFile, szRing) != 0) {
					if (!IsFileNameOK (szFile)) {
						PGPcomdlgMessageBox (g_hWndPreferences, 
							IDS_CAPTION, IDS_ILLEGALFILENAME,
							MB_OK | MB_ICONHAND);
						SetWindowLong (hDlg, DWL_MSGRESULT, 
							PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					PGPFreeFileSpec (fileRef);
					err = PGPNewFileSpecFromFullPath (ContextRef, szFile, 
														&fileRef);
					if (PGPcomdlgErrorMessage (err)) return TRUE;
					PGPcomdlgErrorMessage (PGPsdkPrefSetFileSpec (ContextRef, 
							kPGPsdkPref_RandomSeedFile, fileRef));
				}
				PGPFreeFileSpec (fileRef);
			}

			returnErr = kPGPError_NoErr;
            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

			// create files if they don't exist
			PGPOpenDefaultKeyRings (ContextRef, 
					kPGPKeyRingOpenFlags_Create, &keysetDummy);
			PGPFreeKeySet (keysetDummy);

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

LRESULT WINAPI 
EncryptPropDlgProc (
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
		
		pcds = pcAlloc (sizeof(CACHEDURATIONSTRUCT));
		SetWindowLong (hDlg, GWL_USERDATA, (LONG)pcds);
		if (bNeedsCentering) {
			hParent = GetParent (hDlg);
			g_hWndPreferences = hParent;
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		b = DEFAULT_ENCRYPTTOSELF;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefEncryptToSelf, &b);
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_UNCHECKED);

		PGPcomdlgInitTimeDateControl (g_hInst);

		// setup decryption cache duration control
		GetWindowRect (GetDlgItem (hDlg, IDC_DECRYPTCACHEENABLE), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndDecryptDuration = 
			PGPcomdlgCreateTimeDateControl (hDlg, g_hInst, 
					rc.right, rc.top-2, 
					PGPCOMDLG_DISPLAY_DURATION);
		SetWindowPos (pcds->hwndDecryptDuration, 
			GetDlgItem (hDlg, IDC_DECRYPTCACHEENABLE), 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE);

		Duration_SetHourLimits (pcds->hwndDecryptDuration, 99, 0);
		Duration_SetMinuteLimits (pcds->hwndDecryptDuration, 59, 0);
		Duration_SetSecondLimits (pcds->hwndDecryptDuration, 59, 0);

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

		u = DEFAULT_DECRYPTCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefDecryptCacheSeconds, &uSec);
		u = uSec / 3600;
		Time_SetHour (pcds->hwndDecryptDuration, u);

		uSec -= (u * 3600);
		u = uSec / 60;
		Time_SetMinute (pcds->hwndDecryptDuration, u);

		uSec -= (u * 60);
		u = uSec;
		Time_SetSecond (pcds->hwndDecryptDuration, u);

		// setup sign cache duration control
		GetWindowRect (GetDlgItem (hDlg, IDC_SIGNCACHEENABLE), &rc);
		MapWindowPoints (NULL, hDlg, (LPPOINT)&rc, 2);
		pcds->hwndSignDuration = 
			PGPcomdlgCreateTimeDateControl (hDlg, g_hInst, 
					rc.right, rc.top-2, 
					PGPCOMDLG_DISPLAY_DURATION);
		SetWindowPos (pcds->hwndSignDuration, 
			GetDlgItem (hDlg, IDC_SIGNCACHEENABLE), 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE);

		Duration_SetHourLimits (pcds->hwndSignDuration, 99, 0);
		Duration_SetMinuteLimits (pcds->hwndSignDuration, 59, 0);
		Duration_SetSecondLimits (pcds->hwndSignDuration, 59, 0);

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

		u = DEFAULT_SIGNCACHESECONDS;
		PGPGetPrefNumber (PrefRefClient,kPGPPrefSignCacheSeconds, &uSec);
		u = uSec / 3600;
		Time_SetHour (pcds->hwndSignDuration, u);

		uSec -= (u * 3600);
		u = uSec / 60;
		Time_SetMinute (pcds->hwndSignDuration, u);

		uSec -= (u * 60);
		u = uSec;
		Time_SetSecond (pcds->hwndSignDuration, u);

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

		b = DEFAULT_USEFASTKEYGEN;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefFastKeyGen, &b);
		if (b) CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_UNCHECKED);

		b = DEFAULT_WIPECONFIRM;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefWarnOnWipe, &b);
		if (b) CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WIPECONFIRM, BST_UNCHECKED);

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
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
				IDH_PGPCDPREF_ENCRYPTDIALOG); 
			break;

		case PSN_APPLY :
			pcds = (PCACHEDURATIONSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			u = Time_GetHour (pcds->hwndDecryptDuration);
			uEncrypt = u * 3600;

			u = Time_GetMinute (pcds->hwndDecryptDuration);
			uEncrypt += (u * 60);

			u = Time_GetSecond (pcds->hwndDecryptDuration);
			uEncrypt += u;

			u = Time_GetHour (pcds->hwndSignDuration);
			uSign = u * 3600;

			u = Time_GetMinute (pcds->hwndSignDuration);
			uSign += (u * 60);

			u = Time_GetSecond (pcds->hwndSignDuration);
			uSign += u;

			if ((uEncrypt > MAXCACHETIMELIMIT)||(uSign > MAXCACHETIMELIMIT)) {
				PGPcomdlgMessageBox (hDlg, IDS_CAPTION, 
					IDS_INVALIDCACHEVALUE, MB_OK|MB_ICONHAND);
				SetWindowLong (hDlg, DWL_MSGRESULT, 
					PSNRET_INVALID_NOCHANGEPAGE);
				return TRUE;
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTTOSELF) == BST_CHECKED)
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
								kPGPPrefEncryptToSelf, b));
			
			b = FALSE;
			if (uEncrypt > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_DECRYPTCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDecryptCacheEnable, b));
			PGPcomdlgErrorMessage (PGPSetPrefNumber (PrefRefClient,
									kPGPPrefDecryptCacheSeconds, uEncrypt));

			b = FALSE;
			if (uSign > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_SIGNCACHEENABLE) ==
							BST_CHECKED) 
					b = TRUE;
			}
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefSignCacheEnable, b));
			PGPcomdlgErrorMessage (PGPSetPrefNumber (PrefRefClient,
									kPGPPrefSignCacheSeconds, uSign));
			
			if (!GetDlgItemText (hDlg, IDC_COMMENTBLOCK, sz, sizeof(sz))) {
				sz[0] = '\0';
			}
			PGPcomdlgErrorMessage (PGPSetPrefString (PrefRefClient,
									kPGPPrefComment, sz));
			
			if (IsDlgButtonChecked (hDlg, IDC_USEFASTKEYGEN) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefFastKeyGen, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_WIPECONFIRM) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnWipe, b));
			
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

LRESULT WINAPI 
EmailPropDlgProc (
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
			g_hWndPreferences = hParent;
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
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
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
				IDH_PGPCDPREF_EMAILDIALOG); 
			break;

		case PSN_APPLY :
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) == BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefWordWrapEnable, b));
			
			if (b) {
				u = GetDlgItemInt (hDlg, IDC_WRAPNUMBER, NULL, FALSE);
				if (u == 0) {
					PGPcomdlgMessageBox (hDlg, IDS_CAPTION, 
										IDS_INVALIDWRAP, MB_OK|MB_ICONHAND);
					SetWindowLong (hDlg, DWL_MSGRESULT, 
										PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}
				PGPcomdlgErrorMessage (PGPSetPrefNumber (PrefRefClient,
										kPGPPrefWordWrapWidth, u));
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTPGPMIME) ==
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailEncryptPGPMIME, b));
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailSignPGPMIME, b));
						
			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTBYDEFAULT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailEncryptDefault, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_SIGNBYDEFAULT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
										kPGPPrefMailSignDefault, b));
		
			if (IsDlgButtonChecked (hDlg, IDC_AUTODECRYPT) == 
						BST_CHECKED) 
				b = TRUE;
			else 
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
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

VOID
InitPreferredAlgComboBox (HWND hDlg, UINT uInit) {

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

	if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) {
		LoadString (g_hInst, IDS_3DES, sz, sizeof(sz));
		uIndex = SendDlgItemMessage (hDlg, IDC_PREFERREDALG, 
								CB_ADDSTRING, 0, (LPARAM)sz);
		if (uIndex != CB_ERR)
			SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETITEMDATA, 
				uIndex, (LPARAM)kPGPCipherAlgorithm_3DES);
		if (iPrevAlg == kPGPCipherAlgorithm_3DES) iIdx = uIndex;
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

	if (iIdx < 0) iIdx = 0;
	SendDlgItemMessage (hDlg, IDC_PREFERREDALG, CB_SETCURSEL, iIdx, 0);

}

//	_____________________________________________________
//
//  Message processing function for property sheet dialog

LRESULT WINAPI 
AdvancedPropDlgProc (
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
			g_hWndPreferences = hParent;
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}

		// enable checkboxes of those ciphers supported by SDK
		err = 
			PGPcomdlgCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_CAST5);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLECAST), TRUE);
		err = 
			PGPcomdlgCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_3DES);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLE3DES), TRUE);
		err = 
			PGPcomdlgCheckSDKSupportForCipherAlg (kPGPCipherAlgorithm_IDEA);
		if (IsntPGPError (err))
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLEIDEA), TRUE);
		
		// check boxes of enabled algorithms
		PGPGetPrefData (PrefRefClient, kPGPPrefAllowedAlgorithmsList,
							  &size, &pAlgs);
		uNumAlgs = size / sizeof(PGPCipherAlgorithm);

		for (u=0; u<uNumAlgs; u++) {
			err = PGPcomdlgCheckSDKSupportForCipherAlg (pAlgs[u]);
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
		InitPreferredAlgComboBox (hDlg, u);

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

		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_ENABLECAST :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_CHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLECAST, BST_UNCHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPcomdlgMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLE3DES :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_CHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLE3DES, BST_UNCHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPcomdlgMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
								MB_OK|MB_ICONEXCLAMATION);
			break;

		case IDC_ENABLEIDEA :
			// if unchecked, check it
			if (IsDlgButtonChecked (hDlg, IDC_ENABLEIDEA) == BST_UNCHECKED) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_CHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			// if checked, make sure it's not the "last" one
			if ((IsDlgButtonChecked (hDlg, IDC_ENABLECAST) == BST_CHECKED) ||
				(IsDlgButtonChecked (hDlg, IDC_ENABLE3DES) == BST_CHECKED)) {
				CheckDlgButton (hDlg, IDC_ENABLEIDEA, BST_UNCHECKED);
				InitPreferredAlgComboBox (hDlg, 0);
				break;
			}
			PGPcomdlgMessageBox (hDlg, IDS_CAPTION, IDS_NEEDALGORITHM,
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
				IDH_PGPCDPREF_ADVANCEDDIALOG); 
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
			PGPcomdlgErrorMessage (PGPSetPrefNumber (PrefRefClient, 
										kPGPPrefPreferredAlgorithm, u));
			
			// save enabled ciphers
			pAlgs = pcAlloc (3 * sizeof(PGPCipherAlgorithm));
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
			pcFree (pAlgs);

			// save trust model preferences
			if (IsDlgButtonChecked (
							hDlg, IDC_DISPMARGVALIDITY) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefDisplayMarginalValidity, b));
			if (IsDlgButtonChecked (
							hDlg, IDC_MARGVALIDISINVALID) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefMarginalIsInvalid, b));

			if (IsDlgButtonChecked (
							hDlg, IDC_WARNONADK) == BST_CHECKED) 
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefWarnOnADK, b));

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

LRESULT WINAPI 
KeyserverPropDlgProc (
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
			g_hWndPreferences = hParent;
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			bNeedsCentering = FALSE;
		}
		hWndTree = CDCreateKeyserverTreeList (PrefRefClient, hDlg);
		SetWindowPos (hWndTree, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		// initialize "Sync On ..." checkboxes
#if PGP_BUSINESS_SECURITY
		b = DEFAULT_KEYSERVERSYNCONUNKNOWN;
		PGPGetPrefBoolean (PrefRefClient, 
										kPGPPrefKeyServerSyncUnknownKeys, &b);
		if (b) CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SYNCONUNKNOWN, BST_UNCHECKED);

		b = DEFAULT_KEYSERVERSYNCONADDNAME;
		PGPGetPrefBoolean (PrefRefClient, kPGPPrefKeyServerSyncOnAddName, &b);
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
#else
		EnableWindow (GetDlgItem (hDlg, IDC_SYNCONUNKNOWN), FALSE);
		ShowWindow (GetDlgItem (hDlg, IDC_SYNCONUNKNOWN), SW_HIDE);
		EnableWindow (GetDlgItem (hDlg, IDC_SYNCONADDNAME), FALSE);
		ShowWindow (GetDlgItem (hDlg, IDC_SYNCONADDNAME), SW_HIDE);
		EnableWindow (GetDlgItem (hDlg, IDC_SYNCONKEYSIGN), FALSE);
		ShowWindow (GetDlgItem (hDlg, IDC_SYNCONKEYSIGN), SW_HIDE);
		EnableWindow (GetDlgItem (hDlg, IDC_SYNCONREVOKE), FALSE);
		ShowWindow (GetDlgItem (hDlg, IDC_SYNCONREVOKE), SW_HIDE);
		ShowWindow (GetDlgItem (hDlg, IDC_SYNCONSTATICTEXT), SW_HIDE);
#endif
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
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_NEWKEYSERVER :
			CDNewKeyserver (hDlg, hWndTree);
			break;

		case IDC_REMOVEKEYSERVER :
			CDRemoveKeyserver (hDlg, hWndTree);
			break;

		case IDC_SETDEFAULTKEYSERVER :
			CDSetDefaultKeyserver (hDlg, hWndTree);
			break;

		case IDM_EDITSERVER :
			CDEditKeyserver (hDlg, hWndTree);
			break;

		}
		break;

	case WM_NOTIFY :
		if (((NMHDR FAR *) lParam)->hwndFrom == hWndTree) {
			CDProcessKeyserverTreeList (hDlg, hWndTree, wParam, lParam);
		}
		else {
			switch (((NMHDR FAR *) lParam)->code) {
			case PSN_SETACTIVE :
				break;
	
			case PSN_HELP :
				WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT, 
					IDH_PGPCDPREF_KEYSERVERDIALOG); 
				break;
	
			case PSN_APPLY :	
				CDSaveKeyserverPrefs (PrefRefClient, hDlg, hWndTree);
				CDDestroyKeyserverTreeList (hWndTree);

				// get and save checkbox values
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONUNKNOWN)==BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncUnknownKeys, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONADDNAME)==BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnAddName, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONKEYSIGN)==BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnKeySign, b));
			
				if (IsDlgButtonChecked (hDlg,IDC_SYNCONREVOKE) == BST_CHECKED) 
					b = TRUE;
				else 
					b = FALSE;
				PGPcomdlgErrorMessage (PGPSetPrefBoolean (PrefRefClient,
									kPGPPrefKeyServerSyncOnRevocation, b));
			
				returnErr = kPGPError_NoErr;
	            SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
	            return PSNRET_NOERROR;
	
			case PSN_KILLACTIVE :
	            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
	            return TRUE;
	
			case PSN_RESET :
				CDDestroyKeyserverTreeList (hWndTree);
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

HANDLE 
ShowExistingPropertySheet (
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

PGPError PGPcdExport 
PGPcomdlgPreferences (
		PGPContextRef	Context, 
		HWND			hWnd, 
		INT				startsheet) 
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
	hSemaphore = ShowExistingPropertySheet (SEMAPHORENAME, szTitle);
	if (!hSemaphore) return kPGPError_Win32_AlreadyOpen;

	g_hWndPreferences = (HWND)1;

	hWndParent = hWnd;

	if (Context != NULL)
		ContextRef = Context;
	else
		PGPNewContext( kPGPsdkAPIVersion, &ContextRef);

	PGPsdkLoadDefaultPrefs (ContextRef);

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[0].hInstance = g_hInst;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PREF0);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = EncryptPropDlgProc;
	psp[0].pszTitle = (LPSTR)IDS_TITLE0;
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[1].hInstance = g_hInst;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PREF1);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = FilePropDlgProc;
	psp[1].pszTitle = (LPSTR)IDS_TITLE1;
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[2].hInstance = g_hInst;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PREF2);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = EmailPropDlgProc;
	psp[2].pszTitle = (LPSTR)IDS_TITLE2;
	psp[2].lParam = 0;
	psp[2].pfnCallback = NULL;

	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[3].hInstance = g_hInst;
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PREF3);
	psp[3].pszIcon = NULL;
	psp[3].pfnDlgProc = KeyserverPropDlgProc;
	psp[3].pszTitle = (LPSTR)IDS_TITLE3;
	psp[3].lParam = 0;
	psp[3].pfnCallback = NULL;

	psp[4].dwSize = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags = PSP_USETITLE|PSP_HASHELP;
	psp[4].hInstance = g_hInst;
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PREF4);
	psp[4].pszIcon = NULL;
	psp[4].pfnDlgProc = AdvancedPropDlgProc;
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

	err = kPGPError_NoErr;
#if PGP_BUSINESS_SECURITY
#if PGP_ADMIN_BUILD
	err = PGPcomdlgOpenAdminPrefs (&PrefRefAdmin, TRUE);
#else
	err = PGPcomdlgOpenAdminPrefs (&PrefRefAdmin, FALSE);
#endif
#endif

	if (IsntPGPError (err)) {
		PGPcomdlgOpenClientPrefs (&PrefRefClient);
		PropertySheet(&psh);

		PGPsdkSavePrefs (ContextRef);
		PGPcomdlgCloseClientPrefs (PrefRefClient, TRUE);
	}
#if PGP_BUSINESS_SECURITY
	PGPcomdlgCloseAdminPrefs (PrefRefAdmin, FALSE);
#endif

	if (bReloadKeyring) {
		PGPcomdlgNotifyKeyringChanges (0);
	}

	g_hWndPreferences = NULL;

	if (Context == NULL)
		PGPFreeContext(ContextRef);

	CloseHandle (hSemaphore);

    return returnErr;
}
 


