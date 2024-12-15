/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMAddUser.c - add userID to key
//
//	$Id: KMAddUser.c,v 1.3 1997/05/27 22:07:05 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"
#include "..\include\pgpphrase.h"
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndTree;
extern CHAR g_szbuf[G_BUFLEN];
extern CHAR g_szHelpFile[MAX_PATH];
extern PGPKeySet* g_pKeySetMain;

// local globals
LPSTR pszUserID;

static DWORD aIds[] = {			// Help IDs
    IDC_NEWUSERNAME,	IDH_PGPKEYS_NEWUSERID,
	IDC_NEWEMAILADDR,	IDH_PGPKEYS_NEWEMAILADDR,
    0,0
};


//----------------------------------------------------|
//  Dialog Message procedure
//	When user asks to add a userID to a key, a dialog
//	appears asking for the new userID to be typed in.
//	This is the message processing procedure for that
//	dialog.

BOOL CALLBACK AddUserDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	switch(uMsg) {

	case WM_INITDIALOG:
		return TRUE;

    case WM_HELP:
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
             HELP_WM_HELP, (DWORD) (LPSTR) aIds);
        break;

    case WM_CONTEXTMENU:
         WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
             (DWORD) (LPVOID) aIds);
         break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDOK:
			if (KMConstructUserID (hDlg, IDC_NEWUSERNAME, IDC_NEWEMAILADDR,
									&pszUserID))
				EndDialog (hDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Add User to key
//	This routine is called when the user chooses to add
//	a userID to an existing secret key.

BOOL KMAddUserToKey (HWND hparent) {
	Boolean bNeedsPhrase;
	BOOL bPhraseOK;
	UINT uError;
	PGPKey* pKey;
	LPSTR szPhrase = NULL;

	// get selected key
	pKey = (PGPKey*)KMFocusedObject ();

	// get new userid from user
	if (DialogBox (g_hInst, MAKEINTRESOURCE (IDD_NEWUSERID),
					hparent, AddUserDlgProc)) {
		// if the key has a passphrase, get it from user
		pgpGetKeyBoolean (pKey, kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		do {
			if (bNeedsPhrase) {
				LoadString (g_hInst, IDS_SELKEYPASSPHRASE, g_szbuf, G_BUFLEN);
				uError = PGPcomdlgGetPhrase (hparent, g_szbuf, &szPhrase,
												NULL, NULL, NULL, NULL, 0);
				PGPcomdlgErrorMessage (uError);
				bPhraseOK = (uError == PGPCOMDLG_OK);
			}
			else {
				szPhrase = NULL;
				bPhraseOK = TRUE;
			}
			// add the userid to the key and then update this one key
			// in the TreeList control.  Note that the new userid becomes
			// the primary userid so the sort order may change.
			if (bPhraseOK) {
				uError = pgpAddUserID (pKey, pszUserID, lstrlen (pszUserID),
										szPhrase);
				if (!PGPcomdlgErrorMessage (uError)) {
					if (szPhrase) {
						PGPFreePhrase (szPhrase);
						szPhrase = NULL;
					}
					KMFree (pszUserID);
					KMCommitKeyRingChanges (g_pKeySetMain);
					KMUpdateKeyInTree (g_hWndTree, pKey);
					InvalidateRect (g_hWndTree, NULL, TRUE);
					return TRUE;
				}
			}
			if (szPhrase) {
				PGPFreePhrase (szPhrase);
				szPhrase = NULL;
			}
		} while (bPhraseOK && (uError == PGPERR_KEYDB_BADPASSPHRASE));

		KMFree (pszUserID);
	}

	return FALSE;
}
