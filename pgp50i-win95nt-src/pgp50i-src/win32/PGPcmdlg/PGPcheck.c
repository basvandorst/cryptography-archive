/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:PGPcheck.c - verify integrity of keyring files
//
//	$Id: PGPcheck.c,v 1.2.2.3 1997/06/06 14:38:56 pbj Exp $
//

#include "pgpcomdlgx.h"
#include <time.h>
#include "..\include\pgpErr.h"

#define CANCEL		0
#define USESELECTED	1
#define SELECTNEW	2

// external globals
extern HINSTANCE g_hInst;
extern char g_szHelpFile[MAX_PATH];
extern CHAR g_szbuf[G_BUFLEN];
extern HWND g_hWndCheckKeyrings;


// local globals
static CHAR szPubRing[MAX_PATH];
static CHAR szPrivRing[MAX_PATH];
static CHAR szPubRingBackup[MAX_PATH];
static CHAR szPrivRingBackup[MAX_PATH];
static CHAR szPubRingBackup0[MAX_PATH];
static CHAR szPrivRingBackup0[MAX_PATH];

static BOOL bPubRing;
static BOOL bPrivRing;
static BOOL bPubRingBackup;
static BOOL bPrivRingBackup;

static FILETIME ftPubRing;
static FILETIME ftPrivRing;
static FILETIME ftPubRingBackup;
static FILETIME ftPrivRingBackup;

static DWORD dwPubRing;
static DWORD dwPrivRing;
static DWORD dwPubRingBackup;
static DWORD dwPrivRingBackup;

static DWORD aIds[] = {			// Help IDs
    0,0
};


//----------------------------------------------------|
// Convert File to 'C' time

VOID FileTimeToString (FILETIME ft, LPSTR sz) {
	FILETIME ftLocal;
	SYSTEMTIME st;
	CHAR szTmp[64];

	lstrcpy (sz, "???");

	if (FileTimeToLocalFileTime (&ft, &ftLocal)) {
		if (FileTimeToSystemTime (&ftLocal, &st)) {

			if (GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, &st,
							NULL, szTmp, sizeof (szTmp))) {
				lstrcpy (sz, szTmp);
				lstrcat (sz, " ");

				if (GetTimeFormat (LOCALE_USER_DEFAULT, 0, &st,
						NULL, szTmp, sizeof (szTmp))) {
					lstrcat (sz, szTmp);
				}
			}
		}
	}
}


//----------------------------------------------------|
//  get file information

VOID LoadFileInfo (void) {
	UINT u;
	HANDLE hFile;
	CHAR* p;

	// public key ring file
	bPubRing = FALSE;
	u = sizeof (szPubRing);
	PGPcomdlgErrorMessage (pgpGetPrefString (kPGPPrefPubRing, szPubRing, &u));
	szPubRing[u] = '\0';

	hFile = CreateFile (szPubRing, 0, FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		bPubRing = TRUE;
		dwPubRing = GetFileSize (hFile, NULL);
		GetFileTime (hFile, NULL, NULL, &ftPubRing);
		CloseHandle (hFile);
	}

	// private key ring file
	bPrivRing = FALSE;
	u = sizeof (szPrivRing);
	PGPcomdlgErrorMessage (pgpGetPrefString (kPGPPrefPrivRing,
											 szPrivRing, &u));
	szPrivRing[u] = '\0';

	hFile = CreateFile (szPrivRing, 0, FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		bPrivRing = TRUE;
		dwPrivRing = GetFileSize (hFile, NULL);
		GetFileTime (hFile, NULL, NULL, &ftPrivRing);
		CloseHandle (hFile);
	}

	// public key ring file backup
	bPubRingBackup = FALSE;
	lstrcpy (szPubRingBackup, szPubRing);
	p = strchr (szPubRingBackup, '.');
	if (p) {
		*p = '\0';
		lstrcat (szPubRingBackup, ".bak");

		hFile = CreateFile (szPubRingBackup, 0, FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			bPubRingBackup = TRUE;
			dwPubRingBackup = GetFileSize (hFile, NULL);
			GetFileTime (hFile, NULL, NULL, &ftPubRingBackup);
			CloseHandle (hFile);
		}
	}

	// private key ring file backup
	bPrivRingBackup = FALSE;
	lstrcpy (szPrivRingBackup, szPrivRing);
	p = strchr (szPrivRingBackup, '.');
	if (p) {
		*p = '\0';
		lstrcat (szPrivRingBackup, ".bak");

		hFile = CreateFile (szPrivRingBackup, 0, FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			bPrivRingBackup = TRUE;
			dwPrivRingBackup = GetFileSize (hFile, NULL);
			GetFileTime (hFile, NULL, NULL, &ftPrivRingBackup);
			CloseHandle (hFile);
		 }
	}
}


