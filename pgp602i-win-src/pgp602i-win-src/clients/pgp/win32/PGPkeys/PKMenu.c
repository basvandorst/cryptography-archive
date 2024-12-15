/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPkeysMenu.c - handle menu enabling/disabling chores
	

	$Id: PKMenu.c,v 1.54 1998/08/11 15:20:08 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkeysx.h"

// pgp header files
#include "pgpkeyserverprefs.h"

// constant definitions
#define	SENDTOSERVERPOS	0

#define MENU_FILE		0
#define MENU_EDIT		1
#define MENU_VIEW		2
#define MENU_KEYS		3
#define MENU_SERVER		4
#define MENU_GROUPS		5
#define MENU_HELP		6

#define SUBMENU_ADD		2

// External globals
extern HINSTANCE		g_hInst;
extern PGPBoolean		g_bKeyGenEnabled;
extern BOOL				g_bReadOnly;
extern PGPContextRef	g_Context;

//	___________________________________________________
//
//	create popup menu which is list of keyservers

static HMENU
sCreateKeyserverMenu (VOID)
{
	PGPUInt32			uNumServers	= 0;
	HMENU				hmenu		= NULL;

	PGPKeyServerEntry*	keyserverList;
	PGPPrefRef			prefref;
	PGPUInt32			u, uID;
	PGPError			err;
	CHAR				sz[256];

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), 
									&prefref);
	if (IsPGPError (err)) return NULL;

	hmenu = CreatePopupMenu ();
	err = PGPGetKeyServerPrefs (prefref, &keyserverList, &uNumServers);

	if (IsntPGPError (err)) {
		LoadString (g_hInst, IDS_DOMAINSERVER, sz, sizeof(sz));
		AppendMenu (hmenu, MF_STRING, IDM_DOMAINKEYSERVERX, sz);
		AppendMenu (hmenu, MF_SEPARATOR, 0, NULL);

		uID = 1;
		for (u=0; u<uNumServers; u++) {
			if (IsKeyServerListed (keyserverList[u].flags)) {
				PGPGetKeyServerURL (&(keyserverList[u]), sz);
				AppendMenu (hmenu, MF_STRING, IDM_DOMAINKEYSERVERX +uID, sz);
				++uID;
			}
		}

		if (keyserverList) PGPDisposePrefData (prefref, keyserverList);
	}

	PGPclCloseClientPrefs (prefref, FALSE);

	return hmenu;
}


//	___________________________________________________
//
//	initialize the main window menu based on build flags

VOID
PKInitMenuKeyMan (HMENU hmenu)
{
#if !PGP_ADMIN_BUILD
	HMENU			hmenuServer;

	// get "Servers" menu
	hmenuServer = GetSubMenu (hmenu, MENU_SERVER);

	// delete "Send Group Lists" item
	DeleteMenu (hmenuServer, IDM_SENDGROUPLISTS, MF_BYCOMMAND);

#if !PGP_BUSINESS_SECURITY
	// delete "Update" items
	DeleteMenu (hmenuServer, IDM_UPDATEGROUPLISTS, MF_BYCOMMAND);
	DeleteMenu (hmenuServer, IDM_UPDATEINTRODUCERS, MF_BYCOMMAND);

	// delete separator
	DeleteMenu (hmenuServer, (GetMenuItemCount (hmenuServer) -1), 
					MF_BYPOSITION);

#endif //!PGP_BUSINESS_SECURITY
#endif //!PGP_ADMIN_BUILD
}


//	___________________________________________________
//
//	derive the keyserver string name from the menu ID

