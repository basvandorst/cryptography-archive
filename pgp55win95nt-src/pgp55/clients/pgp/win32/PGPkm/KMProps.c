//:KMProps.c - handle Key properties dialogs
//
//	$Id: KMProps.c,v 1.47.2.1 1997/11/20 21:54:34 pbj Exp $
//

#include "pgpkmx.h"
#include <process.h>
#include "resource.h"
#include "..\include\pgpphras.h"
#include "..\include\treelist.h"

#define MAXSHEETS	16		// maximum number of simultaneous dialogs

typedef struct {
	PKEYMAN pKM;
	WNDPROC wpOrigFingerProc;
	PGPKeyRef Key;
	PGPKeySetRef keyset;
	INT iIndex;
	DWORD retval;
	UINT uOrigTrust;
	UINT uFinalTrust;
	UINT uOrigValidity;
	UINT uPrevTrust;
	UINT uNumberADKs;
	UINT uPrevTrustMessage;
	PGPBoolean bSecret;
	PGPBoolean bOrigDisabled;
	PGPBoolean bFinalDisabled;
	PGPBoolean bOrigAxiomatic;
	PGPBoolean bFinalAxiomatic;
	BOOL bSettingsModified;
} KMPROPSHEETSTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PKEYMAN pKM;
} PROPSTRUCT;

typedef struct {
	PKEYMAN pKM;
	PGPKeyRef Key;
	PGPKeySetRef keyset;
} THREADSTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static DWORD aIds[] = {			// Help IDs
	IDC_KEYID,			IDH_PGPKM_KEYID,
	IDC_KEYTYPE,		IDH_PGPKM_KEYTYPE,
	IDC_CREATEDATE,		IDH_PGPKM_CREATEDATE,
	IDC_EXPIREDATE,		IDH_PGPKM_KEYEXPIRES,
	IDC_VALIDITY,		IDH_PGPKM_VALIDITY,
	IDC_VALIDITYBAR,	IDH_PGPKM_VALIDITYBAR,
	IDC_TRUST,			IDH_PGPKM_TRUST,
	IDC_TRUSTSLIDER,	IDH_PGPKM_TRUSTSLIDER,
	IDC_FINGERPRINT,	IDH_PGPKM_FINGERPRINT,
	IDC_ENABLED,		IDH_PGPKM_ENABLED,
	IDC_CHANGEPHRASE,	IDH_PGPKM_CHANGEPHRASE,
	IDC_AXIOMATIC,		IDH_PGPKM_AXIOMATIC,
	IDC_ADKAGENT,		IDH_PGPKM_ADKAGENT,
	IDC_KEYSIZE,		IDH_PGPKM_KEYSIZE,
	IDC_CIPHER,			IDH_PGPKM_CIPHER,
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
	CHAR sz32[32];

	switch (uVal) {
		case kPGPValidity_Unknown : i = IDS_VALIDITY0; break;
		case kPGPValidity_Invalid : i = IDS_VALIDITY1; break;
		case kPGPValidity_Marginal : i = IDS_VALIDITY2; break;
		case kPGPValidity_Complete : i = IDS_VALIDITY3; break;
	}
	LoadString (g_hInst, i, sz32, 32);
	SetDlgItemText (hDlg, IDC_VALIDITY, sz32);
	u = KMConvertFromPGPValidity (uVal);
	SendDlgItemMessage (hDlg, IDC_VALIDITYBAR, PBM_SETPOS, u, 0);
}


//----------------------------------------------------|
//  update controls dealing with trust

void SetTrustControls (HWND hDlg, KMPROPSHEETSTRUCT* pkmpss, UINT uTrust) {
	INT i;
	UINT u;
	CHAR sz32[32];

	pkmpss->uPrevTrust = uTrust;
	pkmpss->uPrevTrustMessage = uTrust;
	u = KMConvertFromPGPTrust (uTrust);
	SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETPOS, (WPARAM)TRUE, u);
	switch (uTrust) {
		case kPGPKeyTrust_Undefined	:
		case kPGPKeyTrust_Unknown :
		case kPGPKeyTrust_Never :     i = IDS_TRUST2; break;
		case kPGPKeyTrust_Marginal :  i = IDS_TRUST3; break;
		case kPGPKeyTrust_Complete :  i = IDS_TRUST4; break;
		case kPGPKeyTrust_Ultimate :  i = IDS_TRUST5; break;
	}
	LoadString (g_hInst, i, sz32, 32);
	SetDlgItemText (hDlg, IDC_TRUST, sz32);
}


