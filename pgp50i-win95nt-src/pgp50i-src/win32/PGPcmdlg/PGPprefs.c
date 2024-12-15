/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//:PGPprefs.c - handle PGP preferences dialogs
//


#include "pgpcomdlgx.h"
#include "..\include\pgpkeyserversupport.h"

#define MAXCACHETIMELIMIT		300

#define SEMAPHORENAME	("PGPprefsInstSem")

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
extern CHAR g_szbuf[G_BUFLEN];
extern HWND g_hWndPreferences;

// local globals
static HWND hWndParent;
static UINT uReturnValue;
static BOOL bNeedsCentering;
static BOOL bReloadKeyring;

static DWORD aIds[] = {			// Help IDs
     IDC_ENCRYPTTOSELF,	 	 IDH_PGPCDPREF_ENCRYPTTOSELF,
	IDC_USEFASTKEYGEN,		IDH_PGPCDPREF_USEFASTKEYGEN,
     IDC_PRIVATEKEYRING,		 IDH_PGPCDPREF_PRIVATEKEYRING,
	IDC_SETPRIVATEKEYRING,	IDH_PGPCDPREF_SETPRIVATEKEYRING,
     IDC_PUBLICKEYRING,	 	 IDH_PGPCDPREF_PUBLICKEYRING,
	IDC_SETPUBLICKEYRING,	IDH_PGPCDPREF_SETPUBLICKEYRING,
	IDC_ENCRYPTPGPMIME,		IDH_PGPCDPREF_ENCRYPTPGPMIME,
	IDC_SIGNPGPMIME,		IDH_PGPCDPREF_SIGNPGPMIME,
	IDC_PASSCACHEENABLE,	IDH_PGPCDPREF_PASSCACHEENABLE,
	IDC_PASSCACHENUMBER,	IDH_PGPCDPREF_PASSCACHESECONDS,
	IDC_WRAPENABLE,			IDH_PGPCDPREF_WORDWRAPENABLE,
	IDC_WRAPNUMBER,			IDH_PGPCDPREF_WORDWRAPCOLUMN,
	IDC_ENCRYPTBYDEFAULT,	IDH_PGPCDPREF_ENCRYPTBYDEFAULT,
	IDC_SIGNBYDEFAULT,		IDH_PGPCDPREF_SIGNBYDEFAULT,
	IDC_SIGNCACHEENABLE,	IDH_PGPCDPREF_SIGNCACHEENABLE,
	IDC_SIGNCACHENUMBER,	IDH_PGPCDPREF_SIGNCACHESECONDS,
	IDC_WARNONMARGINAL,		IDH_PGPCDPREF_WARNONMARGINAL,
    0,0
};


//----------------------------------------------------|
//  Check for illegal file names

