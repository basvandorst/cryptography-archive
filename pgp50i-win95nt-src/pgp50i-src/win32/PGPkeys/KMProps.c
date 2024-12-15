/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMProps.c - handle Key properties dialogs
//
//	$Id: KMProps.c,v 1.2 1997/05/09 19:29:20 pbj Exp $
//

#include <windows.h>
#include <process.h>
#include <commctrl.h>
#include "resource.h"
#include "pgpkeys.h"
#include "..\include\pgpphrase.h"
#include "..\include\treelist.h"

#define MAXSHEETS	16		// maximum number of simultaneous dialogs

typedef struct {
	WNDPROC wpOrigFingerProc;
	PGPKey* pKey;
	INT iIndex;
	DWORD retval;
	UINT uOrigTrust;
	UINT uOrigValidity;
	UINT uPrevTrust;
	UINT uPrevTrustMessage;
	Boolean bOrigDisabled;
	Boolean bSecret;
	Boolean bOrigAxiomatic;
	UINT uNumberMRKs;
} KMPROPSHEETSTRUCT;

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndTree;
extern PGPKeySet* g_pKeySetMain;
extern CHAR g_szHelpFile[MAX_PATH];
extern CHAR g_szbuf[G_BUFLEN];
extern BOOL g_bReadOnly;

// local globals
static HWND hWndParent;
static INT iNumberSheets = 0;
static LPSTR szPhrase = NULL;
static HWND hWndTable[MAXSHEETS];
static PGPKey* pKeyTable[MAXSHEETS];

static DWORD aIds[] = {			// Help IDs
	IDC_KEYID,			IDH_PGPKEYS_KEYID,
	IDC_KEYTYPE,		IDH_PGPKEYS_KEYTYPE,
	IDC_CREATEDATE,		IDH_PGPKEYS_CREATEDATE,
	IDC_EXPIREDATE,		IDH_PGPKEYS_KEYEXPIRES,
	IDC_VALIDITY,		IDH_PGPKEYS_VALIDITY,
	IDC_VALIDITYBAR,	IDH_PGPKEYS_VALIDITYBAR,
	IDC_TRUST,			IDH_PGPKEYS_TRUST,
	IDC_TRUSTSLIDER,	IDH_PGPKEYS_TRUSTSLIDER,
	IDC_FINGERPRINT,	IDH_PGPKEYS_FINGERPRINT,
	IDC_ENABLED,		IDH_PGPKEYS_ENABLED,
	IDC_CHANGEPHRASE,	IDH_PGPKEYS_CHANGEPHRASE,
	IDC_AXIOMATIC,		IDH_PGPKEYS_AXIOMATIC,
	IDC_MRKAGENT,		IDH_PGPKEYS_MRKAGENT,
    0,0
};


//----------------------------------------------------|
//  Fingerprint edit box subclass procedure

LRESULT APIENTRY FingerSubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {
	KMPROPSHEETSTRUCT* pkmpss;
	pkmpss =
		(KMPROPSHEETSTRUCT*)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

    switch (uMsg) {
	case WM_LBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
		SendMessage (hWnd, EM_SETSEL, 0, -1);
		SetFocus (hWnd);
		return 0;
	}
    return CallWindowProc (pkmpss->wpOrigFingerProc, hWnd, uMsg,
								wParam, lParam);
}


//----------------------------------------------------|
//  update controls dealing with validity

void SetValidityControls (HWND hDlg, UINT uVal) {
	INT i;
	UINT u;

	switch (uVal) {
		case PGP_VALIDITY_UNKNOWN : i = IDS_VALIDITY0; break;
		case PGP_VALIDITY_INVALID : i = IDS_VALIDITY1; break;
		case PGP_VALIDITY_MARGINAL : i = IDS_VALIDITY2; break;
		case PGP_VALIDITY_COMPLETE : i = IDS_VALIDITY3; break;
	}
	LoadString (g_hInst, i, g_szbuf, G_BUFLEN);
	SetDlgItemText (hDlg, IDC_VALIDITY, g_szbuf);
	u = KMConvertFromPGPValidity (uVal);
	SendDlgItemMessage (hDlg, IDC_VALIDITYBAR, PBM_SETPOS, u, 0);
}