//----------------------------------------------------|
//  Key Properties Dialog Message procedure

BOOL CALLBACK KeyPropDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	KMPROPSHEETSTRUCT* pkmpss;
	CHAR szbuf[kPGPMaxUserIDSize];
	INT i, iError;
	UINT u, uAlgorithm, uTrust;
	PGPBoolean bB, bAxiomatic, bRevoked, bExpired;
	PGPUInt32 prefAlg[6];
	PGPTime tm;
	HWND hParent;
	RECT rc;
	PGPKeyRef Key;
	CHAR* psz;
	CHAR* pszOld;

	pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

	switch (uMsg) {

	case WM_INITDIALOG:
		// store pointer to data structure
		SetWindowLong (hDlg, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pkmpss = (KMPROPSHEETSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		// save HWND to table
		pkmpss->pKM->hWndTable[pkmpss->iIndex] = hDlg;

		// center dialog on screen
		if (pkmpss->pKM->iNumberSheets == 1) {
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
		PostMessage (hDlg, WM_APP, 0, 0);
		return TRUE;

	case WM_APP :
		KMRequestSDKAccess (pkmpss->pKM);

		PGPGetKeyBoolean (pkmpss->Key,kPGPKeyPropIsSecret,&pkmpss->bSecret);
		PGPGetKeyBoolean (pkmpss->Key,kPGPKeyPropIsRevoked,&bRevoked);
		PGPGetKeyBoolean (pkmpss->Key,kPGPKeyPropIsExpired,&bExpired);

		// initialize key id edit control
		KMGetKeyIDFromKey (pkmpss->Key, szbuf, sizeof(szbuf));
		SetDlgItemText (hDlg, IDC_KEYID, szbuf);

		// initialize key type edit control
		PGPGetKeyNumber (pkmpss->Key, kPGPKeyPropAlgID, &uAlgorithm);
		switch (uAlgorithm) {
		case kPGPPublicKeyAlgorithm_RSA :
			LoadString (g_hInst, IDS_RSA, szbuf, sizeof (szbuf));
			break;

		case kPGPPublicKeyAlgorithm_DSA :
			LoadString (g_hInst, IDS_DSA_ELGAMAL, szbuf, sizeof (szbuf));
			break;

		default :
			LoadString (g_hInst, IDS_UNKNOWN, szbuf, sizeof (szbuf));
			break;
		}
		SetDlgItemText (hDlg, IDC_KEYTYPE, szbuf);

		// initialize key size edit control
		KMGetKeyBitsString (pkmpss->keyset, 
							pkmpss->Key, szbuf, sizeof(szbuf));
		SetDlgItemText (hDlg, IDC_KEYSIZE, szbuf);

		// initialize key creation date edit control
		PGPGetKeyTime (pkmpss->Key, kPGPKeyPropCreation, &tm);
		KMConvertTimeToString (tm, szbuf, sizeof (szbuf));
		SetDlgItemText (hDlg, IDC_CREATEDATE, szbuf);

		// initialize key expiration date edit control
		PGPGetKeyTime (pkmpss->Key, kPGPKeyPropExpiration, &tm);
		if (tm != kPGPExpirationTime_Never) 
			KMConvertTimeToString (tm, szbuf, sizeof (szbuf));
		else 
			LoadString (g_hInst, IDS_NEVER, szbuf, sizeof (szbuf));
		SetDlgItemText (hDlg, IDC_EXPIREDATE, szbuf);

		// initialize preferred cipher edit control
		PGPGetKeyPropertyBuffer (pkmpss->Key, kPGPKeyPropPreferredAlgorithms,
							sizeof(prefAlg), (PGPByte*)&prefAlg[0], &u);
		if (u < sizeof(PGPCipherAlgorithm)) 
			prefAlg[0] = kPGPCipherAlgorithm_IDEA;
		switch (prefAlg[0]) {
			case kPGPCipherAlgorithm_CAST5 :
				LoadString (g_hInst, IDS_CAST, szbuf, sizeof(szbuf));
				break;

			case kPGPCipherAlgorithm_3DES :
				LoadString (g_hInst, IDS_3DES, szbuf, sizeof(szbuf));
				break;

			default :
				LoadString (g_hInst, IDS_IDEA, szbuf, sizeof(szbuf));
				break;
		}
		SetDlgItemText (hDlg, IDC_CIPHER, szbuf);

		// initialize fingerprint edit control
		PGPGetKeyPropertyBuffer(pkmpss->Key, kPGPKeyPropFingerprint,
				sizeof( szbuf ), szbuf, &u);
		KMConvertStringFingerprint (uAlgorithm, szbuf);
		SetDlgItemText (hDlg, IDC_FINGERPRINT, szbuf);

		// initialize validity edit and bar controls
		if (!pkmpss->bSettingsModified) {
			i = KMConvertFromPGPValidity (kPGPValidity_Complete);
			SendDlgItemMessage (hDlg, IDC_VALIDITYBAR, PBM_SETRANGE, 
										0, MAKELPARAM (0,i));
			PGPGetKeyNumber (pkmpss->Key, kPGPKeyPropValidity, 
									&pkmpss->uOrigValidity);
			SetValidityControls (hDlg, pkmpss->uOrigValidity);
		}

		// initialize "Axiomatic" checkbox control
		if (!pkmpss->bSettingsModified) {
			PGPGetKeyBoolean (pkmpss->Key, kPGPKeyPropIsAxiomatic, 
								&pkmpss->bOrigAxiomatic);
			if (pkmpss->bOrigAxiomatic) CheckDlgButton (hDlg, IDC_AXIOMATIC, 
														BST_CHECKED);
			else CheckDlgButton (hDlg, IDC_AXIOMATIC, BST_UNCHECKED);
			if (bRevoked || bExpired || 
					(pkmpss->pKM->ulOptionFlags&KMF_READONLY)) 
				EnableWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), FALSE);
			if (!pkmpss->bSecret && !pkmpss->bOrigAxiomatic) 
				ShowWindow (GetDlgItem (hDlg, IDC_AXIOMATIC), SW_HIDE);
		}

		// initialize trust edit and slider controls
		if (!pkmpss->bSettingsModified) {
			i = KMConvertFromPGPTrust (kPGPKeyTrust_Complete);
			SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, TBM_SETRANGE, 
									0, MAKELPARAM (0,i));
			PGPGetKeyNumber (pkmpss->Key, 
								kPGPKeyPropTrust, &pkmpss->uOrigTrust);
			SetTrustControls (hDlg, pkmpss, pkmpss->uOrigTrust);
			if (pkmpss->bOrigAxiomatic || bRevoked || bExpired || 
								(pkmpss->pKM->ulOptionFlags&KMF_READONLY)) {
				EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), FALSE);
			}
		}

		// initialize enable/disable checkbox
		if (!pkmpss->bSettingsModified) {
			PGPGetKeyBoolean (pkmpss->Key, kPGPKeyPropIsDisabled, 
								&pkmpss->bOrigDisabled);
			if (pkmpss->bOrigDisabled) CheckDlgButton (hDlg, IDC_ENABLED, 
														BST_UNCHECKED);
			else CheckDlgButton (hDlg, IDC_ENABLED, BST_CHECKED);
			if (bRevoked || bExpired || 
					(pkmpss->pKM->ulOptionFlags&KMF_READONLY)) 
				EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
			if (pkmpss->bSecret) {
				PGPGetKeyBoolean (pkmpss->Key, kPGPKeyPropIsAxiomatic, &bB);
				if (bB) EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
			}
		}

		// initialize change passphrase button
		if (!pkmpss->bSecret) 
			ShowWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), SW_HIDE);
		if (pkmpss->pKM->ulOptionFlags & KMF_READONLY) 
			EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);
		if (pkmpss->pKM->ulOptionFlags & KMF_DISABLERSAOPS) {
			if (uAlgorithm == kPGPPublicKeyAlgorithm_RSA)
				EnableWindow (GetDlgItem (hDlg, IDC_CHANGEPHRASE), FALSE);
		}

		// initialize ADK edit box
		if (pkmpss->uNumberADKs > 0) {
			psz = KMAlloc (4);
			psz[0] = '\0';
			for (u=0; u<pkmpss->uNumberADKs; u++) {
				iError = PGPGetIndexedAdditionalRecipientRequest (pkmpss->Key, 
						pkmpss->pKM->KeySetDisp, u, &Key, NULL);
				if (IsntPGPError (iError) && !Key) 
					iError = kPGPError_UnknownError;
				switch (iError) {
				case kPGPError_NoErr :
					KMGetKeyName (Key, szbuf, sizeof(szbuf));
					break;

				case kPGPError_AdditionalRecipientRequestKeyNotFound :
				{
					PGPError	err;
					PGPKeyID	keyid;
					CHAR		szID[kPGPMaxKeyIDStringSize];

					LoadString (g_hInst, IDS_UNKNOWNADK, 
											szbuf, sizeof(szbuf));

					err = PGPGetIndexedAdditionalRecipientRequestKeyID (
								pkmpss->Key, u, &keyid, NULL);

					if (IsntPGPError (err)) {
						err = PGPGetKeyIDString (&keyid, 
									kPGPKeyIDString_Abbreviated, szID);
						if (IsntPGPError (err)) {
							LoadString (g_hInst, IDS_UNKNOWNADKID, 
											szbuf, sizeof(szbuf));
							lstrcat (szbuf, szID);
						}
					}
					break;
				}

				default :
					LoadString (g_hInst, IDS_ERRONEOUSADK, 
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

			SetDlgItemText (hDlg, IDC_ADKAGENT, psz);
			KMFree (psz);
		}

		KMReleaseSDKAccess (pkmpss->pKM);
		break;

	case WM_APP+1 :
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), FALSE);
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
				case kPGPKeyTrust_Undefined	:
				case kPGPKeyTrust_Unknown :
				case kPGPKeyTrust_Never :     i = IDS_TRUST2; break;
				case kPGPKeyTrust_Marginal :  i = IDS_TRUST3; break;
				case kPGPKeyTrust_Complete :  i = IDS_TRUST4; break;
				case kPGPKeyTrust_Ultimate :  i = IDS_TRUST5; break;
			}
			LoadString (g_hInst, i, szbuf, sizeof (szbuf));
			SetDlgItemText (hDlg, IDC_TRUST, szbuf);
			if (LOWORD (wParam) == TB_ENDTRACK) {
				if ((pkmpss->uOrigValidity <= kPGPValidity_Invalid) && 
					(uTrust >= kPGPKeyTrust_Marginal)) {
					if (uTrust != pkmpss->uPrevTrustMessage) {
						pkmpss->uPrevTrustMessage = uTrust;
						SetTrustControls (hDlg, pkmpss, kPGPKeyTrust_Never);
						KMMessageBox (hDlg, IDS_CAPTION, 
							IDS_TRUSTONINVALIDKEY, MB_OK|MB_ICONEXCLAMATION);
						pkmpss->bSettingsModified = TRUE;
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
				if (pkmpss->bSecret) 
					KMChangePhrase (pkmpss->pKM, hDlg, pkmpss->Key);
				break;

			case IDC_AXIOMATIC :
				if (IsDlgButtonChecked (hDlg, IDC_AXIOMATIC) == 
						BST_UNCHECKED) {
					if (!pkmpss->bSecret) {
						EnableWindow (GetDlgItem (hDlg,IDC_AXIOMATIC), FALSE);
					}
					bAxiomatic = FALSE;
					EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), TRUE);
					SetValidityControls (hDlg, kPGPValidity_Unknown);
					SetTrustControls (hDlg, pkmpss, kPGPKeyTrust_Unknown);
				}
				else {
					bAxiomatic = TRUE;
					CheckDlgButton (hDlg, IDC_ENABLED, BST_CHECKED);
					EnableWindow (GetDlgItem (hDlg, IDC_ENABLED), FALSE);
					SetValidityControls (hDlg, kPGPValidity_Complete);
					SetTrustControls (hDlg, pkmpss, kPGPKeyTrust_Ultimate);
				}
				if (bAxiomatic || 
					(KMConvertFromPGPValidity (pkmpss->uOrigValidity) == 0)) {
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), FALSE);
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), FALSE);
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), FALSE);
				}
				else {
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTSLIDER), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT1), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_TRUSTTEXT2), TRUE);
				}
				pkmpss->bSettingsModified = TRUE;
				break;
				
			case IDC_ENABLED :
				pkmpss->bSettingsModified = TRUE;
				break;
		}
		return TRUE;

    case WM_HELP: 
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pkmpss->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
        WinHelp ((HWND) wParam, pkmpss->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_NOTIFY :
		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_APPLY :
			pkmpss->retval = 1;

			// determine axiomatic state
			if (IsDlgButtonChecked (hDlg, IDC_AXIOMATIC) == BST_CHECKED) 
				pkmpss->bFinalAxiomatic = TRUE;
			else 
				pkmpss->bFinalAxiomatic = FALSE;

			// determine trust state
			pkmpss->uFinalTrust = SendDlgItemMessage (hDlg, IDC_TRUSTSLIDER, 
											TBM_GETPOS, 0, 0);
			pkmpss->uFinalTrust = KMConvertToPGPTrust (pkmpss->uFinalTrust);

			// determine enabled state
			if (IsDlgButtonChecked (hDlg, IDC_ENABLED) == BST_CHECKED) 
				pkmpss->bFinalDisabled = FALSE;
			else 
				pkmpss->bFinalDisabled = TRUE;

			// set return value
			SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

		case PSN_HELP :
			WinHelp (hDlg, pkmpss->pKM->szHelpFile, HELP_CONTEXT, 
				IDH_PGPKM_PROPDIALOG); 
			break;

		case PSN_KILLACTIVE :
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
            return TRUE;

		case PSN_RESET :
			pkmpss->retval = 0;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;

		case PSN_QUERYCANCEL :
			pkmpss->retval = 0;
            SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
			break;

		}
	}
	return FALSE;
}


