/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PKMsgPrc - main message processing and associated routines
	

	$Id: PKMsgPrc.c,v 1.102.4.1 1997/11/20 20:55:26 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

#include "pgpkeysx.h"
#include <stdio.h>
#include "search.h"
#include "resource.h"
#include "..\include\PGPpk.h"

#include "pgpKeyserverPrefs.h"
#include "pgpSDKPrefs.h"

#define RELOADDELAY		 200L		//delay to allow clearing of window
#define RELOADTIMERID	1112L		//

#define WRITELOCKTRIES	3			//num times to try keyring when locked
#define WRITELOCKDELAY	1500L		//ms delay when keyring is writelocked


#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L

#define NUMLEDS 10
#define LEDWIDTH 6
#define LEDSPACING 2
#define TOTALLEDWIDTH (NUMLEDS*(LEDWIDTH+LEDSPACING+1))

#define TOOLBARHEIGHT 0
#define STATUSBARHEIGHT	20
#define SEARCHCONTROLHEIGHT 100

#define GRABBARHEIGHT	2

#define MINSEARCHWINDOWHEIGHT	260
#define MINSEARCHWINDOWWIDTH	460

// External globals
extern HINSTANCE		g_hInst;
extern HWND				g_hWndMain;
extern CHAR				g_szHelpFile[MAX_PATH];
extern UINT				g_uReloadKeyringMessage;
extern PGPContextRef	g_ContextRef;
extern PGPBoolean		g_bExpertMode;
extern PGPBoolean		g_bKeyGenEnabled;
extern PGPBoolean		g_bRSAEnabled;
extern PGPBoolean		g_bMarginalAsInvalid;
extern BOOL				g_bReadOnly;
extern BOOL				g_bShowGroups;
extern INT				g_iGroupsPercent;

// Local globals
static BOOL		bKeyHasBeenGenerated = FALSE;
static BOOL		bMakeBackups = FALSE;
static BOOL		bIgnoreReloads = FALSE;
static HWND		hwndOpenSearch = NULL;
static BOOL		bFirstKeyringLoad = TRUE;


//	____________________________________
//
//  Look for secret keys

BOOL 
PGPkeysCheckForSecretKeys (PGPKeySetRef KeySet) 
{
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPBoolean		bSecret;
	BOOL			bSecretKeys;

	PGPOrderKeySet (KeySet, kPGPAnyOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);

	bSecretKeys = FALSE;
	PGPKeyIterNext (KeyIter, &Key);

	while (Key && !bSecretKeys) {
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			bSecretKeys = TRUE;
		}
		PGPKeyIterNext (KeyIter, &Key);
	}

	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return bSecretKeys;
}

//	____________________________________
//
//  Process files in command line

VOID 
ProcessFileList (LPSTR			pszList, 
				 BOOL			bCommandLine, 
				 BOOL			bAllowSelect,
				 PGPKeySetRef	keysetMain) 
{
	PGPFileSpecRef	fileref;
	PGPKeySetRef	keysetToAdd;
	LPSTR			p, p2;
	INT				iNumKeys;
	CHAR			cTerm;
	
	p = pszList;

	// skip over path of program 
	if (bCommandLine) {
		while (*p && (*p == ' ')) p++;
		if (*p) {
			if (*p == '"') {
				p++;
				cTerm = '"';
			}
			else cTerm = ' ';
		}
		while (*p && (*p != cTerm)) p++;
		if (*p && (cTerm == '"')) p++;
	}

	// parse file names
	// Unfortunately, the OS hands me names in the command line in all 
	// sorts of forms: space delimited; quoted and space delimited; space
	// delimiter after the program path but then NULL terminated.
	// And this is just NT ...
	while (p && *p) {
		while (*p && (*p == ' ')) p++;
		if (*p) {
			if (*p == '"') {
				p++;
				cTerm = '"';
			}
			else cTerm = ' ';

			p2 = strchr (p, cTerm);
			if (p2) {
				if (*(p2+2) == ':') *p2 = '\0';
			}

			PGPNewFileSpecFromFullPath (g_ContextRef, p, &fileref);
			if (fileref) {
				PGPImportKeySet (g_ContextRef, &keysetToAdd, 
								PGPOInputFile (g_ContextRef, fileref),
								PGPOLastOption (g_ContextRef));
				if (keysetToAdd) {
					PGPCountKeys (keysetToAdd, &iNumKeys);
					if (iNumKeys > 0) {
						if (bAllowSelect) {
							PGPkmQueryAddKeys (g_ContextRef, g_hWndMain, 
										keysetToAdd, keysetMain);
						}
						else {
							PGPAddKeys (keysetToAdd, keysetMain);
							PGPkeysCommitKeyRingChanges (keysetMain, TRUE);
						}
					}
					PGPFreeKeySet (keysetToAdd);
				}
				PGPFreeFileSpec (fileref);
			}
			if (p2) *p2 = ' ';
		}
		while (*p && (*p != cTerm)) p++;
		if (*p && (cTerm == '"')) p++;
	}
}

//	____________________________________
//
//  Import keys from WM_COPYDATA struct

BOOL 
ImportData (PGPKEYSSTRUCT* ppks, PCOPYDATASTRUCT pcds) 
{
	BOOL			bPrompt;
	PGPError		err;
	PGPKeySetRef	keyset;


	bPrompt = pcds->dwData & PGPPK_SELECTIVEIMPORT;

	switch (pcds->dwData & PGPPK_IMPORTKEYMASK) {
	case PGPPK_IMPORTKEYBUFFER :
		err = PGPImportKeySet (g_ContextRef, &keyset, 
				PGPOInputBuffer (g_ContextRef, pcds->lpData, pcds->cbData),
				PGPOLastOption (g_ContextRef));
		if (IsntPGPError (err) && keyset) {
			if (bPrompt) {
				PGPkmQueryAddKeys (g_ContextRef, g_hWndMain, keyset,
									ppks->KeySetMain);
			}
			else {
				err = PGPAddKeys (keyset, ppks->KeySetMain);
				PGPkeysCommitKeyRingChanges (ppks->KeySetMain, TRUE);
				PGPkmReLoadKeySet (ppks->hKM, TRUE);
				PGPFreeKeySet (keyset);
				return FALSE;
			}
		}
		if (keyset) PGPFreeKeySet (keyset);
		return (IsntPGPError (err));

	case PGPPK_IMPORTKEYFILELIST :
		ProcessFileList (pcds->lpData, FALSE, bPrompt, ppks->KeySetMain);
		return TRUE;

	case PGPPK_IMPORTKEYCOMMANDLINE :
		ProcessFileList (pcds->lpData, TRUE, bPrompt, ppks->KeySetMain);
		return TRUE;
	}
}

