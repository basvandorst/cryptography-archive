/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLksPref.c - handle keyserver preferences dialogs
	

	$Id: CLkspref.c,v 1.31 1998/08/11 14:43:09 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"
#include "..\Include\treelist.h"

// constant definitions
#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

#define BUTTONOFFSETY	10

#define DOMAINLISTWIDTH 80
#define CHECKLISTWIDTH	48

#define IDX_NONE				-1
#define IDX_UNCHECKED			0
#define IDX_UNCHECKEDPUSHED		1
#define IDX_CHECKED				2
#define IDX_CHECKEDPUSHED		3
#define IDX_WORLDDOMAIN			4
#define IDX_SERVER				5
#define NUM_BITMAPS				6

// typedefs
typedef struct {
	HWND					hWndTree;
	BOOL					bEdit;
	CHAR					szServer[256];
	PGPUInt16				iPort;
	PGPKeyServerType		iType;
	CHAR					szKeyIDAuth[kPGPMaxKeyIDStringSize];
	PGPPublicKeyAlgorithm	authAlg;
	CHAR					szUserIDAuth[kPGPMaxUserIDSize +1];
	CHAR					szDomain[256];
	BOOL					bListed;
	BOOL					bRoot;
	VOID*					pOrigStruct;
} KEYSERVERSTRUCT, *PKEYSERVERSTRUCT;

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// local globals
static HIMAGELIST hIml;
static HTLITEM hTSelected;
static HTLITEM hTDefault;
static INT iSelectedType;
static BOOL bSelectedRoot;
static UINT uNumberOfServers;

static DWORD aIds[] = {			// Help IDs
	IDC_PROTOCOL,		IDH_PGPCLPREF_SERVERPROTOCOL,
    IDC_SERVERNAME,		IDH_PGPCLPREF_SERVERNAME, 
	IDC_PORT,			IDH_PGPCLPREF_SERVERPORT,
	IDC_SERVERKEY,		IDH_PGPCLPREF_SERVERAUTHKEY,
	IDC_ANYDOMAIN,		IDH_PGPCLPREF_SERVERNODOMAIN,
    IDC_DOMAIN,			IDH_PGPCLPREF_SERVERDOMAIN, 
    IDC_SPECIFIEDDOMAIN,IDH_PGPCLPREF_SERVERDOMAIN, 
    IDC_LISTED,			IDH_PGPCLPREF_SERVERLISTED, 
    0,0 
}; 

//	_____________________________________________________
//
//  concatenate protocol and server names

static VOID
sURLfromInfo (
		PGPKeyServerType	iType, 
		LPSTR				szServer, 
		PGPUInt16			iPort, 
		LPSTR				szURL)
{
	switch (iType) {
	case kPGPKeyServerType_LDAP :	lstrcpy (szURL, "ldap://"); break;
	case kPGPKeyServerType_LDAPS :	lstrcpy (szURL, "ldaps://"); break;
	case kPGPKeyServerType_HTTPS :	lstrcpy (szURL, "https://"); break;
	default :						lstrcpy (szURL, "http://"); break;
	}

	lstrcat (szURL, szServer);

	if (iPort != 0) {
		CHAR szPort[8];
		wsprintf (szPort, ":%i", iPort);
		lstrcat (szURL, szPort);
	}
}

//	_____________________________________________________
//
//  put data for one keyserver into treelist

