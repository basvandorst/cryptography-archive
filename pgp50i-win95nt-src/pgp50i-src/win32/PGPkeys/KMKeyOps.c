/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMKeyOps.c - implements various operations performed on keys.
//
//	$Id: KMKeyOps.c,v 1.4 1997/05/27 22:07:06 pbj Exp $
//

#include <windows.h>
#include <shlobj.h>
#include "pgpkeys.h"
#include "..\include\pgpphrase.h"
#include "..\include\pgpkeyserversupport.h"
#include "..\pgpkeyserver\winids.h"
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndMain;
extern HWND g_hWndTree;
extern CHAR g_szbuf[G_BUFLEN];
extern CHAR g_szHelpFile[MAX_PATH];
extern PGPKeySet* g_pKeySetMain;

// local globals
static HWND hWndParent;
static BOOL bItemModified;
static BOOL bItemNotDeleted;
static BOOL bDeleteAll;
static LPSTR szPhrase = NULL;
static PGPKey* pDefaultKey;
static PGPKey* pSigningKey;
static HTLITEM hPostDeleteFocusItem;

static DWORD aIds[] = {			// Help IDs
    IDOK,			IDH_PGPKEYS_DELETEKEY,
    IDC_YESTOALL,	IDH_PGPKEYS_DELETEALLKEYS,
    IDNO,			IDH_PGPKEYS_DONTDELETEKEY,
    IDCANCEL,		IDH_PGPKEYS_CANCELDELETE,
    0,0
};


//----------------------------------------------------|
//  Certify a single object
//	routine called either from KMCertifyKeyOrUserID or as a
//	callback function from the TreeList control to
//	certify a single item.
//
//	lptli	= pointer to TreeList item to certify

