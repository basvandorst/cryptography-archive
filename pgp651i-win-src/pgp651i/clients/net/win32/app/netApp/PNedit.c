/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNedit.c - PGPnet edit host/gateway entry dialog
	

	$Id: PNedit.c,v 1.43.4.2.2.1 1999/06/29 18:46:09 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>

#include "resource.h"
#include "PGPnetApp.h"
#include "IPAddressControl.h"
#include "PGPnetHelp.h"
#include "pgpClientLib.h"
#include "pgpBuildFlags.h"

#include "PGPcl.h"
#include "pgpIke.h"

#define	INSECURE_HOST		0
#define INSECURE_SUBNET		1
#define SECURE_HOST			2
#define	SECURE_SUBNET		3
#define SECURE_GATEWAY		4

#define BADIDENTITY			-2
#define NOIDENTITY			-1
#define IPADDRESS			0
#define	DOMAINNAME			1
#define	USERDOMAINNAME		2
#define DISTINGUISHEDNAME	3

#define	LOOKUP_TIMER_ID		111L
#define	LOOKUP_TIMER_MS		100L
#define	NUMLEDS				10
#define	LEDSPACING			2

extern HINSTANCE		g_hinst;
extern PGPContextRef	g_context;
extern CHAR				g_szHelpFile[];
extern APPOPTIONSSTRUCT	g_AOS;

static BOOL				g_bAskingUserForKey		= FALSE;

typedef struct {
	HWND				hwndIPAddr;
	HWND				hwndIPSubnet;
	HWND				hwndIPID;
	PPNCONFIG			ppnconfig;
	INT					iIndex;
	BOOL				bIsChild;
	BOOL				bHasChildren;
	PGPNetHostEntry		host;
	DWORD				dwAddress;
	INT					iPrevIDIndex;
	CHAR				szIdentity [kMaxNetHostIdentityLength +1];
	HIMAGELIST			hil;
} HOSTEDITSTRUCT, *PHOSTEDITSTRUCT;

typedef struct {
	CHAR		szHostName[256];
	DWORD		dwAddress;
	HWND		hwndAddress;
	HWND		hwndThreadParent;
	INT			iStatusValue;
	INT			iStatusDirection;
	HPALETTE	hPalette;
} DNSLOOKUPSTRUCT, *PDNSLOOKUPSTRUCT;

static DWORD aEditIds[] = {			// Help IDs
    IDC_DOMAINNAME,		IDH_PNEDITHOST_NAME, 
	IDC_IPADDRESS,		IDH_PNEDITHOST_IPADDRESS,
	IDC_DNSLOOKUP,		IDH_PNEDITHOST_DNSLOOKUP,
	IDC_TYPE,			IDH_PNEDITHOST_HOSTTYPE,
	IDC_IPMASK,			IDH_PNEDITHOST_SUBNETMASK,
	IDC_PASSPHRASE,		IDH_PNEDITHOST_SETCLEARPASSPHRASE,
	IDC_IDENTITYTYPE,	IDH_PNEDITHOST_IDENTITYTYPE,
	IDC_IDENTITY,		IDH_PNEDITHOST_IDENTITY,
	IDC_IPID,			IDH_PNEDITHOST_IDENTITY,
	IDC_ANYVALIDKEY,	IDH_PNEDITHOST_ANYVALIDKEY,
	IDC_PGPAUTHKEY,		IDH_PNEDITHOST_PGPAUTHKEY,
	IDC_X509CERT,		IDH_PNEDITHOST_X509AUTHCERT,
	IDC_AUTHEDIT,		IDH_PNEDITHOST_AUTHKEY,
	0,0
};

static DWORD aDNSIds[] = {			// Help IDs
    IDC_HOSTNAME,		IDH_PNDNSLOOKUP_HOST, 
	IDC_LOOKUP,			IDH_PNDNSLOOKUP_LOOKUP,
	IDC_IPADDRESS,		IDH_PNDNSLOOKUP_ADDRESS,
	IDOK,				IDH_PNDNSLOOKUP_USEADDRESS,
	0,0
};


//	____________________________________
//
//  Determine if the specified host is already in the host list

BOOL
PNIsSubnetMaskValid (
		PGPUInt32	uSubnetMask)
{
	PGPUInt32	uBitMask	= 0x80000000;
	PGPUInt32	uMask		= 0x00;
	INT			i;

	if (uSubnetMask == 0xFFFFFFFF)
		return FALSE;

	if (uSubnetMask == 0x00000000)
		return FALSE;

	for (i=0; i<4; i++)
	{
		uMask <<= 8;
		uMask |= (uSubnetMask >> (i*8)) & 0xFF;
	}

	while (uBitMask && (uBitMask & uMask))
		uBitMask >>= 1;

	while (uBitMask && !(uBitMask & uMask))
		uBitMask >>= 1;

	if (uBitMask)
		return FALSE;

	return TRUE;
}


//	____________________________________
//
//  Determine if the specified hosts/subnets overlap

BOOL
sOverlap (
		PGPUInt32			uIP,
		PGPUInt32			uSubnetMask,
		PGPNetHostEntry*	pHost)
{
	PGPUInt32	uMask;

	uMask = uSubnetMask;
	if ((uIP & uMask) == (pHost->ipAddress & uMask))
	{
		 return TRUE;
	}

	uMask = pHost->ipMask;
	if ((uIP & uMask) == (pHost->ipAddress & uMask))
	{
		 return TRUE;
	}
	
	return FALSE;
}


//	____________________________________
//
//  Determine if the specified host is already in the host list

