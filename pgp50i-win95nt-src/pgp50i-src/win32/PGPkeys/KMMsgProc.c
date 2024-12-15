/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMMsgProc - main message processing and associated routines
//
//	$Id: KMMsgProc.c,v 1.6 1997/05/27 18:39:26 pbj Exp $
//

#include <windows.h>
#include <stdio.h>
#include "pgpkeys.h"
#include "..\include\pgpRndPool.h"
#include "resource.h"
#include <commctrl.h>
#include "..\include\pgpkeyserversupport.h"

#define RELOADDELAY		 200L		//delay to allow clearing of window
#define RELOADTIMERID	1112L		//

#define WRITELOCKTRIES	3			//num times to try keyring when locked
#define WRITELOCKDELAY	1500L		//ms delay when keyring is writelocked

// External globals
extern HINSTANCE g_hInst;
extern HWND g_hWndMain;
extern HWND g_hWndTree;
extern CHAR g_szHelpFile[MAX_PATH];
extern PGPKeySet* g_pKeySetMain;
extern LONG g_lKeyListSortField;
extern CHAR g_szbuf[G_BUFLEN];
extern BOOL g_bReadOnly;
extern UINT g_uReloadKeyringMessage;
extern BOOL g_bKeyServerSupportLoaded;

// Local globals
static HMENU hMenuKeyMan;					//handle of main menu
static BOOL bKeyHasBeenGenerated = FALSE;	//TRUE => new key has been genned
static HHOOK hhookMouse;					//handle of mouse hook
static LPDROPTARGET pDropTarget;			//pointer to DropTarget object
static HIMAGELIST hImlLogo = NULL;
static HBRUSH hBrushBackground;


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
//  Init logo

VOID InitLogo (void) {
	HBITMAP hBmp;
	HBITMAP hBmpMsk;

	hImlLogo = ImageList_Create (LOGO_WIDTH, LOGO_HEIGHT,
								 ILC_COLOR|ILC_MASK, 1, 0);

	if (hImlLogo) {
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_LOGO));
		hBmpMsk = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_LOGOMSK));
		ImageList_Add (hImlLogo, hBmp, hBmpMsk);
		DeleteObject (hBmpMsk);
		DeleteObject (hBmp);
	}
}


//----------------------------------------------------|
//  Paint logo

VOID PaintLogo (HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint (hWnd, &ps);
	ImageList_Draw (hImlLogo, 0, hdc, 0, 0, ILD_TRANSPARENT);
	EndPaint (hWnd, &ps);
}


//----------------------------------------------------|
//  Reload keyrings

VOID ReloadKeyrings (void) {
	PGPError pgpError;
	INT iLockTries;
	Boolean bKeyGenerated, bMutable;

	iLockTries = 0;
	bMutable = TRUE;

	PGPcomdlgErrorMessage (pgpLoadPrefs ());
	g_pKeySetMain = pgpOpenDefaultKeyRings (bMutable, &pgpError);
	while (!g_pKeySetMain || !KMLoadKeyRingIntoTree (g_hWndMain,
								g_hWndTree, FALSE, FALSE)) {
		switch (pgpError) {
		case PGPERR_FILE_PERMISSIONS :
			bMutable = FALSE;
			break;

		case PGPERR_FILE_WRITELOCKED :
			iLockTries++;
			if (iLockTries < WRITELOCKTRIES) {
				Sleep (WRITELOCKDELAY);
			}
			else {
				KMMessageBox (g_hWndMain, IDS_CAPTION, IDS_LOCKEDKEYRING,
					MB_OK|MB_ICONSTOP);
				if (!KMPGPPreferences (g_hWndMain, 1)) {
					SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
					return;
				}
			}
			break;

		default :
			KMMessageBox (g_hWndMain, IDS_CAPTION, IDS_CORRUPTKEYRING,
				MB_OK|MB_ICONSTOP);
			if (!KMPGPPreferences (g_hWndMain, 1)) {
				SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
				return;
			}
			break;
		}
		g_pKeySetMain = pgpOpenDefaultKeyRings (bMutable, &pgpError);
	}

	InvalidateRect (g_hWndTree, NULL, TRUE);

	g_bReadOnly = !bMutable;

	KMCreateMenus (hMenuKeyMan);
	KMEnableDropTarget (pDropTarget, !g_bReadOnly);
	DragAcceptFiles (g_hWndMain, !g_bReadOnly);
	if (g_bReadOnly) KMReadOnlyWarning (g_hWndMain);
	else {
		pgpGetPrefBoolean (kPGPPrefFirstKeyGenerated, &bKeyGenerated);
		if (!bKeyGenerated) {
			if (!KMCheckForSecretKeys (g_pKeySetMain)) {
				ShowWindow (g_hWndMain, SW_HIDE);
				KMCreateKey (g_hWndMain);
			}
		}
	}
}


