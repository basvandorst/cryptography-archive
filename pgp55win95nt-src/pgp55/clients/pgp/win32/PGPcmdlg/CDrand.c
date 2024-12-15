/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPrand.c - post dialog to get entropy from mouse and keyboard
	

	$Id: CDrand.c,v 1.6 1997/09/05 18:01:17 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"
#include "pgpRandomPool.h"

#define TIMERID			4321L
#define DELAYTIME		2000L

// external globals
extern HINSTANCE g_hInst;
extern char g_szHelpFile[MAX_PATH];
extern HWND g_hWndEntropy;

// local globals
static HWND hWndRandDlg;
static LONG lTotalNeeded; 
static HHOOK hhookKeyboard;
static BOOL bCancelFocused;
static BOOL bCollectionEnabled;

//	___________________________________________
//
//  Get Key Randomness Dialog Message procedure

BOOL CALLBACK 
KeyRandDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	static LONG lOrigNeeded;
	static LONG lStillNeeded;
	LONG		l;
	CHAR		sz[256];

	switch (uMsg) {

	case WM_INITDIALOG:
		hWndRandDlg = hDlg;
		g_hWndEntropy = hDlg;

		lStillNeeded = lTotalNeeded - PGPGlobalRandomPoolGetEntropy ();

		if (lStillNeeded <= 0) EndDialog (hDlg, kPGPError_NoErr);
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

			PGPGlobalRandomPoolAddMouse (LOWORD (lParam), HIWORD (lParam));
			lStillNeeded = lTotalNeeded - PGPGlobalRandomPoolGetEntropy ();

			if (lStillNeeded > 0) {
				l = lOrigNeeded - lStillNeeded;
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, l, 0);
			}
			else {
				bCollectionEnabled = FALSE;
				LoadString (g_hInst, IDS_RANDOMDONE, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_TEXTPROMPT, sz); 
				SetTimer (hDlg, TIMERID, DELAYTIME, NULL);
			}
		}
		break;

	case WM_CHAR :
		if (bCollectionEnabled) {

			PGPGlobalRandomPoolAddKeystroke (wParam);
			lStillNeeded = lTotalNeeded - PGPGlobalRandomPoolGetEntropy ();

			if (lStillNeeded > 0) {
				l = lOrigNeeded - lStillNeeded;
				SendDlgItemMessage (hDlg, IDC_BITSPROGBAR, PBM_SETPOS, l, 0);
			}
			else {
				bCollectionEnabled = FALSE;
				LoadString (g_hInst, IDS_RANDOMDONE, sz, sizeof(sz));
				SetDlgItemText (hDlg, IDC_TEXTPROMPT, sz); 
				SetTimer (hDlg, TIMERID, DELAYTIME, NULL);
			}
		}
		break;

	case WM_TIMER :
		KillTimer (hDlg, TIMERID);
		EndDialog (hDlg, kPGPError_NoErr);
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
			LoadString (g_hInst, IDS_RANDOMPROMPT, sz, sizeof(sz));
			SetDlgItemText (hDlg, IDC_TEXTPROMPT, sz);
			bCollectionEnabled = TRUE;
			break;

		case IDCANCEL:
			EndDialog (hDlg, kPGPError_UserAbort);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	________________________________________________________
//
//  Hook procedure for WH_KEYBOARD hook -- 
//  This allows keystrokes to be used for entropy collection 
 
LRESULT CALLBACK 
KeyboardHookProc (
		INT		nCode, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{ 
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
 
//	______________________________________________
//
//  Entropy collection routine -- posts dialog for 
//  mouse/keyboard entropy collection.

PGPError PGPcdExport 
PGPcomdlgRandom (
		PGPContextRef	Context,
		HWND			hWnd, 
		UINT			uNeeded) 
{
	PGPError Retval;

	if (g_hWndEntropy) 
		return kPGPError_Win32_AlreadyOpen;

	g_hWndEntropy = (HWND)1;
	bCollectionEnabled = FALSE;

	// if zero passed in, use default
	if (uNeeded) lTotalNeeded = uNeeded;
	else lTotalNeeded = PGPGlobalRandomPoolGetMinimumEntropy ();

	// if we have enough already, then just return
	if (lTotalNeeded < (LONG)PGPGlobalRandomPoolGetEntropy ()) 
		return kPGPError_NoErr;

	// collect the entropy
	hhookKeyboard = SetWindowsHookEx (WH_KEYBOARD, KeyboardHookProc, 
										NULL, GetCurrentThreadId ());

	Retval = DialogBox (g_hInst, MAKEINTRESOURCE (IDD_KEYRAND), 
								hWnd, KeyRandDlgProc);

	UnhookWindowsHookEx (hhookKeyboard);

	g_hWndEntropy = NULL;

	return Retval;
}
