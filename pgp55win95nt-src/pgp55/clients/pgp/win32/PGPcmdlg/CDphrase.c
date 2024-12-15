/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPphrase.c - get passphrase from user

	This code originally attempted to thwart some common 
	situations which could haved compromised the security 
	of the passphrase.  However, various sections of this
	code have been removed or disabled as they were found
	to cause unwanted side-effects.  Thus, this code is
	susceptable to hook functions intercepting keystroke
	messages.

	The edit boxes are subclassed to inhibit the 
	WM_GETTEXT message from returning their text (they
	place their text directly into the change_szbuf buffer, 
	instead) to thwart hooks from asking their text.
	NB: the subclassing is also used to hide the text
	as it is typed by setting the text color to the 
	background color.
	

	$Id: CDphrase.c,v 1.46.4.1 1997/11/10 15:16:32 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"

#include "pgpBuildFlags.h"

#include "pgpRandomPool.h"

#define MAXDECRYPTIONNAMECHAR		36

typedef struct {
	PGPContextRef	context;
	LPSTR			pszPromptString;
	HWND			hwndParent;
	PGPKeyRef*		pKeySelected;
	PGPKeySetRef	keysetDisplay;
	PGPKeyID*		pKeyIDsDisplay;
	UINT			uKeyCountDisplay;
	UINT*			puSigningOptions;
	BOOL			bSearchKeys;
} PHRASESTRUCT, *PPHRASESTRUCT;

// external globals
extern HINSTANCE	g_hInst;
extern CHAR			g_szHelpFile[MAX_PATH];

// local globals
static LPSTR		pszPassPhrase = NULL;
static LPSTR		pszPassPhraseConf = NULL;
static WNDPROC		wpOrigPhraseProc;  
static WNDPROC		wpOrigPhraseConfProc;  
static HHOOK		hhookKeyboard;
static HHOOK		hhookMouse;
static INT			iNextTabControl;
static BOOL			bHideText = TRUE;
static HWND			hwndQuality;


static DWORD aIds[] = {			// Help IDs
    IDC_HIDETYPING,		IDH_PGPCDPHRASE_HIDETYPING, 
    IDC_PHRASE,			IDH_PGPCDPHRASE_PHRASE, 
	IDC_PHRASECONV,		IDH_PGPCDPHRASE_CONVPHRASE,
	IDC_PHRASE2,		IDH_PGPCDPHRASE_CONFIRMATION,
	IDC_SIGNKEYCOMBO,	IDH_PGPCDPHRASE_SIGNINGKEY,
	IDC_KEYLISTBOX,		IDH_PGPCDPHRASE_KEYLIST,
	IDC_PHRASEQUALITY,	IDH_PGPCDPHRASE_QUALITY,
	IDC_ASCIIARMOR,		IDH_PGPCDPHRASE_ASCIIARMOR,
	IDC_DETACHEDSIG,	IDH_PGPCDPHRASE_DETACHEDSIG,
    0,0 
}; 

//___________________________
//
// Secure memory allocation routines
//

VOID* 
secAlloc (PGPContextRef context, UINT uBytes) 
{
	return (PGPNewSecureData (context, uBytes, NULL));
}


VOID 
secFree (VOID* p) 
{
	if (p) {
		FillMemory (p, lstrlen (p), '\0');
		PGPFreeData (p);
	}
}

//	__________________
//
//	Wipe edit box clean

VOID 
WipeEditBox (
		HWND hDlg, 
		UINT uID) 
{
	CHAR*	p;
	INT		i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = pcAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			pcFree (p);
		}
	}
}

//	____________________________________________________________
//
//  Check if message is dangerous to pass to passphrase edit box

BOOL 
CommonPhraseMsgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
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

	case WM_LBUTTONDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) return TRUE;
		break;

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

//	______________________________________
//
//  Passphrase edit box subclass procedure

LRESULT APIENTRY 
PhraseSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	UINT				uQuality;
	CHAR				szBuf[256];
	LRESULT				lResult;

	if (CommonPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;

	switch (uMsg) {
	case WM_GETTEXT :
		if (!pszPassPhrase) return 0;
		lParam = (LPARAM)pszPassPhrase;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_HIDETYPING));
			else 
				SetFocus (GetDlgItem (GetParent (hWnd), iNextTabControl));
		}
		else {
			if (hwndQuality) {
				lResult = CallWindowProc (wpOrigPhraseProc, 
						hWnd, uMsg, wParam, lParam); 
				CallWindowProc (wpOrigPhraseProc, 
						hWnd, WM_GETTEXT, sizeof(szBuf), (LPARAM)szBuf); 
				uQuality = PGPcomdlgPassphraseQuality (szBuf);
				memset (szBuf, 0, sizeof(szBuf));
				SendMessage (hwndQuality, PBM_SETPOS, uQuality, 0);
				return lResult;
			}
		}
		break;
	}
    return CallWindowProc (wpOrigPhraseProc, hWnd, uMsg, wParam, lParam); 
} 

//	______________________________________
//
//  Passphrase edit box subclass procedure