//----------------------------------------------------|
//  Check for suspicious conditions

BOOL VerifyDefaultKeyRings (void) {

	if (bPubRingBackup || bPrivRingBackup) {
		if (!bPubRing || !bPrivRing)
			return PGPERR_GENERIC;
		if ((dwPubRing == 0) && (dwPubRingBackup > 0))
			return PGPERR_GENERIC;
		if ((dwPrivRing == 0) && (dwPrivRingBackup > 0))
			return PGPERR_GENERIC;
	}
	return PGPERR_OK;
}


//----------------------------------------------------|
//  Query for use of backup files Dialog Message procedure

UINT CALLBACK QueryBackupsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	CHAR szFormat[32];
	CHAR szDateTime[128];
	CHAR* p;

	switch (uMsg) {

	case WM_INITDIALOG:
		g_hWndCheckKeyrings = hDlg;

		// pub ring
		SetDlgItemText (hDlg, IDC_PUBRING, szPubRing);
		if (bPubRing) {
			FileTimeToString (ftPubRing, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat,
							sizeof(szFormat));
			wsprintf (g_szbuf, szFormat, dwPubRing, szDateTime);
		}
		else
			LoadString (g_hInst, IDS_FILENOTEXIST, g_szbuf, sizeof(g_szbuf));
		SetDlgItemText (hDlg, IDC_PUBRINGINFO, g_szbuf);

		// backup pub ring
		SetDlgItemText (hDlg, IDC_PUBRINGBACKUP, szPubRingBackup);
		if (bPubRingBackup) {
			FileTimeToString (ftPubRingBackup, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat,
							sizeof(szFormat));
			wsprintf (g_szbuf, szFormat, dwPubRingBackup, szDateTime);
			if (dwPubRing > 0)
				CheckRadioButton (hDlg, IDC_PUBRING,
									IDC_PUBRINGBACKUP, IDC_PUBRING);
			else
				CheckRadioButton (hDlg, IDC_PUBRING,
									IDC_PUBRINGBACKUP, IDC_PUBRINGBACKUP);
		}
		else {
			LoadString (g_hInst, IDS_FILENOTEXIST, g_szbuf, sizeof(g_szbuf));
			CheckRadioButton (hDlg, IDC_PUBRING,
									IDC_PUBRINGBACKUP, IDC_PUBRING);
			EnableWindow (GetDlgItem (hDlg, IDC_PUBRINGBACKUP), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_PUBRINGBACKUPINFO), FALSE);
		}
		SetDlgItemText (hDlg, IDC_PUBRINGBACKUPINFO, g_szbuf);

		// priv ring
		SetDlgItemText (hDlg, IDC_PRIVRING, szPrivRing);
		if (bPrivRing) {
			FileTimeToString (ftPrivRing, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat,
								sizeof(szFormat));
			wsprintf (g_szbuf, szFormat, dwPrivRing, szDateTime);
		}
		else
			LoadString (g_hInst, IDS_FILENOTEXIST, g_szbuf, sizeof(g_szbuf));
		SetDlgItemText (hDlg, IDC_PRIVRINGINFO, g_szbuf);

		// backup priv ring
		SetDlgItemText (hDlg, IDC_PRIVRINGBACKUP, szPrivRingBackup);
		if (bPrivRingBackup) {
			FileTimeToString (ftPrivRingBackup, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat,
								sizeof(szFormat));
			wsprintf (g_szbuf, szFormat, dwPrivRingBackup, szDateTime);
			if (dwPrivRing > 0)
				CheckRadioButton (hDlg, IDC_PRIVRING,
									IDC_PRIVRINGBACKUP, IDC_PRIVRING);
			else
				CheckRadioButton (hDlg, IDC_PRIVRING,
									IDC_PRIVRINGBACKUP, IDC_PRIVRINGBACKUP);
		}
		else {
			LoadString (g_hInst, IDS_FILENOTEXIST, g_szbuf, sizeof(g_szbuf));
			CheckRadioButton (hDlg, IDC_PRIVRING,
									IDC_PRIVRINGBACKUP, IDC_PRIVRING);
			EnableWindow (GetDlgItem (hDlg, IDC_PRIVRINGBACKUP), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_PRIVRINGBACKUPINFO), FALSE);
		}
		SetDlgItemText (hDlg, IDC_PRIVRINGBACKUPINFO, g_szbuf);

		return TRUE;

    case WM_DESTROY:
         break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDC_USESELECTEDFILES :
			// move around pub rings, if desired
			if (IsDlgButtonChecked (hDlg, IDC_PUBRINGBACKUP) == BST_CHECKED) {
				lstrcpy (szPubRingBackup0, szPubRing);
				p = strchr (szPubRingBackup0, '.');
				*p = '\0';
				lstrcat (szPubRingBackup0, "0.bak");
				CopyFile (szPubRing, szPubRingBackup0, FALSE);
				CopyFile (szPubRingBackup, szPubRing, FALSE);
			}

			// move around priv rings, if desired
			if (IsDlgButtonChecked (hDlg,IDC_PRIVRINGBACKUP) == BST_CHECKED) {
				lstrcpy (szPrivRingBackup0, szPrivRing);
				p = strchr (szPrivRingBackup0, '.');
				*p = '\0';
				lstrcat (szPrivRingBackup0, "0.bak");
				CopyFile (szPrivRing, szPrivRingBackup0, FALSE);
				CopyFile (szPrivRingBackup, szPrivRing, FALSE);
			}

			EndDialog (hDlg, USESELECTED);
			break;

		case IDC_SELECTNEW :
			EndDialog (hDlg, SELECTNEW);
			break;

		case IDHELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT,
						IDH_PGPCDCHECK_CHECKDIALOG);
			break;

		case IDCANCEL:
			EndDialog (hDlg, CANCEL);
			break;
		}
		return TRUE;
	 }
	return FALSE;
}


