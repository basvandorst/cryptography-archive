/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	CDksPref.c - handle keyserver preferences dialogs
	

	$Id: CDkspref.c,v 1.19 1997/10/21 13:36:26 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"
#include "pgpKeyServerPrefs.h"
#include "..\Include\treelist.h"

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

#define BUTTONOFFSETY	10

#define CHECKWIDTH	48

#define IDX_NONE				-1
#define IDX_UNCHECKED			0
#define IDX_UNCHECKEDPUSHED		1
#define IDX_CHECKED				2
#define IDX_CHECKEDPUSHED		3
#define IDX_DOMAIN				4
#define IDX_SERVER				5
#define NUM_BITMAPS				6


typedef struct {
	HWND	hWndTree;
	BOOL	bEdit;
	CHAR	szDomain[256];
	CHAR	szServer[256];
	INT		idx;
} NEWSERVERSTRUCT;

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];
extern HWND g_hWndPreferences;

// local globals
static HIMAGELIST hIml;
static HTLITEM hTSelected;
static HTLITEM hTDefault;
static INT iSelectedType;
static UINT uNumberOfServers;

static DWORD aIds[] = {			// Help IDs
    IDC_DOMAINNAME,		IDH_PGPCDPREF_NEWDOMAINNAME, 
    IDC_SERVERNAME,		IDH_PGPCDPREF_NEWSERVERNAME, 
	IDC_LDAP,			IDH_PGPCDPREF_NEWLDAPSERVER,
	IDC_HTTP,			IDH_PGPCDPREF_NEWHTTPSERVER,
    0,0 
}; 

//	_____________________________________________________
//
//  populate keyserver treelist with data from preferences

HTLITEM
SetOneItem (HWND hWndTree, HTLITEM hTPrev, HTLITEM hTParent, LPSTR szText, 
			INT iImage, BOOL bDefault, BOOL bShowCheckbox, 
			BOOL bCheckCheckbox) 
{
	TL_TREEITEM tlI;
	TL_LISTITEM tlL;
	TL_INSERTSTRUCT tlIns;
	HTLITEM hTNew;

	tlI.hItem = NULL;
	tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_STATE | TLIF_PARAM;
	tlI.stateMask = TLIS_BOLD | TLIS_ITALICS;
	tlI.state = 0;
	tlI.pszText = szText;
	tlI.cchTextMax = lstrlen (szText);
	tlI.iImage = iImage;
	tlI.iSelectedImage = iImage;
	tlI.lParam = 0;

	if (bDefault) tlI.state |= TLIS_BOLD;

	tlIns.item = tlI;
	tlIns.hInsertAfter = hTPrev;
	tlIns.hParent = hTParent;
	hTNew = TreeList_InsertItem (hWndTree, &tlIns);

	tlL.pszText = NULL;
	tlL.hItem = hTNew;
	tlL.stateMask = TLIS_VISIBLE;
	tlL.iSubItem = 1;
	tlL.mask = TLIF_DATAVALUE | TLIF_STATE;

	if (bShowCheckbox) tlL.state = TLIS_VISIBLE;
	else tlL.state = 0;

	if (bCheckCheckbox) tlL.lDataValue = IDX_CHECKED;
	else tlL.lDataValue = IDX_UNCHECKED;

	TreeList_SetListItem (hWndTree, &tlL, FALSE);

	return hTNew;
}

//	_____________________________________________________
//
//  populate keyserver treelist with data from preferences

