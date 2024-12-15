/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLquery.c - selective import dialog
	

	$Id: CLquery.c,v 1.10 1998/08/11 14:43:12 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "pgpClientPrefs.h"

// system header files
#include <zmouse.h>

// External globals
extern HINSTANCE	g_hInst;
extern UINT			g_uMouseWheelMessage;

typedef struct {
	PGPContextRef		Context;
	PGPtlsContextRef	tlsContext;
	LPSTR				pszPrompt;
	PGPKeySetRef		KeySetToAdd;
	PGPKeySetRef		KeySetMain;
	HKEYMAN				hKM;
	KMCONFIG			KMConfig;
	CHAR				szHelpFile[MAX_PATH];
	HWND				hWndTree;
	INT					iyOffset;
	INT					iyTop;
} QUERYSTRUCT;

#define IDC_QUERYTREELIST		0x0101
#define BUTTONOFFSETX			20
#define BUTTONOFFSETY			10
#define DIALOGMINWIDTH			400
#define DIALOGMINHEIGHT			140

// local globals
static DWORD aIds[] = {			// Help IDs
	IDOK,				IDH_PGPKM_IMPORT,
	IDC_SELECTALL,		IDH_PGPKM_IMPORTSELALL,
	IDC_UNSELECTALL,	IDH_PGPKM_IMPORTUNSELALL,
	IDC_QUERYTREELIST,	IDH_PGPKM_IMPORTLIST,
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
sUpdateWheelScrollLines (QUERYSTRUCT* pqs)
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

	TreeList_SetWheelScrollLines (pqs->hWndTree, uLines);
}

//	___________________________________________________
//
//  Selective import dialog message procedure