VOID
PKGetServerFromID (
		UINT				uID, 
		PGPKeyServerEntry*	pkeyserver)
{
	PGPUInt32			uNumServers		= 0;
	HMENU				hmenu			= NULL;
	PGPKeyServerEntry*	keyserverList	= NULL;
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;

	PGPUInt32			u1, u2;
	PGPError			err;
//	CHAR				sz[kPGPMaxUserIDSize];
//	PGPKeySetRef		keyset;

	err = PGPclOpenClientPrefs (PGPGetContextMemoryMgr (g_Context), 
									&prefref);
	if (IsPGPError (err)) return;

	if (uID == IDM_DOMAINKEYSERVERX) {
		err = PGPCreateKeyServerPath (prefref, "", 
							&keyserverList, &uNumServers);
		if (IsntPGPError (err)) {
			CopyMemory (pkeyserver, &(keyserverList[0]),
									sizeof(PGPKeyServerEntry));
			PGPDisposeKeyServerPath (keyserverList);
		}
	}
	else {
		PGPGetKeyServerPrefs (prefref, &keyserverList, &uNumServers);

		if ((uID - IDM_DOMAINKEYSERVERX) <= uNumServers) {
			u1 = IDM_DOMAINKEYSERVERX+1;
			for (u2=0; u2<uNumServers; u2++) {
				if (IsKeyServerListed (keyserverList[u2].flags)) {
					if (uID == u1) {
						CopyMemory (pkeyserver, &(keyserverList[u2]),
									sizeof(PGPKeyServerEntry));
						break;
					}
					u1++;
				}
			}
		}
		if (keyserverList) PGPDisposePrefData (prefref, keyserverList);
	}

	PGPclCloseClientPrefs (prefref, FALSE);

	return;
}


//	_____________________________________________________
//
//  Set the Expand/Collapse items to "All" or "Selection"

static VOID 
sSetMenuAllOrSelected (
		HMENU	hMenu, 
		BOOL	bAll) 
{
	MENUITEMINFO	mii;
	CHAR			sz[64];

	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	if (bAll) {
		LoadString (g_hInst, IDS_COLLAPSEALL, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_COLLAPSESEL, FALSE, &mii);
		LoadString (g_hInst, IDS_EXPANDALL, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_EXPANDSEL, FALSE, &mii);
	}
	else {
		LoadString (g_hInst, IDS_COLLAPSESEL, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_COLLAPSESEL, FALSE, &mii);
		LoadString (g_hInst, IDS_EXPANDSEL, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_EXPANDSEL, FALSE, &mii);
	}
}

//	______________________________________________
//
//  Set the "Set As ..." to "Default" or "Primary"

static VOID 
sSetMenuDefaultOrPrimary (
		HMENU	hMenu, 
		BOOL	bDefault) 
{
	MENUITEMINFO	mii;
	CHAR			sz[64];

	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	if (bDefault) {
		LoadString (g_hInst, IDS_SETASDEFAULT, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_SETASDEFAULT, FALSE, &mii);
	}
	else {
		LoadString (g_hInst, IDS_SETASPRIMARY, sz, sizeof(sz));
		mii.dwTypeData = sz;
		mii.cch = lstrlen (sz);
		SetMenuItemInfo (hMenu, IDM_SETASDEFAULT, FALSE, &mii);
	}
}

//	____________________________________
//
//  Enable/Disable menu items

static VOID 
sSetItem (
		HMENU	hMenu, 
		INT		iId, 
		BOOL	bEnable) 
{
	if (bEnable)
		EnableMenuItem (hMenu, iId, MF_BYCOMMAND|MF_ENABLED);
	else 
		EnableMenuItem (hMenu, iId, MF_BYCOMMAND|MF_GRAYED);
}

//	_______________________________________________
//
//  Enable/Disable menu items on basis of currently
//  focused key type