INT
FillTreeList (PGPPrefRef PrefRef, HWND hWndTree) 
{
	HTLITEM				hTDomain, hTServer;
	INT					idx;
	CHAR				szDomain[256];
	BOOL				bListed, bDefault;
	PGPKeyServerEntry*	keyserverList;
	PGPUInt32			u;
	PGPError			err;

	// load keyserverprefs
	err = PGPGetKeyServerPrefs (PrefRef, &keyserverList, &uNumberOfServers);
	if (IsPGPError (err)) 
		uNumberOfServers = 0;

	hTDefault = NULL;
	szDomain[0] = '\0';

	for (u=0; u<uNumberOfServers; u++) {
		if (lstrcmpi (keyserverList[u].domain, szDomain)) {
			lstrcpy (szDomain, keyserverList[u].domain);
			hTDomain = (HTLITEM)TLI_SORT;
			hTDomain = SetOneItem (hWndTree, hTDomain, NULL, szDomain, 
								IDX_DOMAIN, FALSE, FALSE, FALSE);
			hTServer = (HTLITEM)TLI_FIRST;
		}

		idx = IDX_SERVER;

		bDefault = IsKeyServerDefault (keyserverList[u].flags);
		bListed = IsKeyServerListed (keyserverList[u].flags);

		hTServer = SetOneItem (hWndTree, hTServer, hTDomain, 
								keyserverList[u].serverURL, 
								idx, bDefault, TRUE, bListed);

		if (bDefault) hTDefault = hTServer;
	}

	if (keyserverList) PGPDisposePrefData (PrefRef, keyserverList);

	return 0;
}

//	_____________________________________________________
//
//  populate keyserver treelist with data from preferences

INT
FillDomainComboBox (HWND hWndTree, HWND hwndCombo) 
{
	INT					iDefault		= 0;
	TL_TREEITEM			tlI;
	HTLITEM				hTDomain, hTServer;
	CHAR				szDomain[256];
	INT					idx;

	// loop through all servers
	hTDomain = TreeList_GetFirstItem (hWndTree);
	while (hTDomain) {

		// get domain name
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_TEXT | TLIF_CHILDHANDLE;
		tlI.cchTextMax = sizeof (szDomain);
		tlI.pszText = szDomain;
		TreeList_GetItem (hWndTree, &tlI);

		// put name in combo box
		idx = SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)szDomain);

		// see if the default server is in this domain
		hTServer = tlI.hItem;
		while (hTServer) {
			tlI.hItem = hTServer;
			tlI.mask = TLIF_STATE | TLIF_NEXTHANDLE;
			TreeList_GetItem (hWndTree, &tlI);

			hTServer = tlI.hItem;
			if (tlI.state & TLIS_BOLD) iDefault = idx;
		}

		// step to next domain
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		hTDomain = tlI.hItem;
	}

	SendMessage (hwndCombo, CB_SETCURSEL, iDefault, 0);

	return 0;
}

//	_____________________________________________________
//
//  add columns to treelist

BOOL AddColumns (HWND hWndTree, INT iWidth) {
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
	tlc.cx = iWidth - CHECKWIDTH - 20;
	tlc.bMouseNotify = FALSE;
	LoadString (g_hInst, IDS_DOMAIN, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 0, &tlc);

	tlc.fmt = TLCFMT_IMAGE;
	tlc.iDataType = TLC_DATALONG;
	tlc.cx = CHECKWIDTH;
	tlc.bMouseNotify = TRUE;
	LoadString (g_hInst, IDS_LISTED, sz, sizeof(sz));
	TreeList_InsertColumn (hWndTree, 1, &tlc);

	return TRUE;
}


//	_____________________________________________________
//
//  create new treelist control for keyserver prefs