BOOL
PNIsHostAlreadyInList (
		HWND				hwnd,
		PGPNetHostType		hosttype,
		PGPUInt32			uIP,
		PGPUInt32			uSubnetMask,
		PGPNetHostEntry*	pHostList,
		PGPUInt32			uHostCount,
		PGPInt32			iIndex)
{
	BOOL		bInList		= FALSE;
	UINT		u;

	UINT		uNewType;
	UINT		uExistingType;

	switch (hosttype) {
		case kPGPnetInsecureHost :
			if (uSubnetMask == 0xFFFFFFFF)
				uNewType = INSECURE_HOST;
			else
				uNewType = INSECURE_SUBNET;
			break;

		case kPGPnetSecureHost :
			if (uSubnetMask == 0xFFFFFFFF)
				uNewType = SECURE_HOST;
			else
				uNewType = SECURE_SUBNET;
			break;

		default :
			uNewType = SECURE_GATEWAY;
			break;
	}

	for (u=0; u<uHostCount; u++)
	{
		if ((INT)u == iIndex)
			continue;

		switch (pHostList[u].hostType) {
			case kPGPnetInsecureHost :
				if (pHostList[u].ipMask == 0xFFFFFFFF)
					uExistingType = INSECURE_HOST;
				else
					uExistingType = INSECURE_SUBNET;
				break;

			case kPGPnetSecureHost :
				if (pHostList[u].ipMask == 0xFFFFFFFF)
					uExistingType = SECURE_HOST;
				else
					uExistingType = SECURE_SUBNET;
				break;

			default :
				uExistingType = SECURE_GATEWAY;
				break;
		}

		switch (uNewType) {
		case INSECURE_HOST :
			switch (uExistingType) {
				case INSECURE_HOST :
				case SECURE_HOST :
				case SECURE_GATEWAY :
					bInList = sOverlap (uIP, uSubnetMask, &pHostList[u]);
					break;
				case INSECURE_SUBNET :
				case SECURE_SUBNET :
					break;
			}
			break;

		case INSECURE_SUBNET :
			switch (uExistingType) {
				case INSECURE_SUBNET :
				case SECURE_SUBNET :
					bInList = sOverlap (uIP, uSubnetMask, &pHostList[u]);
					break;
				case INSECURE_HOST :
				case SECURE_HOST :
				case SECURE_GATEWAY :
					break;
			}
			break;

		case SECURE_HOST :
			switch (uExistingType) {
				case INSECURE_HOST :
				case SECURE_HOST :
				case SECURE_GATEWAY :
					bInList = sOverlap (uIP, uSubnetMask, &pHostList[u]);
					break;
				case INSECURE_SUBNET :
				case SECURE_SUBNET :
					break;
			}
			break;

		case SECURE_SUBNET :
			switch (uExistingType) {
				case INSECURE_SUBNET :
				case SECURE_SUBNET :
					bInList = sOverlap (uIP, uSubnetMask, &pHostList[u]);
					break;
				case INSECURE_HOST :
				case SECURE_HOST :
				case SECURE_GATEWAY :
					break;
			}
			break;

		case SECURE_GATEWAY :
			switch (uExistingType) {
				case INSECURE_HOST :
				case SECURE_HOST :
				case SECURE_GATEWAY :
					bInList = sOverlap (uIP, 0xFFFFFFFF, &pHostList[u]);
					break;
				case INSECURE_SUBNET :
				case SECURE_SUBNET :
					break;
			}
		}

		if (bInList)
		{
			break;
		}
	}

	if (bInList)
	{
		CHAR	szCaption[64];
		CHAR	szFormat[128];
		CHAR	szText[128+kMaxNetHostNameLength];

		if (uSubnetMask == 0xFFFFFFFF)
			LoadString (g_hinst, 
					IDS_ADDRESSUSED, szFormat, sizeof(szFormat));
		else
			LoadString (g_hinst, 
					IDS_ADDRESSSUBNETUSED, szFormat, sizeof(szFormat));

		wsprintf (szText, szFormat, pHostList[u].hostName);
		LoadString (g_hinst, IDS_CAPTION, szCaption, sizeof(szCaption));
		MessageBox (hwnd, szText, szCaption, MB_OK|MB_ICONHAND);
	}

	return bInList;
}


//	____________________________________
//
//  Draw the "LED" progress indicator

static VOID
sDrawSendStatus (
		HWND				hwnd,
		PDNSLOOKUPSTRUCT	pdls) 
{
	HBRUSH			hBrushLit, hBrushUnlit, hBrushOld;
	HPEN			hPen, hPenOld;
	INT				i;
	INT				itop, ibot, ileft, iright, iwidth;
	RECT			rc;
	HDC				hdc;
	PAINTSTRUCT		ps;

	if (pdls->iStatusValue < -1) return;

	hdc = BeginPaint (hwnd, &ps);

	// draw 3D shadow
	GetClientRect (hwnd, &rc);
	itop = rc.top+1;
	ibot = rc.bottom-2;

	iwidth = (rc.right-rc.left) / NUMLEDS;
	iwidth -= LEDSPACING;

	ileft = rc.left + 4;
	for (i=0; i<NUMLEDS; i++) {
		iright = ileft + iwidth;

		MoveToEx (hdc, ileft, ibot, NULL);
		LineTo (hdc, iright, ibot);
		LineTo (hdc, iright, itop);

		ileft += iwidth + LEDSPACING;
	}

	hPen = CreatePen (PS_SOLID, 0, RGB (128, 128, 128));
	hPenOld = SelectObject (hdc, hPen);
	hBrushLit = CreateSolidBrush (RGB (0, 255, 0));
	hBrushUnlit = CreateSolidBrush (RGB (0, 128, 0));

	ileft = rc.left + 4;

	// draw "Knight Rider" LEDs
	if (pdls->iStatusDirection) {
		hBrushOld = SelectObject (hdc, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i == pdls->iStatusValue) {
				SelectObject (hdc, hBrushLit);
				Rectangle (hdc, ileft, itop, iright, ibot);
				SelectObject (hdc, hBrushUnlit);
			}
			else  {
				Rectangle (hdc, ileft, itop, iright, ibot);
			}
	
			ileft += iwidth + LEDSPACING;
		}
	}

	// draw "progress bar" LEDs
	else { 
		if (pdls->iStatusValue >= 0) 
			hBrushOld = SelectObject (hdc, hBrushLit);
		else
			hBrushOld = SelectObject (hdc, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i > pdls->iStatusValue) {
				SelectObject (hdc, hBrushUnlit);
			}
			Rectangle (hdc, ileft, itop, iright, ibot);
	
			ileft += iwidth + LEDSPACING;
		}
	}

	SelectObject (hdc, hBrushOld);
	SelectObject (hdc, hPenOld);
	DeleteObject (hPen);
	DeleteObject (hBrushLit);
	DeleteObject (hBrushUnlit);

	EndPaint (hwnd, &ps);
}


