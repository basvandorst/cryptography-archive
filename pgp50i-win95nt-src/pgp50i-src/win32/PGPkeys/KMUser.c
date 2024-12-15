/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: KMUser.c - miscellaneous user interface routines
//
//	$Id: KMUser.c,v 1.8 1997/05/21 22:30:59 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"
#include "..\include\pgpVersion.h"
#include "resource.h"

// external global variables
extern CHAR g_szbuf[G_BUFLEN];
extern HINSTANCE g_hInst;
extern HWND g_hWndTree;
extern CHAR g_szHelpFile[MAX_PATH];
extern PGPKeySet* g_pKeySetMain;
extern LONG g_lKeyListSortField;

static DWORD aSignConfirmIds[] = {			// Help IDs
	IDC_SENDSIG, IDH_PGPKEYS_SENDSIGNATURE,
    0,0
};

//----------------------------------------------------|
// Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT KMMessageBox (HWND hWnd, INT iCaption, INT iMessage,
					  ULONG ulFlags) {
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}


//----------------------------------------------------|
// Backup warning dialog message procedure

BOOL CALLBACK BackupWarnDlgProc(HWND hWndDlg, UINT uMsg,
								WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog (hWndDlg, 0);
			break;

		case IDC_BACKUPNOW:
			EndDialog (hWndDlg, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Warn user of need for backup

BOOL KMBackupWarn (HWND hWnd) {
	HKEY hKey;
	LONG lResult;
	DWORD dwValueType, dwSize;
	OPENFILENAME OpenFileName;
	CHAR szFile[MAX_PATH];
	CHAR sz256[256];
	CHAR szDefExt[8];
	CHAR szFilter[256];
	CHAR* p;
	UINT u;

	if (DialogBox (g_hInst, MAKEINTRESOURCE (IDD_BACKUPWARN), hWnd,
			BackupWarnDlgProc)) {
		lResult = RegOpenKeyEx (HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\"
			"Shell Folders",
			0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS) {

			// backup public keyring file
			dwSize = G_BUFLEN;
			lResult = RegQueryValueEx (hKey, "Desktop", 0, &dwValueType,
				(LPBYTE)&g_szbuf, &dwSize);
			if (lResult != ERROR_SUCCESS)
				LoadString (g_hInst, IDS_BACKUPDEFAULTDIR, g_szbuf, G_BUFLEN);
			OpenFileName.lStructSize        = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner          = hWnd;
			OpenFileName.hInstance          = (HANDLE)g_hInst;
			LoadString (g_hInst, IDS_PUBBACKUPFILTER,
				szFilter, sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			OpenFileName.lpstrFilter        = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			u = sizeof (sz256);
			if (!PGPcomdlgErrorMessage (pgpGetPrefString (
								kPGPPrefPubRing, sz256, &u))) {
				sz256[u] = '\0';
				p = strrchr (sz256, '\\');
				if (p) {
					p++;
					lstrcpy (szFile, p);
				}
				else lstrcpy (szFile, sz256);
				p = strrchr (szFile, '.');
				if (p) *p = '\0';
			}
			else {
				LoadString (g_hInst, IDS_PUBBACKUPDEFAULTFILE,
									szFile, sizeof(szFile));
			}
			OpenFileName.lpstrFile          = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = g_szbuf;
			LoadString (g_hInst, IDS_PUBBACKUPCAPTION, sz256, sizeof(sz256));
			OpenFileName.lpstrTitle         = sz256;
			OpenFileName.Flags			    = OFN_HIDEREADONLY|
											 OFN_OVERWRITEPROMPT|
											 OFN_NOREADONLYRETURN|
											 OFN_NOCHANGEDIR;
			OpenFileName.nFileOffset        = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFPUBKEYRINGEXTENSION,
									szDefExt, sizeof(szDefExt));
			OpenFileName.lpstrDefExt        = szDefExt;
			OpenFileName.lCustData          = 0;

			if (GetSaveFileName (&OpenFileName)) {
				u = G_BUFLEN;
				if (!PGPcomdlgErrorMessage (pgpGetPrefString (
								kPGPPrefPubRing, g_szbuf, &u))) {
					g_szbuf[u] = '\0';
					if (!CopyFile (g_szbuf, szFile, FALSE)) {
						KMMessageBox (hWnd, IDS_CAPTION, IDS_BACKUPERROR,
										MB_OK|MB_ICONSTOP);
						return TRUE;
					}
				}
			}

			// backup private keyring file
			lstrcpy (g_szbuf, szFile);
			p = strrchr (g_szbuf, '\\');
			if (p) *p = '\0';
			OpenFileName.lStructSize        = sizeof (OPENFILENAME);
			OpenFileName.hwndOwner          = hWnd;
			OpenFileName.hInstance          = (HANDLE)g_hInst;
			LoadString (g_hInst, IDS_BACKUPFILTER,
				szFilter, sizeof(szFilter));
			while (p = strrchr (szFilter, '@')) *p = '\0';
			OpenFileName.lpstrFilter        = szFilter;
			OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
			OpenFileName.nMaxCustFilter    = 0L;
			OpenFileName.nFilterIndex      = 1L;
			u = sizeof (sz256);
			if (!PGPcomdlgErrorMessage (pgpGetPrefString (
								kPGPPrefPrivRing, sz256, &u))) {
				sz256[u] = '\0';
				p = strrchr (sz256, '\\');
				if (p) {
					p++;
					lstrcpy (szFile, p);
				}
				else lstrcpy (szFile, sz256);
				p = strrchr (szFile, '.');
				if (p) *p = '\0';
			}
			else {
				LoadString (g_hInst, IDS_BACKUPDEFAULTFILE,
									szFile, sizeof(szFile));
			}
			OpenFileName.lpstrFile          = szFile;
			OpenFileName.nMaxFile          = sizeof (szFile);
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = g_szbuf;
			LoadString (g_hInst, IDS_BACKUPCAPTION, sz256, sizeof(sz256));
			OpenFileName.lpstrTitle         = sz256;
			OpenFileName.Flags			   = OFN_HIDEREADONLY|
											 OFN_OVERWRITEPROMPT|
											 OFN_NOREADONLYRETURN|
											 OFN_NOCHANGEDIR;
			OpenFileName.nFileOffset        = 0;
			OpenFileName.nFileExtension    = 0;
			LoadString (g_hInst, IDS_DEFKEYRINGEXTENSION,
									szDefExt, sizeof(szDefExt));
			OpenFileName.lpstrDefExt        = szDefExt;
			OpenFileName.lCustData          = 0;

			if (GetSaveFileName (&OpenFileName)) {
				u = G_BUFLEN;
				if (!PGPcomdlgErrorMessage (pgpGetPrefString (
								kPGPPrefPrivRing, g_szbuf, &u))) {
					g_szbuf[u] = '\0';
					if (!CopyFile (g_szbuf, szFile, FALSE)) {
						KMMessageBox (hWnd, IDS_CAPTION, IDS_BACKUPERROR,
										MB_OK|MB_ICONSTOP);
						return TRUE;
					}
				}
			}

		}
	}

	return FALSE;
}
	

//----------------------------------------------------|
// Bad passphrase dialog message procedure

BOOL CALLBACK BadPhraseDlgProc(HWND hWndDlg, UINT uMsg,
							    WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog (hWndDlg, 0);
			break;

		case IDC_USEBADPHRASE:
			EndDialog (hWndDlg, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Ask user if bad passphrase is acceptable

BOOL KMUseBadPassPhrase (HWND hWnd) {

	return (DialogBox (g_hInst, MAKEINTRESOURCE (IDD_BADPHRASE),
			hWnd, BadPhraseDlgProc) == 1);
}


//----------------------------------------------------|
// Signature confirmation dialog message procedure

BOOL CALLBACK SignConfirmDlgProc(HWND hWndDlg, UINT uMsg,
								 WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG :
		CheckDlgButton (hWndDlg, IDC_SENDSIG, BST_UNCHECKED);
		break;

	case WM_HELP:
	     WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
	       HELP_WM_HELP, (DWORD) (LPSTR) aSignConfirmIds);
	     break;

	case WM_CONTEXTMENU:
	     WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
	       (DWORD) (LPVOID) aSignConfirmIds);
	     break;

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			EndDialog (hWndDlg, 0);
			break;

		case IDOK :
			if (IsDlgButtonChecked (hWndDlg, IDC_SENDSIG) == BST_CHECKED)
				EndDialog (hWndDlg, 2);
			else
				EndDialog (hWndDlg, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Ask user for signature confirmation

BOOL KMSignConfirm (HWND hWnd, BOOL* pbSendToServer) {
	INT iError;

	iError = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_SIGNCONFIRM),
						hWnd, SignConfirmDlgProc);

	switch (iError) {
	case 0 :
		*pbSendToServer = FALSE;
		return FALSE;

	case 1 :
		*pbSendToServer = FALSE;
		return TRUE;

	case 2 :
		*pbSendToServer = TRUE;
		return TRUE;
	}	
}


//----------------------------------------------------|
// Read-only warning dialog message procedure

BOOL CALLBACK ReadOnlyDlgProc(HWND hWndDlg, UINT uMsg,
							  WPARAM wParam, LPARAM lParam) {
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


//----------------------------------------------------|
// Warn user that keyrings are read-only

VOID KMReadOnlyWarning (HWND hWnd) {
	HKEY hKey;
	LONG lResult;
	DWORD dwValueType, dwSize;
	BOOL bWarn;

	LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, g_szbuf, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		dwSize = sizeof (bWarn);
		lResult = RegQueryValueEx (hKey, "ReadOnlyWarn", 0, &dwValueType,
									(LPBYTE)&bWarn, &dwSize);
		RegCloseKey (hKey);
		if ((lResult != ERROR_SUCCESS) || (bWarn)) {
			if (DialogBox (g_hInst, MAKEINTRESOURCE (IDD_READONLY),
							hWnd, ReadOnlyDlgProc)) {
				LoadString (g_hInst, IDS_REGISTRYKEY, g_szbuf, G_BUFLEN);
				lResult = RegCreateKeyEx (HKEY_CURRENT_USER, g_szbuf, 0,
						NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hKey, &dwSize);
				if (lResult == ERROR_SUCCESS) {
					bWarn = FALSE;
					RegSetValueEx (hKey, "ReadOnlyWarn", 0, REG_DWORD,
						(LPBYTE)&bWarn, sizeof(DWORD));
					RegCloseKey (hKey);
				}
			}
		}
	}
}


//----------------------------------------------------|
// Post Help About dialog

VOID KMHelpAbout (HWND hWnd) {
	PGPcomdlgHelpAbout (hWnd, NULL, NULL, NULL);
}


//----------------------------------------------------|
// Scroll to keyboard-selected key
//
//	c =				character just typed by user
//	bStillTyping =	TRUE => this character is part of a previous
//					string of characters => append to end.

BOOL KMSelectByKeyboard (HWND hWnd, LONG c, BOOL bStillTyping) {

	static CHAR* bufptr;
	static CHAR buf[32];
	TL_TREEITEM tli;
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	INT i, iLen;

	if (c == VK_ESCAPE) {
		bufptr = &buf[0];
		tli.hItem = NULL;
		TreeList_Select (g_hWndTree, &tli, TRUE);
		return FALSE;
	}

	if (!bStillTyping) bufptr = &buf[0];
	*bufptr = (CHAR)c;
	bufptr++;
	*bufptr = '\0';

	pKeyList = pgpOrderKeySet (g_pKeySetMain, kPGPUserIDOrdering);
	pKeyIter = pgpNewKeyIter (pKeyList);

	pKey = pgpKeyIterNext (pKeyIter);

	iLen = lstrlen (buf);
	while (pKey) {
		KMGetKeyName (pKey, g_szbuf, sizeof(g_szbuf));
		i = _strnicmp (buf, g_szbuf, iLen);
		if (i == 0) {
			pgpGetKeyUserVal (pKey, (LONG*)&(tli.hItem));
			TreeList_Select (g_hWndTree, &tli, TRUE);
			pgpFreeKeyIter (pKeyIter);
			pgpFreeKeyList (pKeyList);
			return TRUE;
		}
		else if (i < 0) {
			pgpFreeKeyIter (pKeyIter);
			pgpFreeKeyList (pKeyList);
			return FALSE;
		}
		pKey = pgpKeyIterNext (pKeyIter);
	}

	pgpFreeKeyIter (pKeyIter);
	pgpFreeKeyList (pKeyList);
	return FALSE;
}


//----------------------------------------------------|
// check for blank or nonstandard userid name

BOOL KMUserNameOK (HWND hWnd, LPSTR szUserName) {
	CHAR* p1;
	CHAR* p;

	if (!szUserName || (lstrlen (szUserName) <= 0)) {
		KMMessageBox (hWnd, IDS_CAPTION, IDS_NOUSERNAME,
			MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	p1 = &szUserName[1];

	p = strstr (p1, " <");
	if (p) {
		p1 = p;
		p = strchr (p1, '@');
		if (p) {
			p1 = p;
			p = strchr (p1, '>');
			if (p) return TRUE;
		}
	}

	return (KMMessageBox (hWnd, IDS_CAPTION, IDS_NONSTANDARDNAME,
		MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES);

}


//----------------------------------------------------|
// construct userid from name and address edit box contents

BOOL KMConstructUserID (HWND hDlg, UINT uNameIDC,
						UINT uAddrIDC, LPSTR* pszUserID) {
	UINT uNameLen, uAddrLen, uUserIDLen;
	CHAR* p;

	uNameLen = SendDlgItemMessage (hDlg, uNameIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uNameLen == 0) {
		KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERNAME,
			MB_OK|MB_ICONEXCLAMATION);
		SetFocus (GetDlgItem (hDlg, uNameIDC));
		return FALSE;
	}

	uAddrLen = SendDlgItemMessage (hDlg, uAddrIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uAddrLen == 0) {
		if (KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERADDR,
			MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDNO) {
			SetFocus (GetDlgItem (hDlg, uAddrIDC));
			return FALSE;
		}
	}

	uUserIDLen = uNameLen + uAddrLen + 4;
	*pszUserID = KMAlloc (uUserIDLen * sizeof(char));
	if (!*pszUserID) return FALSE;

	p = *pszUserID;
	GetDlgItemText (hDlg, uNameIDC, p, uUserIDLen);

	if (uAddrLen > 0) {
		p += uNameLen;
		lstrcpy (p, " <");

		p += 2;
		GetDlgItemText (hDlg, uAddrIDC, p, uUserIDLen);

		p += uAddrLen;
		lstrcpy (p, ">");
	}
}


//----------------------------------------------------|
// Wipe edit box clean

VOID KMWipeEditBox (HWND hDlg, UINT uID) {
	CHAR* p;
	INT i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = KMAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			KMFree (p);
		}
	}
}
