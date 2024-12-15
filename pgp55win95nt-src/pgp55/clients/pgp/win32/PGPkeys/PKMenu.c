/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPkeysMenu.c - handle menu enabling/disabling chores
	

	$Id: PKMenu.c,v 1.28.4.1 1997/11/06 23:17:13 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkeysx.h"
#include "resource.h"
#include "pgpkeyserverprefs.h"

#include "pgpBuildFlags.h"

#define	SENDTOSERVERPOS	5

// External globals
extern HINSTANCE	g_hInst;
extern PGPBoolean	g_bKeyGenEnabled;
extern BOOL			g_bReadOnly;

//	___________________________________________________
//
//	create popup menu which is list of keyservers

HMENU
CreateKeyserverMenu (VOID)
{
	PGPUInt32			uNumServers	= 0;
	HMENU				hmenu		= NULL;
	BOOL				bSeparated	= FALSE;

	PGPKeyServerEntry*	keyserverList;
	PGPPrefRef			prefref;
	PGPUInt32			u1, u2;
	PGPError			err;
	CHAR				sz[64];

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsPGPError (err)) return NULL;

	u1 = sizeof (sz);
	err = PGPGetDefaultKeyServer (prefref, &u1, sz);
	if (IsPGPError (err)) return NULL;

	hmenu = CreatePopupMenu ();
	AppendMenu (hmenu, MF_STRING, IDM_DEFAULTKEYSERVER, sz);

	PGPGetKeyServerPrefs (prefref, &keyserverList, &uNumServers);

	u1 = 1;
	for (u2=0; u2<uNumServers; u2++) {
		if (!IsKeyServerDefault (keyserverList[u2].flags)) {
			if (IsKeyServerListed (keyserverList[u2].flags)) {
				if (!bSeparated) {
					AppendMenu (hmenu, MF_SEPARATOR, 0, 0);
					bSeparated = TRUE;
				}
				AppendMenu (hmenu, MF_STRING, IDM_DEFAULTKEYSERVER +u1, 
						keyserverList[u2].serverURL);
				++u1;
			}
		}
	}

	if (keyserverList) PGPDisposePrefData (prefref, keyserverList);
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	return hmenu;
}


//	___________________________________________________
//
//	derive the keyserver string name from the menu ID

VOID
PKGetServerFromID (UINT uID, LPSTR szServer, UINT uLen)
{
	PGPUInt32			uNumServers	= 0;
	HMENU				hmenu		= NULL;

	PGPKeyServerEntry*	keyserverList;
	PGPPrefRef			prefref;
	PGPUInt32			u1, u2;
	PGPError			err;

	szServer[0] = '\0';

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsPGPError (err)) return;

	if (uID == IDM_DEFAULTKEYSERVER) {
		PGPGetDefaultKeyServer (prefref, &uLen, szServer);
		PGPcomdlgCloseClientPrefs (prefref, FALSE);
		return;
	}

	PGPGetKeyServerPrefs (prefref, &keyserverList, &uNumServers);

	if ((uID - IDM_DEFAULTKEYSERVER) < uNumServers) {
		u1 = IDM_DEFAULTKEYSERVER;
		for (u2=0; u2<uNumServers; u2++) {
			if (!IsKeyServerDefault (keyserverList[u2].flags)) {
				if (IsKeyServerListed (keyserverList[u2].flags)) {
					u1++;
					if (uID == u1) {
						lstrcpyn (szServer, 
							keyserverList[u2].serverURL, uLen);
						break;
					}
				}
			}
		}
	}

	if (keyserverList) PGPDisposePrefData (prefref, keyserverList);
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	return;
}


//	_____________________________________________________
//
//  Set the Expand/Collapse items to "All" or "Selection"