HWND
CDCreateKeyserverTreeList (PGPPrefRef PrefRef, HWND hWndParent) 
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


	// create tree view window
	GetWindowRect (GetDlgItem (hWndParent, IDC_NEWKEYSERVER), &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient (hWndParent, &pt);
	iy = pt.y - BUTTONOFFSETY;

	GetClientRect (hWndParent, &rc);

	hWndTree = CreateWindowEx (WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_GROUP |
			TLS_HASBUTTONS | TLS_HASLINES | TLS_AUTOSCROLL | 
			TLS_SINGLESELECT,
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
	AddColumns (hWndTree, rc.right);
	FillTreeList (PrefRef, hWndTree);
	TreeList_Expand (hWndTree, NULL, TLE_EXPANDALL);

	return hWndTree;
}

//	_____________________________________________________
//
//  create new treelist control for keyserver prefs

INT
CDDestroyKeyserverTreeList (HWND hWndTree) 
{
	SendMessage (hWndTree, WM_CLOSE, 0, 0);
	return 0;
}

//	___________________________________________
//
//  New Keyserver Dialog Message procedure

BOOL CALLBACK 
NewServerDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	NEWSERVERSTRUCT* pnss;
	CHAR sz[256];
	LPSTR p;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pnss = (NEWSERVERSTRUCT*)lParam;
		if (pnss->bEdit) {
			LoadString (g_hInst, IDS_EDITSERVERTITLE, sz, sizeof(sz));
			SetWindowText (hDlg, sz);
			SendDlgItemMessage (hDlg, IDC_DOMAINNAME, CB_ADDSTRING, 
									0, (LPARAM)(pnss->szDomain));
			SendDlgItemMessage (hDlg, IDC_DOMAINNAME, CB_SETCURSEL, 0, 0);
			EnableWindow (GetDlgItem (hDlg, IDC_DOMAINNAME), FALSE);
			if (pnss->szServer[0] == 'l')
				CheckDlgButton (hDlg, IDC_LDAP, BST_CHECKED);
			else 
				CheckDlgButton (hDlg, IDC_HTTP, BST_CHECKED);
			p = strchr (pnss->szServer, ':');
			if (p) {
				p += 3;
				SetDlgItemText (hDlg, IDC_SERVERNAME, p);
			}

			SetFocus (GetDlgItem (hDlg, IDC_SERVERNAME));
			return FALSE;
		}
		else {
			FillDomainComboBox (pnss->hWndTree, 
							GetDlgItem (hDlg, IDC_DOMAINNAME));
			CheckDlgButton (hDlg, IDC_LDAP, BST_CHECKED);
			return TRUE;
		}

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

		case IDOK :
			pnss = (NEWSERVERSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);

			// get domain name
			if (!GetDlgItemText (hDlg, IDC_DOMAINNAME, pnss->szDomain, 
					sizeof (pnss->szDomain))) {
				PGPcomdlgMessageBox (hDlg, IDS_PROPCAPTION, IDS_NODOMAINNAME,
					MB_OK|MB_ICONEXCLAMATION);
				SetFocus (GetDlgItem (hDlg, IDC_DOMAINNAME));
				return TRUE;
			}

			// construct prefix based on radio button selection
			if (IsDlgButtonChecked (hDlg, IDC_LDAP) == BST_CHECKED)
				lstrcpy (pnss->szServer, "ldap://");
			else 
				lstrcpy (pnss->szServer, "http://");

			// get server name
			if (!GetDlgItemText (hDlg, IDC_SERVERNAME, sz, sizeof (sz))) {
				PGPcomdlgMessageBox (hDlg, IDS_PROPCAPTION, IDS_NOSERVERNAME,
					MB_OK|MB_ICONEXCLAMATION);
				SetFocus (GetDlgItem (hDlg, IDC_SERVERNAME));
				return TRUE;
			}

			// make sure user didn't type in prefix
			p = strstr (sz, "//");
			if (!p) p = strstr (sz, "\\\\");
			if (p) p += 2;
			else p = &sz[0];

			// concatenate server name onto prefix
			lstrcat (pnss->szServer, p);

			// set bitmap index
			pnss->idx = IDX_SERVER;

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
//  create new keyserver and add to treelist

INT
CDNewKeyserver (HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM tlI;
	HTLITEM hTDomain, hTServer;
	NEWSERVERSTRUCT nss;
	CHAR szDomain[256];
	CHAR szServer[256];

	nss.hWndTree = hWndTree;
	nss.bEdit = FALSE;

	if (!DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_NEWSERVER), hWndParent, 
					NewServerDlgProc, (LPARAM)&nss)) return 0;

	hTDomain = TreeList_GetFirstItem (hWndTree);
	while (hTDomain) {
		// get domain name
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_TEXT | TLIF_CHILDHANDLE;
		tlI.cchTextMax = sizeof (szDomain);
		tlI.pszText = szDomain;
		TreeList_GetItem (hWndTree, &tlI);

		if (lstrcmpi (szDomain, nss.szDomain) == 0) {
			hTServer = tlI.hItem;
			while (hTServer) {
				// get server name
				tlI.hItem = hTServer;
				tlI.mask = TLIF_TEXT | TLIF_IMAGE | TLIF_NEXTHANDLE;
				tlI.cchTextMax = sizeof (szServer);
				tlI.pszText = szServer;
				TreeList_GetItem (hWndTree, &tlI);
				
				if (lstrcmpi (szServer, nss.szServer) == 0) {
					PGPcomdlgMessageBox (hWndParent, IDS_PROPCAPTION,
						IDS_SERVEREXISTS, MB_OK|MB_ICONEXCLAMATION);
					return 0;
				}

				// step to next server
				hTServer = tlI.hItem;
			}
			break;
		}

		// step to next domain
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		hTDomain = tlI.hItem;
	}

	// insert new domain, if necessary
	if (!hTDomain) {
		hTDomain = (HTLITEM)TLI_SORT;
		hTDomain = SetOneItem (hWndTree, hTDomain, NULL, nss.szDomain, 
								IDX_DOMAIN, FALSE, FALSE, FALSE);
	}

	// insert new server item
	hTServer = (HTLITEM)TLI_LAST;
	SetOneItem (hWndTree, hTServer, hTDomain, nss.szServer, 
							nss.idx, FALSE, TRUE, TRUE);

	// expand domain of new server
	tlI.hItem = hTDomain;
	TreeList_Expand (hWndTree, &tlI, TLE_EXPAND);

	InvalidateRect (hWndTree, NULL, TRUE);

	++uNumberOfServers;

	return 0;
}

