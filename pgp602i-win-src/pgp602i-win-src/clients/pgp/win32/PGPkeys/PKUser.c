/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PKUser.c - miscellaneous user interface routines
	

	$Id: PKUser.c,v 1.27.2.1 1998/10/05 21:30:32 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

// project header files
#include "pgpkeysx.h"

// pgp header files
#include "pgpSDKPrefs.h"

// system header files
#include <commdlg.h>

// external global variables
extern HINSTANCE		g_hInst;
extern PGPContextRef	g_Context;

//	____________________________________
//
//	Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT 
PKMessageBox (
		HWND	hWnd, 
		INT		iCaption, 
		INT		iMessage,
		ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}

//	____________________________________
//
//	Backup warning dialog message procedure

static BOOL CALLBACK 
sBackupWarnDlgProc(
		HWND	hWndDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	switch (uMsg) {

	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDC_BACKUPNOW:
			EndDialog (hWndDlg, IDC_BACKUPNOW);
			break;

		case IDC_DONTBACKUP :
			EndDialog (hWndDlg, IDC_DONTBACKUP);
			break;

		case IDCANCEL:
			EndDialog (hWndDlg, IDCANCEL);
			break;

		}
		return TRUE;
	}
	return FALSE;
}

//	____________________________________
//
//	Warn user of need for backup

BOOL 
PKBackupWarn (HWND hWnd, BOOL* pbBackup) 
{
	INT		iReturn;

	iReturn = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_BACKUPWARN), hWnd, 
			sBackupWarnDlgProc); 

	if (iReturn == IDCANCEL) {
		*pbBackup = FALSE;
		return FALSE;
	}

	else if (iReturn == IDC_BACKUPNOW) {
		*pbBackup = TRUE;
		return TRUE;
	}

	else {
		*pbBackup = FALSE;
		return TRUE;
	}
}


//	____________________________________
//
//	Warn user of existing split key dialogs