//----------------------------------------------------|
//  update controls dealing with trust

void SetTrustControls (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss, UINT uTrust) {
	INT i;
	UINT u;

	pkmpss->uPrevTrust = uTrust;
	pkmpss->uPrevTrustMessage = uTrust;
	u = KMConvertFromPGPTrust (uTrust);
	SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETPOS, (WPARAM)TRUE, u);
	switch (uTrust) {
		case PGP_KEYTRUST_UNDEFINED	:
		case PGP_KEYTRUST_UNKNOWN :
		case PGP_KEYTRUST_NEVER :      i = IDS_TRUST2; break;
		case PGP_KEYTRUST_MARGINAL :  i = IDS_TRUST3; break;
		case PGP_KEYTRUST_COMPLETE :  i = IDS_TRUST4; break;
		case PGP_KEYTRUST_ULTIMATE :  i = IDS_TRUST5; break;
	}
	LoadString (g_hInst, i, g_szbuf, G_BUFLEN);
	SetDlgItemText (hDlg, IDC_TRUST, g_szbuf);
}


//----------------------------------------------------|
//  Key Properties Dialog Message procedure

BOOL CALLBACK KeyPropDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	KMPROPSHEETSTRUCT* pkmpss;
	CHAR szbuf[128];
	INT i, iError;
	UINT u, uAlgorithm, uTrust;
	Boolean bB, bAxiomatic, bRevoked, bExpired;
	PGPTime tm;
	HWND hParent;
	RECT rc;
	PGPKey* pKey;
	CHAR* psz;
	CHAR* pszOld;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// save HWND to table
		hWndTable[pkmpss->iIndex] = hDlg;

		// center dialog on screen
		if (iNumberSheets == 1) {
			hParent = GetParent (hDlg);
			GetWindowRect (hParent, &rc);
			SetWindowPos (hParent, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		// subclass fingerprint edit control to prevent partial selection
		pkmpss->wpOrigFingerProc =
			(WNDPROC) SetWindowLong(GetDlgItem (hDlg, IDC_FINGERPRINT),
                 GWL_WNDPROC, (LONG) FingerSubclassProc);

		// initialize all controls
		PostMessage (hDlg, WM_USER, 0, 0);
		return TRUE;

	case WM_USER :
		pgpGetKeyBoolean (pkmpss->pKey,kPGPKeyPropIsSecret,&pkmpss->bSecret);
		pgpGetKeyBoolean (pkmpss->pKey,kPGPKeyPropIsRevoked,&bRevoked);
		pgpGetKeyBoolean (pkmpss->pKey,kPGPKeyPropIsExpired,&bExpired);

		// initialize key id edit control
		u = sizeof (szbuf);
		pgpGetKeyString (pkmpss->pKey, kPGPKeyPropKeyId, szbuf, &u);
		KMConvertStringKeyID (szbuf);
		SetDlgItemText (hDlg, IDC_KEYID, szbuf);

		// initialize key type edit control
		pgpGetKeyNumber (pkmpss->pKey, kPGPKeyPropAlgId, &uAlgorithm);
		switch (uAlgorithm) {
		case PGP_PKALG_RSA :
			LoadString (g_hInst, IDS_RSA, szbuf, sizeof (szbuf));
			break;

		case PGP_PKALG_DSA :
			LoadString (g_hInst, IDS_DSA_ELGAMAL, szbuf, sizeof (szbuf));
			break;

		default :
			LoadString (g_hInst, IDS_UNKNOWN, szbuf, sizeof (szbuf));
			break;
		}
		SetDlgItemText (hDlg, IDC_KEYTYPE, szbuf);

		// initialize key creation date edit control
		pgpGetKeyTime (pkmpss->pKey, kPGPKeyPropCreation, &tm);
		KMConvertTimeToString (&tm, szbuf, sizeof (szbuf));
		SetDlgItemText (hDlg, IDC_CREATEDATE, szbuf);

		// initialize key expiration date edit control
		pgpGetKeyTime (pkmpss->pKey, kPGPKeyPropExpiration, &tm);
		if (tm) KMConvertTimeToString (&tm, szbuf, sizeof (szbuf));
		else LoadString (g_hInst, IDS_NEVER, szbuf, sizeof (szbuf));
		SetDlgItemText (hDlg, IDC_EXPIREDATE, szbuf);

		// initialize fingerprint edit control
		u = sizeof (szbuf);
		pgpGetKeyString (pkmpss->pKey, kPGPKeyPropFingerprint, szbuf, &u);
		KMConvertStringFingerprint (uAlgorithm, szbuf);
		SetDlgItemText (hDlg, IDC_FINGERPRINT, szbuf);

		// initialize validity edit and bar controls
		i = KMConvertFromPGPValidity (PGP_VALIDITY_COMPLETE);
		SendDlgItemMessage (hDlg, IDC_VALIDITYBAR, PBM_SETRANGE,
									0, MAKELPARAM (0,i));
		pgpGetKeyNumber (pkmpss->pKey, kPGPKeyPropValidity,
								&pkmpss->uOrigValidity);
		SetValidityControls (hDlg, pkmpss->uOrigValidity);

		// initialize "Axiomatic" checkbox control
		pgpGetKeyBoolean (pkmpss->pKey, kPGPKeyPropIsAxiomatic,
								&pkmpss->bOrigAxiomatic);
		if (pkmpss->bOrigAxiomatic) CheckDlgButton (hDlg, IDC_AXIOMATIC,
														BST_CHECKED);
		else CheckDlgButton (hDlg, IDC_AXIOMATIC, BST_UNCHECKED);
		if (bRevoked || bExpired || g_bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), FALSE);
		if (!pkmpss->bSecret && !pkmpss->bOrigAxiomatic)
			ShowWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), SW_HIDE);

		// initialize trust edit and slider controls
		i = KMConvertFromPGPTrust (PGP_KEYTRUST_COMPLETE);
		SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETRANGE,
									0, MAKELPARAM (0,i));
		pgpGetKeyNumber (pkmpss->pKey, kPGPKeyPropTrust, &pkmpss->uOrigTrust);
		SetTrustControls (hDlg, pkmpss, pkmpss->uOrigTrust);
		if (pkmpss->bOrigAxiomatic || bRevoked || bExpired || g_bReadOnly) {
			EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
		}

		// initialize enable/disable checkbox
		pgpGetKeyBoolean (pkmpss->pKey, kPGPKeyPropIsDisabled,
								&pkmpss->bOrigDisabled);
		if (pkmpss->bOrigDisabled) CheckDlgButton (hDlg, IDC_ENABLED,
														BST_UNCHECKED);
		else CheckDlgButton (hDlg, IDC_ENABLED, BST_CHECKED);
		if (bRevoked || bExpired || g_bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
		if (pkmpss->bSecret) {
			pgpGetKeyBoolean (pkmpss->pKey, kPGPKeyPropIsAxiomatic, &bB);
			if (bB) EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
		}

		// initialize change passphrase button
		if (!pkmpss->bSecret)
			ShowWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), SW_HIDE);
		if (g_bReadOnly)
			EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);

		// initialize MRK edit box
		if (pkmpss->uNumberMRKs > 0) {
			psz = KMAlloc (4);
			psz[0] = '\0';
			for (u=0; u<pkmpss->uNumberMRKs; u++) {
				iError = pgpGetMessageRecoveryKey (pkmpss->pKey,
								g_pKeySetMain, u, &pKey, NULL, NULL);
				if (!pKey) iError = PGPERR_GENERIC;
				switch (iError) {
				case PGPERR_OK :
					KMGetKeyName (pKey, szbuf, sizeof(szbuf));
					break;

				case PGPERR_NO_RECOVERYKEY :
					i = sizeof (szbuf);
					pgpGetKeyString (pkmpss->pKey, kPGPKeyPropFingerprint,
												szbuf, &i);
					szbuf[i] = '\0';
					pgpGetKeyNumber (pKey, kPGPKeyPropAlgId, &uAlgorithm);
					KMConvertStringFingerprint (uAlgorithm, szbuf);
					break;

				default :
					LoadString (g_hInst, IDS_ERRONEOUSMRK,
									szbuf, sizeof(szbuf));
					break;
				}

				pszOld = psz;
				psz = KMAlloc (lstrlen(pszOld)+lstrlen(szbuf)+4);
				if (!psz) {
					psz = pszOld;
					break;
				}
				lstrcpy (psz, pszOld);
				if (lstrlen (psz) > 0) lstrcat (psz, "\r\n");
				lstrcat (psz, szbuf);
				KMFree (pszOld);
			}

			SetDlgItemText (hDlg, IDC_MRKAGENT, psz);
			KMFree (psz);

		}
		break;

	case WM_USER+1 :
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);
		EnableWindow (GetDlgItem (GetParent(hDlg), IDOK), FALSE);
		break;

	case WM_HSCROLL :
		if ((LOWORD (wParam) == TB_THUMBTRACK) ||
			(LOWORD (wParam) == TB_THUMBPOSITION) ||
			(LOWORD (wParam) == TB_ENDTRACK)) {
			u = SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_GETPOS, 0, 0);
			uTrust = KMConvertToPGPTrust (u);
			pkmpss->uPrevTrust = uTrust;
			switch (uTrust) {
				case PGP_KEYTRUST_UNDEFINED	:
				case PGP_KEYTRUST_UNKNOWN :
				case PGP_KEYTRUST_NEVER :     i = IDS_TRUST2; break;
				case PGP_KEYTRUST_MARGINAL :  i = IDS_TRUST3; break;
				case PGP_KEYTRUST_COMPLETE :  i = IDS_TRUST4; break;
				case PGP_KEYTRUST_ULTIMATE :  i = IDS_TRUST5; break;
			}
			LoadString (g_hInst, i, szbuf, sizeof (szbuf));
			SetDlgItemText (hDlg, IDC_TRUST, szbuf);
			if (LOWORD (wParam) == TB_ENDTRACK) {
				if ((pkmpss->uOrigValidity <= PGP_VALIDITY_INVALID) &&
					(uTrust >= PGP_KEYTRUST_MARGINAL)) {
					if (uTrust != pkmpss->uPrevTrustMessage) {
						pkmpss->uPrevTrustMessage = uTrust;
						SetTrustControls (hDlg, pkmpss, PGP_KEYTRUST_NEVER);
						KMMessageBox (hDlg, IDS_CAPTION,
							IDS_TRUSTONINVALIDKEY, MB_OK|MB_ICONEXCLAMATION);
					}
				}
			}
		}
		break;

	case WM_DESTROY:
		SetWindowLong(GetDlgItem(hDlg, IDC_FINGERPRINT),
							GWL_WNDPROC, (LONG)pkmpss->wpOrigFingerProc);
		break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
			case IDC_CHANGEPHRASE :
				KMChangePhrase (hDlg, pkmpss->pKey);
				break;

			case IDC_AXIOMATIC :
				if (IsDlgButtonChecked (hDlg, IDC_AXIOMATIC) ==
						BST_UNCHECKED) {
					if (!pkmpss->bSecret) {
						EnableWindow (GetDlgItem (hDlg,IDC_AXIOMATIC), FALSE);
					}
					bAxiomatic = FALSE;
					EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), TRUE);
					SetValidityControls (hDlg, PGP_VALIDITY_UNKNOWN);
					SetTrustControls (hDlg, pkmpss, PGP_KEYTRUST_UNKNOWN);
				}
				else {
					bAxiomatic = TRUE;
					CheckDlgButton (hDlg, IDC_ENABLED, BST_CHECKED);
					EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
					SetValidityControls (hDlg, PGP_VALIDITY_COMPLETE);
					SetTrustControls (hDlg, pkmpss, PGP_KEYTRUST_ULTIMATE);
				}
				if (bAxiomatic ||
				     (KMConvertFromPGPValidity (pkmpss->uOrigValidity) == 0))
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
				else
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), TRUE);
				break;
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

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_APPLY :
			pkmpss->retval = 0;

			// update axiomatic state
			if (IsDlgButtonChecked (hDlg, IDC_AXIOMATIC) == BST_CHECKED) {
				if (!pkmpss->bOrigAxiomatic) {
					if (pkmpss->bOrigDisabled)
						KMEnableKey (hDlg, pkmpss->pKey);
					PGPcomdlgErrorMessage (pgpSetKeyAxiomatic (pkmpss->pKey,
													FALSE, ""));
					pkmpss->retval = 1;
				}
			}
			else {
				if (pkmpss->bOrigAxiomatic) {
					PGPcomdlgErrorMessage (
						pgpUnsetKeyAxiomatic (pkmpss->pKey));
					pkmpss->retval = 1;
				}

				// update trust level
				u = SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER,
											TBM_GETPOS, 0, 0);
				u = KMConvertToPGPTrust (u);
				if (u != pkmpss->uOrigTrust) {
					PGPcomdlgErrorMessage (pgpSetKeyTrust (pkmpss->pKey,
													(unsigned char)u));
					pkmpss->retval = 1;
				}
			}

			// update enable/disable state
			if (IsDlgButtonChecked (hDlg, IDC_ENABLED) == BST_CHECKED) {
				if (pkmpss->bOrigDisabled) {
					KMEnableKey (hDlg, pkmpss->pKey);
					pkmpss->retval = 1;
				}
			}
			else {
				if (!pkmpss->bOrigDisabled) {
					KMDisableKey (hDlg, pkmpss->pKey);
					pkmpss->retval = 1;
				}
			}

			// set return value
			SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return PSNRET_NOERROR;

		case PSN_HELP :
			WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT,
				IDH_PGPKEYS_PROPDIALOG);
			break;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			pkmpss->retval = 0;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;

		}
	}
	return FALSE;
}


