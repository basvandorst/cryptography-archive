/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	check keyring files for suspicious conditions; if found, post dialog to 
	let user deal with them.

	$Id: CDcheck.c,v 1.7 1997/08/25 16:39:39 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpcdlgx.h"
#include <time.h>

#define CANCEL		0
#define USESELECTED	1
#define SELECTNEW	2

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
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


//	_________________________________________________
//
//	Convert Windows FileTime to string representation

VOID 
FileTimeToString (
		  FILETIME	ft, 
		  LPSTR		sz) 
{
	FILETIME	ftLocal;
	SYSTEMTIME	st;
	CHAR		szTmp[64];

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

//	___________________________________________________
//
//  load keyring file information into global variables

VOID 
LoadFileInfo (PGPContextRef Context) 
{
	HANDLE			hFile;
	CHAR*			p;
	PGPError		err;
	PGPFileSpecRef	fileRef;
	LPSTR			lpsz;

	// public key ring file
	bPubRing = FALSE;
	err = PGPsdkPrefGetFileSpec (Context, kPGPsdkPref_PublicKeyring,
									&fileRef);
	if (IsntPGPError (err) && fileRef) {
		PGPGetFullPathFromFileSpec (fileRef, &lpsz);
		lstrcpy (szPubRing, lpsz);
		PGPFreeData (lpsz);
		PGPFreeFileSpec (fileRef);
		hFile = CreateFile (szPubRing, 0, FILE_SHARE_WRITE, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			bPubRing = TRUE;
			dwPubRing = GetFileSize (hFile, NULL);
			GetFileTime (hFile, NULL, NULL, &ftPubRing);
			CloseHandle (hFile);
		}
	}

	// private key ring file
	bPrivRing = FALSE;
	err = PGPsdkPrefGetFileSpec (Context, kPGPsdkPref_PrivateKeyring,
								 &fileRef);
	if (IsntPGPError (err) && fileRef) {
		PGPGetFullPathFromFileSpec (fileRef, &lpsz);
		lstrcpy (szPrivRing, lpsz);
		PGPFreeData (lpsz);
		PGPFreeFileSpec (fileRef);
		hFile = CreateFile (szPrivRing, 0, FILE_SHARE_WRITE, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			bPrivRing = TRUE;
			dwPrivRing = GetFileSize (hFile, NULL);
			GetFileTime (hFile, NULL, NULL, &ftPrivRing);
			CloseHandle (hFile);
		}
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

//	________________________________________________________
//
//  Check global keyring variables for suspicious conditions

UINT 
VerifyDefaultKeyRings (VOID) 
{

	if (bPubRingBackup || bPrivRingBackup) {
		if (!bPubRing || !bPrivRing) 
			return kPGPError_FileNotFound;
		if ((dwPubRing == 0) && (dwPubRingBackup > 0)) 
			return kPGPError_FileOpFailed; 
		if ((dwPrivRing == 0) && (dwPrivRingBackup > 0)) 
			return kPGPError_FileOpFailed; 
	}
	return kPGPError_NoErr;
}

//	______________________________________________________
//
//  Query for use of backup files Dialog Message procedure

UINT CALLBACK 
QueryBackupsDlgProc (
		 HWND	hDlg, 
		 UINT	uMsg, 
		 WPARAM wParam,
		 LPARAM lParam) 
{
	CHAR	szFormat[32];
	CHAR	szDateTime[128];
	CHAR	sz[256];
	CHAR*	p;

	switch (uMsg) {

	case WM_INITDIALOG:
		g_hWndCheckKeyrings = hDlg;

		// pub ring
		SetDlgItemText (hDlg, IDC_PUBRING, szPubRing);
		if (bPubRing) {
			FileTimeToString (ftPubRing, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat, 
							sizeof(szFormat));
			wsprintf (sz, szFormat, dwPubRing, szDateTime);
		}
		else 
			LoadString (g_hInst, IDS_FILENOTEXIST, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_PUBRINGINFO, sz);

		// backup pub ring
		SetDlgItemText (hDlg, IDC_PUBRINGBACKUP, szPubRingBackup);
		if (bPubRingBackup) {
			FileTimeToString (ftPubRingBackup, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat, 
							sizeof(szFormat));
			wsprintf (sz, szFormat, dwPubRingBackup, szDateTime);
			if (dwPubRing > 0) 
				CheckRadioButton (hDlg, IDC_PUBRING, 
									IDC_PUBRINGBACKUP, IDC_PUBRING);
			else 
				CheckRadioButton (hDlg, IDC_PUBRING, 
									IDC_PUBRINGBACKUP, IDC_PUBRINGBACKUP);
		}
		else {
			LoadString (g_hInst, IDS_FILENOTEXIST, sz, sizeof(sz));
			CheckRadioButton (hDlg, IDC_PUBRING, 
									IDC_PUBRINGBACKUP, IDC_PUBRING);
			EnableWindow (GetDlgItem (hDlg, IDC_PUBRINGBACKUP), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_PUBRINGBACKUPINFO), FALSE);
		}
		SetDlgItemText (hDlg, IDC_PUBRINGBACKUPINFO, sz);

		// priv ring
		SetDlgItemText (hDlg, IDC_PRIVRING, szPrivRing);
		if (bPrivRing) {
			FileTimeToString (ftPrivRing, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat, 
								sizeof(szFormat));
			wsprintf (sz, szFormat, dwPrivRing, szDateTime);
		}
		else 
			LoadString (g_hInst, IDS_FILENOTEXIST, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDC_PRIVRINGINFO, sz);

		// backup priv ring
		SetDlgItemText (hDlg, IDC_PRIVRINGBACKUP, szPrivRingBackup);
		if (bPrivRingBackup) {
			FileTimeToString (ftPrivRingBackup, szDateTime);
			LoadString (g_hInst, IDS_FILEINFOFORMAT, szFormat, 
								sizeof(szFormat));
			wsprintf (sz, szFormat, dwPrivRingBackup, szDateTime);
			if (dwPrivRing > 0) 
				CheckRadioButton (hDlg, IDC_PRIVRING, 
									IDC_PRIVRINGBACKUP, IDC_PRIVRING);
			else 
				CheckRadioButton (hDlg, IDC_PRIVRING, 
									IDC_PRIVRINGBACKUP, IDC_PRIVRINGBACKUP);
		}
		else {
			LoadString (g_hInst, IDS_FILENOTEXIST, sz, sizeof(sz));
			CheckRadioButton (hDlg, IDC_PRIVRING, 
									IDC_PRIVRINGBACKUP, IDC_PRIVRING);
			EnableWindow (GetDlgItem (hDlg, IDC_PRIVRINGBACKUP), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_PRIVRINGBACKUPINFO), FALSE);
		}
		SetDlgItemText (hDlg, IDC_PRIVRINGBACKUPINFO, sz);

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

//	________________________________________________________
//
//  Check for problems with keyring file
//  if problems exist, post dialog for selecting backup file.

PGPError PGPcdExport 
PGPcomdlgCheckKeyRings (
		PGPContextRef	Context,
		HWND			hWnd, 
		BOOL			bForceQuery) 
{
	PGPError Retval;
	BOOL bExit;

	if (g_hWndCheckKeyrings) return kPGPError_Win32_AlreadyOpen;
	g_hWndCheckKeyrings = (HWND)1;

	do {
		LoadFileInfo (Context);
		Retval = VerifyDefaultKeyRings ();

		if ((Retval != kPGPError_NoErr) || bForceQuery) {
			Retval = DialogBox (g_hInst, 
								 MAKEINTRESOURCE (IDD_QUERYUSEBACKUPS), 
								 hWnd, QueryBackupsDlgProc);
			switch (Retval) {
			case USESELECTED :
				{
					PGPKeySetRef keysetDummy;
					PGPOpenDefaultKeyRings (Context, 
						kPGPKeyRingOpenFlags_Create, &keysetDummy);
					PGPFreeKeySet (keysetDummy);
					Retval = kPGPError_NoErr;
					bExit = TRUE;
					break;
				}
	
			case CANCEL :
				Retval = kPGPError_UserAbort;
				bExit = TRUE;
				break;

			case SELECTNEW :
				PGPcomdlgPreferences (Context, hWnd, 1);
				bExit = FALSE;
				break;
			}
		}
		else bExit = TRUE;

	} while (!bExit);

	g_hWndCheckKeyrings = NULL;

	return Retval;
}