LRESULT APIENTRY 
PhraseConfirmSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	if (CommonPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;

	switch (uMsg) {
	case WM_GETTEXT :
		if (!pszPassPhraseConf) return 0;
		lParam = (LPARAM)pszPassPhraseConf;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_PHRASECONV));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDOK));
		}
		break;
	}
    return CallWindowProc(wpOrigPhraseProc, hWnd, uMsg, wParam, lParam); 
} 


//	____________________________
//
//  search keys for matching phrase

BOOL 
FindSigningKeyForThisPhrase (PPHRASESTRUCT pps, 
					  LPSTR pszPhrase, 
					  PGPKeyRef* matchingkey) 
{
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPKeyRef		key;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;
	BOOL			bFoundMatchingKey;

	PGPOrderKeySet (pps->keysetDisplay, kPGPAnyOrdering, &keylist);
	PGPNewKeyIter (keylist, &keyiter);

	bFoundMatchingKey = FALSE;
	if (matchingkey) *matchingkey = NULL;

	PGPKeyIterNext (keyiter, &key);
	while (PGPRefIsValid (key) && !bFoundMatchingKey) {
		PGPGetKeyBoolean (key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			PGPGetKeyBoolean (key, kPGPKeyPropIsRevoked, &bRevoked);
			PGPGetKeyBoolean (key, kPGPKeyPropIsExpired, &bExpired);
			PGPGetKeyBoolean (key, kPGPKeyPropCanSign, &bCanSign);
			if (!bRevoked && !bExpired && bCanSign) {
				if (PGPPassphraseIsValid (key, pszPhrase)) {
					bFoundMatchingKey = TRUE;
					if (matchingkey) *matchingkey = key;
				}
			}
		}
		PGPKeyIterNext (keyiter, &key);
	}
	PGPFreeKeyIter (keyiter);
	PGPFreeKeyList (keylist);

	return (bFoundMatchingKey);
}

//	____________________________
//
//  search keys for matching phrase

BOOL 
FindDecryptionKeyForThisPhrase (PPHRASESTRUCT pps, 
					  LPSTR pszPhrase, 
					  PGPKeyRef* matchingkey) 
{
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPKeyRef		key;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanEncrypt;
	BOOL			bFoundMatchingKey;

	PGPOrderKeySet (pps->keysetDisplay, kPGPAnyOrdering, &keylist);
	PGPNewKeyIter (keylist, &keyiter);

	bFoundMatchingKey = FALSE;
	if (matchingkey) *matchingkey = NULL;

	PGPKeyIterNext (keyiter, &key);
	while (PGPRefIsValid (key) && !bFoundMatchingKey) {
		PGPGetKeyBoolean (key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			PGPGetKeyBoolean (key, kPGPKeyPropIsRevoked, &bRevoked);
			PGPGetKeyBoolean (key, kPGPKeyPropIsExpired, &bExpired);
			PGPGetKeyBoolean (key, kPGPKeyPropCanEncrypt, &bCanEncrypt);
			if (!bRevoked && !bExpired && bCanEncrypt) {
				if (PGPPassphraseIsValid (key, pszPhrase)) {
					bFoundMatchingKey = TRUE;
					if (matchingkey) *matchingkey = key;
				}
			}
		}
		PGPKeyIterNext (keyiter, &key);
	}
	PGPFreeKeyIter (keyiter);
	PGPFreeKeyList (keylist);

	return (bFoundMatchingKey);
}

//	____________________________
//
//  setup key display list box

PGPError 
ValidateSigningPhrase (HWND hDlg, 
				PPHRASESTRUCT pps, 
				LPSTR pszPhrase, 
				PGPKeyRef key) 
{
	CHAR	szName[kPGPMaxUserIDSize];
	CHAR	sz[128];
	CHAR	sz2[kPGPMaxUserIDSize + 128];
	PGPSize	size;

	// does phrase match selected key ?
	if (PGPPassphraseIsValid (key, pszPhrase)) {
		*(pps->pKeySelected) = key;
		return kPGPError_NoErr;
	}

	// does phrase match any private key ?
	if (FindSigningKeyForThisPhrase (pps, pszPhrase, &key)) {
		// ask user to use other key
		LoadString (g_hInst, IDS_FOUNDMATCHFORPHRASE, sz, sizeof(sz));
		PGPGetPrimaryUserIDNameBuffer (key, sizeof(szName), szName, &size);
		wsprintf (sz2, sz, szName);
		LoadString (g_hInst, IDS_PGP, sz, sizeof(sz));
		if (MessageBox (hDlg, sz2, sz, MB_ICONQUESTION|MB_YESNO) == IDYES) {
			*(pps->pKeySelected) = key;
			return kPGPError_NoErr;
		}
	}

	// phrase doesn't match any key
	else {
		WipeEditBox (hDlg, IDC_PHRASE);
		SetDlgItemText (hDlg, IDC_PHRASE, "");
		PGPcomdlgErrorMessage (kPGPError_BadPassphrase);
		LoadString (g_hInst, IDS_WRONGPHRASE, sz, sizeof (sz));
		SetDlgItemText (hDlg, IDC_PROMPTSTRING, sz);
		SetFocus (GetDlgItem (hDlg, IDC_PHRASE));
	}

	return kPGPError_BadPassphrase;

}


