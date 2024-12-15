/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMChange.c - handle dialog for changing key passphrase
	
	$Id: KMChange.c,v 1.19 1997/10/21 13:36:37 pbj Exp $

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

____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include <process.h>
#include "resource.h"
#include "pgpAdminPrefs.h"
#include "pgpBuildFlags.h"

#define DEFAULT_MINPHRASELEN	8
#define DEFAULT_MINPHRASEQUAL	60

// external globals
extern HINSTANCE	g_hInst;
extern CHAR			g_szHelpFile[MAX_PATH];

// local globals
typedef struct {
	PKEYMAN		pKM;
	LPSTR		szOldPhrase;
	LPSTR		szNewPhrase1;
	LPSTR		szNewPhrase2;
	LONG		lOldPhraseLen;
	LONG		lNewPhrase1Len;
	LONG		lNewPhrase2Len;
	WNDPROC		wpOrigOldPhraseProc;  
	WNDPROC		wpOrigNewPhrase1Proc;  
	WNDPROC		wpOrigNewPhrase2Proc;  
	HWND		hWndParent;
	BOOL		bHideText;
	PGPKeyRef	ChangeKey;
	BOOL		bMinPhraseQuality;
	INT			iMinPhraseQuality;
	BOOL		bMinPhraseLength;
	INT			iMinPhraseLength;
} CHANGEDLGSTRUCT;

static BOOL bChangingPhrase;
static HHOOK hHookDebug;
static HHOOK hHookKeyboard;

static DWORD aIds[] = {			// Help IDs
    IDC_HIDETYPING,			IDH_PGPKM_HIDETYPING, 
    IDC_OLDPHRASE,			IDH_PGPKM_OLDPHRASE, 
    IDC_NEWPHRASE1,			IDH_PGPKM_NEWPHRASE1, 
    IDC_NEWPHRASE2,			IDH_PGPKM_NEWPHRASE2, 
	IDC_PHRASEQUALITY,		IDH_PGPKM_PHRASEQUALITY,
	IDC_MINPHRASEQUALITY,	IDH_PGPKM_RECPHRASEQUALITY,
    0,0 
}; 

//	___________________________________________________
//
//  Handles some common edit box messages  
//  for which default handling presents security risks.
//  Returns TRUE if message presents potential security 
//  risk and default handling should not be called.

BOOL 
CommonChangePhraseMsgProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
 	CHANGEDLGSTRUCT*	pcds;

	switch (uMsg) {
	case WM_RBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
	case WM_CONTEXTMENU :
		return TRUE;

	case WM_LBUTTONDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) return TRUE;
		break;

	case WM_ENABLE :
		if (!wParam) return TRUE;
		break;

	case WM_PAINT :
		if (wParam) {
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (GetParent (hWnd), 
													GWL_USERDATA);
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (pcds->bHideText) 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		}
		break; 

	case WM_SETFOCUS :
		SendMessage (hWnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	}
    return FALSE; 
} 

//	___________________________________________________
//
//  Old passphrase edit box subclass procedure

LRESULT APIENTRY 
OldPhraseSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
 	CHANGEDLGSTRUCT*	pcds;

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	pcds = (CHANGEDLGSTRUCT*)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

	switch (uMsg) {
	case WM_GETTEXT :
		wParam = pcds->lOldPhraseLen;
		lParam = (LPARAM)pcds->szOldPhrase;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_HIDETYPING));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE1));
		}
		break;
	}
    return CallWindowProc (pcds->wpOrigOldPhraseProc, 
							hWnd, uMsg, wParam, lParam); 
} 

//	___________________________________________________
//
//  New passphrase 1 edit box subclass procedure