static HTLITEM
sSetOneKSItem (HWND hWndTree, PKEYSERVERSTRUCT pkss)
{
	TL_TREEITEM		tlI;
	TL_LISTITEM		tlL;
	TL_INSERTSTRUCT tlIns;
	HTLITEM			hTNew;
	CHAR			sz[256];

	sURLfromInfo (pkss->iType, pkss->szServer, pkss->iPort, sz);

	tlI.hItem = NULL;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE | TLIF_PARAM;
	tlI.stateMask = TLIS_BOLD;
	if (pkss->bRoot) 
		tlI.state = TLIS_BOLD;
	else
		tlI.state = 0;
	tlI.pszText = sz;
	tlI.cchTextMax = lstrlen (sz);
	tlI.iImage = IDX_SERVER;
	tlI.iSelectedImage = IDX_SERVER;
	tlI.lParam = (LPARAM)pkss;

	tlIns.item = tlI;
	tlIns.hInsertAfter = (HTLITEM)TLI_LAST;
	tlIns.hParent = NULL;
	hTNew = TreeList_InsertItem (hWndTree, &tlIns);

	if (lstrlen (pkss->szDomain))
		tlL.pszText = pkss->szDomain;
	else 
		tlL.pszText = NULL;
	tlL.hItem = hTNew;
	tlL.stateMask = TLIS_VISIBLE;
	tlL.iSubItem = 1;
	tlL.mask = TLIF_DATAVALUE | TLIF_TEXT | TLIF_STATE;
	tlL.state = TLIS_VISIBLE;
	tlL.lDataValue = IDX_WORLDDOMAIN;

	TreeList_SetListItem (hWndTree, &tlL, FALSE);

	tlL.pszText = NULL;
	tlL.hItem = hTNew;
	tlL.stateMask = TLIS_VISIBLE;
	tlL.iSubItem = 2;
	tlL.mask = TLIF_DATAVALUE | TLIF_STATE;
	tlL.state = TLIS_VISIBLE;

	if (pkss->bListed) tlL.lDataValue = IDX_CHECKED;
	else tlL.lDataValue = IDX_UNCHECKED;

	TreeList_SetListItem (hWndTree, &tlL, FALSE);

	return hTNew;
}

//	_____________________________________________________
//
//  populate keyserver treelist with data from preferences

static INT
sFillTreeList (
		PGPPrefRef		PrefRef, 
		PGPContextRef	context, 
		HWND			hWndTree,
		PGPKeySetRef	keysetMain) 
{
	PGPKeyServerEntry*	keyserverList;
	PGPKeyRef			key;
	PGPUInt32			u, u2;
	PGPError			err;
	PKEYSERVERSTRUCT	pkss;
	PGPKeySetRef		keyset;
	PGPKeyID			keyid;

	// load keyserverprefs
	err = PGPGetKeyServerPrefs (PrefRef, &keyserverList, &uNumberOfServers);
	if (IsPGPError (err)) 
		uNumberOfServers = 0;

	// get keyset for resolving auth key information
	if (keysetMain == NULL) {
		err = PGPOpenDefaultKeyRings (context, 0, &keyset);
		if (IsPGPError (err)) 
			keyset = NULL;
	}
	else 
		keyset = keysetMain;

	for (u=0; u<uNumberOfServers; u++) {

		pkss = clAlloc (sizeof(KEYSERVERSTRUCT));
		if (pkss) {
			pkss->hWndTree = hWndTree;
			pkss->bEdit = FALSE;
			lstrcpy (pkss->szServer, keyserverList[u].serverDNS);
			pkss->iPort = keyserverList[u].serverPort;
			pkss->iType = keyserverList[u].protocol;
			lstrcpy (pkss->szDomain, keyserverList[u].domain);
			pkss->bListed = (keyserverList[u].flags & kKeyServerListed);
			pkss->authAlg = keyserverList[u].authAlg;
			pkss->bRoot = (keyserverList[u].flags & kKeyServerIsRoot);

			// get auth key userid
			pkss->szUserIDAuth[0] = '\0';
			if ((pkss->authAlg != kPGPPublicKeyAlgorithm_Invalid) &&
					keyserverList[u].authKeyIDString[0]) {
				lstrcpy (pkss->szKeyIDAuth, 
								keyserverList[u].authKeyIDString);
				err = PGPGetKeyIDFromString (
						keyserverList[u].authKeyIDString, &keyid);
				if (IsntPGPError (err)) {
					err = PGPGetKeyByKeyID (keyset, &keyid, 
								keyserverList[u].authAlg, &key);
					if (IsntPGPError (err)) {
						PGPGetPrimaryUserIDNameBuffer (key, 
									sizeof(pkss->szUserIDAuth), 
									pkss->szUserIDAuth, &u2);
					}
					else {
						lstrcpy (pkss->szUserIDAuth, 
								keyserverList[u].authKeyIDString);
					}
				}
			}
			if (pkss->szUserIDAuth[0] == '\0') {
				LoadString (g_hInst, IDS_UNKNOWNKEY, pkss->szUserIDAuth, 
							sizeof(pkss->szUserIDAuth));
			}

			sSetOneKSItem (hWndTree, pkss); 
		}
	}

	if (keysetMain == NULL)
		PGPFreeKeySet (keyset);

	if (keyserverList) PGPDisposePrefData (PrefRef, keyserverList);

	return 0;
}


//	_____________________________________________________
//
//  add columns to treelist