//	____________________________
//
//  setup key display list box

BOOL 
InitEncryptedToKeyListBox (HWND hDlg, PPHRASESTRUCT pps) 
{
	BOOL			bAtLeastOneValidSecretKey	= FALSE;
	UINT			uUnknownKeys				= 0;
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPSubKeyRef	SubKey;
	UINT			u, uIndex, uKeyBits, uAlgorithm;
	INT				iKeyDefault, iKeySelected;
	PGPBoolean		bSecret, bCanEncrypt;
	CHAR			szName[kPGPMaxUserIDSize];
	CHAR			sz[128];
	CHAR			sz2[128];
	PGPError		err;

	if (pps->keysetDisplay) {
		PGPOrderKeySet (pps->keysetDisplay, kPGPValidityOrdering, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);

		iKeySelected = -1;
		iKeyDefault = -1;

		PGPKeyIterNext (KeyIter, &Key);
		while (Key) {
			PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
			PGPGetKeyBoolean (Key, kPGPKeyPropCanEncrypt, &bCanEncrypt);
			PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlgorithm);
			if (bSecret && bCanEncrypt) bAtLeastOneValidSecretKey = TRUE;

			// get name on key
			PGPGetPrimaryUserIDNameBuffer (Key, sizeof(szName), 
														szName, &u);
			if (u > MAXDECRYPTIONNAMECHAR) {
				u = MAXDECRYPTIONNAMECHAR;
				lstrcat (szName, "...");
			}
			else szName[u] = '\0';

			// append key type / size info to name
			lstrcat (szName, " (");
			switch (uAlgorithm) {
			case kPGPPublicKeyAlgorithm_RSA :
				LoadString (g_hInst, IDS_RSA, sz, sizeof(sz));
				lstrcat (szName, sz);
				lstrcat (szName, "/");
				PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
				wsprintf (sz, "%i", uKeyBits);
				lstrcat (szName, sz);
				break;

			case kPGPPublicKeyAlgorithm_DSA :
				LoadString (g_hInst, IDS_DH, sz, sizeof(sz));
				lstrcat (szName, sz);
				lstrcat (szName, "/");
				PGPKeyIterNextSubKey (KeyIter, &SubKey);
				if (SubKey) {
					PGPGetSubKeyNumber (SubKey, kPGPKeyPropBits, 
										&uKeyBits);
					wsprintf (sz, "%i", uKeyBits);
					lstrcat (szName, sz);
				}
				else lstrcat (szName, "???");
				break;

			default :
				LoadString (g_hInst, IDS_UNKNOWN, sz, sizeof(sz));
				lstrcat (szName, sz);
				lstrcat (szName, "/");
				lstrcat (szName, sz);
				break;
			}
			lstrcat (szName, ")");

			uIndex = SendDlgItemMessage (hDlg, IDC_KEYLISTBOX, 
								LB_ADDSTRING, 0, (LPARAM)szName);
			PGPKeyIterNext (KeyIter, &Key);
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);

		// Look for unknowns in the KeyID array
		for (u=0; u<pps->uKeyCountDisplay; u++) {

			err = PGPGetKeyByKeyID (pps->keysetDisplay, 
									&(pps->pKeyIDsDisplay[u]),
									kPGPPublicKeyAlgorithm_Invalid, 
									&Key);
			
			if (!PGPRefIsValid (Key) || IsPGPError (err)) {

				err = PGPGetKeyByKeyID (pps->keysetDisplay, 
										&(pps->pKeyIDsDisplay[u]),
										kPGPPublicKeyAlgorithm_ElGamal, 
										&Key);
	
				if (!PGPRefIsValid (Key) || IsPGPError (err)) {
					uUnknownKeys++;
				}
			}
		}
	}

	// no known keys
	else {
		uUnknownKeys = pps->uKeyCountDisplay;
	}

	if (uUnknownKeys) {
		LoadString (g_hInst, IDS_UNKNOWNKEY, sz, sizeof(sz));
		wsprintf (sz2, sz, uUnknownKeys);
		SendDlgItemMessage (hDlg, IDC_KEYLISTBOX, LB_INSERTSTRING, -1, 
								(LPARAM)sz2);
	}


	return bAtLeastOneValidSecretKey;
}

//	________________________
//
//  Dialog Message procedure

