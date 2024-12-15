/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLquery.c - selective import dialog
	

	$Id: CLselect.c,v 1.13 1999/01/28 22:23:55 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// PGP SDK header files
#include "pgpClientPrefs.h"

// system header files
#include <zmouse.h>

// External globals
extern HINSTANCE	g_hInst;
extern UINT			g_uMouseWheelMessage;

typedef struct {
	PGPContextRef		context;
	PGPtlsContextRef	tlsContext;
	LPSTR				pszPrompt;
	PGPKeySetRef		keysetToDisplay;
	PGPKeySetRef		keysetMain;
	PGPKeySetRef		keysetSelected;
	HKEYMAN				hKM;
	KMCONFIG			KMConfig;
	CHAR				szHelpFile[MAX_PATH];
	HWND				hwndTree;
	INT					iyOffset;
	INT					iyTop;
	UINT				uFlags;
} SELECTSTRUCT;

#define IDC_QUERYTREELIST		0x0101
#define BUTTONOFFSETY			10
#define DIALOGMINWIDTH			400

#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_SPACING			8
#define HOR_OFFSET				4

// local globals
static DWORD aIds[] = {			// Help IDs
	IDOK,				IDH_PGPKM_SELECT,
	IDC_SELECTALL,		IDH_PGPKM_SELECTSELALL,
	IDC_UNSELECTALL,	IDH_PGPKM_SELECTUNSELALL,
	IDC_QUERYTREELIST,	IDH_PGPKM_SELECTLIST,
	0,0 
}; 

//	___________________________________________________
//
//  Put up appropriate context menu on basis of selection flags.
//  Called in response to right mouse click.
//
//	hWnd		= handle of parent window
//	uSelFlags	= bits indicate what types of objects are selected
//	x, y		= mouse position when right button clicked (screen coords)

static VOID 
sQueryContextMenu (
		HWND	hWnd, 
		HKEYMAN hKM, 
		UINT	uSelFlags, 
		INT		x, 
		INT		y) 
{
	HMENU	hMC;
	HMENU	hMenuTrackPopup;
	PKEYMAN pKM = (PKEYMAN)hKM;

	switch (uSelFlags) {
	case PGPKM_KEYFLAG :
		hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUIMPORT));
		break;

	default :
		hMC = NULL;
		break;
	}

	if (!hMC) return;
  	hMenuTrackPopup = GetSubMenu (hMC, 0);
	if (KMMultipleSelected (pKM)) {
		EnableMenuItem (hMenuTrackPopup, IDM_KEYPROPERTIES, 
			MF_BYCOMMAND|MF_GRAYED);
	}

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					x, y, 0, hWnd, NULL);

	DestroyMenu (hMC);
}

//	____________________________________
//
//  get number of wheel scroll lines and pass to treelist controls

static VOID
sUpdateWheelScrollLines (SELECTSTRUCT* pss)
{
	UINT uLines;

	if (!SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &uLines, 0)) 
	{
		HWND hwnd = NULL;
		UINT umsg = 0;

		umsg = RegisterWindowMessage (MSH_SCROLL_LINES);
		hwnd = FindWindow (MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);

		if (hwnd && umsg) 
			uLines = (UINT)SendMessage (hwnd, umsg, 0, 0);
	}

	TreeList_SetWheelScrollLines (pss->hwndTree, uLines);
}

//	____________________________________
//
//	we're resizing -- move all the buttons 

static VOID
sMoveButtons (
		HWND	hwnd,
		WORD	wXsize,
		WORD	wYsize)
{
	HDWP	hdwp; 
	INT		iButtonWidth, iButtonHeight;
	RECT	rc;

	GetWindowRect (GetDlgItem (hwnd, IDOK), &rc);
	iButtonWidth = rc.right - rc.left;
	iButtonHeight = rc.bottom - rc.top;

	hdwp = BeginDeferWindowPos (4);

	// position the "cancel" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDCANCEL), NULL, 
		wXsize - HOR_OFFSET - iButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - iButtonHeight, 
		0, 0, SWP_NOSIZE|SWP_NOZORDER);

	// position the "OK" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDOK), NULL, 
		wXsize - HOR_OFFSET - BUTTON_SPACING - 2*iButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - iButtonHeight, 
		0, 0, SWP_NOSIZE|SWP_NOZORDER);

	// position the "unselectall" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_UNSELECTALL), NULL, 
		wXsize - HOR_OFFSET - 2*BUTTON_SPACING - 3*iButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - iButtonHeight, 
		0, 0, SWP_NOSIZE|SWP_NOZORDER);

	// position the "selectall" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_SELECTALL), NULL, 
		wXsize - HOR_OFFSET - 3*BUTTON_SPACING - 4*iButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - iButtonHeight, 
		0, 0, SWP_NOSIZE|SWP_NOZORDER);

	EndDeferWindowPos(hdwp);
}

