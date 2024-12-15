/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPhosts.c - PGPnet hosts window message procedure
	

	$Id: PNhosts.c,v 1.55.4.1 1999/06/08 16:32:07 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPnetApp.h"

#include "TreeList.h"
#include "PGPnetHelp.h"
#include "pgpNetIPC.h"

#include "pgpIke.h"

#define IDC_PN_HOSTTREELIST		50001

#define TOP_LIST_OFFSET			7
#define HOR_LIST_OFFSET			7
#define HOR_CONTROL_OFFSET		8
#define BOTTOM_LIST_OFFSET		49
#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_HEIGHT			24
#define BUTTON_WIDTH			64
#define BUTTON_SPACING			8

#define HOSTNAMEWIDTH			204
#define IPADDRESSWIDTH			90
#define SUBNETMASKWIDTH			90
#define AUTHENTICATIONWIDTH		82
#define SAWIDTH					36

#define HOSTNAMECOLUMN			0
#define IPADDRESSCOLUMN			1
#define SUBNETMASKCOLUMN		2
#define AUTHENTICATIONCOLUMN	3
#define SACOLUMN				4

static UINT				g_uBottomListOffset		= BOTTOM_LIST_OFFSET;
static UINT				g_uOnOffBoxHeight		= ONOFFBOX_HEIGHT;
static UINT				g_uOnOffBoxWidth		= ONOFFBOX_WIDTH;
static UINT				g_uButtonHeight			= BUTTON_HEIGHT;
static UINT				g_uButtonWidth			= BUTTON_WIDTH;

static HIMAGELIST		g_hil;
static HTLITEM			g_hTLSelected;

extern HINSTANCE		g_hinst;
extern INT				g_iMinWindowWidth;
extern PGPContextRef	g_context;
extern APPOPTIONSSTRUCT	g_AOS;
extern CHAR				g_szHelpFile[];
extern BOOL				g_bReadOnly;	
extern BOOL				g_bAuthAccess;
extern HWND				g_hwndForeground;	

static DWORD aHostsIds[] = {			// Help IDs
	IDC_PN_HOSTTREELIST,	IDH_PNHOSTS_LIST,
	IDC_PGPNETON,			IDH_PNMAIN_PGPNETENABLE,
	IDC_PGPNETOFF,			IDH_PNMAIN_PGPNETDISABLE,
	IDC_EDIT,				IDH_PNHOSTS_EDIT,
	IDC_REMOVE,				IDH_PNHOSTS_REMOVE,
	IDC_ADD,				IDH_PNHOSTS_ADD,
	IDC_CONNECT,			IDH_PNHOSTS_CONNECT,
	IDC_DISCONNECT,			IDH_PNHOSTS_DISCONNECT,
	0,0
};


//	____________________________________
//
//	initialize the tree control

static VOID
sInitHostTreeList (
		HWND	hwndTree)
{
	HBITMAP			hBmp; 
	HDC				hDC;
	INT				iNumBits;
	TL_COLUMN		tlc;
	CHAR			sz[64];

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps based on 
	// current display capabilities.
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		g_hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_HOST_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES4));
		ImageList_AddMasked (g_hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		g_hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_HOST_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES24));
		ImageList_AddMasked (g_hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	TreeList_SetImageList (hwndTree, g_hil);

	// setup columns
	tlc.mask = TLCF_FMT | TLCF_WIDTH | TLCF_TEXT | 
				TLCF_SUBITEM | TLCF_DATATYPE | TLCF_DATAMAX;
	tlc.pszText = sz;
	tlc.iSubItem	= 0;

	tlc.fmt			= TLCFMT_LEFT;
	tlc.iDataType	= TLC_DATASTRING;
	tlc.cx			= HOSTNAMEWIDTH;
	LoadString (g_hinst, IDS_HOSTNAMEFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hwndTree, HOSTNAMECOLUMN, &tlc);

	tlc.fmt			= TLCFMT_LEFT;
	tlc.iDataType	= TLC_DATASTRING;
	tlc.cx			= IPADDRESSWIDTH;
	LoadString (g_hinst, IDS_IPADDRESSFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hwndTree, IPADDRESSCOLUMN, &tlc);

	tlc.fmt			= TLCFMT_LEFT;
	tlc.iDataType	= TLC_DATASTRING;
	tlc.cx			= SUBNETMASKWIDTH;
	LoadString (g_hinst, IDS_SUBNETMASKFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hwndTree, SUBNETMASKCOLUMN, &tlc);

	tlc.fmt			= TLCFMT_IMAGELIST;
	tlc.iDataType	= TLC_DATALONG;
	tlc.cx			= AUTHENTICATIONWIDTH;
	LoadString (g_hinst, IDS_AUTHENTICATIONFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hwndTree, AUTHENTICATIONCOLUMN, &tlc);

	tlc.fmt			= TLCFMT_IMAGE;
	tlc.iDataType	= TLC_DATALONG;
	tlc.cx			= SAWIDTH;
	LoadString (g_hinst, IDS_SAFIELD, sz, sizeof(sz));
	TreeList_InsertColumn (hwndTree, SACOLUMN, &tlc);
}


//	____________________________________
//
//	size the list control and move the buttons