//	_____________________________________________________
//
//  create new keyserver and add to treelist

VOID
CDEditKeyserver (HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM tlI;
	NEWSERVERSTRUCT nss;
	HTLITEM hTDomain;

	nss.hWndTree = hWndTree;
	nss.bEdit = TRUE;

	// get server name and handle of domain
	tlI.hItem = hTSelected;
	tlI.mask = TLIF_TEXT | TLIF_PARENTHANDLE;
	tlI.pszText = nss.szServer;
	tlI.cchTextMax = sizeof(nss.szServer);
	TreeList_GetItem (hWndTree, &tlI);
	hTDomain = tlI.hItem;

	// get domain name
	tlI.mask = TLIF_TEXT;
	tlI.pszText = nss.szDomain;
	tlI.cchTextMax = sizeof(nss.szDomain);
	TreeList_GetItem (hWndTree, &tlI);

	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_NEWSERVER), hWndParent, 
		NewServerDlgProc, (LPARAM)&nss)) {

		// insert new server text into treelist
		tlI.hItem = hTSelected;
		tlI.mask = TLIF_TEXT;
		tlI.cchTextMax = lstrlen (nss.szServer);
		tlI.pszText = nss.szServer;
		TreeList_SetItem (hWndTree, &tlI);
	}
}

//	_____________________________________________________
//
//  remove selected keyserver from treelist

INT
CDRemoveKeyserver (HWND hWndParent, HWND hWndTree)
{
	CHAR sz0[256];
	CHAR sz1[256];
	CHAR sz2[256];
	TL_TREEITEM tlI;
	HTLITEM hTParent;

	tlI.hItem = hTSelected;
	tlI.mask = TLIF_TEXT | TLIF_PARENTHANDLE;
	tlI.pszText = sz1;
	tlI.cchTextMax = sizeof(sz1);
	TreeList_GetItem (hWndTree, &tlI);
	hTParent = tlI.hItem;

	LoadString (g_hInst, IDS_DELKSPROMPT, sz0, sizeof(sz0));
	wsprintf (sz2, sz0, sz1);

	LoadString (g_hInst, IDS_PROPCAPTION, sz1, sizeof(sz1));
	if (MessageBox (hWndParent, sz2, sz1, 
			MB_YESNO|MB_ICONEXCLAMATION) == IDYES) {
		// delete selected item
		tlI.hItem = hTSelected;
		TreeList_DeleteItem (hWndTree, &tlI);

		// get number of remaining children of parent
		tlI.hItem = hTParent;
		tlI.mask = TLIF_CHILDHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		if (!tlI.hItem) {
			tlI.hItem = hTParent;
			TreeList_DeleteItem (hWndTree, &tlI);
		}
		hTSelected = NULL;
		iSelectedType = IDX_NONE;

		// finish up
		InvalidateRect (hWndTree, NULL, TRUE);
		EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
							FALSE);
		EnableWindow (GetDlgItem (hWndParent, IDC_SETDEFAULTKEYSERVER),
							FALSE);
		--uNumberOfServers;
	}
	return 0;
}

