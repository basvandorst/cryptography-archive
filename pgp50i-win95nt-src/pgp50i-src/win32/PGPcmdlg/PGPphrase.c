//:PGPphrase - get passphrase from user
//
//	This code originally attempted to thwart some common
//	situations which could haved compromised the security
//	of the passphrase.  However, various sections of this
//	code have been removed or disabled as they were found
//	to cause unwanted side-effects.  Thus, this code is
//	susceptable to hook functions intercepting keystroke
//	messages.
//
//	The edit boxes are subclassed to inhibit the
//	WM_GETTEXT message from returning their text (they
//	place their text directly into the change_szbuf buffer,
//	instead) to thwart hooks from asking their text.
//  NB: the subclassing is also used to hide the text
//  as it is typed by setting the text color to the
//  background color.
/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#include "pgpcomdlgx.h"
#include "..\include\pgpmem.h"

#define MAXNAMECHAR		30

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static LPSTR szPassPhrase;
static HWND hwndParent;
static BOOL bHideText;
static WNDPROC wpOrigPhraseProc;
static HHOOK hhookDebug;
static HHOOK hhookKeyboard;
static HHOOK hhookMouse;
static LPSTR szPromptString;
static PGPKey** ppKeySelected;
static PGPKeySet* pKeySetMain;
static LPSTR szInitialKeyID;
static UINT* uSigningOptions;
static INT iNextTabControl;
static UINT uEntryFlags;

static DWORD aIds[] = {			// Help IDs
    IDC_HIDETYPING,		IDH_PGPCDPHRASE_HIDETYPING,
    IDC_PHRASE,			IDH_PGPCDPHRASE_PHRASE,
	IDC_SIGNKEYCOMBO,	IDH_PGPCDPHRASE_SIGNINGKEY,
	IDC_ASCIIARMOR,		IDH_PGPCDPHRASE_ASCIIARMOR,
	IDC_DETACHEDSIG,	IDH_PGPCDPHRASE_DETACHEDSIG,
    0,0
};


//----------------------------------------------------|
// Wipe edit box clean

VOID WipeEditBox (HWND hDlg, UINT uID) {
	CHAR* p;
	INT i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = pgpMemAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			pgpMemFree (p);
		}
	}
}


//----------------------------------------------------|
//  Check if message is dangerous to pass to passphrase edit box

BOOL CommonPhraseMsgProc (HWND hwnd, UINT uMsg,
						  WPARAM wParam, LPARAM lParam) {
     switch (uMsg) {
	case WM_RBUTTONDOWN :
	case WM_CONTEXTMENU :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
		return TRUE;

	case WM_PAINT :
		if (wParam) {
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (bHideText)
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		}
		break;

	case WM_KEYDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) {
			switch (wParam) {
			case VK_HOME :
			case VK_END :
			case VK_UP :
			case VK_DOWN :
			case VK_LEFT :
			case VK_RIGHT :
			case VK_NEXT :
			case VK_PRIOR :
				return TRUE;
			}
		}
		break;

	case WM_SETFOCUS :
		SendMessage (hwnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	}
     return FALSE;
}


//----------------------------------------------------|
//  Passphrase edit box subclass procedure