//	______________________________________________
//
//	thread procedure for looking up IP address

static DWORD WINAPI 
sDNSLookupThread (LPVOID pvoid)
{
	PDNSLOOKUPSTRUCT	pdls	= (PDNSLOOKUPSTRUCT)pvoid;

	if (PNGetRemoteHostIP (pdls->szHostName, &pdls->dwAddress))
	{
		if (pdls->hwndThreadParent)
			SendMessage (pdls->hwndThreadParent, WM_APP, TRUE, 0);
	}
	else
	{
		if (pdls->hwndThreadParent)
			SendMessage (pdls->hwndThreadParent, WM_APP, FALSE, 0);
	}

	return 0;
}


//	____________________________________
//
//	dialog proc of DNS lookup dialog

static BOOL CALLBACK
sDNSLookupDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PDNSLOOKUPSTRUCT	pdls;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		RECT			rc;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pdls = (PDNSLOOKUPSTRUCT)lParam;

		// misc initialization
		pdls->hwndThreadParent = NULL;

		// initialize host name
		SetDlgItemText (hwnd, IDC_HOSTNAME, pdls->szHostName);

		// create IP edit control for address
		GetWindowRect (GetDlgItem (hwnd, IDC_IPADDRESS), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pdls->hwndAddress = CreateWindowEx (
				WS_EX_STATICEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_TABSTOP|ES_READONLY,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, (HMENU)IDC_IPADDRESS, g_hinst, NULL);

		// set tab-order of control
		SetWindowPos (pdls->hwndAddress, 
						GetDlgItem (hwnd, IDC_IPADDRESS),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		// disable window
		EnableWindow (pdls->hwndAddress, FALSE);

		return FALSE;
	}

	case WM_APP :
		pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		pdls->hwndThreadParent = NULL;
		ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_HIDE);
		EnableWindow (GetDlgItem (hwnd, IDC_LOOKUP), TRUE);

		KillTimer (hwnd, LOOKUP_TIMER_ID);
		if (wParam)
		{
			ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_HIDE);
			ShowWindow (GetDlgItem (hwnd, IDC_HOSTFOUND), SW_SHOW);
			ShowWindow (pdls->hwndAddress, SW_SHOW);
			SendMessage (pdls->hwndAddress, 
						PGP_IPM_SETADDRESS, 0, (LPARAM)pdls->dwAddress);
			EnableWindow (GetDlgItem (hwnd, IDOK), TRUE);
		}
		else
		{
			ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_HIDE);
			ShowWindow (GetDlgItem (hwnd, IDC_HOSTNOTFOUND), SW_SHOW);
			EnableWindow (GetDlgItem (hwnd, IDOK), FALSE);
		}
		break;

	case WM_TIMER :
		pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pdls->hwndThreadParent)
		{
			ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_SHOW);
			pdls->iStatusValue += pdls->iStatusDirection;
			if (pdls->iStatusValue <= 0) {
				pdls->iStatusValue = 0;
				pdls->iStatusDirection = 1;
			}
			else if (pdls->iStatusValue >= NUMLEDS-1) {
				pdls->iStatusValue = NUMLEDS-1;
				pdls->iStatusDirection = -1;
			}
			InvalidateRect (hwnd, NULL, FALSE);
		}
		else 
		{
			DWORD	dw;

			pdls->iStatusValue = -1;
			pdls->iStatusDirection = 1;

			pdls->hwndThreadParent = hwnd;
			CreateThread (NULL, 0, sDNSLookupThread, (LPVOID)pdls, 0, &dw);
		}
		break;

	case WM_DESTROY :
		pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		DestroyWindow (pdls->hwndAddress);
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pdls->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pdls->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		sDrawSendStatus (GetDlgItem (hwnd, IDC_PROGRESS), pdls);
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aDNSIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aDNSIds); 
		break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDCANCEL :
			pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pdls->hwndThreadParent)
			{
				pdls->hwndThreadParent = NULL;
				SendMessage (hwnd, WM_APP, FALSE, 0);
			}
			else 
				EndDialog (hwnd, 0);
			break;

		case IDOK :
			EndDialog (hwnd, 1);
			break;

		case IDC_LOOKUP :
			pdls = (PDNSLOOKUPSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			EnableWindow (GetDlgItem (hwnd, IDC_LOOKUP), FALSE);
			ShowWindow (pdls->hwndAddress, SW_HIDE);
			ShowWindow (GetDlgItem (hwnd, IDC_HOSTFOUND), SW_HIDE);
			ShowWindow (GetDlgItem (hwnd, IDC_HOSTNOTFOUND), SW_HIDE);

			GetDlgItemText (hwnd, IDC_HOSTNAME, 
						pdls->szHostName, sizeof(pdls->szHostName));

			pdls->hwndThreadParent = NULL;
			SetTimer (hwnd, LOOKUP_TIMER_ID, LOOKUP_TIMER_MS, NULL); 
			break;

		case IDC_HOSTNAME :
			if (HIWORD (wParam) == EN_CHANGE) 
			{
				if (GetWindowTextLength ((HWND)lParam) > 0)
					EnableWindow (GetDlgItem (hwnd, IDC_LOOKUP), TRUE);
				else
					EnableWindow (GetDlgItem (hwnd, IDC_LOOKUP), FALSE);
			}
			break;
		}

		return 0;
	}

	return FALSE;
}


//	____________________________________
//
//	push the right button

static VOID
sSetAuthKeyRadioButtons (
		HWND			hwnd,
		PHOSTEDITSTRUCT	phes)
{
	UINT	uPushed		= IDC_ANYVALIDKEY;

	if (phes->host.authKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		// if no IASN, must be a PGP key
		if (phes->host.authCertIASNLength == 0)
			uPushed = IDC_PGPAUTHKEY;
#if !PGP_FREEWARE
		else
			uPushed = IDC_X509CERT;
#endif //PGP_FREEWARE
	}

	CheckRadioButton (hwnd, IDC_ANYVALIDKEY, IDC_X509CERT, uPushed);
	SetFocus (GetDlgItem (hwnd, uPushed));
}


