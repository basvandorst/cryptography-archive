/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PKMsgPrc - main message processing and associated routines
	

	$Id: PKMsgPrc.c,v 1.182 1999/05/03 22:17:00 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

// project header files
#include "pgpkeysx.h"
#include "search.h"
#include "PGPpk.h"
#include "pgpImage.h"

// pgp header files
#include "pgpKeyserverPrefs.h" 
#include "pgpSDKPrefs.h"

// system header files
#include <zmouse.h>

// constant defitions
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

#define LOCK_PANE			0
#define STATUS_MSG_PANE		1
#define PROGRESS_PANE		2

#define TLS_NOSEARCHYET			0
#define TLS_NOTAUTHENTICATED	1
#define TLS_AUTHENTICATED		2

#define LOCKWIDTH			18

#define STATUSBARHEIGHT		20
#define SEARCHCONTROLHEIGHT	100

#define GRABBARHEIGHT		2

#define MINSEARCHWINDOWHEIGHT	260
#define MINSEARCHWINDOWWIDTH	460

// External globals
extern HINSTANCE		g_hInst;
extern HWND				g_hWndMain;
extern CHAR				g_szHelpFile[MAX_PATH];
extern UINT				g_uReloadPrefsMessage;
extern UINT				g_uReloadKeyringMessage;
extern UINT				g_uReloadKeyserverPrefsMessage;
extern UINT				g_uPurgePassphraseCacheMessage;
extern UINT				g_uMouseWheelMessage;
extern PGPContextRef	g_Context;
extern PGPtlsContextRef	g_TLSContext;
extern PGPBoolean		g_bExpertMode;
extern PGPBoolean		g_bKeyGenEnabled;
extern PGPBoolean		g_bMarginalAsInvalid;
extern BOOL				g_bReadOnly;
extern BOOL				g_bShowGroups;
extern INT				g_iGroupsPercent;
extern INT				g_iToolHeight;

// Local globals
static BOOL			bKeyHasBeenGenerated = FALSE;
static BOOL			bMakeBackups = FALSE;
static BOOL			bIgnoreReloads = FALSE;
static HWND			hwndOpenSearch = NULL;
static BOOL			bFirstKeyringLoad = TRUE;
static ULONG		ulColumns = 0;
static HIMAGELIST	himlLocks = NULL;


//	____________________________________
//
//  Look for secret keys

static BOOL 
sPKCheckForSecretKeys (PGPKeySetRef KeySet) 
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