//----------------------------------------------------|
//  Post Key Properties Dialog

DWORD WINAPI SingleKeyThread (PGPKey* pKey) {
	CHAR szTitle0[32];
	CHAR szUserName[256];
    PROPSHEETPAGE psp[1];
    PROPSHEETHEADER psh;
	KMPROPSHEETSTRUCT kmpss;
	INT i;

	for (i=0; i<MAXSHEETS; i++) {
		if (!pKeyTable[i]) {
			if (!hWndTable[i]) {
				pKeyTable[i] = pKey;
				kmpss.iIndex = i;
				break;
			}
		}
	}

	kmpss.pKey = pKey;
	kmpss.retval = 0;

	pgpGetMessageRecoveryKey (pKey, g_pKeySetMain, 0, NULL, NULL,
									&kmpss.uNumberMRKs);

	LoadString (g_hInst, IDS_PROPTITLE0, szTitle0, sizeof(szTitle0));
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance = g_hInst;
	if (kmpss.uNumberMRKs > 0)
		psp[0].pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPMRK);
    else
		psp[0].pszTemplate = MAKEINTRESOURCE (IDD_KEYPROP);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = KeyPropDlgProc;
    psp[0].pszTitle = szTitle0;
    psp[0].lParam = (LPARAM)&kmpss;
    psp[0].pfnCallback = NULL;

	KMGetKeyName (pKey, szUserName, sizeof(szUserName));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = NULL;
    psh.hInstance = g_hInst;
    psh.pszIcon = NULL;
    psh.pszCaption = (LPSTR) szUserName;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

	pKeyTable[kmpss.iIndex] = NULL;
	hWndTable[kmpss.iIndex] = NULL;
	iNumberSheets--;

	if (kmpss.retval) {
		KMCommitKeyRingChanges (g_pKeySetMain);
		KMUpdateKeyInTree (g_hWndTree, pKey);
		KMUpdateAllValidities ();
		InvalidateRect (g_hWndTree, NULL, TRUE);
		return TRUE;
	}
	return 0;
}


