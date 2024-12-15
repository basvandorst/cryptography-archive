/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPQuery.c - selective import dialog
	

	$Id: KMquery.c,v 1.18 1997/11/05 16:16:46 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include <stdio.h>
#include "resource.h"
#include "pgpClientPrefs.h"

// External globals
extern HINSTANCE g_hInst;

typedef struct {
	PGPContextRef Context;
	PGPKeySetRef KeySetToAdd;
	PGPKeySetRef KeySetMain;
	HKEYMAN hKM;
	KMCONFIG KMConfig;
	CHAR szHelpFile[MAX_PATH];
	HWND hWndTree;
	INT iyOffset;
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

VOID 
KMQueryContextMenu (
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

//	___________________________________________________
//
//  Selective import dialog message procedure

BOOL CALLBACK 
QueryAddDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	RECT			rc;
	QUERYSTRUCT*	pqs;
	INT				ix, iy;
	POINT			pt;
	HWND			hWndButton;
	PGPBoolean		b;
	PGPPrefRef		prefRef;
	HCURSOR			hCursorOld;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pqs = (QUERYSTRUCT*)lParam;

		// create keymanager window
		hWndButton = GetDlgItem (hDlg, IDC_SELECTALL);
		GetWindowRect (hWndButton, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		iy = pt.y;
		GetClientRect (hDlg, &rc);
		pqs->iyOffset = rc.bottom - (iy - BUTTONOFFSETY);
		pqs->hKM = PGPkmCreateKeyManager (pqs->Context, hDlg, 
								IDC_QUERYTREELIST, 0, 0, 
								rc.right-rc.left, 
								rc.bottom-pqs->iyOffset);

		// configure key manager window
		pqs->KMConfig.lpszHelpFile = NULL;
		pqs->KMConfig.lpszPutKeyserver = NULL;
		pqs->KMConfig.hWndStatusBar = NULL;
		pqs->KMConfig.ulDisableActions = KM_ALLACTIONS & 
				~(KM_ADDTOMAIN|KM_SELECTALL|KM_UNSELECTALL|KM_PROPERTIES);
		pqs->KMConfig.ulShowColumns = 0;
		pqs->KMConfig.ulHideColumns = 0;
		pqs->KMConfig.ulOptionFlags = 
			KMF_ONLYSELECTKEYS|KMF_READONLY|KMF_MODALPROPERTIES;

		// get those options which are based on prefs
		PGPcomdlgOpenClientPrefs (&prefRef);
		PGPGetPrefBoolean (prefRef, kPGPPrefDisplayMarginalValidity, &b);
		if (!b) pqs->KMConfig.ulOptionFlags |= KMF_NOVICEMODE;
		PGPGetPrefBoolean (prefRef, kPGPPrefMarginalIsInvalid, &b);
		if (b) pqs->KMConfig.ulOptionFlags |= KMF_MARGASINVALID;
		PGPcomdlgCloseClientPrefs (prefRef, FALSE);

		pqs->KMConfig.ulMask = PGPKM_ALLITEMS;
		PGPkmConfigure (pqs->hKM, &pqs->KMConfig);
		
		// will be useful later to have HWND of keymanager
		pqs->hWndTree = PGPkmGetManagerWindow (pqs->hKM);

		// initialize keymanager
		PGPkmLoadKeySet (pqs->hKM, pqs->KeySetToAdd, pqs->KeySetMain);
		PGPkmPerformAction (pqs->hKM, KM_SELECTALL);

		// post message to sync keysets
		PostMessage (hDlg, WM_APP, 0, 0);

		return FALSE;

	case WM_APP :
		// this syncs the import keyset with the main one
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		PGPcomdlgSyncKeySets (pqs->Context, 
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
		SetWindowPos (pqs->hWndTree, NULL, 0, 0, 
			LOWORD(lParam), HIWORD(lParam) - pqs->iyOffset, 
			SWP_NOMOVE|SWP_NOZORDER);

		// reposition buttons
		hWndButton = GetDlgItem (hDlg, IDC_SELECTALL);
		GetWindowRect (hWndButton, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + BUTTONOFFSETY;
		SetWindowPos (hWndButton, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hWndButton = GetDlgItem (hDlg, IDC_UNSELECTALL);
		GetWindowRect (hWndButton, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + BUTTONOFFSETY;
		SetWindowPos (hWndButton, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hWndButton = GetDlgItem (hDlg, IDCANCEL);
		GetWindowRect (hWndButton, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + BUTTONOFFSETY;
		SetWindowPos (hWndButton, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);

		hWndButton = GetDlgItem (hDlg, IDOK);
		GetWindowRect (hWndButton, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient (hDlg, &pt);
		ix = pt.x;
		iy = HIWORD(lParam) - pqs->iyOffset + BUTTONOFFSETY;
		SetWindowPos (hWndButton, NULL, ix, iy, 0, 0, 
						SWP_NOSIZE|SWP_NOZORDER);
		return 0;

	case WM_SYSCOLORCHANGE :
	case WM_SETTINGCHANGE :
		// pass along to keymanager
		pqs = (QUERYSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
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
				KMQueryContextMenu (hDlg, pqs->hKM,
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

	}
	return FALSE;
}

//	___________________________________________________
//
//  Display and handle selective import dialog

PGPError PGPkmExport 
PGPkmQueryAddKeys (
		PGPContextRef	Context,
		HWND			hWndParent, 
		PGPKeySetRef	KeySetToAdd,			
		PGPKeySetRef	KeySetMain) 
{
	QUERYSTRUCT		QueryStruct;
	CHAR			sz[64];

	QueryStruct.hKM = NULL;
	QueryStruct.Context = Context;
	QueryStruct.KeySetToAdd = KeySetToAdd;
	QueryStruct.KeySetMain = KeySetMain;

	PGPcomdlgGetPGPPath (QueryStruct.szHelpFile, 
							sizeof(QueryStruct.szHelpFile));
	LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (QueryStruct.szHelpFile, sz);

	return (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_QUERYADD), 
					hWndParent, QueryAddDlgProc, (LPARAM)&QueryStruct));

}