static VOID 
sProcessFileList (LPSTR			pszList, 
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

			PGPNewFileSpecFromFullPath (g_Context, p, &fileref);
			if (fileref) {
				PGPImportKeySet (g_Context, &keysetToAdd, 
								PGPOInputFile (g_Context, fileref),
								PGPOLastOption (g_Context));
				if (keysetToAdd) {
					PGPCountKeys (keysetToAdd, &iNumKeys);
					if (iNumKeys > 0) {
						if (bAllowSelect) {
							PGPclQueryAddKeys (g_Context, g_TLSContext,
								g_hWndMain, keysetToAdd, keysetMain);
						}
						else {
							PGPAddKeys (keysetToAdd, keysetMain);
							PKCommitKeyRingChanges (keysetMain, TRUE);
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

static BOOL 
sImportData (PGPKEYSSTRUCT* ppks, PCOPYDATASTRUCT pcds) 
{
	PGPKeySetRef	keyset		= kInvalidPGPKeySetRef;
	BOOL			bPrompt;
	PGPError		err;


	bPrompt = pcds->dwData & PGPPK_SELECTIVEIMPORT;

	switch (pcds->dwData & PGPPK_IMPORTKEYMASK) {
	case PGPPK_IMPORTKEYBUFFER :
		err = PGPImportKeySet (g_Context, &keyset, 
				PGPOInputBuffer (g_Context, pcds->lpData, pcds->cbData),
				PGPOLastOption (g_Context));
		if (IsntPGPError (err) && PGPKeySetRefIsValid (keyset)) {
			if (bPrompt) {
				PGPclQueryAddKeys (g_Context, g_TLSContext, 
								g_hWndMain, keyset, ppks->KeySetMain);
			}
			else {
				err = PGPAddKeys (keyset, ppks->KeySetMain);
				PKCommitKeyRingChanges (ppks->KeySetMain, TRUE);
				PGPkmReLoadKeySet (ppks->hKM, TRUE);
			}
		}
		if (PGPKeySetRefIsValid (keyset)) 
			PGPFreeKeySet (keyset);
		return (IsntPGPError (err));

	case PGPPK_IMPORTKEYFILELIST :
		sProcessFileList (pcds->lpData, FALSE, bPrompt, ppks->KeySetMain);
		return TRUE;

	case PGPPK_IMPORTKEYCOMMANDLINE :
		sProcessFileList (pcds->lpData, TRUE, bPrompt, ppks->KeySetMain);
		return TRUE;
	}

	return FALSE;
}

//	____________________________________
//
//  import keys from admin prefs file

static PGPError 
sImportPrefsKeys (PGPKeySetRef keysetMain)
{
	PGPError		err			= kPGPError_UnknownError;

#if PGP_BUSINESS_SECURITY
	PGPKeySetRef	keysetNew;
	PGPPrefRef		prefref;
	LPSTR			psz;

	err = PGPclOpenAdminPrefs (
				PGPGetContextMemoryMgr (g_Context),
				&prefref, 
				PGPclIsAdminInstall());

	if (IsntPGPError (err)) {
		err = PGPGetPrefStringAlloc (prefref, kPGPPrefDefaultKeys, &psz);
		if (IsntPGPError (err) && psz) {
			if (psz[0]) {
				err = PGPImportKeySet (g_Context, &keysetNew, 
						PGPOInputBuffer (g_Context, psz, lstrlen(psz)+1),
						PGPOLastOption (g_Context));

				if (IsntPGPError (err)) {
					PGPAddKeys (keysetNew, keysetMain);
					err = PKCommitKeyRingChanges (keysetMain, TRUE);
					PGPFreeKeySet (keysetNew);
				}
			}
			PGPDisposePrefData (prefref, psz);
		}
		PGPclCloseAdminPrefs (prefref, FALSE);
	}

#endif //PGP_BUSINESS_SECURITY

	return err;
}

//	____________________________________
//
//  Reload keyrings

static BOOL 
sReloadKeyrings (
		PGPKEYSSTRUCT*	ppks) 
{
	BOOL				bMutable			= TRUE;
	PGPKeyRingOpenFlags	flags				= kPGPKeyRingOpenFlags_Mutable;
	INT					iLockTries			= 0;

	PGPError			pgpError;
	HCURSOR				hCursorOld;

	hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
	PGPsdkLoadDefaultPrefs (g_Context);
	pgpError = PGPOpenDefaultKeyRings (g_Context, flags, 
										&(ppks->KeySetMain));
	SetCursor (hCursorOld);

	while (!PGPKeySetRefIsValid (ppks->KeySetMain) || 
		   PGPkmLoadKeySet (ppks->hKM, ppks->KeySetMain, ppks->KeySetMain)) 
	{
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
				PKMessageBox (g_hWndMain, IDS_CAPTION, IDS_LOCKEDKEYRING,
					MB_OK|MB_ICONSTOP);
				if (!PKPGPPreferences (ppks, g_hWndMain, PGPCL_KEYRINGPREFS)) 
				{
					SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
					return FALSE;
				}
			}
			break;

		case kPGPError_CantOpenFile :
		case kPGPError_FileNotFound :
			if (!PKPGPPreferences (ppks, g_hWndMain, PGPCL_KEYRINGPREFS)) 
			{
				SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
				return FALSE;
			}
			break;

		default :
			PKMessageBox (g_hWndMain, IDS_CAPTION, IDS_CORRUPTKEYRING, 
				MB_OK|MB_ICONSTOP);
			if (!PKPGPPreferences (ppks, g_hWndMain, PGPCL_KEYRINGPREFS)) 
			{
				SendMessage (g_hWndMain, WM_CLOSE, 0, 0);
				return FALSE;
			}
			break;
		}
		hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
		flags = 0;
		if (bMutable) flags |= kPGPKeyRingOpenFlags_Mutable;
		pgpError = PGPOpenDefaultKeyRings (g_Context, flags, 
											&(ppks->KeySetMain));

		SetCursor (hCursorOld);
	}

	PGPPropagateTrust (ppks->KeySetMain);
	g_bReadOnly = !bMutable;

	if (g_bReadOnly) 
	{
		ppks->kmConfig.ulOptionFlags |= KMF_READONLY;
		PKReadOnlyWarning (ppks->hWndMain);
	}
	else 
		ppks->kmConfig.ulOptionFlags &= ~KMF_READONLY;

	// pass readonly flag to keymanager
	ppks->kmConfig.ulMask = PGPKM_OPTIONS;
	PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));

	// reload the groups file
	ppks->gmConfig.keysetMain = ppks->KeySetMain;
	PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
	PGPgmLoadGroups (ppks->hGM);

	return TRUE;
}

//	____________________________________
//
//  Draw the owner-drawn part of the status bar

static VOID
sDrawStatus (
		LPDRAWITEMSTRUCT lpdis, 
		PGPKEYSSTRUCT* ppks) 
{
	HBRUSH	hBrushLit, hBrushUnlit, hBrushOld;
	HPEN	hPen, hPenOld;
	INT		i;
	INT		itop, ibot, ileft, iright;

	if(lpdis->itemID == PROGRESS_PANE)
	{
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
	else if(lpdis->itemID == LOCK_PANE)
	{
		BOOL bSecure = (BOOL) lpdis->itemData;

		if (bSecure) {
			bSecure = TRUE;
		}

		ImageList_Draw (himlLocks, (bSecure ? IDX_CLOSEDLOCK:IDX_OPENLOCK),
			lpdis->hDC, 1, 3, ILD_TRANSPARENT);

	}
}


//	____________________________________
//
//  display downloaded key count in status bar

static INT
sDisplayKSKeyResult (
		PGPKeySetRef	keyset, 
		HWND			hwnd, 
		PGPError		err) 
{
	CHAR sz1[256];
	CHAR sz2[256];
	INT	 iCount;

	if (keyset)
		(void)PGPCountKeys (keyset, &iCount);
	else 
		iCount = 0;

	if (err == kPGPError_ServerPartialSearchResults)
		LoadString (g_hInst, IDS_KSTOOMANYKEYCOUNT, sz1, sizeof(sz1));
	else 
		LoadString (g_hInst, IDS_KSKEYCOUNT, sz1, sizeof(sz1));
	wsprintf (sz2, sz1, iCount);
	SendMessage (hwnd, SB_SETTEXT, STATUS_MSG_PANE, (LPARAM)sz2);
	
	return iCount;
}

//	____________________________________
//
//  get number of wheel scroll lines and pass to treelist controls

static VOID
sUpdateWheelScrollLines (PGPKEYSSTRUCT* ppks)
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

	if (ppks->hWndTreeList)
		TreeList_SetWheelScrollLines (ppks->hWndTreeList, uLines);
	if (ppks->hWndTreeListGroups)
		TreeList_SetWheelScrollLines (ppks->hWndTreeListGroups, uLines);
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
	INT				iWidths[3];

	switch (uMsg) {

	case WM_CREATE :
		ppks = (PGPKEYSSTRUCT*)pkAlloc (sizeof (PGPKEYSSTRUCT));
		if (!ppks) return -1;
		SetWindowLong (hWnd, GWL_USERDATA, (LPARAM)ppks);

		GetClientRect (hWnd, &rc);

		// if lpCreateParams is NULL, this is the main window
		if (((LPCREATESTRUCT)lParam)->lpCreateParams == NULL) {
			ppks->hWndMain = hWnd;
			ppks->bMainWindow = TRUE;
			ppks->iToolHeight = g_iToolHeight; 
			ppks->iKeysHeight = 
				rc.bottom-rc.top-ppks->iToolHeight-STATUSBARHEIGHT;
			ppks->bGroupsVisible = FALSE;
			ppks->iGroupsPercent = g_iGroupsPercent;
			ppks->iGroupsHeight = 0;
			ppks->bLocalKeySet = TRUE;
			ppks->KeySetMain = NULL;
			ppks->KeySetDisp = NULL;
			ppks->kmConfig.ulOptionFlags = 
				KMF_ENABLECOMMITS|KMF_ENABLEDRAGOUT|
				KMF_ENABLEDROPIN|KMF_ENABLERELOADS;
			if (!g_bExpertMode) 
				ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			if (g_bMarginalAsInvalid) 
				ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			ppks->kmConfig.ulDisableActions = 
				KM_ADDTOMAIN;

			ppks->hMenuKeyserver = NULL;
			ppks->hMenuKeyMan = GetMenu (hWnd);
			PKInitMenuKeyMan (ppks->hMenuKeyMan);

			// setup group manager
			ppks->hGM = PGPgmCreateGroupManager (g_Context, g_TLSContext,
					hWnd, IDC_TREELISTGROUP,
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
			ppks->hWndToolBar = PKCreateToolbar (hWnd);
		}

		// otherwise this is a search window and lpCreateParams is 
		// the main KeySetRef
		else {
			hwndOpenSearch = hWnd;
			ppks->bMainWindow = FALSE;
			ppks->iToolHeight = SEARCHCONTROLHEIGHT;
			ppks->iKeysHeight = 
				rc.bottom-rc.top-ppks->iToolHeight-STATUSBARHEIGHT;
			ppks->bGroupsVisible = FALSE;
			ppks->iGroupsHeight = 0;
			ppks->iGroupsPercent = 0;
			ppks->bLocalKeySet = TRUE;
			ppks->KeySetMain = 
				(PGPKeySetRef)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			ppks->KeySetDisp = NULL;
			ppks->kmConfig.ulOptionFlags = 
				KMF_ENABLEDRAGOUT;
			if (!g_bExpertMode) 
				ppks->kmConfig.ulOptionFlags |= KMF_NOVICEMODE;
			if (g_bMarginalAsInvalid) 
				ppks->kmConfig.ulOptionFlags |= KMF_MARGASINVALID;
			ppks->kmConfig.ulDisableActions = KM_ALLACTIONS;
			ppks->pGroupFile = NULL;

			ppks->hMenuKeyserver = NULL;
			ppks->hMenuKeyMan = NULL;

			if (InitSearch ()) {
				ppks->hWndSearchControl = CreateSearch (g_hInst, hWnd);

				SendMessage (ppks->hWndSearchControl, SEARCH_SET_LOCAL_KEYSET,
							0, (LPARAM)(ppks->KeySetMain));
				GetWindowRect (ppks->hWndSearchControl, &rc);
				ppks->iToolHeight = rc.bottom-rc.top;
			}
			ppks->hWndToolBar = NULL;
		}

		PKDialogListFunc (hWnd, TRUE, NULL, NULL);

		ppks->bGroupsFocused = FALSE;
		ppks->uKeySelectionFlags = 0;
		ppks->uGroupSelectionFlags = 0;
		ppks->hCursorOld = NULL;
		ppks->bGrabEnabled = FALSE;
		ppks->bGrabbed = FALSE;

		ppks->hKM = PGPkmCreateKeyManagerEx (g_Context, g_TLSContext,
							hWnd, IDC_TREELIST, 
							(HWNDLISTPROC)PKDialogListFunc, 
							0, ppks->iToolHeight, 
							rc.right-rc.left, 
							ppks->iKeysHeight, 0);

		ppks->kmConfig.lpszHelpFile = NULL;
		ppks->kmConfig.keyserver.structSize = 0;
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
		
		// setup status bar
		if (ppks->bMainWindow)
			iWidths[0] = 0;
		else
			iWidths[0] = LOCKWIDTH;
		iWidths[1] = rc.right-TOTALLEDWIDTH-16;
		iWidths[2] = rc.right-16;
	
		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETPARTS, 3, 
					(LPARAM)iWidths);

		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
					LOCK_PANE|SBT_OWNERDRAW|SBT_POPOUT, 0);

		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
					PROGRESS_PANE|SBT_OWNERDRAW, 0);

		// load the lock icons
		if (himlLocks == NULL)
		{
			HDC		hDC;
			INT		iNumBits;
			HBITMAP	hBmp;

			hDC = GetDC (NULL);		// DC for desktop
			iNumBits = 
				GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
			ReleaseDC (NULL, hDC);
	
			if (iNumBits <= 8) {
				himlLocks =	
					ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
											NUM_BITMAPS, 0); 
				hBmp = 
					LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
				ImageList_AddMasked (himlLocks, hBmp, RGB(255, 0, 255));
				DeleteObject (hBmp);
			}
			else {
				himlLocks =	
					ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
											NUM_BITMAPS, 0); 
				hBmp = 
					LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
				ImageList_AddMasked (himlLocks, hBmp, RGB(255, 0, 255));
				DeleteObject (hBmp);
			}
		}

		// set the tooltip text for the lock icon (requires comctl 4.71)