static VOID
sSizeControls (
		HWND	hwnd,
		WORD	wXsize,
		WORD	wYsize)
{
	HDWP	hdwp; 

	// size the tree control accordingly
	hdwp = BeginDeferWindowPos (9); 
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_PN_HOSTTREELIST), NULL, 
		HOR_LIST_OFFSET, TOP_LIST_OFFSET, 
		wXsize - (2*HOR_LIST_OFFSET), 
		wYsize - g_uBottomListOffset - TOP_LIST_OFFSET,
		SWP_NOZORDER);  

	// move the "On/Off" box
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_ONOFFBOX), NULL, 
		HOR_CONTROL_OFFSET, 
		wYsize - g_uBottomListOffset + TOP_ONOFFBOX_OFFSET,
		g_uOnOffBoxWidth, g_uOnOffBoxHeight, 
		SWP_NOZORDER); 

	// move the "On" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_PGPNETON), NULL, 
		HOR_CONTROL_OFFSET + (g_uOnOffBoxWidth/5), 
		wYsize - g_uBottomListOffset + 
					TOP_ONOFFBOX_OFFSET + (g_uOnOffBoxHeight/3),
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "Off" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_PGPNETOFF), NULL, 
		HOR_CONTROL_OFFSET + 3*(g_uOnOffBoxWidth/5), 
		wYsize - g_uBottomListOffset + 
					TOP_ONOFFBOX_OFFSET + (g_uOnOffBoxHeight/3),
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "connect" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_CONNECT), NULL, 
		wXsize - HOR_CONTROL_OFFSET - g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		g_uButtonWidth, g_uButtonHeight, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "disconnect" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_DISCONNECT), NULL, 
		wXsize - HOR_CONTROL_OFFSET - g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		g_uButtonWidth, g_uButtonHeight, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "add" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_ADD), NULL, 
		wXsize - HOR_CONTROL_OFFSET - BUTTON_SPACING - 2*g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		g_uButtonWidth, g_uButtonHeight, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "remove" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_REMOVE), NULL, 
		wXsize - HOR_CONTROL_OFFSET - 2*BUTTON_SPACING - 3*g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		g_uButtonWidth, g_uButtonHeight, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "edit" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_EDIT), NULL, 
		wXsize - HOR_CONTROL_OFFSET - 3*BUTTON_SPACING - 4*g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		g_uButtonWidth, g_uButtonHeight, 
		SWP_NOZORDER|SWP_NOSIZE); 

	EndDeferWindowPos (hdwp);
}


//	____________________________________
//
//	find the specified item

static HTLITEM
sFindTreeListItemByParam (
		HWND		hwndTree,
		LPARAM		lParam)
{
	HTLITEM			hItem;
	TL_TREEITEM		tlI;
	
	// first get the index of the first host
	hItem = TreeList_GetFirstItem (hwndTree);

	while (hItem) 
	{
		tlI.hItem = hItem;
		tlI.mask = TLIF_PARAM | TLIF_NEXTHANDLE;
		TreeList_GetItem (hwndTree, &tlI);

		if (tlI.lParam == lParam)
			return hItem;
		else
			hItem = tlI.hItem;
	}

	return NULL;
}


//	____________________________________
//
//	determine if disconnect button should be shown for selected host

static BOOL
sIsHostEntryDisconnectable (
		HWND				hwnd,
		PAPPOPTIONSSTRUCT	paos)
{
	TL_LISTITEM		tlI;

	if (g_hTLSelected)
	{
		tlI.hItem = g_hTLSelected;
		tlI.mask = TLIF_DATAVALUE;
		tlI.iSubItem = SACOLUMN;
		TreeList_GetListItem (paos->hwndHostTreeList, &tlI);

		if (tlI.lDataValue == IDX_EXISTINGSA) 
			return TRUE;
	}

	return FALSE;
}


//	____________________________________
//
//	determine if connect button should be enabled for selected host

static BOOL
sIsHostEntryConnectable (
		HWND				hwnd,
		PAPPOPTIONSSTRUCT	paos)
{
	TL_TREEITEM		tlI;

	if (g_hTLSelected)
	{
		tlI.hItem = g_hTLSelected;
		tlI.mask = TLIF_PARAM | TLIF_IMAGE;
		TreeList_GetItem (paos->hwndHostTreeList, &tlI);

		if (paos->pnconfig.pHostList[tlI.lParam].childOf == -1)
		{
			// entry is not behind gateway
			switch (tlI.iImage) {
				case IDX_GATEWAY :
				case IDX_SECUREHOST :
					return TRUE;
				default :
					return FALSE;
			}
		}
		else
		{
			// entry is behind gateway
			switch (tlI.iImage) {
				case IDX_SECUREHOST :
				case IDX_INSECUREHOST :
				case IDX_INSECURESUBNET :
					return TRUE;
				default :
					return FALSE;
			}
		}
	}

	return FALSE;
}


//	____________________________________
//
//	load the tree control with configuration

