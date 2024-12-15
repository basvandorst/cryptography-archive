/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPstatus.c - PGPnet status window message procedure
	

	$Id: PNstatus.c,v 1.49.4.1 1999/06/08 16:32:07 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PGPnetApp.h"
#include "pgpNetIPC.h"
#include "PGPnetHelp.h"

#include "pflPrefTypes.h"
#include "pgpIke.h"

#define TOP_LIST_OFFSET			8
#define HOR_LIST_OFFSET			8
#define HOR_CONTROL_OFFSET		8
#define BOTTOM_LIST_OFFSET		50
#define BOTTOM_BUTTON_OFFSET	10
#define BUTTON_HEIGHT			24
#define BUTTON_WIDTH			64
#define BUTTON_SPACING			8

#define DESTCOLWIDTH			100
#define PROTOCOLWIDTH			55
#define ENCRYPTCOLWIDTH			65
#define AUTHCOLWIDTH			80
#define EXPIRESCOLWIDTH			120
#define REMAININGCOLWIDTH		100

#define COL_DEST				0
#define COL_PROTOCOL			1
#define COL_ENCRYPTION			2
#define COL_AUTHENTICATION		3
#define COL_EXPIRATION			4
#define COL_REMAINING			5

static UINT				g_uBottomListOffset		= BOTTOM_LIST_OFFSET;
static UINT				g_uOnOffBoxHeight		= ONOFFBOX_HEIGHT;
static UINT				g_uOnOffBoxWidth		= ONOFFBOX_WIDTH;
static UINT				g_uButtonHeight			= BUTTON_HEIGHT;
static UINT				g_uButtonWidth			= BUTTON_WIDTH;

extern HINSTANCE		g_hinst;
extern INT				g_iMinWindowWidth;
extern HWND				g_hwndHostDlg;
extern PGPContextRef	g_context;
extern APPOPTIONSSTRUCT	g_AOS;
extern CHAR				g_szHelpFile[];
extern BOOL				g_bReadOnly;	

#define SORT_ASCENDING			1
#define SORT_DESCENDING			-1

typedef struct {
	LONG	lSPI;
	DWORD	dwIP;
	DWORD	dwIPstart;
	DWORD	dwIPmask;
} SALISTSTRUCT, *PSALISTSTRUCT;

static struct {
	INT		iDestinationWidth;
	INT		iProtocolWidth;
	INT		iEncryptionWidth;
	INT		iAuthenticationWidth;
	INT		iExpirationWidth;
	INT		iRemainingWidth;
	INT		iSortField;
	INT		iSortDirection;
} s_ColumnPrefs;

static INT				s_iCurrentIndex		= 0;
static PSALISTSTRUCT	g_pSelectedItem		= NULL;
static HIMAGELIST		hil;

static DWORD aStatusIds[] = {			// Help IDs
	IDC_STATUSLIST,		IDH_PNSTATUS_LIST,
	IDC_PGPNETON,		IDH_PNMAIN_PGPNETENABLE,
	IDC_PGPNETOFF,		IDH_PNMAIN_PGPNETDISABLE,
	IDC_SAVE,			IDH_PNSTATUS_SAVE,
	IDC_REMOVE,			IDH_PNSTATUS_REMOVE,
	0,0
};


//	___________________________________________
//
//	Get widths of columns from prefs file

static VOID 
sGetColumnPreferences (VOID)
{
	PGPError			err;
	PGPPrefRef			prefref;
	PGPSize				size;
	PVOID				pvoid;

	s_ColumnPrefs.iDestinationWidth		= DESTCOLWIDTH;
	s_ColumnPrefs.iProtocolWidth		= PROTOCOLWIDTH;
	s_ColumnPrefs.iEncryptionWidth		= ENCRYPTCOLWIDTH;
	s_ColumnPrefs.iAuthenticationWidth	= AUTHCOLWIDTH;
	s_ColumnPrefs.iExpirationWidth		= EXPIRESCOLWIDTH;
	s_ColumnPrefs.iRemainingWidth		= REMAININGCOLWIDTH;
	s_ColumnPrefs.iSortField			= COL_DEST;
	s_ColumnPrefs.iSortDirection		= SORT_ASCENDING;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (g_context), &prefref);
	if (IsntPGPError (err))
	{
		err = PGPGetPrefData (prefref, 
					kPGPNetPrefAppWinStatusWindowData, &size, &pvoid);

		if (IsntPGPError (err)) 
		{
			if (size == sizeof(s_ColumnPrefs)) 
			{
				memcpy (&s_ColumnPrefs, pvoid, sizeof(s_ColumnPrefs));
			}
			PGPDisposePrefData (prefref, pvoid);
		}
		PGPnetClosePrefs (prefref, FALSE);
	}
}


