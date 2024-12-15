/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMAddUsr.c - add userID to key
	

	$Id: KMAddUsr.c,v 1.19 1997/09/26 18:04:41 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include "..\include\pgpphras.h"
#include "resource.h"

#include "pgpclientprefs.h"

#define MAX_FULL_NAME_LEN		126
#define MAX_EMAIL_LEN			126

typedef struct {
	PKEYMAN pKM;
	LPSTR pszUserID;
	PGPBoolean bSyncWithServer;
} ADDNAMESTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static DWORD aIds[] = {			// Help IDs
    IDC_NEWUSERNAME,	IDH_PGPKM_NEWUSERID, 
	IDC_NEWEMAILADDR,	IDH_PGPKM_NEWEMAILADDR,
    0,0 
}; 


//	___________________________________________________
//
//  Dialog Message procedure
//	When user asks to add a userID to a key, a dialog 
//	appears asking for the new userID to be typed in.
//	This is the message processing procedure for that
//	dialog.

BOOL CALLBACK 
AddUserDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{

	ADDNAMESTRUCT* pans;

	switch(uMsg) {

	case WM_INITDIALOG:
		pans = (ADDNAMESTRUCT*)lParam;
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage (hDlg, IDC_NEWUSERNAME, EM_SETLIMITTEXT, 
				MAX_FULL_NAME_LEN, 0);
		SendDlgItemMessage (hDlg, IDC_NEWEMAILADDR, EM_SETLIMITTEXT, 
				MAX_EMAIL_LEN, 0);
		return TRUE;

    case WM_HELP: 
		pans = (ADDNAMESTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pans->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		pans = (ADDNAMESTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pans->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDOK: 
			pans = (ADDNAMESTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			if (KMConstructUserID (hDlg, IDC_NEWUSERNAME, IDC_NEWEMAILADDR, 
					&(pans->pszUserID))) {
				EndDialog (hDlg, 1);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Add User to key
//	This routine is called when the user chooses to add
//	a userID to an existing secret key.

BOOL 
KMAddUserToKey (PKEYMAN pKM) 
{
	LPSTR			szPhrase		= NULL;
	BOOL			bRetVal			= TRUE;
	Boolean			bNeedsPhrase;
	UINT			uError;
	PGPKeyRef		Key;
	ADDNAMESTRUCT	ans;
	CHAR			sz[256];
	PGPPrefRef		prefref;

	// get selected key
	Key = (PGPKeyRef)KMFocusedObject (pKM);

	// initialize struct
	ans.pKM = pKM;
	PGPcomdlgOpenClientPrefs (&prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnAddName, 
						&(ans.bSyncWithServer));
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	// get new userid from user
	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWUSERID), 
					pKM->hWndParent, AddUserDlgProc, (LPARAM)&ans)) {

		uError = PGPGetKeyBoolean (Key, 
							kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		if (PGPcomdlgErrorMessage (uError)) return FALSE;

		// get valid passphrase, if required
		if (bNeedsPhrase) uError = kPGPError_BadPassphrase;
		while ((uError == kPGPError_BadPassphrase) && bNeedsPhrase) {
			LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
			uError = PGPcomdlgGetPhrase (pKM->Context, pKM->hWndParent, sz, 
							&szPhrase, NULL, NULL, 0, NULL, NULL, 0);
			if (uError == kPGPError_NoErr) {
				if (!PGPPassphraseIsValid (Key, szPhrase)) {
					PGPcomdlgFreePhrase (szPhrase);
					szPhrase = NULL;
					uError = kPGPError_BadPassphrase;
				}
			}
			PGPcomdlgErrorMessage (uError);
		}

		// now we have a valid passphrase, if required
		if (uError == kPGPError_NoErr) {

			// update from server
			if (ans.bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEADDINGNAME, 	
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
						bRetVal = FALSE;
					}
				}
			}
		
			if (bRetVal) {
				// make sure we have enough entropy
				PGPcomdlgRandom (pKM->Context, pKM->hWndParent, 0);

				if (bNeedsPhrase) {
					uError = PGPAddUserID (Key, ans.pszUserID, 
							PGPOPassphrase (pKM->Context, szPhrase),
							PGPOLastOption (pKM->Context));
				}
				else {
					uError = PGPAddUserID (Key, ans.pszUserID, 
							PGPOLastOption (pKM->Context));
				}
						
				if (!PGPcomdlgErrorMessage (uError)) {
					KMCommitKeyRingChanges (pKM);
					KMUpdateKeyInTree (pKM, Key);
					InvalidateRect (pKM->hWndTree, NULL, TRUE);

					// send to server
					if (ans.bSyncWithServer) {
						pKM->szPutKeyserver[0] = '\0';
						KMSendToServer (pKM);
					}
				}
				else bRetVal = FALSE;
			}
		}
		else bRetVal = FALSE;

		KMFree (ans.pszUserID);
	}
	else bRetVal = FALSE;

	if (szPhrase) {
		PGPcomdlgFreePhrase (szPhrase);
		szPhrase = NULL;
	}

	return bRetVal;
}