//----------------------------------------------------|
//  Check for problems with keyring file
//  if problems exist, post dialog for selecting backup file.

UINT PGPExport PGPcomdlgCheckKeyRings (HWND hWnd, BOOL bForceQuery) {
	UINT uRetval;
	BOOL bExit;

	if (g_hWndCheckKeyrings) return PGPCOMDLG_ALREADYOPEN;
	g_hWndCheckKeyrings = (HWND)1;

	if (!PGPcomdlgErrorMessage (pgpLibInit ())) {
		do {
			LoadFileInfo ();
			uRetval = VerifyDefaultKeyRings ();
			if ((uRetval != PGPERR_OK) || bForceQuery) {
				uRetval = DialogBox (g_hInst,
									 MAKEINTRESOURCE (IDD_QUERYUSEBACKUPS),
									 hWnd, QueryBackupsDlgProc);
				switch (uRetval) {
				case USESELECTED :
					uRetval = PGPCOMDLG_OK;
					bExit = TRUE;
					break;
	
				case CANCEL :
					uRetval = PGPCOMDLG_CANCEL;
					bExit = TRUE;
					break;

				case SELECTNEW :
					PGPcomdlgPreferences (hWnd, 1);
					bExit = FALSE;
					break;
				  }
			}
			else bExit = TRUE;
		} while (!bExit);
		pgpLibCleanup ();
	}
	else uRetval = PGPCOMDLG_LIBERROR;

	g_hWndCheckKeyrings = NULL;

	return uRetval;
}