//	___________________________________________________
//
//  Select keys dialog message procedure

static BOOL CALLBACK 
sSelectKeysDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	UINT			uFlags			= PGPKM_SHOWSELECTION;
	RECT			rc;
	SELECTSTRUCT*	pss;
	INT				iy;
	POINT			pt;
	HWND			hwndControl;
	PGPBoolean		b;
	PGPPrefRef		prefRef;
	HCURSOR			hCursorOld;
	PGPError		err;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pss = (SELECTSTRUCT*)lParam;

		// set window title
		hwndControl = GetDlgItem (hDlg, IDC_PROMPT);
		SetWindowText (hwndControl, pss->pszPrompt);

		// create keymanager window
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.bottom;
		ScreenToClient (hDlg, &pt);
		pss->iyTop = pt.y;

		hwndControl = GetDlgItem (hDlg, IDC_SELECTALL);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		iy = pt.y;
		GetClientRect (hDlg, &rc);
		pss->iyOffset = rc.bottom - (iy - BUTTONOFFSETY) + pss->iyTop;

		if (pss->uFlags & PGPCL_SINGLESELECTION)
		{
			ShowWindow (GetDlgItem (hDlg, IDC_SELECTALL), SW_HIDE);
			ShowWindow (GetDlgItem (hDlg, IDC_UNSELECTALL), SW_HIDE);
			uFlags |= PGPKM_SINGLESELECT;
		}

		pss->hKM = PGPkmCreateKeyManagerEx (pss->context, pss->tlsContext,
								hDlg, IDC_QUERYTREELIST, NULL, 0, pss->iyTop, 
								rc.right-rc.left, 
								rc.bottom-pss->iyOffset,
								uFlags);

		// configure key manager window
		pss->KMConfig.lpszHelpFile = NULL;
		pss->KMConfig.keyserver.structSize = 0;
		pss->KMConfig.hWndStatusBar = NULL;
		pss->KMConfig.ulDisableActions = KM_ALLACTIONS & 
				~(KM_SELECTALL|KM_UNSELECTALL|KM_PROPERTIES);
		pss->KMConfig.ulShowColumns = 0;
		pss->KMConfig.ulHideColumns = 0;
		pss->KMConfig.ulOptionFlags = 
			KMF_ONLYSELECTKEYS|KMF_READONLY|KMF_MODALPROPERTIES;

		// get those options which are based on prefs
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pss->context),
									&prefRef);
		PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, &b);
		if (!b) pss->KMConfig.ulOptionFlags |= KMF_NOVICEMODE;
		PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, &b);
		if (b) pss->KMConfig.ulOptionFlags |= KMF_MARGASINVALID;
		PGPclCloseClientPrefs (prefRef, FALSE);

		pss->KMConfig.ulMask = PGPKM_ALLITEMS;
		PGPkmConfigure (pss->hKM, &pss->KMConfig);
		
		// will be useful later to have HWND of keymanager
		pss->hwndTree = PGPkmGetManagerWindow (pss->hKM);

		// initialize keymanager
		PGPkmLoadKeySet (pss->hKM, pss->keysetToDisplay, pss->keysetMain);

		// initialize wheel scrolling
		sUpdateWheelScrollLines (pss);

		// position buttons
		GetClientRect (hDlg, &rc);
		sMoveButtons (hDlg, (WORD)rc.right, (WORD)rc.bottom);

		// no keys initially selected => disable "OK" button
		EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);

		// post message to sync keysets
		PostMessage (hDlg, WM_APP, 0, 0);

		return FALSE;

	case WM_APP :
		// this syncs the import keyset with the main one
		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPclSyncKeySets (pss->context, 
									pss->keysetMain, pss->keysetToDisplay);
		PGPkmLoadKeySet (pss->hKM, pss->keysetToDisplay, pss->keysetMain);
		SetCursor (hCursorOld);

		SetForegroundWindow (hDlg);
		SetFocus (pss->hwndTree);
		break;

	case WM_ACTIVATE :
		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		SetFocus (pss->hwndTree);
		break;

	case WM_SIZING :
		{
			LPRECT prc = (LPRECT)lParam;
			switch (wParam) {
			case WMSZ_TOPLEFT :
			case WMSZ_LEFT :
			case WMSZ_BOTTOMLEFT :
				if ((prc->right - prc->left) < DIALOGMINWIDTH)
					prc->left = prc->right - DIALOGMINWIDTH;
				break;

			case WMSZ_TOPRIGHT :
			case WMSZ_RIGHT :
			case WMSZ_BOTTOMRIGHT :
				if ((prc->right - prc->left) < DIALOGMINWIDTH)
					prc->right = prc->left + DIALOGMINWIDTH;
				break;
			}
		}
		break;

	case WM_SIZE :
		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		// resize keymanager window
		SetWindowPos (pss->hwndTree, NULL, 0, pss->iyTop, 
			LOWORD(lParam), HIWORD(lParam) - pss->iyOffset, 
			SWP_NOMOVE|SWP_NOZORDER);

		sMoveButtons (hDlg, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_SYSCOLORCHANGE :
		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		PostMessage (pss->hwndTree, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_SETTINGCHANGE :
		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		if (wParam == SPI_SETWHEELSCROLLLINES)
			sUpdateWheelScrollLines (pss);
		else 
			PostMessage (pss->hwndTree, WM_SYSCOLORCHANGE, 0, 0);
		break;

    case WM_HELP: 
 		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, pss->szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pss->szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aIds); 
        break; 

	case WM_NOTIFY :
		if (wParam == IDC_QUERYTREELIST) {
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			// let keymanager have first whack at the notification
			PGPkmDefaultNotificationProc (pss->hKM, lParam);

			switch (((LPNM_TREELIST)lParam)->hdr.code) {
			// post the context menu
			case TLN_CONTEXTMENU :
				sQueryContextMenu (hDlg, pss->hKM,
					((LPNM_TREELIST)lParam)->flags,
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			// enable or disable the "import" button depending on whether
			// item(s) are selected or not
			case TLN_SELCHANGED :
				if ((((LPNM_TREELIST)lParam)->itemNew.hItem) ||
					(((LPNM_TREELIST)lParam)->flags & TLC_MULTIPLE)) 
					EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
				else
					EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
				break;

			default :
				break;
			}
		}
		break;

	case KM_M_REQUESTSDKACCESS :
 		pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		PGPkmSynchronizeThreadAccessToSDK (pss->hKM);
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK :
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			err = PGPkmGetSelectedKeys (pss->hKM, &pss->keysetSelected);
			PGPkmDestroyKeyManager (pss->hKM, FALSE);
			KMFree (pss->hKM);
			pss->hwndTree = NULL;
			EndDialog (hDlg, err);
			break;

		case IDCANCEL :
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmDestroyKeyManager (pss->hKM, FALSE);
			KMFree (pss->hKM);
			pss->hwndTree = NULL;
			EndDialog (hDlg, kPGPError_UserAbort);
			break;

		case IDC_SELECTALL :
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pss->hKM, KM_SELECTALL); 
			EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
			SetFocus (pss->hwndTree);
			break;

		case IDC_UNSELECTALL :
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pss->hKM, KM_UNSELECTALL); 
			EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
			break;

		case IDM_KEYPROPERTIES :
			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pss->hKM, KM_PROPERTIES); 
			break;

		default :
			break;
		}
		return TRUE;

	default :
		if (uMsg == g_uMouseWheelMessage) {
			WORD iDelta;

			pss = (SELECTSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			iDelta = (WORD)wParam;
			
			SendMessage (pss->hwndTree, 
								WM_MOUSEWHEEL, MAKEWPARAM (0, iDelta), 0);
			return FALSE;
		}
		break;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Display and handle select keys dialog

PGPError PGPclExport 
PGPclSelectKeys (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		keysetToDisplay,			
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetSelected)
{
	SELECTSTRUCT	ss;
	CHAR			sz[64];
	PGPError		err;

	PGPValidatePtr (pkeysetSelected);

	ss.hKM = NULL;
	ss.context = context;
	ss.tlsContext = tlsContext;
	ss.pszPrompt = pszPrompt;
	ss.keysetToDisplay = keysetToDisplay;
	ss.keysetMain = keysetMain;
	ss.uFlags = 0;

	PGPclGetPGPPath (ss.szHelpFile, sizeof(ss.szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (ss.szHelpFile, sz);

	err = DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_QUERYADD), 
					hwndParent, sSelectKeysDlgProc, (LPARAM)&ss);

	if (IsntPGPError (err)) {
		*pkeysetSelected = ss.keysetSelected;
	}

	return err;
}

//	___________________________________________________
//
//  Display and handle select keys dialog

PGPError PGPclExport 
PGPclSelectKeysEx (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		LPSTR				pszPrompt,
		PGPKeySetRef		keysetToDisplay,			
		PGPKeySetRef		keysetMain,
		UINT				uFlags,
		PGPKeySetRef*		pkeysetSelected)
{
	SELECTSTRUCT	ss;
	CHAR			sz[64];
	PGPError		err;

	PGPValidatePtr (pkeysetSelected);

	ss.hKM = NULL;
	ss.context = context;
	ss.tlsContext = tlsContext;
	ss.pszPrompt = pszPrompt;
	ss.keysetToDisplay = keysetToDisplay;
	ss.keysetMain = keysetMain;
	ss.uFlags = uFlags;

	PGPclGetPGPPath (ss.szHelpFile, sizeof(ss.szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (ss.szHelpFile, sz);

	err = DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_QUERYADD), 
					hwndParent, sSelectKeysDlgProc, (LPARAM)&ss);

	if (IsntPGPError (err)) {
		*pkeysetSelected = ss.keysetSelected;
	}

	return err;
}