BOOL IsFileNameOK (LPSTR sz) {
	UINT uLen;
	DWORD dwAttrib;

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


//----------------------------------------------------|
//  Message processing function for property sheet dialog

LRESULT WINAPI FilePropDlgProc (HWND hDlg, UINT uMsg,
								WPARAM wParam, LPARAM lParam) {
	OPENFILENAME OpenFileName;
	CHAR szFile[MAX_PATH];
	CHAR szInitDir[MAX_PATH];
	CHAR szFilter[128];
	CHAR szDefExt[8];
	CHAR* p;
	UINT uLen;
	HWND hParent;
	RECT rc;
	BOOL bWarn;
	DWORD dwSize;
	HKEY hKey;
	LRESULT lResult;
	
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

		uLen = sizeof(szFile);
		pgpGetPrefString (kPGPPrefPrivRing, szFile, &uLen);
		szFile[uLen] = '\0';
		SetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile);
		uLen = sizeof(szFile);
		pgpGetPrefString (kPGPPrefPubRing, szFile, &uLen);
		szFile[uLen] = '\0';
		SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile);
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
			LoadString (g_hInst, IDS_PRIVKEYCAPTION, g_szbuf,
								sizeof(g_szbuf));
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner          = hDlg;
			OpenFileName.hInstance          = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter        = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile          = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle         = g_szbuf;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset        = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPRIVRINGEXT, szDefExt,
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt        = szDefExt;
			OpenFileName.lCustData          = 0;
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
			LoadString (g_hInst, IDS_PUBKEYCAPTION, g_szbuf, G_BUFLEN);
			OpenFileName.lStructSize       = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner          = hDlg;
			OpenFileName.hInstance          = (HANDLE)g_hInst;
			OpenFileName.lpstrFilter       = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			OpenFileName.lpstrFile          = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = szInitDir;
			OpenFileName.lpstrTitle         = g_szbuf;
			OpenFileName.Flags			   = OFN_HIDEREADONLY;
			OpenFileName.nFileOffset        = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPUBRINGEXT, szDefExt,
								sizeof(szDefExt));
			OpenFileName.lpstrDefExt        = szDefExt;
			OpenFileName.lCustData          = 0;
			if (GetOpenFileName (&OpenFileName)) {
				SetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile);
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
			uLen = G_BUFLEN;
			pgpGetPrefString (kPGPPrefPrivRing, g_szbuf, &uLen);
			g_szbuf[uLen] = '\0';
			GetDlgItemText (hDlg, IDC_PRIVATEKEYRING, szFile, sizeof(szFile));
			if (lstrcmpi (szFile, g_szbuf) != 0) {
				if (!IsFileNameOK (szFile)) {
					PGPcomdlgMessageBox (g_hWndPreferences,
						IDS_CAPTION, IDS_ILLEGALFILENAME,
						MB_OK | MB_ICONHAND);
					SetWindowLong (hDlg, DWL_MSGRESULT,
						PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}
				pgpSetPrefString (kPGPPrefPrivRing, szFile, lstrlen (szFile));
				bReloadKeyring = TRUE;
			}
			uLen = G_BUFLEN;
			pgpGetPrefString (kPGPPrefPubRing, g_szbuf, &uLen);
			g_szbuf[uLen] = '\0';
			GetDlgItemText (hDlg, IDC_PUBLICKEYRING, szFile, sizeof(szFile));
			if (lstrcmpi (szFile, g_szbuf) != 0) {
				if (!IsFileNameOK (szFile)) {
					PGPcomdlgMessageBox (g_hWndPreferences,
						IDS_CAPTION, IDS_ILLEGALFILENAME,
						MB_OK | MB_ICONHAND);
					SetWindowLong (hDlg, DWL_MSGRESULT,
						PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}
				pgpSetPrefString (kPGPPrefPubRing, szFile, lstrlen (szFile));
				bReloadKeyring = TRUE;
			}
			uReturnValue = PGPCOMDLG_OK;
             SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

			if (bReloadKeyring) {
				LoadString (g_hInst, IDS_PGPKEYSREGID, g_szbuf, G_BUFLEN);
				lResult = RegCreateKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hKey, &dwSize);
				if (lResult == ERROR_SUCCESS) {
					bWarn = TRUE;
					RegSetValueEx (hKey, "ReadOnlyWarn", 0, REG_DWORD,
						(LPBYTE)&bWarn, sizeof(DWORD));
					RegCloseKey (hKey);
				}
			}
			return TRUE;
            break;

		case PSN_KILLACTIVE :
             SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
             return TRUE;
			break;

		case PSN_RESET :
			uReturnValue = PGPCOMDLG_CANCEL;
             SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}

	}
	return FALSE;

}

				
//----------------------------------------------------|
//  Message processing function for property sheet dialog