BOOL CALLBACK 
GetPhraseDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PPHRASESTRUCT	pps;
	CHAR			szDummy[4];
	INT				i;

	switch (uMsg) {

	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pps = (PPHRASESTRUCT)lParam;

		iNextTabControl = IDOK;

		SetDlgItemText (hDlg, IDC_PROMPTSTRING, pps->pszPromptString);

		if (pps->bSearchKeys) {
			if (!InitEncryptedToKeyListBox (hDlg, pps)) {
				EnableWindow (GetDlgItem (hDlg, IDC_PHRASE), FALSE);
				ShowWindow (GetDlgItem (hDlg, IDC_PHRASE), SW_HIDE);
				EnableWindow (GetDlgItem (hDlg, IDC_HIDETYPING), FALSE);
				ShowWindow (GetDlgItem (hDlg, IDC_HIDETYPING), SW_HIDE);
				EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
				ShowWindow (GetDlgItem (hDlg, IDC_PROMPTSTRING), SW_HIDE);
				ShowWindow (GetDlgItem (hDlg, IDC_CANNOTDECRYPTTEXT),SW_SHOW);
			}
		}

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
		break;

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
		pps = (PPHRASESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		switch(LOWORD (wParam)) {
			case IDOK: 
				if (pszPassPhrase) {
					secFree (pszPassPhrase);
					pszPassPhrase = NULL;
				}
				i = SendDlgItemMessage (hDlg, IDC_PHRASE, 
										WM_GETTEXTLENGTH, 0, 0) +1;
				pszPassPhrase = secAlloc (pps->context, i);
				if (pszPassPhrase) {
					GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
					if ((!pps->bSearchKeys) ||
						(FindDecryptionKeyForThisPhrase (pps, 
											pszPassPhrase, NULL))) {
						EndDialog (hDlg, kPGPError_NoErr);
					}
					else {
						CHAR sz[256];
						if (pszPassPhrase) {
							secFree (pszPassPhrase);
							pszPassPhrase = NULL;
						}
						WipeEditBox (hDlg, IDC_PHRASE);
						SetDlgItemText (hDlg, IDC_PHRASE, "");
						PGPcomdlgMessageBox (hDlg, IDS_CAPTION, 
							IDS_BADDECRYPTPHRASE, MB_OK|MB_ICONEXCLAMATION);
						LoadString (g_hInst, IDS_WRONGDECRYPTPHRASE, 
														sz, sizeof (sz));
						SetDlgItemText (hDlg, IDC_PROMPTSTRING, sz);
						SetFocus (GetDlgItem (hDlg, IDC_PHRASE));
					}
				}
				else 
					EndDialog (hDlg, kPGPError_OutOfMemory);
				break;

			case IDCANCEL:
				EndDialog (hDlg, kPGPError_UserAbort);
				break;

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

//	________________________
//
//  Dialog Message procedure

BOOL CALLBACK 
GetPhraseEncryptDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PPHRASESTRUCT	pps;
	CHAR			szDummy[4];
	INT				i;

	switch (uMsg) {

	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pps = (PPHRASESTRUCT)lParam;

		iNextTabControl = IDC_PHRASE2;
		hwndQuality = GetDlgItem (hDlg, IDC_PHRASEQUALITY);

		SetDlgItemText (hDlg, IDC_PROMPTSTRING, pps->pszPromptString);

		bHideText = TRUE;
		CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

		wpOrigPhraseProc = (WNDPROC) SetWindowLong (
									GetDlgItem (hDlg, IDC_PHRASECONV), 
									GWL_WNDPROC, 
									(LONG) PhraseSubclassProc); 
		wpOrigPhraseConfProc = (WNDPROC) SetWindowLong (
									GetDlgItem (hDlg, IDC_PHRASE2), 
									GWL_WNDPROC, 
									(LONG) PhraseConfirmSubclassProc); 
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
        SetWindowLong (GetDlgItem(hDlg, IDC_PHRASECONV), 
					   GWL_WNDPROC, 
					   (LONG)wpOrigPhraseProc); 
        SetWindowLong (GetDlgItem(hDlg, IDC_PHRASE2), 
					   GWL_WNDPROC, 
					   (LONG)wpOrigPhraseConfProc); 
		WipeEditBox (hDlg, IDC_PHRASECONV);
		WipeEditBox (hDlg, IDC_PHRASE2);
		break;

	case WM_CTLCOLOREDIT:
		if (((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASECONV)) ||
			((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASE2))) {
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
		pps = (PPHRASESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		switch(LOWORD (wParam)) {
			case IDOK: 
				if (pszPassPhrase) {
					secFree (pszPassPhrase);
					pszPassPhrase = NULL;
				}

				// get passphrase
				i = SendDlgItemMessage (hDlg, IDC_PHRASECONV, 
										WM_GETTEXTLENGTH, 0, 0) +1;

				// check for NULL phrase
				if (i <= 1) {
					PGPcomdlgMessageBox (hDlg, IDS_CAPTION, IDS_EMPTYPHRASE,
						MB_OK|MB_ICONEXCLAMATION);
					break;
				}

				pszPassPhrase = secAlloc (pps->context, i);
				if (pszPassPhrase) {
					GetDlgItemText (hDlg, IDC_PHRASECONV, szDummy, i);
					// get passphrase confirmation
					i = SendDlgItemMessage (hDlg, IDC_PHRASE2, 
										WM_GETTEXTLENGTH, 0, 0) +1;
					pszPassPhraseConf = secAlloc (pps->context, i);
					if (pszPassPhraseConf) {
						GetDlgItemText (hDlg, IDC_PHRASE2, szDummy, i);
					}
					else {
						secFree (pszPassPhrase);
						pszPassPhrase = NULL;
						EndDialog (hDlg, kPGPError_OutOfMemory);
						break;
					}
				}
				else {
					EndDialog (hDlg, kPGPError_OutOfMemory);
					break;
				}

				// zero out phrases
				WipeEditBox (hDlg, IDC_PHRASECONV);
				WipeEditBox (hDlg, IDC_PHRASE2);
				SetDlgItemText (hDlg, IDC_PHRASECONV, "");
				SetDlgItemText (hDlg, IDC_PHRASE2, "");

				// compare phrases
				if (lstrcmp (pszPassPhrase, pszPassPhraseConf)) {
					PGPcomdlgMessageBox (hDlg, IDS_CAPTION, 
						IDS_PHRASEMISMATCH, MB_OK|MB_ICONEXCLAMATION);
					secFree (pszPassPhrase);
					pszPassPhrase = NULL;
					secFree (pszPassPhraseConf);
					pszPassPhraseConf = NULL;
					SendMessage (hwndQuality, PBM_SETPOS, 0, 0);
					SetFocus (GetDlgItem (hDlg, IDC_PHRASECONV));
					break;
				}

				secFree (pszPassPhraseConf);
				pszPassPhraseConf = NULL;

				EndDialog (hDlg, kPGPError_NoErr);
				break;

			case IDCANCEL:
				EndDialog (hDlg, kPGPError_UserAbort);
				break;

			case IDC_HIDETYPING :
				if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
							== BST_CHECKED) 
					bHideText = TRUE;
				else 
					bHideText = FALSE;

				InvalidateRect (GetDlgItem (hDlg,IDC_PHRASECONV), NULL, TRUE);
				InvalidateRect (GetDlgItem (hDlg,IDC_PHRASE2), NULL, TRUE);
				break;
		}
		return TRUE;
	}
	return FALSE;
}

//	____________________________
//
//  Truncate text string

VOID
TruncateKeyText (HDC	hdc, 
				 LPSTR	pszOrig, 
				 LPSTR	szInfo, 
				 INT	iXmax, 
				 LPSTR	pszTrunc) 
{
	SIZE	s;
	INT		l, iW;

	GetTextExtentPoint32 (hdc, szInfo, lstrlen (szInfo), &s);
	iXmax -= s.cx;

	if (iXmax <= 0) {
		lstrcpy (pszTrunc, "");
		return;
	}

	lstrcpy (pszTrunc, pszOrig);
	GetTextExtentPoint32 (hdc, pszOrig, lstrlen (pszOrig), &s);
	iW = s.cx + 4;

	l = lstrlen (pszOrig);
	if (l < 3) {
		if (iW > iXmax) lstrcpy (pszTrunc, "");
	}
	else {
		l = lstrlen (pszOrig) - 3;
		while ((iW > iXmax) && (l >= 0)) {
			lstrcpy (&pszTrunc[l], "...");
			GetTextExtentPoint32 (hdc, pszTrunc, lstrlen (pszTrunc), &s);
			iW = s.cx + 4;
			l--;
		}	
		if (l < 0) lstrcpy (pszTrunc, "");
	}

	lstrcat (pszTrunc, szInfo);
}


//	____________________________
//
//  setup keyselection combo box

BOOL 
InitSigningKeyComboBox (HWND hDlg, PPHRASESTRUCT pps) 
{
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPKeyRef		KeyDefault;
	UINT			u, uIndex, uKeyBits, uAlgorithm;
	INT				iKeyDefault, iKeySelected;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;
	BOOL			bAtLeastOneSecretKey;
	CHAR			szName[kPGPMaxUserIDSize];
	CHAR			szNameFinal[kPGPMaxUserIDSize];
	CHAR			sz1[32], sz2[32];
	HDC				hdc;
	RECT			rc;
	INT				iComboWidth;
	HFONT			hFont;

	PGPOrderKeySet (pps->keysetDisplay, kPGPValidityOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);
	PGPGetDefaultPrivateKey (pps->keysetDisplay, &KeyDefault);

	SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, CB_GETDROPPEDCONTROLRECT,
						0, (LPARAM)&rc);
	iComboWidth = rc.right-rc.left - GetSystemMetrics (SM_CXVSCROLL);
	hdc = GetDC (GetDlgItem (hDlg, IDC_SIGNKEYCOMBO));
	hFont = GetStockObject (DEFAULT_GUI_FONT);
	SelectObject (hdc, hFont);

	iKeySelected = -1;
	iKeyDefault = 0;

	bAtLeastOneSecretKey = FALSE;

	PGPKeyIterNext (KeyIter, &Key);
	while (Key) {
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
			PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
			PGPGetKeyBoolean (Key, kPGPKeyPropCanSign, &bCanSign);
			PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &uAlgorithm);
			if (!bRevoked && !bExpired && bCanSign) {
				bAtLeastOneSecretKey = TRUE;

				// get key type / size info to append to name
				lstrcpy (sz2, "   (");
				switch (uAlgorithm) {
				case kPGPPublicKeyAlgorithm_RSA :
					LoadString (g_hInst, IDS_RSA, sz1, sizeof(sz1));
					lstrcat (sz2, sz1);
					lstrcat (sz2, "/");
					PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
					wsprintf (sz1, "%i", uKeyBits);
					lstrcat (sz2, sz1);
					break;

				case kPGPPublicKeyAlgorithm_DSA :
					LoadString (g_hInst, IDS_DSS, sz1, sizeof(sz1));
					lstrcat (sz2, sz1);
					lstrcat (sz2, "/");
					PGPGetKeyNumber (Key, kPGPKeyPropBits, &uKeyBits);
					wsprintf (sz1, "%i", uKeyBits);
					lstrcat (sz2, sz1);
					break;

				default :
					LoadString (g_hInst, IDS_UNKNOWN, sz1, sizeof(sz1));
					lstrcat (sz2, sz1);
					lstrcat (sz2, "/");
					lstrcat (sz2, sz1);
					break;
				}
				lstrcat (sz2, ")");

				// get name on key
				PGPGetPrimaryUserIDNameBuffer (Key, sizeof(szName), 
															szName, &u);
				TruncateKeyText (hdc, szName, sz2, iComboWidth, szNameFinal);

				uIndex = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, 
								CB_ADDSTRING, 0, (LPARAM)szNameFinal);
				if (uIndex != CB_ERR) {
					SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, 
								CB_SETITEMDATA, uIndex, (LPARAM)Key);
					if (*(pps->pKeySelected)) {
						if (Key == *(pps->pKeySelected)) 
							iKeySelected = uIndex;
					}
					if (Key == KeyDefault) iKeyDefault = uIndex;
				}
			}
		}
		PGPKeyIterNext (KeyIter, &Key);
	}
	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	if (iKeySelected == -1) iKeySelected = iKeyDefault;
	SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, CB_SETCURSEL, 
							iKeySelected, 0);

	ReleaseDC (GetDlgItem (hDlg, IDC_SIGNKEYCOMBO), hdc);

	return (bAtLeastOneSecretKey);

}

