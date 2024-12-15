/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMChange.c - handle dialog for changing key passphrase
//
//	$Id: KMChange.c,v 1.5 1997/05/15 17:17:18 pbj Exp $
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

#include <windows.h>
#include <process.h>
#include "pgpkeys.h"
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndMain;
extern PGPKeySet* g_pKeySetMain;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static LPSTR szOldPhrase;
static LPSTR szNewPhrase1;
static LPSTR szNewPhrase2;
static LONG lOldPhraseLen;
static LONG lNewPhrase1Len;
static LONG lNewPhrase2Len;
static HWND hwndParent;
static BOOL bHideText;
static WNDPROC wpOrigOldPhraseProc;
static WNDPROC wpOrigNewPhrase1Proc;
static WNDPROC wpOrigNewPhrase2Proc;
static DWORD dwChangeThreadID;
static HHOOK hhookDebug;
static HHOOK hhookKeyboard;
static PGPKey* pChangeKey;
static BOOL bChangingPhrase = FALSE;

static DWORD aIds[] = {			// Help IDs
    IDC_HIDETYPING,		IDH_PGPKEYS_HIDETYPING,
    IDC_OLDPHRASE,		IDH_PGPKEYS_OLDPHRASE,
    IDC_NEWPHRASE1,		IDH_PGPKEYS_NEWPHRASE1,
    IDC_NEWPHRASE2,		IDH_PGPKEYS_NEWPHRASE2,
     0,0
};


//----------------------------------------------------|
//  Handles some common edit box messages
//  for which default handling presents security risks.
//  Returns TRUE if message presents potential security
//  risk and default handling should not be called.

BOOL CommonChangePhraseMsgProc (HWND hWnd, UINT uMsg,
								WPARAM wParam, LPARAM lParam) {
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

	case WM_ENABLE :
		if (!wParam) return TRUE;
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

	case WM_SETFOCUS :
		SendMessage (hWnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	}
    return FALSE;
}


//----------------------------------------------------|
//  Old passphrase edit box subclass procedure