BOOL 
PKSplitKeyWarn (HWND hWnd) 
{
	if (PKMessageBox (hWnd, 
					IDS_CAPTION, 
					IDS_SPLITKEYWARNING, 
					MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		return TRUE;
	else
		return FALSE;
}


//	____________________________________
//
//	Actually do the backing-up

VOID 
PKBackup (HWND hWnd) 
{
	BOOL			bFileError;
	HKEY			hKey;
	LONG			lResult;
	DWORD			dwValueType, dwSize;
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szDir[MAX_PATH];
	CHAR			sz256[256];
	CHAR			szDefExt[8];
	CHAR			szFilter[256];
	PGPError		err;
	PGPFileSpecRef	fileref;
	LPSTR			lpsz;
	CHAR*			p;

	do {
		bFileError = FALSE;

		lResult = RegOpenKeyEx (HKEY_CURRENT_USER, 
			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\"
			"Shell Folders", 
			0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS) {

		// backup public keyring file
			dwSize = sizeof(szDir);
			lResult = RegQueryValueEx (hKey, "Desktop", 0, &dwValueType, 
				(LPBYTE)&szDir, &dwSize);
			if (lResult != ERROR_SUCCESS) 
				LoadString (g_hInst, IDS_BACKUPDEFAULTDIR, 
											szDir, sizeof(szDir));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hWnd;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			LoadString (g_hInst, IDS_PUBBACKUPFILTER, 
				szFilter, sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
	
			err = PGPsdkPrefGetFileSpec (g_Context, 
						kPGPsdkPref_PublicKeyring, &fileref);
			if (IsntPGPError (PGPclErrorBox (NULL, err))) {
				PGPGetFullPathFromFileSpec (fileref, &lpsz);
				p = strrchr (lpsz, '\\');
				if (p) {
					p++;
					lstrcpy (szFile, p);
				}
				else lstrcpy (szFile, lpsz);
				p = strrchr (szFile, '.');
				if (p) *p = '\0';
				PGPFreeData (lpsz);
				PGPFreeFileSpec (fileref);
			}
			else {
				LoadString (g_hInst, IDS_PUBBACKUPDEFAULTFILE, 
									szFile, sizeof(szFile));
			}
					
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szDir;
			LoadString (g_hInst, IDS_PUBBACKUPCAPTION, sz256, 
												sizeof(sz256));
			OpenFileName.lpstrTitle        = sz256;
			OpenFileName.Flags			   = OFN_HIDEREADONLY|
											 OFN_OVERWRITEPROMPT|
											 OFN_NOREADONLYRETURN|
											 OFN_NOCHANGEDIR;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPUBKEYRINGEXTENSION, 
									szDefExt, sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
	
			if (GetSaveFileName (&OpenFileName)) { 
				err = PGPsdkPrefGetFileSpec (g_Context, 
						kPGPsdkPref_PublicKeyring, &fileref);
				if (IsntPGPError (PGPclErrorBox (NULL, err))) {
	
					PGPGetFullPathFromFileSpec (fileref, &lpsz);
					if (!CopyFile (lpsz, szFile, FALSE)) 
						bFileError = TRUE;
					PGPFreeData (lpsz);
					PGPFreeFileSpec (fileref);
	
				}
				else bFileError = TRUE;
	
				if (bFileError) {
					PKMessageBox (hWnd, IDS_CAPTION, IDS_BACKUPERROR, 
										MB_OK|MB_ICONSTOP);
				}
			}

			if (bFileError) continue;
	
			// backup private keyring file
			lstrcpy (szDir, szFile);
			p = strrchr (szDir, '\\');
			if (p) *p = '\0';
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner         = hWnd;
			OpenFileName.hInstance         = (HANDLE)g_hInst;
			LoadString (g_hInst, IDS_BACKUPFILTER, 
				szFilter, sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
	
			err = PGPsdkPrefGetFileSpec (g_Context, 
						kPGPsdkPref_PrivateKeyring, &fileref);
			if (IsntPGPError (PGPclErrorBox (NULL, err))) {
				PGPGetFullPathFromFileSpec (fileref, &lpsz);
				p = strrchr (lpsz, '\\');
				if (p) {
					p++;
					lstrcpy (szFile, p);
				}
				else lstrcpy (szFile, lpsz);
				p = strrchr (szFile, '.');
				if (p) *p = '\0';
				PGPFreeData (lpsz);
				PGPFreeFileSpec (fileref);
			}
			else {
				LoadString (g_hInst, IDS_BACKUPDEFAULTFILE, 
									szFile, sizeof(szFile));
			}
	
			OpenFileName.lpstrFile         = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szDir;
			LoadString (g_hInst, IDS_BACKUPCAPTION, sz256, sizeof(sz256));
			OpenFileName.lpstrTitle        = sz256;
			OpenFileName.Flags			   = OFN_HIDEREADONLY|
											 OFN_OVERWRITEPROMPT|
											 OFN_NOREADONLYRETURN|
											 OFN_NOCHANGEDIR;
			OpenFileName.nFileOffset       = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFKEYRINGEXTENSION, 
									szDefExt, sizeof(szDefExt));
			OpenFileName.lpstrDefExt       = szDefExt;
			OpenFileName.lCustData         = 0;
	
			if (GetSaveFileName (&OpenFileName)) { 
				err = PGPsdkPrefGetFileSpec (g_Context, 
						kPGPsdkPref_PrivateKeyring, &fileref);
				if (IsntPGPError (PGPclErrorBox (NULL, err))) {

					PGPGetFullPathFromFileSpec (fileref, &lpsz);
					if (!CopyFile (lpsz, szFile, FALSE)) 
						bFileError = TRUE;
					PGPFreeData (lpsz);
					PGPFreeFileSpec (fileref);
	
				}
				else bFileError = TRUE;
	
				if (bFileError) {
					PKMessageBox (hWnd, IDS_CAPTION, IDS_BACKUPERROR, 
										MB_OK|MB_ICONSTOP);
				}
			}
		}
	} while (bFileError);
	
	return;
}
	
//	____________________________________
//
//	Read-only warning dialog message procedure

static BOOL CALLBACK 
sReadOnlyDlgProc (
		HWND	hWndDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	switch (uMsg) {
	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDOK:
			if (IsDlgButtonChecked (hWndDlg, IDC_WARNNOMORE) == BST_CHECKED)
				EndDialog (hWndDlg, 1);
			else 
				EndDialog (hWndDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	____________________________________
//
// Warn user that keyrings are read-only

VOID 
PKReadOnlyWarning (HWND hWnd) 
{
	PGPBoolean		bWarn		= TRUE;
	PGPPrefRef		prefref;
	PGPError		err;

	err = PGPclOpenClientPrefs (
			PGPGetContextMemoryMgr (g_Context),
			&prefref);
	if (IsntPGPError (err)) {
		PGPGetPrefBoolean (prefref, kPGPPrefWarnOnReadOnlyKeyRings, &bWarn);

		if (bWarn) {
			if (DialogBox (g_hInst, MAKEINTRESOURCE (IDD_READONLY), 
						hWnd, sReadOnlyDlgProc)) {
				bWarn = FALSE;
				PGPSetPrefBoolean (prefref, kPGPPrefWarnOnReadOnlyKeyRings, 
									bWarn);
			}
		}

		PGPclCloseClientPrefs (prefref, !bWarn);
	}
}

//	____________________________________
//
//	Post Help About dialog

VOID 
PKHelpAbout (HWND hWnd) 
{
	PGPclHelpAbout (g_Context, hWnd, NULL, NULL, NULL); 
}

//	____________________________________
//
//	Wipe edit box clean

VOID 
PKWipeEditBox (
		HWND hDlg, 
		UINT uID) 
{
	CHAR* p;
	INT i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = pkAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			pkFree (p);
		}
		SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)"");
	}
}

