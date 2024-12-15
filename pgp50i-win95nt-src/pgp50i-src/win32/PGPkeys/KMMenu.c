/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMMenu.c - handle menu enabling/disabling chores
//
//	$Id: KMMenu.c,v 1.3 1997/05/09 19:29:19 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"
#include "resource.h"
#include "..\include\pgpkeyserversupport.h"

// External globals
extern HINSTANCE g_hInst;
extern HWND g_hWndTree;
extern CHAR g_szbuf[G_BUFLEN];
extern BOOL g_bReadOnly;
extern BOOL g_bKeyServerSupportLoaded;

// Local globals
static HMENU hMenuKSMainKey = NULL;		//ks drop menu when key is selected
static HMENU hMenuKSMainCert = NULL;	//ks drop menu when cert is selected
static HMENU hMenuKSMainElse = NULL;	//ks drop menu else


//----------------------------------------------------|
//	Create Keyserver menus to be inserted later

VOID KMCreateMenus (HMENU hMenuMain) {
	HMENU hMenuSub;
	INT i, iCount;

	if (g_bKeyServerSupportLoaded) {
		hMenuSub = GetSubMenu (hMenuMain, 2);
		iCount = GetMenuItemCount (hMenuSub);
		for (i=0; i<iCount; i++) {
			if (GetMenuItemID (hMenuSub, i) == IDM_EXPORTKEYS) {
				i++;
				break;
			}
		}

		if (hMenuKSMainKey) {
			KMDestroyMenus ();
			DeleteMenu (hMenuSub, i, MF_BYPOSITION);
		}
	
		hMenuKSMainKey = CreateMenu ();
		hMenuKSMainCert = CreateMenu ();
		hMenuKSMainElse = CreateMenu ();

		if (g_bReadOnly) {
			BuildMenu (KS_NO_GET_NEW_KEY | KS_NO_GET_KEYS,
				hMenuKSMainKey, 0xFFFFFFFF);
			InsertMenu (hMenuKSMainCert, 0,
				MF_BYPOSITION|MF_STRING|MF_GRAYED, 0, "empty");
			InsertMenu (hMenuKSMainElse, 0,
				MF_BYPOSITION|MF_STRING|MF_GRAYED, 0, "empty");
		}
		else {
			BuildMenu (KS_ALL, hMenuKSMainKey, 0xFFFFFFFF);
			BuildMenu (KS_NO_SEND_KEYS, hMenuKSMainCert, 0xFFFFFFFF);
			BuildMenu (KS_NO_SEND_KEYS | KS_NO_GET_KEYS,
				hMenuKSMainElse, 0xFFFFFFFF);
		}

		LoadString (g_hInst, IDS_KEYSERVERMENU, g_szbuf, G_BUFLEN);
		InsertMenu (hMenuSub, i, MF_BYPOSITION|MF_POPUP, 0, g_szbuf);
	}
}

	
//----------------------------------------------------|
//	Destroy Keyserver menus

VOID KMDestroyMenus (void) {
	if (g_bKeyServerSupportLoaded) {
		DestroyMenu (hMenuKSMainKey);
		DestroyMenu (hMenuKSMainCert);
		DestroyMenu (hMenuKSMainElse);
	}
}

	
//----------------------------------------------------|
//	Insert Keyserver menu items into context menu

VOID SetKeyserverMainMenu (HMENU hMenuSub, HMENU hMenuKS) {
	INT i, iCount;

	if (g_bKeyServerSupportLoaded) {
		iCount = GetMenuItemCount (hMenuSub);
		for (i=0; i<iCount; i++) {
			if (GetMenuItemID (hMenuSub, i) == IDM_EXPORTKEYS) {
				i++;
				break;
			}
		}
		LoadString (g_hInst, IDS_KEYSERVERMENU, g_szbuf, G_BUFLEN);
		RemoveMenu (hMenuSub, i, MF_BYPOSITION);
		InsertMenu (hMenuSub, i, MF_BYPOSITION|MF_POPUP,
			(UINT)hMenuKS, g_szbuf);
	}
}

	
//----------------------------------------------------|
//	Insert Keyserver menu items into context menu

VOID SetKeyserverKeyContextMenu (HMENU hMenuContext) {
	INT i, iCount;
	HMENU hMenu;
	HMENU hMenuKS;

	if (g_bKeyServerSupportLoaded) {
		hMenuKS = CreateMenu ();
	
		if (g_bReadOnly)
			BuildMenu (KS_NO_GET_NEW_KEY | KS_NO_GET_KEYS,
						hMenuKS, 0xFFFFFFFF);
		else
			BuildMenu (KS_NO_GET_NEW_KEY, hMenuKS, 0xFFFFFFFF);

		hMenu = GetSubMenu (hMenuContext, 0);
		iCount = GetMenuItemCount (hMenu);
		for (i=0; i<iCount; i++) {
			if (GetMenuItemID (hMenu, i) == IDM_EXPORTKEYS) {
				i++;
				break;
			}
		}
		LoadString (g_hInst, IDS_KEYSERVERMENU, g_szbuf, G_BUFLEN);
		InsertMenu (hMenu, i, MF_BYPOSITION|MF_POPUP, (UINT)hMenuKS, g_szbuf);
	}
}

	
//----------------------------------------------------|
//	Insert Keyserver menu items into context menu