LRESULT WINAPI EncryptPropDlgProc (HWND hDlg, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	HWND hParent;
	RECT rc;
	UINT u, uEncrypt, uSign;
	Boolean b;

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

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (kPGPPrefEncryptToSelf, &b));
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTTOSELF, BST_UNCHECKED);

		PGPcomdlgErrorMessage (pgpGetPrefNumber (
								kPGPPrefMailPassCacheDuration, &u));
		SetDlgItemInt (hDlg, IDC_PASSCACHENUMBER, u, FALSE);

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMailPassCacheEnable, &b));
		if (b) {
			CheckDlgButton (hDlg, IDC_PASSCACHEENABLE, BST_CHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_PASSCACHENUMBER), TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_PASSCACHEENABLE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_PASSCACHENUMBER), FALSE);
		}

		PGPcomdlgErrorMessage (pgpGetPrefNumber (
								kPGPPrefSignCacheDuration, &u));
		SetDlgItemInt (hDlg, IDC_SIGNCACHENUMBER, u, FALSE);

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefSignCacheEnable, &b));
		if (b) {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_CHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_SIGNCACHENUMBER), TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_SIGNCACHEENABLE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_SIGNCACHENUMBER), FALSE);
		}

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMarginallyValidWarning, &b));
		if (b) CheckDlgButton (hDlg, IDC_WARNONMARGINAL, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_WARNONMARGINAL, BST_UNCHECKED);

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (kPGPPrefFastKeyGen, &b));
		if (b) CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_USEFASTKEYGEN, BST_UNCHECKED);

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

		case IDC_PASSCACHEENABLE :
			if (IsDlgButtonChecked (hDlg, IDC_PASSCACHEENABLE) == BST_CHECKED)
				EnableWindow (GetDlgItem (hDlg, IDC_PASSCACHENUMBER), TRUE);
			else
				EnableWindow (GetDlgItem (hDlg, IDC_PASSCACHENUMBER), FALSE);
			break;

		case IDC_SIGNCACHEENABLE :
			if (IsDlgButtonChecked (hDlg, IDC_SIGNCACHEENABLE) == BST_CHECKED)
				EnableWindow (GetDlgItem (hDlg, IDC_SIGNCACHENUMBER), TRUE);
			else
				EnableWindow (GetDlgItem (hDlg, IDC_SIGNCACHENUMBER), FALSE);
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
			uEncrypt = GetDlgItemInt (hDlg, IDC_PASSCACHENUMBER, NULL, FALSE);
			uSign = GetDlgItemInt (hDlg, IDC_SIGNCACHENUMBER, NULL, FALSE);
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
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
								kPGPPrefEncryptToSelf, b));
			
			PGPcomdlgErrorMessage (pgpSetPrefNumber (
								kPGPPrefMailPassCacheDuration, uEncrypt));

			b = FALSE;
			if (uEncrypt > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_PASSCACHEENABLE) ==
							BST_CHECKED)
					b = TRUE;
			}
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
									kPGPPrefMailPassCacheEnable, b));
			
			PGPcomdlgErrorMessage (pgpSetPrefNumber (
									kPGPPrefSignCacheDuration, uSign));

			b = FALSE;
			if (uSign > 0) {
				if (IsDlgButtonChecked (hDlg, IDC_SIGNCACHEENABLE) ==
							BST_CHECKED)
					b = TRUE;
			}
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
									kPGPPrefSignCacheEnable, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_WARNONMARGINAL) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
									kPGPPrefMarginallyValidWarning, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_USEFASTKEYGEN) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (kPGPPrefFastKeyGen, b));
			
			uReturnValue = PGPCOMDLG_OK;
             SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
             return TRUE;

		case PSN_KILLACTIVE :
             SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
             return TRUE;

		case PSN_RESET :
			uReturnValue = PGPCOMDLG_CANCEL;
             SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}


//----------------------------------------------------|
//  Message processing function for property sheet dialog