LRESULT APIENTRY 
NewPhrase1SubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
 	CHANGEDLGSTRUCT*	pcds;
	UINT				uQuality;
	CHAR				szBuf[256];
	LRESULT				lResult;

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	pcds = (CHANGEDLGSTRUCT*)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

	switch (uMsg) {
	case WM_GETTEXT :
		wParam = pcds->lNewPhrase1Len;
		lParam = (LPARAM)pcds->szNewPhrase1;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_OLDPHRASE));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE2));
		}
		else {
			lResult = CallWindowProc (pcds->wpOrigNewPhrase1Proc, 
						hWnd, uMsg, wParam, lParam); 
			CallWindowProc (pcds->wpOrigNewPhrase1Proc, 
						hWnd, WM_GETTEXT, sizeof(szBuf), (LPARAM)szBuf); 
			uQuality = PGPcomdlgPassphraseQuality (szBuf);
			memset (szBuf, 0, sizeof(szBuf));
			SendDlgItemMessage (GetParent (hWnd), IDC_PHRASEQUALITY, 
						PBM_SETPOS, uQuality, 0);
			return lResult;
		}
		break;
	}
    return CallWindowProc (pcds->wpOrigNewPhrase1Proc, 
						hWnd, uMsg, wParam, lParam); 
} 

//	___________________________________________________
//
//  New passphrase 2 edit box subclass procedure

LRESULT APIENTRY 
NewPhrase2SubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
 	CHANGEDLGSTRUCT*	pcds;

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	pcds = (CHANGEDLGSTRUCT*)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

	switch (uMsg) {
	case WM_GETTEXT :
		wParam = pcds->lNewPhrase2Len;
		lParam = (LPARAM)pcds->szNewPhrase2;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE1));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDOK));
		}
		break;
	}
    return CallWindowProc (pcds->wpOrigNewPhrase2Proc, 
							hWnd, uMsg, wParam, lParam); 
} 

//	___________________________________________________
//
//  Change Passphrase of all subkeys

VOID 
KMChangeSubKeyPhrase (
		PKEYMAN		pKM,
		PGPKeyRef	Key, 
		LPSTR		szOld, 
		LPSTR		szNew) 
{
	UINT			u;
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPSubKeyRef	SubKey;

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (pKM->KeySetMain, kPGPAnyOrdering, &KeyList);
		PGPNewKeyIter (KeyList, &KeyIter);
		PGPKeyIterSeek (KeyIter, Key);
		PGPKeyIterNextSubKey (KeyIter, &SubKey);
		while (SubKey) {
			PGPcomdlgErrorMessage (PGPChangeSubKeyPassphrase (
											SubKey, szOld, szNew));
			PGPKeyIterNextSubKey (KeyIter, &SubKey);
		}
		PGPFreeKeyIter (KeyIter);
		PGPFreeKeyList (KeyList);
		break;

	default :
		break;
	}
}

//	___________________________________________________
//
//  Verify that passphrase meets required criteria