static BOOL CALLBACK 
sQueryAddDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	RECT			rc;
	QUERYSTRUCT*	pqs;
	INT				ix, iy;
	POINT			pt;
	HWND			hwndControl;
	PGPBoolean		b;
	PGPPrefRef		prefRef;
	HCURSOR			hCursorOld;
	CHAR			sz[16];

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pqs = (QUERYSTRUCT*)lParam;

		// change "OK" button to "Import"
		LoadString (g_hInst, IDS_IMPORT, sz, sizeof(sz));
		SetDlgItemText (hDlg, IDOK, sz);

		// set prompt text
		hwndControl = GetDlgItem (hDlg, IDC_PROMPT);
		SetWindowText (hwndControl, pqs->pszPrompt);

		// create keymanager window
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.bottom;
		ScreenToClient (hDlg, &pt);
		pqs->iyTop = pt.y;

		hwndControl = GetDlgItem (hDlg, IDC_SELECTALL);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		iy = pt.y;
		GetClientRect (hDlg, &rc);
		pqs->iyOffset = rc.bottom - (iy - BUTTONOFFSETY) + pqs->iyTop;
		pqs->hKM = PGPkmCreateKeyManager (pqs->Context, pqs->tlsContext,
								hDlg, IDC_QUERYTREELIST, NULL, 0, pqs->iyTop, 
								rc.right-rc.left, 
								rc.bottom-pqs->iyOffset);

		// configure key manager window
		pqs->KMConfig.lpszHelpFile = NULL;
		pqs->KMConfig.keyserver.structSize = 0;
		pqs->KMConfig.hWndStatusBar = NULL;
		pqs->KMConfig.ulDisableActions = KM_ALLACTIONS & 
				~(KM_ADDTOMAIN|KM_SELECTALL|KM_UNSELECTALL|KM_PROPERTIES);
		pqs->KMConfig.ulShowColumns = 0;
		pqs->KMConfig.ulHideColumns = 0;
		pqs->KMConfig.ulOptionFlags = 
			KMF_ONLYSELECTKEYS|KMF_READONLY|KMF_MODALPROPERTIES;

		// get those options which are based on prefs
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pqs->Context),
									&prefRef);
		PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, &b);
		if (!b) pqs->KMConfig.ulOptionFlags |= KMF_NOVICEMODE;
		PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, &b);
		if (b) pqs->KMConfig.ulOptionFlags |= KMF_MARGASINVALID;
		PGPclCloseClientPrefs (prefRef, FALSE);

		pqs->KMConfig.ulMask = PGPKM_ALLITEMS;
		PGPkmConfigure (pqs->hKM, &pqs->KMConfig);
		
		// will be useful later to have HWND of keymanager
		pqs->hWndTree = PGPkmGetManagerWindow (pqs->hKM);

		// initialize keymanager
		PGPkmLoadKeySet (pqs->hKM, pqs->KeySetToAdd, pqs->KeySetMain);
		PGPkmPerformAction (pqs->hKM, KM_SELECTALL);

		// initialize wheel scrolling
		sUpdateWheelScrollLines (pqs);

		// post message to sync keysets
		PostMessage (hDlg, WM_APP, 0, 0);

		return FALSE;

	case WM_APP :
		// this syncs the import keyset with the main one
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPclSyncKeySets (pqs->Context, 
									pqs->KeySetMain, pqs->KeySetToAdd);
		PGPkmLoadKeySet (pqs->hKM, pqs->KeySetToAdd, pqs->KeySetMain);
		PGPkmPerformAction (pqs->hKM, KM_SELECTALL);
		SetCursor (hCursorOld);

		SetForegroundWindow (hDlg);
		SetFocus (pqs->hWndTree);
		break;

	case WM_ACTIVATE :
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		SetFocus (pqs->hWndTree);
		break;

	case WM_SIZE :
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		// resize keymanager window
		SetWindowPos (pqs->hWndTree, NULL, 0, pqs->iyTop, 
			LOWORD(lParam), HIWORD(lParam) - pqs->iyOffset, 
			SWP_NOMOVE|SWP_NOZORDER);

		// reposition buttons
		hwndControl = GetDlgItem (hDlg, IDC_SELECTALL);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + pqs->iyTop + BUTTONOFFSETY;
		SetWindowPos (hwndControl, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hwndControl = GetDlgItem (hDlg, IDC_UNSELECTALL);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + pqs->iyTop + BUTTONOFFSETY;
		SetWindowPos (hwndControl, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hwndControl = GetDlgItem (hDlg, IDCANCEL);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + pqs->iyTop + BUTTONOFFSETY;
		SetWindowPos (hwndControl, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hwndControl = GetDlgItem (hDlg, IDOK);
		GetWindowRect (hwndControl, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + pqs->iyTop + BUTTONOFFSETY;
		SetWindowPos (hwndControl, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);
		return 0;

	case WM_SYSCOLORCHANGE :
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		PostMessage (pqs->hWndTree, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_SETTINGCHANGE :
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		if (wParam == SPI_SETWHEELSCROLLLINES)
			sUpdateWheelScrollLines (pqs);
		else 
			PostMessage (pqs->hWndTree, WM_SYSCOLORCHANGE, 0, 0);
		break;

    case WM_HELP: 
 		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, pqs->szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pqs->szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aIds); 
        break; 

	case WM_NOTIFY :
		if (wParam == IDC_QUERYTREELIST) {
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			// let keymanager have first whack at the notification
			PGPkmDefaultNotificationProc (pqs->hKM, lParam);

			switch (((LPNM_TREELIST)lParam)->hdr.code) {
			// post the context menu
			case TLN_CONTEXTMENU :
				sQueryContextMenu (hDlg, pqs->hKM,
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
 		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		PGPkmSynchronizeThreadAccessToSDK (pqs->hKM);
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK :
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			if (PGPkmIsActionEnabled (pqs->hKM, KM_ADDTOMAIN)) {
				PGPkmPerformAction (pqs->hKM, KM_ADDTOMAIN);
				PGPkmDestroyKeyManager (pqs->hKM, FALSE);
				KMFree (pqs->hKM);
				EndDialog (hDlg, kPGPError_NoErr);
			}
			break;

		case IDCANCEL :
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmDestroyKeyManager (pqs->hKM, FALSE);
			KMFree (pqs->hKM);
			EndDialog (hDlg, kPGPError_UserAbort);
			break;

		case IDC_SELECTALL :
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pqs->hKM, KM_SELECTALL); 
			EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
			SetFocus (pqs->hWndTree);
			break;

		case IDC_UNSELECTALL :
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pqs->hKM, KM_UNSELECTALL); 
			EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
			break;

		case IDM_KEYPROPERTIES :
			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			PGPkmPerformAction (pqs->hKM, KM_PROPERTIES); 
			break;

		default :
			break;
		}
		return TRUE;

	default :
		if (uMsg == g_uMouseWheelMessage) {
			WORD iDelta;

			pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			iDelta = (WORD)wParam;
			
			SendMessage (pqs->hWndTree, 
								WM_MOUSEWHEEL, MAKEWPARAM (0, iDelta), 0);
			return FALSE;
		}
		break;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Display and handle selective import dialog

PGPError PGPclExport 
PGPclQueryAddKeys (
		PGPContextRef		Context,
		PGPtlsContextRef	tlsContext,
		HWND				hWndParent, 
		PGPKeySetRef		KeySetToAdd,			
		PGPKeySetRef		KeySetMain) 
{
	QUERYSTRUCT		QueryStruct;
	CHAR			sz[256];

	QueryStruct.hKM = NULL;
	QueryStruct.Context = Context;
	QueryStruct.tlsContext = tlsContext;
	QueryStruct.KeySetToAdd = KeySetToAdd;
	QueryStruct.KeySetMain = KeySetMain;

	PGPclGetPGPPath (QueryStruct.szHelpFile, 
							sizeof(QueryStruct.szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (QueryStruct.szHelpFile, sz);

	LoadString (g_hInst, IDS_QUERYADDPROMPT, sz, sizeof(sz));
	QueryStruct.pszPrompt = sz;

	return (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_QUERYADD), 
					hWndParent, sQueryAddDlgProc, (LPARAM)&QueryStruct));

}