LRESULT WINAPI EmailPropDlgProc (HWND hDlg, UINT uMsg,
								 WPARAM wParam, LPARAM lParam) {

	HWND hParent;
	RECT rc;
	Boolean b;
	UINT u;

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
	
		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMailEncryptPGPMIME, &b));
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTPGPMIME, BST_UNCHECKED);

		PGPcomdlgErrorMessage (pgpGetPrefNumber (
								kPGPPrefMailWordWrapWidth, &u));
		SetDlgItemInt (hDlg, IDC_WRAPNUMBER, u, FALSE);

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMailWordWrapEnable, &b));
		if (b) {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_CHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), TRUE);
		}
		else {
			CheckDlgButton (hDlg, IDC_WRAPENABLE, BST_UNCHECKED);
			EnableWindow (GetDlgItem (hDlg, IDC_WRAPNUMBER), FALSE);
		}

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMailEncryptDefault, &b));
		if (b) CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_ENCRYPTBYDEFAULT, BST_UNCHECKED);

		PGPcomdlgErrorMessage (pgpGetPrefBoolean (
								kPGPPrefMailSignDefault, &b));
		if (b) CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_SIGNBYDEFAULT, BST_UNCHECKED);

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
			if (IsDlgButtonChecked (hDlg, IDC_WRAPENABLE) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
										kPGPPrefMailWordWrapEnable, b));
			
			if (b) {
				u = GetDlgItemInt (hDlg, IDC_WRAPNUMBER, NULL, FALSE);
				if (u == 0) {
					PGPcomdlgMessageBox (hDlg, IDS_CAPTION,
										IDS_INVALIDWRAP, MB_OK|MB_ICONHAND);
					SetWindowLong (hDlg, DWL_MSGRESULT,
										PSNRET_INVALID_NOCHANGEPAGE);
					return TRUE;
				}
				PGPcomdlgErrorMessage (pgpSetPrefNumber (
										kPGPPrefMailWordWrapWidth, u));
			}

			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTPGPMIME) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
										kPGPPrefMailEncryptPGPMIME, b));
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
										kPGPPrefMailSignPGPMIME, b));
						
			if (IsDlgButtonChecked (hDlg, IDC_ENCRYPTBYDEFAULT) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
										kPGPPrefMailEncryptDefault, b));
			
			if (IsDlgButtonChecked (hDlg, IDC_SIGNBYDEFAULT) ==
						BST_CHECKED)
				b = TRUE;
			else
				b = FALSE;
			PGPcomdlgErrorMessage (pgpSetPrefBoolean (
										kPGPPrefMailSignDefault, b));
			
			uReturnValue = PGPCOMDLG_OK;
             SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
             return PSNRET_NOERROR;

		case PSN_KILLACTIVE :
             SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
             return TRUE;

		case PSN_RESET :
			uReturnValue = PGPCOMDLG_CANCEL;
              SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;
		}
	}

	return FALSE;

}


//-------------------------------------------------------------------|
// Check for previously running instances

HANDLE ShowExistingPropertySheet (LPSTR lpszSemName,
			    LPSTR lpszWindowTitle) {

    HANDLE hSem;
    HWND hWndMe;

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


//----------------------------------------------------|
//  Create property sheet dialog

UINT PGPExport PGPcomdlgPreferences (HWND hWnd, INT startsheet) {
     PROPSHEETPAGE psp[4];
     PROPSHEETHEADER psh;
	UINT uKeyServerPage;
	UINT uNumPages = 0;
	CHAR szTitle[128];
	HANDLE hSemaphore;

	//assume no KeyServer page
	uNumPages = (sizeof(psp) / sizeof(PROPSHEETPAGE)) - 1;

	// get title of property sheet
	LoadString (g_hInst, IDS_PROPCAPTION, szTitle, sizeof(szTitle));

	// if preferences dialog already exists, move it to the foreground
	hSemaphore = ShowExistingPropertySheet (SEMAPHORENAME, szTitle);
	if (!hSemaphore) return PGPCOMDLG_ALREADYOPEN;

	g_hWndPreferences = (HWND)1;

	hWndParent = hWnd;
	if (PGPcomdlgErrorMessage (pgpLibInit ())) return PGPCOMDLG_CANCEL;

	pgpLoadPrefs ();

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

	if(LoadKeyserverSupport() == SUCCESS) {
		uKeyServerPage = uNumPages;
		if (SetupKeyServerSettingsPage (&psp[uKeyServerPage]) == SUCCESS)
			uNumPages++;
	}

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = NULL;
	psh.hInstance = g_hInst;
	psh.pszIcon = NULL;
	psh.pszCaption = (LPSTR) szTitle;
	psh.nPages = uNumPages;
	psh.nStartPage = startsheet;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback = NULL;

	bNeedsCentering = TRUE;
	bReloadKeyring = FALSE;

	PropertySheet(&psh);

	pgpSavePrefs ();
	pgpLibCleanup ();

	if (bReloadKeyring) {
		PGPcomdlgNotifyKeyringChanges (0);
	}

	g_hWndPreferences = NULL;

	UnLoadKeyserverSupport();

	CloseHandle (hSemaphore);

    return uReturnValue;
}