VOID 
SetMenuAllOrSelected (
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

VOID 
SetMenuDefaultOrPrimary (
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

//	______________________________________________
//
//  Change the "Register ..." to "Upgrade ..."

VOID 
SetMenuUpgrade (
		HMENU	hMenu) 
{
	MENUITEMINFO	mii;
	CHAR			sz[64];

	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	LoadString (g_hInst, IDS_UPGRADE, sz, sizeof(sz));
	mii.dwTypeData = sz;
	mii.cch = lstrlen (sz);
	SetMenuItemInfo (hMenu, IDM_REGISTER, FALSE, &mii);
}

//	____________________________________
//
//  Enable/Disable menu items

VOID 
SetItem (
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
PGPkeysSetMainMenu (PGPKEYSSTRUCT* ppks) 
{
	HMENU			hMP;
	HKEYMAN			hKM;
	HGROUPMAN		hGM;
	CHAR			sz[64];


	hKM = ppks->hKM;
	hGM = ppks->hGM;

	// "Edit" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, 1);

	SetItem (hMP, IDM_COPYKEY, PGPkmIsActionEnabled (hKM, KM_COPY));
	if (ppks->bGroupsFocused) {
		SetItem (hMP, IDM_PASTEKEY, PGPgmIsActionEnabled (hGM, GM_PASTE));
		SetItem (hMP, IDM_DELETEKEY, PGPgmIsActionEnabled (hGM, GM_DELETE));
		SetMenuAllOrSelected (hMP, !PGPgmIsActionEnabled (hGM, GM_EXPANDSEL));
	}
	else {
		SetItem (hMP, IDM_PASTEKEY, PGPkmIsActionEnabled (hKM, KM_PASTE));
		SetItem (hMP, IDM_DELETEKEY, PGPkmIsActionEnabled (hKM, KM_DELETE));
		SetMenuAllOrSelected (hMP, !PGPkmIsActionEnabled (hKM, KM_EXPANDSEL));
	}

	// "Keys" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, 2);

	SetItem (hMP, IDM_CERTIFYKEY, PGPkmIsActionEnabled (hKM, KM_CERTIFY));
	SetItem (hMP, IDM_ENABLEKEY, PGPkmIsActionEnabled (hKM, KM_ENABLE));
	SetItem (hMP, IDM_DISABLEKEY, PGPkmIsActionEnabled (hKM, KM_DISABLE));
	SetItem (hMP, IDM_ADDUSERID, PGPkmIsActionEnabled (hKM, KM_ADDUSERID));
	SetItem (hMP, IDM_REVOKEKEY, PGPkmIsActionEnabled (hKM, KM_REVOKE));

	if (PGPkmIsActionEnabled (hKM, KM_SETASPRIMARY)) {
		SetMenuDefaultOrPrimary (hMP, FALSE);
		EnableMenuItem (hMP, IDM_SETASDEFAULT, MF_BYCOMMAND|MF_ENABLED);
	}
	else {
		SetMenuDefaultOrPrimary (hMP, TRUE);
		SetItem (hMP, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASDEFAULT));
	}

	SetItem (hMP, IDM_CREATEKEY, (!g_bReadOnly) && g_bKeyGenEnabled);

	SetItem (hMP, IDM_IMPORTKEYS, PGPkmIsActionEnabled (hKM, KM_IMPORT));
	SetItem (hMP, IDM_EXPORTKEYS, PGPkmIsActionEnabled (hKM, KM_EXPORT));

	DeleteMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION);
	LoadString (g_hInst, IDS_SENDTOSERVERMENU, sz, sizeof(sz));
	if (PGPkmIsActionEnabled (hKM, KM_SENDTOSERVER)) {
		ppks->hMenuKeyserver = CreateKeyserverMenu ();
		InsertMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION|MF_POPUP|MF_STRING, 
			(UINT)ppks->hMenuKeyserver, sz);
	}
	else {		
		InsertMenu (hMP, SENDTOSERVERPOS, MF_BYPOSITION|MF_STRING, 
			IDM_SENDTOSERVERPOPUP, sz);
		SetItem (hMP, IDM_SENDTOSERVERPOPUP, FALSE);
	}

	SetItem (hMP, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));
	SetItem (hMP, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));

	// "Groups" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, 3);

	if (ppks->bGroupsVisible) {
		SetItem (hMP, IDM_GROUPPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_PROPERTIES));
	}
	else {
		SetItem (hMP, IDM_GROUPPROPERTIES, FALSE);
	}

#if PGP_FREEWARE
	// "Help" menu
	hMP = GetSubMenu (ppks->hMenuKeyMan, 4);
	SetMenuUpgrade (hMP);