static HTLITEM
sInsertHostIntoTreeList (
		HWND				hwndTree,
		PGPNetHostEntry*	pentry,
		UINT				uIndex)
{
	TL_TREEITEM		tlI;
	TL_INSERTSTRUCT tlIns;
	INT				iImage;
	INT				iParent;

	tlI.hItem			= NULL;
	tlI.mask			= TLIF_TEXT|TLIF_IMAGE|TLIF_PARAM;
	tlI.pszText			= pentry->hostName;
	tlI.lParam			= uIndex;

	// determine icon to display
	switch (pentry->hostType) 
	{
		case kPGPnetInsecureHost :	
			if (pentry->ipMask == 0xFFFFFFFF)
				iImage = IDX_INSECUREHOST;	
			else
				iImage = IDX_INSECURESUBNET;
			break;

		case kPGPnetSecureHost :	
			if (pentry->ipMask == 0xFFFFFFFF)
				iImage = IDX_SECUREHOST;	
			else
				iImage = IDX_SECURESUBNET;
			break;

		case kPGPnetSecureGateway :	
			iImage = IDX_GATEWAY;	
			break;
	}
	tlI.iImage			= iImage;
	tlI.iSelectedImage	= iImage;

	// Insert the data into the tree.
	iParent = pentry->childOf;
	if (iParent < 0)
		tlIns.hParent = NULL;
	else
		tlIns.hParent = 
			sFindTreeListItemByParam (hwndTree, (LPARAM)iParent);

	tlIns.item			= tlI;
	tlIns.hInsertAfter	= TLI_SORT;

	return (TreeList_InsertItem (hwndTree, &tlIns));
}


//	_______________________________________________
//
//  Set treelist list data for item

static VOID 
sAddHostInfoToItem (
		PAPPOPTIONSSTRUCT	paos,
		PGPNetHostEntry*	pentry,
		HTLITEM				hItem)
{
	TL_LISTITEM		tlI;
	CHAR			szText [128];
	struct in_addr	ia;

	tlI.pszText		= szText;
	tlI.hItem		= hItem;
	tlI.stateMask	= TLIS_VISIBLE;

	// IP address
	ia.S_un.S_addr = pentry->ipAddress;
	lstrcpy (szText, inet_ntoa(ia));
	tlI.state		= TLIS_VISIBLE;
	tlI.mask		= TLIF_TEXT | TLIF_STATE;
	tlI.iSubItem	= IPADDRESSCOLUMN;
	TreeList_SetListItem (paos->hwndHostTreeList, &tlI, FALSE);

	// subnet mask
	if (pentry->ipMask != 0xFFFFFFFF)
	{
		ia.S_un.S_addr = pentry->ipMask;
		lstrcpy (szText, inet_ntoa(ia));
	}
	else
		szText[0] = '\0';
	tlI.state		= TLIS_VISIBLE;
	tlI.mask		= TLIF_TEXT | TLIF_STATE;
	tlI.iSubItem	= SUBNETMASKCOLUMN;
	TreeList_SetListItem (paos->hwndHostTreeList, &tlI, FALSE);
}


//	_______________________________________________
//
//  Update treelist list data for item

static VOID 
sUpdateSingleHostItem (
		PAPPOPTIONSSTRUCT	paos,
		PGPNetHostEntry*	pentry,
		HTLITEM				hItem)
{
	TL_LISTITEM		tlI;
	BOOL			bBehindGateway;

	tlI.hItem		= hItem;
	tlI.stateMask	= TLIS_VISIBLE;

	// authentication stuff
	if (g_bAuthAccess)
	{
		tlI.state		= TLIS_VISIBLE;
		tlI.mask		= TLIF_DATAVALUE | TLIF_STATE;
		tlI.iSubItem	= AUTHENTICATIONCOLUMN;
		tlI.lDataValue	= 0;
		if ((pentry->hostType == kPGPnetSecureHost) ||
			(pentry->hostType == kPGPnetSecureGateway))
		{
			if (paos->bPGPKey)
				tlI.lDataValue |= IDXB_PGPCRYPTO;
			if (paos->bX509Key)
				tlI.lDataValue |= IDXB_X509CRYPTO;
			if (pentry->sharedSecret[0])
				tlI.lDataValue |= IDXB_SSCRYPTO;
		}
		TreeList_SetListItem (paos->hwndHostTreeList, &tlI, FALSE);
	}

	// SA
	tlI.mask		= TLIF_DATAVALUE | TLIF_STATE;
	tlI.iSubItem	= SACOLUMN;

	bBehindGateway = (pentry->childOf != -1);
	if (PNDoesHostHaveSA (bBehindGateway, pentry->ipAddress, pentry->ipMask))
	{
		tlI.state		= TLIS_VISIBLE;
		tlI.lDataValue	= IDX_EXISTINGSA;
	}
	else
	{
		tlI.state		= TLIS_VISIBLE;
		tlI.lDataValue	= -1;
	}
	TreeList_SetListItem (paos->hwndHostTreeList, &tlI, FALSE);
}


//	_______________________________________________
//
//  Update treelist list data for item