//	______________________________________________________________
//
//  Enable or disable passphrase edit box on basis of selected key

VOID 
EnablePassPhraseControl (HWND hDlg) 
{
	PGPKeyRef	Key;
	PGPBoolean	bNeedsPhrase;
	BOOL		bEnabled;
	INT			i;

	bEnabled = FALSE;
	i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, CB_GETCURSEL, 0, 0);
	if (i != CB_ERR) {					
		Key = (PGPKeyRef)SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, 
											CB_GETITEMDATA, i, 0);
		if (Key) {
			PGPGetKeyBoolean (Key, kPGPKeyPropNeedsPassphrase, 
								&bNeedsPhrase);
			if (bNeedsPhrase) bEnabled = TRUE;
		}
	}
	if (!bEnabled) SetDlgItemText (hDlg, IDC_PHRASE, "");
	EnableWindow (GetDlgItem (hDlg, IDC_PHRASE), bEnabled);
	EnableWindow (GetDlgItem (hDlg, IDC_HIDETYPING), bEnabled);
}

//	__________________________________________________________
//
//  Post warning about encrypting to RSA and signing w/DSA key

PGPError PGPcdExport 
PGPcomdlgRSADSAMixWarning (HWND hWnd) 
{
	INT i;

	i = PGPcomdlgMessageBox (hWnd, IDS_CAPTION, IDS_RSADSAMIXWARNING, 
		MB_YESNOCANCEL | MB_ICONEXCLAMATION);

	switch (i) {
	case IDYES : return kPGPError_NoErr;
	case IDNO : return kPGPError_Win32_NegativeResponse;
	default : return kPGPError_UserAbort;
	}

}