#endif

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
PGPkeysContextMenu (
		PGPKEYSSTRUCT*	ppks,
		HWND			hWnd, 
		INT				x, 
		INT				y) 
{
	HMENU			hMC;
	HMENU			hMenuTrackPopup;
	HKEYMAN			hKM;
	HGROUPMAN		hGM;
	CHAR			sz[64];

	if (ppks->bGroupsFocused) {
		hGM = ppks->hGM;
		switch (ppks->uGroupSelectionFlags) {
		case 0 :
			hMC = NULL;
			break;

		case PGPGM_GROUPFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUP));
			SetItem (hMC, IDM_PASTEKEY, PGPgmIsActionEnabled (hGM, GM_PASTE));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			SetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			SetItem (hMC, IDM_GROUPPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_PROPERTIES));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPGM_KEYFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUPKEY));
			SetItem (hMC, IDM_PASTEKEY, PGPgmIsActionEnabled (hGM, GM_PASTE));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			SetItem (hMC, IDM_FINDKEY, 
							PGPgmIsActionEnabled (hGM, GM_LOCATEKEYS));
			SetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			SetItem (hMC, IDM_KEYPROPERTIES, 
							PGPgmIsActionEnabled (hGM, GM_LOCATEKEYS));
			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		default :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUGROUP));
			SetItem (hMC, IDM_PASTEKEY, FALSE);
			SetItem (hMC, IDM_DELETEKEY, 
							PGPgmIsActionEnabled (hGM, GM_DELETE));
			SetItem (hMC, IDM_GETFROMSERVER, 
							PGPgmIsActionEnabled (hGM, GM_GETFROMSERVER));
			SetItem (hMC, IDM_GROUPPROPERTIES, FALSE);
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;
		}
	}
	else {
		hKM = ppks->hKM;

		switch (ppks->uKeySelectionFlags) {
		case 0 :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUNONE));
			SetItem (hMC, IDM_PASTEKEY, PGPkmIsActionEnabled (hKM, KM_PASTE));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPKM_KEYFLAG :
			if (ppks->bMainWindow) 
				hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUKEY));
			else {
#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
				if (ppks->bLocalKeySet) 
					hMC = LoadMenu (g_hInst, 
							MAKEINTRESOURCE (IDR_MENUKEYSEARCHLOCAL));
				else 
					hMC = LoadMenu (g_hInst, 
							MAKEINTRESOURCE (IDR_MENUKEYSEARCHSERVER));
#else
				hMC = LoadMenu (g_hInst, 
							MAKEINTRESOURCE (IDR_MENUKEYSEARCHLOCAL));
#endif	
				SetItem (hMC, IDM_ADDTOMAIN, 
							PGPkmIsActionEnabled (hKM, KM_ADDTOMAIN));
			}
  			hMenuTrackPopup = GetSubMenu (hMC, 0);

			SetItem (hMC, IDM_COPYKEY, 
							PGPkmIsActionEnabled (hKM, KM_COPY));
			SetItem (hMC, IDM_PASTEKEY, 
							PGPkmIsActionEnabled (hKM, KM_PASTE));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
			SetItem (hMC, IDM_DELETESERVER, 
							PGPkmIsActionEnabled (hKM, KM_DELETEFROMSERVER));
#else
			SetItem (hMC, IDM_DELETESERVER, FALSE);
#endif
			SetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			SetItem (hMC, IDM_ENABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_ENABLE));
			if (ppks->bLocalKeySet)
				SetItem (hMC, IDM_DISABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DISABLE));
			else
#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
				SetItem (hMC, IDM_DISABLEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DISABLEONSERVER));
#else
				SetItem (hMC, IDM_DISABLEKEY, FALSE);
#endif
			SetItem (hMC, IDM_ADDUSERID, 
							PGPkmIsActionEnabled (hKM, KM_ADDUSERID));
			SetItem (hMC, IDM_REVOKEKEY, 
							PGPkmIsActionEnabled (hKM, KM_REVOKE));
			SetItem (hMC, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASDEFAULT));
			SetItem (hMC, IDM_EXPORTKEYS, 
							PGPkmIsActionEnabled (hKM, KM_EXPORT));

			LoadString (g_hInst, IDS_SENDTOSERVERMENU, sz, sizeof(sz));
			if (PGPkmIsActionEnabled (hKM, KM_SENDTOSERVER)) {
				ppks->hMenuKeyserver = CreateKeyserverMenu ();
				InsertMenu (hMenuTrackPopup, 13, 
					MF_BYPOSITION|MF_POPUP|MF_STRING, 
					(UINT)ppks->hMenuKeyserver, sz);
			}
			else {		
				InsertMenu (hMC, 13, MF_BYPOSITION|MF_STRING, 
					IDM_SENDTOSERVERPOPUP, sz);
				SetItem (hMenuTrackPopup, IDM_SENDTOSERVERPOPUP, FALSE);
			}

			SetItem (hMC, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));
			SetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
			break;

		case PGPKM_UIDFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUUID));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			SetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			SetItem (hMC, IDM_SETASDEFAULT, 
							PGPkmIsActionEnabled (hKM, KM_SETASPRIMARY));
			SetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
	  		hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		case PGPKM_CERTFLAG :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUCERT));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			SetItem (hMC, IDM_REVOKEKEY, 
							PGPkmIsActionEnabled (hKM, KM_REVOKE));
			SetItem (hMC, IDM_GETFROMSERVER, 
							PGPkmIsActionEnabled (hKM, KM_GETFROMSERVER));
			SetItem (hMC, IDM_KEYPROPERTIES, 
							PGPkmIsActionEnabled (hKM, KM_PROPERTIES));
  			hMenuTrackPopup = GetSubMenu (hMC, 0);
			break;

		default :
			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUPROMISCUOUS));
			SetItem (hMC, IDM_DELETEKEY, 
							PGPkmIsActionEnabled (hKM, KM_DELETE));
			SetItem (hMC, IDM_CERTIFYKEY, 
							PGPkmIsActionEnabled (hKM, KM_CERTIFY));
			SetItem (hMC, IDM_KEYPROPERTIES, 
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