static BOOL 
sAddColumns (HWND hWndTree, INT iWidth) {
	TL_COLUMN tlc;
	CHAR sz[64];

	TreeList_DeleteAllColumns (hWndTree);

	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX |
				TLCF_MOUSENOTIFY;
	tlc.pszText = sz;

	tlc.iSubItem = 0;
	tlc.fmt = TLCFMT_LEFT;
	tlc.iDataType = TLC_DATASTRING;
	tlc.cx = iWidth - DOMAINLISTWIDTH - CHECKLISTWIDTH - 20;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_SERVER, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 0, &tlc);

	tlc.fmt = TLCFMT_IMAGE;
	tlc.iDataType = TLC_DATALONG;
	tlc.cx = DOMAINLISTWIDTH;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_DOMAIN, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 1, &tlc);

	tlc.fmt = TLCFMT_IMAGE;
	tlc.iDataType = TLC_DATALONG;
	tlc.cx = CHECKLISTWIDTH;
	tlc.bMouseNotify = TRUE;
	LoadString (g_hInst, IDS_LISTED, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 2, &tlc);

	return TRUE;
}


//	_____________________________________________________
//
//  create new treelist control for keyserver prefs

HWND
CLCreateKeyserverTreeList (
		PGPPrefRef		PrefRef, 
		PGPContextRef	context, 
		HWND			hWndParent,
		PGPKeySetRef	keysetMain) 
{
	HBITMAP hBmp;      // handle to a bitmap
	HDC hDC;
	HWND hWndTree;
	RECT rc;
	INT iy, iNumBits;
	POINT pt;

	// Ensure that the common control DLL is loaded.
	InitCommonControls ();

	// Ensure that the custom control DLL is loaded.
	InitTreeListControl ();

	// initialize flags
	iSelectedType = IDX_NONE;
	bSelectedRoot = FALSE;

	// create tree view window
	GetWindowRect (GetDlgItem (hWndParent, IDC_NEWKEYSERVER), &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient (hWndParent, &pt);
	iy = pt.y - BUTTONOFFSETY;

	GetClientRect (hWndParent, &rc);

	hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_AUTOSCROLL | TLS_SINGLESELECT,
			rc.left, rc.top, rc.right-rc.left, iy,
			hWndParent, (HMENU)IDC_KSTREELIST, g_hInst, NULL);

	if (hWndTree == NULL) return NULL;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps based on 
	// current display capabilities.
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4));
		ImageList_AddMasked (hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24));
		ImageList_AddMasked (hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	TreeList_SetImageList (hWndTree, hIml);
	sAddColumns (hWndTree, rc.right);
	sFillTreeList (PrefRef, context, hWndTree, keysetMain);

	return hWndTree;
}

//	_____________________________________________________
//
//  destroy treelist control for keyserver prefs

INT
CLDestroyKeyserverTreeList (HWND hWndTree) 
{
	HTLITEM		hTServer;
	TL_TREEITEM tlI;

	// free all KEYSERVERSTRUCT structures
	hTServer = TreeList_GetFirstItem (hWndTree);
	while (hTServer) {
		tlI.hItem = hTServer;
		tlI.mask = TLIF_NEXTHANDLE | TLIF_PARAM;
		TreeList_GetItem (hWndTree, &tlI);

		clFree ((VOID*)(tlI.lParam));

		hTServer = tlI.hItem;
	}

	// close treelist
	SendMessage (hWndTree, WM_CLOSE, 0, 0);
	return 0;
}

//	_____________________________________________________
//
//  check if specified server is already in list

static BOOL
sIsThisServerAlreadyInList (PKEYSERVERSTRUCT pkssThis)
{
	TL_TREEITEM			tlI;
	HTLITEM				hTServer;
	INT					iServerCount;
	PKEYSERVERSTRUCT	pkss;

	// count number of servers
	iServerCount = 0;
	hTServer = TreeList_GetFirstItem (pkssThis->hWndTree);
	while (hTServer) {
		++iServerCount;
		tlI.hItem = hTServer;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (pkssThis->hWndTree, &tlI);
		hTServer = tlI.hItem;
	}

	hTServer = TreeList_GetFirstItem (pkssThis->hWndTree);
	while (hTServer) {
		// get server structure
		tlI.hItem = hTServer;
		tlI.mask = TLIF_PARAM | TLIF_NEXTHANDLE;
		TreeList_GetItem (pkssThis->hWndTree, &tlI);

		pkss = (PKEYSERVERSTRUCT)(tlI.lParam);

		if (pkss != (PKEYSERVERSTRUCT)pkssThis->pOrigStruct) {
			if (!lstrcmpi (pkss->szServer, pkssThis->szServer)) {
				if (pkss->iType == pkssThis->iType) {
					if (pkss->iPort == pkssThis->iPort) {
						return TRUE;
					}
				}
			}
		}

		// step to next server
		hTServer = tlI.hItem;
	}

	return FALSE;
}