//	_____________________________________________________
//
//  set selected keyserver as default

INT
CDSetDefaultKeyserver (HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM tlI;

	tlI.hItem = hTDefault;
	tlI.mask = TLIF_STATE;
	tlI.stateMask = TLIS_BOLD;
	tlI.state = 0;
	TreeList_SetItem (hWndTree, &tlI);

	tlI.hItem = hTSelected;
	tlI.mask = TLIF_STATE;
	tlI.stateMask = TLIS_BOLD;
	tlI.state = TLIS_BOLD;
	TreeList_SetItem (hWndTree, &tlI);

	hTDefault = hTSelected;

	EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), FALSE);
	EnableWindow (GetDlgItem (hWndParent, IDC_SETDEFAULTKEYSERVER), FALSE);

	return 0;
}

//	_____________________________________________________
//
//  process treelist notification

VOID
ServerContextMenu (HWND hwnd, INT iSel, INT iX, INT iY)
{
	HMENU	hMC;
	HMENU	hMenuTrackPopup;

	hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUEDITSERVER));
	if (iSel == IDX_SERVER) 
		EnableMenuItem (hMC, IDM_EDITSERVER, MF_BYCOMMAND|MF_ENABLED);
	else
		EnableMenuItem (hMC, IDM_EDITSERVER, MF_BYCOMMAND|MF_GRAYED);

	hMenuTrackPopup = GetSubMenu (hMC, 0);

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					iX, iY, 0, hwnd, NULL);

	DestroyMenu (hMC);
}

//	_____________________________________________________
//
//  process treelist notification

INT
CDProcessKeyserverTreeList (HWND hWndParent, HWND hWndTree, 
							WPARAM wParam, LPARAM lParam)
{
	TL_TREEITEM tlI;
	TL_LISTITEM tlL;
	BOOL bEnable;

	switch (((LPNM_TREELIST)lParam)->hdr.code) {

	case TLN_SELCHANGED :
		hTSelected = ((LPNM_TREELIST)lParam)->itemNew.hItem;

		if (!hTSelected) {
			iSelectedType = IDX_NONE;
			EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
							FALSE);
			EnableWindow (GetDlgItem (hWndParent, IDC_SETDEFAULTKEYSERVER),
							FALSE);
			break;
		}

		tlI.hItem = hTSelected;
		tlI.mask = TLIF_IMAGE | TLIF_STATE;
		TreeList_GetItem (hWndTree, &tlI);
		iSelectedType = tlI.iImage;
	
		switch (iSelectedType) {
		case IDX_DOMAIN :
			EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
							FALSE);
			EnableWindow (GetDlgItem (hWndParent, IDC_SETDEFAULTKEYSERVER),
							FALSE);
			break;

		case IDX_SERVER :
			if ((uNumberOfServers > 1) && !(tlI.state & TLIS_BOLD)) 
				bEnable = TRUE;
			else 
				bEnable = FALSE;
			EnableWindow (GetDlgItem (hWndParent, IDC_REMOVEKEYSERVER), 
								bEnable);

			if (tlI.state & TLIS_BOLD) bEnable = FALSE;
			else bEnable = TRUE;
			EnableWindow (GetDlgItem (hWndParent, IDC_SETDEFAULTKEYSERVER),
								bEnable);
			break;
		}
		break;

	case TLN_LISTITEMCLICKED : 
		tlL.hItem = hTSelected;
		tlL.iSubItem = 1;
		tlL.mask = TLIF_DATAVALUE;
		TreeList_GetListItem (hWndTree, &tlL);

		if (tlL.lDataValue == IDX_UNCHECKED) tlL.lDataValue = IDX_CHECKED;
		else tlL.lDataValue = IDX_UNCHECKED;

		TreeList_SetListItem (hWndTree, &tlL, FALSE);
		break;

	case TLN_CONTEXTMENU :
		if (iSelectedType != IDX_NONE) {
			ServerContextMenu (hWndParent, iSelectedType,
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
		}
		break;

	}
	return 0;
}