BOOL CALLBACK 
VerifyPhrase (HWND hwnd, CHANGEDLGSTRUCT* pcds) 
{
	BOOL	bPhraseOK = TRUE;
	BOOL	bRejected = FALSE;
	CHAR	sz1[256], sz2[256], sz3[64];
	INT		iQuality, iLength;
	UINT	u;

	iLength = lstrlen (pcds->szNewPhrase1);
	iQuality = PGPcomdlgPassphraseQuality (pcds->szNewPhrase1);

	if ((iLength < pcds->iMinPhraseLength) && pcds->bMinPhraseLength)
		bRejected = TRUE;
	if ((iQuality < pcds->iMinPhraseQuality) && pcds->bMinPhraseQuality)
		bRejected = TRUE;

	if (iLength < pcds->iMinPhraseLength) {
		if (pcds->bMinPhraseLength) {
			LoadString (g_hInst, IDS_REQMINPHRASELEN, sz1, sizeof(sz1));
			wsprintf (sz2, sz1, pcds->iMinPhraseLength);
			LoadString (g_hInst, IDS_CAPTION, sz3, sizeof(sz3));
			MessageBox (hwnd, sz2, sz3, MB_OK|MB_ICONEXCLAMATION);
			bPhraseOK = FALSE;
		}
		else {
			if (!bRejected) {
				LoadString (g_hInst, IDS_SUGMINPHRASELEN, sz1, sizeof(sz1));
				wsprintf (sz2, sz1, pcds->iMinPhraseLength);
				LoadString (g_hInst, IDS_CAPTION, sz3, sizeof(sz3));
				u = MessageBox (hwnd, sz2, sz3, 
								MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2);
				if (u == IDNO) bPhraseOK = FALSE;
			}
		}
	}

	if(!bPhraseOK) return FALSE;

	if (iQuality < pcds->iMinPhraseQuality) {
		if (pcds->bMinPhraseQuality) {
			LoadString (g_hInst, IDS_REQMINPHRASEQUAL, sz1, sizeof(sz1));
			wsprintf (sz2, sz1, pcds->iMinPhraseQuality);
			LoadString (g_hInst, IDS_CAPTION, sz3, sizeof(sz3));
			MessageBox (hwnd, sz2, sz3, MB_OK|MB_ICONEXCLAMATION);
			bPhraseOK = FALSE;
		}
		else {
			LoadString (g_hInst, IDS_SUGMINPHRASEQUAL, sz1, sizeof(sz1));
			wsprintf (sz2, sz1, pcds->iMinPhraseQuality);
			LoadString (g_hInst, IDS_CAPTION, sz3, sizeof(sz3));
			u = MessageBox (hwnd, sz2, sz3, 
							MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2);
			if (u == IDNO) bPhraseOK = FALSE;
		}
	}

	return bPhraseOK;
}

//	___________________________________________________
//
//  Change Passphrase Dialog Message procedure