//	_____________________________________________________
//
//  find out if there are any domain-less servers among those
//  which are not selected.

static BOOL
sIsThisTheOnlyWorldServer (HWND hWndTree)
{
	HTLITEM				hTServer;
	TL_TREEITEM			tlI;
	PKEYSERVERSTRUCT	pkss;

	// is the selected server a "world" server?
	tlI.hItem = hTSelected;
	tlI.mask = TLIF_PARAM;
	TreeList_GetItem (hWndTree, &tlI);
	pkss = (PKEYSERVERSTRUCT)tlI.lParam;

	// if server has a domain, then it's not a "world" server.
	if (pkss->szDomain[0]) 
		return FALSE;

	// interate through tree
	hTServer = TreeList_GetFirstItem (hWndTree);
	while (hTServer) {

		if (hTServer != hTSelected) {
			// get keyserver structure
			tlI.hItem = hTServer;
			tlI.mask = TLIF_PARAM;
			TreeList_GetItem (hWndTree, &tlI);

			pkss = (PKEYSERVERSTRUCT)tlI.lParam;

			// if this server doesn't have a domain, then the
			// selected server isn't the last "world" server
			if (pkss->szDomain[0] == 0)
				return FALSE;
		}

		// get next server
		tlI.hItem = hTServer;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);

		hTServer = tlI.hItem;
	}
	
	// didn't find any other "world" servers
	return TRUE;
}

//	___________________________________________
//
//  New/Edit Keyserver Dialog Message procedure