LRESULT APIENTRY OldPhraseSubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lOldPhraseLen;
		lParam = (LPARAM)szOldPhrase;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_HIDETYPING));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE1));
		}
		break;
	}
    return CallWindowProc(wpOrigOldPhraseProc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  New passphrase 1 edit box subclass procedure

LRESULT APIENTRY NewPhrase1SubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lNewPhrase1Len;
		lParam = (LPARAM)szNewPhrase1;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_OLDPHRASE));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE2));
		}
		break;
	}
    return CallWindowProc (wpOrigNewPhrase1Proc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  New passphrase 2 edit box subclass procedure

LRESULT APIENTRY NewPhrase2SubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (CommonChangePhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lNewPhrase2Len;
		lParam = (LPARAM)szNewPhrase2;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_NEWPHRASE1));
			else SetFocus (GetDlgItem (GetParent (hWnd), IDOK));
		}
		break;
	}
    return CallWindowProc (wpOrigNewPhrase2Proc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  Change Passphrase of all subkeys

void KMChangeSubKeyPhrase (PGPKey* pKey, LPSTR szOld, LPSTR szNew) {
	UINT u;
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPSubKey* pSubKey;

	pgpGetKeyNumber (pChangeKey, kPGPKeyPropAlgId, &u);
	switch (u) {
	case PGP_PKALG_RSA :
		break;

	case PGP_PKALG_DSA :
		pKeyList = pgpOrderKeySet (g_pKeySetMain, kPGPAnyOrdering);
		pKeyIter = pgpNewKeyIter (pKeyList);
		pgpKeyIterSet (pKeyIter, pKey);
		pSubKey = pgpKeyIterNextSubKey (pKeyIter);
		while (pSubKey) {
			PGPcomdlgErrorMessage (pgpChangeSubKeyPassphrase (
											pSubKey, szOld, szNew));
			pSubKey = pgpKeyIterNextSubKey (pKeyIter);
		}
		pgpFreeKeyIter (pKeyIter);
		pgpFreeKeyList (pKeyList);
		break;

	default :
		break;
	}
}


//----------------------------------------------------|
//  Change Passphrase Dialog Message procedure

BOOL CALLBACK ChangeDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	CHAR szDummy[4];
	Boolean bNeedsPhrase;
	static HBRUSH hBrushBackground;

	switch (uMsg) {

		case WM_INITDIALOG:

			bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			pgpGetKeyBoolean (pChangeKey,
							   kPGPKeyPropNeedsPassphrase,
							  &bNeedsPhrase);
			if (bNeedsPhrase) EnableWindow (GetDlgItem (hDlg, IDC_OLDPHRASE),
											TRUE);

			hBrushBackground = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

			// subclass the three edit boxes
			wpOrigOldPhraseProc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_OLDPHRASE),
                  GWL_WNDPROC, (LONG) OldPhraseSubclassProc);
			wpOrigNewPhrase1Proc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_NEWPHRASE1),
                  GWL_WNDPROC, (LONG) NewPhrase1SubclassProc);
			wpOrigNewPhrase2Proc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_NEWPHRASE2),
                  GWL_WNDPROC, (LONG) NewPhrase2SubclassProc);

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
			// restore the window procedures of the subclassed edit boxes
             SetWindowLong(GetDlgItem(hDlg, IDC_OLDPHRASE),
				GWL_WNDPROC, (LONG)wpOrigOldPhraseProc);
             SetWindowLong(GetDlgItem(hDlg, IDC_NEWPHRASE1),
				GWL_WNDPROC, (LONG)wpOrigNewPhrase1Proc);
             SetWindowLong(GetDlgItem(hDlg, IDC_NEWPHRASE2),
				GWL_WNDPROC, (LONG)wpOrigNewPhrase2Proc);
			DeleteObject (hBrushBackground);
			KMWipeEditBox (hDlg, IDC_OLDPHRASE);
			KMWipeEditBox (hDlg, IDC_NEWPHRASE);
			KMWipeEditBox (hDlg, IDC_NEWPHRASE2);
			return TRUE;

		case WM_CTLCOLOREDIT:
			if (((HWND)lParam == GetDlgItem (hDlg, IDC_OLDPHRASE)) ||
				((HWND)lParam == GetDlgItem (hDlg, IDC_NEWPHRASE1)) ||
				((HWND)lParam == GetDlgItem (hDlg, IDC_NEWPHRASE2))) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideText)
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam,
									GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)hBrushBackground;
			}
			break;

		case WM_COMMAND:
			switch(LOWORD (wParam)) {
				case IDOK:
					// edit boxes are subclassed to always fill these buffers
					lNewPhrase1Len = SendDlgItemMessage (hDlg,
											IDC_NEWPHRASE1,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szNewPhrase1 = KMAlloc (lNewPhrase1Len);
					if (szNewPhrase1)
						GetDlgItemText (hDlg, IDC_NEWPHRASE1,
										szDummy, sizeof(szDummy));

					// edit boxes are subclassed to always fill these buffers
					lNewPhrase2Len = SendDlgItemMessage (hDlg,
											IDC_NEWPHRASE2,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szNewPhrase2 = KMAlloc (lNewPhrase2Len);
					if (szNewPhrase2)
						GetDlgItemText (hDlg, IDC_NEWPHRASE2,
										szDummy, sizeof(szDummy));

					if (strcmp (szNewPhrase1, szNewPhrase2)) {
						KMFree (szNewPhrase1);
						KMFree (szNewPhrase2);
						KMMessageBox (hDlg, IDS_CAPTIONKEYGEN,
							IDS_PASSPHRASEMISMATCH,
							MB_OK|MB_ICONEXCLAMATION);
						SetDlgItemText (hDlg, IDC_NEWPHRASE1, "");
						SetDlgItemText (hDlg, IDC_NEWPHRASE2, "");
						SetFocus (GetDlgItem (hDlg, IDC_NEWPHRASE1));
						return 0;
					}
					else {
						KMFree (szNewPhrase2);
					}

					if (strlen (szNewPhrase1) < 8) {
						if (!KMUseBadPassPhrase (hDlg)) {
							KMFree (szNewPhrase1);
							SetDlgItemText (hDlg, IDC_NEWPHRASE1, "");
							SetDlgItemText (hDlg, IDC_NEWPHRASE2, "");
							SetFocus (GetDlgItem (hDlg, IDC_NEWPHRASE1));
							return 0;
						}
					}

					// edit boxes are subclassed to always fill these buffers
					lOldPhraseLen = SendDlgItemMessage (hDlg, IDC_OLDPHRASE,
											WM_GETTEXTLENGTH, 0, 0) +1;
					szOldPhrase = KMAlloc (lOldPhraseLen);
					if (szOldPhrase)
						GetDlgItemText (hDlg, IDC_OLDPHRASE,
										szDummy, sizeof(szDummy));

					if (PGPcomdlgErrorMessage (pgpChangePassphrase (
							pChangeKey, szOldPhrase, szNewPhrase1))) {
						KMFree (szNewPhrase1);
						KMFree (szOldPhrase);
						SetDlgItemText (hDlg, IDC_OLDPHRASE, "");
						SetFocus (GetDlgItem (hDlg, IDC_OLDPHRASE));
					}
					else {
						KMChangeSubKeyPhrase (pChangeKey,
												szOldPhrase, szNewPhrase1);
						KMCommitKeyRingChanges (g_pKeySetMain);
						KMUpdateAllValidities ();
						KMFree (szNewPhrase1);
						KMFree (szOldPhrase);
						EndDialog (hDlg, 1);
					}
					return TRUE;

				case IDHELP :
					WinHelp (hDlg, g_szHelpFile, HELP_CONTEXT,
						IDH_PGPKEYS_CHANGEDIALOG);
					break;

				case IDCANCEL:
					EndDialog (hDlg, 0);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING) ==
								BST_CHECKED)
						bHideText = TRUE;
					else
						bHideText = FALSE;

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


//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook --
//  This is just a dummy routine to allow the
//  the Debug hook to trap this event.

LRESULT CALLBACK ChangeKBHookProc (INT iCode, WPARAM wParam, LPARAM lParam) {

	return CallNextHookEx (hhookKeyboard, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Hook procedure for WH_DEBUG hook -- this procedure is
//  called before another hook is about to be called.

LRESULT CALLBACK ChangeDebugHookProc (INT iCode, WPARAM wParam,
									   LPARAM lParam) {
//	DWORD dw;
//	MSG msg;

     if (iCode < 0)  // do not process message
         return CallNextHookEx (hhookDebug, iCode, wParam, lParam);

	// if not about to call a Keyboard hook then don't worry about it
//	if (wParam != WH_KEYBOARD)
//        return CallNextHookEx (hhookDebug, iCode, wParam, lParam);

	// if one of the passphrase edit boxes has the focus then send
	// the WM_CHAR message directly to the edit box window and do
	// not pass the WM_KEYUP/WM_KEYDOWN message along to the next hook.

	// else don't worry about it
	return CallNextHookEx (hhookDebug, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Dialog box thread procedure (used to thwart potential
//    passphrase copy by hook procedures)

void ChangePhraseThread (LPVOID lpVoid) {

	EnableWindow (g_hWndMain, FALSE);
	EnableWindow (hwndParent, FALSE);

	hhookKeyboard = SetWindowsHookEx (WH_JOURNALRECORD, ChangeKBHookProc,
									  NULL, dwChangeThreadID);
	hhookDebug = SetWindowsHookEx (WH_DEBUG, ChangeDebugHookProc,
								    NULL, dwChangeThreadID);

	DialogBox (g_hInst, MAKEINTRESOURCE (IDD_CHANGEPHRASE), hwndParent,
			    ChangeDlgProc);

	UnhookWindowsHookEx (hhookDebug);
	UnhookWindowsHookEx (hhookKeyboard);
	EnableWindow (hwndParent, TRUE);
	EnableWindow (g_hWndMain, TRUE);
	bChangingPhrase = FALSE;
}


//----------------------------------------------------|
//  Create thread to handle dialog box

void KMChangePhrase (HWND hWndParent, PGPKey* pKey) {

	if (!bChangingPhrase) {
		bChangingPhrase = TRUE;
		hwndParent = hWndParent;
		pChangeKey = pKey;
		_beginthreadex (NULL, 0, (LPTHREAD_START_ROUTINE)ChangePhraseThread,
			0, 0, &dwChangeThreadID);
	}
}