BOOL CALLBACK 
ChangeDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	CHANGEDLGSTRUCT*	pcds;
	CHAR				szDummy[4];
	Boolean				bNeedsPhrase;
	static HBRUSH		hBrushBackground;

	switch (uMsg) {

		case WM_INITDIALOG:
			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			pcds = (CHANGEDLGSTRUCT*)lParam;

			pcds->bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			PGPGetKeyBoolean (pcds->ChangeKey, 
							  kPGPKeyPropNeedsPassphrase, 
							  &bNeedsPhrase);
			if (bNeedsPhrase) EnableWindow (GetDlgItem (hDlg, IDC_OLDPHRASE),
											TRUE);

			hBrushBackground = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

			// initialize the quality progress bar
			if (pcds->bMinPhraseQuality) {
				SendDlgItemMessage (hDlg, IDC_MINPHRASEQUALITY, 
						PBM_SETRANGE, 0, MAKELPARAM (0, 100));
				SendDlgItemMessage (hDlg, IDC_MINPHRASEQUALITY, 
						PBM_SETPOS, pcds->iMinPhraseQuality, 0);
			}
			else {
				ShowWindow (GetDlgItem (hDlg, IDC_MINPHRASEQUALITY), SW_HIDE);
				ShowWindow (GetDlgItem (hDlg, IDC_MINQUALITYTEXT), SW_HIDE);
			}
			SendDlgItemMessage (hDlg, IDC_PHRASEQUALITY, PBM_SETRANGE, 0,
										MAKELPARAM (0, 100));
			SendDlgItemMessage (hDlg, IDC_PHRASEQUALITY, PBM_SETPOS, 0, 0);

			// subclass the three edit boxes
			pcds->wpOrigOldPhraseProc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_OLDPHRASE), 
                GWL_WNDPROC, (LONG) OldPhraseSubclassProc); 
			pcds->wpOrigNewPhrase1Proc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_NEWPHRASE1), 
                GWL_WNDPROC, (LONG) NewPhrase1SubclassProc); 
			pcds->wpOrigNewPhrase2Proc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_NEWPHRASE2), 
                GWL_WNDPROC, (LONG) NewPhrase2SubclassProc); 

			return TRUE;

	    case WM_HELP: 
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
	        WinHelp (((LPHELPINFO) lParam)->hItemHandle, 
				pcds->pKM->szHelpFile, 
	            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
	        break; 
 
		case WM_CONTEXTMENU: 
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		    WinHelp ((HWND) wParam, pcds->pKM->szHelpFile, HELP_CONTEXTMENU, 
		        (DWORD) (LPVOID) aIds); 
		    break; 

        case WM_DESTROY: 
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			// restore the window procedures of the subclassed edit boxes
            SetWindowLong(GetDlgItem(hDlg, IDC_OLDPHRASE), 
				GWL_WNDPROC, (LONG)pcds->wpOrigOldPhraseProc); 
            SetWindowLong(GetDlgItem(hDlg, IDC_NEWPHRASE1), 
				GWL_WNDPROC, (LONG)pcds->wpOrigNewPhrase1Proc); 
            SetWindowLong(GetDlgItem(hDlg, IDC_NEWPHRASE2), 
				GWL_WNDPROC, (LONG)pcds->wpOrigNewPhrase2Proc); 
			DeleteObject (hBrushBackground);
			KMWipeEditBox (hDlg, IDC_OLDPHRASE);
			KMWipeEditBox (hDlg, IDC_NEWPHRASE);
			KMWipeEditBox (hDlg, IDC_NEWPHRASE2);
			return TRUE;

		case WM_CTLCOLOREDIT:
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			if (((HWND)lParam == GetDlgItem (hDlg, IDC_OLDPHRASE)) ||
				((HWND)lParam == GetDlgItem (hDlg, IDC_NEWPHRASE1)) ||
				((HWND)lParam == GetDlgItem (hDlg, IDC_NEWPHRASE2))) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (pcds->bHideText) 
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else 
					SetTextColor ((HDC)wParam, 
									GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)hBrushBackground;
			}
			break;

		case WM_COMMAND:
			pcds = (CHANGEDLGSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			switch(LOWORD (wParam)) {
				case IDOK: 
					// edit boxes are subclassed to always fill these buffers
					pcds->lNewPhrase1Len = SendDlgItemMessage (hDlg, 
											IDC_NEWPHRASE1, 
											WM_GETTEXTLENGTH, 0, 0) +1;
					pcds->szNewPhrase1 = KMAlloc (pcds->lNewPhrase1Len);
					if (pcds->szNewPhrase1) 
						GetDlgItemText (hDlg, IDC_NEWPHRASE1, 
										szDummy, sizeof(szDummy));

					// edit boxes are subclassed to always fill these buffers
					pcds->lNewPhrase2Len = SendDlgItemMessage (hDlg, 
											IDC_NEWPHRASE2, 
											WM_GETTEXTLENGTH, 0, 0) +1;
					pcds->szNewPhrase2 = KMAlloc (pcds->lNewPhrase2Len);
					if (pcds->szNewPhrase2) 
						GetDlgItemText (hDlg, IDC_NEWPHRASE2, 
										szDummy, sizeof(szDummy));

					if (strcmp (pcds->szNewPhrase1, pcds->szNewPhrase2)) {
						KMFree (pcds->szNewPhrase1);
						KMFree (pcds->szNewPhrase2);
						KMMessageBox (hDlg, IDS_CAPTIONKEYGEN, 
							IDS_PASSPHRASEMISMATCH,
							MB_OK|MB_ICONEXCLAMATION);
						SetDlgItemText (hDlg, IDC_NEWPHRASE1, "");
						SetDlgItemText (hDlg, IDC_NEWPHRASE2, "");
						SetFocus (GetDlgItem (hDlg, IDC_NEWPHRASE1));
						return 0;
					}
					else {
						KMFree (pcds->szNewPhrase2);
					}

					if (!VerifyPhrase (hDlg, pcds)) {
						KMFree (pcds->szNewPhrase1);
						SetDlgItemText (hDlg, IDC_NEWPHRASE1, "");
						SetDlgItemText (hDlg, IDC_NEWPHRASE2, "");
						SetFocus (GetDlgItem (hDlg, IDC_NEWPHRASE1));
						return 0;
					}

					// edit boxes are subclassed to always fill these buffers
					pcds->lOldPhraseLen = SendDlgItemMessage (hDlg, 
											IDC_OLDPHRASE, 
											WM_GETTEXTLENGTH, 0, 0) +1;
					pcds->szOldPhrase = KMAlloc (pcds->lOldPhraseLen);
					if (pcds->szOldPhrase) 
						GetDlgItemText (hDlg, IDC_OLDPHRASE, 
										szDummy, sizeof(szDummy));

					if (PGPcomdlgErrorMessage (PGPChangePassphrase (
							pcds->ChangeKey, pcds->szOldPhrase, 
							pcds->szNewPhrase1))) {
						KMFree (pcds->szNewPhrase1);
						KMFree (pcds->szOldPhrase);
						SetDlgItemText (hDlg, IDC_OLDPHRASE, "");
						SetFocus (GetDlgItem (hDlg, IDC_OLDPHRASE));
					}
					else {
						KMChangeSubKeyPhrase (pcds->pKM, pcds->ChangeKey, 
									pcds->szOldPhrase, pcds->szNewPhrase1);
						KMCommitKeyRingChanges (pcds->pKM);
						KMUpdateAllValidities (pcds->pKM);
						KMFree (pcds->szNewPhrase1);
						KMFree (pcds->szOldPhrase);
						EndDialog (hDlg, 1);
					}
					return TRUE;

				case IDHELP :
					WinHelp (hDlg, pcds->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_CHANGEDIALOG); 
					break;

				case IDCANCEL:
					EndDialog (hDlg, 0);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING) == 
								BST_CHECKED) 
						pcds->bHideText = TRUE;
					else 
						pcds->bHideText = FALSE;

					InvalidateRect (GetDlgItem (hDlg, IDC_OLDPHRASE), 
									NULL, TRUE);
					InvalidateRect (GetDlgItem (hDlg, IDC_NEWPHRASE1), 
									NULL, TRUE);
					InvalidateRect (GetDlgItem (hDlg, IDC_NEWPHRASE2), 
									NULL, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Hook procedure for WH_KEYBOARD hook -- 
//  This is just a dummy routine to allow the
//  the Debug hook to trap this event.

LRESULT CALLBACK 
ChangeKBHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
	return CallNextHookEx (hHookKeyboard, iCode, wParam, lParam); 
} 
 