//	____________________________________
//
//	put name of key into control

static VOID
sSetKeyControl (
		HWND			hwnd,
		PHOSTEDITSTRUCT	phes)
{
	CHAR			sz[kPGPMaxUserIDSize +1];
	UINT			u, uIcon;
	HICON			hIcon;
	PGPKeyRef		key;
	PGPSigRef		sig;
	PGPKeyID		keyid;
	PGPError		err;

	sz[0] = '\0';
	uIcon = 0;
	hIcon = NULL;

	err = PGPnetGetConfiguredHostAuthKeys (g_context, &phes->host, 
										g_AOS.keysetMain, &key, &sig);

	if (IsntPGPError (err))
	{
		if (PGPKeyRefIsValid (key))
		{
			// X.509 certification
			if (PGPSigRefIsValid (sig))
			{
				uIcon = PNDetermineCertIcon (sig, NULL);
				PGPGetSigPropertyBuffer (sig,
							kPGPSigPropX509LongName, sizeof(sz), sz, &u);
			}
			// PGP key
			else
			{
				uIcon = PNDetermineKeyIcon (key, NULL);
				PGPGetPrimaryUserIDNameBuffer (key, sizeof(sz), sz, &u);
			}

			hIcon = ImageList_GetIcon (phes->hil, uIcon, ILD_TRANSPARENT);
		}
	}

	// couldn't find key or sig on keyring
	else
	{
		err = PGPImportKeyID (phes->host.authKeyExpKeyID, &keyid);
		if (IsntPGPError (err))
		{
			CHAR	szID[kPGPMaxKeyIDStringSize];

			if (PGPKeyRefIsValid (key))
				LoadString (g_hinst, IDS_UNKNOWNCERT, sz, sizeof(sz));
			else
				LoadString (g_hinst, IDS_UNKNOWNKEY, sz, sizeof(sz));

			PGPGetKeyIDString (&keyid, 
					kPGPKeyIDString_Abbreviated, szID);
			lstrcat (sz, szID);
		}
	}

	SetDlgItemText (hwnd, IDC_AUTHEDIT, sz);
	SendDlgItemMessage (hwnd, IDC_AUTHICON, STM_SETICON, (WPARAM)hIcon, 0);
}


//	____________________________________
//
//	initialize the list view control

static VOID
sInitKeyControl (
		PHOSTEDITSTRUCT	phes)
{
	HBITMAP			hBmp; 
	HDC				hDC;
	INT				iNumBits;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps based on 
	// current display capabilities.
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		phes->hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG4));
		ImageList_AddMasked (phes->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		phes->hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG24));
		ImageList_AddMasked (phes->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
}


//	___________________________________________________
//
//	get the first key from a keyset

static PGPError
sGetFirstKeyInSet(
	PGPKeySetRef 	keyset,
	PGPKeyRef*		pkey)
{
	PGPError		err;
	PGPKeyListRef	keylist;
	
	*pkey = kInvalidPGPKeyRef;
	
	err = PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
	if (IsntPGPError (err))
	{
		PGPKeyIterRef	keyiter;
		
		err = PGPNewKeyIter (keylist, &keyiter);
		if (IsntPGPError (err))
		{
			err = PGPKeyIterNext (keyiter, pkey);
			if (err == kPGPError_EndOfIteration)
				err = kPGPError_ItemNotFound;
			
			PGPFreeKeyIter (keyiter);
		}
		
		PGPFreeKeyList (keylist);
	}

	return err;
}


//	____________________________________
//
//	convert the key/cert to export format

BOOL
sRefsToExport (
		HWND				hwnd,
		PGPKeyRef			key,
		PGPSigRef			sig,
		PGPNetHostEntry*	pHost)
{
	BOOL		bReturn		= FALSE;
	PGPError	err;

	if (PGPSigRefIsValid (sig))
	{
		PGPByte*	pIASN	= NULL;

		err = PGPX509CertToExport (g_context, key, sig,
			&pHost->authKeyAlg, pHost->authKeyExpKeyID,
			&pIASN,
			&pHost->authCertIASNLength);

		if (!PGPclErrorBox (hwnd, err))
		{
			if (pHost->authCertIASNLength > kMaxNetHostIASNLength) 
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_IASNTOOLONG, 
								MB_OK|MB_ICONSTOP);
			}
			else
			{
				pgpCopyMemory (pIASN, pHost->authCertIASN, 
										pHost->authCertIASNLength);
				bReturn = TRUE;
			}
		}

		if (IsntNull (pIASN))
			PGPFreeData (pIASN);
	}

	else if (PGPKeyRefIsValid (key))
	{
		PGPKeyID	keyid;
		PGPSize		size;

		err = PGPGetKeyNumber (key, 
						kPGPKeyPropAlgID, &pHost->authKeyAlg);

		if (IsntPGPError (err))
		{
			err = PGPGetKeyIDFromKey (key, &keyid);
			if (IsntPGPError (err))
			{
				err = PGPExportKeyID (
							&keyid, pHost->authKeyExpKeyID, &size);
				if (IsntPGPError (err))
				{
					pHost->authCertIASNLength = 0;
					bReturn = TRUE;
				}
			}
		}
	}

	else
	{
		pHost->authKeyAlg = kPGPPublicKeyAlgorithm_Invalid;
		pHost->authCertIASNLength = 0;
		bReturn = TRUE;
	}

	return bReturn;
}


//	____________________________________
//
//	dialog proc of edit dialog