//	____________________________________
//
//  import keys from admin prefs file

PGPError 
ImportPrefsKeys (PGPKeySetRef keysetMain)
{
	PGPError		err			= kPGPError_UnknownError;

#if PGP_BUSINESS_SECURITY
	BOOL			bSave		= FALSE;
	PGPKeySetRef	keysetNew;
	PGPPrefRef		prefref;
	LPSTR			psz;

#if PGP_ADMIN_BUILD
	err = PGPcomdlgOpenAdminPrefs (&prefref, TRUE);
#else
	err = PGPcomdlgOpenAdminPrefs (&prefref, FALSE);
#endif

	if (IsntPGPError (err)) {
		err = PGPGetPrefStringAlloc (prefref, kPGPPrefDefaultKeys, &psz);
		if (IsntPGPError (err) && psz) {
			if (psz[0]) {
				err = PGPImportKeySet (g_ContextRef, &keysetNew, 
						PGPOInputBuffer (g_ContextRef, psz, lstrlen(psz)+1),
						PGPOLastOption (g_ContextRef));

				if (IsntPGPError (err)) {
					PGPAddKeys (keysetNew, keysetMain);
					err = PGPkeysCommitKeyRingChanges (keysetMain, TRUE);
/// >>> don't delete the keys -- import every time
///					if (IsntPGPError (err)) {
///						psz[0] = '\0';
///						PGPSetPrefString (prefref, kPGPPrefDefaultKeys, psz);
///						bSave = TRUE;
///					}
/// <<<
					PGPFreeKeySet (keysetNew);
				}
			}
			PGPDisposePrefData (prefref, psz);
		}
		PGPcomdlgCloseAdminPrefs (prefref, bSave);
	}

#endif //PGP_BUSINESS_SECURITY

	return err;
}

//	____________________________________
//
//  Reload keyrings

VOID 
ReloadKeyrings (PGPKEYSSTRUCT* ppks) 
{
	BOOL				bMutable			= TRUE;
	PGPKeyRingOpenFlags	flags				= kPGPKeyRingOpenFlags_Mutable;
	INT					iLockTries			= 0;

	PGPError			pgpError;
	HCURSOR				hCursorOld;

	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	PGPsdkLoadDefaultPrefs (g_ContextRef);
	pgpError = PGPOpenDefaultKeyRings (g_ContextRef, flags, 
										&(ppks->KeySetMain));
	SetCursor (hCursorOld);

	while (!ppks->KeySetMain || 
		   PGPkmLoadKeySet (ppks->hKM, ppks->KeySetMain, ppks->KeySetMain)) {

		switch (pgpError) {

		case kPGPError_FilePermissions :
			bMutable = FALSE;
			break;

		case kPGPError_FileLocked :
			iLockTries++;
			if (iLockTries < WRITELOCKTRIES) {
				Sleep (WRITELOCKDELAY);
			}
			else {
				PGPkeysMessageBox (g_hWndMain, IDS_CAPTION, IDS_LOCKEDKEYRING,
					MB_OK|MB_ICONSTOP);
				if (!PGPkeysPGPPreferences (ppks, g_hWndMain, 1)) {
					SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
					return;
				}
			}
			break;

		case kPGPError_CantOpenFile :
		case kPGPError_FileNotFound :
			if (!PGPkeysPGPPreferences (ppks, g_hWndMain, 1)) {
				SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
				return;
			}
			break;

		default :
			PGPkeysMessageBox (g_hWndMain, IDS_CAPTION, IDS_CORRUPTKEYRING, 
				MB_OK|MB_ICONSTOP);
			if (!PGPkeysPGPPreferences (ppks, g_hWndMain, 1)) {
				SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
				return;
			}
			break;
		}
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		flags = 0;
		if (bMutable) flags |= kPGPKeyRingOpenFlags_Mutable;
		pgpError = PGPOpenDefaultKeyRings (g_ContextRef, flags, 
											&(ppks->KeySetMain));
		SetCursor (hCursorOld);
	}

	InvalidateRect (ppks->hWndTreeList, NULL, TRUE);

	g_bReadOnly = !bMutable;

	if (g_bReadOnly) {
		ppks->kmConfig.ulOptionFlags |= KMF_READONLY;
		PGPkeysReadOnlyWarning (ppks->hWndMain);
	}
	else {
		ppks->kmConfig.ulOptionFlags &= ~KMF_READONLY;
	}

	// pass readonly flag to keymanager
	ppks->kmConfig.ulMask = PGPKM_OPTIONS;
	PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));

	// reload the groups file
	ppks->gmConfig.keysetMain = ppks->KeySetMain;
	PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
	PGPgmLoadGroups (ppks->hGM);
}

//	____________________________________
//
//  Draw the owner-drawn part of the status bar

VOID
DrawStatus (
		LPDRAWITEMSTRUCT lpdis, 
		PGPKEYSSTRUCT* ppks) 
{
	HBRUSH	hBrushLit, hBrushUnlit, hBrushOld;
	HPEN	hPen, hPenOld;
	INT		i;
	INT		itop, ibot, ileft, iright;

	if (ppks->iStatusValue < -1) return;

	// draw 3D shadow
	itop = lpdis->rcItem.top+3;
	ibot = lpdis->rcItem.bottom-5;

	ileft = lpdis->rcItem.left + 4;
	for (i=0; i<NUMLEDS; i++) {
		iright = ileft + LEDWIDTH;

		MoveToEx (lpdis->hDC, ileft, ibot, NULL);
		LineTo (lpdis->hDC, iright, ibot);
		LineTo (lpdis->hDC, iright, itop);

		ileft += LEDWIDTH + LEDSPACING;
	}

	hPen = CreatePen (PS_SOLID, 0, RGB (128, 128, 128));
	hPenOld = SelectObject (lpdis->hDC, hPen);
	hBrushLit = CreateSolidBrush (RGB (0, 255, 0));
	hBrushUnlit = CreateSolidBrush (RGB (0, 128, 0));

	ileft = lpdis->rcItem.left + 4;

	// draw "Knight Rider" LEDs
	if (ppks->iStatusDirection) {
		hBrushOld = SelectObject (lpdis->hDC, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + LEDWIDTH;
	
			if (i == ppks->iStatusValue) {
				SelectObject (lpdis->hDC, hBrushLit);
				Rectangle (lpdis->hDC, ileft, itop, iright, ibot);
				SelectObject (lpdis->hDC, hBrushUnlit);
			}
			else  {
				Rectangle (lpdis->hDC, ileft, itop, iright, ibot);
			}
	
			ileft += LEDWIDTH + LEDSPACING;
		}
	}

	// draw "progress bar" LEDs
	else { 
		if (ppks->iStatusValue >= 0) 
			hBrushOld = SelectObject (lpdis->hDC, hBrushLit);
		else
			hBrushOld = SelectObject (lpdis->hDC, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + LEDWIDTH;
	
			if (i > ppks->iStatusValue) {
				SelectObject (lpdis->hDC, hBrushUnlit);
			}
			Rectangle (lpdis->hDC, ileft, itop, iright, ibot);
	
			ileft += LEDWIDTH + LEDSPACING;
		}
	}

	SelectObject (lpdis->hDC, hBrushOld);
	SelectObject (lpdis->hDC, hPenOld);
	DeleteObject (hPen);
	DeleteObject (hBrushLit);
	DeleteObject (hBrushUnlit);
}