//----------------------------------------------------|
//  Post Key Properties Dialog 

DWORD WINAPI SingleKeyThread (THREADSTRUCT* pts) {
	CHAR szTitle0[32];
	CHAR szUserName[kPGPMaxUserIDSize];
    PROPSHEETPAGE psp[1];
    PROPSHEETHEADER psh;
	KMPROPSHEETSTRUCT kmpss;
	INT i, iRetVal;

	iRetVal = 0;

	for (i=0; i<MAXSHEETS; i++) {
		if (!pts->pKM->KeyTable[i]) {
			if (!pts->pKM->hWndTable[i]) {
				pts->pKM->KeyTable[i] = pts->Key;
				kmpss.iIndex = i;
				break;
			}
		}
	}

	kmpss.pKM = pts->pKM;
	kmpss.Key = pts->Key;
	kmpss.keyset = pts->keyset;
	kmpss.bSettingsModified = FALSE;
	kmpss.retval = 0;

	PGPCountAdditionalRecipientRequests (pts->Key, &kmpss.uNumberADKs);

	LoadString (g_hInst, IDS_PROPTITLE0, szTitle0, sizeof(szTitle0));
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance = g_hInst;
	if (kmpss.uNumberADKs > 0) 
		psp[0].pszTemplate = MAKEINTRESOURCE (IDD_KEYPROPADK);
    else 
		psp[0].pszTemplate = MAKEINTRESOURCE (IDD_KEYPROP);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = KeyPropDlgProc;
    psp[0].pszTitle = szTitle0;
    psp[0].lParam = (LPARAM)&kmpss;
    psp[0].pfnCallback = NULL;

	KMGetKeyName (pts->Key, szUserName, sizeof(szUserName));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	if (pts->pKM->ulOptionFlags & KMF_MODALPROPERTIES)
		psh.hwndParent = pts->pKM->hWndParent;
	else
		psh.hwndParent = NULL;
    psh.hInstance = g_hInst;
    psh.pszIcon = NULL;
    psh.pszCaption = (LPSTR) szUserName;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

	kmpss.pKM->KeyTable[kmpss.iIndex] = NULL;
	kmpss.pKM->iNumberSheets--;

	if (kmpss.retval && (kmpss.pKM->ulOptionFlags & KMF_ENABLECOMMITS)) {
		HCURSOR		hCursorOld; 
		PGPError	err;
		UINT		uReloadMessage;

		KMRequestSDKAccess (kmpss.pKM);

		iRetVal = 0;

		// update axiomatic state
		if (kmpss.bFinalAxiomatic) {
			if (!kmpss.bOrigAxiomatic) {
				if (kmpss.bOrigDisabled) 
					PGPEnableKey (kmpss.Key);
				PGPcomdlgErrorMessage (PGPSetKeyAxiomatic (kmpss.Key, 
												FALSE, "")); 
				iRetVal = 1;
			}
		}
		else {
			if (kmpss.bOrigAxiomatic) {
				PGPcomdlgErrorMessage (
					PGPUnsetKeyAxiomatic (kmpss.Key));
				iRetVal = 1;
			}

			// update trust level
			if (kmpss.uFinalTrust != kmpss.uOrigTrust) {
				PGPcomdlgErrorMessage (PGPSetKeyTrust (kmpss.Key, 
									(unsigned char)kmpss.uFinalTrust)); 
				iRetVal = 1;
			}
		}

		// update enable/disable state
		if (!kmpss.bFinalDisabled) {
			if (kmpss.bOrigDisabled) {
				PGPEnableKey (kmpss.Key);
				iRetVal = 1;
			}
		}
		else {
			if (!kmpss.bOrigDisabled) {
				PGPDisableKey (kmpss.Key);
				iRetVal = 1;
			}
		}

		if (iRetVal == 0) {
			KMReleaseSDKAccess (kmpss.pKM);
		}
		else {
			hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
			err = PGPCommitKeyRingChanges (kmpss.pKM->KeySetMain);
			SetCursor (hCursorOld);
			KMReleaseSDKAccess (kmpss.pKM);
			SendMessage (kmpss.pKM->hWndParent, KM_M_UPDATEKEYINTREE, 0,
							(LPARAM)(kmpss.Key));

			KMUpdateKeyProperties (kmpss.pKM);

			if (kmpss.pKM->ulOptionFlags & KMF_ENABLERELOADS) {
				if (IsntPGPError (err)) {
					uReloadMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
					PostMessage (HWND_BROADCAST, uReloadMessage, 
						MAKEWPARAM (LOWORD (kmpss.pKM->hWndParent), FALSE), 
						GetCurrentProcessId ());
				}
			}
		}
		InvalidateRect (kmpss.pKM->hWndTree, NULL, TRUE);

	}

	kmpss.pKM->hWndTable[kmpss.iIndex] = NULL;

	if (kmpss.pKM->iNumberSheets == 0) {
		SetForegroundWindow (kmpss.pKM->hWndParent);
	}

	return iRetVal;
}