VOID 
PKSetMainMenu (PGPKEYSSTRUCT* ppks) 
{
	HMENU			hMP;
	HKEYMAN			hKM;
	HGROUPMAN		hGM;
	ULONG			ulColumns;
	CHAR			sz[64];


	hKM = ppks->hKM;
	hGM = ppks->hGM;

	// "Edit" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, MENU_EDIT);

	sSetItem (hMP, IDM_COPYKEY, PGPkmIsActionEnabled (hKM, KM_COPY));
	if (ppks->bGroupsFocused) {
		sSetItem (hMP, IDM_PASTEKEY, PGPgmIsActionEnabled (hGM, GM_PASTE));
		sSetItem (hMP, IDM_DELETEKEY, PGPgmIsActionEnabled (hGM, GM_DELETE));
		sSetMenuAllOrSelected (hMP, 
							!PGPgmIsActionEnabled (hGM, GM_EXPANDSEL));
	}
	else {
		sSetItem (hMP, IDM_PASTEKEY, PGPkmIsActionEnabled (hKM, KM_PASTE));
		sSetItem (hMP, IDM_DELETEKEY, PGPkmIsActionEnabled (hKM, KM_DELETE));
		sSetMenuAllOrSelected (hMP, 
							!PGPkmIsActionEnabled (hKM, KM_EXPANDSEL));
	}

	// "View" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, MENU_VIEW);
	PGPkmGetSelectedColumns (hKM, &ulColumns);

	if (ulColumns & KM_VALIDITY)
		CheckMenuItem (hMP, IDM_VIEWVALIDITY, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWVALIDITY, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_SIZE)
		CheckMenuItem (hMP, IDM_VIEWSIZE, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWSIZE, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_DESCRIPTION)
		CheckMenuItem (hMP, IDM_VIEWDESC, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWDESC, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_KEYID)
		CheckMenuItem (hMP, IDM_VIEWKEYID, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWKEYID, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_TRUST)
		CheckMenuItem (hMP, IDM_VIEWTRUST, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWTRUST, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_CREATION)
		CheckMenuItem (hMP, IDM_VIEWCREATION, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWCREATION, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_EXPIRATION)
		CheckMenuItem (hMP, IDM_VIEWEXPIRATION, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWEXPIRATION, MF_BYCOMMAND|MF_UNCHECKED);

	if (ulColumns & KM_ADK)
		CheckMenuItem (hMP, IDM_VIEWADK, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWADK, MF_BYCOMMAND|MF_UNCHECKED);

	if (ppks->iToolHeight > 0) 
		CheckMenuItem (hMP, IDM_VIEWTOOLBAR, MF_BYCOMMAND|MF_CHECKED);
	else
		CheckMenuItem (hMP, IDM_VIEWTOOLBAR, MF_BYCOMMAND|MF_UNCHECKED);


	// "Keys" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, MENU_KEYS);

	sSetItem (hMP, IDM_CERTIFYKEY, PGPkmIsActionEnabled (hKM, KM_CERTIFY));
	sSetItem (hMP, IDM_ENABLEKEY, PGPkmIsActionEnabled (hKM, KM_ENABLE));
	sSetItem (hMP, IDM_DISABLEKEY, PGPkmIsActionEnabled (hKM, KM_DISABLE));
	sSetItem (hMP, IDM_REVOKEKEY, PGPkmIsActionEnabled (hKM, KM_REVOKE));
	sSetItem (hMP, IDM_REVERIFY, 
							PGPkmIsActionEnabled (hKM, KM_REVERIFYSIGS));

	if (PGPkmIsActionEnabled (hKM, KM_SETASPRIMARY)) {
		sSetMenuDefaultOrPrimary (hMP, FALSE);
		EnableMenuItem (hMP, IDM_SETASDEFAULT, MF_BYCOMMAND|MF_ENABLED);
	}
	else {
		sSetMenuDefaultOrPrimary (hMP, TRUE);
		sSetItem (hMP, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASDEFAULT));
	}

	sSetItem (hMP, IDM_CREATEKEY, (!g_bReadOnly) && g_bKeyGenEnabled);

	sSetItem (hMP, IDM_SPLITKEY, PGPkmIsActionEnabled (hKM, KM_SPLITKEY));

	sSetItem (hMP, IDM_IMPORTKEYS, PGPkmIsActionEnabled (hKM, KM_IMPORT));
	sSetItem (hMP, IDM_EXPORTKEYS, PGPkmIsActionEnabled (hKM, KM_EXPORT));

	sSetItem (hMP, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));

	// "Add" popup submenu
	hMP = GetSubMenu (hMP, SUBMENU_ADD);
	sSetItem (hMP, IDM_ADDUSERID, PGPkmIsActionEnabled (hKM, KM_ADDUSERID));
	sSetItem (hMP, IDM_ADDPHOTOID, PGPkmIsActionEnabled (hKM, KM_ADDPHOTOID));
	sSetItem (hMP, IDM_ADDREVOKER, PGPkmIsActionEnabled (hKM, KM_ADDREVOKER));

	// "Servers" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, MENU_SERVER);

	DeleteMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION);
	LoadString (g_hInst, IDS_SENDTOSERVERMENU, sz, sizeof(sz));
	if (PGPkmIsActionEnabled (hKM, KM_SENDTOSERVER)) {
		ppks->hMenuKeyserver = sCreateKeyserverMenu ();
		InsertMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION|MF_POPUP|MF_STRING, 
			(UINT)ppks->hMenuKeyserver, sz);
	}
	else {		
		InsertMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION|MF_STRING, 
			IDM_SENDTOSERVERPOPUP, sz);
		sSetItem (hMP, IDM_SENDTOSERVERPOPUP, FALSE);
	}

	sSetItem (hMP, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));

	// "Groups" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, MENU_GROUPS);

	if (ppks->bGroupsVisible) {
		sSetItem (hMP, IDM_GROUPPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_PROPERTIES));
	}
	else {
		sSetItem (hMP, IDM_GROUPPROPERTIES, FALSE);
	}

}