//	___________________________________________________
//
//  Hook procedure for WH_DEBUG hook -- this procedure is
//  called before another hook is about to be called.
 
LRESULT CALLBACK 
ChangeDebugHookProc (
		INT		iCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
//	DWORD dw;
//	MSG msg;
 
    if (iCode < 0)  // do not process message 
        return CallNextHookEx (hHookDebug, iCode, wParam, lParam); 

	// if not about to call a Keyboard hook then don't worry about it
//	if (wParam != WH_KEYBOARD) 
//        return CallNextHookEx (hhookDebug, iCode, wParam, lParam); 

	// if one of the passphrase edit boxes has the focus then send
	// the WM_CHAR message directly to the edit box window and do
	// not pass the WM_KEYUP/WM_KEYDOWN message along to the next hook.

	// else don't worry about it
	return CallNextHookEx (hHookDebug, iCode, wParam, lParam);
} 
 
//	___________________________________________________
//
//  Dialog box thread procedure (used to thwart potential 
//    passphrase copy by hook procedures)

VOID 
ChangePhraseThread (LPVOID lpv) 
{
	CHANGEDLGSTRUCT* pcds;
	DWORD dwThreadID;

	pcds = (CHANGEDLGSTRUCT*)lpv;

	EnableWindow (pcds->pKM->hWndParent, FALSE);
	EnableWindow (pcds->hWndParent, FALSE);

	dwThreadID = GetCurrentThreadId ();

	hHookKeyboard = SetWindowsHookEx (WH_JOURNALRECORD, 
										ChangeKBHookProc, 
										NULL, 
										dwThreadID);
	hHookDebug = SetWindowsHookEx (WH_DEBUG, 
										ChangeDebugHookProc, 
										NULL, 
										dwThreadID);

	DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_CHANGEPHRASE), 
				pcds->hWndParent, ChangeDlgProc, (LPARAM)pcds);

	UnhookWindowsHookEx (hHookDebug);
	UnhookWindowsHookEx (hHookKeyboard);
	EnableWindow (pcds->pKM->hWndParent, TRUE);
	EnableWindow (pcds->hWndParent, TRUE);
	SetForegroundWindow (pcds->hWndParent);
	KMFree (pcds);
	bChangingPhrase = FALSE;
}