//----------------------------------------------------|
//  Post Key Properties Dialog 

BOOL CALLBACK SingleKeyProperties (TL_TREEITEM* lptli, LPARAM lParam) {
	DWORD dwID;
	PGPKeyRef Key;
	PGPKeySetRef keyset;
	PGPUserIDRef UserID;
	PGPError err;
	PGPSigRef Cert;
	BOOL bContinue;
	INT i;
	THREADSTRUCT* pts;
	PROPSTRUCT* pps = (PROPSTRUCT*)lParam;

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
		Key = (PGPKeyRef)(lptli->lParam);
		keyset = pps->pKM->KeySetDisp;
		bContinue = TRUE;
		break;

	case IDX_RSAUSERID :
	case IDX_DSAUSERID :
		UserID = (PGPUserIDRef)(lptli->lParam);
		Key = KMGetKeyFromUserID (pps->pKM, UserID);
		keyset = pps->pKM->KeySetDisp;
		bContinue = FALSE;
		break;

	case IDX_CERT :
	case IDX_REVCERT :
	case IDX_BADCERT :
	case IDX_EXPORTCERT :
	case IDX_METACERT :
	case IDX_EXPORTMETACERT :
		Cert = (PGPSigRef)(lptli->lParam);
		Key = NULL;

		err = PGPGetSigCertifierKey (Cert, pps->pKM->KeySetDisp, &Key);
		if (IsntPGPError (err)) {
			keyset = pps->pKM->KeySetDisp;
		}

		if (!Key && (pps->pKM->KeySetDisp != pps->pKM->KeySetMain)) {
			err = PGPGetSigCertifierKey (Cert, pps->pKM->KeySetMain, &Key);
			if (IsntPGPError (err)) {
				keyset = pps->pKM->KeySetMain;
			}
		}

		PGPcomdlgErrorMessage (err);
		bContinue = TRUE;
		break;

	default :
		return FALSE;
	}

	// see if property sheet is already open for this key
	for (i=0; i<MAXSHEETS; i++) {
		if (Key == pps->pKM->KeyTable[i]) {
			SetForegroundWindow (pps->pKM->hWndTable[i]);
			return TRUE;
		}
	}

	// no existing property sheet for this key, 
	// see if we've reached the max number of sheets
	pps->pKM->iNumberSheets++;
	if (pps->pKM->iNumberSheets > MAXSHEETS) {
		pps->pKM->iNumberSheets--;
		return FALSE;
	}

	// create sheet for this key
	if (Key) {
		pts = (THREADSTRUCT*)KMAlloc (sizeof (THREADSTRUCT));
		if (pts) {
			pts->pKM = pps->pKM;
			pts->Key = Key;
			pts->keyset = keyset;
			_beginthreadex (NULL, 0, SingleKeyThread, (LPVOID)pts, 0, &dwID);
		}
	}
	return bContinue;
}

	
//----------------------------------------------------|
//  Put up key properties dialog(s)