BOOL CertifySingleObject (TL_TREEITEM* lptli) {
	PGPKey* pKey;	
	PGPUserID* pUserID;
	UINT uError;
	CHAR sz512[512];
	CHAR sz256[256];
	CHAR sz64[64];
	Boolean bNeedsPhrase;

	switch (lptli->iImage) {
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
		pKey = (PGPKey*)(lptli->lParam);
		pUserID = pgpGetPrimaryUserID (pKey);
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		pUserID = (PGPUserID*)(lptli->lParam);
		pKey = KMGetKeyFromUserID (g_hWndTree, pUserID);
		break;

	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		return TRUE;

	default :
		return FALSE;
	}

	uError = pgpCertifyUserID (pUserID, pSigningKey, szPhrase);
	if (uError == PGPERR_KEYDB_BADPASSPHRASE) {
		pgpGetKeyBoolean (pSigningKey,
			kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		if (!bNeedsPhrase) return FALSE;
		while (uError == PGPERR_KEYDB_BADPASSPHRASE) {
			PGPcomdlgErrorMessage (uError);
			if (szPhrase) {
				PGPFreePhrase (szPhrase);
				szPhrase = NULL;
			}
			LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, g_szbuf, G_BUFLEN);
			uError = PGPcomdlgGetPhrase (hWndParent, g_szbuf,
										 &szPhrase, g_pKeySetMain,
										 &pSigningKey, NULL, NULL, 0);
			PGPcomdlgErrorMessage (uError);
			if (uError != PGPCOMDLG_OK) {
				if (szPhrase) {
					PGPFreePhrase (szPhrase);
					szPhrase = NULL;
				}
				return FALSE;
			}
			uError = pgpCertifyUserID (pUserID, pSigningKey, szPhrase);
		}	 	
	}

	if (uError == PGPERR_OK) {
		bItemModified = TRUE;
		return TRUE;
	}

	LoadString (g_hInst, IDS_CERTIFYERROR, sz512, 512);
	PGPcomdlgErrorToString (uError, sz256, 256);
	wsprintf (g_szbuf, sz512, lptli->pszText, sz256);
	LoadString (g_hInst, IDS_CAPTION, sz64, 64);
	if (KMMultipleSelected ()) {
		if (MessageBox (g_hWndMain, g_szbuf, sz64,
						MB_OKCANCEL|MB_ICONEXCLAMATION) == IDOK)
			return TRUE;
	}
	else {
		if (MessageBox (g_hWndMain, g_szbuf, sz64,
						MB_OK|MB_ICONEXCLAMATION) == IDOK)
			return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------|
//  Certify selected key or userid

BOOL KMCertifyKeyOrUserID (HWND hWnd) {
	TL_TREEITEM tli;
	CHAR sz256[256];
	UINT uError;
	BOOL bSendToServer;
 	BOOL bUpdatedKeys;
	LPSTR szSelectedKeyIDs;
	PGPKeySet* pKeySetSelected;

	hWndParent = hWnd;
	bItemModified = FALSE;
	pSigningKey = NULL;
	
	// more than one item selected
	if (KMMultipleSelected ()) {
		if (!KMSignConfirm (hWnd, &bSendToServer)) return FALSE;
		LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, g_szbuf, G_BUFLEN);
		uError = PGPcomdlgGetPhrase (hWnd, g_szbuf, &szPhrase, g_pKeySetMain,
											&pSigningKey, NULL, NULL, 0);
		PGPcomdlgErrorMessage (uError);
		if (uError == PGPCOMDLG_OK) {
			TreeList_IterateSelected (g_hWndTree, CertifySingleObject);
		}
	}

	// only one item selected
	else {
		if (!KMSignConfirm (hWnd, &bSendToServer)) return FALSE;
		LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE, g_szbuf, G_BUFLEN);
		uError = PGPcomdlgGetPhrase (hWnd, g_szbuf, &szPhrase, g_pKeySetMain,
											&pSigningKey, NULL, NULL, 0);
		PGPcomdlgErrorMessage (uError);
		if (uError == PGPCOMDLG_OK) {
			tli.hItem = KMFocusedItem ();
			tli.pszText = sz256;
			tli.cchTextMax = sizeof(sz256);
			tli.mask = TLIF_IMAGE | TLIF_PARAM | TLIF_TEXT;
			if (TreeList_GetItem (g_hWndTree, &tli)) {	
				CertifySingleObject (&tli);
			}
		}
	}

	// changes have been made; save them and update all validities
	if (bItemModified) {
		KMCommitKeyRingChanges (g_pKeySetMain);
		KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, FALSE);
		InvalidateRect (g_hWndTree, NULL, TRUE);
		if (bSendToServer) {
			KMGetSelectedObjects (&pKeySetSelected, &szSelectedKeyIDs);
 			ProcessKeyServerMessageKR (hWnd, ID_SEND_KEYS_TO_SERVER,
									   szSelectedKeyIDs,
									   &bUpdatedKeys, pKeySetSelected,
									   g_pKeySetMain);
			KMFreeSelectedObjects (pKeySetSelected, szSelectedKeyIDs);
 			if (bUpdatedKeys) {
				KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, TRUE);
				InvalidateRect (g_hWndTree, NULL, TRUE);
			}
		}
	}

	if (szPhrase) {
		PGPFreePhrase (szPhrase);
		szPhrase = NULL;
	}
	return (bItemModified);
}


//----------------------------------------------------|
//  Enable selected key