//	___________________________________________
// 
//	Put window position information in registry

static VOID 
sSetColumnPreferences (
		HWND	hwndList) 
{
	PGPError			err;
	PGPPrefRef			prefref;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (g_context), &prefref);
	if (IsntPGPError (err))
	{
		s_ColumnPrefs.iDestinationWidth = 
					ListView_GetColumnWidth (hwndList, COL_DEST);
		s_ColumnPrefs.iProtocolWidth = 
					ListView_GetColumnWidth (hwndList, COL_PROTOCOL);
		s_ColumnPrefs.iEncryptionWidth = 
					ListView_GetColumnWidth (hwndList, COL_ENCRYPTION);
		s_ColumnPrefs.iAuthenticationWidth = 
					ListView_GetColumnWidth (hwndList, COL_AUTHENTICATION);
		s_ColumnPrefs.iExpirationWidth = 
					ListView_GetColumnWidth (hwndList, COL_EXPIRATION);
		s_ColumnPrefs.iRemainingWidth = 
					ListView_GetColumnWidth (hwndList, COL_REMAINING);

		PGPSetPrefData (prefref, 
					kPGPNetPrefAppWinStatusWindowData, 
					sizeof(s_ColumnPrefs), &s_ColumnPrefs);

		PGPnetClosePrefs (prefref, TRUE);
	}

}

//	____________________________________
//
//	initialize the list control