VOID SetKeyserverCertContextMenu (HMENU hMenuContext) {
	INT i, iCount;
	HMENU hMenu;
	HMENU hMenuKS;

	if (g_bKeyServerSupportLoaded && !g_bReadOnly) {
		hMenuKS = CreateMenu ();
		BuildMenu (KS_NO_SEND_KEYS | KS_NO_GET_NEW_KEY, hMenuKS, 0xFFFFFFFF);

		hMenu = GetSubMenu (hMenuContext, 0);
		iCount = GetMenuItemCount (hMenu);
		for (i=0; i<iCount; i++) {
			if (GetMenuItemID (hMenu, i) == IDM_KEYPROPERTIES) {
				break;
			}
		}
		LoadString (g_hInst, IDS_KEYSERVERMENU, g_szbuf, G_BUFLEN);
		InsertMenu (hMenu, i, MF_BYPOSITION|MF_POPUP, (UINT)hMenuKS, g_szbuf);
	}
}


//----------------------------------------------------|
//  Put up appropriate context menu on basis of key idx
//  of currently focused item.  Called in response to right
//  mouse click.
//
//	hWnd	= handle of parent window
//	x, y	= mouse position when right button clicked (screen coords)

VOID KMContextMenu (HWND hWnd, INT x, INT y) {
	HMENU hMenuContext;
	HMENU hMenuTrackPopup;
	Boolean b;
	UINT uWriteEnabled, uPasteEnabled;

	if (g_bReadOnly) {
		uWriteEnabled = MF_BYCOMMAND | MF_GRAYED;
		uPasteEnabled = MF_BYCOMMAND | MF_GRAYED;
	}
	else {
		uWriteEnabled = MF_BYCOMMAND | MF_ENABLED;
		if (KMDataToPaste ()) uPasteEnabled = MF_BYCOMMAND | MF_ENABLED;
		else uPasteEnabled = MF_BYCOMMAND | MF_GRAYED;
	}

	hMenuContext = NULL;

	if (KMPromiscuousSelected ()) {
		hMenuContext = LoadMenu (g_hInst,
			MAKEINTRESOURCE(IDR_MENUPROMISCUOUS));
		EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);
		if (!KMSigningAllowed ())
			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY,
								MF_BYCOMMAND | MF_GRAYED);
		else
			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
	}

	else if (KMMultipleSelected ()) {
		switch (KMFocusedItemType ()) {
		case IDX_RSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_RSASECREVKEY :
		case IDX_RSASECEXPKEY :
		case IDX_RSAPUBKEY :
		case IDX_RSAPUBDISKEY :
		case IDX_RSAPUBREVKEY :
		case IDX_RSAPUBEXPKEY :
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
		case IDX_DSASECREVKEY :
		case IDX_DSASECEXPKEY :
		case IDX_DSAPUBKEY :
		case IDX_DSAPUBDISKEY :
		case IDX_DSAPUBREVKEY :
		case IDX_DSAPUBEXPKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUUID));
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			break;

		case IDX_CERT :
		case IDX_REVCERT :
		case IDX_BADCERT :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUCERT));
			SetKeyserverCertContextMenu (hMenuContext);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_REVOKECERT,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		default :
			return;
		}
	}
	else {
		switch (KMFocusedItemType ()) {
		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			pgpGetKeyBoolean ((PGPKey*)KMFocusedObject (),
				kPGPKeyPropIsAxiomatic, &b);
			if (b) EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuContext, IDM_DISABLEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY, uWriteEnabled);
			break;
	
		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);
			
			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY, uWriteEnabled);
			break;
		
		case IDX_RSASECREVKEY :
		case IDX_DSASECREVKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;
		
		case IDX_RSASECEXPKEY :
		case IDX_DSASECEXPKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;
		
		case IDX_RSAPUBKEY :
		case IDX_DSAPUBKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_RSAPUBDISKEY :
		case IDX_DSAPUBDISKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY, uWriteEnabled);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_RSAPUBEXPKEY :
		case IDX_DSAPUBEXPKEY :
		case IDX_RSAPUBREVKEY :
		case IDX_DSAPUBREVKEY :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUKEY));
			SetKeyserverKeyContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_PASTEKEY, uPasteEnabled);
			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuContext, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUUID));

			if (KMIsThisTheOnlyUserID (g_hWndTree,
										(PGPUserID*)KMFocusedObject ()))
				EnableMenuItem (hMenuContext, IDM_DELETEKEY,
										MF_BYCOMMAND | MF_GRAYED);
			else
				EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			if (KMIsThisThePrimaryUserID (g_hWndTree,
										(PGPUserID*)KMFocusedObject ()))
				EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
										MF_BYCOMMAND | MF_GRAYED);
			else
				EnableMenuItem (hMenuContext, IDM_SETASDEFAULT,
										uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_CERTIFYKEY, uWriteEnabled);
			break;

		case IDX_CERT :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUCERT));
			SetKeyserverCertContextMenu (hMenuContext);

			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
									kPGPCertPropIsMyCert, &b);
			if (!b) EnableMenuItem (hMenuContext, IDM_REVOKECERT,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuContext, IDM_REVOKECERT, uWriteEnabled);

			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
									kPGPCertPropIsVerified, &b);
			if (!b) EnableMenuItem (hMenuContext, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_REVCERT :
		case IDX_BADCERT :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUCERT));

			EnableMenuItem (hMenuContext, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuContext, IDM_REVOKECERT,
				MF_BYCOMMAND | MF_GRAYED);

			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
								kPGPCertPropIsVerified, &b);
			if (!b) EnableMenuItem (hMenuContext, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_NONE :
			hMenuContext = LoadMenu (g_hInst, MAKEINTRESOURCE(IDR_MENUNONE));
			break;

		default :
			return;
		}
	}

	if (!hMenuContext) return;
	hMenuTrackPopup = GetSubMenu (hMenuContext, 0);

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					x, y, 0, hWnd, NULL);

	DestroyMenu (hMenuContext);
}