//----------------------------------------------------|
//  Post Key Properties Dialog

BOOL SingleKeyProperties (TL_TREEITEM* lptli) {
	DWORD dwID;
	PGPKey* pKey;
	PGPUserID* pUserID;
	PGPCert* pCert;
	BOOL bContinue;
	INT i;

	switch (lptli->iImage) {
	case IDX_RSASECKEY :
	case IDX_RSASECDISKEY :
	case IDX_RSASECREVKEY :
	case IDX_RSASECEXPKEY :
	case IDX_RSAPUBKEY :
	case IDX_RSAPUBDISKEY :
	case IDX_RSAPUBREVKEY :
	case IDX_RSAPUBEXPKEY :
	case IDX_DSASECKEY :
	case IDX_DSASECDISKEY :
	case IDX_DSASECREVKEY :
	case IDX_DSASECEXPKEY :
	case IDX_DSAPUBKEY :
	case IDX_DSAPUBDISKEY :
	case IDX_DSAPUBREVKEY :
	case IDX_DSAPUBEXPKEY :
		pKey = (PGPKey*)(lptli->lParam);
		bContinue = TRUE;
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		pUserID = (PGPUserID*)(lptli->lParam);
		pKey = KMGetKeyFromUserID (g_hWndTree, pUserID);
		bContinue = FALSE;
		break;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
		pCert = (PGPCert*)(lptli->lParam);
		PGPcomdlgErrorMessage (pgpGetCertifier (pCert, g_pKeySetMain, &pKey));
		bContinue = TRUE;
		break;

	default :
		return FALSE;
	}

	// see if property sheet is already open for this key
	for (i=0; i<MAXSHEETS; i++) {
		if (pKey == pKeyTable[i]) {
			SetForegroundWindow (hWndTable[i]);
			return TRUE;
		}
	}

	// no existing property sheet for this key,
	// see if we've reached the max number of sheets
	iNumberSheets++;
	if (iNumberSheets > MAXSHEETS) {
		iNumberSheets--;
		return FALSE;
	}

	// create sheet for this key
	if (pKey) {
		_beginthreadex (NULL, 0, SingleKeyThread, (LPVOID)pKey, 0, &dwID);
	}
	return bContinue;
}

	
//----------------------------------------------------|
//  Put up key properties dialog(s)