static VOID 
sUpdateEditedHostItem (
		PAPPOPTIONSSTRUCT	paos,
		PGPNetHostEntry*	pentry,
		HTLITEM				hItem)
{
	TL_TREEITEM		tTI;
	TL_LISTITEM		tlI;
	INT				iImage;

	tTI.hItem		= hItem;
	tlI.hItem		= hItem;
	tlI.stateMask	= TLIS_VISIBLE;

	// if not a gateway but there are children, delete them
	if (pentry->hostType != kPGPnetSecureGateway)
		TreeList_DeleteChildren (paos->hwndHostTreeList, &tTI);

	// determine icon to display
	switch (pentry->hostType) 
	{
		case kPGPnetInsecureHost :	
			if (pentry->ipMask == 0xFFFFFFFF)
				iImage = IDX_INSECUREHOST;	
			else
				iImage = IDX_INSECURESUBNET;
			break;

		case kPGPnetSecureHost :	
			if (pentry->ipMask == 0xFFFFFFFF)
				iImage = IDX_SECUREHOST;	
			else
				iImage = IDX_SECURESUBNET;
			break;

		case kPGPnetSecureGateway :	
			iImage = IDX_GATEWAY;	
			break;
	}
	tTI.mask			= TLIF_TEXT | TLIF_IMAGE;
	tTI.pszText			= pentry->hostName;
	tTI.iImage			= iImage;
	tTI.iSelectedImage	= iImage;
	TreeList_SetItem (paos->hwndHostTreeList, &tTI);
}


//	____________________________________
//
//	update the tree control recursively

static VOID
sUpdateHostItems (
		HWND				hwnd,
		HTLITEM				hItem,
		PAPPOPTIONSSTRUCT	paos)
{
	PGPNetHostEntry*	pentry;
	TL_TREEITEM			tlI;

	tlI.mask = TLIF_PARAM;
	tlI.hItem = hItem;

	while (hItem)
	{
		tlI.mask = TLIF_PARAM | TLIF_CHILDHANDLE;
		TreeList_GetItem (paos->hwndHostTreeList, &tlI);

		pentry = &paos->pnconfig.pHostList[tlI.lParam];
		if (pentry)
			sUpdateSingleHostItem (paos, pentry, hItem);

		// if there are children, update them too
		if (tlI.hItem)
			sUpdateHostItems (hwnd, tlI.hItem, paos);

		tlI.hItem = hItem;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (paos->hwndHostTreeList, &tlI);
		hItem = tlI.hItem;
	}
}


//	____________________________________
//
//	update the authentication and SA columns of the tree control

static VOID
sUpdateHostTreeList (
		HWND				hwnd,
		PAPPOPTIONSSTRUCT	paos)
{
	HTLITEM				hItem;

	hItem = TreeList_GetFirstItem (paos->hwndHostTreeList);

	sUpdateHostItems (hwnd, hItem, paos);

	if (sIsHostEntryDisconnectable (hwnd, paos))
	{
		ShowWindow (GetDlgItem (hwnd, IDC_CONNECT), SW_HIDE);
		ShowWindow (GetDlgItem (hwnd, IDC_DISCONNECT), SW_SHOW);
		EnableWindow (GetDlgItem (hwnd, IDC_DISCONNECT), TRUE);
	}
	else
	{
		ShowWindow (GetDlgItem (hwnd, IDC_DISCONNECT), SW_HIDE);
		ShowWindow (GetDlgItem (hwnd, IDC_CONNECT), SW_SHOW);
		EnableWindow (GetDlgItem (hwnd, IDC_CONNECT), 
				sIsHostEntryConnectable (hwnd, paos));
	}
}


//	____________________________________
//
//	load the tree control with configuration

static VOID
sLoadHostTreeList (
		HWND				hwnd,
		PAPPOPTIONSSTRUCT	paos,
		UINT				uPrevCount)
{
	UINT				u;
	PGPNetHostEntry*	pentry;
	HTLITEM				hItem;

	g_hTLSelected = NULL;

	for (u=uPrevCount; u<paos->pnconfig.uHostCount; u++)
	{
		pentry = &paos->pnconfig.pHostList[u];
		hItem = sInsertHostIntoTreeList (paos->hwndHostTreeList, pentry, u);
		sAddHostInfoToItem (paos, pentry, hItem);
	}

	EnableWindow (GetDlgItem (hwnd, IDC_EDIT), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_CONNECT), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_DISCONNECT), FALSE);
}


//	____________________________________
//
//	send message to service to attempt IKE connection with host

static BOOL
sConnectToHostEntry (
		HWND				hwndParent,
		PAPPOPTIONSSTRUCT	paos)
{
	TL_TREEITEM		tlI;
	UINT			uIndex;
	
	tlI.hItem = g_hTLSelected;
	tlI.mask = TLIF_PARAM;
	TreeList_GetItem (paos->hwndHostTreeList, &tlI);
	uIndex = tlI.lParam;

	if (!PNSendServiceMessage (PGPNET_M_APPMESSAGE,
		(WPARAM)PGPNET_ATTEMPTIKE, 
		(LPARAM)uIndex))
	{
		PNMessageBox (hwndParent, IDS_CAPTION, IDS_SERVICENOTAVAIL, 
				MB_OK|MB_ICONEXCLAMATION);
	}

	return TRUE;
}


//	____________________________________
//
//	remove the selected host entry