LRESULT APIENTRY PhraseSubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {
	if (CommonPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;

	switch (uMsg) {
	case WM_GETTEXT :
		if (!szPassPhrase) return 0;
		lParam = (LPARAM)szPassPhrase;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_HIDETYPING));
			else SetFocus (GetDlgItem (GetParent (hWnd), iNextTabControl));
		}
		break;
	}
     return CallWindowProc(wpOrigPhraseProc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  Dialog Message procedure

BOOL CALLBACK GetPhraseDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	CHAR szDummy[4];
	INT i;

	switch (uMsg) {

		case WM_INITDIALOG:
			iNextTabControl = IDOK;

			SetDlgItemText (hDlg, IDC_PROMPTSTRING, szPromptString);

			bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			wpOrigPhraseProc = (WNDPROC) SetWindowLong (
										GetDlgItem (hDlg, IDC_PHRASE),
										GWL_WNDPROC,
										(LONG) PhraseSubclassProc);
			SetForegroundWindow (hDlg);
			return TRUE;

		case WM_HELP:
	       WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
	       HELP_WM_HELP, (DWORD) (LPSTR) aIds);
	       break;

		case WM_CONTEXTMENU:
		    WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
		       (DWORD) (LPVOID) aIds);
		    break;

         case WM_DESTROY:
             SetWindowLong (GetDlgItem(hDlg, IDC_PHRASE),
						   GWL_WNDPROC,
						    (LONG)wpOrigPhraseProc);
			WipeEditBox (hDlg, IDC_PHRASE);
			return TRUE;

		case WM_CTLCOLOREDIT:
			if ((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASE)) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideText)
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam,
								  GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			}
			break;

		case WM_COMMAND:
			switch(LOWORD (wParam)) {
				case IDOK:
					if (szPassPhrase) {
						FillMemory (szPassPhrase,
									lstrlen (szPassPhrase), '\0');
						pgpMemFree (szPassPhrase);
						szPassPhrase = NULL;
					}
					i = SendDlgItemMessage (hDlg, IDC_PHRASE,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szPassPhrase = pgpMemAlloc (i);
					if (szPassPhrase) {
						GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
						EndDialog (hDlg, PGPCOMDLG_OK);
					}
					else
						EndDialog (hDlg, PGPCOMDLG_MEMERROR);
					return TRUE;

				case IDCANCEL:
					EndDialog (hDlg, PGPCOMDLG_CANCEL);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
								== BST_CHECKED)
						bHideText = TRUE;
					else
						bHideText = FALSE;

					InvalidateRect (GetDlgItem (hDlg,IDC_PHRASE), NULL, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Convert entire KeyID from big endian to
// character string format.
// NB: sz must be at least 19 bytes long

VOID ConvertStringKeyID64 (LPSTR sz) {
	INT i;
	ULONG ulFirst, ulSecond;

	// convert first half of keyid to little endian
	ulFirst = 0;
	for (i=0; i<4; i++) {
		ulFirst <<= 8;
		ulFirst |= ((ULONG)sz[i] & 0xFF);
	}

	// convert first half of keyid to little endian
	ulSecond = 0;
	for (i=0; i<4; i++) {
		ulSecond <<= 8;
		ulSecond |= ((ULONG)sz[i+4] & 0xFF);
	}

	// convert to string
	wsprintf (sz, "0x%08lX%08lX", ulFirst, ulSecond);
}


//----------------------------------------------------|
//  setup keyselection combo box

BOOL InitKeyComboBox (HWND hDlg) {

	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKey* pKey;
	PGPKey* pKeyDefault;
	UINT u, uIndex, uKeyBits, uAlgorithm;
	INT iKeyDefault, iKeySelected;
	Boolean bSecret, bRevoked, bExpired, bCanSign;
	BOOL bAtLeastOneSecretKey;
	CHAR szName[256];
	CHAR szKeyID[20];

	pKeyList = pgpOrderKeySet (pKeySetMain, kPGPUserIDOrdering);
	pKeyIter = pgpNewKeyIter (pKeyList);
	pKeyDefault = pgpGetDefaultPrivateKey (pKeySetMain);

	iKeySelected = -1;
	iKeyDefault = -1;

	bAtLeastOneSecretKey = FALSE;

	pKey = pgpKeyIterNext (pKeyIter);
	while (pKey) {
		pgpGetKeyBoolean (pKey, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			pgpGetKeyBoolean (pKey, kPGPKeyPropIsRevoked, &bRevoked);
			pgpGetKeyBoolean (pKey, kPGPKeyPropIsExpired, &bExpired);
			pgpGetKeyBoolean (pKey, kPGPKeyPropCanSign, &bCanSign);
			if (!bRevoked && !bExpired && bCanSign) {
				bAtLeastOneSecretKey = TRUE;

				// get name on key
				u = sizeof (szName);
				pgpGetPrimaryUserIDName (pKey, szName, &u);
				if (u > MAXNAMECHAR) {
					u = MAXNAMECHAR;
					szName[u] = '\0';
					lstrcat (szName, "...");
				}
				else szName[u] = '\0';

				// append key type / size info to name
				lstrcat (szName, " (");
				pgpGetKeyNumber (pKey, kPGPKeyPropAlgId, &uAlgorithm);
				switch (uAlgorithm) {
				case PGP_PKALG_RSA :
					LoadString (g_hInst, IDS_RSA, g_szbuf, G_BUFLEN);
					lstrcat (szName, g_szbuf);
					lstrcat (szName, "/");
					pgpGetKeyNumber (pKey, kPGPKeyPropBits, &uKeyBits);
					wsprintf (g_szbuf, "%i", uKeyBits);
					lstrcat (szName, g_szbuf);
					break;

				case PGP_PKALG_DSA :
					LoadString (g_hInst, IDS_DSA, g_szbuf, G_BUFLEN);
					lstrcat (szName, g_szbuf);
					lstrcat (szName, "/");
					pgpGetKeyNumber (pKey, kPGPKeyPropBits, &uKeyBits);
					wsprintf (g_szbuf, "%i", uKeyBits);
					lstrcat (szName, g_szbuf);
					break;

				default :
					LoadString (g_hInst, IDS_UNKNOWN, g_szbuf, G_BUFLEN);
					lstrcat (szName, g_szbuf);
					lstrcat (szName, "/");
					lstrcat (szName, g_szbuf);
					break;
				}
				lstrcat (szName, ")");

				uIndex = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO,
								CB_ADDSTRING, 0, (LPARAM)szName);
				if (uIndex != CB_ERR) {
					SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO,
								CB_SETITEMDATA, uIndex, (LPARAM)pKey);
					if (*ppKeySelected) {
						if (pKey == *ppKeySelected) iKeySelected = uIndex;
					}
					if (szInitialKeyID) {
						u = sizeof (szKeyID);
						pgpGetKeyString (pKey, kPGPKeyPropKeyId, szKeyID, &u);
						szKeyID[u] = '\0';
						ConvertStringKeyID64 (szKeyID);
						if (lstrcmpi (szKeyID, szInitialKeyID) == 0)
							iKeySelected = uIndex;
					}
					if (pKey == pKeyDefault) iKeyDefault = uIndex;
				}
			}
		}
		pKey = pgpKeyIterNext (pKeyIter);
	}
	pgpFreeKeyIter (pKeyIter);
	pgpFreeKeyList (pKeyList);

	if (iKeySelected == -1) iKeySelected = iKeyDefault;
	if (iKeySelected != -1)
		SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, CB_SETCURSEL,
							iKeySelected, 0);

	return (bAtLeastOneSecretKey);

}

