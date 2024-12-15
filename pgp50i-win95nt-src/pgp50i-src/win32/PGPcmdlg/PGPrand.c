/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
//:PGPrand.c - get entropy from mouse and keyboard
//

#include "pgpcomdlgx.h"

#include "..\include\pgpRndPool.h"

#define TIMERID			4321L
#define DELAYTIME		2000L

// external globals
extern HINSTANCE g_hInst;
extern char g_szHelpFile[MAX_PATH];
extern HWND g_hWndEntropy;
extern char g_szbuf[G_BUFLEN];

// local globals
static HWND hWndRandDlg;
static LONG lTotalNeeded;
static HHOOK hhookKeyboard;
static BOOL bCancelFocused;
static BOOL bCollectionEnabled;


//----------------------------------------------------|
//  Get Key Randomness Dialog Message procedure

BOOL CALLBACK KeyRandDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam) {
	static LONG lOrigNeeded;
	static LONG lStillNeeded;
	LONG l;

	switch (uMsg) {

	case WM_INITDIALOG:
		hWndRandDlg = hDlg;
		g_hWndEntropy = hDlg;

		lStillNeeded = lTotalNeeded - pgpRandPoolEntropy ();
		if (lStillNeeded <= 0) EndDialog (hDlg, PGPCOMDLG_OK);
		else {
			lOrigNeeded = lStillNeeded;
			SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETRANGE,
									0, MAKELPARAM (0, lOrigNeeded));
			SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, 0, 0);
		}
		SetFocus (NULL);
		bCancelFocused = FALSE;
		return FALSE;

    case WM_DESTROY:
		hWndRandDlg = NULL;
         break;

	case WM_MOUSEMOVE :
		if (bCollectionEnabled) {
			pgpRandPoolMouse (LOWORD (lParam), HIWORD (lParam));
			lStillNeeded = lTotalNeeded - pgpRandPoolEntropy ();
			if (lStillNeeded > 0) {
				l = lOrigNeeded - lStillNeeded;
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, l, 0);
			}
			else {
				bCollectionEnabled = FALSE;
				LoadString (g_hInst, IDS_RANDOMDONE, g_szbuf, G_BUFLEN);
				SetDlgItemText (hDlg, IDC_TEXTPROMPT, g_szbuf);
				SetTimer (hDlg, TIMERID, DELAYTIME, NULL);
			}
		}
		break;

	case WM_CHAR :
		if (bCollectionEnabled) {
			pgpRandPoolKeystroke (wParam);
			lStillNeeded = lTotalNeeded - pgpRandPoolEntropy ();
			if (lStillNeeded > 0) {
				l = lOrigNeeded - lStillNeeded;
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, l, 0);
			}
			else {
				bCollectionEnabled = FALSE;
				LoadString (g_hInst, IDS_RANDOMDONE, g_szbuf, G_BUFLEN);
				SetDlgItemText (hDlg, IDC_TEXTPROMPT, g_szbuf);
				SetTimer (hDlg, TIMERID, DELAYTIME, NULL);
			}
		}
		break;

	case WM_TIMER :
		KillTimer (hDlg, TIMERID);
		EndDialog (hDlg, PGPCOMDLG_OK);
		break;

	case WM_COMMAND:
		switch(LOWORD (wParam)) {

		case IDC_BEGIN :
			EnableWindow (GetDlgItem (hDlg, IDC_BEGIN), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDCANCEL), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_STATIC1), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_STATIC2), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_STATIC3), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_TEXTPROMPT), TRUE);
			EnableWindow (GetDlgItem (hDlg, IDC_BITSPROGBAR), TRUE);
			LoadString (g_hInst, IDS_RANDOMPROMPT, g_szbuf, G_BUFLEN);
			SetDlgItemText (hDlg, IDC_TEXTPROMPT, g_szbuf);
			bCollectionEnabled = TRUE;
			break;

		case IDCANCEL:
			EndDialog (hDlg, PGPCOMDLG_CANCEL);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook --
//  This allows keystrokes to be used for entropy collection

LRESULT CALLBACK KeyboardHookProc (int nCode, WPARAM wParam, LPARAM lParam) {
	if (!(lParam & 0x80000000)) {
		if (bCancelFocused) {
			if ((wParam == VK_SPACE) || (wParam == VK_RETURN)) {
				wParam = VK_SPACE;
			}
			else {
				SetFocus (NULL);
				bCancelFocused = FALSE;
			}
		}
		else {
			if ((wParam == VK_TAB) || (wParam == VK_RETURN)) {
				SetFocus (GetDlgItem (hWndRandDlg, IDCANCEL));
				bCancelFocused = TRUE;
			}
			else SendMessage (hWndRandDlg, WM_CHAR, wParam, 0);
		}
	}
	if (bCancelFocused)
		return (CallNextHookEx (hhookKeyboard, nCode, wParam, lParam));
	else return 1;
}


//----------------------------------------------------|
//  Entropy collection routine -- posts dialog for
//  mouse/keyboard entropy collection.

UINT PGPExport PGPcomdlgRandom (HWND hWnd, UINT uNeeded) {
	UINT uRetval;

	if (g_hWndEntropy) return PGPCOMDLG_ALREADYOPEN;

	g_hWndEntropy = (HWND)1;
	bCollectionEnabled = FALSE;
	lTotalNeeded = uNeeded;

	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, KeyboardHookProc,
										NULL, GetCurrentThreadId ());

	if (!PGPcomdlgErrorMessage (pgpLibInit ())) {
		uRetval = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_KEYRAND),
								hWnd, KeyRandDlgProc);
		pgpLibCleanup ();
	}
	else uRetval = PGPCOMDLG_LIBERROR;

	UnhookWindowsHookEx (hhookKeyboard);

	g_hWndEntropy = NULL;

	return uRetval;
}