//	______________________________________________________
//
//  Put up appropriate context menu on basis of key idx
//  of currently focused item.  Called in response to right
//  mouse click.
//
//	hWnd		= handle of parent window
//	uSelFlags	= bits indicate what types of objects are selected
//	x, y		= mouse position when right button clicked (screen coords)

VOID 
PKContextMenu (
		PGPKEYSSTRUCT*	ppks,
		HWND			hWnd, 
		INT				x, 
		INT				y) 
{
	HMENU			hMC;
	HMENU			hMCS;
	HMENU			hMenuTrackPopup;
	HKEYMAN			hKM;
	HGROUPMAN		hGM;
	UINT			uAddUserIDIndex;
	CHAR			sz[64];

	if (ppks->bGroupsFocused) {
		hGM = ppks->hGM;
		switch (ppks->uGroupSelectionFlags) {
		case 0 :
			hMC = NULL;
			break;

		case PGPGM_GROUPFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUP));
			sSetItem (hMC, IDM_PASTEKEY, 
							PGPgmIsActionEnabled (hGM, GM_PASTE));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			sSetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			sSetItem (hMC, IDM_GROUPPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_PROPERTIES));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPGM_KEYFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUPKEY));
			sSetItem (hMC, IDM_PASTEKEY, 
							PGPgmIsActionEnabled (hGM, GM_PASTE));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			sSetItem (hMC, IDM_FINDKEY, 
							PGPgmIsActionEnabled (hGM, GM_LOCATEKEYS));
			sSetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			sSetItem (hMC, IDM_KEYPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_LOCATEKEYS));
			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		default :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUP));
			sSetItem (hMC, IDM_PASTEKEY, FALSE);
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			sSetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			sSetItem (hMC, IDM_GROUPPROPERTIES, FALSE);
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;
		}
	}
	else {
		hKM = ppks->hKM;

		switch (ppks->uKeySelectionFlags) {
		case 0 :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUNONE));
			sSetItem (hMC, IDM_PASTEKEY, 
							PGPkmIsActionEnabled (hKM, KM_PASTE));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPKM_KEYFLAG :
			if (ppks->bMainWindow) {
				hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUKEY));
				uAddUserIDIndex = 6;
			}
			else {
				if (ppks->bLocalKeySet) {
					hMC = LoadMenu (g_hInst, 
							MAKEINTRESOURCE (IDR_MENUKEYSEARCHLOCAL));
					uAddUserIDIndex = 6;
				}
				else {
					hMC = LoadMenu (g_hInst, 
							MAKEINTRESOURCE (IDR_MENUKEYSEARCHSERVER));
					uAddUserIDIndex = 7;
				}
				sSetItem (hMC, IDM_ADDTOMAIN, 
							PGPkmIsActionEnabled (hKM, KM_ADDTOMAIN));
			}
  			hMenuTrackPopup = GetSubMenu (hMC, 0);

			sSetItem (hMC, IDM_COPYKEY, 
							PGPkmIsActionEnabled (hKM, KM_COPY));
			sSetItem (hMC, IDM_PASTEKEY, 
							PGPkmIsActionEnabled (hKM, KM_PASTE));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			sSetItem (hMC, IDM_DELETESERVER, 
							PGPkmIsActionEnabled (hKM, KM_DELETEFROMSERVER));
			sSetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			sSetItem (hMC, IDM_ENABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_ENABLE));
			if (ppks->bLocalKeySet)
				sSetItem (hMC, IDM_DISABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DISABLE));
			else
				sSetItem (hMC, IDM_DISABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DISABLEONSERVER));

			sSetItem (hMC, IDM_SPLITKEY, 
							PGPkmIsActionEnabled (hKM, KM_SPLITKEY));

			sSetItem (hMC, IDM_REVOKEKEY, 
							PGPkmIsActionEnabled (hKM, KM_REVOKE));
			sSetItem (hMC, IDM_REVERIFY, 
							PGPkmIsActionEnabled (hKM, KM_REVERIFYSIGS));
			sSetItem (hMC, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASDEFAULT));
			sSetItem (hMC, IDM_EXPORTKEYS, 
							PGPkmIsActionEnabled (hKM, KM_EXPORT));

			LoadString (g_hInst, IDS_SENDTOSERVERMENU, sz, sizeof(sz));
			if (PGPkmIsActionEnabled (hKM, KM_SENDTOSERVER)) {
				ppks->hMenuKeyserver = sCreateKeyserverMenu ();
				InsertMenu (hMenuTrackPopup, 13, 
					MF_BYPOSITION|MF_POPUP|MF_STRING, 
					(UINT)ppks->hMenuKeyserver, sz);
			}
			else {		
				InsertMenu (hMC, 13, MF_BYPOSITION|MF_STRING, 
					IDM_SENDTOSERVERPOPUP, sz);
				sSetItem (hMenuTrackPopup, IDM_SENDTOSERVERPOPUP, FALSE);
			}

			sSetItem (hMC, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));
			sSetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));

			// take care of "Add" popup submenu
			hMCS = GetSubMenu (hMenuTrackPopup, uAddUserIDIndex);
			sSetItem (hMCS, IDM_ADDUSERID, 
							PGPkmIsActionEnabled (hKM, KM_ADDUSERID));
			sSetItem (hMCS, IDM_ADDPHOTOID, 
							PGPkmIsActionEnabled (hKM, KM_ADDPHOTOID));
			sSetItem (hMCS, IDM_ADDREVOKER, 
							PGPkmIsActionEnabled (hKM, KM_ADDREVOKER));
			break;

		case PGPKM_UIDFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUUID));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			sSetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			sSetItem (hMC, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASPRIMARY));
			sSetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
	  		hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPKM_CERTFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUCERT));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			sSetItem (hMC, IDM_REVOKEKEY, 
							PGPkmIsActionEnabled (hKM, KM_REVOKE));
			sSetItem (hMC, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));
			sSetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		default :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUPROMISCUOUS));
			sSetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			sSetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			sSetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;
		}
	}

	if (!hMC) return;

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					x, y, 0, hWnd, NULL);

	DestroyMenu (hMC);

}


//	______________________________________________________
//
//  Put up keyserver menu for send to server button in toolbar
//
//	hWnd		= handle of parent window

VOID 
PKToolbarKeyserverMenu (
		HWND			hWnd,
		LPRECT			lprect) 
{
	HMENU		hMenuKeyserver;
	POINT		pt;

	hMenuKeyserver = sCreateKeyserverMenu ();
	GetCursorPos (&pt);
	TrackPopupMenu (hMenuKeyserver, 
					TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON,
					lprect->left +2, lprect->bottom +4, 0, hWnd, NULL);

	DestroyMenu (hMenuKeyserver);
}