//----------------------------------------------------|
//  Enable or disable passphrase edit box on basis of selected key

VOID EnablePassPhraseControl (HWND hDlg) {
	PGPKey* pKey;
	Boolean bNeedsPhrase;
	BOOL bEnabled;
	INT i;

	bEnabled = FALSE;
	i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, CB_GETCURSEL, 0, 0);
	if (i != CB_ERR) {					
		pKey = (PGPKey*)SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO,
											CB_GETITEMDATA, i, 0);
		if (pKey) {
			pgpGetKeyBoolean (pKey, kPGPKeyPropNeedsPassphrase,
								&bNeedsPhrase);
			if (bNeedsPhrase) bEnabled = TRUE;
		}
	}
	if (!bEnabled) SetDlgItemText (hDlg, IDC_PHRASE, "");
	EnableWindow (GetDlgItem (hDlg, IDC_PHRASE), bEnabled);
	EnableWindow (GetDlgItem (hDlg, IDC_HIDETYPING), bEnabled);
}


//----------------------------------------------------|
//  Post warning about encrypting to RSA and signing w/DSA key

LRESULT PGPExport PGPcomdlgRSADSAMixWarning (HWND hWnd) {
	INT i;

	i = PGPcomdlgMessageBox (hWnd, IDS_CAPTION, IDS_RSADSAMIXWARNING,
		MB_YESNOCANCEL | MB_ICONEXCLAMATION);

	switch (i) {
	case IDYES : return PGPCOMDLG_OK;
	case IDNO : return PGPCOMDLG_NO;
	default : return PGPCOMDLG_CANCEL;
	}

}


//----------------------------------------------------|
//  Dialog Message procedure - get passphrase and signing key

