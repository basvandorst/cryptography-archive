/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//:KMPhrase - get passphrase from user
//	This code attempts to thwart some common situations
//	which could compromise the security of the passphrase.
//  NB: this code is susceptable to JournalRecord hooks
//  (which is probably the way someone would try to catch
//  your keystrokes anyway) so it is not very secure.
//
//	First, a separate thread is created to handle the dialog
//	in order to thwart hooks placed on the main thread.
//
//	Second, Keyboard and Debug hooks are installed to hook
//	keyboard messages and send them directly to the passphrase
//  	edit boxes to try to thwart previously-installed system
//  	hook procedures.
//
//	Third, the edit boxes are subclassed to inhibit the
//	WM_GETTEXT message from returning their text (they
//	place their text directly into the change_szbuf buffer,
//	instead) to thwart hooks from asking their text.
//  NB: the subclassing is also used to hide the text
//  as it is typed by setting the text color to the
//  background color.


#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "resource.h"


#define G_BUFLEN 512

// local globals
HWND hwndPhraseDlg;
HWND hwndParent;
BOOL bHideText;
WNDPROC wpOrigPhraseProc;
HHOOK hhookDebug;
HHOOK hhookKeyboard;
HWND hwndFocus;
int iPromptString;
char phrase_szbuf[G_BUFLEN];


//----------------------------------------------------|
//  Check if message is dangerous to pass to passphrase edit box

BOOL CommonPhraseMsgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
	case WM_RBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
	case WM_CONTEXTMENU:
		return TRUE;

	case WM_ENABLE:
		wParam = (WPARAM) TRUE;
		return TRUE;

	case WM_PAINT :
		SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
		if (bHideText) SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
		else SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		break;

	case WM_SETFOCUS :
		SendMessage (hwnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		hwndFocus = hwnd;
		break;

	case WM_KILLFOCUS :
		hwndFocus = NULL;
		break;
	}
    return FALSE;
}


//----------------------------------------------------|
//  Passphrase edit box subclass procedure

LRESULT APIENTRY PhraseSubclassProc (HWND hwnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (CommonPhraseMsgProc (hwnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = G_BUFLEN;
		lParam = (LPARAM)phrase_szbuf;
		break;

	case WM_CHAR :
		if (wParam == VK_TAB) {
			SetFocus (GetDlgItem (hwndPhraseDlg, IDC_HIDETYPING));
		}
		break;
	}
    return CallWindowProc(wpOrigPhraseProc, hwnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  Dialog Message procedure

BOOL CALLBACK GetPhraseDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam,
								LPARAM lParam)
{
	switch(iMsg) {

		case WM_INITDIALOG:
			hwndPhraseDlg = hDlg;
			hwndFocus = NULL;

			//LoadString (g_hInst, iPromptString, phrase_szbuf, G_BUFLEN);
			//SetDlgItemText (hDlg, IDC_PROMPTSTRING, phrase_szbuf);

			bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			wpOrigPhraseProc = (WNDPROC) SetWindowLong(
				GetDlgItem(hDlg, IDC_PHRASE),
                GWL_WNDPROC, (LONG) PhraseSubclassProc);

			return TRUE;

        case WM_DESTROY:
             SetWindowLong(GetDlgItem(hDlg, IDC_PHRASE), GWL_WNDPROC,
							(LONG)wpOrigPhraseProc);
			return TRUE;

		case WM_CTLCOLOREDIT:
			if ((HWND)lParam == GetDlgItem (hDlg, IDC_PHRASE)) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideText)
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			}
			break;

		case WM_COMMAND:
			switch(LOWORD (wParam)) {
				case IDOK:
					GetDlgItemText (hDlg, IDC_PHRASE, phrase_szbuf, G_BUFLEN);
					EndDialog (hDlg, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog (hDlg, 0);
					return TRUE;

				case IDC_HIDETYPING :
					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
								== BST_CHECKED)
						bHideText = TRUE;
					else bHideText = FALSE;

					InvalidateRect (GetDlgItem (hDlg, IDC_PHRASE), NULL, TRUE);
					break;
			}
			return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook --
//     used to inhibit any other hook procedures

LRESULT CALLBACK GetKBHookProc (int nCode, WPARAM wParam, LPARAM lParam) {

	return CallNextHookEx (hhookKeyboard, nCode, wParam, lParam);
}


//----------------------------------------------------|
//  Hook procedure for WH_DEBUG hook --
//     used to inhibit any other hook procedures

LRESULT CALLBACK GetDebugHookProc (int nCode, WPARAM wParam, LPARAM lParam) {
//	DWORD l;
//	MSG msg;

    if (nCode < 0)  /* do not process message */
         return CallNextHookEx (hhookDebug, nCode, wParam, lParam);

	if (wParam != 2)
         return CallNextHookEx (hhookDebug, nCode, wParam, lParam);

//>>>
// The following code has been removed because, under Windows NT, it causes
// double characters.  Apparently, either the debug hook returning 1 does not
// prevent further processing; or the WM_CHAR message is getting through from
// somewhere else.
//		
//	if (hwndFocus) {
//		l = ((DEBUGHOOKINFO*)lParam)->lParam;
//		if (l & 0x8000)
//			msg.message = WM_KEYUP;
//		else
//			msg.message = WM_KEYDOWN;
//		msg.hwnd = hwndFocus;
//		msg.wParam = ((DEBUGHOOKINFO*)lParam)->wParam;
//		msg.lParam = ((DEBUGHOOKINFO*)lParam)->lParam;
//		msg.time = 0;
//		msg.pt.x = 0;
//		msg.pt.y = 0;
//
//		TranslateMessage (&msg);
//		return 1;
//	}
//<<<

	return CallNextHookEx (hhookDebug, nCode, wParam, lParam);
}


//----------------------------------------------------|
//  Dialog box thread procedure (used to thwart potential
//    passphrase copy by hook procedures)

BOOL KMGetPhrase (HINSTANCE hInst,HWND hparent, int istring, LPSTR buffer,
					int buflen)
{
	DWORD PhraseThreadID;
	BOOL retval;

	iPromptString = istring;

	PhraseThreadID = GetCurrentThreadId ();
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, GetKBHookProc, NULL,
										PhraseThreadID);
	hhookDebug = SetWindowsHookEx (WH_DEBUG, GetDebugHookProc, NULL,
									PhraseThreadID);

	if (DialogBox (hInst, MAKEINTRESOURCE (IDD_PASSPHRASE), hparent,
					GetPhraseDlgProc))
	{
		if (lstrlen (phrase_szbuf) < buflen)
		{
			lstrcpy (buffer, phrase_szbuf);
			retval = TRUE;
		}
		else
		{
			lstrcpy (buffer, "");
			retval = FALSE;
		}
	}
	else
	{
		lstrcpy (buffer, "");
		retval = FALSE;
	}

	memset(phrase_szbuf, 0x00, sizeof(phrase_szbuf));

	UnhookWindowsHookEx (hhookDebug);
	UnhookWindowsHookEx (hhookKeyboard);
	EnableWindow (hwndParent, TRUE);

	return retval;
}