//----------------------------------------------------|
//  Main PGPkeys Window Message procedure

LONG APIENTRY KeyManagerWndProc (HWND hWnd, UINT uMsg,
								 WPARAM wParam, LPARAM lParam) {

	HTLITEM hFocused;
	BOOL bMultiple;
	INT i;

	switch (uMsg) {

	case WM_CREATE :
		g_pKeySetMain = NULL;
		g_uReloadKeyringMessage = RegisterWindowMessage (RELOADKEYRINGMSG);
		KMSetFocus (NULL, FALSE);
		hMenuKeyMan = GetMenu (hWnd);
		g_hWndTree = KMCreateTreeList (hWnd);
		KMAddColumns (g_hWndTree);
		pDropTarget = KMCreateDropTarget ();
		CoLockObjectExternal ((IUnknown*)pDropTarget, TRUE, TRUE);
		RegisterDragDrop (hWnd, pDropTarget);
		KMEnableDropTarget (pDropTarget, FALSE);
		hhookMouse = SetWindowsHookEx (WH_MOUSE, MouseHookProc,
										NULL, GetCurrentThreadId());
		PGPcomdlgSetSplashParent (hWnd);
		if (PGPcomdlgCheckKeyRings (hWnd, FALSE) == PGPCOMDLG_CANCEL)
			PostMessage (hWnd, WM_CLOSE, 0, 0);
		else
			PostMessage (hWnd, KM_M_RELOADKEYRINGS, 0, 0);
		return 0;

	case WM_CLOSE :
		UnhookWindowsHookEx (hhookMouse);
		RevokeDragDrop (hWnd);
		KMReleaseDropTarget (pDropTarget);
		CoLockObjectExternal ((IUnknown*)pDropTarget, FALSE, TRUE);
		DragAcceptFiles (hWnd, FALSE);
		KMDestroyMenus ();
		if (g_pKeySetMain) pgpFreeKeySet (g_pKeySetMain);
		if (bKeyHasBeenGenerated) while (KMBackupWarn (hWnd));
		KMSetWindowRegistryData (hWnd);
		KMDestroyTree ();
		g_hWndTree = NULL;
		WinHelp (hWnd, g_szHelpFile, HELP_QUIT, 0);
		DestroyWindow (hWnd);
		break;

	case WM_ACTIVATE :
		SetFocus (g_hWndTree);
		break;

	case WM_DESTROY :
		PostQuitMessage(0);
		break;

	case WM_CONTEXTMENU :
		if ((HWND)wParam == g_hWndTree)
			KMContextMenu (hWnd, LOWORD (lParam), HIWORD (lParam));
		break;

	case WM_INITMENU :
		if ((HMENU)wParam == hMenuKeyMan) KMSetMainMenu (hMenuKeyMan);
		break;

	case WM_SIZE :
		SetWindowPos (g_hWndTree, NULL, 0, LOGO_HEIGHT,
			LOWORD(lParam), HIWORD(lParam) -LOGO_HEIGHT,
			SWP_NOMOVE|SWP_NOZORDER);
		return 0;

	case WM_SYSCOLORCHANGE :
	case WM_SETTINGCHANGE :
		PostMessage (g_hWndTree, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_SIZING :
		if ((((LPRECT)lParam)->right - ((LPRECT)lParam)->left) < 300) {
			((LPRECT)lParam)->right = ((LPRECT)lParam)->left + 300;
		}
		if ((((LPRECT)lParam)->bottom - ((LPRECT)lParam)->top) < 140) {
			((LPRECT)lParam)->bottom = ((LPRECT)lParam)->top + 140;
		}
		break;

	case WM_DROPFILES :
		KMImportKey (hWnd, (HDROP)wParam);
		DragFinish ((HDROP)wParam);
		break;

	case WM_NOTIFY :
		if (wParam == IDC_TREELIST) {
			switch (((LPNM_TREELIST)lParam)->hdr.code) {
			case TLN_SELCHANGED :
				bMultiple = (((LPNM_TREELIST)lParam)->flags & TLC_MULTIPLE);
				hFocused = ((LPNM_TREELIST)lParam)->itemNew.hItem;
				KMSetFocus (hFocused, bMultiple);
				break;

			case TLN_KEYDOWN :
				switch (((TL_KEYDOWN*)lParam)->wVKey) {
				case VK_DELETE :
					KMDeleteObject (hWnd);
					break;
				}
				break;

			case TLN_BEGINDRAG :
				KMEnableDropTarget (pDropTarget, FALSE);
				KMDragAndDrop (hWnd);
				KMEnableDropTarget (pDropTarget, !g_bReadOnly);
				break;

			case TLN_HEADERCLICKED :
				i = g_lKeyListSortField;
				switch (((LPNM_TREELIST)lParam)->index) {
					case 0 :
						if (g_lKeyListSortField == kPGPUserIDOrdering)
							i = kPGPReverseUserIDOrdering;
						else
							i = kPGPUserIDOrdering;
						break;

					case 1 :
						if (g_lKeyListSortField == kPGPValidityOrdering)
							i = kPGPReverseValidityOrdering;
						else
							i = kPGPValidityOrdering;
						break;

					case 2 :
						if (g_lKeyListSortField == kPGPTrustOrdering)
							i = kPGPReverseTrustOrdering;
						else
							i = kPGPTrustOrdering;
						break;

					case 3 :
						if (g_lKeyListSortField == kPGPCreationOrdering)
							i = kPGPReverseCreationOrdering;
						else
							i = kPGPCreationOrdering;
						break;

					case 4 :
						if (g_lKeyListSortField == kPGPKeySizeOrdering)
							i = kPGPReverseKeySizeOrdering;
						else
							i = kPGPKeySizeOrdering;
						break;

					default : break;
				}
				if (i != g_lKeyListSortField) {
					g_lKeyListSortField = i;
					TreeList_DeleteTree (g_hWndTree, FALSE);
					InvalidateRect (g_hWndTree, NULL, TRUE);
					UpdateWindow (g_hWndTree);
					KMLoadKeyRingIntoTree (hWnd, g_hWndTree, TRUE, FALSE);
					InvalidateRect (g_hWndTree, NULL, TRUE);
				}
				break;
			}
		}
		break;

	case WM_TIMER :
		KillTimer (hWnd, wParam);
		if (wParam == RELOADTIMERID) {
			ReloadKeyrings ();
		}
		break;

	case KM_M_RELOADKEYRINGS :
		if (g_pKeySetMain) {
			KMDeleteAllKeyProperties ();
			KMSetFocus (NULL, FALSE);
			pgpFreeKeySet (g_pKeySetMain);
			g_pKeySetMain = NULL;
			TreeList_DeleteTree (g_hWndTree, TRUE);
		}
		InvalidateRect (g_hWndTree, NULL, TRUE);
		UpdateWindow (g_hWndTree);
		SetTimer (hWnd, RELOADTIMERID, RELOADDELAY, NULL);
		break;

	case KM_M_CREATEDONE :
		ShowWindow (hWnd, SW_SHOW);
		EnableWindow (hWnd, TRUE);
		SetForegroundWindow (hWnd);
		if (wParam & KCD_NEWKEY) {
			TL_TREEITEM tli;
			pgpSetPrefBoolean (kPGPPrefFirstKeyGenerated, (Boolean)TRUE);
			bKeyHasBeenGenerated = TRUE;
			KMCommitKeyRingChanges (g_pKeySetMain);
			KMUpdateKeyInTree (g_hWndTree, (PGPKey*)lParam);
			InvalidateRect (g_hWndTree, NULL, TRUE);
			pgpGetKeyUserVal ((PGPKey*)lParam, (long*)&(tli.hItem));
			TreeList_Select (g_hWndTree, &tli, TRUE);
			TreeList_Expand (g_hWndTree, &tli, TLE_EXPANDALL);
			KMSetFocus (tli.hItem, FALSE);
			if (wParam & KCD_NEWDEFKEY) KMSetDefaultKey (g_hWndTree);
		}
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDM_FILEEXIT:
			SendMessage (hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_HELPABOUT :
			KMHelpAbout (hWnd);
			break;

		case IDM_CERTIFYKEY :
			KMCertifyKeyOrUserID (hWnd);
			break;

		case IDM_DISABLEKEY :
			KMDisableKey (hWnd, (PGPKey*)KMFocusedObject ());
			break;

		case IDM_ENABLEKEY :
			KMEnableKey (hWnd, (PGPKey*)KMFocusedObject ());
			break;

		case IDM_DELETEKEY :
			KMDeleteObject (hWnd);
			break;

		case IDM_CREATEKEY :
			KMCreateKey (hWnd);
			break;

		case IDM_SETASDEFAULT :
			if (KMFocusedObjectType () == OBJECT_KEY) KMSetDefaultKey (hWnd);
			else KMSetPrimaryUserID (hWnd);
			break;

		case IDM_KEYPROPERTIES :
			KMKeyProperties (hWnd);
			break;
		
		case IDM_PROPERTIES :
			KMPGPPreferences (hWnd, 0);
			break;

		case IDM_IMPORTKEYS :
			KMImportKey (hWnd, NULL);
			break;
		
		case IDM_EXPORTKEYS :
			KMExportKeys (hWnd, NULL);
			break;

		case IDM_EXPANDSEL :
			KMExpandSelected (g_hWndTree);
			break;

		case IDM_COLLAPSESEL :
			KMCollapseSelected (g_hWndTree);
			break;

		case IDM_SELECTALL :
			TreeList_SelectChildren (g_hWndTree, NULL);
			hFocused = TreeList_GetFirstItem (g_hWndTree);
			KMSetFocus (hFocused, TRUE);
			break;

		case IDM_REVOKECERT :
			KMRevokeCert (hWnd);
			break;

		case IDM_REVOKEKEY :
			if (KMFocusedItemType () == IDX_CERT) KMRevokeCert (hWnd);
			else KMRevokeKey (hWnd);
			break;

		case IDM_COPYKEY :
			KMCopyKeys (NULL);
			break;

		case IDM_PASTEKEY :
			KMPasteKeys (hWnd);
			break;

		case IDM_ADDUSERID :
			KMAddUserToKey (hWnd);
			break;

		case IDM_HELPTOPICS :
			WinHelp (hWnd, g_szHelpFile, HELP_FINDER, 0);
			break;

		case IDM_ADVERTISE1 :
			PGPcomdlgAdvertisement (hWnd, 0);
			break;

		default :
			if (g_bKeyServerSupportLoaded) {
 				BOOL bUpdatedKeys;
				LPSTR szSelectedKeyIDs;
				PGPKeySet* pKeySetSelected = NULL;
 				if (KS_ID_IS_IN_RANGE(LOWORD(wParam))) {
					KMGetSelectedObjects (&pKeySetSelected,
											&szSelectedKeyIDs);
 					ProcessKeyServerMessageKR (hWnd, LOWORD(wParam),
												szSelectedKeyIDs,
												&bUpdatedKeys,
												pKeySetSelected,
												g_pKeySetMain);
					KMFreeSelectedObjects (pKeySetSelected, szSelectedKeyIDs);
 					if (bUpdatedKeys) {
						KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, TRUE);
						InvalidateRect (g_hWndTree, NULL, TRUE);
						PGPcomdlgNotifyKeyringChanges
							(GetCurrentProcessId ());
					}
 				}
			}
			break;
		}
		break;

	default :
		if (uMsg == g_uReloadKeyringMessage) {
			if ((DWORD)lParam != GetCurrentProcessId ())
				PostMessage (hWnd, KM_M_RELOADKEYRINGS, 0, 0);
			return TRUE;
		}
		else return (DefWindowProc (hWnd, uMsg, wParam, lParam));
	}
	return FALSE;
}