BOOL KMKeyProperties (PKEYMAN pKM) {
	TL_TREEITEM tli;
	CHAR sz256[256];
	INT i;
	PROPSTRUCT ps;

	if (pKM->iNumberSheets == 0) {
		for (i=0; i<MAXSHEETS; i++) {
			pKM->hWndTable[i] = NULL;
			pKM->KeyTable[i] = NULL;
		}
	}

	ps.pKM = pKM;

	if (KMMultipleSelected (pKM)) {
		ps.lpfnCallback = SingleKeyProperties;
		TreeList_IterateSelected (pKM->hWndTree, &ps);
	}
	else {
		tli.hItem = KMFocusedItem (pKM);
		tli.pszText = sz256;
		tli.cchTextMax = 256;
		tli.mask = TLIF_IMAGE | TLIF_TEXT | TLIF_PARAM;
		if (!TreeList_GetItem (pKM->hWndTree, &tli)) return FALSE;	
		SingleKeyProperties (&tli, (LPARAM)&ps);
	}
	return TRUE;
}


//----------------------------------------------------|
//  Update all existing propertysheets

VOID KMUpdateKeyProperties (PKEYMAN pKM) {
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i] && pKM->KeyTable[i]) {
				PostMessage (pKM->hWndTable[i], WM_APP, 0, 0);
			}
		}
	}
}