//----------------------------------------------------|
//  Set the Expand/Collapse items to "All" or "Selection"

VOID SetMenuAllOrSelected (HMENU hMenu, BOOL bAll) {
	MENUITEMINFO mii;

	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	if (bAll) {
		LoadString (g_hInst, IDS_COLLAPSEALL, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_COLLAPSESEL, FALSE, &mii);
		LoadString (g_hInst, IDS_EXPANDALL, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_EXPANDSEL, FALSE, &mii);
	}
	else {
		LoadString (g_hInst, IDS_COLLAPSESEL, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_COLLAPSESEL, FALSE, &mii);
		LoadString (g_hInst, IDS_EXPANDSEL, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_EXPANDSEL, FALSE, &mii);
	}
}

	
//----------------------------------------------------|
//  Set the "Set As ..." to "Default" or "Primary"

VOID SetMenuDefaultOrPrimary (HMENU hMenu, BOOL bDefault) {
	MENUITEMINFO mii;

	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	if (bDefault) {
		LoadString (g_hInst, IDS_SETASDEFAULT, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_SETASDEFAULT, FALSE, &mii);
	}
	else {
		LoadString (g_hInst, IDS_SETASPRIMARY, g_szbuf, G_BUFLEN);
		mii.dwTypeData = g_szbuf;
		mii.cch = lstrlen (g_szbuf);
		SetMenuItemInfo (hMenu, IDM_SETASDEFAULT, FALSE, &mii);
	}
}

	
//----------------------------------------------------|
//  Enable/Disable menu items on basis of currently
//  focused key type