//	____________________________________
//
//  display downloaded key count in status bar

INT
DisplayKSKeyResult (PGPKeySetRef keyset, HWND hwnd, PGPError err) {
	CHAR sz1[256];
	CHAR sz2[256];
	INT	 iCount;

	if (keyset)
		(void)PGPCountKeys (keyset, &iCount);
	else 
		iCount = 0;

	if (err == kPGPError_ServerTooManyResults)
		LoadString (g_hInst, IDS_KSTOOMANYKEYCOUNT, sz1, sizeof(sz1));
	else 
		LoadString (g_hInst, IDS_KSKEYCOUNT, sz1, sizeof(sz1));
	wsprintf (sz2, sz1, iCount);
	SendMessage (hwnd, SB_SETTEXT, 0, (LPARAM)sz2);
	
	return iCount;
}

//	____________________________________
//
//  Main PGPkeys Window Message procedure

LONG APIENTRY 
KeyManagerWndProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	RECT			rc;
	PGPKEYSSTRUCT*	ppks;
	INT				iWidths[2];

	switch (uMsg) {

	case WM_CREATE :
		ppks = (PGPKEYSSTRUCT*)pkAlloc (sizeof (PGPKEYSSTRUCT));
		if (!ppks) return -1;
		SetWindowLong (hWnd, GWL_USERDATA, (LPARAM)ppks);

		GetClientRect (hWnd, &rc);

		// if lpCreateParams is NULL, this is the main window
		if (((LPCREATESTRUCT)lParam)->lpCreateParams == NULL) {
			ppks->bMainWindow = TRUE;
			ppks->iKeysHeight = 
				rc.bottom-rc.top-ppks->iToolHeight-STATUSBARHEIGHT;
			ppks->bGroupsVisible = FALSE;
			ppks->iGroupsPercent = g_iGroupsPercent;
			ppks->iGroupsHeight = 0;
			ppks->hMenuKeyserver = NULL;
			ppks->bLocalKeySet = TRUE;
			ppks->iToolHeight = TOOLBARHEIGHT; 
			ppks->KeySetMain = NULL;
			ppks->KeySetDisp = NULL;
			ppks->kmConfig.ulOptionFlags = 
				KMF_ENABLECOMMITS|KMF_ENABLEDRAGOUT|
				KMF_ENABLEDROPIN|KMF_ENABLERELOADS;
			if (!g_bExpertMode) 
				ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			if (g_bMarginalAsInvalid) 
				ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			if (!g_bRSAEnabled)
				ppks->kmConfig.ulOptionFlags |= KMF_DISABLERSAOPS;
			ppks->kmConfig.ulDisableActions = 
				KM_ADDTOMAIN;
			// setup group manager
			ppks->hGM = PGPgmCreateGroupManager (g_ContextRef, hWnd, 
					IDC_TREELISTGROUP,
					0, 
					(rc.bottom-rc.top)-ppks->iGroupsHeight-
												STATUSBARHEIGHT,
					(rc.right-rc.left),
					ppks->iGroupsHeight);
			ppks->hWndTreeListGroups = PGPgmGetManagerWindow (ppks->hGM);
			ppks->gmConfig.ulOptionFlags = 0;
			if (!g_bExpertMode) 
				ppks->gmConfig.ulOptionFlags |= GMF_NOVICEMODE;
			ppks->gmConfig.ulDisableActions = 0;
			ppks->gmConfig.keysetMain = NULL;
		}

		// otherwise this is a search window and lpCreateParams is 
		// the main KeySetRef
		else {
			hwndOpenSearch = hWnd;
			PKAddSearchDlgToList (hWnd);
			ppks->bMainWindow = FALSE;
			ppks->iKeysHeight = 
				rc.bottom-rc.top-ppks->iToolHeight-STATUSBARHEIGHT;
			ppks->bGroupsVisible = FALSE;
			ppks->iGroupsHeight = 0;
			ppks->iGroupsPercent = 0;
			ppks->bLocalKeySet = TRUE;
			ppks->iToolHeight = SEARCHCONTROLHEIGHT;
			ppks->KeySetMain = 
				(PGPKeySetRef)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			ppks->KeySetDisp = NULL;
			ppks->kmConfig.ulOptionFlags = 
				KMF_ENABLEDRAGOUT;
			if (!g_bExpertMode) 
				ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			if (g_bMarginalAsInvalid) 
				ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			if (!g_bRSAEnabled)
				ppks->kmConfig.ulOptionFlags |= KMF_DISABLERSAOPS;
			ppks->kmConfig.ulDisableActions = KM_ALLACTIONS;
			ppks->pGroupFile = NULL;
			if (InitSearch ()) {
				ppks->hWndSearchControl = CreateSearch (g_hInst, hWnd);

				SendMessage (ppks->hWndSearchControl, SEARCH_SET_LOCAL_KEYSET,
							0, (LPARAM)(ppks->KeySetMain));
				GetWindowRect (ppks->hWndSearchControl, &rc);
				ppks->iToolHeight = rc.bottom-rc.top;
			}
		}
		ppks->bGroupsFocused = FALSE;
		ppks->uKeySelectionFlags = 0;
		ppks->uGroupSelectionFlags = 0;
		ppks->hMenuKeyMan = GetMenu (hWnd);
		ppks->hCursorOld = NULL;
		ppks->bGrabEnabled = FALSE;
		ppks->bGrabbed = FALSE;

		ppks->hKM = PGPkmCreateKeyManager (g_ContextRef, hWnd, IDC_TREELIST, 
							0, ppks->iToolHeight, 
							rc.right-rc.left, 
							ppks->iKeysHeight);

		ppks->kmConfig.lpszHelpFile = NULL;
		ppks->kmConfig.lpszPutKeyserver = NULL;
		ppks->kmConfig.ulShowColumns = 0;
		ppks->kmConfig.ulHideColumns = 0;

		// create status bar
		ppks->bSearchInProgress = FALSE;
		ppks->iStatusValue = -2;
		ppks->iStatusDirection = 1;
		ppks->kmConfig.hWndStatusBar = CreateStatusWindow (
			WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,
			"",
			hWnd,
			IDC_STATUSBAR);
		iWidths[0] = rc.right-TOTALLEDWIDTH-16;
		iWidths[1] = rc.right-16;
		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETPARTS, 2, 
						(LPARAM)iWidths);
		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
			1|SBT_OWNERDRAW, 0);

		// set initial configuration of keymanager
		ppks->kmConfig.ulMask = PGPKM_ALLITEMS;
		PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
		
		ppks->hWndTreeList = PGPkmGetManagerWindow (ppks->hKM);

		if (ppks->bMainWindow) {
			ppks->gmConfig.lpszHelpFile = NULL;
			ppks->gmConfig.hKM = ppks->hKM;
			ppks->gmConfig.hWndStatusBar = ppks->kmConfig.hWndStatusBar;
			PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
			PGPcomdlgSetSplashParent (hWnd);
			PostMessage (hWnd, KM_M_RELOADKEYRINGS, 0, 0);
		}
		else {
			PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, ppks->KeySetMain);
			ppks->iStatusValue = -1;
			InvalidateRect (ppks->kmConfig.hWndStatusBar, NULL, FALSE);
		}
		return 0;

	case WM_CLOSE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bMainWindow) {
			if (bKeyHasBeenGenerated) {
				if (!PGPkeysBackupWarn (hWnd, &bMakeBackups)) break;
			}
			PGPkeysSetWindowRegistryData (hWnd, 
					ppks->bGroupsVisible, ppks->iGroupsPercent);
			WinHelp (hWnd, g_szHelpFile, HELP_QUIT, 0);
			if (hwndOpenSearch) {
				SendMessage (hwndOpenSearch, WM_CLOSE, 0, 0);
			}
		}
		else {
			KillTimer (hWnd, LEDTIMER);
			hwndOpenSearch = NULL;
		}
		DestroyWindow (hWnd);
		return 0;

	case WM_ACTIVATE :
		if (LOWORD (wParam) != WA_INACTIVE) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			if (ppks->bMainWindow) {
				if (ppks->bGroupsFocused) SetFocus (ppks->hWndTreeListGroups);
				else SetFocus (ppks->hWndTreeList);
			}
			else {
				SetFocus (ppks->hWndTreeList);
			}
		}
		break;

	case WM_ENABLE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bMainWindow) PKEnableSearchDlgList ((BOOL)wParam);
		if (wParam)
			ppks->kmConfig.ulOptionFlags &= ~KMF_DISABLEKEYPROPS;
		else
			ppks->kmConfig.ulOptionFlags |= KMF_DISABLEKEYPROPS;
		ppks->kmConfig.ulMask = PGPKM_OPTIONS;
		PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
		break;

	case WM_DESTROY :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		if (ppks->hGM) {
			PGPgmDestroyGroupManager (ppks->hGM);
			ppks->hGM = NULL;
		}
	
		PGPkmDestroyKeyManager (ppks->hKM, ppks->bMainWindow);
		PGPgmDestroyGroupManager (ppks->hGM);

		if (ppks->bMainWindow) {
			if (ppks->KeySetMain) PGPFreeKeySet (ppks->KeySetMain);
			if (bMakeBackups) PGPkeysBackup (hWnd);
			PostQuitMessage(0);
		}
		else {
			PKRemoveSearchDlgFromList (hWnd);
			if (ppks->KeySetDisp) PGPFreeKeySet (ppks->KeySetDisp);
		}
		pkFree (ppks);
		break;

	case WM_INITMENU :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if ((HMENU)wParam == ppks->hMenuKeyMan) 
			PGPkeysSetMainMenu (ppks);
		break;

	case WM_SIZE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bGroupsVisible) {
			ppks->iGroupsHeight = HIWORD(lParam) * ppks->iGroupsPercent;
			ppks->iGroupsHeight /= 100;
			ppks->iKeysHeight = 
				HIWORD(lParam)-ppks->iToolHeight-STATUSBARHEIGHT-
				ppks->iGroupsHeight-GRABBARHEIGHT;
			SetWindowPos (ppks->hWndTreeList, NULL, 
				0, 0, 
				LOWORD(lParam), 
				ppks->iKeysHeight,
				SWP_NOMOVE|SWP_NOZORDER);
			SetWindowPos (ppks->hWndTreeListGroups, NULL, 0,
				HIWORD(lParam)-ppks->iGroupsHeight-STATUSBARHEIGHT,
				LOWORD(lParam),
				ppks->iGroupsHeight,
				SWP_NOZORDER);
		}
		else {
			ppks->iKeysHeight = 
				HIWORD(lParam)-ppks->iToolHeight-STATUSBARHEIGHT;

			SetWindowPos (ppks->hWndTreeList, NULL, 
				0, 0, 
				LOWORD(lParam), 
				ppks->iKeysHeight,
				SWP_NOMOVE|SWP_NOZORDER);
		}

		SendMessage (ppks->kmConfig.hWndStatusBar, WM_SIZE, wParam, lParam);
		iWidths[0] = LOWORD(lParam)-TOTALLEDWIDTH-16;
		iWidths[1] = LOWORD(lParam)-16;
		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETPARTS, 2, 
						(LPARAM)iWidths);

		if (ppks->hWndSearchControl) {
			SendMessage (ppks->hWndSearchControl, WM_SIZE, wParam, 
				MAKELPARAM (LOWORD(lParam), ppks->iToolHeight));
			InvalidateRect (ppks->hWndSearchControl, NULL, FALSE);
		}
		return 0;

	case WM_MOUSEMOVE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bGroupsVisible) {
			INT		iY, iVSize;
			iY = HIWORD(lParam);
			if (ppks->bGrabbed) {
				GetClientRect (hWnd, &rc);
				iVSize = rc.bottom-rc.top;
				if (iVSize <= 0) break;

				iY = (iVSize-STATUSBARHEIGHT-GRABBARHEIGHT) -iY;
				if ((iY > 50) && (iY < (iVSize -50))) {
					ppks->iGroupsPercent = (100 * iY) / iVSize;
					ppks->iGroupsHeight = iY;
				}

				ppks->iKeysHeight = 
					iVSize-ppks->iToolHeight-STATUSBARHEIGHT-
					ppks->iGroupsHeight-GRABBARHEIGHT;
				SetWindowPos (ppks->hWndTreeList, NULL, 
					0, 0, 
					rc.right-rc.left, 
					ppks->iKeysHeight,
					SWP_NOMOVE|SWP_NOZORDER);
				SetWindowPos (ppks->hWndTreeListGroups, NULL, 0,
					iVSize-ppks->iGroupsHeight-STATUSBARHEIGHT,
					rc.right-rc.left,
					ppks->iGroupsHeight,
					SWP_NOZORDER);
			}
			else {
				if ((iY >= ppks->iKeysHeight-2) && 
					(iY <= ppks->iKeysHeight+GRABBARHEIGHT+2)) {
					ppks->bGrabEnabled = TRUE;
					ppks->hCursorOld = 
						SetCursor (LoadCursor (NULL, IDC_SIZENS));
					SetCapture (hWnd);
				}
				else {
					if (ppks->bGrabEnabled) {
						ppks->bGrabEnabled = FALSE;
						SetCursor (ppks->hCursorOld);
						ReleaseCapture ();
					}
				}
			}
		}
		break;

	case WM_LBUTTONDOWN :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bGrabEnabled) {
			ppks->bGrabbed = TRUE;
		}
		break;

	case WM_LBUTTONUP :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bGrabbed) {
			ppks->bGrabbed = FALSE;
		}
		break;

	case WM_SYSCOLORCHANGE :
	case WM_SETTINGCHANGE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PostMessage (ppks->hWndTreeList, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_DRAWITEM :
		if (wParam == IDC_STATUSBAR) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			DrawStatus ((LPDRAWITEMSTRUCT)lParam, ppks);
			return TRUE;
		}
		break;

	case WM_COPYDATA :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		return (ImportData (ppks, (PCOPYDATASTRUCT)lParam));

	case WM_NOTIFY :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		// did it come from the keys treelist ?
		if (wParam == IDC_TREELIST) {
			if (pnmh->code == TLN_SELCHANGED) {
				PGPgmPerformAction (ppks->hGM, GM_UNSELECTALL);
			}

			PGPkmDefaultNotificationProc (ppks->hKM, lParam);

			switch (pnmh->code) {
			case TLN_CONTEXTMENU :
				ppks->bGroupsFocused = FALSE;
				ppks->uKeySelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				PGPkeysContextMenu (ppks, hWnd, 
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			case TLN_SELCHANGED :
				ppks->bGroupsFocused = FALSE;
				ppks->uKeySelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				break;

			case TLN_KEYDOWN :
				if (ppks->bGroupsVisible) {
					switch (((TL_KEYDOWN*)lParam)->wVKey) {
					case VK_TAB :
						SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
									0, (LPARAM)"");
						ppks->bGroupsFocused = !ppks->bGroupsFocused;
						if (ppks->bGroupsFocused) 
							SetFocus (ppks->hWndTreeListGroups);
						else 
							SetFocus (ppks->hWndTreeList);
						break;
					}
				}
				break;

			default :
				break;
			}
		}

		// did it come from the group treelist ?
		else if (wParam == IDC_TREELISTGROUP) {
			if (pnmh->code == TLN_SELCHANGED) {
				PGPkmPerformAction (ppks->hKM, KM_UNSELECTALL);
			}

			PGPgmDefaultNotificationProc (ppks->hGM, lParam);

			switch (pnmh->code) {
			case TLN_CONTEXTMENU :
				ppks->bGroupsFocused = TRUE;
				ppks->uGroupSelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				PGPkeysContextMenu (ppks, hWnd, 
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			case TLN_SELCHANGED :
				ppks->bGroupsFocused = TRUE;
				ppks->uGroupSelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				break;

			case TLN_KEYDOWN :
				switch (((TL_KEYDOWN*)lParam)->wVKey) {
				case VK_TAB :
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 0, 
									(LPARAM)"");
					ppks->bGroupsFocused = !ppks->bGroupsFocused;
					if (ppks->bGroupsFocused) 
						SetFocus (ppks->hWndTreeListGroups);
					else 
						SetFocus (ppks->hWndTreeList);
					break;
				}
				break;

			default :
				break;
			}
		}

		// did it come from the search window keyserver code ?
		else if ((ppks->hWndSearchControl) &&
				 (pnmh->hwndFrom == ppks->hWndSearchControl)) {
			switch (pnmh->code)
			{
				case SEARCH_REQUEST_SIZING: 
				{
					PSIZEREQUEST pRequest = (PSIZEREQUEST)lParam;
					RECT rc;
					GetClientRect (ppks->hWndTreeList, &rc);
					if ((rc.bottom - rc.top) > (pRequest->delta + 50))
						return TRUE;
					else
						return FALSE;
				}

				case SEARCH_SIZING: 
				{
					PSIZEREQUEST pRequest = (PSIZEREQUEST)lParam;
					RECT rcTool, rcMain;
					GetWindowRect (pnmh->hwndFrom, &rcTool);
					ppks->iToolHeight = rcTool.bottom-rcTool.top;

					GetClientRect (hWnd, &rcMain);

					SetWindowPos (ppks->hWndTreeList, NULL, 
						0, ppks->iToolHeight, 
						rcMain.right-rcMain.left, 
						rcMain.bottom-rcMain.top
							-ppks->iToolHeight
							-STATUSBARHEIGHT, 
						SWP_NOZORDER);
					InvalidateRect (hWnd, NULL, FALSE);
					break;
				}

				case SEARCH_DISPLAY_KEYSET: 
				{
					PSEARCHRESULT	pResult = (PSEARCHRESULT)lParam;
					PGPKeySetRef	KeySetPrevious;
					HCURSOR			hCursorOld;
					CHAR			sz[64];

					// stop LEDs, if going
					if (ppks->bSearchInProgress) {
						KillTimer (hWnd, LEDTIMER);
						ppks->bSearchInProgress = FALSE;
						ppks->iStatusValue = -1;
						InvalidateRect (ppks->kmConfig.hWndStatusBar, 
											NULL, FALSE);
						UpdateWindow (ppks->kmConfig.hWndStatusBar);
					}

					// if error, post message and drop out
					if (IsPGPError (pResult->error) && 
						(pResult->error != kPGPError_ServerTooManyResults)) {
						KillTimer (hWnd, LEDTIMER);
						ppks->bSearchInProgress = FALSE;
						ppks->iStatusValue = -1;
						InvalidateRect (ppks->kmConfig.hWndStatusBar, 
											NULL, FALSE);
						PGPcomdlgErrorMessage (pResult->error);
						if (pResult->error == kPGPError_ServerSocketError) {
							CloseSearch ();
							InitSearch ();
						}
						break;
					}

					// load new keyset into window
					LoadString (g_hInst, IDS_SYNCINGKEYSETS, sz, sizeof(sz));
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 0, 
									(LPARAM)sz);
					UpdateWindow (ppks->kmConfig.hWndStatusBar);

					hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));

					KeySetPrevious = ppks->KeySetDisp;
					ppks->KeySetDisp = (PGPKeySetRef)(pResult->pData);