static BOOL CALLBACK 
sEditServerDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PKEYSERVERSTRUCT	pkss;
	CHAR				sz[256];
	LPSTR				p;
	INT					i, iSelected;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pkss = (PKEYSERVERSTRUCT)lParam;
		if (pkss->bEdit) {
			LoadString (g_hInst, IDS_EDITSERVERTITLE, sz, sizeof(sz));
			SetWindowText (hDlg, sz);
		}

		// load protocol combo box
		LoadString (g_hInst, IDS_LDAP, sz, sizeof(sz));
		i = SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_ADDSTRING, 
								0, (LPARAM)sz);
		SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 
								i, (LPARAM)kPGPKeyServerType_LDAP);
		if (pkss->iType == kPGPKeyServerType_LDAP) iSelected = i;
		
		LoadString (g_hInst, IDS_LDAPS, sz, sizeof(sz));
		i = SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_ADDSTRING, 
								0, (LPARAM)sz);
		SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 
								i, (LPARAM)kPGPKeyServerType_LDAPS);
		if (pkss->iType == kPGPKeyServerType_LDAPS) iSelected = i;
		
		LoadString (g_hInst, IDS_HTTP, sz, sizeof(sz));
		i = SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_ADDSTRING, 
								0, (LPARAM)sz);
		SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 
								i, (LPARAM)kPGPKeyServerType_HTTP);
		if (pkss->iType == kPGPKeyServerType_HTTP) iSelected = i;

		// initialize protocol combo box
		SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_SETCURSEL, iSelected, 0);

		// initialize server name
		SetDlgItemText (hDlg, IDC_SERVERNAME, pkss->szServer);

		// initialize port number
		if (pkss->iPort != 0)
			SetDlgItemInt (hDlg, IDC_PORT, pkss->iPort, FALSE);


		// initialize authentication key name
		SetDlgItemText (hDlg, IDC_SERVERKEY, pkss->szUserIDAuth);

		// initialize domain buttons
		if (pkss->szDomain[0]) {
			CheckRadioButton (hDlg, IDC_ANYDOMAIN, IDC_SPECIFIEDDOMAIN,
								IDC_SPECIFIEDDOMAIN);
			SetDlgItemText (hDlg, IDC_DOMAIN, pkss->szDomain);
		}
		else {
			CheckRadioButton (hDlg, IDC_ANYDOMAIN, IDC_SPECIFIEDDOMAIN,
								IDC_ANYDOMAIN);
			SetDlgItemText (hDlg, IDC_DOMAIN, "");
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), FALSE);
		}

		// initialize checkboxes
		if (pkss->bListed) 
			CheckDlgButton (hDlg, IDC_LISTED, BST_CHECKED);
		else
			CheckDlgButton (hDlg, IDC_LISTED, BST_UNCHECKED);
			
		SetFocus (GetDlgItem (hDlg, IDC_SERVERNAME));
		return TRUE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aIds); 
		break; 

	case WM_COMMAND :
		switch(LOWORD (wParam)) {

		case IDC_ANYDOMAIN :
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), FALSE);
			break;

		case IDC_SPECIFIEDDOMAIN :
			pkss = (PKEYSERVERSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if ((pkss->bEdit) &&
				(sIsThisTheOnlyWorldServer (pkss->hWndTree))) 
			{
				PGPclMessageBox (hDlg, IDS_PROPCAPTION, 
					IDS_MUSTHAVEDEFAULTSERVER, MB_OK|MB_ICONEXCLAMATION);
				CheckRadioButton (hDlg, IDC_ANYDOMAIN, IDC_SPECIFIEDDOMAIN,
									IDC_ANYDOMAIN);
			}
			else 
				EnableWindow (GetDlgItem (hDlg, IDC_DOMAIN), TRUE);
			break;

		case IDOK :
			pkss = (PKEYSERVERSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
				
			// get protocol
			iSelected = 
				SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_GETCURSEL, 0, 0);
			pkss->iType = 
				SendDlgItemMessage (hDlg, IDC_PROTOCOL, CB_GETITEMDATA, 
										iSelected, 0);

			// get server name
			if (!GetDlgItemText (hDlg, IDC_SERVERNAME, sz, sizeof (sz))) {
				PGPclMessageBox (hDlg, IDS_PROPCAPTION, IDS_NOSERVERNAME,
					MB_OK|MB_ICONEXCLAMATION);
				SetFocus (GetDlgItem (hDlg, IDC_SERVERNAME));
				return TRUE;
			}
			else {
				// make sure user didn't type in prefix
				p = strstr (sz, "//");
				if (!p) p = strstr (sz, "\\\\");
				if (p) p += 2;
				else p = &sz[0];
				lstrcpy (pkss->szServer, p);
			}

			// get port number
			pkss->iPort = GetDlgItemInt (hDlg, IDC_PORT, NULL, FALSE);

			// get domain name
			if (IsDlgButtonChecked (hDlg, IDC_SPECIFIEDDOMAIN)) {
				GetDlgItemText (hDlg, IDC_DOMAIN, pkss->szDomain, 
									sizeof (pkss->szDomain));
			}
			else 
				pkss->szDomain[0] = '\0';

			// get checkbox states			
			if (IsDlgButtonChecked (hDlg, IDC_LISTED) == BST_CHECKED) 
				pkss->bListed = TRUE;
			else 
				pkss->bListed = FALSE;

			if (sIsThisServerAlreadyInList (pkss)) {
				PGPclMessageBox (hDlg, IDS_PROPCAPTION, IDS_DUPLICATESERVER,
					MB_OK|MB_ICONEXCLAMATION);
				SetFocus (GetDlgItem (hDlg, IDC_SERVERNAME));
				return TRUE;
			}

			EndDialog (hDlg, 1);
			break;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

//	_____________________________________________________
//
//  set all text to non-bold

static VOID
sRemoveAllRoots (HWND hWndTree) 
{
	HTLITEM				hTServer;
	TL_TREEITEM			tlI;
	PKEYSERVERSTRUCT	pkss;

	// interate through tree
	hTServer = TreeList_GetFirstItem (hWndTree);
	while (hTServer) {
		// get current state
		tlI.hItem = hTServer;
		tlI.mask = TLIF_STATE | TLIF_PARAM;
		TreeList_GetItem (hWndTree, &tlI);

		// make non-root
		pkss = (PKEYSERVERSTRUCT)tlI.lParam;
		pkss->bRoot = FALSE;

		// remove "bold" property
		tlI.state &= ~TLIS_BOLD;
		tlI.stateMask = TLIS_BOLD;
		TreeList_SetItem (hWndTree, &tlI);

		// get next item
		tlI.hItem = hTServer;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);

		hTServer = tlI.hItem;
	}
}

//	_____________________________________________________
//
//  create new keyserver and add to treelist