///		if (!ppks->bMainWindow) {
///			CHAR szTip[32];
///			LoadString (g_hInst, IDS_LOCKICONTIP, szTip, sizeof(szTip));
///			SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTIPTEXT, 
///					(WPARAM)LOCK_PANE, (LPARAM)szTip);
///		}

		// initialize TLS info
		ppks->iTLSstatus = TLS_NOSEARCHYET;
		ppks->szTLSserver[0] = '\0';
		ppks->keyAuth = kInvalidPGPKeyRef;

		// set initial configuration of keymanager
		ppks->kmConfig.ulMask = PGPKM_ALLITEMS;
		PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
		
		ppks->hWndTreeList = PGPkmGetManagerWindow (ppks->hKM);

		if (ppks->bMainWindow) {
			PGPkmGetSelectedColumns (ppks->hKM, &ulColumns);
			ppks->gmConfig.lpszHelpFile = NULL;
			ppks->gmConfig.hKM = ppks->hKM;
			ppks->gmConfig.hWndStatusBar = ppks->kmConfig.hWndStatusBar;
			PGPgmConfigure (ppks->hGM, &(ppks->gmConfig));
			PGPclSetSplashParent (hWnd);
			PostMessage (hWnd, KM_M_RELOADKEYRINGS, 0, 0);
		}
		else {
			PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, ppks->KeySetMain);
			ppks->iStatusValue = -1;
			InvalidateRect (ppks->kmConfig.hWndStatusBar, NULL, FALSE);
		}

		// initialize enabled/disabled states of toolbar buttons
		PKSetToolbarButtonStates (ppks);

		// initialize the number of mouse wheel scroll lines
		sUpdateWheelScrollLines (ppks);

		return 0;

	case WM_CLOSE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bMainWindow) {
			UINT uReason;

			if (!PGPkmOKToClose (ppks->hKM, &uReason)) {
				if (uReason == KMR_EXISTINGSPLITKEYDLGS) {
					if (!PKSplitKeyWarn (hWnd)) 
						break;
				}
			}

			if (bKeyHasBeenGenerated) {
				if (!PKBackupWarn (hWnd, &bMakeBackups)) 
					break;
			}

			PKScheduleNextCRLUpdate (g_Context, ppks->KeySetMain);

			PKSetPrivatePrefData (hWnd, 
					ppks->bGroupsVisible, ppks->iGroupsPercent,
					ppks->iToolHeight);

			WinHelp (hWnd, g_szHelpFile, HELP_QUIT, 0);
			if (hwndOpenSearch) 
				SendMessage (hwndOpenSearch, WM_CLOSE, 0, 0);
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
				if (ppks->bGroupsFocused) 
					SetFocus (ppks->hWndTreeListGroups);
				else 
					SetFocus (ppks->hWndTreeList);
			}
			else {
				SetFocus (ppks->hWndTreeList);
			}
		}
		break;

	case WM_ENABLE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		if (ppks->bMainWindow) 
			PKEnableDlgList ((BOOL)wParam);

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
			if (PGPKeySetRefIsValid (ppks->KeySetMain))
				PGPFreeKeySet (ppks->KeySetMain);
			if (bMakeBackups) PKBackup (hWnd);
			PKDestroyToolbar (ppks->hWndToolBar);
			ImageList_Destroy (himlLocks);
			PostQuitMessage(0);
		}
		else {
			if (PGPKeySetRefIsValid (ppks->KeySetDisp))
				PGPFreeKeySet (ppks->KeySetDisp);
		}

		if (PGPKeySetRefIsValid (ppks->keysetAuth))
			PGPFreeKeySet (ppks->keysetAuth);

		PKDialogListFunc (hWnd, FALSE, NULL, NULL);
		pkFree (ppks);
		ppks=NULL;
		break;

	case WM_INITMENU :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if ((HMENU)wParam == ppks->hMenuKeyMan) 
			PKSetMainMenu (ppks);
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

		if (ppks->bMainWindow) 
			iWidths[0] = 0;
		else 
			iWidths[0] = LOCKWIDTH;
		iWidths[1] = LOWORD(lParam)-TOTALLEDWIDTH-16;
		iWidths[2] = LOWORD(lParam)-16;
		SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETPARTS, 3, 
					(LPARAM)iWidths);

		if (ppks->hWndSearchControl) {
			SendMessage (ppks->hWndSearchControl, WM_SIZE, wParam, 
				MAKELPARAM (LOWORD(lParam), ppks->iToolHeight));
			InvalidateRect (ppks->hWndSearchControl, NULL, FALSE);
		}

		if (ppks->hWndToolBar) {
			SetWindowPos (ppks->hWndToolBar, NULL, 
				0, 0, 
				LOWORD(lParam), 
				ppks->iToolHeight - TOOLBARYOFFSET,
				SWP_NOMOVE|SWP_NOZORDER); 
		}

		return 0;

	case WM_MOUSEMOVE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (ppks->bGroupsVisible) {
			INT		iY, iVSize, iYGrab;
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
				iYGrab = ppks->iToolHeight+ppks->iKeysHeight;
				if ((iY >= iYGrab-2) && 
					(iY <= iYGrab+GRABBARHEIGHT+2)) {
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
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PostMessage (ppks->hWndTreeList, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_SETTINGCHANGE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		if (wParam == SPI_SETWHEELSCROLLLINES)
			sUpdateWheelScrollLines (ppks);
		else 
			PostMessage (ppks->hWndTreeList, WM_SYSCOLORCHANGE, 0, 0);
		break;

	case WM_DRAWITEM :
		if (wParam == IDC_STATUSBAR) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			sDrawStatus ((LPDRAWITEMSTRUCT)lParam, ppks);
			return TRUE;
		}
		break;

	case WM_COPYDATA :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		sImportData (ppks, (PCOPYDATASTRUCT)lParam);
		return TRUE;

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
				PKContextMenu (ppks, hWnd, 
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			case TLN_SELCHANGED :
				ppks->bGroupsFocused = FALSE;
				ppks->uKeySelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				PKSetToolbarButtonStates (ppks);
				break;

			case TLN_KEYDOWN :
				if (ppks->bGroupsVisible) {
					switch (((TL_KEYDOWN*)lParam)->wVKey) {
					case VK_TAB :
						SendMessage (ppks->kmConfig.hWndStatusBar, 
								SB_SETTEXT, STATUS_MSG_PANE, (LPARAM)"");
						ppks->bGroupsFocused = !ppks->bGroupsFocused;
						if (ppks->bGroupsFocused) 
							SetFocus (ppks->hWndTreeListGroups);
						else 
							SetFocus (ppks->hWndTreeList);
						PKSetToolbarButtonStates (ppks);
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
				PKContextMenu (ppks, hWnd, 
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			case TLN_SELCHANGED :
				ppks->bGroupsFocused = TRUE;
				ppks->uGroupSelectionFlags = ((LPNM_TREELIST)lParam)->flags;
				PKSetToolbarButtonStates (ppks);
				break;

			case TLN_KEYDOWN :
				switch (((TL_KEYDOWN*)lParam)->wVKey) {
				case VK_TAB :
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
									STATUS_MSG_PANE, (LPARAM)"");
					ppks->bGroupsFocused = !ppks->bGroupsFocused;
					if (ppks->bGroupsFocused) 
						SetFocus (ppks->hWndTreeListGroups);
					else 
						SetFocus (ppks->hWndTreeList);
					PKSetToolbarButtonStates (ppks);
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
						(pResult->error != 
								kPGPError_ServerPartialSearchResults)) 
					{
						KillTimer (hWnd, LEDTIMER);
						ppks->bSearchInProgress = FALSE;
						ppks->iStatusValue = -1;
						SendMessage (ppks->kmConfig.hWndStatusBar, 
								SB_SETTEXT, STATUS_MSG_PANE, (LPARAM)"");
						InvalidateRect (ppks->kmConfig.hWndStatusBar, 
										NULL, FALSE);
						PGPclErrorBox (NULL, pResult->error);
						break;
					}

					// load new keyset into window
					LoadString (g_hInst, IDS_SYNCINGKEYSETS, sz, sizeof(sz));
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
									STATUS_MSG_PANE, (LPARAM)sz);
					UpdateWindow (ppks->kmConfig.hWndStatusBar);

					hCursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));

					KeySetPrevious = ppks->KeySetDisp;
					ppks->KeySetDisp = (PGPKeySetRef)(pResult->pData);

					if (pResult->flags & FLAG_SEARCH_LOCAL_KEYSET) {
						ppks->bLocalKeySet = TRUE;
						ppks->kmConfig.keyserver.structSize = 0;
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
						memcpy (&ppks->kmConfig.keyserver, 
								&pResult->keyserver, 
								sizeof(PGPKeyServerEntry));
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
					SetFocus (ppks->hWndTreeList);

					// sync downloaded keyset with main and redisplay
					PGPclSyncKeySets (g_Context, ppks->KeySetMain, 
											ppks->KeySetDisp);
					PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, 
												ppks->KeySetMain);

					sDisplayKSKeyResult (ppks->KeySetDisp, 
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

					SendMessage (ppks->hWndSearchControl, 
											SEARCH_SET_FOCUS, 0, 0);
					break;
				}

				case SEARCH_PROGRESS :
				{
					PSEARCHPROGRESS pProgress = (PSEARCHPROGRESS)lParam;

					if (pProgress->total) {
						// we're starting a query
						if (!ppks->bSearchInProgress) {
							ppks->bSearchInProgress = TRUE;
							ppks->kmConfig.ulOptionFlags |= 
													KMF_DISABLESTATUSBAR;
							ppks->kmConfig.ulMask = PGPKM_OPTIONS;
							PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));

							ppks->iStatusValue = 0;
							if (pProgress->step == SEARCH_PROGRESS_INFINITE) 
							{
								ppks->iStatusDirection = 1;
								SetTimer (hWnd, LEDTIMER, 
													LEDTIMERPERIOD, NULL);
							}
							else 
							{
								ppks->iStatusDirection = 0;
								ppks->iStatusValue = 0;
							}
							// reset icon to "unlocked"
							SendMessage (ppks->kmConfig.hWndStatusBar, 
									SB_SETTEXT, 
									LOCK_PANE|SBT_OWNERDRAW|SBT_POPOUT, 
									0);
							// signal that we're not authenticated yet
							ppks->iTLSstatus = TLS_NOTAUTHENTICATED;
						}
						else {
							if (pProgress->step != SEARCH_PROGRESS_INFINITE) 
							{
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

					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT, 
							STATUS_MSG_PANE, (LPARAM)(pProgress->message));
					break;
				}

				case SEARCH_SECURE_STATUS :
				{
					PSEARCHSECURE	pSecure = (PSEARCHSECURE)lParam;
					PGPKeySetRef	keyset;
					PGPKeyListRef	keylist;
					PGPKeyIterRef	keyiter;
					
					SendMessage (ppks->kmConfig.hWndStatusBar, SB_SETTEXT,
									LOCK_PANE|SBT_OWNERDRAW|SBT_POPOUT, 
									(LPARAM)(pSecure->secure));

					if (pSecure->secure) {
						ppks->iTLSstatus = TLS_AUTHENTICATED;

						lstrcpy (ppks->szTLSserver, pSecure->szServerName);
						ppks->tlsCipher = pSecure->tlsCipher;

						if (PGPKeySetRefIsValid (ppks->keysetAuth))
							PGPFreeKeySet (ppks->keysetAuth);
						PGPNewSingletonKeySet (pSecure->keyAuth, &keyset);
						PGPNewKeySet (g_Context, &ppks->keysetAuth);
						PGPAddKeys (keyset, ppks->keysetAuth);
						PGPCommitKeyRingChanges (ppks->keysetAuth);
						PGPFreeKeySet (keyset);
	
						PGPOrderKeySet (ppks->keysetAuth, 
										kPGPAnyOrdering, &keylist);
						PGPNewKeyIter (keylist, &keyiter);
						PGPKeyIterNext (keyiter, &ppks->keyAuth);
						PGPFreeKeyIter (keyiter);
						PGPFreeKeyList (keylist);
					}
					else 
						ppks->iTLSstatus = TLS_NOTAUTHENTICATED;
	
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
									STATUS_MSG_PANE, (LPARAM)sz);
					SendMessage (ppks->hWndSearchControl, 
											SEARCH_SET_FOCUS, 0, 0);
					break;
				}

				default :
					break;
			}

			if (!ppks->bSearchInProgress) {
				ppks->kmConfig.ulOptionFlags &= ~KMF_DISABLESTATUSBAR;
				ppks->kmConfig.ulMask = PGPKM_OPTIONS;
				PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
			}
		}

		// did it come from the status bar?
		else if ((ppks->kmConfig.hWndStatusBar) &&
				 (pnmh->hwndFrom == ppks->kmConfig.hWndStatusBar)) {
			if (pnmh->code == NM_CLICK) {
				if (!ppks->bMainWindow) {
					POINT pt;
					GetCursorPos (&pt);
					ScreenToClient (ppks->kmConfig.hWndStatusBar, &pt);
					if (pt.x <= LOCKWIDTH) {
						switch (ppks->iTLSstatus) {
						case TLS_NOSEARCHYET :
							PKMessageBox (hWnd, IDS_CAPTION, 
								IDS_NOCONNECTIONYET, 
								MB_OK|MB_ICONEXCLAMATION);
							break;

						case TLS_NOTAUTHENTICATED :
							PKMessageBox (hWnd, IDS_CAPTION, 
								IDS_CONNECTIONNOTAUTHENTICATED, 
								MB_OK|MB_ICONEXCLAMATION);
							break;

						case TLS_AUTHENTICATED :
							PGPclConfirmRemoteAuthentication (
									g_Context, hWnd, ppks->szTLSserver,
									ppks->keyAuth, ppks->tlsCipher,
									ppks->KeySetMain, 
									PGPCL_SHOWAUTHENTICATION);
							break;

						}
					}
				}
			}
		}

		// else assume it's from the tooltip control of the toolbar
		else if (pnmh->code == TTN_NEEDTEXT) {
			PKGetToolbarTooltipText ((LPTOOLTIPTEXT)lParam);
		}
		}

		break;

	case WM_PAINT :
		{
			HDC			hdc;
			PAINTSTRUCT ps;
			RECT		rc;
			HPEN		hpenOld;
			INT			iWidth;

			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

			hdc = BeginPaint (hWnd, &ps);

			if (ppks->hWndToolBar && (ppks->iToolHeight > 0)) {
				GetWindowRect (hWnd, &rc);
				iWidth = rc.right-rc.left;

				hpenOld = SelectObject (hdc, CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DSHADOW)));
				MoveToEx (hdc, 0, 0, NULL);
				LineTo (hdc, iWidth, 0);

				SelectObject (hdc, CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DHILIGHT)));
				MoveToEx (hdc, 0, 1, NULL);
				LineTo (hdc, iWidth, 1);

				SelectObject (hdc, hpenOld);
			}

			EndPaint (hWnd, &ps);
		}
		return 0;

	case WM_TIMER :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		// it's time to load the keyring from disk
		if (wParam == RELOADTIMERID) {

			KillTimer (hWnd, wParam);

			// do the actual loading
			bIgnoreReloads = TRUE;
			if (sReloadKeyrings (ppks))
			{
				// repaint window
				InvalidateRect (hWnd, NULL, FALSE);

				// do some first-time post-processing
				if (bFirstKeyringLoad) {
					PGPBoolean bUpdateAllKeys;
					PGPBoolean bUpdateTrustedIntroducers;
					PGPBoolean bUpdateCRLs;

					bFirstKeyringLoad = FALSE;

					// import key files if passed from the command line
					sProcessFileList (GetCommandLine (), TRUE, TRUE,
										ppks->KeySetMain);

					// import keys from prefs file if available
					if (IsntPGPError (sImportPrefsKeys (ppks->KeySetMain)))
						PGPkmReLoadKeySet (ppks->hKM, FALSE);

					// display groups if groups were displayed last session
					if (g_bShowGroups) 
						PostMessage (hWnd, WM_COMMAND,
										MAKEWPARAM (IDM_VIEWGROUPS, 0), 0);

					// check if we are to run the keygen wizard
					if (g_bKeyGenEnabled) {
						PGPBoolean	bKeyGenerated;
						PGPPrefRef	prefref;

						PGPclOpenClientPrefs (
									PGPGetContextMemoryMgr (g_Context), 
									&prefref);
						PGPGetPrefBoolean (prefref, 
									kPGPPrefFirstKeyGenerated, &bKeyGenerated);
						PGPclCloseClientPrefs (prefref, FALSE);
						if (!bKeyGenerated) {
							if (!sPKCheckForSecretKeys (ppks->KeySetMain)) {
								ShowWindow (g_hWndMain, SW_HIDE);
								PKCreateKey (g_hWndMain, ppks->KeySetMain);
							}
						}
					}

					// compute when we should next update CRLs
					PKScheduleNextCRLUpdate (g_Context, ppks->KeySetMain);

					// see if it's time to auto-update anything
					PGPclCheckAutoUpdate (PGPGetContextMemoryMgr (g_Context),
						TRUE, &bUpdateAllKeys, &bUpdateTrustedIntroducers,
						&bUpdateCRLs);

					if (bUpdateAllKeys) {
						if (PKAutoUpdateAllKeys (hWnd, 
											ppks->KeySetMain, FALSE))
							PGPkmReLoadKeySet (ppks->hKM, FALSE);
					}

					if (bUpdateTrustedIntroducers) {
						if (PKAutoUpdateIntroducers (hWnd, 
											ppks->KeySetMain, FALSE))
							PGPkmReLoadKeySet (ppks->hKM, TRUE);
					}

					if (bUpdateCRLs) {
						if (PKUpdateCARevocations (
									hWnd, ppks->hKM, ppks->KeySetMain))
							PGPkmReLoadKeySet (ppks->hKM, TRUE);
					}
				}

				// if there is an open search window, notify it of change
				if (ppks->bMainWindow && hwndOpenSearch)
				{
					PostMessage (hwndOpenSearch, KM_M_RELOADKEYRINGS,
									0, (LPARAM)(ppks->KeySetMain));
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
		bIgnoreReloads = FALSE;
		break;

	case KM_M_RELOADKEYRINGS : 
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		PGPkmLoadKeySet (ppks->hKM, NULL, NULL);
		if (ppks->bMainWindow) {
			if (hwndOpenSearch)
				SendMessage (hwndOpenSearch, KM_M_RELOADKEYRINGS, 0, 0);
			if (ppks->KeySetMain) {
				PGPFreeKeySet (ppks->KeySetMain);
				ppks->KeySetMain = NULL;
			}
			SetTimer (hWnd, RELOADTIMERID, RELOADDELAY, NULL);
		}
		else {
			if (PGPKeySetRefIsValid (ppks->KeySetDisp))
			{
				PGPFreeKeySet (ppks->KeySetDisp);
				ppks->KeySetDisp = NULL;
			}
			if (lParam)
				ppks->KeySetMain = (PGPKeySetRef)lParam;
			PGPkmLoadKeySet (ppks->hKM, 
								ppks->KeySetDisp, ppks->KeySetMain);
		}
		break;

	case KM_M_CREATEDONE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		ShowWindow (hWnd, SW_SHOW);
		EnableWindow (hWnd, TRUE);
		SetForegroundWindow (hWnd);
		if (ppks->bGroupsFocused) SetFocus (ppks->hWndTreeListGroups);
		else SetFocus (ppks->hWndTreeList);
		if (wParam & KCD_NEWKEY) {
			PGPPrefRef PrefRef;
			PGPclOpenClientPrefs (
					PGPGetContextMemoryMgr (g_Context), 
					&PrefRef);
			PGPSetPrefBoolean (PrefRef, kPGPPrefFirstKeyGenerated, 
											(Boolean)TRUE);
			PGPclCloseClientPrefs (PrefRef, TRUE);
			bKeyHasBeenGenerated = TRUE;
			PKCommitKeyRingChanges (ppks->KeySetMain, TRUE);
			PGPkmReLoadKeySet (ppks->hKM, TRUE);
			if (wParam & KCD_NEWDEFKEY) 
				PGPkmPerformAction (ppks->hKM, KM_SETASDEFAULT);
			PGPkmSelectKey (ppks->hKM, (PGPKeyRef)lParam, TRUE);
		}
		InvalidateRect (hWnd, NULL, FALSE);
		return TRUE;

	case KM_M_REQUESTSDKACCESS :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PGPkmSynchronizeThreadAccessToSDK (ppks->hKM);
		break;

	case KM_M_KEYPROPACTION :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		PGPkmProcessKeyPropMessage (ppks->hKM, wParam, lParam);
		break;

	case KM_M_RESIZE :
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
		GetClientRect (hWnd, &rc);
		if (ppks->hWndToolBar) {
			SetWindowPos (ppks->hWndToolBar, NULL, 
					0, 0, 
					(rc.right-rc.left), 
					ppks->iToolHeight, 
					SWP_NOMOVE|SWP_NOZORDER);
		}
		if (ppks->hWndTreeList) {
			SetWindowPos (ppks->hWndTreeList, NULL, 
					0, ppks->iToolHeight, 
					(rc.right-rc.left), 
					ppks->iKeysHeight, 
					SWP_NOZORDER);
		}
		if (ppks->hWndTreeListGroups) {
			SetWindowPos (ppks->hWndTreeListGroups, NULL, 
					0, 
					(rc.bottom-rc.top)-ppks->iGroupsHeight-STATUSBARHEIGHT,
					(rc.right-rc.left),
					ppks->iGroupsHeight,
					SWP_NOZORDER);
		}
		UpdateWindow (hWnd);
		break;

	case WM_COMMAND:
		ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);

		switch (LOWORD(wParam)) {
		case IDM_FILEEXIT:
			SendMessage (hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_SENDSHARES :
			PGPclSendShares (g_Context, g_TLSContext, hWnd, ppks->KeySetMain);
			break;

		case IDM_HELPABOUT :
			PKHelpAbout (hWnd);
			break;

		case IDM_HELPTOPICS :
			WinHelp (hWnd, g_szHelpFile, HELP_FINDER, 0);
			break;

		case IDM_PREFERENCES :
			PKPGPPreferences (ppks, hWnd, PGPCL_GENERALPREFS);
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
				PKCreateKey (hWnd, ppks->KeySetMain);
			}
			break;

		case IDM_SETASDEFAULT :
			if (PGPkmIsActionEnabled (ppks->hKM, KM_SETASPRIMARY))
				PGPkmPerformAction (ppks->hKM, KM_SETASPRIMARY); 
			else
				PGPkmPerformAction (ppks->hKM, KM_SETASDEFAULT); 
			break;

		case IDM_PROPERTIES :
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

		case IDM_RETRIEVECERTIFICATE :
			PGPkmPerformAction (ppks->hKM, KM_RETRIEVECERTIFICATE);
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
			PKSetToolbarButtonStates (ppks);
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

		case IDM_ADDPHOTOID :
			PGPkmPerformAction (ppks->hKM, KM_ADDPHOTOID); 
			break;

		case IDM_ADDREVOKER :
			PGPkmPerformAction (ppks->hKM, KM_ADDREVOKER); 
			break;

		case IDM_ADDCERTIFICATE :
			PGPkmPerformAction (ppks->hKM, KM_ADDCERTIFICATE); 
			break;

		case IDM_SPLITKEY :
			PGPkmPerformAction (ppks->hKM, KM_SPLITKEY); 
			break;

		case IDM_REVERIFY :
			PGPkmPerformAction (ppks->hKM, KM_REVERIFYSIGS);
			break;

		case IDM_VIEWVALIDITY :
			ulColumns ^= KM_VALIDITY;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWSIZE :
			ulColumns ^= KM_SIZE;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWDESC :
			ulColumns ^= KM_DESCRIPTION;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWKEYID :
			ulColumns ^= KM_KEYID;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWTRUST :
			ulColumns ^= KM_TRUST;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWCREATION :
			ulColumns ^= KM_CREATION;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWEXPIRATION :
			ulColumns ^= KM_EXPIRATION;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWADK :
			ulColumns ^= KM_ADK;
			PGPkmSelectColumns (ppks->hKM, ulColumns, TRUE);
			break;

		case IDM_VIEWTOOLBAR :
			if (ppks->iToolHeight == 0) 
				ppks->iToolHeight = DEFAULTTOOLHEIGHT;
			else 
				ppks->iToolHeight = 0;
			GetClientRect (hWnd, &rc);
			ppks->iKeysHeight = 
					(rc.bottom-rc.top)-ppks->iToolHeight-STATUSBARHEIGHT;
			if (ppks->iGroupsHeight)
				ppks->iKeysHeight -= (ppks->iGroupsHeight + GRABBARHEIGHT);
			PostMessage (hWnd, KM_M_RESIZE, 0, 0);
			break;

		case IDM_FINDKEY :
			PGPgmPerformAction (ppks->hGM, GM_LOCATEKEYS);
			ppks->bGroupsFocused = FALSE;
			SetFocus (ppks->hWndTreeList);
			break;

		case IDM_UPDATEINTRODUCERS :
			if (PKAutoUpdateIntroducers (hWnd, ppks->KeySetMain, FALSE))
				PGPkmReLoadKeySet (ppks->hKM, TRUE);
			break;

		case IDM_UPDATEREVOCATIONS :
			PKUpdateCARevocations (hWnd, ppks->hKM, ppks->KeySetMain);
			break;

		case IDM_UPDATEGROUPLISTS :
			PGPgmPerformAction (ppks->hGM, GM_UPDATEALLGROUPS); 
			break;

		case IDM_SENDGROUPLISTS :
			PGPgmPerformAction (ppks->hGM, GM_SENDALLGROUPS); 
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
				PostMessage (hWnd, KM_M_RESIZE, 0, 0);
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
				PostMessage (hWnd, KM_M_RESIZE, 0, 0);
			}
			break;

		case IDM_SEARCH :
			{
				RECT	rc;
				INT		iHeight, iWidth;
				CHAR	sz[64];
				ULONG	ulColumns;

				// if main window has not finished initializing, then ignore
				if (!ppks->KeySetMain)
					break;

				// if search window already exists, move to front
				if (hwndOpenSearch) {
					ShowWindow (hwndOpenSearch, SW_RESTORE);
					SetForegroundWindow (hwndOpenSearch);
					break;
				}

				// save column info of main window
				PGPkmGetSelectedColumns (ppks->hKM, &ulColumns);
				PGPkmSelectColumns (ppks->hKM, ulColumns, FALSE);

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

		case IDM_DOMAINKEYSERVER :
			SendMessage (hWnd, WM_COMMAND, IDM_DOMAINKEYSERVERX, 0);
			break;

		case IDM_DOMAINKEYSERVERX :
			ZeroMemory (&ppks->kmConfig.keyserver, sizeof(PGPKeyServerEntry));
			ppks->kmConfig.ulMask = PGPKM_KEYSERVER;
			PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
			PGPkmPerformAction (ppks->hKM, KM_SENDTOSERVER);
			break;

		case IDM_TOOLBARSENDTOSERVER :
			PKGetSendToServerButtonRect (ppks->hWndToolBar, &rc);
			MapWindowPoints (ppks->hWndMain, NULL, (LPPOINT)&rc, 2);
			PKToolbarKeyserverMenu (hWnd, &rc);
			break;

		default :
			if (LOWORD (wParam) > IDM_DOMAINKEYSERVERX) {
				PKGetServerFromID (LOWORD (wParam),
									&ppks->kmConfig.keyserver);
				ppks->kmConfig.ulMask = PGPKM_KEYSERVER;
				PGPkmConfigure (ppks->hKM, &(ppks->kmConfig));
				PGPkmPerformAction (ppks->hKM, KM_SENDTOSERVER);
			}
			break;
		}
		break;

	default :
		// check for mousewheel message
		if (uMsg == g_uMouseWheelMessage) {
			WORD iDelta;

			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			iDelta = (WORD)wParam;
			
			if (ppks->bGroupsFocused)
				SendMessage (ppks->hWndTreeListGroups, 
								WM_MOUSEWHEEL, MAKEWPARAM (0, iDelta), 0);
			else
				SendMessage (ppks->hWndTreeList, 
								WM_MOUSEWHEEL, MAKEWPARAM (0, iDelta), 0);
			return FALSE;
		}

		// check for prefs broadcast message
		else if (uMsg == g_uReloadPrefsMessage) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			PKReloadPGPPreferences (ppks);
		}

		// check for keyring broadcast message
		else if (uMsg == g_uReloadKeyringMessage) {
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
						(LOWORD (wParam) != LOWORD (hWnd))) 
					{
						if (ppks->KeySetDisp)
						{
							PGPFreeKeySet (ppks->KeySetDisp);
							ppks->KeySetDisp = NULL;
						}
						PGPkmLoadKeySet (ppks->hKM, ppks->KeySetDisp, 
													ppks->KeySetMain);
						SendMessage (ppks->hWndSearchControl, 
							SEARCH_SET_CURRENT_SEARCH, 0, 0);
					}
				}
			}
			return TRUE;
		}

		// check for keyserver prefs broadcast message
		else if (uMsg == g_uReloadKeyserverPrefsMessage) {
			ppks = (PGPKEYSSTRUCT*)GetWindowLong (hWnd, GWL_USERDATA);
			if (!ppks->bMainWindow) {
				NMHDR nmhdr;

				nmhdr.hwndFrom = hWnd;
				nmhdr.idFrom = 0;
				nmhdr.code = REFRESH_KEYSERVER_LIST;

				SendMessage (ppks->hWndSearchControl, 
					WM_NOTIFY, 
					0, 
					(LPARAM)&nmhdr);
			}
			return TRUE;
		}

		// check for purge passphrase cache broadcast message
		else if (uMsg == g_uPurgePassphraseCacheMessage) {
			PGPclPurgeCachedPassphrase (wParam);
			return TRUE;
		}

		else return (DefWindowProc (hWnd, uMsg, wParam, lParam));
	}
	return FALSE;
}