//					PGPcomdlgSyncKeySets (g_ContextRef, ppks->KeySetMain, 
//											ppks->KeySetDisp);

					if (pResult->flags & FLAG_SEARCH_LOCAL_KEYSET) {
						ppks->bLocalKeySet = TRUE;
						ppks->kmConfig.lpszPutKeyserver = NULL;
						ppks->kmConfig.ulOptionFlags = 
							KMF_ENABLERELOADS |
							KMF_ENABLECOMMITS |
							KMF_ENABLEDRAGOUT;
						if (!g_bExpertMode)
							ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
						if (g_bMarginalAsInvalid) 
							ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
						if (g_bReadOnly)
							ppks->kmConfig.ulOptionFlags |= KMF_READONLY;
						ppks->kmConfig.ulDisableActions = 
							KM_ADDTOMAIN|KM_IMPORT|KM_PASTE|
							KM_SETASPRIMARY|KM_SETASDEFAULT;
					}
					else {
						ppks->bLocalKeySet = FALSE;
						ppks->kmConfig.lpszPutKeyserver = pResult->url;
						ppks->kmConfig.ulOptionFlags = 
							KMF_ENABLEDRAGOUT;
						if (!g_bExpertMode)
							ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
						if (g_bMarginalAsInvalid) 
							ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
						if (pResult->flags & FLAG_AREA_PENDING) 
							ppks->kmConfig.ulOptionFlags |= KMF_PENDINGBUCKET;
						ppks->kmConfig.ulDisableActions = 
							KM_IMPORT|KM_PASTE|
							KM_SETASPRIMARY|KM_SETASDEFAULT;
					}
					ppks->kmConfig.ulMask = PGPKM_OPTIONS |
											PGPKM_KEYSERVER |
											PGPKM_DISABLEFLAGS;
					PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));

					// display keyset
					PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, 
												ppks->KeySetMain);

					// sync downloaded keyset with main and redisplay
					PGPcomdlgSyncKeySets (g_ContextRef, ppks->KeySetMain, 
											ppks->KeySetDisp);
					PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, 
												ppks->KeySetMain);

					DisplayKSKeyResult (ppks->KeySetDisp, 
										ppks->kmConfig.hWndStatusBar,
										pResult->error);

					// stop LEDs, if going
					if (ppks->bSearchInProgress) {
						KillTimer (hWnd, LEDTIMER);
						ppks->bSearchInProgress = FALSE;
						ppks->iStatusValue = -1;
						InvalidateRect (ppks->kmConfig.hWndStatusBar, 
											NULL, FALSE);
					}

					if (KeySetPrevious) PGPFreeKeySet (KeySetPrevious);
					SetCursor (hCursorOld);

					break;
				}

				case SEARCH_PROGRESS :
				{
					PSEARCHPROGRESS pProgress = (PSEARCHPROGRESS)lParam;

					if (pProgress->total) {
						if (!ppks->bSearchInProgress) {
							ppks->bSearchInProgress = TRUE;
							ppks->iStatusValue = 0;
							if (pProgress->step == SEARCH_PROGRESS_INFINITE) {
								ppks->iStatusDirection = 1;
								SetTimer (hWnd, LEDTIMER, 
													LEDTIMERPERIOD, NULL);
							}
							else {
								ppks->iStatusDirection = 0;
								ppks->iStatusValue = 0;
							}
						}
						else {
							if (pProgress->step != SEARCH_PROGRESS_INFINITE) {
								ppks->iStatusDirection = 0;
								ppks->iStatusValue = 
									(pProgress->step * 9) / pProgress->total;
								InvalidateRect (ppks->kmConfig.hWndStatusBar, 
													NULL, FALSE);
							}
						}
					}
					else {
						if (ppks->bSearchInProgress) {
							KillTimer (hWnd, LEDTIMER);
							ppks->bSearchInProgress = FALSE;
							ppks->iStatusValue = -1;
							InvalidateRect (ppks->kmConfig.hWndStatusBar, 
											NULL, FALSE);
						}
					}

					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 0, 
									(LPARAM)(pProgress->message));
					break;
				}

				case SEARCH_ABORT :
				{
					PSEARCHABORT pAbort = (PSEARCHABORT)lParam;
					CHAR sz[256];
					if (ppks->bSearchInProgress) {
						KillTimer (hWnd, LEDTIMER);
						ppks->bSearchInProgress = FALSE;
						ppks->iStatusValue = -1;
						InvalidateRect (ppks->kmConfig.hWndStatusBar, 
											NULL, FALSE);
					}
					LoadString (g_hInst, IDS_SEARCHABORTED, 
									sz, sizeof(sz));
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT,
									0, (LPARAM)sz);
					break;
				}

				default :
					break;
			}

			break;
		}
		}
		break;

	case WM_TIMER :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		// it's time to load the keyring from disk
		if (wParam == RELOADTIMERID) {
			KillTimer (hWnd, wParam);

			// do the actual loading
			bIgnoreReloads = TRUE;
			ReloadKeyrings (ppks);
			bIgnoreReloads = FALSE;

			// do some first-time post-processing
			if (bFirstKeyringLoad) {
				bFirstKeyringLoad = FALSE;

				// import key files if passed from the command line
				ProcessFileList (GetCommandLine (), TRUE, TRUE,
									ppks->KeySetMain);

				// import keys from prefs file if available
				if (IsntPGPError (ImportPrefsKeys (ppks->KeySetMain)))
					PGPkmReLoadKeySet (ppks->hKM, FALSE);

				// display groups if groups were displayed last session
				if (g_bShowGroups) PostMessage (hWnd, WM_COMMAND,
							MAKEWPARAM (IDM_VIEWGROUPS, 0), 0);

				// check if we are to run the keygen wizard
				if (g_bKeyGenEnabled) {
					PGPBoolean	bKeyGenerated;
					PGPPrefRef	prefref;

					PGPcomdlgOpenClientPrefs (&prefref);
					PGPGetPrefBoolean (prefref, 
								kPGPPrefFirstKeyGenerated, &bKeyGenerated);
					PGPcomdlgCloseClientPrefs (prefref, FALSE);
					if (!bKeyGenerated) {
						if (!PGPkeysCheckForSecretKeys (ppks->KeySetMain)) {
							ShowWindow (g_hWndMain, SW_HIDE);
							PGPkeysCreateKey (g_hWndMain, ppks->KeySetMain);
						}
					}
				}
			}
		}
		else {
			ppks->iStatusValue += ppks->iStatusDirection;
			if (ppks->iStatusValue <= 0) {
				ppks->iStatusValue = 0;
				ppks->iStatusDirection = 1;
			}
			else if (ppks->iStatusValue >= NUMLEDS-1) {
				ppks->iStatusValue = NUMLEDS-1;
				ppks->iStatusDirection = -1;
			}
			InvalidateRect (ppks->kmConfig.hWndStatusBar, NULL, FALSE);
		}
		break;

	case KM_M_RELOADKEYRINGS : 
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bMainWindow) {
			PGPkmLoadKeySet (ppks->hKM, NULL, NULL);
			if (ppks->KeySetMain) {
				PGPFreeKeySet (ppks->KeySetMain);
				ppks->KeySetMain = NULL;
			}
			SetTimer (hWnd, RELOADTIMERID, RELOADDELAY, NULL);
		}
		else {
			if (wParam) {
				PGPkmLoadKeySet (ppks->hKM, 
									ppks->KeySetDisp, ppks->KeySetMain);
			}
		}
		break;

	case KM_M_CREATEDONE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		ShowWindow (hWnd, SW_SHOW);
		EnableWindow (hWnd, TRUE);
		SetForegroundWindow (hWnd);
		if (wParam & KCD_NEWKEY) {
			PGPPrefRef PrefRef;
			PGPcomdlgOpenClientPrefs (&PrefRef);
			PGPSetPrefBoolean (PrefRef, kPGPPrefFirstKeyGenerated, 
											(Boolean)TRUE);
			PGPcomdlgCloseClientPrefs (PrefRef, TRUE);
			bKeyHasBeenGenerated = TRUE;
			PGPkeysCommitKeyRingChanges (ppks->KeySetMain, TRUE);
			PGPkmReLoadKeySet (ppks->hKM, TRUE);
			if (wParam & KCD_NEWDEFKEY) 
				PGPkmPerformAction (ppks->hKM, KM_SETASDEFAULT);
		}
		return TRUE;

	case KM_M_REQUESTSDKACCESS :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PGPkmSynchronizeThreadAccessToSDK (ppks->hKM);
		break;

	case KM_M_UPDATEKEYINTREE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PGPkmUpdateKeyInTree (ppks->hKM, (PGPKeyRef)lParam);
		break;

	case WM_COMMAND:
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		switch (LOWORD(wParam)) {
		case IDM_FILEEXIT:
			SendMessage (hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_HELPABOUT :
			PGPkeysHelpAbout (hWnd);
			break;

		case IDM_HELPTOPICS :
			WinHelp (hWnd, g_szHelpFile, HELP_FINDER, 0);
			break;

		case IDM_REGISTER : 
			PGPkeysRegister ();
			break;
		
		case IDM_PROPERTIES :
			PGPkeysPGPPreferences (ppks, hWnd, 0);
			break;

		case IDM_CERTIFYKEY :
			PGPkmPerformAction (ppks->hKM, KM_CERTIFY); 
			break;

		case IDM_DISABLEKEY :
			if (!ppks->bMainWindow && !ppks->bLocalKeySet &&
				(ppks->uKeySelectionFlags == PGPKM_KEYFLAG))
				PGPkmPerformAction (ppks->hKM, KM_DISABLEONSERVER); 
			else 
				PGPkmPerformAction (ppks->hKM, KM_DISABLE); 
			break;

		case IDM_ENABLEKEY :
			PGPkmPerformAction (ppks->hKM, KM_ENABLE); 
			break;

		case IDM_DELETEKEY :
			if (ppks->bGroupsFocused) 
				PGPgmPerformAction (ppks->hGM, GM_DELETE); 
			else
				PGPkmPerformAction (ppks->hKM, KM_DELETE); 
			break;

		case IDM_DELETESERVER :
			PGPkmPerformAction (ppks->hKM, KM_DELETEFROMSERVER); 
			break;

		case IDM_CREATEKEY :
			if (g_bKeyGenEnabled) {
				PGPkeysCreateKey (hWnd, ppks->KeySetMain);
			}
			break;

		case IDM_SETASDEFAULT :
			if (PGPkmIsActionEnabled (ppks->hKM, KM_SETASPRIMARY))
				PGPkmPerformAction (ppks->hKM, KM_SETASPRIMARY); 
			else
				PGPkmPerformAction (ppks->hKM, KM_SETASDEFAULT); 
			break;

		case IDM_KEYPROPERTIES :
			if (ppks->bGroupsFocused) {
				PGPgmPerformAction (ppks->hGM, GM_LOCATEKEYS);
				PGPkmPerformAction (ppks->hKM, KM_PROPERTIES); 
			}
			else 
				PGPkmPerformAction (ppks->hKM, KM_PROPERTIES); 
			break;
		
		case IDM_GROUPPROPERTIES :
			PGPgmPerformAction (ppks->hGM, GM_PROPERTIES);
			break;
		
		case IDM_IMPORTKEYS :
			PGPkmPerformAction (ppks->hKM, KM_IMPORT); 
			break;

		case IDM_IMPORTGROUPS :
			PGPgmPerformAction (ppks->hGM, GM_IMPORTGROUPS); 
			break;
		
		case IDM_EXPORTKEYS :
			PGPkmPerformAction (ppks->hKM, KM_EXPORT); 
			break;

		case IDM_GETFROMSERVER :
			if (ppks->bGroupsFocused)
				PGPgmPerformAction (ppks->hGM, GM_GETFROMSERVER);
			else
				PGPkmPerformAction (ppks->hKM, KM_GETFROMSERVER);
			break;

		case IDM_ADDTOMAIN :
			PGPkmPerformAction (ppks->hKM, KM_ADDTOMAIN);
			break;

		case IDM_EXPANDSEL :
			if (ppks->bGroupsFocused) {
				if (PGPgmIsActionEnabled (ppks->hGM, GM_EXPANDSEL))
					PGPgmPerformAction (ppks->hGM, GM_EXPANDSEL); 
				else
					PGPgmPerformAction (ppks->hGM, GM_EXPANDALL); 
			}
			else {
				if (PGPkmIsActionEnabled (ppks->hKM, KM_EXPANDSEL))
					PGPkmPerformAction (ppks->hKM, KM_EXPANDSEL); 
				else
					PGPkmPerformAction (ppks->hKM, KM_EXPANDALL); 
			}
			break;

		case IDM_COLLAPSESEL :
			if (ppks->bGroupsFocused) {
				if (PGPgmIsActionEnabled (ppks->hGM, GM_COLLAPSESEL))
					PGPgmPerformAction (ppks->hGM, GM_COLLAPSESEL); 
				else
					PGPgmPerformAction (ppks->hGM, GM_COLLAPSEALL); 
			}
			else {
				if (PGPkmIsActionEnabled (ppks->hKM, KM_COLLAPSESEL))
					PGPkmPerformAction (ppks->hKM, KM_COLLAPSESEL); 
				else
					PGPkmPerformAction (ppks->hKM, KM_COLLAPSEALL); 
			}
			break;

		case IDM_SELECTALL :
			if (ppks->bGroupsFocused)
				PGPgmPerformAction (ppks->hGM, GM_SELECTALL); 
			else
				PGPkmPerformAction (ppks->hKM, KM_SELECTALL); 
			break;

		case IDM_REVOKEKEY :
			PGPkmPerformAction (ppks->hKM, KM_REVOKE); 
			break;

		case IDM_COPYKEY :
			PGPkmPerformAction (ppks->hKM, KM_COPY); 
			break;

		case IDM_PASTEKEY :
			if (ppks->bGroupsFocused) 
				PGPgmPerformAction (ppks->hGM, GM_PASTE); 
			else 
				PGPkmPerformAction (ppks->hKM, KM_PASTE); 
			break;

		case IDM_ADDUSERID :
			PGPkmPerformAction (ppks->hKM, KM_ADDUSERID); 
			break;

		case IDM_SELECTCOLUMNS :
			PGPkmSelectColumns (ppks->hKM, TRUE);
			PKReloadSearchDlgList ();
			break;

		case IDM_FINDKEY :
			PGPgmPerformAction (ppks->hGM, GM_LOCATEKEYS);
			ppks->bGroupsFocused = FALSE;
			SetFocus (ppks->hWndTreeList);
			break;

		case IDM_NEWGROUP :
			PGPgmNewGroup (ppks->hGM);
			break;

		case IDM_VIEWGROUPS :
			if (ppks->bGroupsVisible) {
				ppks->bGroupsVisible = FALSE;
				CheckMenuItem (ppks->hMenuKeyMan, IDM_VIEWGROUPS, 
															MF_UNCHECKED);
				ppks->iGroupsHeight = 0;
				GetClientRect (hWnd, &rc);
				ppks->iKeysHeight = 
					(rc.bottom-rc.top)-ppks->iToolHeight-STATUSBARHEIGHT;
				SetWindowPos (ppks->hWndTreeList, NULL, 
					0, 0, 
					(rc.right-rc.left), 
					ppks->iKeysHeight, 
					SWP_NOMOVE|SWP_NOZORDER);
				SetWindowPos (ppks->hWndTreeListGroups, NULL, 
						0, 
						(rc.bottom-rc.top)-ppks->iGroupsHeight-
													STATUSBARHEIGHT,
						(rc.right-rc.left),
						ppks->iGroupsHeight,
						SWP_NOZORDER);
			}
			else {
				ppks->bGroupsVisible = TRUE;
				PGPgmReLoadGroups (ppks->hGM);
				CheckMenuItem (ppks->hMenuKeyMan, IDM_VIEWGROUPS, MF_CHECKED);
				GetClientRect (hWnd, &rc);
				ppks->iGroupsHeight = 
					(rc.bottom-rc.top) * ppks->iGroupsPercent;
				ppks->iGroupsHeight /= 100;
				ppks->iKeysHeight = 
					(rc.bottom-rc.top)-ppks->iToolHeight-STATUSBARHEIGHT-
					ppks->iGroupsHeight-GRABBARHEIGHT;
				SetWindowPos (ppks->hWndTreeList, NULL, 
					0, 0, 
					(rc.right-rc.left), 
					ppks->iKeysHeight, 
					SWP_NOMOVE|SWP_NOZORDER);
				SetWindowPos (ppks->hWndTreeListGroups, NULL, 
						0, 
						(rc.bottom-rc.top)-ppks->iGroupsHeight-
													STATUSBARHEIGHT,
						(rc.right-rc.left),
						ppks->iGroupsHeight,
						SWP_NOZORDER);
			}
			break;

		case IDM_SEARCH :
			{
				RECT	rc;
				INT		iHeight, iWidth;
				CHAR	sz[64];

				// if search window already exists, move to front
				if (hwndOpenSearch) {
					SetForegroundWindow (hwndOpenSearch);
					break;
				}

				// save column info of main window
				PGPkmSelectColumns (ppks->hKM, FALSE);

				// create new search window
				LoadString (g_hInst, IDS_SEARCHTITLE, sz, sizeof(sz));
				GetWindowRect (hWnd, &rc);
				iWidth = rc.right - rc.left;
				if (iWidth < MINSEARCHWINDOWWIDTH)
					iWidth = MINSEARCHWINDOWWIDTH;
				iHeight = ((rc.bottom-rc.top)*3)>>2;
				if (iHeight < MINSEARCHWINDOWHEIGHT) 
					iHeight = MINSEARCHWINDOWHEIGHT;
				if (rc.left+60+iWidth > GetSystemMetrics (SM_CXSCREEN))
					rc.left = GetSystemMetrics (SM_CXSCREEN) - iWidth - 60;
				CreateWindow (WINCLASSNAME, sz, 
					WS_OVERLAPPEDWINDOW|WS_VISIBLE, rc.left+60, 
					rc.top+60, iWidth,
					iHeight, NULL, NULL, g_hInst, 
					(LPVOID)(ppks->KeySetMain));
				}
			break;

		default :
			if (LOWORD (wParam) >= IDM_DEFAULTKEYSERVER) {
				CHAR	szServer[256];
				PKGetServerFromID (LOWORD (wParam), szServer, 
										sizeof(szServer));
				ppks->kmConfig.lpszPutKeyserver = szServer;
				ppks->kmConfig.ulMask = PGPKM_KEYSERVER;
				PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
				PGPkmPerformAction (ppks->hKM, KM_SENDTOSERVER);
			}
			break;
		}
		break;

	default :
		if (uMsg == g_uReloadKeyringMessage) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			if (ppks->kmConfig.ulOptionFlags & KMF_ENABLERELOADS) {
				if (ppks->bMainWindow) {
					if ((DWORD)lParam != GetCurrentProcessId ()) {
						if (!bIgnoreReloads)
							PostMessage (hWnd, KM_M_RELOADKEYRINGS, 0, 0);
					}
					else {
						if (LOWORD (wParam) != LOWORD (hWnd)) {
							if (HIWORD (wParam))
								PGPkmReLoadKeySet (ppks->hKM, TRUE);
							else 
								PGPkmLoadKeySet (ppks->hKM, ppks->KeySetMain, 
													ppks->KeySetMain);
						}
						if (ppks->bGroupsVisible)
							PGPgmReLoadGroups (ppks->hGM);
					}
				}
				else {
					if (((DWORD)lParam != GetCurrentProcessId ()) ||
						(LOWORD (wParam) != LOWORD (hWnd))) {
						ppks->KeySetDisp = NULL;
						PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, 
													ppks->KeySetMain);
						SendMessage (ppks->hWndSearchControl, 
							SEARCH_SET_CURRENT_SEARCH, 0, 0);
					}
				}
			}
			return TRUE;
		}
		else return (DefWindowProc (hWnd, uMsg, wParam, lParam));
	}
	return FALSE;
}