//	_________________________________________________________
//
//  Dialog Message procedure - get passphrase and signing key

BOOL CALLBACK 
GetPhraseKeyDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PPHRASESTRUCT	pps;
	CHAR			szDummy[4];
	INT				i;

	switch (uMsg) {

	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pps = (PPHRASESTRUCT)lParam;

		iNextTabControl = IDOK;
		SetDlgItemText (hDlg, IDC_PROMPTSTRING, pps->pszPromptString);

		bHideText = TRUE;
		CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

		wpOrigPhraseProc = (WNDPROC) SetWindowLong(
										GetDlgItem(hDlg, IDC_PHRASE), 
										GWL_WNDPROC, 
										(LONG) PhraseSubclassProc); 

		if (InitSigningKeyComboBox (hDlg, pps)) {
			EnablePassPhraseControl (hDlg);
		}
		else {
			EndDialog (hDlg, kPGPError_Win32_NoSecretKeys);
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
		break;

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
		pps = (PPHRASESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		switch (LOWORD (wParam)) {
			case IDOK: 
				if (pszPassPhrase) {
					secFree (pszPassPhrase);
					pszPassPhrase = NULL;
				}
				i = SendDlgItemMessage (hDlg, IDC_PHRASE, 
										WM_GETTEXTLENGTH, 0, 0) +1;
				pszPassPhrase = secAlloc (pps->context, i);
				if (!pszPassPhrase) {
					EndDialog (hDlg, kPGPError_OutOfMemory);
					break;
				}

				GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
				i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, 
										CB_GETCURSEL, 0, 0);
				if (i == CB_ERR) {
					PGPcomdlgMessageBox (hDlg, 
						IDS_ERRORMESSAGECAPTION, IDS_NOSIGNKEY, 
						MB_OK | MB_ICONEXCLAMATION);
				}
				else {
					PGPKeyRef	key;
					PGPError	err;

					key = (PGPKeyRef)SendDlgItemMessage (hDlg, 
								IDC_SIGNKEYCOMBO, CB_GETITEMDATA, i, 0);

					err = 
						ValidateSigningPhrase (hDlg, pps, pszPassPhrase, key);

					if (IsntPGPError (err)) {
						EndDialog (hDlg, kPGPError_NoErr);
					}
					else {
						if (pszPassPhrase) {
							secFree (pszPassPhrase);
							pszPassPhrase = NULL;
						}
					}
				}
				break;

			case IDC_SIGNKEYCOMBO :
				EnablePassPhraseControl (hDlg);
				break;

			case IDCANCEL:
				EndDialog (hDlg, kPGPError_UserAbort);
				break;

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

//	___________________________________________________________________
//
//  Dialog Message procedure - get passphrase, signing key, and options

BOOL CALLBACK 
GetPhraseOptionsDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PPHRASESTRUCT	pps;
	CHAR			szDummy[4];
	INT				i;

	switch (uMsg) {

	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pps = (PPHRASESTRUCT)lParam;

		iNextTabControl = IDC_ASCIIARMOR;
		SetDlgItemText (hDlg, IDC_PROMPTSTRING, pps->pszPromptString);

		bHideText = TRUE;
		CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

		if (*(pps->puSigningOptions) & PGPCOMDLG_ASCIIARMOR) 
			CheckDlgButton (hDlg, IDC_ASCIIARMOR, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_ASCIIARMOR, BST_UNCHECKED);

		if (*(pps->puSigningOptions) & PGPCOMDLG_DETACHEDSIG) 
			CheckDlgButton (hDlg, IDC_DETACHEDSIG, BST_CHECKED);
		else 
			CheckDlgButton (hDlg, IDC_DETACHEDSIG, BST_UNCHECKED);

		wpOrigPhraseProc = (WNDPROC) SetWindowLong(
										GetDlgItem(hDlg, IDC_PHRASE), 
										GWL_WNDPROC, 
										(LONG) PhraseSubclassProc); 

		if (InitSigningKeyComboBox (hDlg, pps)) {
			EnablePassPhraseControl (hDlg);
		}
		else {
			EndDialog (hDlg, kPGPError_Win32_NoSecretKeys);
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
		break;

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
		pps = (PPHRASESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		switch(LOWORD (wParam)) {
			case IDOK: 
				if (IsDlgButtonChecked (hDlg, IDC_ASCIIARMOR) 
						== BST_CHECKED)
					*(pps->puSigningOptions) |= PGPCOMDLG_ASCIIARMOR;
				else
					*(pps->puSigningOptions) &= ~PGPCOMDLG_ASCIIARMOR;

				if (IsDlgButtonChecked (hDlg, IDC_DETACHEDSIG) 
						== BST_CHECKED)
					*(pps->puSigningOptions) |= PGPCOMDLG_DETACHEDSIG;
				else
					*(pps->puSigningOptions) &= ~PGPCOMDLG_DETACHEDSIG;

				if (pszPassPhrase) {
					secFree (pszPassPhrase);
					pszPassPhrase = NULL;
				}
				i = SendDlgItemMessage (hDlg, IDC_PHRASE, 
										WM_GETTEXTLENGTH, 0, 0) +1;
				pszPassPhrase = secAlloc (pps->context, i);

				if (!pszPassPhrase) {
					EndDialog (hDlg, kPGPError_OutOfMemory);
					break;
				}

				GetDlgItemText (hDlg, IDC_PHRASE, szDummy, i);
				i = SendDlgItemMessage (hDlg, IDC_SIGNKEYCOMBO, 
										CB_GETCURSEL, 0, 0);
				if (i == CB_ERR) {
					PGPcomdlgMessageBox (hDlg, 
						IDS_ERRORMESSAGECAPTION, IDS_NOSIGNKEY, 
						MB_OK | MB_ICONEXCLAMATION);
				}
				else {
					PGPKeyRef	key;
					PGPError	err;

					key = (PGPKeyRef)SendDlgItemMessage (hDlg, 
								IDC_SIGNKEYCOMBO, CB_GETITEMDATA, i, 0);

					err = 
						ValidateSigningPhrase (hDlg, pps, pszPassPhrase, key);

					if (IsntPGPError (err)) {
						EndDialog (hDlg, kPGPError_NoErr);
					}
					else {
						if (pszPassPhrase) {
							secFree (pszPassPhrase);
							pszPassPhrase = NULL;
						}
					}
				}
				break;

			case IDC_SIGNKEYCOMBO :
				EnablePassPhraseControl (hDlg);
				break;

			case IDCANCEL:
				EndDialog (hDlg, kPGPError_UserAbort);
				break;

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

//	________________________________
//
//  Hook procedure for WH_MOUSE hook
  
LRESULT CALLBACK 
MouseHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
	MOUSEHOOKSTRUCT* pmhs;

    if (iCode >= 0) {
		pmhs = (MOUSEHOOKSTRUCT*) lParam; 
		PGPGlobalRandomPoolAddMouse (pmhs->pt.x, pmhs->pt.y);
	}

	return CallNextHookEx (hhookMouse, iCode, wParam, lParam);
} 

//	___________________________________
//
//  Hook procedure for WH_KEYBOARD hook 

 
LRESULT CALLBACK 
GetKBHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
    if (iCode >= 0) {
		PGPGlobalRandomPoolAddKeystroke (wParam);
	}

	return CallNextHookEx (hhookKeyboard, iCode, wParam, lParam);
} 
  
//	_______________________________________________________
//
//  Entry point called by app to post dialog and get phrase

PGPError PGPcdExport 
PGPcomdlgGetPhrase (
		PGPContextRef	context,
		HWND			hWndParent, 
		LPSTR			szPrompt,
		LPSTR*			ppszPhrase, 
		PGPKeySetRef	KeySet,
		PGPKeyID*		pKeyIDs,
		UINT			uKeyCount,
		PGPKeyRef*		pKey, 
		UINT*			puOptions, 
		UINT			uFlags) 
{
	PHRASESTRUCT	ps;
	DWORD			dwPhraseThreadID;
	PGPError		Retval;

	hwndQuality = NULL;
	pszPassPhrase = NULL;
	pszPassPhraseConf = NULL;

	if (szPrompt) {
		ps.pszPromptString = pcAlloc (lstrlen (szPrompt) +2);
		if (ps.pszPromptString) {
			if (!strchr (szPrompt, '&')) 
				lstrcpy (ps.pszPromptString, "&");
			else 
				ps.pszPromptString[0] = '\0';
			lstrcat (ps.pszPromptString, szPrompt);
		}
	}
	else ps.pszPromptString = NULL;

	ps.context = context;
	ps.keysetDisplay = KeySet;
	ps.pKeyIDsDisplay = pKeyIDs;
	ps.uKeyCountDisplay = uKeyCount;
	ps.pKeySelected = pKey;
	ps.puSigningOptions = puOptions;
	ps.bSearchKeys = FALSE;

	dwPhraseThreadID = GetCurrentThreadId ();
	hhookMouse = SetWindowsHookEx (WH_MOUSE, MouseHookProc, 
									NULL, GetCurrentThreadId());
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, GetKBHookProc, 
									NULL, dwPhraseThreadID);

	// if a decryption
	if (uFlags & PGPCOMDLG_DECRYPTION) {
		if (ps.keysetDisplay || ps.pKeyIDsDisplay) {
			ps.bSearchKeys = TRUE;
			Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASEDECRYPTKEYS), 
							hWndParent, GetPhraseDlgProc, (LPARAM)&ps);
		}
		else {
			Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASE), 
							hWndParent, GetPhraseDlgProc, (LPARAM)&ps);
		}
	}

	// if a conventional encryption procedure
	else if (uFlags & PGPCOMDLG_ENCRYPTION) {
		Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASEENCRYPT),
							hWndParent, GetPhraseEncryptDlgProc, (LPARAM)&ps);
	}

	// otherwise a signing procedure
	else {
		if (ps.keysetDisplay && ps.pKeySelected) {
			ps.bSearchKeys = TRUE;
			if (puOptions) {
				Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASEOPTIONS), 
							hWndParent, GetPhraseOptionsDlgProc, (LPARAM)&ps);
			}
			else {
				Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASEKEYS), 
							hWndParent, GetPhraseKeyDlgProc, (LPARAM)&ps);
			}
		}
		else {
			ps.keysetDisplay = NULL;
			pKeyIDs = NULL;
			Retval = DialogBoxParam (g_hInst, 
							MAKEINTRESOURCE (IDD_PASSPHRASE), 
							hWndParent, GetPhraseDlgProc, (LPARAM)&ps);
		}
	}

	if (Retval == kPGPError_NoErr) *ppszPhrase = pszPassPhrase;
	else *ppszPhrase = NULL;

	UnhookWindowsHookEx (hhookKeyboard);
	UnhookWindowsHookEx (hhookMouse);
	EnableWindow (hWndParent, TRUE);

	if (ps.pszPromptString) pcFree (ps.pszPromptString);

	return Retval;
}

//	________________________
//
//  wipe and free passphrase 

VOID PGPcdExport 
PGPcomdlgFreePhrase (LPSTR pszPhrase) 
{
	if (pszPhrase) {
		secFree (pszPhrase);
	}
}