BOOL KMEnableKey (HWND hWnd, PGPKey* pKey) {
	TL_TREEITEM tli;

	if (!PGPcomdlgErrorMessage (pgpEnableKey (pKey))) {
		pgpGetKeyUserVal (pKey, (long*)&tli.hItem);
		tli.iImage = KMDetermineKeyIcon (pKey, NULL);
		tli.state = 0;
		tli.stateMask = TLIS_ITALICS;
		tli.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_SetItem (g_hWndTree, &tli);
		KMSetFocus (tli.hItem, FALSE);
		KMCommitKeyRingChanges (g_pKeySetMain);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Disable selected key

BOOL KMDisableKey (HWND hWnd, PGPKey* pKey) {
	TL_TREEITEM tli;

	if (!PGPcomdlgErrorMessage (pgpDisableKey (pKey))) {
		pgpGetKeyUserVal (pKey, (long*)&tli.hItem);
		tli.iImage = KMDetermineKeyIcon (pKey, NULL);
		tli.state = TLIS_ITALICS;
		tli.stateMask = TLIS_ITALICS;
		tli.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_SetItem (g_hWndTree, &tli);
		KMSetFocus (tli.hItem, FALSE);
		KMCommitKeyRingChanges (g_pKeySetMain);
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Revoke selected key

BOOL KMRevokeKey (HWND hWnd) {
	Boolean bNeedsPhrase;
	BOOL bPhraseOK;
	LPSTR szRevPhrase = NULL;
	UINT uError;
	PGPKey* pKey;
	PGPKey* pDefKey;

	pKey = (PGPKey*) KMFocusedObject ();
	pDefKey = pgpGetDefaultPrivateKey (g_pKeySetMain);

	if (pKey == pDefKey) {
		if (KMMessageBox (hWndParent, IDS_CAPTION, IDS_REVCONFDEFKEY,
			MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)==IDNO)
			return FALSE;
	}
	else if (KMMessageBox (hWnd, IDS_CAPTION, IDS_REVOKECONFIRM,
						MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
			return FALSE;

	pgpGetKeyBoolean ((PGPKey*) KMFocusedObject (),
						kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
	do {
		if (bNeedsPhrase) {
			LoadString (g_hInst, IDS_SELKEYPASSPHRASE, g_szbuf, G_BUFLEN);
			uError = PGPcomdlgGetPhrase (hWnd, g_szbuf, &szRevPhrase,
										 NULL, NULL, NULL, NULL, 0);
			PGPcomdlgErrorMessage (uError);
			bPhraseOK = (uError == PGPCOMDLG_OK);
		}
		else {
			szRevPhrase = NULL;
			bPhraseOK = TRUE;
		}
		if (bPhraseOK) {
			uError = pgpRevokeKey ((PGPKey*) KMFocusedObject (), szRevPhrase);
			if (!PGPcomdlgErrorMessage (uError)) {
				if (szRevPhrase) {
					PGPFreePhrase (szRevPhrase);
					szRevPhrase = NULL;
				}
				KMCommitKeyRingChanges (g_pKeySetMain);
				KMUpdateKeyInTree (g_hWndTree, (PGPKey*) KMFocusedObject ());
				KMUpdateAllValidities ();
				InvalidateRect (g_hWndTree, NULL, TRUE);
				return TRUE;
			}
			if (szRevPhrase) {
				PGPFreePhrase (szRevPhrase);
				szRevPhrase = NULL;
			}
		}
	} while (bPhraseOK && (uError == PGPERR_KEYDB_BADPASSPHRASE));

	return FALSE;
}


//----------------------------------------------------|
//  Revoke selected Certification

BOOL KMRevokeCert (HWND hWnd) {
	TL_TREEITEM tli;
	PGPCert* pCert;
	PGPKey* pKey;
	Boolean bNeedsPhrase;
	BOOL bPhraseOK;
	LPSTR szRevPhrase = NULL;
	UINT uError;

	pCert = (PGPCert*) KMFocusedObject ();

	if (!PGPcomdlgErrorMessage (pgpGetCertifier (
									pCert, g_pKeySetMain, &pKey))) {
		if (!pKey) {
			KMMessageBox (hWnd, IDS_CAPTION, IDS_CERTKEYNOTONRING,
							MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
		pgpGetKeyBoolean (pKey, kPGPKeyPropNeedsPassphrase, &bNeedsPhrase);
		do {
			if (bNeedsPhrase) {
				LoadString (g_hInst, IDS_SIGNKEYPASSPHRASE,
									g_szbuf, G_BUFLEN);
				uError = PGPcomdlgGetPhrase (hWnd, g_szbuf, &szRevPhrase,
											 NULL, NULL, NULL, NULL, 0);
				PGPcomdlgErrorMessage (uError);
				bPhraseOK = (uError == PGPCOMDLG_OK);
			}
			else {
				szRevPhrase = NULL;
				bPhraseOK = TRUE;
			}

			if (bPhraseOK) {
				uError = pgpRevokeCert ((PGPCert*) KMFocusedObject (),
						g_pKeySetMain, szRevPhrase);
				if (!PGPcomdlgErrorMessage (uError)) {
					if (szRevPhrase) {
						PGPFreePhrase (szRevPhrase);
						szRevPhrase = NULL;
					}
					tli.hItem = KMFocusedItem ();
					tli.iImage = IDX_REVCERT;
					tli.state = TLIS_ITALICS;
					tli.stateMask = TLIS_ITALICS;
					tli.mask = TLIF_IMAGE | TLIF_STATE;
					TreeList_SetItem (g_hWndTree, &tli);
					KMSetFocus (tli.hItem, FALSE);
					KMCommitKeyRingChanges (g_pKeySetMain);
					KMUpdateAllValidities ();
					return TRUE;
				}
			}
			if (szRevPhrase) {
				PGPFreePhrase (szRevPhrase);
				szRevPhrase = NULL;
			}

		} while (bPhraseOK && (uError == PGPERR_KEYDB_BADPASSPHRASE));
	}

	return FALSE;
}


//----------------------------------------------------|
// Delete All dialog message procedure

BOOL CALLBACK DeleteAllDlgProc (HWND hWndDlg, UINT uMsg,
								WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_INITDIALOG :
		SetDlgItemText (hWndDlg, IDC_STRING, g_szbuf);
		break;

    case WM_HELP:
         WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
             HELP_WM_HELP, (DWORD) (LPSTR) aIds);
         break;

    case WM_CONTEXTMENU:
         WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
             (DWORD) (LPVOID) aIds);
         break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			bItemNotDeleted = TRUE;
			EndDialog (hWndDlg, IDCANCEL);
			break;

		case IDOK:
		case IDYES:
			EndDialog (hWndDlg, IDYES);
			break;

		case IDNO:
			EndDialog (hWndDlg, IDNO);
			break;

		case IDC_YESTOALL :
			bDeleteAll = TRUE;
			EndDialog (hWndDlg, IDYES);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Ask user for delete confirmation

int DeleteConfirm (TL_TREEITEM* lptli, INT iPromptID) {
	CHAR sz256[256];
	CHAR sz512[512];
	INT iRetVal;

	if (bDeleteAll) return IDYES;

	LoadString (g_hInst, iPromptID, sz512, 512);
	wsprintf (g_szbuf, sz512, lptli->pszText);

	if (KMMultipleSelected ()) {
		iRetVal = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_DELETEALL),
			hWndParent, DeleteAllDlgProc);
		if (!bItemNotDeleted && (iRetVal == IDNO)) {
			bItemNotDeleted = TRUE;
			hPostDeleteFocusItem = lptli->hItem;
		}
	}
	else {
		LoadString (g_hInst, IDS_DELCONFCAPTION, sz256, 256);
		return (MessageBox (hWndParent, g_szbuf, sz256,
				MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING));
	}
}


//----------------------------------------------------|
//  Get handle of nearby item
//
//	lptli	= pointer to TreeList item

HTLITEM GetAdjacentItem (HWND hWndTree, TL_TREEITEM* lptli) {
	TL_TREEITEM tli;

	tli.hItem = lptli->hItem;
	tli.mask = TLIF_NEXTHANDLE;
	TreeList_GetItem (hWndTree, &tli);
	if (!tli.hItem) {
		tli.hItem = lptli->hItem;
		tli.mask = TLIF_PREVHANDLE;
		TreeList_GetItem (hWndTree, &tli);
		if (!tli.hItem) {
			tli.hItem = lptli->hItem;
			tli.mask = TLIF_PARENTHANDLE;
			TreeList_GetItem (hWndTree, &tli);
		}
	}

	return tli.hItem;
}

	
//----------------------------------------------------|
//  Delete a single object
//	routine called either from KMDeleteObject or as a
//	callback function from the TreeList control to
//	delete a single item.
//
//	lptli	= pointer to TreeList item to delete

BOOL DeleteSingleObject (TL_TREEITEM* lptli) {
	PGPKeySet* pKeySet;
	PGPKey* pKey;
	BOOL bPrimary;
	INT iError, iConfirm;

	switch (lptli->iImage) {
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFPRIVKEY);
		if (iConfirm == IDYES) {
			if ((PGPKey*)(lptli->lParam) == pDefaultKey) {
				if (KMMessageBox (hWndParent, IDS_CAPTION, IDS_DELCONFDEFKEY,
					MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING)==IDNO)
					return TRUE;
			}
			pKeySet = pgpNewSingletonKeySet ((PGPKey*)(lptli->lParam));
			if (!PGPcomdlgErrorMessage (pgpRemoveKeys (
					g_pKeySetMain, pKeySet))) {
				KMDeletePropertiesKey ((PGPKey*)(lptli->lParam));
				bItemModified = TRUE;
				if (!bItemNotDeleted)
					hPostDeleteFocusItem =
							GetAdjacentItem (g_hWndTree, lptli);
				TreeList_DeleteItem (g_hWndTree, lptli);
			}
			pgpFreeKeySet (pKeySet);
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFKEY);
		if (iConfirm == IDYES) {
			pKeySet = pgpNewSingletonKeySet ((PGPKey*)(lptli->lParam));
			if (!PGPcomdlgErrorMessage (pgpRemoveKeys (
											g_pKeySetMain, pKeySet))) {
				KMDeletePropertiesKey ((PGPKey*)(lptli->lParam));
				bItemModified = TRUE;
				if (!bItemNotDeleted)
					hPostDeleteFocusItem =
							GetAdjacentItem (g_hWndTree, lptli);
				TreeList_DeleteItem (g_hWndTree, lptli);
			}
			pgpFreeKeySet (pKeySet);
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFUSERID);
		if (iConfirm == IDYES) {
			pKey = KMGetKeyFromUserID (g_hWndTree,
											(PGPUserID*)(lptli->lParam));

			if (pgpGetPrimaryUserID (pKey) == (PGPUserID*)(lptli->lParam))
				bPrimary = TRUE;
			else bPrimary = FALSE;

			iError = pgpRemoveUserID ((PGPUserID*)(lptli->lParam));
			if (iError == PGPERR_BADPARAM) {
				KMMessageBox (hWndParent, IDS_CAPTION, IDS_DELONLYUSERID,
					MB_OK|MB_ICONEXCLAMATION);
			}
			else {
				if (!PGPcomdlgErrorMessage (iError)) {
					bItemModified = TRUE;
					if (bPrimary) KMUpdateKeyInTree (g_hWndTree, pKey);
					else {
						if (!bItemNotDeleted)
							hPostDeleteFocusItem =
									GetAdjacentItem (g_hWndTree, lptli);
						TreeList_DeleteItem (g_hWndTree, lptli);
					}
				}
			}
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;

	case IDX_CERT :
	case IDX_REVCERT :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFCERT);
		if (iConfirm == IDYES) {
			if (!PGPcomdlgErrorMessage (pgpRemoveCert (
											(PGPCert*)(lptli->lParam)))) {
				bItemModified = TRUE;
				if (!bItemNotDeleted)
					hPostDeleteFocusItem =
							GetAdjacentItem (g_hWndTree, lptli);
				TreeList_DeleteItem (g_hWndTree, lptli);
			}
		}
		if (iConfirm == IDCANCEL) return FALSE;
		else return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------|
//  Delete selected key or keys

BOOL KMDeleteObject (HWND hWnd) {
	TL_TREEITEM tli;
	CHAR sz256[256];

	hWndParent = hWnd;
	bItemModified = FALSE;
	bDeleteAll = FALSE;
	bItemNotDeleted = FALSE;
	hPostDeleteFocusItem = NULL;

	pDefaultKey = pgpGetDefaultPrivateKey (g_pKeySetMain);

	if (KMMultipleSelected ()) {
		TreeList_IterateSelected (g_hWndTree, DeleteSingleObject);
	}
	else {
		tli.hItem = KMFocusedItem ();
		tli.pszText = sz256;
		tli.cchTextMax = 256;
		tli.mask = TLIF_IMAGE | TLIF_TEXT | TLIF_PARAM;
		if (!TreeList_GetItem (g_hWndTree, &tli)) return FALSE;	
		DeleteSingleObject (&tli);
	}
	if (bItemModified) {
		KMCommitKeyRingChanges (g_pKeySetMain);
		KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, FALSE);
		if (bItemNotDeleted) {
			if (hPostDeleteFocusItem) {
				tli.hItem = hPostDeleteFocusItem;
				TreeList_Select (g_hWndTree, &tli, FALSE);
			}
		}
		else {
			if (hPostDeleteFocusItem) {
				tli.hItem = hPostDeleteFocusItem;
				TreeList_Select (g_hWndTree, &tli, TRUE);
				tli.stateMask = TLIS_SELECTED;
				tli.state = 0;
				tli.mask = TLIF_STATE;
				TreeList_SetItem (g_hWndTree, &tli);
			}
		}
		InvalidateRect (g_hWndTree, NULL, TRUE);
	}
	return (bItemModified);
}


//----------------------------------------------------|
// Set focused key to be default signing key

BOOL KMSetDefaultKey (HWND hwnd) {
	PGPKey* pKey;
	PGPKey* pDefKey;
	TL_TREEITEM tli;

	pDefKey = pgpGetDefaultPrivateKey (g_pKeySetMain);
	pKey = (PGPKey*) KMFocusedObject ();

	if (!PGPcomdlgErrorMessage (pgpSetDefaultPrivateKey (pKey))) {
		// set old default to non-bold
		if (pDefKey) {
			pgpGetKeyUserVal (pDefKey, (long*)&(tli.hItem));
			tli.state = 0;
			tli.stateMask = TLIS_BOLD;
			tli.mask = TLIF_STATE;
			TreeList_SetItem (g_hWndTree, &tli);
		}
		// set new default to bold
		tli.hItem = KMFocusedItem ();
		tli.state = TLIS_BOLD;
		tli.stateMask = TLIS_BOLD;
		tli.mask = TLIF_STATE;
		TreeList_SetItem (g_hWndTree, &tli);
		PGPcomdlgErrorMessage (pgpSavePrefs ());
		KMCommitKeyRingChanges (g_pKeySetMain);
		KMUpdateAllValidities ();
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Set focused UID to be primary UID

BOOL KMSetPrimaryUserID (HWND hwnd) {
	PGPUserID* pUserID;
	PGPKey* pKey;

	pUserID = (PGPUserID*) KMFocusedObject ();

	if (!PGPcomdlgErrorMessage (pgpSetPrimaryUserID (pUserID))) {
		KMCommitKeyRingChanges (g_pKeySetMain);
		pKey = KMGetKeyFromUserID (g_hWndTree, pUserID);
		KMUpdateKeyInTree (g_hWndTree, pKey);
		InvalidateRect (g_hWndTree, NULL, TRUE);
		return TRUE;
	}
	return FALSE;
}