static BOOL CALLBACK
sEditHostDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PHOSTEDITSTRUCT		phes;
	INT					iIndex;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		CHAR			sz[32];
		RECT			rc;
		DWORD			dw;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		phes = (PHOSTEDITSTRUCT)lParam;

		sInitKeyControl (phes);

		SetDlgItemText (hwnd, IDC_DOMAINNAME, phes->host.hostName);
		SetDlgItemText (hwnd, IDC_IDENTITY, phes->host.identity);
		SendDlgItemMessage (hwnd, IDC_IDENTITY,
					EM_LIMITTEXT, kMaxNetHostIdentityLength, 0);

		// create IP edit control for address
		GetWindowRect (GetDlgItem (hwnd, IDC_IPADDRESS), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		phes->hwndIPAddr = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, (HMENU)IDC_IPADDRESS, g_hinst, NULL);
		// set tab-order of control
		SetWindowPos (phes->hwndIPAddr, GetDlgItem (hwnd, IDC_IPADDRESS),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		// initialize contents of control
		dw = phes->host.ipAddress;
		if (dw != 0)
			SendMessage (phes->hwndIPAddr, PGP_IPM_SETADDRESS, 0, (LPARAM)dw);

		// create IP edit control for mask
		GetWindowRect (GetDlgItem (hwnd, IDC_IPMASK), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		phes->hwndIPSubnet = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, (HMENU)IDC_IPMASK, g_hinst, NULL);
		// set tab-order of control
		SetWindowPos (phes->hwndIPSubnet, GetDlgItem (hwnd, IDC_IPMASK),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		// initialize contents of control
		dw = phes->host.ipMask;
		SendMessage (phes->hwndIPSubnet, PGP_IPM_SETADDRESS, 0, (LPARAM)dw);

		// create IP edit control for ID
		GetWindowRect (GetDlgItem (hwnd, IDC_IPID), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		phes->hwndIPID = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, (HMENU)IDC_IPID, g_hinst, NULL);
		// set tab-order of control
		SetWindowPos (phes->hwndIPID, GetDlgItem (hwnd, IDC_IPID),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		// initialize passphrase static text
		if (phes->host.sharedSecret[0] != '\0')
			LoadString (g_hinst, IDS_CLEARPASSPHRASE, sz, sizeof(sz));
		else
			LoadString (g_hinst, IDS_PASSPHRASE, sz, sizeof(sz));

		SetDlgItemText (hwnd, IDC_PASSPHRASE, sz);

		// initialize host type combo box
		LoadString (g_hinst, IDS_INSECUREHOST, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_TYPE, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_INSECURESUBNET, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_TYPE, CB_ADDSTRING, 0, (LPARAM)sz);

#if !PGPNET_SECURE_HOSTS_BEHIND_GATEWAYS
		if (!phes->bIsChild) 
#endif
		{
			LoadString (g_hinst, IDS_SECUREHOST, sz, sizeof(sz));
			SendDlgItemMessage (hwnd, IDC_TYPE, CB_ADDSTRING, 0, (LPARAM)sz);
			LoadString (g_hinst, IDS_SECURESUBNET, sz, sizeof(sz));
			SendDlgItemMessage (hwnd, IDC_TYPE, CB_ADDSTRING, 0, (LPARAM)sz);
		}

#if !PGP_FREEWARE
		if (!phes->bIsChild) 
		{
			LoadString (g_hinst, IDS_SECUREGATEWAY, sz, sizeof(sz));
			SendDlgItemMessage (hwnd, IDC_TYPE, CB_ADDSTRING, 0, (LPARAM)sz);
		}
#endif //PGP_FREEWARE

		switch (phes->host.hostType) {
		case kPGPnetInsecureHost :	
			if (phes->host.ipMask == 0xFFFFFFFF)
				iIndex = INSECURE_HOST;		
			else
				iIndex = INSECURE_SUBNET;
			break; 

		case kPGPnetSecureHost :	
			if (phes->host.ipMask == 0xFFFFFFFF)
				iIndex = SECURE_HOST;		
			else
				iIndex = SECURE_SUBNET;
			break;

		case kPGPnetSecureGateway :	
			iIndex = SECURE_GATEWAY;	
			break;
		}
		SendDlgItemMessage (hwnd, IDC_TYPE, CB_SETCURSEL, (WPARAM)iIndex, 0);
		PostMessage (hwnd, WM_COMMAND, 
							MAKEWPARAM(IDC_TYPE, CBN_SELCHANGE), 0);

		// initialize identity type combo box
		LoadString (g_hinst, IDS_IPADDRESS, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_IDENTITYTYPE, 
									CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_DOMAINNAME, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_IDENTITYTYPE, 
									CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_USERDOMAINNAME, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_IDENTITYTYPE, 
									CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_DISTINGUISHEDNAME, sz, sizeof(sz));
		SendDlgItemMessage (hwnd, IDC_IDENTITYTYPE, 
									CB_ADDSTRING, 0, (LPARAM)sz);

		switch (phes->host.identityType) {
		case kPGPike_ID_FQDN :	
			phes->iPrevIDIndex = DOMAINNAME;		
			phes->szIdentity[0] = '\0';
			SetDlgItemText (hwnd, IDC_IDENTITY, phes->host.identity);
			break; 

		case kPGPike_ID_UserFDQN :	
			phes->iPrevIDIndex = USERDOMAINNAME;
			lstrcpy (phes->szIdentity, phes->host.identity);
			SetDlgItemText (hwnd, IDC_IDENTITY, phes->host.identity);
			break; 

		case kPGPike_ID_DER_ASN1_DN :	
			phes->iPrevIDIndex = DISTINGUISHEDNAME;		
			lstrcpy (phes->szIdentity, phes->host.identity);
			SetDlgItemText (hwnd, IDC_IDENTITY, phes->host.identity);
			break; 

		case kPGPike_ID_IPV4_Addr :	
		default :
		{
			phes->iPrevIDIndex = IPADDRESS;	
			phes->szIdentity[0] = '\0';
			
			ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_HIDE);
			ShowWindow (phes->hwndIPID, SW_SHOW);

			phes->dwAddress = phes->host.identityIPAddress;
			SendMessage (phes->hwndIPID, PGP_IPM_SETADDRESS, 0, 
						(LPARAM)phes->dwAddress);
			break; 
		}

		}
		SendDlgItemMessage (hwnd, IDC_IDENTITYTYPE, 
							CB_SETCURSEL, (WPARAM)phes->iPrevIDIndex, 0);

		// initialize authentication info
		sSetAuthKeyRadioButtons (hwnd, phes);
		sSetKeyControl (hwnd, phes);

		// allow keyring reload messages to percolate
		g_AOS.hwndHostDlg = hwnd;

		return TRUE;
	}

	case WM_DESTROY :
		g_AOS.hwndHostDlg = NULL;
		break;

	case PGPNET_M_RELOADKEYRINGS :
		phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		sSetKeyControl (hwnd, phes);
		break;

	case WM_HELP : 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aEditIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aEditIds); 
		break; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDOK :
		{
			DWORD	dwIP, dwMask;
			INT		iIdIndex;
			BOOL	bBadMask = FALSE;

			phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			GetDlgItemText (hwnd, IDC_DOMAINNAME,
					phes->host.hostName, sizeof(phes->host.hostName));

			// get IP address
			SendMessage (phes->hwndIPAddr, 
								PGP_IPM_GETADDRESS, 0, (LPARAM)&dwIP);
			if ((dwIP == 0x00000000) ||
				(dwIP == 0xFFFFFFFF))
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_BADIPADDRESS,
									MB_OK | MB_ICONHAND);
				break;
			}
			else
				phes->host.ipAddress = dwIP;

			// get IP subnet
			SendMessage (phes->hwndIPSubnet, 
								PGP_IPM_GETADDRESS, 0, (LPARAM)&dwMask);
			phes->host.ipMask = dwMask;

			GetDlgItemText (hwnd, IDC_IDENTITY,
					phes->host.identity, sizeof(phes->host.identity));

			iIndex = SendDlgItemMessage (hwnd, IDC_TYPE, CB_GETCURSEL, 0, 0);
			switch (iIndex) {
			case SECURE_HOST :		
				phes->host.hostType = kPGPnetSecureHost;	
				phes->host.ipMask = 0xFFFFFFFF;
				iIdIndex = SendDlgItemMessage (hwnd, 
									IDC_IDENTITYTYPE, CB_GETCURSEL, 0, 0);
				if (iIdIndex == NOIDENTITY)
					iIdIndex = BADIDENTITY;
				break;
	
			case INSECURE_HOST :		
				phes->host.hostType = kPGPnetInsecureHost;		
				phes->host.ipMask = 0xFFFFFFFF;
				phes->host.identityType = kPGPike_ID_None;
				phes->host.identity[0] = '\0';
				phes->host.identityIPAddress = 0;
				phes->host.sharedSecret[0] = '\0';
				sRefsToExport (hwnd, kInvalidPGPKeyRef, kInvalidPGPSigRef, 
							&phes->host);
				sSetAuthKeyRadioButtons (hwnd, phes);
				sSetKeyControl (hwnd, phes);
				iIdIndex = NOIDENTITY;
				break;

			case SECURE_SUBNET :		
				phes->host.hostType = kPGPnetSecureHost;		
				iIdIndex = SendDlgItemMessage (hwnd, 
									IDC_IDENTITYTYPE, CB_GETCURSEL, 0, 0);
				if (!PNIsSubnetMaskValid (dwMask))
					bBadMask = TRUE;
				if (iIdIndex == NOIDENTITY)
					iIdIndex = BADIDENTITY;
				break;

			case INSECURE_SUBNET :		
				phes->host.hostType = kPGPnetInsecureHost;		
				phes->host.identityType = kPGPike_ID_None;
				phes->host.identity[0] = '\0';
				phes->host.identityIPAddress = 0;
				phes->host.sharedSecret[0] = '\0';
				sRefsToExport (hwnd, kInvalidPGPKeyRef, kInvalidPGPSigRef, 
							&phes->host);
				sSetAuthKeyRadioButtons (hwnd, phes);
				sSetKeyControl (hwnd, phes);
				if (!PNIsSubnetMaskValid (dwMask))
					bBadMask = TRUE;
				iIdIndex = NOIDENTITY;
				break;

			case SECURE_GATEWAY :	
				phes->host.hostType = kPGPnetSecureGateway;		
				phes->host.ipMask = 0xFFFFFFFF;
				iIdIndex = SendDlgItemMessage (hwnd, 
									IDC_IDENTITYTYPE, CB_GETCURSEL, 0, 0);
				if (iIdIndex == NOIDENTITY)
					iIdIndex = BADIDENTITY;
				break;
			}

			if (bBadMask)
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_BADSUBNETMASK,
									MB_OK | MB_ICONHAND);
				break;
			}

			if (PNIsHostAlreadyInList (hwnd, 
					phes->host.hostType, 
					phes->host.ipAddress, 
					phes->host.ipMask, 
					phes->ppnconfig->pHostList, 
					phes->ppnconfig->uHostCount,
					phes->iIndex))
				break;

			switch (iIdIndex) {
			case IPADDRESS :
				phes->host.identityType = kPGPike_ID_IPV4_Addr;
				SendMessage (phes->hwndIPID, 
						PGP_IPM_GETADDRESS, 0, (LPARAM)&phes->dwAddress);
				phes->host.identityIPAddress = phes->dwAddress;
				break;

			case DOMAINNAME :
				phes->host.identityType = kPGPike_ID_FQDN;
				GetDlgItemText (hwnd, IDC_IDENTITY, 
							phes->host.identity, kMaxNetHostIdentityLength);
				break;

			case USERDOMAINNAME :
				phes->host.identityType = kPGPike_ID_UserFDQN;
				GetDlgItemText (hwnd, IDC_IDENTITY, 
							phes->host.identity, kMaxNetHostIdentityLength);
				break;

			case DISTINGUISHEDNAME :
			{
				PGPByte*	pbyte	= NULL;
				PGPSize		size;
				PGPError	err;

				phes->host.identityType = kPGPike_ID_DER_ASN1_DN;
				GetDlgItemText (hwnd, IDC_IDENTITY, 
							phes->host.identity, kMaxNetHostIdentityLength);

				if (phes->host.sharedSecret[0])
				{
					err = PGPCreateDistinguishedName (
									g_context, phes->host.identity, 
									&pbyte, &size);
					if (err == kPGPError_InvalidDistinguishedName)
						PNMessageBox (hwnd, IDS_CAPTION, IDS_DNINVALID,
											MB_OK|MB_ICONSTOP);
					else
						PGPclErrorBox (hwnd, err);

					if (IsPGPError (err))
					{
						SetFocus (GetDlgItem (hwnd, IDC_IDENTITY));
						SendDlgItemMessage (
								hwnd, IDC_IDENTITY, EM_SETSEL, 0, -1);
						iIdIndex = BADIDENTITY;
					}
				}

				if (pbyte)
					PGPFreeData (pbyte);
				break;
			}

			case NOIDENTITY :
				phes->host.identityType = kPGPike_ID_None;
				pgpClearMemory (phes->host.identity, 
										kMaxNetHostIdentityLength);
				break;

			default :
				PNMessageBox (hwnd, IDS_CAPTION, IDS_BADIDENTITY,
									MB_OK | MB_ICONHAND);
				break;
			}

			if (iIdIndex != BADIDENTITY)
				EndDialog (hwnd, 1);
			break;
		}

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;

		case IDC_IDENTITYTYPE :
		{
			CHAR	sz[256];

			phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			if (HIWORD(wParam) != CBN_SELCHANGE)
				break;

			switch (phes->iPrevIDIndex) {
			case USERDOMAINNAME :
			case DISTINGUISHEDNAME :
				GetDlgItemText (hwnd, IDC_IDENTITY, phes->szIdentity,
									sizeof (phes->szIdentity));
				break;
			}

			phes->iPrevIDIndex = SendDlgItemMessage (hwnd, 
								IDC_IDENTITYTYPE, CB_GETCURSEL, 0, 0);

			switch (phes->iPrevIDIndex) {
			case IPADDRESS :
				PNGetLocalHostInfo (&phes->dwAddress, sz, sizeof(sz));
				SendMessage (phes->hwndIPID, PGP_IPM_SETADDRESS, 0, 
							(LPARAM)phes->dwAddress);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_HIDE);
				ShowWindow (phes->hwndIPID, SW_SHOW);
				break;

			case DOMAINNAME :
				PNGetLocalHostInfo (NULL, sz, sizeof(sz));
				SetDlgItemText (hwnd, IDC_IDENTITY, sz);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_SHOW);
				ShowWindow (phes->hwndIPID, SW_HIDE);
				break;

			case USERDOMAINNAME :
				SetDlgItemText (hwnd, IDC_IDENTITY, phes->szIdentity);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_SHOW);
				ShowWindow (phes->hwndIPID, SW_HIDE);
				break;

			case DISTINGUISHEDNAME :
				SetDlgItemText (hwnd, IDC_IDENTITY, phes->szIdentity);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_SHOW);
				ShowWindow (phes->hwndIPID, SW_HIDE);
				break;
			}
			break;
		}

		case IDC_TYPE :
		{
			BOOL	bSecure	= FALSE;

			phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			if (HIWORD(wParam) != CBN_SELCHANGE)
				break;

			iIndex = SendDlgItemMessage (hwnd, IDC_TYPE, CB_GETCURSEL, 0, 0);

			if (phes->host.hostType == kPGPnetSecureGateway)
			{
				if ((iIndex != SECURE_GATEWAY) &&
					(phes->bHasChildren))
				{
					if (PNMessageBox (hwnd, IDS_CAPTION, 
							IDS_DELETEGATEWAYCONFIRMATION, 
							MB_OKCANCEL|MB_ICONEXCLAMATION) == IDCANCEL)
					{
						SendDlgItemMessage (hwnd, IDC_TYPE, CB_SETCURSEL, 
											SECURE_GATEWAY, 0);
						iIndex = SECURE_GATEWAY;
					}
				}
			}

			switch (iIndex) 
			{
			case SECURE_HOST :
				ShowWindow (GetDlgItem (hwnd, IDC_SUBNETTEXT), SW_HIDE);
				ShowWindow (phes->hwndIPSubnet, SW_HIDE);
				bSecure = TRUE;
				break;

			case INSECURE_HOST :
				ShowWindow (GetDlgItem (hwnd, IDC_SUBNETTEXT), SW_HIDE);
				ShowWindow (phes->hwndIPSubnet, SW_HIDE);
				break;

			case SECURE_SUBNET :
				ShowWindow (GetDlgItem (hwnd, IDC_SUBNETTEXT), SW_SHOW);
				ShowWindow (phes->hwndIPSubnet, SW_SHOW);
				bSecure = TRUE;
				break;

			case INSECURE_SUBNET :
				ShowWindow (GetDlgItem (hwnd, IDC_SUBNETTEXT), SW_SHOW);
				ShowWindow (phes->hwndIPSubnet, SW_SHOW);
				break;

			case SECURE_GATEWAY :
				ShowWindow (GetDlgItem (hwnd, IDC_SUBNETTEXT), SW_HIDE);
				ShowWindow (phes->hwndIPSubnet, SW_HIDE);
				bSecure = TRUE;
				break;
			}

			if (phes->host.sharedSecret[0] == '\0')
			{
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPETEXT), FALSE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPE), FALSE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTEXT), FALSE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITY), FALSE);
				EnableWindow (phes->hwndIPID, FALSE);
			}
			else
			{	
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPETEXT), TRUE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPE), TRUE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITYTEXT), TRUE);
				EnableWindow (GetDlgItem (hwnd, IDC_IDENTITY), TRUE);
				EnableWindow (phes->hwndIPID, TRUE);
			}

			if (bSecure)
			{
				ShowWindow (GetDlgItem (hwnd, IDC_SECRETGROUP), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPETEXT), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPE), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTEXT), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_PASSPHRASE), SW_SHOW);
				EnableWindow (GetDlgItem (hwnd, IDC_PASSPHRASE), TRUE);

				ShowWindow (GetDlgItem (hwnd, IDC_AUTHFRAME), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHICON), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHBORDER), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHEDIT), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_ANYVALIDKEY), SW_SHOW);
				EnableWindow (GetDlgItem (hwnd, IDC_ANYVALIDKEY), TRUE);
				ShowWindow (GetDlgItem (hwnd, IDC_PGPAUTHKEY), SW_SHOW);
				EnableWindow (GetDlgItem (hwnd, IDC_PGPAUTHKEY), TRUE);