static BOOL
sRemoveHostEntry (
		HWND				hwndParent,
		PAPPOPTIONSSTRUCT	paos)
{
	TL_TREEITEM		tlI;
#if 0 // Delete confirmation
	UINT			uIndex;
	CHAR			szText[kMaxNetHostNameLength + 129];
	CHAR			szCaption[32];
#endif
	
	tlI.hItem = g_hTLSelected;

#if 0 // Delete confirmation?
	tlI.mask = TLIF_PARAM;
	TreeList_GetItem (paos->hwndHostTreeList, &tlI);
	uIndex = tlI.lParam;

	LoadString (g_hinst, IDS_REMOVEHOSTCONFIRM, szText, sizeof(szText));
	lstrcat (szText, paos->pnconfig.pHostList[uIndex].hostName);
	lstrcat (szText, " ?");

	LoadString (g_hinst, IDS_CAPTION, szCaption, sizeof(szCaption));

	if (MessageBox (hwndParent, szText, szCaption, 
					MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
#endif
	{
		TreeList_DeleteItem (paos->hwndHostTreeList, &tlI);
		g_hTLSelected = NULL;

		return TRUE;
	}

	return FALSE;
}


//	______________________________________________
//
//  save the host data structure to the prefs file

static INT
sCreateNewHostEntry (
		PPNCONFIG	ppnconfig,
		INT			iIndex)
{
	PGPNetHostEntry		entryNew;
	PGPNetHostEntry*	pNewHostList;
	DWORD				dwAddress;
	CHAR				szAddress[128];

	memset (&entryNew, '\0', sizeof(PGPNetHostEntry));
	entryNew.childOf		= iIndex;

#if !PGPNET_SECURE_HOSTS_BEHIND_GATEWAYS
	if (iIndex >= 0)
		entryNew.hostType		= kPGPnetInsecureHost;
	else
#endif
		entryNew.hostType		= kPGPnetSecureHost;

	entryNew.ipMask				= 0xFFFFFFFF;
	entryNew.identityType		= kPGPike_ID_IPV4_Addr;
	dwAddress					= 0;
	PNGetLocalHostInfo (&dwAddress, szAddress, sizeof(szAddress));
	entryNew.identityIPAddress	= dwAddress;
	entryNew.authKeyAlg			= kPGPPublicKeyAlgorithm_Invalid;

	pNewHostList = 
		PGPNewData (PGPGetContextMemoryMgr (g_context),
				(ppnconfig->uHostCount+1) * sizeof(PGPNetHostEntry),
				kPGPMemoryMgrFlags_Clear);

	if (pNewHostList) 
	{
		if (ppnconfig->uHostCount > 0)
			pgpCopyMemory (ppnconfig->pHostList, pNewHostList,
					ppnconfig->uHostCount * sizeof(PGPNetHostEntry));
		pgpCopyMemory (&entryNew, &pNewHostList[ppnconfig->uHostCount],
				sizeof(PGPNetHostEntry));
		ppnconfig->uHostCount++;

		if (ppnconfig->pHostList)
			PGPFreeData (ppnconfig->pHostList);
		ppnconfig->pHostList = pNewHostList;

		return ppnconfig->uHostCount -1;
	}

	return -1;
}

//	____________________________________
//
//	add a host entry

BOOL
PNAddHostEntry (
		HWND			hwnd,
		PPNCONFIG		ppnconfig,
		BOOL			bGateway,
		INT				iChildOf,
		LPSTR			pszParent)
{
	INT				iNewIndex;
	CHAR			szPrompt[256];
	CHAR			sz[128];
	
	if (ppnconfig->bExpertMode)
	{
		if (iChildOf >= 0)
		{
			if (bGateway)
			{
				LoadString (g_hinst, IDS_ADDHOSTPROMPT, sz, sizeof(sz));
				wsprintf (szPrompt, sz, pszParent);
				LoadString (g_hinst, IDS_CAPTION, sz, sizeof(sz));
				if (MessageBox (hwnd, szPrompt, 
							sz, MB_YESNO|MB_ICONQUESTION) == IDNO)
					iChildOf = -1;
			}
			else
				iChildOf = -1;
		}

		iNewIndex = sCreateNewHostEntry (ppnconfig, iChildOf);
		if (!PNEditHostEntry (hwnd, ppnconfig, iNewIndex, FALSE))
		{
			ppnconfig->uHostCount--;
			return FALSE;
		}
		else 
		{
			if (ppnconfig->pHostList[iNewIndex].sharedSecret[0] == '\0')
			{
				if (!g_AOS.bPGPKey && !g_AOS.bX509Key)
				{
					if (PNMessageBox (hwnd, IDS_CAPTION, IDS_NEEDAUTHKEY,
								MB_ICONINFORMATION|MB_YESNO) == IDYES)
					{
						PNOptionsPropSheet (hwnd, PGPNET_AUTHOPTIONS);
					}
				}
			}
			return TRUE;
		}
	}
	else
		return (PNAddHostWizard (hwnd, ppnconfig, iChildOf));
}


//	____________________________________
//
//	add a host entry

static BOOL
sAddHostEntry (
		HWND				hwnd,
		PAPPOPTIONSSTRUCT	paos)
{
	TL_TREEITEM		tlI;
	CHAR			szParent[256];
	INT				iChildOf;
	BOOL			bGateway;
	
	tlI.lParam = -1;

	// first get the index of the selected host
	if (g_hTLSelected)
	{
		tlI.hItem = g_hTLSelected;
		tlI.pszText = szParent;
		tlI.cchTextMax = sizeof(szParent);
		tlI.mask = TLIF_PARAM | TLIF_TEXT | TLIF_IMAGE;
		TreeList_GetItem (paos->hwndHostTreeList, &tlI);
	}

	iChildOf = tlI.lParam;
	bGateway = (tlI.iImage == IDX_GATEWAY);

	return (PNAddHostEntry (hwnd, 
				&paos->pnconfig, bGateway, iChildOf, szParent));
}


//	_____________________________________________________
//
//  process host list context menu 

static VOID
sHostContextMenu (
		HWND				hwnd, 
		PAPPOPTIONSSTRUCT	paos,
		INT					iX, 
		INT					iY)
{
	HMENU			hMC;
	HMENU			hMenuTrackPopup;

	hMC = LoadMenu (g_hinst, MAKEINTRESOURCE (IDR_MENUHOSTCONTEXT));
	hMenuTrackPopup = GetSubMenu (hMC, 0);

	if (g_hTLSelected) 
	{
		if (!g_bReadOnly)
		{
			EnableMenuItem (hMenuTrackPopup, IDC_EDIT, 
									MF_BYCOMMAND|MF_ENABLED);
			EnableMenuItem (hMenuTrackPopup, IDC_REMOVE, 
									MF_BYCOMMAND|MF_ENABLED);
		}

		if (sIsHostEntryDisconnectable (hwnd, paos))
		{
			EnableMenuItem (hMenuTrackPopup, IDC_DISCONNECT, 
									MF_BYCOMMAND|MF_ENABLED);
		}
		else if (sIsHostEntryConnectable (hwnd, paos))
		{
			EnableMenuItem (hMenuTrackPopup, IDC_CONNECT, 
									MF_BYCOMMAND|MF_ENABLED);
		}
	}

	if (!g_bReadOnly)
	{
		EnableMenuItem (hMenuTrackPopup, IDC_ADD, 
								MF_BYCOMMAND|MF_ENABLED);
	}

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					iX, iY, 0, hwnd, NULL);

	DestroyMenu (hMC);
}


//	_____________________________________________________
//
//  traverse treelist and save contents to host list

static VOID
sTreeListToHostList (
		HWND		hwndTree,
		PPNCONFIG	ppnconfig)
{
	TL_TREEITEM			tlI;
	HTLITEM				hTHost;
	HTLITEM				hTChild;
	INT					iHost, iParent, iHostCount;
	PGPNetHostEntry*	pNewHostList;

	// count number of hosts in list
	iHostCount = 0;
	hTHost = TreeList_GetFirstItem (hwndTree);
	while (hTHost) 
	{
		++iHostCount;

		tlI.hItem = hTHost;
		tlI.mask = TLIF_CHILDHANDLE;
		TreeList_GetItem (hwndTree, &tlI);
		hTChild = tlI.hItem;
		while (hTChild)
		{
			++iHostCount;
			tlI.hItem = hTChild;
			tlI.mask = TLIF_NEXTHANDLE;
			TreeList_GetItem (hwndTree, &tlI);
			hTChild = tlI.hItem;
		}

		tlI.hItem = hTHost;
		tlI.mask = TLIF_NEXTHANDLE;
		TreeList_GetItem (hwndTree, &tlI);
		hTHost = tlI.hItem;
	}

	pNewHostList = PGPNewData (PGPGetContextMemoryMgr (g_context),
				iHostCount * sizeof(PGPNetHostEntry),
				kPGPMemoryMgrFlags_Clear);

	if (pNewHostList) 
	{
		iHost = -1;
		hTHost = TreeList_GetFirstItem (hwndTree);
		while (hTHost) 
		{
			++iHost;

			// get host index
			tlI.hItem = hTHost;
			tlI.mask = TLIF_PARAM | TLIF_CHILDHANDLE;
			TreeList_GetItem (hwndTree, &tlI);
			hTChild = tlI.hItem;

			pgpCopyMemory (&ppnconfig->pHostList[tlI.lParam], 
					&pNewHostList[iHost], 
					sizeof(PGPNetHostEntry));
			pNewHostList[iHost].childOf = -1;

			tlI.hItem = hTHost;
			tlI.mask = TLIF_PARAM;
			tlI.lParam = iHost;
			TreeList_SetItem (hwndTree, &tlI);

			iParent = iHost;
			while (hTChild)
			{
				++iHost;
				tlI.hItem = hTChild;
				tlI.mask = TLIF_PARAM;
				TreeList_GetItem (hwndTree, &tlI);

				pgpCopyMemory (&ppnconfig->pHostList[tlI.lParam], 
						&pNewHostList[iHost], 
						sizeof(PGPNetHostEntry));
				pNewHostList[iHost].childOf = iParent;

				tlI.lParam = iHost;
				TreeList_SetItem (hwndTree, &tlI);

				tlI.mask = TLIF_NEXTHANDLE;
				TreeList_GetItem (hwndTree, &tlI);
				hTChild = tlI.hItem;
			}

			// step to next host
			tlI.hItem = hTHost;
			tlI.mask = TLIF_NEXTHANDLE;
			TreeList_GetItem (hwndTree, &tlI);
			hTHost = tlI.hItem;
		}

		PGPFreeData (ppnconfig->pHostList);
		ppnconfig->pHostList = pNewHostList;
		ppnconfig->uHostCount = iHostCount;
	}
}


//	____________________________________
//
//	dialog proc of hosts dialog

BOOL CALLBACK
PNHostDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PAPPOPTIONSSTRUCT	paos;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		RECT	rc;

		g_hTLSelected = NULL;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		// setup control size values
		GetWindowRect (GetDlgItem (hwnd, IDC_EDIT), &rc);
		g_uButtonWidth = rc.right - rc.left;
		g_uButtonHeight = rc.bottom - rc.top;
		g_uBottomListOffset	= 2 * (rc.bottom - rc.top);

		GetWindowRect (GetDlgItem (hwnd, IDC_ONOFFBOX), &rc);
		g_uOnOffBoxWidth = rc.right - rc.left;
		g_uOnOffBoxHeight = rc.bottom - rc.top;

		GetWindowRect (GetDlgItem (hwnd, IDC_HOSTTREE), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		g_iMinWindowWidth = max (g_iMinWindowWidth,
					(rc.right-rc.left) + (6*HOR_TAB_OFFSET));

		// create host tree list
		paos->hwndHostTreeList = CreateWindowEx (
			WS_EX_CLIENTEDGE, WC_TREELIST, "",
			WS_VISIBLE|WS_CHILD|WS_BORDER|WS_TABSTOP|WS_GROUP|
			TLS_HASBUTTONS|TLS_HASLINES|TLS_AUTOSCROLL|
			TLS_SINGLESELECT|TLS_SHOWSELECTIONALWAYS,
			rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
			hwnd, (HMENU)IDC_PN_HOSTTREELIST, g_hinst, NULL);

		sInitHostTreeList (paos->hwndHostTreeList);
		sLoadHostTreeList (hwnd, paos, 0);
		sUpdateHostTreeList (hwnd, paos);

		// init on/off buttons
		if (g_AOS.pnconfig.bPGPnetEnabled)
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
		else
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);

		// disable "On/Off" and "Add" buttons if we don't have privileges
		if (g_bReadOnly)
		{
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETON), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETOFF), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_ADD), FALSE);
		}

		return FALSE;
	}

	case WM_SIZE :
		sSizeControls (hwnd, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_DESTROY :
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		SendMessage (paos->hwndHostTreeList, WM_CLOSE, 0, 0);
		ImageList_Destroy (g_hil);
		break;

	case PGPNET_M_APPMESSAGE :
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (wParam) {
		case PGPNET_PROMISCUOUSHOSTADDED :
		{
			UINT	uPrevHostCount	= paos->pnconfig.uHostCount;

			PGPnetReloadConfiguration (g_context, &paos->pnconfig);

			sLoadHostTreeList (hwnd, paos, uPrevHostCount);
			sUpdateHostTreeList (hwnd, paos);
			InvalidateRect (hwnd, NULL, FALSE);
			paos->bUpdateHostList = FALSE;
			break;
		}
		}
		break;

	case PGPNET_M_UPDATELIST :
		// update treelist
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (paos->bUpdateHostList)
		{
			sUpdateHostTreeList (hwnd, paos);
			InvalidateRect (hwnd, NULL, FALSE);
			paos->bUpdateHostList = FALSE;
		}
		break;

	case PGPNET_M_ACTIVATEPAGE :
		// update treelist
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (paos->bUpdateHostList)
		{
			sUpdateHostTreeList (hwnd, paos);
			InvalidateRect (hwnd, NULL, FALSE);
			paos->bUpdateHostList = FALSE;
		}

		// nothing is selected
		g_hTLSelected = NULL;
		EnableWindow (GetDlgItem (hwnd, IDC_EDIT), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_CONNECT), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_DISCONNECT), FALSE);

		// init on/off buttons
		if (g_AOS.pnconfig.bPGPnetEnabled)
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
		else
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);
		break;

	case PGPNET_M_SETFOCUS :
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		SetFocus (paos->hwndHostTreeList);
		break;

	case WM_NOTIFY :
		if (wParam == IDC_PN_HOSTTREELIST)
		{
			LPNM_TREELIST pnmtl = (LPNM_TREELIST)lParam;
			paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			switch (pnmtl->hdr.code) {
			case TLN_SELCHANGED :
				g_hTLSelected = pnmtl->itemNew.hItem;
				if (pnmtl->itemNew.hItem)
				{
					EnableWindow (
						GetDlgItem (hwnd, IDC_EDIT), !g_bReadOnly);
					EnableWindow (
						GetDlgItem (hwnd, IDC_REMOVE), !g_bReadOnly);
				}
				else
				{
					EnableWindow (GetDlgItem (hwnd, IDC_EDIT), FALSE);
					EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
				}

				if (sIsHostEntryDisconnectable (hwnd, paos))
				{
					ShowWindow (GetDlgItem (hwnd, IDC_CONNECT), SW_HIDE);
					ShowWindow (GetDlgItem (hwnd, IDC_DISCONNECT), SW_SHOW);
					EnableWindow (GetDlgItem (hwnd, IDC_DISCONNECT), TRUE);
				}
				else
				{
					ShowWindow (GetDlgItem (hwnd, IDC_DISCONNECT), SW_HIDE);
					ShowWindow (GetDlgItem (hwnd, IDC_CONNECT), SW_SHOW);
					EnableWindow (GetDlgItem (hwnd, IDC_CONNECT), 
							sIsHostEntryConnectable (hwnd, paos));
				}
				break;

			case TLN_ITEMDBLCLICKED :
				if (!g_bReadOnly)
					SendMessage (hwnd, WM_COMMAND, IDC_EDIT, 0);
				break;

			case TLN_CONTEXTMENU :
				sHostContextMenu (hwnd, paos,
					((LPNM_TREELIST)lParam)->ptDrag.x,
					((LPNM_TREELIST)lParam)->ptDrag.y);
				break;

			case TLN_KEYDOWN :
				switch (((TL_KEYDOWN*)lParam)->wVKey) {
				case VK_DELETE :
					if (IsWindowEnabled (GetDlgItem (hwnd, IDC_REMOVE)))
						PostMessage (hwnd, WM_COMMAND, IDC_REMOVE, 0);
					break;
				}
				break;
			}
		}
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aHostsIds); 
		return TRUE; 

	case WM_CONTEXTMENU :
		if ((HWND)wParam == GetDlgItem (hwnd, IDC_HOSTTREE))
		{
			paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			sHostContextMenu (hwnd, paos, LOWORD(lParam), HIWORD(lParam));
		}
		else
			WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
					(DWORD) (LPVOID) aHostsIds); 
		break;

	case WM_COMMAND :
		paos = (PAPPOPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {
		case IDC_EDIT :
			if (g_hTLSelected)
			{
				INT			iIndex;
				TL_TREEITEM	tlI;
				BOOL		bHasChildren;

				tlI.hItem = g_hTLSelected;
				tlI.mask = TLIF_PARAM | TLIF_CHILDHANDLE;
				TreeList_GetItem (paos->hwndHostTreeList, &tlI);

				iIndex = tlI.lParam;
				bHasChildren = (tlI.hItem != NULL);

				if (PNEditHostEntry (hwnd, 
							&paos->pnconfig, iIndex, bHasChildren))
				{
					InvalidateRect (hwnd, NULL, FALSE);

					sUpdateEditedHostItem (paos, 
							&paos->pnconfig.pHostList[iIndex], g_hTLSelected);
					sUpdateSingleHostItem (paos, 
							&paos->pnconfig.pHostList[iIndex], g_hTLSelected);
					sAddHostInfoToItem (paos,
							&paos->pnconfig.pHostList[iIndex], g_hTLSelected);
					sTreeListToHostList (
							paos->hwndHostTreeList, &paos->pnconfig);

					PNSaveConfiguration (hwnd, &paos->pnconfig, FALSE);
				}

				SetFocus (paos->hwndHostTreeList);
			}
			break;

		case IDC_REMOVE :
			if (g_hTLSelected)
			{
				if (sRemoveHostEntry (hwnd, paos))
				{
					InvalidateRect (hwnd, NULL, FALSE);

					sTreeListToHostList (
							paos->hwndHostTreeList, &paos->pnconfig);

					PNSaveConfiguration (hwnd, &paos->pnconfig, FALSE);
					SetFocus (GetDlgItem (hwnd, IDC_REMOVE));
				}
				else
					SetFocus (paos->hwndHostTreeList);
			}
			break;

		case IDC_ADD :
		{
			UINT uPrevHostCount = paos->pnconfig.uHostCount;
			if (sAddHostEntry (hwnd, paos))
			{
				sLoadHostTreeList (hwnd, paos, uPrevHostCount);
				sUpdateHostTreeList (hwnd, paos);
				InvalidateRect (hwnd, NULL, FALSE);
				g_hwndForeground = GetParent (hwnd);
				PNSaveConfiguration (hwnd, &paos->pnconfig, FALSE);
			}
			SetFocus (paos->hwndHostTreeList);
			break;
		}

		case IDC_CONNECT :
			if (g_hTLSelected)
				sConnectToHostEntry (hwnd, paos);
			break;

		case IDC_DISCONNECT :
			if (g_hTLSelected)
			{
				TL_TREEITEM	tlI;

				tlI.hItem = g_hTLSelected;
				tlI.mask = TLIF_PARAM;
				TreeList_GetItem (paos->hwndHostTreeList, &tlI);

				if (PNRemoveHostSAs (
						paos->pnconfig.pHostList[tlI.lParam].ipAddress, 
						paos->pnconfig.pHostList[tlI.lParam].ipMask))
				{
					sUpdateHostTreeList (hwnd, paos);
				}
			}
			break;

		case IDC_PGPNETOFF :
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);
			PNSaveOnOffButtons (hwnd, g_context);
			SetFocus (GetDlgItem (hwnd, IDC_PGPNETOFF));
			break;

		case IDC_PGPNETON :
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
			PNSaveOnOffButtons (hwnd, g_context);
			SetFocus (GetDlgItem (hwnd, IDC_PGPNETON));
			break;

		case IDM_HOSTHELP :
			WinHelp (paos->hwndHostTreeList, g_szHelpFile, 
					HELP_WM_HELP, (DWORD) (LPVOID) aHostsIds); 
			break;
		}
		return 0;
	} 

	return FALSE;
}