//----------------------------------------------------|
//  Delete existing propertysheets

VOID KMDeletePropertiesKey (PKEYMAN pKM, PGPKeyRef Key) {
	INT i;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->KeyTable[i] == Key) {
				SendMessage (pKM->hWndTable[i], WM_APP+1, 0, 0);
				pKM->KeyTable[i] = NULL;
			}
		}
	}
}


//----------------------------------------------------|
//  Delete existing propertysheets

VOID 
KMDeleteAllKeyProperties (PKEYMAN pKM, 
						  BOOL bCloseWindows) 
{
	INT i;
	BOOL bStillOpen;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i]) {
				if (bCloseWindows) 
					PropSheet_PressButton (GetParent (pKM->hWndTable[i]),
										   PSBTN_CANCEL);
				else
					SendMessage (pKM->hWndTable[i], WM_APP+1, 0, 0);	
				pKM->KeyTable[i] = NULL;
			}
		}
	}

	// wait for all threads to terminate
	if (bCloseWindows) {
		do {
			bStillOpen = FALSE;
			for (i=0; i<MAXSHEETS; i++) {
				if (pKM->hWndTable[i]) bStillOpen = TRUE;
			}
		} while (bStillOpen);
	}

	Sleep (100);
}


//----------------------------------------------------|
//  Enable/Disable existing propertysheets

VOID
KMEnableAllKeyProperties (PKEYMAN pKM, 
						  BOOL bEnable) 
{
	INT		i;
	HWND	hwndParent;

	if (pKM->iNumberSheets > 0) {
		for (i=0; i<MAXSHEETS; i++) {
			if (pKM->hWndTable[i]) {
				hwndParent = GetParent (pKM->hWndTable[i]);
				EnableWindow (hwndParent, bEnable);
			}
		}
	}
}