#if !PGP_FREEWARE
				ShowWindow (GetDlgItem (hwnd, IDC_X509CERT), SW_SHOW);
				EnableWindow (GetDlgItem (hwnd, IDC_X509CERT), TRUE);
#endif //PGP_FREEWARE

				switch (phes->iPrevIDIndex) {
				case IPADDRESS :	
					ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_HIDE);
					ShowWindow (phes->hwndIPID, SW_SHOW);
					break;
				default :
					ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_SHOW);
					ShowWindow (phes->hwndIPID, SW_HIDE);
					break;
				}
			}
			else
			{
				ShowWindow (GetDlgItem (hwnd, IDC_SECRETGROUP), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPETEXT), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTYPE), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITYTEXT), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_PASSPHRASE), SW_HIDE);
				EnableWindow (GetDlgItem (hwnd, IDC_PASSPHRASE), FALSE);

				ShowWindow (GetDlgItem (hwnd, IDC_AUTHFRAME), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHICON), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHBORDER), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_AUTHEDIT), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_ANYVALIDKEY), SW_HIDE);
				EnableWindow (GetDlgItem (hwnd, IDC_ANYVALIDKEY), FALSE);
				ShowWindow (GetDlgItem (hwnd, IDC_PGPAUTHKEY), SW_HIDE);
				EnableWindow (GetDlgItem (hwnd, IDC_PGPAUTHKEY), FALSE);

				ShowWindow (GetDlgItem (hwnd, IDC_X509CERT), SW_HIDE);
				EnableWindow (GetDlgItem (hwnd, IDC_X509CERT), FALSE);

				ShowWindow (GetDlgItem (hwnd, IDC_IDENTITY), SW_HIDE);
				ShowWindow (phes->hwndIPID, SW_HIDE);
			}

			break;
		}

		case IDC_PASSPHRASE :
			{
				CHAR sz[64];
				phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

				if (phes->host.sharedSecret[0] == '\0')
					PNGetSharedSecret (hwnd, 
								phes->host.sharedSecret,
								kMaxNetHostSharedSecretLength);
				else
					pgpClearMemory (phes->host.sharedSecret, 
										kMaxNetHostSharedSecretLength);

				if (phes->host.sharedSecret[0] != '\0')
					LoadString (g_hinst, IDS_CLEARPASSPHRASE, sz, sizeof(sz));
				else 
					LoadString (g_hinst, IDS_PASSPHRASE, sz, sizeof(sz));
				SetDlgItemText (hwnd, IDC_PASSPHRASE, sz);

				PostMessage (hwnd, WM_COMMAND, 
									MAKEWPARAM(IDC_TYPE, CBN_SELCHANGE), 0);
			}
			break;

		case IDC_DNSLOOKUP :
		{
			DNSLOOKUPSTRUCT		dls;

			GetDlgItemText (hwnd, IDC_DOMAINNAME, 
								dls.szHostName, sizeof(dls.szHostName));

			if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_DNSLOOKUP),
							hwnd, sDNSLookupDlgProc, (LPARAM)&dls))
			{
				phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
				SendMessage (phes->hwndIPAddr, PGP_IPM_SETADDRESS, 
								0, (LPARAM)dls.dwAddress);
			}
			break;
		}

		case IDC_ANYVALIDKEY :
			phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			sRefsToExport (hwnd, kInvalidPGPKeyRef, kInvalidPGPSigRef, 
							&phes->host);
			sSetKeyControl (hwnd, phes);
			sSetAuthKeyRadioButtons (hwnd, phes);
			break;

		case IDC_PGPAUTHKEY :
			if (!g_bAskingUserForKey)
			{
				PGPKeySetRef	keyset;
				PGPKeyRef		key;
				PGPError		err;

				phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

				g_bAskingUserForKey = TRUE;
				err = PGPclSelectKeysEx (g_context, NULL, hwnd, NULL,
							g_AOS.keysetMain, g_AOS.keysetMain, 
							PGPCL_SINGLESELECTION, &keyset);

				if (IsntPGPError (err))
				{
					sGetFirstKeyInSet (keyset, &key);

					if (sRefsToExport (hwnd, key, kInvalidPGPSigRef, &phes->host))
					{
						PGPFreeKeySet (keyset);
						sSetKeyControl (hwnd, phes);
					}
				}
				sSetAuthKeyRadioButtons (hwnd, phes);

				g_bAskingUserForKey = FALSE;
			}
			break;

		case IDC_X509CERT :
			if (!g_bAskingUserForKey)
			{
				PGPError	err;
				PGPKeyRef	key;
				PGPSigRef	sig;

				phes = (PHOSTEDITSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

				g_bAskingUserForKey = TRUE;
				err = PGPclSelectX509Cert (g_context, hwnd, NULL,
							g_AOS.keysetMain, 
							PGPCL_NOSPLITKEYS,
							&key, &sig);

				if (IsntPGPError (err) &&
					sRefsToExport (hwnd, key, sig, &phes->host))
				{
					sSetKeyControl (hwnd, phes);
				}
				sSetAuthKeyRadioButtons (hwnd, phes);

				g_bAskingUserForKey = FALSE;
			}

			break;
		}
		return 0;
	}

	return FALSE;
}


//	____________________________________
//
//	post the edit host dialog

BOOL
PNEditHostEntry (
		HWND		hwndParent,
		PPNCONFIG	ppnconfig,
		INT			iIndex,
		BOOL		bHasChildren)
{
	HOSTEDITSTRUCT		hes;
	
	if (ppnconfig->pHostList[iIndex].childOf == -1)
		hes.bIsChild = FALSE;
	else
		hes.bIsChild = TRUE;

	hes.ppnconfig		= ppnconfig;
	hes.iIndex			= iIndex;
	hes.bHasChildren	= bHasChildren;

	memcpy (&hes.host, &(ppnconfig->pHostList[iIndex]), sizeof(hes.host));

	if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_HOSTGATEWAY),
			hwndParent, sEditHostDlgProc, (LPARAM)&hes))
	{
		// copy new host entry over old
		memcpy (&(ppnconfig->pHostList[iIndex]), &hes.host, sizeof(hes.host));
		return TRUE;
	}
	else
		return FALSE;
}