BOOL CALLBACK GetPhraseKeyDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	CHAR szDummy[4];
	INT i;
	UINT uAlgorithm;

	switch (uMsg) {

		case WM_INITDIALOG:
			iNextTabControl = IDOK;
			SetDlgItemText (hDlg, IDC_PROMPTSTRING, szPromptString);

			bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			wpOrigPhraseProc = (WNDPROC) SetWindowLong(
											GetDlgItem(hDlg, IDC_PHRASE),
											GWL_WNDPROC,
											(LONG) PhraseSubclassProc);

			if (InitKeyComboBox (hDlg)) {
				EnablePassPhraseControl (hDlg);
			}
			else {
				EndDialog (hDlg, PGPCOMDLG_NOSECRETKEYS);
			}
			SetForegroundWindow (hDlg);
			return TRUE;

		case WM_HELP:
	       WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
	       HELP_WM_HELP, (DWORD) (LPSTR) aIds);
	       break;

		case WM_CONTEXTMENU:
		     WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
		       (DWORD) (LPVOID) aIds);
		     break;

         case WM_DESTROY:
             SetWindowLong (GetDlgItem (hDlg, IDC_PHRASE),
						   GWL_WNDPROC,
						   (LONG)wpOrigPhraseProc);
			WipeEditBox (hDlg, IDC_PHRASE);
			return TRUE;

		case WM_CTLCOLOREDIT:
			if ((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASE)) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideText)
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam,
									GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			}
			break;

		case WM_COMMAND:
			switch(LOWORD (wParam)) {
				case IDOK:
					if (szPassPhrase) {
						FillMemory (szPassPhrase,
									lstrlen (szPassPhrase), '\0');
						pgpMemFree (szPassPhrase);
						szPassPhrase = NULL;
					}
					i = SendDlgItemMessage (hDlg, IDC_PHRASE,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szPassPhrase = pgpMemAlloc (i);
					if (szPassPhrase) {
						GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
						i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO,
												CB_GETCURSEL, 0, 0);
						if (i == CB_ERR) {
							PGPcomdlgMessageBox (hDlg,
								IDS_ERRORMESSAGECAPTION, IDS_NOSIGNKEY,
								MB_OK | MB_ICONEXCLAMATION);
						}
						else {
							if (ppKeySelected) {
								*ppKeySelected =
									(PGPKey*)SendDlgItemMessage (hDlg,
										IDC_SIGNKEYCOMBO,
											CB_GETITEMDATA, i, 0);
								if (uEntryFlags & PGPCOMDLG_RSAENCRYPT) {
									pgpGetKeyNumber (*ppKeySelected,
										kPGPKeyPropAlgId, &uAlgorithm);
									if (uAlgorithm == PGP_PKALG_DSA) {
										i = PGPcomdlgRSADSAMixWarning (hDlg);
										switch (i) {
										case PGPCOMDLG_OK :
											EndDialog (hDlg, PGPCOMDLG_OK);
											break;

										case PGPCOMDLG_CANCEL :
											EndDialog (hDlg,
												PGPCOMDLG_CANCEL);
											break;

										case PGPCOMDLG_NO :
											break;
										}
										return TRUE;
									}
								}
							}
							EndDialog (hDlg, PGPCOMDLG_OK);
						}
					}
					else
						EndDialog (hDlg, PGPCOMDLG_MEMERROR);
					return TRUE;

				case IDC_SIGNKEYCOMBO :
					EnablePassPhraseControl (hDlg);
					break;

				case IDCANCEL:
					EndDialog (hDlg, PGPCOMDLG_CANCEL);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
								== BST_CHECKED)
						bHideText = TRUE;
					else
						bHideText = FALSE;

					InvalidateRect (GetDlgItem (hDlg, IDC_PHRASE),
									NULL, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Dialog Message procedure - get passphrase, signing key, and options

BOOL CALLBACK GetPhraseOptionsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	CHAR szDummy[4];
	INT i;
	UINT uAlgorithm;

	switch (uMsg) {

		case WM_INITDIALOG:
			iNextTabControl = IDC_ASCIIARMOR;
			SetDlgItemText (hDlg, IDC_PROMPTSTRING, szPromptString);

			bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			if (*uSigningOptions & PGPCOMDLG_ASCIIARMOR)
				CheckDlgButton (hDlg, IDC_ASCIIARMOR, BST_CHECKED);
			else
				CheckDlgButton (hDlg, IDC_ASCIIARMOR, BST_UNCHECKED);

			if (*uSigningOptions & PGPCOMDLG_DETACHEDSIG)
				CheckDlgButton (hDlg, IDC_DETACHEDSIG, BST_CHECKED);
			else
				CheckDlgButton (hDlg, IDC_DETACHEDSIG, BST_UNCHECKED);

			wpOrigPhraseProc = (WNDPROC) SetWindowLong(
											GetDlgItem(hDlg, IDC_PHRASE),
											GWL_WNDPROC,
											(LONG) PhraseSubclassProc);

			if (InitKeyComboBox (hDlg)) {
				EnablePassPhraseControl (hDlg);
			}
			else {
				EndDialog (hDlg, PGPCOMDLG_NOSECRETKEYS);
			}
			SetForegroundWindow (hDlg);
			return TRUE;

		case WM_HELP:
	       WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile,
	       HELP_WM_HELP, (DWORD) (LPSTR) aIds);
	       break;

		case WM_CONTEXTMENU:
		     WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
		       (DWORD) (LPVOID) aIds);
		     break;

         case WM_DESTROY:
             SetWindowLong (GetDlgItem (hDlg, IDC_PHRASE),
						   GWL_WNDPROC,
						    (LONG)wpOrigPhraseProc);
			WipeEditBox (hDlg, IDC_PHRASE);
			return TRUE;

		case WM_CTLCOLOREDIT:
			if ((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASE)) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideText)
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam,
									GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			}
			break;

		case WM_COMMAND:
			switch(LOWORD (wParam)) {
				case IDOK:
					if (IsDlgButtonChecked (hDlg, IDC_ASCIIARMOR)
							== BST_CHECKED)
						*uSigningOptions |= PGPCOMDLG_ASCIIARMOR;
					else
						*uSigningOptions &= ~PGPCOMDLG_ASCIIARMOR;

					if (IsDlgButtonChecked (hDlg, IDC_DETACHEDSIG)
							== BST_CHECKED)
						*uSigningOptions |= PGPCOMDLG_DETACHEDSIG;
					else
						*uSigningOptions &= ~PGPCOMDLG_DETACHEDSIG;

					if (szPassPhrase) {
						FillMemory (szPassPhrase,
									lstrlen (szPassPhrase), '\0');
						pgpMemFree (szPassPhrase);
						szPassPhrase = NULL;
					}
					i = SendDlgItemMessage (hDlg, IDC_PHRASE,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szPassPhrase = pgpMemAlloc (i);
					if (szPassPhrase) {
						GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
						i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO,
												CB_GETCURSEL, 0, 0);
						if (i == CB_ERR) {
							PGPcomdlgMessageBox (hDlg,
								IDS_ERRORMESSAGECAPTION, IDS_NOSIGNKEY,
								MB_OK | MB_ICONEXCLAMATION);
						}
						else {
							if (ppKeySelected) {
								*ppKeySelected =
									(PGPKey*)SendDlgItemMessage (hDlg,
										IDC_SIGNKEYCOMBO,
											CB_GETITEMDATA, i, 0);
								if (uEntryFlags & PGPCOMDLG_RSAENCRYPT) {
									pgpGetKeyNumber (*ppKeySelected,
										kPGPKeyPropAlgId, &uAlgorithm);
									if (uAlgorithm == PGP_PKALG_DSA) {
										i = PGPcomdlgRSADSAMixWarning (hDlg);
										switch (i) {
										case IDYES :
											EndDialog (hDlg, PGPCOMDLG_OK);
											break;

										case IDCANCEL :
											EndDialog (hDlg,
												PGPCOMDLG_CANCEL);
											break;

										case IDNO :
											break;
										}
										return TRUE;
									}
								}
							}
							EndDialog (hDlg, PGPCOMDLG_OK);
						}
					}
					else
						EndDialog (hDlg, PGPCOMDLG_MEMERROR);

					return TRUE;

				case IDC_SIGNKEYCOMBO :
					EnablePassPhraseControl (hDlg);
					break;

				case IDCANCEL:
					EndDialog (hDlg, PGPCOMDLG_CANCEL);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
								== BST_CHECKED)
						bHideText = TRUE;
					else
						bHideText = FALSE;

					InvalidateRect (GetDlgItem (hDlg, IDC_PHRASE),
										NULL, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Hook procedure for WH_MOUSE hook.

LRESULT CALLBACK MouseHookProc (INT iCode, WPARAM wParam, LPARAM lParam) {
	MOUSEHOOKSTRUCT* pmhs;

    if (iCode < 0)  // do not process message
         return CallNextHookEx (hhookMouse, iCode, wParam, lParam);

	pmhs = (MOUSEHOOKSTRUCT*) lParam;
	pgpRandPoolMouse (pmhs->pt.x, pmhs->pt.y);

	return CallNextHookEx (hhookMouse, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook -- inhibits other hook procedures

LRESULT CALLBACK GetKBHookProc (INT nCode, WPARAM wParam, LPARAM lParam) {

	pgpRandPoolKeystroke (0);
	return CallNextHookEx (hhookKeyboard, nCode, wParam, lParam);
}


//----------------------------------------------------|
//  Hook procedure for WH_DEBUG hook -- inhibits other hook procedures

LRESULT CALLBACK GetDebugHookProc (INT iCode, WPARAM wParam, LPARAM lParam) {

     if (iCode < 0)  /* do not process message */
         return CallNextHookEx (hhookDebug, iCode, wParam, lParam);

//	if (wParam != 2)
//        return CallNextHookEx (hhookDebug, iCode, wParam, lParam);

	return CallNextHookEx (hhookDebug, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Entry point called by app to post dialog and get phrase

UINT PGPExport PGPcomdlgGetPhrase (HWND hWndParent, LPSTR szPrompt,
									LPSTR* pszPhrase, PGPKeySet* pKeySet,
									PGPKey** ppKey, LPSTR szKeyID,
									UINT* uOptions, UINT uFlags) {

	DWORD dwPhraseThreadID;
	UINT uRetval;

	szPassPhrase = NULL;
	szPromptString = szPrompt;

	pKeySetMain = pKeySet;
	ppKeySelected = ppKey;
	szInitialKeyID = szKeyID;
	uSigningOptions = uOptions;
	uEntryFlags = uFlags;

	dwPhraseThreadID = GetCurrentThreadId ();
	hhookMouse = SetWindowsHookEx (WH_MOUSE, MouseHookProc,
									NULL, GetCurrentThreadId());
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, GetKBHookProc,
									NULL, dwPhraseThreadID);
	hhookDebug = SetWindowsHookEx (WH_DEBUG, GetDebugHookProc,
									NULL, dwPhraseThreadID);

	if (pKeySetMain && ppKeySelected) {
		if (uOptions) {
			uRetval = DialogBox (g_hInst,
								 MAKEINTRESOURCE (IDD_PASSPHRASEOPTIONS),
								  hWndParent, GetPhraseOptionsDlgProc);
		}
		else {
			uRetval = DialogBox (g_hInst,
								 MAKEINTRESOURCE (IDD_PASSPHRASEKEYS),
								 hWndParent, GetPhraseKeyDlgProc);
		}
	}
	else {
		uRetval = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_PASSPHRASE),
							 hWndParent, GetPhraseDlgProc);
	}
	if (uRetval == PGPCOMDLG_OK) *pszPhrase = szPassPhrase;
	else *pszPhrase = NULL;

	UnhookWindowsHookEx (hhookDebug);
	UnhookWindowsHookEx (hhookKeyboard);
	UnhookWindowsHookEx (hhookMouse);
	EnableWindow (hwndParent, TRUE);

	return uRetval;
}


//----------------------------------------------------|
//  Routine to wipe and free passphrase

VOID PGPExport PGPcomdlgFreePhrase (LPSTR szPhrase) {

	if (szPhrase) {
		FillMemory (szPhrase, lstrlen (szPhrase), '\0');
		pgpMemFree (szPhrase);
	}
}