INT
CLNewKeyserver (HWND hWndParent, HWND hWndTree)
{
	PKEYSERVERSTRUCT pkss;

	pkss = (PKEYSERVERSTRUCT)clAlloc (sizeof(KEYSERVERSTRUCT));
	if (!pkss) return 1;

	pkss->hWndTree = hWndTree;
	pkss->bEdit = FALSE;

	pkss->szServer[0] = '\0';
	pkss->iPort = 0;
	pkss->iType = kPGPKeyServerType_LDAP;
	LoadString (g_hInst, IDS_UNKNOWNKEY, pkss->szUserIDAuth, 
					sizeof(pkss->szUserIDAuth));
	pkss->szDomain[0] = '\0';
	pkss->bListed = TRUE;
	pkss->bRoot = FALSE;
	pkss->authAlg = kPGPPublicKeyAlgorithm_Invalid;
	pkss->pOrigStruct = NULL;

	if (!DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_NEWSERVER), hWndParent, 
				sEditServerDlgProc, (LPARAM)pkss)) {
		clFree (pkss);
		SetFocus (hWndTree);
		return 0;
	}

	sSetOneKSItem (hWndTree, pkss); 
	InvalidateRect (hWndTree, NULL, TRUE);

	++uNumberOfServers;

	EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
						FALSE);
	EnableWindow (GetDlgItem (hWndParent, IDC_EDITKEYSERVER),
						FALSE);
	EnableWindow (GetDlgItem (hWndParent, IDC_SETASROOT),
						FALSE);

	return 0;
}

//	_____________________________________________________
//
//  edit existing keyserver params

VOID
CLEditKeyserver (HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM			tlI;
	TL_LISTITEM			tlL;
	KEYSERVERSTRUCT		kss;
	PKEYSERVERSTRUCT	pkss;
	CHAR				sz[256];

	// get server struct
	tlI.hItem = hTSelected;
	tlI.mask = TLIF_PARAM;
	TreeList_GetItem (hWndTree, &tlI);

	pkss = (PKEYSERVERSTRUCT)tlI.lParam;
	memcpy (&kss, pkss, sizeof(KEYSERVERSTRUCT));
	kss.bEdit = TRUE;
	kss.pOrigStruct = (VOID*)pkss;

	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_NEWSERVER), hWndParent, 
		sEditServerDlgProc, (LPARAM)&kss)) {

		memcpy (pkss, &kss, sizeof(KEYSERVERSTRUCT));

		sURLfromInfo (pkss->iType, pkss->szServer, pkss->iPort, sz);
		pkss->authAlg = kPGPPublicKeyAlgorithm_Invalid;

		// insert new server text into treelist
		tlI.hItem = hTSelected;
		tlI.mask = TLIF_TEXT | TLIF_STATE;
		tlI.stateMask = TLIS_BOLD;
		if (pkss->bRoot) 
			tlI.state = TLIS_BOLD;
		else
			tlI.state = 0;
		tlI.cchTextMax = lstrlen (sz);
		tlI.pszText = sz;
		TreeList_SetItem (hWndTree, &tlI);

		// set new domain text
		tlL.hItem = hTSelected;
		tlL.mask = TLIF_TEXT;
		tlL.iSubItem = 1;
		tlL.cchTextMax = lstrlen (pkss->szDomain);
		if (tlL.cchTextMax)
			tlL.pszText = pkss->szDomain;
		else
			tlL.pszText = NULL;
		TreeList_SetListItem (hWndTree, &tlL, FALSE);

		// set listed state
		tlL.hItem = hTSelected;
		tlL.mask = TLIF_DATAVALUE;
		tlL.iSubItem = 2;
		if (pkss->bListed) tlL.lDataValue = IDX_CHECKED;
		else tlL.lDataValue = IDX_UNCHECKED;
		TreeList_SetListItem (hWndTree, &tlL, FALSE);
	}

	SetFocus (hWndTree);
}

//	_____________________________________________________
//
//  remove selected keyserver from treelist