VOID KMSetMainMenu (HMENU hMenu) {
	HMENU hMenuPopup;
	Boolean b;
	UINT uWriteEnabled, uPasteEnabled;

	if (g_bReadOnly) {
		uWriteEnabled = MF_BYCOMMAND | MF_GRAYED;
		uPasteEnabled = MF_BYCOMMAND | MF_GRAYED;
	}
	else {
		uWriteEnabled = MF_BYCOMMAND | MF_ENABLED;
		if (KMDataToPaste ()) uPasteEnabled = MF_BYCOMMAND | MF_ENABLED;
		else uPasteEnabled = MF_BYCOMMAND | MF_GRAYED;
	}

	hMenuPopup = GetSubMenu (hMenu, 1);
	EnableMenuItem (hMenuPopup, IDM_PASTEKEY, uPasteEnabled);

	// multiple objects of different types selected
	if (KMPromiscuousSelected ()) {
		SetMenuAllOrSelected (hMenuPopup, FALSE);

		EnableMenuItem (hMenuPopup, IDM_COPYKEY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

		EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
			MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
			MF_BYCOMMAND | MF_ENABLED);

		hMenuPopup = GetSubMenu (hMenu, 2);
		SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
		SetKeyserverMainMenu (hMenuPopup, hMenuKSMainElse);

		if (KMSigningAllowed ())
			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
		else
			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
			MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
			MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
			MF_BYCOMMAND | MF_GRAYED);

		EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
			MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
			MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
			MF_BYCOMMAND | MF_GRAYED);

		EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
			MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
			MF_BYCOMMAND | MF_ENABLED);
	}

	// multiple objects of the same type selected
	else if (KMMultipleSelected ()) {
		switch (KMFocusedItemType ()) {
		case IDX_RSAPUBKEY :
		case IDX_RSAPUBDISKEY :
		case IDX_RSAPUBREVKEY :
		case IDX_RSAPUBEXPKEY :
		case IDX_RSASECKEY :
		case IDX_RSASECDISKEY :
		case IDX_RSASECREVKEY :
		case IDX_RSASECEXPKEY :
		case IDX_DSAPUBKEY :
		case IDX_DSAPUBDISKEY :
		case IDX_DSAPUBREVKEY :
		case IDX_DSAPUBEXPKEY :
		case IDX_DSASECKEY :
		case IDX_DSASECDISKEY :
		case IDX_DSASECREVKEY :
		case IDX_DSASECEXPKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, FALSE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainElse);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_CERT :
		case IDX_REVCERT :
		case IDX_BADCERT :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainCert);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			break;
		}
	}

	// only single item selected
	else {
		switch (KMFocusedItemType ()) {
		case IDX_NONE :
			SetMenuAllOrSelected (hMenuPopup, TRUE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainElse);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			break;

		case IDX_RSASECKEY :
		case IDX_DSASECKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			pgpGetKeyBoolean ((PGPKey*)KMFocusedObject (),
										kPGPKeyPropIsAxiomatic, &b);
			if (b) EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuPopup, IDM_DISABLEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_ENABLED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSASECDISKEY :
		case IDX_DSASECDISKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			pgpGetKeyBoolean ((PGPKey*)KMFocusedObject (),
									kPGPKeyPropIsAxiomatic, &b);
			if (b) EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuPopup, IDM_ENABLEKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_ENABLED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSASECEXPKEY :
		case IDX_DSASECEXPKEY :
		case IDX_RSASECREVKEY :
		case IDX_DSASECREVKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSAPUBKEY :
		case IDX_DSAPUBKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSAPUBDISKEY :
		case IDX_DSAPUBDISKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSAPUBEXPKEY :
		case IDX_DSAPUBEXPKEY :
		case IDX_RSAPUBREVKEY :
		case IDX_DSAPUBREVKEY :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainKey);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_RSAUSERID :
		case IDX_DSAUSERID :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			if (KMIsThisTheOnlyUserID (g_hWndTree,
										(PGPUserID*)KMFocusedObject ()))
				EnableMenuItem (hMenuPopup, IDM_DELETEKEY,
									MF_BYCOMMAND | MF_GRAYED);
			else
				EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_ENABLED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, FALSE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainElse);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			if (KMIsThisThePrimaryUserID (g_hWndTree,
								(PGPUserID*)KMFocusedObject ()))
				EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
								MF_BYCOMMAND | MF_GRAYED);
			else
				EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_CERT :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_GRAYED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainCert);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
				kPGPCertPropIsMyCert, &b);
			if (!b) EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuPopup, IDM_REVOKEKEY, uWriteEnabled);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
				kPGPCertPropIsVerified, &b);
			if (!b) EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		case IDX_REVCERT :
		case IDX_BADCERT :
			SetMenuAllOrSelected (hMenuPopup, FALSE);
			EnableMenuItem (hMenuPopup, IDM_COPYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DELETEKEY, uWriteEnabled);

			EnableMenuItem (hMenuPopup, IDM_COLLAPSESEL,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_EXPANDSEL,
				MF_BYCOMMAND | MF_GRAYED);

			hMenuPopup = GetSubMenu (hMenu, 2);
			SetMenuDefaultOrPrimary (hMenuPopup, TRUE);
			SetKeyserverMainMenu (hMenuPopup, hMenuKSMainCert);

			EnableMenuItem (hMenuPopup, IDM_CERTIFYKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_ENABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_DISABLEKEY,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_ADDUSERID,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_REVOKEKEY,
				MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenuPopup, IDM_SETASDEFAULT,
				MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem (hMenuPopup, IDM_EXPORTKEYS,
				MF_BYCOMMAND | MF_GRAYED);
			pgpGetCertBoolean ((PGPCert*)KMFocusedObject (),
				kPGPCertPropIsVerified, &b);
			if (!b) EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_GRAYED);
			else EnableMenuItem (hMenuPopup, IDM_KEYPROPERTIES,
				MF_BYCOMMAND | MF_ENABLED);
			break;

		}
	}
	EnableMenuItem (hMenuPopup, IDM_CREATEKEY, uWriteEnabled);
	EnableMenuItem (hMenuPopup, IDM_IMPORTKEYS, uWriteEnabled);
}