static VOID
sInitStatusList (
		HWND	hwnd)
{
	HWND			hwndList;
	LV_COLUMN		lvc;
	CHAR			sz[64];
	HBITMAP			hBmp; 
	HDC				hDC;
	INT				iNumBits;

	hwndList = GetDlgItem (hwnd, IDC_STATUSLIST);

	// load appropriate ImageList
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_HOST_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES4));
		ImageList_AddMasked (hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_HOST_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES24));
		ImageList_AddMasked (hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the listview control.
	ListView_SetImageList (hwndList, hil, LVSIL_SMALL);


	// create columns
	sGetColumnPreferences ();

	lvc.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = sz;

	LoadString (g_hinst, IDS_DESTCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iDestinationWidth;
	lvc.iSubItem = COL_DEST;
	ListView_InsertColumn (hwndList, COL_DEST, &lvc);

	LoadString (g_hinst, IDS_PROTOCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iProtocolWidth;
	lvc.iSubItem = COL_PROTOCOL;
	ListView_InsertColumn (hwndList, COL_PROTOCOL, &lvc);

	LoadString (g_hinst, IDS_ENCRYPTCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iEncryptionWidth;
	lvc.iSubItem = COL_ENCRYPTION;
	ListView_InsertColumn (hwndList, COL_ENCRYPTION, &lvc);

	LoadString (g_hinst, IDS_AUTHCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iAuthenticationWidth;
	lvc.iSubItem = COL_AUTHENTICATION;
	ListView_InsertColumn (hwndList, COL_AUTHENTICATION, &lvc);

	LoadString (g_hinst, IDS_EXPIRESCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iExpirationWidth;
	lvc.iSubItem = COL_EXPIRATION;
	ListView_InsertColumn (hwndList, COL_EXPIRATION, &lvc);

	LoadString (g_hinst, IDS_REMAININGCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = s_ColumnPrefs.iRemainingWidth;
	lvc.iSubItem = COL_REMAINING;
	ListView_InsertColumn (hwndList, COL_REMAINING, &lvc);
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
	hdwp = BeginDeferWindowPos (6); 
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_STATUSLIST), NULL, 
		HOR_LIST_OFFSET, TOP_LIST_OFFSET, 
		wXsize - (2*HOR_LIST_OFFSET), 
		wYsize - g_uBottomListOffset - TOP_LIST_OFFSET -1,
		SWP_NOZORDER);  
	
	// move the "On/Off" box
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_ONOFFBOX), NULL, 
		HOR_LIST_OFFSET, 
		wYsize - BOTTOM_ONOFF_OFFSET - g_uOnOffBoxHeight, 
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

	// move the "remove" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_REMOVE), NULL, 
		wXsize - HOR_LIST_OFFSET - g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	// move the "save" button
	DeferWindowPos (hdwp, 
		GetDlgItem (hwnd, IDC_SAVE), NULL, 
		wXsize - HOR_LIST_OFFSET - BUTTON_SPACING - 2*g_uButtonWidth, 
		wYsize - BOTTOM_BUTTON_OFFSET - g_uButtonHeight, 
		0, 0, 
		SWP_NOZORDER|SWP_NOSIZE); 

	EndDeferWindowPos (hdwp);
}


//	____________________________________
//
//	find the encryption transform

static INT
sGetEncryptionTransformIndex (
		PGPikeSA*	pSA)
{
	INT	iTransform	= -1;
	INT nTransforms = min (pSA->numTransforms, kPGPike_MaxTransforms);
	INT	i;
	
	for (i=0; i<nTransforms; i++)
	{
		if (pSA->transform[i].u.ipsec.protocol == kPGPike_PR_ESP)
		{
			iTransform = i;
			break;
		}
	}

	return iTransform;
}


//	____________________________________
//
//	find the compression transform

static INT
sGetCompressionTransformIndex (
		PGPikeSA*	pSA)
{
	INT	iTransform	= -1;
	INT nTransforms = min (pSA->numTransforms, kPGPike_MaxTransforms);
	INT	i;
	
	for (i=0; i<nTransforms; i++)
	{
		if (pSA->transform[i].u.ipsec.protocol == kPGPike_PR_IPCOMP)
		{
			iTransform = i;
			break;
		}
	}

	return iTransform;
}


//	____________________________________
//
//	find the authentication transform

static INT
sGetAuthenticationTransformIndex (
		PGPikeSA*	pSA)
{
	INT	iTransform	= -1;
	INT nTransforms = min (pSA->numTransforms, kPGPike_MaxTransforms);
	INT	i;
	
	for (i=0; i<nTransforms; i++)
	{
		if (pSA->transform[i].u.ipsec.protocol == kPGPike_PR_AH)
		{
			iTransform = i;
			break;
		}
	}

	return iTransform;
}


//	____________________________________
//
//	get the ListView index based on the SPI

static INT
sGetIndexFromSPI (
		HWND	hwndList,
		LONG	lSPI)
{
	LV_ITEM			lvi;
	PSALISTSTRUCT	psals;

	lvi.mask		= LVIF_PARAM;
	lvi.iItem		= 0;

	while (ListView_GetItem (hwndList, &lvi))
	{
		psals = (PSALISTSTRUCT)lvi.lParam;
		if (psals->lSPI == lSPI)
			return lvi.iItem;
		else
			lvi.iItem++;
	}

	return -1;
}


//	____________________________________
//
//	free the SALISTSTRUCT associated with a particular ListView item

static BOOL
sFreeSAListStruct (
		HWND	hwndList,
		INT		iIndex)
{
	LV_ITEM		lvi;

	lvi.mask		= LVIF_PARAM;
	lvi.iItem		= iIndex;

	if (ListView_GetItem (hwndList, &lvi))
	{
		free ((VOID*)lvi.lParam);
		return TRUE;
	}
	else
		return FALSE;
}


//	____________________________________
//
//	free the SALISTSTRUCT associated with a particular ListView item

static VOID
sFreeAllSAListStructs (
		HWND	hwndList)
{
	INT		iIndex		= 0;

	while (sFreeSAListStruct (hwndList, iIndex))
		iIndex++;
}


//	____________________________________
//
//	remove a single SA from the list

static VOID
sSADied (
		HWND		hwndList,
		LPARAM*		pSPI)
{
	INT		iIndex;

	iIndex = sGetIndexFromSPI (hwndList, *pSPI);

	if (iIndex != -1)
	{
		sFreeSAListStruct (hwndList, iIndex);
		ListView_DeleteItem (hwndList, iIndex);
		PNDecStatusSACount ();
	}
}


//	____________________________________
//
//	create string reprentation of remaining data

static VOID
sConvertBytesToString (
		PGPUInt32	uKBytes,
		LPSTR		psz,
		UINT		uLen)
{
	CHAR	szUnits[16];
	UINT	uDivisor;
	UINT	uWhole;
	UINT	uDecimal;
	UINT	uPlaces;

	if (uKBytes < 0x00000400)
	{
		LoadString (g_hinst, IDS_KBYTES, szUnits, sizeof(szUnits));
		uDivisor = 1;
	}
	else if (uKBytes < 0x00100000)
	{
		LoadString (g_hinst, IDS_MBYTES, szUnits, sizeof(szUnits));
		uDivisor = 0x00000400;
	}
	else if (uKBytes < 0x40000000)
	{
		LoadString (g_hinst, IDS_GBYTES, szUnits, sizeof(szUnits));
		uDivisor = 0x00100000;
	}
	else
	{
		LoadString (g_hinst, IDS_TBYTES, szUnits, sizeof(szUnits));
		uDivisor = 0x40000000;
	}


	uWhole = uKBytes / uDivisor;
	if (uWhole >= 100) 
		uPlaces = 0;
	else if (uWhole >= 10)
	{
		uPlaces = 1;
		uDecimal = 10 * (uKBytes % uDivisor) / uDivisor;
	}
	else 
	{
		uPlaces = 2;
		uDecimal = 100 * (uKBytes % uDivisor) / uDivisor;
	}

	if (uPlaces) 
		wsprintf (psz, "%i.%i", uWhole, uDecimal);
	else
		wsprintf (psz, "%i", uWhole);

	lstrcat (psz, szUnits);
}


//	____________________________________
//
//	create string reprentation of remaining data

static VOID
sComputeRemaining (
		PGPUInt32	uTotal,
		PGPUInt32	uRemaining,
		LPSTR		psz,
		UINT		uLen)
{
	CHAR	szTemp[64];

	psz[0] = '\0';

	if (uTotal == 0)
		return;

	if (uLen >= 64)
	{
///		sConvertBytesToString (uRemaining, psz, sizeof(szTemp));
///		lstrcat (psz, " / ");
		sConvertBytesToString (uTotal, szTemp, sizeof(szTemp));
		lstrcat (psz, szTemp);
	}
}


//	____________________________________
//
//	add a single SA to the list

static VOID
sAddSAToList (
		HWND		hwndList,
		PGPikeSA*	pSA)
{
	LV_ITEM					lvi;
	CHAR					sz[256];
	INT						iEncrTrns;
	INT						iAuthTrns;
	INT						iCompTrns;
	INT						ids;
	INT						i;
	struct in_addr			ia;
	PGPipsecEncapsulation	encap;
	PSALISTSTRUCT			psals;


	iEncrTrns = sGetEncryptionTransformIndex (pSA);
	iAuthTrns = sGetAuthenticationTransformIndex (pSA);
	iCompTrns = sGetCompressionTransformIndex (pSA);

	// we should always have an authentication or encryption transform
	pgpAssert ((iAuthTrns >= 0) || (iEncrTrns >= 0)); 

	if ((iAuthTrns < 0) && (iEncrTrns < 0))
		return;

	// prepare to insert SA into list
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE; 
	lvi.iItem = s_iCurrentIndex;
	lvi.iSubItem = 0;

	// get text for SA (ip address)
	ia.S_un.S_addr = pSA->ipAddress;
	lstrcpyn (sz, inet_ntoa (ia), sizeof(sz));
	lvi.pszText = sz;

	// determine icon
	if (iEncrTrns >= 0) 
		encap = pSA->transform[iEncrTrns].u.ipsec.u.esp.t.mode;
	else
		encap = pSA->transform[iAuthTrns].u.ipsec.u.ah.t.mode;
	if (encap == kPGPike_PM_Tunnel)
		lvi.iImage = IDX_GATEWAY;
	else
		lvi.iImage = IDX_SECUREHOST;


	psals = (PSALISTSTRUCT)malloc (sizeof (SALISTSTRUCT));
	if (!psals)
		return;

	// store IP addresses
	psals->dwIP			= pSA->ipAddress;
	psals->dwIPstart	= pSA->ipAddrStart;
	psals->dwIPmask		= pSA->ipMaskEnd;

	// use inSPI of first transform as index
	memcpy (&(psals->lSPI), 
		&(pSA->transform[0].u.ipsec.inSPI), sizeof(DWORD));

	lvi.lParam = (LPARAM)psals;

	i = ListView_InsertItem (hwndList, &lvi);
	if (i != -1)
	{
		PNIncStatusSACount ();

		lvi.iItem = i;
		s_iCurrentIndex = i +1;

		// the rest are just strings
		lvi.mask = LVIF_TEXT;

		// set protocol text string
		sz[0] = '\0';
		if (iEncrTrns >= 0)
			PNCommaCat (sz, IDS_ESP);
		if (iAuthTrns >= 0)
			PNCommaCat (sz, IDS_AH);
		if (iCompTrns >= 0)
			PNCommaCat (sz, IDS_IPCOMP);

		lvi.iSubItem = COL_PROTOCOL;
		ListView_SetItem (hwndList, &lvi);

		// set encryption string
		if (iEncrTrns >= 0)
		{
			switch (pSA->transform[iEncrTrns].u.ipsec.u.esp.t.cipher) {
				case kPGPike_ET_DES_IV64 :	ids = IDS_DES64;	break;
				case kPGPike_ET_DES :		ids = IDS_DES;		break;
				case kPGPike_ET_3DES :		ids = IDS_3DES;		break;
				case kPGPike_ET_RC5 :		ids = IDS_RC5;		break;
				case kPGPike_ET_IDEA :		ids = IDS_IDEA;		break;
				case kPGPike_ET_CAST :		ids = IDS_CAST;		break;
				case kPGPike_ET_Blowfish :	ids = IDS_BLOWFISH;	break;
				case kPGPike_ET_3IDEA :		ids = IDS_3IDEA;	break;
				case kPGPike_ET_DES_IV32 :	ids = IDS_DES32;	break;
				case kPGPike_ET_RC4 :		ids = IDS_RC4;		break;
				case kPGPike_ET_NULL :		ids = IDS_NULL;		break;
				default :					ids = IDS_UNKNOWN;	break;
			}
		}
		else
			ids = IDS_NA;
		lvi.iSubItem = COL_ENCRYPTION;
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		// set authentication string
		sz[0] = '\0';
		if (iEncrTrns >= 0)
		{
			switch (pSA->transform[iEncrTrns].u.ipsec.u.esp.t.authAttr) {
				case kPGPike_AA_HMAC_MD5 :	ids = IDS_HMAC_MD5;		break;
				case kPGPike_AA_HMAC_SHA :	ids = IDS_HMAC_SHA;		break;
				case kPGPike_AA_DES_MAC :	ids = IDS_DES_MAC;		break;
				case kPGPike_AA_KPDK :		ids = IDS_KPDK;			break;
				default :					ids = IDS_UNKNOWN;		break;
			}
			PNCommaCat (sz, ids);
		}
		if (iAuthTrns >= 0)
		{
			switch (pSA->transform[iAuthTrns].u.ipsec.u.ah.t.authAttr) {
				case kPGPike_AA_HMAC_MD5 :	ids = IDS_HMAC_MD5;		break;
				case kPGPike_AA_HMAC_SHA :	ids = IDS_HMAC_SHA;		break;
				case kPGPike_AA_DES_MAC :	ids = IDS_DES_MAC;		break;
				case kPGPike_AA_KPDK :		ids = IDS_KPDK;			break;
				default :					ids = IDS_UNKNOWN;		break;
			}
			PNCommaCat (sz, ids);
		}
		if (sz[0] == '\0')
			LoadString (g_hinst, IDS_NA, sz, sizeof(sz));
		lvi.iSubItem = COL_AUTHENTICATION;
		ListView_SetItem (hwndList, &lvi);

		// set expiration string
		if (pSA->secLifeTime == 0) 
		{
			LoadString (g_hinst, IDS_NEVER, sz, sizeof(sz));
		}
		else
		{
			PGPTime	timeExpires = pSA->birthTime + pSA->secLifeTime;
			PNConvertTimeToString (timeExpires, sz, sizeof(sz));
		}
		lvi.iSubItem = COL_EXPIRATION;
		ListView_SetItem (hwndList, &lvi);

		// set remaining string
		sComputeRemaining (pSA->kbLifeTime, pSA->kbLifeTime, sz, sizeof(sz));
		lvi.iSubItem = COL_REMAINING;
		ListView_SetItem (hwndList, &lvi);
	}
}


//	____________________________________
//
//	compare entries for sorting purposes

static INT CALLBACK 
sCompareFunction (
			  LPARAM lParam1, 
			  LPARAM lParam2, 	
			  LPARAM lParamSort)
{
	HWND			hwndList = (HWND)lParamSort;
	LV_FINDINFO		lvfi;
	LV_ITEM			lvi;
	INT				index1, index2;
	CHAR			sz1[32], sz2[32];

	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam1;
	index1 = ListView_FindItem (hwndList, -1, &lvfi);
	lvfi.lParam = lParam2;
	index2 = ListView_FindItem (hwndList, -1, &lvfi);

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = s_ColumnPrefs.iSortField;

	lvi.iItem = index1;
	lvi.pszText = sz1;
	lvi.cchTextMax = sizeof(sz1);
	ListView_GetItem (hwndList, &lvi);

	lvi.iItem = index2;
	lvi.pszText = sz2;
	lvi.cchTextMax = sizeof(sz2);
	ListView_GetItem (hwndList, &lvi);

	return (s_ColumnPrefs.iSortDirection * lstrcmp (sz1, sz2));
}

//	____________________________________
//
//	ask user if OK to delete SA, then ask service to delete it

static VOID
sSendRemoveSAMessage (
		HWND	hwnd)
{
	INT				i;
	LV_FINDINFO		lvfi;

	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)g_pSelectedItem;
	i = ListView_FindItem (GetDlgItem (hwnd, IDC_STATUSLIST), -1, &lvfi);

	if (i != -1)
	{
		COPYDATASTRUCT	cds;
		PGPipsecSPI		spi;

		memcpy (spi, &g_pSelectedItem->lSPI, sizeof(PGPipsecSPI));

		cds.dwData = PGPNET_STATUSREMOVESA;
		cds.cbData = sizeof(PGPipsecSPI);
		cds.lpData = spi;

		if (!PNSendServiceMessage (WM_COPYDATA,
				(WPARAM)hwnd, (LPARAM)&cds))
		{
			PNMessageBox (hwnd, IDS_CAPTION, IDS_SERVICENOTAVAIL,
							MB_OK|MB_ICONSTOP);
		}
		else
		{
			// now remove the SA from the list
			sFreeSAListStruct (GetDlgItem (hwnd, IDC_STATUSLIST), i);
			ListView_DeleteItem (GetDlgItem (hwnd, IDC_STATUSLIST), i);
			PNDecStatusSACount ();
		}
	}
}


//	____________________________________
//
//	process status message from service

static VOID
sProcessStatusMessage (
		HWND			hwnd,
		PCOPYDATASTRUCT	pcds)
{
	HWND	hwndList;

	hwndList = GetDlgItem (hwnd, IDC_STATUSLIST);

	switch (pcds->dwData) {
	case PGPNET_STATUSFULL :
	{
		INT			iNumSAs;
		INT			i;
		PGPikeSA*	pSAs;

		sFreeAllSAListStructs (hwndList);
		ListView_DeleteAllItems (hwndList);
		PNSetStatusSACount (0);

		iNumSAs = pcds->cbData / sizeof(PGPikeSA);
		pSAs = (PGPikeSA*)(pcds->lpData);

		for (i=0; i<iNumSAs; i++)
		{
			sAddSAToList (hwndList, pSAs);
			pSAs++;
		}

		ListView_SortItems (hwndList, sCompareFunction, hwndList);
		InvalidateRect (hwndList, NULL, FALSE);

		EnableWindow (GetDlgItem (hwnd, IDC_SAVE), TRUE);
		break;
	}

	case PGPNET_STATUSNEWSA :
		sAddSAToList (hwndList, (PGPikeSA*)(pcds->lpData));
		ListView_SortItems (hwndList, sCompareFunction, hwndList);
		InvalidateRect (hwndList, NULL, FALSE);

		EnableWindow (GetDlgItem (hwnd, IDC_SAVE), TRUE);
		break;

	case PGPNET_STATUSREMOVESA :
		sSADied (hwndList, (LPARAM*)(pcds->lpData));
		break;
	}

	g_AOS.bUpdateHostList = TRUE;
}


//	____________________________________
//
//	kill all SAs related to a host

BOOL
PNRemoveHostSAs (
		DWORD	dwHostAddress,
		DWORD	dwHostMask)
{
	BOOL			bHostRemoved	= FALSE;

	LV_ITEM			lvi;
	PSALISTSTRUCT	psals;
	HWND			hwndList;

	lvi.mask		= LVIF_PARAM;
	lvi.iItem		= 0;

	hwndList = GetDlgItem (g_AOS.hwndStatusDlg, IDC_STATUSLIST);

	while (ListView_GetItem (hwndList, &lvi))
	{
		psals = (PSALISTSTRUCT)lvi.lParam;

		if ((psals->dwIP == dwHostAddress) ||
			((psals->dwIP & dwHostMask) == (dwHostAddress & dwHostMask)))
		{
			COPYDATASTRUCT	cds;
			PGPipsecSPI		spi;

			memcpy (spi, &psals->lSPI, sizeof(PGPipsecSPI));

			cds.dwData = PGPNET_STATUSREMOVESA;
			cds.cbData = sizeof(PGPipsecSPI);
			cds.lpData = spi;

			if (!PNSendServiceMessage (WM_COPYDATA,
					(WPARAM)g_AOS.hwndStatusDlg, (LPARAM)&cds))
			{
				PNMessageBox (g_AOS.hwndStatusDlg, IDS_CAPTION, 
						IDS_SERVICENOTAVAIL, MB_OK|MB_ICONSTOP);
				lvi.iItem++;
			}
			else
			{
				// now remove the SA from the list
				sFreeSAListStruct (hwndList, lvi.iItem);
				ListView_DeleteItem (hwndList, lvi.iItem);
				PNDecStatusSACount ();

				bHostRemoved = TRUE;
			}
		}
		else
			lvi.iItem++;
	}

	return bHostRemoved;
}


//	____________________________________
//
//	determine if specified host has an SA in list

BOOL
PNDoesHostHaveSA (
		BOOL	bBehindGateway,
		DWORD	dwHostAddress,
		DWORD	dwHostMask)
{
	LV_ITEM			lvi;
	PSALISTSTRUCT	psals;
	HWND			hwndList;

	lvi.mask		= LVIF_PARAM;
	lvi.iItem		= 0;

	hwndList = GetDlgItem (g_AOS.hwndStatusDlg, IDC_STATUSLIST);

	while (ListView_GetItem (hwndList, &lvi))
	{
		psals = (PSALISTSTRUCT)lvi.lParam;
		if (psals->dwIP == dwHostAddress)
			return TRUE;
		if ((psals->dwIP & dwHostMask) == (dwHostAddress & dwHostMask))
			return TRUE;
		if (bBehindGateway)
		{
			if ((psals->dwIPstart == dwHostAddress) &&
				(psals->dwIPmask == dwHostMask))
				return TRUE;
		}

		lvi.iItem++;
	}

	return FALSE;
}


//	____________________________________
//
//	dialog proc of status dialog

BOOL CALLBACK
PNStatusDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		RECT	rc;

		sInitStatusList (hwnd);

		// setup control size values
		GetWindowRect (GetDlgItem (hwnd, IDC_SAVE), &rc);
		g_uButtonWidth = rc.right - rc.left;
		g_uButtonHeight = rc.bottom - rc.top;
		g_uBottomListOffset	= 2 * (rc.bottom - rc.top);

		GetWindowRect (GetDlgItem (hwnd, IDC_ONOFFBOX), &rc);
		g_uOnOffBoxWidth = rc.right - rc.left;
		g_uOnOffBoxHeight = rc.bottom - rc.top;

		GetWindowRect (GetDlgItem (hwnd, IDC_STATUSLIST), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		g_iMinWindowWidth = max (g_iMinWindowWidth,
					(rc.right-rc.left) + (6*HOR_TAB_OFFSET));

		// disable "On/Off" buttons if we don't have privileges
		if (g_bReadOnly)
		{
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETON), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_PGPNETOFF), FALSE);
		}

		PostMessage (hwnd, WM_APP, 0, 0);
		return FALSE;
	}

	case WM_APP :
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
						(WPARAM)PGPNET_ENABLESTATUSMESSAGES, (LPARAM)hwnd);
		break;

	case PGPNET_M_ACTIVATEPAGE :
		// init on/off buttons
		if (g_AOS.pnconfig.bPGPnetEnabled)
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETON);
		else
			CheckRadioButton (hwnd, 
						IDC_PGPNETON, IDC_PGPNETOFF, IDC_PGPNETOFF);
		break;

	case PGPNET_M_SETFOCUS :
		SetFocus (GetDlgItem (hwnd, IDC_STATUSLIST));
		break;

	case PGPNET_M_CLEARSALIST :
		sFreeAllSAListStructs (GetDlgItem (hwnd, IDC_STATUSLIST));
		ListView_DeleteAllItems (GetDlgItem (hwnd, IDC_STATUSLIST));
		g_AOS.bUpdateHostList = TRUE;
		PostMessage (g_hwndHostDlg, PGPNET_M_UPDATELIST, 0, 0);
		break;

	case WM_SIZE :
		sSizeControls (hwnd, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_DESTROY :
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
						(WPARAM)PGPNET_DISABLESTATUSMESSAGES, (LPARAM)hwnd);
		sSetColumnPreferences (GetDlgItem (hwnd, IDC_STATUSLIST));
		sFreeAllSAListStructs (GetDlgItem (hwnd, IDC_STATUSLIST));
		ImageList_Destroy (hil);
		break;

	case PGPNET_M_INTERNALCOPYDATA :
		sProcessStatusMessage (hwnd, (PCOPYDATASTRUCT)lParam);
		break;

	case WM_NOTIFY :
		if (wParam == IDC_STATUSLIST)
		{
			NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)lParam;

			switch (pnmlv->hdr.code) {
			case LVN_COLUMNCLICK:
				if (s_ColumnPrefs.iSortField == pnmlv->iSubItem)
				{
					s_ColumnPrefs.iSortDirection *= -1;
				}
				else 
				{
					s_ColumnPrefs.iSortField = pnmlv->iSubItem;
					s_ColumnPrefs.iSortDirection = SORT_ASCENDING;
				}
				ListView_SortItems (GetDlgItem (hwnd, IDC_STATUSLIST),
						sCompareFunction, GetDlgItem (hwnd, IDC_STATUSLIST));
				InvalidateRect (
						GetDlgItem (hwnd, IDC_STATUSLIST), NULL, FALSE);
				break;

			case LVN_ITEMCHANGED :
				if (pnmlv->iItem >= 0)
				{
					if (pnmlv->uNewState & LVIS_SELECTED)
					{
						if (!g_bReadOnly)
						{
							g_pSelectedItem = (PSALISTSTRUCT)pnmlv->lParam;
							EnableWindow (
								GetDlgItem (hwnd, IDC_REMOVE), TRUE);
						}
					}
					else
					{
						g_pSelectedItem = NULL;
						EnableWindow (
							GetDlgItem (hwnd, IDC_REMOVE), FALSE);
					}
				}	
				break;

			case LVN_KEYDOWN :
				if (((LV_KEYDOWN*)lParam)->wVKey == VK_DELETE)
				{
					if (g_pSelectedItem)
						PostMessage (hwnd, WM_COMMAND, IDC_REMOVE, 0);
				}
				break;
			}
		}
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aStatusIds); 
		return TRUE; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aStatusIds); 
		return TRUE; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDC_SAVE :
			if (IsWindowEnabled (GetDlgItem (hwnd, IDC_SAVE)))
			{
				PNSaveStatusToFile (hwnd);
				SetFocus (GetDlgItem (hwnd, IDC_SAVE));
			}
			break;

		case IDC_REMOVE :
			if (IsWindowEnabled (GetDlgItem (hwnd, IDC_REMOVE)))
			{
				sSendRemoveSAMessage (hwnd);
				SetFocus (GetDlgItem (hwnd, IDC_REMOVE));
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
		}
		return 0;
	}

	return FALSE;
}