BOOL KMKeyProperties (HWND hWnd) {
	TL_TREEITEM tli;
	CHAR sz256[256];
	INT i;

	hWndParent = hWnd;

	if (iNumberSheets == 0) {
		for (i=0; i<MAXSHEETS; i++) {
			hWndTable[i] = NULL;
			pKeyTable[i] = NULL;
		}
	}

	if (KMMultipleSelected ()) {
		TreeList_IterateSelected (g_hWndTree, SingleKeyProperties);
	}
	else {
		tli.hItem = KMFocusedItem ();
		tli.pszText = sz256;
		tli.cchTextMax = 256;
		tli.mask = TLIF_IMAGE | TLIF_TEXT | TLIF_PARAM;
		if (!TreeList_GetItem (g_hWndTree, &tli)) return FALSE;	
		SingleKeyProperties (&tli);
	}
	return TRUE;
}


//----------------------------------------------------|
//  Update all existing propertysheets

VOID KMUpdateKeyProperties (void) {
	INT i;

	if (iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (hWndTable[i] && pKeyTable[i]) {
				SendMessage (hWndTable[i], WM_USER, 0, 0);
			}
		}
	}
}


//----------------------------------------------------|
//  Delete existing propertysheets

VOID KMDeletePropertiesKey (PGPKey* pKey) {
	INT i;

	if (iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKeyTable[i] == pKey) {
				SendMessage (hWndTable[i], WM_USER+1, 0, 0);
				pKeyTable[i] = NULL;
			}
		}
	}
}


//----------------------------------------------------|
//  Delete existing propertysheets

VOID KMDeleteAllKeyProperties (void) {
	INT i;

	if (iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (hWndTable[i]) {
				SendMessage (hWndTable[i], WM_USER+1, 0, 0);
				pKeyTable[i] = NULL;
			}
		}
	}
}