VOID
CLRemoveKeyserver (HWND hWndParent, HWND hWndTree)
{
	CHAR sz0[256];
	CHAR sz1[256];
	CHAR sz2[256];
	TL_TREEITEM tlI;
	HTLITEM hTParent;

	if (sIsThisTheOnlyWorldServer (hWndTree)) {
		PGPclMessageBox (hWndParent, IDS_PROPCAPTION, 
				IDS_MUSTHAVEDEFAULTSERVER, MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	tlI.hItem = hTSelected;
	tlI.mask = TLIF_TEXT | TLIF_PARAM;
	tlI.pszText = sz1;
	tlI.cchTextMax = sizeof(sz1);
	TreeList_GetItem (hWndTree, &tlI);
	hTParent = tlI.hItem;

	LoadString (g_hInst, IDS_DELKSPROMPT, sz0, sizeof(sz0));
	wsprintf (sz2, sz0, sz1);

	LoadString (g_hInst, IDS_PROPCAPTION, sz1, sizeof(sz1));
	if (MessageBox (hWndParent, sz2, sz1, 
			MB_YESNO|MB_ICONEXCLAMATION) == IDYES) {

		// free KEYSERVERSTRUCT
		clFree ((VOID*)(tlI.lParam));

		// delete selected item
		tlI.hItem = hTSelected;
		TreeList_DeleteItem (hWndTree, &tlI);

		hTSelected = NULL;
		iSelectedType = IDX_NONE;
		bSelectedRoot = FALSE;

		// finish up
		InvalidateRect (hWndTree, NULL, TRUE);
		EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
							FALSE);
		EnableWindow (GetDlgItem (hWndParent, IDC_EDITKEYSERVER),
							FALSE);
		EnableWindow (GetDlgItem (hWndParent, IDC_SETASROOT),
							FALSE);
		--uNumberOfServers;
	}
	else 
		SetFocus (hWndTree);
}


//	_____________________________________________________
//
//  set selected server as root server

VOID
CLSetKeyserverAsRoot (HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM			tlI;
	PKEYSERVERSTRUCT	pkss;

	sRemoveAllRoots (hWndTree);

	tlI.hItem = hTSelected;
	tlI.mask = TLIF_PARAM;
	TreeList_GetItem (hWndTree, &tlI);
	pkss = (PKEYSERVERSTRUCT)tlI.lParam;

	pkss->bRoot = TRUE;

	tlI.hItem = hTSelected;
	tlI.stateMask = TLIS_BOLD;
	tlI.state = TLIS_BOLD;
	tlI.mask = TLIF_STATE;
	TreeList_SetItem (hWndTree, &tlI);

	EnableWindow (GetDlgItem (hWndParent, IDC_SETASROOT), FALSE);
	SetFocus (hWndTree);
}


//	_____________________________________________________
//
//  process treelist notification

static VOID
sServerContextMenu (HWND hwnd, INT iX, INT iY)
{
	HMENU	hMC;
	HMENU	hMenuTrackPopup;
#if PGP_DEBUG
	BOOL	bControl;

	/// <ctrl> key allows removing last keyserver (for testing purposes)
	if (GetKeyState (VK_CONTROL) & 0x80000000)
		bControl = TRUE;
	else
		bControl = FALSE;
#endif PGP_DEBUG

	hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUEDITSERVER));
	if (iSelectedType == IDX_SERVER) {
#if PGP_DEBUG
		if (uNumberOfServers > 1 || bControl)
#else
		if (uNumberOfServers > 1)
#endif
			EnableMenuItem (hMC, IDM_REMOVESERVER, MF_BYCOMMAND|MF_ENABLED);
		else
			EnableMenuItem (hMC, IDM_REMOVESERVER, MF_BYCOMMAND|MF_GRAYED);

		EnableMenuItem (hMC, IDM_EDITSERVER, MF_BYCOMMAND|MF_ENABLED);
	}
	else {
		EnableMenuItem (hMC, IDM_REMOVESERVER, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem (hMC, IDM_EDITSERVER, MF_BYCOMMAND|MF_GRAYED);
	}

	if (bSelectedRoot)
		EnableMenuItem (hMC, IDM_SETASROOT, MF_BYCOMMAND|MF_GRAYED);
	else
		EnableMenuItem (hMC, IDM_SETASROOT, MF_BYCOMMAND|MF_ENABLED);

	hMenuTrackPopup = GetSubMenu (hMC, 0);

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					iX, iY, 0, hwnd, NULL);

	DestroyMenu (hMC);
}

//	_____________________________________________________
//
//  process treelist notification