//	___________________________________________________
//
//  load passphrase prefs 

BOOL 
InitPhrasePrefs (CHANGEDLGSTRUCT* pcds)
{
	BOOL			bPrefsCorrupt	= FALSE;
	PGPError		err;
#if PGP_BUSINESS_SECURITY
	PGPBoolean		b;
	PGPPrefRef		prefref;
	PGPUInt32		u;
#endif

#if PGP_BUSINESS_SECURITY
	err = PGPcomdlgOpenAdminPrefs (&prefref, FALSE);
#else
	err = kPGPError_NoErr;
#endif

	if (IsPGPError (err)) return FALSE;
	
	// minimum passphrase length
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (prefref, kPGPPrefEnforceMinChars, &b);
	pcds->bMinPhraseLength = b;
	if (b) {
		u = DEFAULT_MINPHRASELEN;
		err = PGPGetPrefNumber (prefref, kPGPPrefMinChars, &u);
		if (IsPGPError (err)) bPrefsCorrupt = TRUE;
		pcds->iMinPhraseLength = (INT)u;
	}
	else pcds->iMinPhraseLength = DEFAULT_MINPHRASELEN;
#else
	pcds->bMinPhraseLength = FALSE;
	pcds->iMinPhraseLength = DEFAULT_MINPHRASELEN;
#endif // PGP_BUSINESS_SECURITY

	// minimum passphrase quality
#if PGP_BUSINESS_SECURITY
	b = FALSE;
	PGPGetPrefBoolean (prefref, kPGPPrefEnforceMinQuality, &b);
	pcds->bMinPhraseQuality = b;
	if (b) {
		u = DEFAULT_MINPHRASEQUAL;
		err = PGPGetPrefNumber (prefref, kPGPPrefMinQuality, &u);
		if (IsPGPError (err)) bPrefsCorrupt = TRUE;
		pcds->iMinPhraseQuality = (INT)u;
	}
	else pcds->iMinPhraseQuality = 0;
#else
	pcds->bMinPhraseQuality = FALSE;
	pcds->iMinPhraseQuality = 0;
#endif // PGP_BUSINESS_SECURITY

	if (bPrefsCorrupt) return FALSE;

	return TRUE;
}

//	___________________________________________________
//
//  Create thread to handle dialog box 

VOID 
KMChangePhrase (
		PKEYMAN		pKM,
		HWND		hWndParent, 
		PGPKeyRef	Key) 
{
	CHANGEDLGSTRUCT* pcds;
	DWORD dw;

	if (!bChangingPhrase) {
		pcds = KMAlloc (sizeof (CHANGEDLGSTRUCT));
		if (pcds) {
			bChangingPhrase = TRUE;

			pcds->pKM = pKM;
			pcds->hWndParent = hWndParent;
			pcds->ChangeKey = Key;

			if (InitPhrasePrefs (pcds)) {			
				_beginthreadex (NULL, 0, 
					(LPTHREAD_START_ROUTINE)ChangePhraseThread, 
					pcds, 0, &dw);
			}
			else {
				KMFree (pcds);
			}
		}
	}
}