//	_____________________________________________________
//
//  traverse treelist and save contents to preferences

INT
CDSaveKeyserverPrefs (PGPPrefRef PrefRef, HWND hWndParent, HWND hWndTree)
{
	TL_TREEITEM			tlI;
	TL_LISTITEM			tlL;
	BOOL				bDefault, bListed, bFirstServerInDomain;
	HTLITEM				hTDomain, hTServer;
	PGPKeyServerEntry*	keyserverList;
	INT					iServer, iServerCount;

	// count number of servers
	iServerCount = 0;
	hTDomain = TreeList_GetFirstItem (hWndTree);
	while (hTDomain) {
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_CHILDHANDLE;
		TreeList_GetItem (hWndTree, &tlI);

		hTServer = tlI.hItem;
		while (hTServer) {
			++iServerCount;
			tlI.hItem = hTServer;
			tlI.mask = TLIF_NEXTHANDLE;
			TreeList_GetItem (hWndTree, &tlI);

			hTServer = tlI.hItem;
		}

		// step to next domain
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		hTDomain = tlI.hItem;
	}

	keyserverList = pcAlloc (iServerCount * sizeof(PGPKeyServerEntry));
	if (!keyserverList) return kPGPError_OutOfMemory;

	iServer = 0;
	hTDomain = TreeList_GetFirstItem (hWndTree);
	while (hTDomain) {
		// get domain name
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_TEXT | TLIF_CHILDHANDLE;
		tlI.cchTextMax = sizeof (keyserverList->domain);
		tlI.pszText = keyserverList[iServer].domain;
		TreeList_GetItem (hWndTree, &tlI);

		hTServer = tlI.hItem;
		bFirstServerInDomain = TRUE;
		while (hTServer) {
			// get domain name
			if (!bFirstServerInDomain) {
				lstrcpy (keyserverList[iServer].domain, 
							keyserverList[iServer-1].domain);
			}

			// get server name
			tlI.hItem = hTServer;
			tlI.mask = TLIF_TEXT | TLIF_STATE | TLIF_NEXTHANDLE;
			tlI.cchTextMax = sizeof (keyserverList->serverURL);
			tlI.pszText = keyserverList[iServer].serverURL;
			TreeList_GetItem (hWndTree, &tlI);

			// get server default status
			if (tlI.state & TLIS_BOLD) bDefault = TRUE;
			else bDefault = FALSE;

			// get server listed status
			tlL.hItem = hTServer;
			tlL.mask = TLIF_DATAVALUE;
			tlL.iSubItem = 1;
			TreeList_GetListItem (hWndTree, &tlL);
			if (tlL.lDataValue == IDX_CHECKED) bListed = TRUE;
			else bListed = FALSE;

			// set server flags
			keyserverList[iServer].flags = 0;
			if (bListed) keyserverList[iServer].flags |= kKeyServerListed;
			if (bDefault) keyserverList[iServer].flags |= kKeyServerDefault;

			// step to next server
			++iServer;
			bFirstServerInDomain = FALSE;
			hTServer = tlI.hItem;
		}

		// step to next domain
		tlI.hItem = hTDomain;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hWndTree, &tlI);
		hTDomain = tlI.hItem;
	}

	// done, set preferences
	PGPSetKeyServerPrefs (PrefRef, keyserverList, iServerCount);

	pcFree (keyserverList);

	return 0;
}