INT
CLProcessKeyserverTreeList (HWND hWndParent, HWND hWndTree, 
							WPARAM wParam, LPARAM lParam)
{
	TL_TREEITEM			tlI;
	TL_LISTITEM			tlL;
	BOOL				bEnable;
	PKEYSERVERSTRUCT	pkss;

	switch (((LPNM_TREELIST)lParam)->hdr.code) {

	case TLN_SELCHANGED :
		hTSelected = ((LPNM_TREELIST)lParam)->itemNew.hItem;

		if (!hTSelected) {
			iSelectedType = IDX_NONE;
			bSelectedRoot = FALSE;
			EnableWindow (GetDlgItem (hWndParent, IDC_EDITKEYSERVER),
								FALSE);
			EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
								FALSE);
			EnableWindow (GetDlgItem (hWndParent, IDC_SETASROOT), 
								FALSE);
			break;
		}

		tlI.hItem = hTSelected;
		tlI.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_GetItem (hWndTree, &tlI);
		iSelectedType = tlI.iImage;
		bSelectedRoot = tlI.state & TLIS_BOLD;
	
		switch (iSelectedType) {
		case IDX_SERVER :
			EnableWindow (GetDlgItem (hWndParent, IDC_EDITKEYSERVER),
								TRUE);

			if (uNumberOfServers > 1)
				bEnable = TRUE;
			else 
				bEnable = FALSE;
			EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
								bEnable);

			if (bSelectedRoot)
				bEnable = FALSE;
			else
				bEnable = TRUE;
			EnableWindow (GetDlgItem (hWndParent, IDC_SETASROOT), bEnable);

			break;
		}
		break;

	case TLN_LISTITEMCLICKED : 
		tlI.hItem = hTSelected;
		tlI.mask = TLIF_PARAM;
		TreeList_GetItem (hWndTree, &tlI);

		pkss = (PKEYSERVERSTRUCT)(tlI.lParam);
		pkss->bListed = !pkss->bListed;

		tlL.hItem = hTSelected;
		tlL.iSubItem = 2;
		tlL.mask = TLIF_DATAVALUE;

		if (pkss->bListed) 
			tlL.lDataValue = IDX_CHECKED;
		else 
			tlL.lDataValue = IDX_UNCHECKED;

		TreeList_SetListItem (hWndTree, &tlL, FALSE);
		break;

	case TLN_CONTEXTMENU :
		sServerContextMenu (hWndParent,
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
		break;

	}
	return 0;
}

//	_____________________________________________________
//
//  traverse treelist and save contents to preferences

INT
CLSaveKeyserverPrefs (PGPPrefRef PrefRef, HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM			tlI;
	HTLITEM				hTServer;
	PGPKeyServerEntry*	keyserverList;
	INT					iServer, iServerCount;
	PKEYSERVERSTRUCT	pkss;

	// count number of servers
	iServerCount = 0;
	hTServer = TreeList_GetFirstItem (hWndTree);
	while (hTServer) {
		++iServerCount;
		tlI.hItem = hTServer;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		hTServer = tlI.hItem;
	}

	keyserverList = clAlloc (iServerCount * sizeof(PGPKeyServerEntry));
	if (!keyserverList) return kPGPError_OutOfMemory;

	iServer = 0;
	hTServer = TreeList_GetFirstItem (hWndTree);
	while (hTServer) {
		// get server structure
		tlI.hItem = hTServer;
		tlI.mask = TLIF_PARAM | TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);

		pkss = (PKEYSERVERSTRUCT)(tlI.lParam);

		keyserverList[iServer].protocol = pkss->iType;
		lstrcpy (keyserverList[iServer].domain, pkss->szDomain);
		lstrcpy (keyserverList[iServer].serverDNS, pkss->szServer);
		keyserverList[iServer].serverPort = pkss->iPort;

		// authentication key
		if (pkss->authAlg != kPGPPublicKeyAlgorithm_Invalid) {
			keyserverList[iServer].authAlg = pkss->authAlg;
			lstrcpy (keyserverList[iServer].authKeyIDString, 
						pkss->szKeyIDAuth);
		}
		else {
			keyserverList[iServer].authAlg = kPGPPublicKeyAlgorithm_Invalid;
			keyserverList[iServer].authKeyIDString[0] = '\0';
		}

		// listed flag
		keyserverList[iServer].flags = 0;
		if (pkss->bListed) 
			keyserverList[iServer].flags |= kKeyServerListed;
		if (pkss->bRoot) 
			keyserverList[iServer].flags |= kKeyServerIsRoot;

		// step to next server
		++iServer;
		hTServer = tlI.hItem;
	}

	// done, set preferences
	PGPSetKeyServerPrefs (PrefRef, keyserverList, iServerCount);

	clFree (keyserverList);

	return 0;
}


