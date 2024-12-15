/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNaddwiz.c - PGPnet add host/gateway wizard
	

	$Id: PNaddwiz.c,v 1.40.4.1.2.1 1999/06/29 18:46:08 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PGPnetApp.h"
#include "PGPnetHelp.h"
#include "IPAddressControl.h"
#include "PGPnetHelp.h"

#include "pgpBuildFlags.h"
#include "PGPcl.h"
#include "pgpUserInterface.h"
#include "pgpIke.h"
#include "pgpClientLib.h"

#define		TIMER_ID				111L
#define		TIMER_MS				100L
#define		NUMLEDS					10
#define		LEDSPACING				2

#define		WIZ_HOST				0
#define		WIZ_SUBNET				1
#define		WIZ_GATEWAY				2

#define		ADDWIZ_INTRO			0
#define		ADDWIZ_BEHINDGATEWAY	1
#define		ADDWIZ_HOSTTYPE			2
#define		ADDWIZ_SECURE			3
#define		ADDWIZ_HOSTNAME			4
#define		ADDWIZ_SUBNETADDRESS	5
#define		ADDWIZ_HOSTADDRESS		6
#define		ADDWIZ_ADDRESSLOOKUP	7
#define		ADDWIZ_USESHAREDSECRET	8
#define		ADDWIZ_SHAREDSECRET		9
#define		ADDWIZ_IDENTITYTYPE		10
#define		ADDWIZ_IDENTITY			11
#define		ADDWIZ_ADDMORE			12
#define		ADDWIZ_AUTHKEYS			13
#define		ADDWIZ_DONE				14
#define		NUM_WIZ_PAGES			15

extern HINSTANCE		g_hinst;
extern PGPContextRef	g_context;
extern CHAR				g_szHelpFile[];
extern APPOPTIONSSTRUCT	g_AOS;
extern BOOL				g_bAuthAccess;	

typedef struct {
	PGPContextRef		context;
	HWND				hwndThreadParent;
	INT					iStatusDirection;
	INT					iStatusValue;

	HBITMAP				hBitmap;
	HPALETTE			hPalette;

	HWND				hwndIPAddress;
	BOOL				bIPValid;
	HWND				hwndSubnetMask;
	BOOL				bSubnetValid;

	HWND				hwndHostIPAddress;
	CHAR				szHostName[256];
	HWND				hwndHostIPAddressLookup;
	DWORD				dwFoundIPAddress;

	BOOL				bNeedsAuthKey;
	PNCONFIG			pncfg;

	PGPNetHostEntry*	pheNew;
	PPNCONFIG			ppnconfig;
	INT					iSelectedHost;
	BOOL				bBehindGateway;
	UINT				uHostType;
	BOOL				bSecure;
	BOOL				bHideTyping;
	HBRUSH				hbrushEdit;
	BOOL				bSharedSecret;
	BOOL				bFirstTime;
	BOOL				bFinished;
	DWORD				dwIdIPAddress;
	WNDPROC				wpOrigSharedSecret1Proc;  
	WNDPROC				wpOrigSharedSecret2Proc;  
	HIMAGELIST			hil;
} ADDHOSTSTRUCT, *PADDHOSTSTRUCT;


//	______________________________________________
//
//  save the host data structure to the prefs file

static INT
sSaveHostEntry (
		PADDHOSTSTRUCT	pahs)
{
	PGPNetHostEntry*	pNewHostList;

	if (pahs->uHostType == WIZ_GATEWAY)
		pahs->pheNew->hostType = kPGPnetSecureGateway;
	else 
	{
		if (pahs->bSecure)
			pahs->pheNew->hostType = kPGPnetSecureHost;
		else
			pahs->pheNew->hostType = kPGPnetInsecureHost;
	}

	//
	if (pahs->bBehindGateway)
		pahs->pheNew->childOf = pahs->iSelectedHost;
	else
		pahs->pheNew->childOf = -1;

	// set IP address if using IPAddress identification
	if (pahs->pheNew->identityType == kPGPike_ID_IPV4_Addr)
		pahs->pheNew->identityIPAddress = pahs->dwIdIPAddress;

	pNewHostList = 
		PGPNewData (PGPGetContextMemoryMgr (pahs->context),
				(pahs->ppnconfig->uHostCount+1) * sizeof(PGPNetHostEntry),
				kPGPMemoryMgrFlags_Clear);

	if (pNewHostList) 
	{
		if (pahs->ppnconfig->uHostCount > 0)
			pgpCopyMemory (pahs->ppnconfig->pHostList, pNewHostList,
				pahs->ppnconfig->uHostCount * sizeof(PGPNetHostEntry));
		pgpCopyMemory (pahs->pheNew, 
						&pNewHostList[pahs->ppnconfig->uHostCount],
						sizeof(PGPNetHostEntry));
		pahs->ppnconfig->uHostCount++;

		if (pahs->ppnconfig->pHostList)
			PGPFreeData (pahs->ppnconfig->pHostList);
		pahs->ppnconfig->pHostList = pNewHostList;

		return pahs->ppnconfig->uHostCount -1;
	}

	return -1;
}

//	______________________________________________
//
//  Check if message is dangerous to pass to shared secret edit box

static BOOL 
sWizCommonSharedSecretMsgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	switch (uMsg) {

	case WM_RBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
	case WM_CONTEXTMENU :
		return TRUE;

	case WM_LBUTTONDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) return TRUE;
		break;

	case WM_PAINT :
		if (wParam) 
		{
			PADDHOSTSTRUCT pahs = 
				(PADDHOSTSTRUCT)GetWindowLong (
									GetParent (hwnd), GWL_USERDATA);

			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (pahs->bHideTyping) 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		}
		break; 

	case WM_KEYDOWN :
		if (GetKeyState (VK_SHIFT) & 0x8000) 
		{
			switch (wParam) {
				case VK_HOME :
				case VK_END :
				case VK_UP :
				case VK_DOWN :
				case VK_LEFT :
				case VK_RIGHT :
				case VK_NEXT :
				case VK_PRIOR :
					return TRUE;
			}
		}
		break;

	case WM_SETFOCUS :
		SendMessage (hwnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	case WM_KILLFOCUS :
		break;
	}

	return FALSE; 
} 


//	______________________________________________
//
//  Shared secret 1 edit box subclass procedure

static LRESULT APIENTRY 
sWizSharedSecret1SubclassProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	UINT				uQuality;
	CHAR				szBuf[256];
	LRESULT				lResult;
	PADDHOSTSTRUCT		pahs;

	if (sWizCommonSharedSecretMsgProc (hwnd, uMsg, wParam, lParam)) 
		return 0;

	pahs = (PADDHOSTSTRUCT)GetWindowLong (GetParent (hwnd), GWL_USERDATA);

	switch (uMsg) {
	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hwnd), IDC_DUMMYSTOP));
			else 
				SetFocus (GetDlgItem (GetParent (hwnd), 
											IDC_SHAREDSECRET2));
			break;

		case VK_RETURN :
			if (CallWindowProc (pahs->wpOrigSharedSecret1Proc, 
						hwnd, WM_GETTEXTLENGTH, 0, 0) > 0)
			{
				HWND hGrandParent = GetParent (GetParent (hwnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;


		default :
			lResult = CallWindowProc (pahs->wpOrigSharedSecret1Proc, 
						hwnd, uMsg, wParam, lParam); 
			CallWindowProc (pahs->wpOrigSharedSecret1Proc, 
						hwnd, WM_GETTEXT, sizeof(szBuf), (LPARAM)szBuf); 
			uQuality = PGPEstimatePassphraseQuality (szBuf);
			memset (szBuf, 0, sizeof(szBuf));
			SendDlgItemMessage (GetParent (hwnd), IDC_SECRETQUALITY, 
						PBM_SETPOS, uQuality, 0);
			return lResult;

		}
		break;

	}
	return CallWindowProc (pahs->wpOrigSharedSecret1Proc, 
									hwnd, uMsg, wParam, lParam); 
} 


//	______________________________________________
//
//  Shared secret 2 edit box subclass procedure

static LRESULT APIENTRY 
sWizSharedSecret2SubclassProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PADDHOSTSTRUCT		pahs;

	if (sWizCommonSharedSecretMsgProc (hwnd, uMsg, wParam, lParam)) 
		return 0;

	pahs = (PADDHOSTSTRUCT)GetWindowLong (GetParent (hwnd), GWL_USERDATA);

	switch (uMsg) {
	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
				SetFocus (GetDlgItem (GetParent (hwnd), IDC_SHAREDSECRET1));
			else 
				SetFocus (GetDlgItem (GetParent (hwnd), IDC_HIDETYPING));
			break;

		case VK_RETURN :
			if (CallWindowProc (pahs->wpOrigSharedSecret2Proc, 
						hwnd, WM_GETTEXTLENGTH, 0, 0) > 0)
			{
				HWND hGrandParent = GetParent (GetParent (hwnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
	return CallWindowProc (pahs->wpOrigSharedSecret2Proc, 
									hwnd, uMsg, wParam, lParam); 
} 


//	____________________________________
//
//  Draw the "LED" progress indicator

static VOID
sDrawSendStatus (
		HWND			hwnd,
		PADDHOSTSTRUCT	pahs) 
{
	HBRUSH			hBrushLit, hBrushUnlit, hBrushOld;
	HPEN			hPen, hPenOld;
	INT				i;
	INT				itop, ibot, ileft, iright, iwidth;
	RECT			rc;
	HDC				hdc;
	PAINTSTRUCT		ps;

	if (pahs->iStatusValue < -1) return;

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
	if (pahs->iStatusDirection) {
		hBrushOld = SelectObject (hdc, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i == pahs->iStatusValue) {
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
		if (pahs->iStatusValue >= 0) 
			hBrushOld = SelectObject (hdc, hBrushLit);
		else
			hBrushOld = SelectObject (hdc, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i > pahs->iStatusValue) {
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
	PADDHOSTSTRUCT	pahs		= (PADDHOSTSTRUCT)pvoid;

	if (PNGetRemoteHostIP (pahs->szHostName, &pahs->dwFoundIPAddress))
	{
		if (pahs->hwndThreadParent)
			SendMessage (pahs->hwndThreadParent, WM_APP, TRUE, 0);
	}
	else
	{
		if (pahs->hwndThreadParent)
			SendMessage (pahs->hwndThreadParent, WM_APP, FALSE, 0);
	}

	return 0;
}


//	______________________________________________
//
//  Dialog procedure for handling beginning introductory
//  dialog.

static LRESULT WINAPI 
sAddWizIntroDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		RECT rc;

		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// center dialog on screen
		GetWindowRect (GetParent (hwnd), &rc);
		SetWindowPos (GetParent (hwnd), NULL,
			(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
			(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE:
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user if host is behind gateway

static LRESULT WINAPI 
sAddWizBehindGatewayDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		CHAR			sz[kMaxNetHostNameLength +128];
		CHAR			szFormat[kMaxNetHostNameLength +1];
		PROPSHEETPAGE*	ppspMsgRec	= (PROPSHEETPAGE *) lParam;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		if (pahs->iSelectedHost >= 0)
		{
			// set text strings
			GetDlgItemText (hwnd, IDC_TEXT1, szFormat, sizeof(szFormat));
			wsprintf (sz, szFormat, 
				pahs->ppnconfig->pHostList[pahs->iSelectedHost].hostName);
			SetDlgItemText (hwnd, IDC_TEXT1, sz);

			GetDlgItemText (hwnd, IDC_TEXT2, szFormat, sizeof(szFormat));
			wsprintf (sz, szFormat, 
				pahs->ppnconfig->pHostList[pahs->iSelectedHost].hostName);
			SetDlgItemText (hwnd, IDC_TEXT2, sz);

			GetDlgItemText (hwnd, IDC_TEXT3, szFormat, sizeof(szFormat));
			wsprintf (sz, szFormat, 
				pahs->ppnconfig->pHostList[pahs->iSelectedHost].hostName);
			SetDlgItemText (hwnd, IDC_TEXT3, sz);
		}

		// set default state
		CheckRadioButton (hwnd, 
				IDC_BEHINDGATEWAY, IDC_NOTBEHINDGATEWAY, IDC_BEHINDGATEWAY);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
		{
			PGPUInt32	uHostType;
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			if (pahs->iSelectedHost < 0)
			{
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L); 
			}
			else
			{
				uHostType = 
					pahs->ppnconfig->pHostList[pahs->iSelectedHost].hostType;

				if (uHostType != kPGPnetSecureGateway) 
					SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
				else 
				{
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
					SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
					SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
				}
			}
			bReturnCode = TRUE;
			break;
		}

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_BEHINDGATEWAY) == BST_CHECKED)
				pahs->bBehindGateway = TRUE;
			else
				pahs->bBehindGateway = FALSE;
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user host type

static LRESULT WINAPI 
sAddWizHostTypeDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// set default state
		CheckRadioButton (hwnd, IDC_HOST, IDC_GATEWAY, IDC_HOST);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		CheckRadioButton (hwnd, IDC_HOST, IDC_GATEWAY, IDC_HOST);
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
		{
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

#if !PGP_FREEWARE
			if (pahs->bBehindGateway) 
			{
				ShowWindow (GetDlgItem (hwnd, IDC_GATEWAY), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_GATEWAYTEXT), SW_HIDE);
				EnableWindow (GetDlgItem (hwnd, IDC_GATEWAY), FALSE);
			}
			else
			{
				ShowWindow (GetDlgItem (hwnd, IDC_GATEWAY), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_GATEWAYTEXT), SW_SHOW);
				EnableWindow (GetDlgItem (hwnd, IDC_GATEWAY), TRUE);
			}
#endif //PGP_FREEWARE

			if (pahs->bFirstTime)
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			else
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);

			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;
		}

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_HOST) == BST_CHECKED)
				pahs->uHostType = WIZ_HOST;
			else if (IsDlgButtonChecked (hwnd, IDC_SUBNET) == BST_CHECKED)
				pahs->uHostType = WIZ_SUBNET;
			else 
				pahs->uHostType = WIZ_GATEWAY;
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user if secure or insecure communications

static LRESULT WINAPI 
sAddWizSecureDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// set default state
		CheckRadioButton (hwnd, IDC_SECURE, IDC_INSECURE, IDC_SECURE);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		CheckRadioButton (hwnd, IDC_SECURE, IDC_INSECURE, IDC_SECURE);
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pahs->uHostType == WIZ_GATEWAY) 
			{
				pahs->bSecure = TRUE;
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
#if !PGPNET_SECURE_HOSTS_BEHIND_GATEWAYS
			else if (pahs->bBehindGateway)
			{
				pahs->bSecure = FALSE;
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
#endif
			else
			{
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_SECURE) == BST_CHECKED)
				pahs->bSecure = TRUE;
			else
				pahs->bSecure = FALSE;
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user the domain name of host

static LRESULT WINAPI 
sAddWizHostNameDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		SetDlgItemText (hwnd, IDC_HOSTNAME, "");
		return 0;
	
	case WM_COMMAND :
		if (LOWORD (wParam) == IDC_HOSTNAME)
		{
			// enable/disable the 'Next' button
			if (GetWindowTextLength (GetDlgItem (hwnd, IDC_HOSTNAME)) > 0)
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_NEXT|PSWIZB_BACK);
			else
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_BACK);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			// enable/disable the 'Next' button
			if (GetWindowTextLength (GetDlgItem (hwnd, IDC_HOSTNAME)) > 0)
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_NEXT|PSWIZB_BACK);
			else
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_BACK);

			// set the window text
			switch (pahs->uHostType) {
			case WIZ_HOST :
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTHOST), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTSUBNET), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTGATEWAY), SW_HIDE);
				break;

			case WIZ_SUBNET :
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTHOST), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTSUBNET), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTGATEWAY), SW_HIDE);
				break;

			case WIZ_GATEWAY :
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTHOST), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTSUBNET), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTGATEWAY), SW_SHOW);
				break;
			}
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			SetFocus (NULL);
			GetDlgItemText (hwnd, IDC_HOSTNAME, pahs->pheNew->hostName,
								sizeof(pahs->pheNew->hostName));
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user the IP address of subnet

static LRESULT WINAPI 
sAddWizSubnetAddressDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE*	ppspMsgRec = (PROPSHEETPAGE *) lParam;
		RECT			rc;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// create IP edit control for address
		GetWindowRect (GetDlgItem (hwnd, IDC_IPADDRESS), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pahs->hwndIPAddress = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, NULL, g_hinst, NULL);
		// set tab-order of control
		SetWindowPos (pahs->hwndIPAddress, 
						GetDlgItem (hwnd, IDC_IPADDRESS),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		// create IP edit control for mask
		GetWindowRect (GetDlgItem (hwnd, IDC_SUBNETMASK), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pahs->hwndSubnetMask = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, NULL, g_hinst, NULL);
		// set tab-order of control
		SetWindowPos (pahs->hwndSubnetMask, 
						GetDlgItem (hwnd, IDC_SUBNETMASK),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		break;
	}

	case WM_DESTROY :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		DestroyWindow (pahs->hwndIPAddress);
		DestroyWindow (pahs->hwndSubnetMask);
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		// set default state
		SendMessage (pahs->hwndIPAddress, PGP_IPM_CLEARADDRESS, 0, 0);
		pahs->bIPValid = FALSE;
		SendMessage (pahs->hwndSubnetMask, PGP_IPM_CLEARADDRESS, 0, 0);
		pahs->bSubnetValid = FALSE;
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {

		case PGP_IPN_INVALIDADDRESS :
		case PGP_IPN_EMPTYADDRESS :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pnmh->hwndFrom == pahs->hwndIPAddress)
				pahs->bIPValid = FALSE;
			else if (pnmh->hwndFrom == pahs->hwndSubnetMask)
				pahs->bSubnetValid = FALSE;

			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
			break;

		case PGP_IPN_VALIDADDRESS :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pnmh->hwndFrom == pahs->hwndIPAddress)
				pahs->bIPValid = TRUE;
			else if (pnmh->hwndFrom == pahs->hwndSubnetMask)
				pahs->bSubnetValid = TRUE;

			if (pahs->bIPValid && pahs->bSubnetValid)
			{
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			}
			break;

		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			if (pahs->uHostType == WIZ_SUBNET)
			{
				if (pahs->bIPValid && pahs->bSubnetValid)
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);

				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);

			bReturnCode = TRUE;
			break;

		case PSN_WIZNEXT :
		{
			DWORD dwMask;
			DWORD dwIP;

			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			// get IP subnet
			SendMessage (pahs->hwndSubnetMask, 
									PGP_IPM_GETADDRESS, 0, (LPARAM)&dwMask);
			pahs->pheNew->ipMask = dwMask;

			if (!PNIsSubnetMaskValid (dwMask))
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_BADSUBNETMASK,
									MB_OK | MB_ICONHAND);
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
				bReturnCode = TRUE;
				break;
			}

			// get IP address
			SendMessage (pahs->hwndIPAddress, 
									PGP_IPM_GETADDRESS, 0, (LPARAM)&dwIP);
			pahs->pheNew->ipAddress = dwIP;

			if (PNIsHostAlreadyInList (hwnd, pahs->uHostType, dwIP, dwMask, 
					pahs->ppnconfig->pHostList,
					pahs->ppnconfig->uHostCount, -1))
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;
		}

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user the IP address of host or gateway

static LRESULT WINAPI 
sAddWizHostAddressDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE*	ppspMsgRec = (PROPSHEETPAGE *) lParam;
		RECT			rc;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// limit host name edit box text
		SendDlgItemMessage (hwnd, IDC_HOSTNAME, EM_LIMITTEXT, 
								(WPARAM)(sizeof(pahs->szHostName)), 0);

		// create IP edit control for address
		GetWindowRect (GetDlgItem (hwnd, IDC_IPADDRESS), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pahs->hwndHostIPAddress = CreateWindowEx (
				WS_EX_CLIENTEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, NULL, g_hinst, NULL);

		// set tab-order of control
		SetWindowPos (pahs->hwndHostIPAddress, 
						GetDlgItem (hwnd, IDC_IPADDRESS),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		break;
	}

	case WM_DESTROY :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		DestroyWindow (pahs->hwndHostIPAddress);
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		pahs->dwFoundIPAddress = 0;

		// set default state
		SendMessage (pahs->hwndHostIPAddress, PGP_IPM_CLEARADDRESS, 0, 0);
		SendMessage (pahs->hwndHostIPAddress, 
									EM_SETREADONLY, (WPARAM)FALSE, 0);
		EnableWindow (pahs->hwndHostIPAddress, TRUE);

		SetDlgItemText (hwnd, IDC_HOSTNAME, "");
		SendDlgItemMessage (hwnd, IDC_HOSTNAME, 
									EM_SETREADONLY, (WPARAM)FALSE, 0);
		EnableWindow (GetDlgItem (hwnd, IDC_HOSTNAME), TRUE);
		return 0;

	case WM_COMMAND :
	{
		if (LOWORD (wParam) == IDC_HOSTNAME)
		{
			if (HIWORD (wParam) == EN_CHANGE) 
			{
				pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
				pahs->dwFoundIPAddress = 0;

				if (GetWindowTextLength ((HWND)lParam) > 0)
				{
					SendMessage (pahs->hwndHostIPAddress, EM_SETREADONLY, 
									(WPARAM)TRUE, 0);
					EnableWindow (pahs->hwndHostIPAddress, FALSE);
					PostMessage	(GetParent (hwnd), 
							PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				}
				else
				{
					SendMessage (pahs->hwndHostIPAddress, EM_SETREADONLY, 
									(WPARAM)FALSE, 0);
					EnableWindow (pahs->hwndHostIPAddress, TRUE);
					PostMessage	(GetParent (hwnd), 
							PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				}
			}
		}
		break;
	}
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;
		DWORD	dw;

		switch(pnmh->code) {

		case PGP_IPN_EMPTYADDRESS :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pnmh->hwndFrom == pahs->hwndHostIPAddress)
			{
				SendDlgItemMessage (hwnd, IDC_HOSTNAME, EM_SETREADONLY, 
									(WPARAM)FALSE, 0);
				EnableWindow (GetDlgItem (hwnd, IDC_HOSTNAME), TRUE);
			}
			break;
		
		case PGP_IPN_INVALIDADDRESS :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pnmh->hwndFrom == pahs->hwndHostIPAddress)
			{
				SendDlgItemMessage (hwnd, IDC_HOSTNAME, EM_SETREADONLY, 
									(WPARAM)TRUE, 0);
				EnableWindow (GetDlgItem (hwnd, IDC_HOSTNAME), FALSE);
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
			}
			break;

		case PGP_IPN_VALIDADDRESS :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pnmh->hwndFrom == pahs->hwndHostIPAddress)
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			break;

		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
			switch (pahs->uHostType) {
			case WIZ_HOST :
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTHOST), SW_SHOW);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTGATEWAY), SW_HIDE);
				SendMessage (pahs->hwndHostIPAddress, 
								PGP_IPM_GETADDRESS, 0, (LPARAM)&dw);
				if ((dw != 0) || 
					(GetWindowTextLength (
							GetDlgItem (hwnd, IDC_HOSTNAME)) > 0) )
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);

				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
				break;

			case WIZ_GATEWAY :
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTHOST), SW_HIDE);
				ShowWindow (GetDlgItem (hwnd, IDC_TEXTGATEWAY), SW_SHOW);
				SendMessage (pahs->hwndHostIPAddress, 
								PGP_IPM_GETADDRESS, 0, (LPARAM)&dw);
				if ((dw != 0) || 
					(GetWindowTextLength (
							GetDlgItem (hwnd, IDC_HOSTNAME)) > 0) )
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				else
					PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);

				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
				break;

			default :
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
				break;
			}
			bReturnCode = TRUE;
			break;

		case PSN_WIZNEXT :
		{
			DWORD dw;

			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			// get host name
			GetDlgItemText (hwnd, IDC_HOSTNAME, pahs->szHostName, 
								sizeof(pahs->szHostName));

			// else get IP address
			if (pahs->szHostName[0] == 0)
				SendMessage (pahs->hwndHostIPAddress, 
									PGP_IPM_GETADDRESS, 0, (LPARAM)&dw);
			else
				dw = 0;
			pahs->pheNew->ipAddress = dw;

			// set IP subnet
			pahs->pheNew->ipMask = 0xFFFFFFFF;

			if (PNIsHostAlreadyInList (hwnd, pahs->uHostType, dw, 0xFFFFFFFF, 
					pahs->ppnconfig->pHostList,
					pahs->ppnconfig->uHostCount, -1))
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;
		}

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for looking up IP and posting result

static LRESULT WINAPI 
sAddWizAddressLookupDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE*	ppspMsgRec = (PROPSHEETPAGE *) lParam;
		RECT			rc;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// create IP edit control for address
		GetWindowRect (GetDlgItem (hwnd, IDC_IPADDRESS), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		pahs->hwndHostIPAddressLookup = CreateWindowEx (
				WS_EX_STATICEDGE,
				WC_PGPIPADDRESS, "", 
				WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_READONLY,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
				hwnd, NULL, g_hinst, NULL);

		// set tab-order of control
		SetWindowPos (pahs->hwndHostIPAddressLookup, 
						GetDlgItem (hwnd, IDC_IPADDRESS),
						0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		// disable window
		EnableWindow (pahs->hwndHostIPAddressLookup, FALSE);
		break;
	}

	case WM_TIMER :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hwndThreadParent)
		{
			ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_SHOW);
			pahs->iStatusValue += pahs->iStatusDirection;
			if (pahs->iStatusValue <= 0) {
				pahs->iStatusValue = 0;
				pahs->iStatusDirection = 1;
			}
			else if (pahs->iStatusValue >= NUMLEDS-1) {
				pahs->iStatusValue = NUMLEDS-1;
				pahs->iStatusDirection = -1;
			}
			InvalidateRect (hwnd, NULL, FALSE);
		}
		else 
		{
			DWORD	dw;

			pahs->iStatusValue = -1;
			pahs->iStatusDirection = 1;

			pahs->hwndThreadParent = hwnd;
			CreateThread (NULL, 0, sDNSLookupThread, (LPVOID)pahs, 0, &dw);
		}
		break;

	case WM_APP :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		pahs->hwndThreadParent = NULL;
		ShowWindow (GetDlgItem (hwnd, IDC_PROGRESS), SW_HIDE);

		KillTimer (hwnd, TIMER_ID);
		if (wParam)
		{
			CHAR	sz1[256];
			CHAR	sz2[512];

			ShowWindow (
				GetDlgItem (hwnd, IDC_WORKING), SW_HIDE);
			ShowWindow (
				GetDlgItem (hwnd, IDC_IPADDRESSTEXT), SW_SHOW);
			ShowWindow (
				pahs->hwndHostIPAddressLookup, SW_SHOW);
			SendMessage (pahs->hwndHostIPAddressLookup, 
				PGP_IPM_SETADDRESS, 0, (LPARAM)pahs->dwFoundIPAddress);

			LoadString (g_hinst, IDS_HOSTFOUND, sz1, sizeof(sz1));
			wsprintf (sz2, sz1, pahs->szHostName);
			SetDlgItemText (hwnd, IDC_HOSTFOUND, sz2);
			ShowWindow (GetDlgItem (hwnd, IDC_HOSTFOUND), SW_SHOW);

			PostMessage	(GetParent (hwnd), 
				PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
		}
		else
		{
			ShowWindow (
				GetDlgItem (hwnd, IDC_WORKING), SW_HIDE);
			ShowWindow (
				GetDlgItem (hwnd, IDC_HOSTNOTFOUND), SW_SHOW);
		}
		break;

	case WM_DESTROY :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		DestroyWindow (pahs->hwndHostIPAddressLookup);
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		sDrawSendStatus (GetDlgItem (hwnd, IDC_PROGRESS), pahs);
		break;

	case PSM_QUERYSIBLINGS :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		// set default state
		SendMessage (pahs->hwndHostIPAddressLookup, PGP_IPM_CLEARADDRESS, 0, 0);
		SetDlgItemText (hwnd, IDC_HOSTFOUND, "");
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {

		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			if ((pahs->szHostName[0] != 0) &&
				(pahs->uHostType != WIZ_SUBNET))
			{
				if (pahs->dwFoundIPAddress == 0)
				{
					CHAR	sz1[64];
					CHAR	sz2[320];

					PostMessage	(GetParent (hwnd), 
							PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);

					ShowWindow (
						GetDlgItem (hwnd, IDC_HOSTFOUND), SW_HIDE);
					ShowWindow (
						GetDlgItem (hwnd, IDC_HOSTNOTFOUND), SW_HIDE);
					ShowWindow (
						GetDlgItem (hwnd, IDC_IPADDRESSTEXT), SW_HIDE);
					ShowWindow (
						pahs->hwndHostIPAddressLookup, SW_HIDE);

					LoadString (g_hinst, IDS_DNSLOOKUP, sz1, sizeof(sz1));
					wsprintf (sz2, sz1, pahs->szHostName);
					SetDlgItemText (hwnd, IDC_WORKING, sz2);
					ShowWindow (GetDlgItem (hwnd, IDC_WORKING), SW_SHOW);

					SetTimer (hwnd, TIMER_ID, TIMER_MS, NULL);
				}

				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);

			bReturnCode = TRUE;
			break;

		case PSN_WIZNEXT :
		{
			DWORD dw;

			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			// get IP address
			SendMessage (pahs->hwndHostIPAddressLookup, 
									PGP_IPM_GETADDRESS, 0, (LPARAM)&dw);
			pahs->pheNew->ipAddress = dw;

			if (PNIsHostAlreadyInList (hwnd, pahs->uHostType, dw, 0xFFFFFFFF, 
					pahs->ppnconfig->pHostList,
					pahs->ppnconfig->uHostCount, -1))
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;
		}

		case PSN_QUERYCANCEL :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			// if querying DNS, just cancel the query
			if (pahs->hwndThreadParent)
			{
				pahs->hwndThreadParent = NULL;
				SendMessage (hwnd, WM_APP, FALSE, 0);
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
			// otherwise cancel the entire wizard
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user if we are to use public-key or
//	shared-secret security

static LRESULT WINAPI 
sAddWizUseSharedSecretDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// set default state
		CheckRadioButton (hwnd, 
				IDC_PUBLICKEY, IDC_SHAREDSECRET, IDC_PUBLICKEY);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		CheckRadioButton (hwnd, 
				IDC_PUBLICKEY, IDC_SHAREDSECRET, IDC_PUBLICKEY);
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pahs->bSecure == FALSE) 
			{
				pahs->bSharedSecret = FALSE;
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
			else
			{
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_SHAREDSECRET) == BST_CHECKED)
				pahs->bSharedSecret = TRUE;
			else
				pahs->bSharedSecret = FALSE;
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user for shared secret

static LRESULT WINAPI 
sAddWizSharedSecretDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		CheckDlgButton (hwnd, IDC_HIDETYPING, BST_CHECKED);
		pahs->hbrushEdit = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

		pahs->wpOrigSharedSecret1Proc = 
			(WNDPROC) SetWindowLong (GetDlgItem (hwnd, IDC_SHAREDSECRET1), 
						GWL_WNDPROC, (LONG) sWizSharedSecret1SubclassProc); 
		pahs->wpOrigSharedSecret2Proc = 
			(WNDPROC) SetWindowLong (GetDlgItem (hwnd, IDC_SHAREDSECRET2), 
						GWL_WNDPROC, (LONG) sWizSharedSecret2SubclassProc); 

		break;
	}

	case WM_DESTROY: 
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		DeleteObject (pahs->hbrushEdit);
		SetWindowLong (GetDlgItem (hwnd, IDC_SHAREDSECRET1),
						GWL_WNDPROC, (LONG)pahs->wpOrigSharedSecret1Proc); 
		SetWindowLong (GetDlgItem (hwnd, IDC_SHAREDSECRET2),
						GWL_WNDPROC, (LONG)pahs->wpOrigSharedSecret2Proc);
		break;

	case WM_CTLCOLOREDIT:
		if (((HWND)lParam == GetDlgItem (hwnd, IDC_SHAREDSECRET1)) ||
			((HWND)lParam == GetDlgItem (hwnd, IDC_SHAREDSECRET2))) 
		{
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (pahs->bHideTyping) 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else 
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
			return (BOOL)(pahs->hbrushEdit);
		}
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		SetDlgItemText (hwnd, IDC_SHAREDSECRET1, "");
		SetDlgItemText (hwnd, IDC_SHAREDSECRET2, "");
		return 0;
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_HIDETYPING:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_HIDETYPING) == BST_CHECKED) 
				pahs->bHideTyping = TRUE;
			else 
				pahs->bHideTyping = FALSE;
			InvalidateRect (GetDlgItem (hwnd, IDC_SHAREDSECRET1), NULL, TRUE);
			InvalidateRect (GetDlgItem (hwnd, IDC_SHAREDSECRET2), NULL, TRUE);
			break;

		case IDC_SHAREDSECRET1 :
			if (GetWindowTextLength (
						GetDlgItem (hwnd, IDC_SHAREDSECRET1)) > 0)
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_NEXT|PSWIZB_BACK);
			else
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_BACK);
			break;
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pahs->bSharedSecret == FALSE) 
			{
 				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
			else
			{
				// enable/disable the 'Next' button
				SetDlgItemText (hwnd, IDC_SHAREDSECRET1, "");
				SetDlgItemText (hwnd, IDC_SHAREDSECRET2, "");

				SendDlgItemMessage (hwnd, 
					IDC_SECRETQUALITY, PBM_SETPOS, 0, 0);

				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			bReturnCode = TRUE;
			break;

		case PSN_WIZNEXT:
		{
			CHAR	szConfirm[kMaxNetHostSharedSecretLength +1];

			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			GetDlgItemText (hwnd, IDC_SHAREDSECRET1, 
				pahs->pheNew->sharedSecret, kMaxNetHostSharedSecretLength+1);
			GetDlgItemText (hwnd, IDC_SHAREDSECRET2, 
				szConfirm, kMaxNetHostSharedSecretLength+1);

			if (lstrcmp (pahs->pheNew->sharedSecret, szConfirm))
			{
				pgpClearMemory (pahs->pheNew->sharedSecret, 
									kMaxNetHostSharedSecretLength);

				SetDlgItemText (hwnd, IDC_SHAREDSECRET1, "");
				SetDlgItemText (hwnd, IDC_SHAREDSECRET2, "");

				SendDlgItemMessage (hwnd, 
						IDC_SECRETQUALITY, PBM_SETPOS, 0, 0);
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);

				PNMessageBox (hwnd, IDS_CAPTION, 
					IDS_SECRETCONFIRMMISMATCH, MB_ICONEXCLAMATION|MB_OK);

				SetWindowLong (hwnd, DWL_MSGRESULT, 1L);
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;
		}

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user identity type

static LRESULT WINAPI 
sAddWizIdentityTypeDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		CHAR	sz1[128];
		CHAR	sz2[128];
		CHAR	sz[256];

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// set IP address 
		PNGetLocalHostInfo (&pahs->dwIdIPAddress, sz1, sizeof(sz1));
		GetDlgItemText (hwnd, IDC_IPADDRESSTEXT, sz2, sizeof(sz2));
		wsprintf (sz, sz2, sz1);
		SetDlgItemText (hwnd, IDC_IPADDRESSTEXT, sz);

		// set host name
		PNGetLocalHostInfo (NULL, sz1, sizeof(sz1));
		GetDlgItemText (hwnd, IDC_HOSTNAMETEXT, sz2, sizeof(sz2));
		wsprintf (sz, sz2, sz1);
		SetDlgItemText (hwnd, IDC_HOSTNAMETEXT, sz);

		// set user domain name
		PNGetLocalHostInfo (NULL, sz1, sizeof(sz1));
		GetDlgItemText (hwnd, IDC_USERDOMAINNAMETEXT, sz2, sizeof(sz2));
		wsprintf (sz, sz2, sz1);
		SetDlgItemText (hwnd, IDC_USERDOMAINNAMETEXT, sz);

		// set default state
		CheckRadioButton (hwnd, 
				IDC_IPADDRESS, IDC_DISTINGUISHEDNAME, IDC_IPADDRESS);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		CheckRadioButton (hwnd, 
				IDC_IPADDRESS, IDC_DISTINGUISHEDNAME, IDC_IPADDRESS);
		return 0;
	
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
		{
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (pahs->bSharedSecret == FALSE) 
			{
 				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			}
			else
			{
				PostMessage	(GetParent (hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}

			bReturnCode = TRUE;
			break;
		}

		case PSN_KILLACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (
							hwnd, IDC_IPADDRESS) == BST_CHECKED)
			{
				pahs->pheNew->identityType = kPGPike_ID_IPV4_Addr;
			}
			else if (IsDlgButtonChecked (
							hwnd, IDC_HOSTNAME) == BST_CHECKED)
			{
				pahs->pheNew->identityType = kPGPike_ID_FQDN;
				PNGetLocalHostInfo (NULL,
					pahs->pheNew->identity, sizeof(pahs->pheNew->identity));
			}
			else if (IsDlgButtonChecked (
							hwnd, IDC_USERDOMAINNAME) == BST_CHECKED)
			{
				pahs->pheNew->identityType = kPGPike_ID_UserFDQN;
			}
			else
			{
				pahs->pheNew->identityType = kPGPike_ID_DER_ASN1_DN;
			}

			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user the identity string

static LRESULT WINAPI 
sAddWizIdentityDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		SendDlgItemMessage (hwnd, IDC_IDENTITY, 
				EM_LIMITTEXT, kMaxNetHostIdentityLength, 0);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case PSM_QUERYSIBLINGS :
		// set default state
		SetDlgItemText (hwnd, IDC_IDENTITY, "");
		return 0;
	
	case WM_COMMAND :
		if (LOWORD (wParam) == IDC_IDENTITY)
		{
			// enable/disable the 'Next' button
			if (GetWindowTextLength (GetDlgItem (hwnd, IDC_IDENTITY)) > 0)
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_NEXT|PSWIZB_BACK);
			else
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_BACK);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			// enable/disable the 'Next' button
			if (GetWindowTextLength (GetDlgItem (hwnd, IDC_IDENTITY)) > 0)
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_NEXT|PSWIZB_BACK);
			else
				PropSheet_SetWizButtons (
						GetParent (hwnd), PSWIZB_BACK);

			if (!pahs->bSharedSecret)
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
			else
			{
				CHAR	sz1[128];
				CHAR	sz2[128];
				CHAR	szPrompt[256];

				switch (pahs->pheNew->identityType) {
				case kPGPike_ID_IPV4_Addr :
				case kPGPike_ID_FQDN :
					SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
					break;

				case kPGPike_ID_UserFDQN :
					PNGetLocalHostInfo (NULL, sz1, sizeof(sz1));
					LoadString (g_hinst, IDS_USERDOMAINNAMEPROMPT, 
									sz2, sizeof(sz2));
					wsprintf (szPrompt, sz2, sz1);
					SetDlgItemText (hwnd, IDC_IDENTITYPROMPT, szPrompt);
					SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
					break;

				case kPGPike_ID_DER_ASN1_DN :
					LoadString (g_hinst, IDS_DISTINGUISHEDNAMEPROMPT, 
									szPrompt, sizeof(szPrompt));
					SetDlgItemText (hwnd, IDC_IDENTITYPROMPT, szPrompt);
					SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
					break;
				}
			}

			bReturnCode = TRUE;
			break;

		case PSN_WIZNEXT :
		{
			PGPByte*	pbyte	= NULL;
			PGPSize		size;
			PGPError	err;

			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			GetDlgItemText (hwnd, IDC_IDENTITY, pahs->pheNew->identity,
								sizeof(pahs->pheNew->identity));

			if (pahs->pheNew->identityType == kPGPike_ID_DER_ASN1_DN)
			{

				err = PGPCreateDistinguishedName (
								pahs->context, pahs->pheNew->identity, 
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
					SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
				}
				else
					SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

				if (pbyte)
					PGPFreeData (pbyte);
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;
		}

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//	Dialog procedure for asking user to continue with host/subnet

static LRESULT WINAPI 
sAddWizAddMoreDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE*	ppspMsgRec	= (PROPSHEETPAGE *) lParam;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		// set default state
		CheckRadioButton (hwnd, IDC_CREATEMORE, IDC_NOMORE, IDC_CREATEMORE);
		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
		{
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			if (pahs->bFirstTime && 
				(pahs->uHostType != WIZ_GATEWAY)) 
			{
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L); // skip this panel
			}
			else 
			{
				// set text strings
				if (pahs->bFirstTime)
				{
					ShowWindow (GetDlgItem (hwnd, IDC_TEXT1), SW_SHOW);
					ShowWindow (GetDlgItem (hwnd, IDC_FIRSTTIME), SW_SHOW);
					ShowWindow (GetDlgItem (hwnd, IDC_CONTINUE), SW_HIDE);
					CheckRadioButton (hwnd, IDC_CREATEMORE, 
										IDC_NOMORE, IDC_CREATEMORE);
				}
				else
				{
					ShowWindow (GetDlgItem (hwnd, IDC_TEXT1), SW_HIDE);
					ShowWindow (GetDlgItem (hwnd, IDC_FIRSTTIME), SW_HIDE);
					ShowWindow (GetDlgItem (hwnd, IDC_CONTINUE), SW_SHOW);
					CheckRadioButton (hwnd, IDC_CREATEMORE, 
										IDC_NOMORE, IDC_NOMORE);
				}
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			bReturnCode = TRUE;
			break;
		}

		case PSN_WIZNEXT :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_CREATEMORE) == BST_CHECKED)
			{
				INT		iEntry;

				// send message to all panels to reset
				PropSheet_QuerySiblings (GetParent (hwnd), 0, 0);

				// reset data structure
				iEntry = sSaveHostEntry (pahs);
				if (iEntry >= 0)
				{
					if (pahs->bFirstTime) 
						pahs->iSelectedHost = iEntry;
					pahs->bFirstTime = FALSE;
					pahs->bBehindGateway = TRUE;
					pahs->uHostType = WIZ_HOST;
					pahs->szHostName[0] = '\0';
					pahs->dwFoundIPAddress = 0;
					pahs->pheNew->ipAddress = 0;
					pahs->pheNew->ipMask = 0;
					pahs->pheNew->hostName[0]		= '\0';
					pahs->pheNew->identity[0]		= '\0';
					pahs->pheNew->identityIPAddress	= 0;
					pahs->pheNew->sharedSecret[0]	= '\0';

					pahs->bNeedsAuthKey |= (pahs->bSecure && 
											!pahs->bSharedSecret);
					pahs->bSharedSecret = FALSE;

					PropSheet_SetCurSel (GetParent (hwnd), 
											NULL, ADDWIZ_HOSTTYPE);
					SetWindowLong (hwnd, DWL_MSGRESULT, -1L);
				}
				else
					SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE :
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	____________________________________
//
//	initialize the list view control

static VOID
sInitKeyLists (
		HWND			hwnd,
		PADDHOSTSTRUCT	pahs)
{
	HBITMAP			hBmp; 
	HDC				hDC;
	INT				iNumBits;
	LV_COLUMN		lvc;
	RECT			rc;

	// Initialize the tree view window.
	// First create imagelist and load the appropriate bitmaps based on 
	// current display capabilities.
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		pahs->hil =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG4));
		ImageList_AddMasked (pahs->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		pahs->hil =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_KEY_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_KEYIMG24));
		ImageList_AddMasked (pahs->hil, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	// Associate the image list with the tree view control.
	ListView_SetImageList (
			GetDlgItem (hwnd, IDC_PGPKEYLIST), pahs->hil, LVSIL_SMALL);

	// add columns
	GetClientRect (GetDlgItem (hwnd, IDC_PGPKEYLIST), &rc);
	lvc.mask = LVCF_WIDTH;
	lvc.cx = rc.right - rc.left;
	ListView_InsertColumn (GetDlgItem (hwnd, IDC_PGPKEYLIST), 0, &lvc);
}


//	____________________________________
//
//	put name of key into listview

static VOID
sSetKeyLists (
		HWND			hwnd,
		PADDHOSTSTRUCT	pahs)
{
	LV_ITEM			lvi;
	CHAR			sz[kPGPMaxUserIDSize +1];
	CHAR			szID[kPGPMaxKeyIDStringSize +1];
	UINT			u;
	PGPKeyID		keyid;
	PGPError		err;
	PGPSize			size;
	PGPKeyRef		keyPGP;
	PGPKeyRef		keyX509;
	PGPSigRef		sigX509;
	HICON			hIcon;
	HWND			hwndList;

	hwndList = GetDlgItem (hwnd, IDC_PGPKEYLIST);

	ListView_DeleteAllItems (hwndList);
	SetDlgItemText (hwnd, IDC_X509KEYEDIT, "");

	PGPnetGetConfiguredAuthKeys (g_context, &pahs->pncfg, g_AOS.keysetMain, 
				&keyPGP, &keyX509, &sigX509);

	lvi.mask = LVIF_TEXT|LVIF_IMAGE;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = sz;
	sz[0] = '\0';

	// first the PGP key
	if (pahs->pncfg.uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyPGP))
		{
			lvi.iImage = PNDetermineKeyIcon (keyPGP, NULL);
			sz[0] = '\0';
			PGPGetPrimaryUserIDNameBuffer (keyPGP, sizeof(sz), sz, &u);
		}
		else
		{
			lvi.iImage = IDX_NONE;
			err = PGPImportKeyID (pahs->pncfg.expkeyidPGPAuthKey, &keyid);
			if (IsntPGPError (err))
			{
				LoadString (g_hinst, IDS_UNKNOWNKEY, sz, sizeof(sz));
				PGPGetKeyIDString (&keyid, 
						kPGPKeyIDString_Abbreviated, szID);
				lstrcat (sz, szID);
			}
		}
		ListView_InsertItem (hwndList, &lvi);
		InvalidateRect (hwndList, NULL, FALSE);
	}

	// then get X509 auth key
	if (pahs->pncfg.uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyX509) &&
			PGPSigRefIsValid (sigX509))
		{
			sz[0] = '\0';
			PGPGetSigPropertyBuffer (sigX509,
						kPGPSigPropX509LongName, sizeof(sz), sz, &size);
			SetDlgItemText (hwnd, IDC_X509KEYEDIT, sz);

			u = PNDetermineCertIcon (sigX509, NULL);
			hIcon = ImageList_GetIcon (pahs->hil, u, ILD_TRANSPARENT);

			SendDlgItemMessage (
						hwnd, IDC_X509ICON, STM_SETICON, (WPARAM)hIcon, 0);		
		}
		else
		{
			err = PGPImportKeyID (pahs->pncfg.expkeyidX509AuthKey, &keyid);
			if (IsntPGPError (err))
			{
				LoadString (g_hinst, IDS_UNKNOWNCERT, sz, sizeof(sz));
				PGPGetKeyIDString (&keyid, 
						kPGPKeyIDString_Abbreviated, szID);
				lstrcat (sz, szID);
				SetDlgItemText (hwnd, IDC_X509KEYEDIT, sz);
			}
		}
	}
}


//	______________________________________________
//
//	Dialog procedure for asking user to set authentication keys

static LRESULT WINAPI 
sAddWizAuthKeysDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE*	ppspMsgRec	= (PROPSHEETPAGE *) lParam;

		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

#if !PGP_FREEWARE
		ShowWindow (GetDlgItem (hwnd, IDC_X509GROUP), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509BORDER), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509ICON), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_X509KEYEDIT), SW_SHOW);
		ShowWindow (GetDlgItem (hwnd, IDC_SELECTX509CERT), SW_SHOW);
		EnableWindow (GetDlgItem (hwnd, IDC_SELECTX509CERT), TRUE);
#endif // !PGP_FREEWARE

		// initialize controls
		sInitKeyLists (hwnd, pahs);
		PostMessage (hwnd, WM_APP, 0, 0);
		break;
	}

	case WM_APP :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (g_bAuthAccess)
		{
			sSetKeyLists (hwnd, pahs); 
		}
		break;

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_DESTROY :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		ImageList_Destroy (pahs->hil);
		break;

	case WM_COMMAND :
	{
		PGPError	err;
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD(wParam)) {
		case IDC_SELECTPGPKEY :
			err = PNSelectPGPKey (g_context, g_AOS.keysetMain, hwnd,
						&pahs->pncfg.uPGPAuthKeyAlg, 
						pahs->pncfg.expkeyidPGPAuthKey);

			PostMessage (hwnd, WM_APP, 0, 0);
			break;

		case IDC_SELECTX509CERT :
		{
			PGPKeyRef	key;
			PGPSigRef	sig;

			err = PGPclSelectX509Cert (g_context, hwnd, NULL,
						g_AOS.keysetMain, 
						PGPCL_CANSIGNKEYSONLY|PGPCL_NOSPLITKEYS,
						&key, &sig);

			if (IsntPGPError (err))
			{
				if (IsntNull (pahs->pncfg.pX509AuthCertIASN))
					PGPFreeData (pahs->pncfg.pX509AuthCertIASN);

				PGPX509CertToExport (g_context, key, sig,
						&(pahs->pncfg.uX509AuthKeyAlg), 
						pahs->pncfg.expkeyidX509AuthKey,
						&(pahs->pncfg.pX509AuthCertIASN),
						&(pahs->pncfg.uX509AuthCertIASNLength));
			}

			PostMessage (hwnd, WM_APP, 0, 0);
			break;
		}
		}
		break;
	}

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
		{
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);

			if (!g_bAuthAccess)
			{
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L); // skip this panel
			}
			else if (pahs->bNeedsAuthKey ||
					(pahs->bSecure && !pahs->bSharedSecret))
			{
				if (pahs->bFirstTime && 
					(pahs->uHostType != WIZ_GATEWAY)) 
				{
					PostMessage	(GetParent (hwnd), 
							PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_NEXT);
				}
				else
				{
					PostMessage	(GetParent (hwnd), 
							PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
				}

				if (!g_AOS.bPGPKey && !g_AOS.bX509Key)
				{
					SetWindowLong (hwnd, DWL_MSGRESULT, 0); 
				}
				else
				{
					SetWindowLong (hwnd, DWL_MSGRESULT, -1L);  //skip
				}
			}
			else 
			{
				SetWindowLong (hwnd, DWL_MSGRESULT, -1L); // skip this panel
			}
			bReturnCode = TRUE;
			break;
		}

		case PSN_WIZNEXT :
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			if (IsDlgButtonChecked (hwnd, IDC_CREATEMORE) == BST_CHECKED)
			{
			}
			else
				SetWindowLong (hwnd, DWL_MSGRESULT, 0L);

			bReturnCode = TRUE;
			break;

		case PSN_KILLACTIVE :
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
//  Dialog procedure for handling final dialog

static LRESULT WINAPI 
sAddWizDoneDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	BOOL			bReturnCode = FALSE;
	PADDHOSTSTRUCT	pahs		= NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
	{
		PROPSHEETPAGE *ppspMsgRec = (PROPSHEETPAGE *) lParam;
		pahs = (PADDHOSTSTRUCT) ppspMsgRec->lParam;
		SetWindowLong (hwnd, GWL_USERDATA, (LPARAM)pahs);

		break;
	}

	case WM_ACTIVATE :
		InvalidateRect (hwnd, NULL, TRUE);
		break;

	case WM_PAINT :
		pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if (pahs->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwnd, &ps);
			SelectPalette (hDC, pahs->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwnd, &ps);
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR) lParam;

		switch(pnmh->code) {
		case PSN_SETACTIVE:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			PostMessage	(GetParent (hwnd), 
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK|PSWIZB_FINISH);
			SendDlgItemMessage (hwnd, IDC_WIZBITMAP, STM_SETIMAGE, 
					IMAGE_BITMAP, (LPARAM) pahs->hBitmap);
			SetWindowLong (hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_WIZFINISH:
			pahs = (PADDHOSTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			pahs->bFinished = TRUE;
			SetWindowLong(hwnd, DWL_MSGRESULT, 0L);
			bReturnCode = TRUE;
			break;

		case PSN_QUERYCANCEL:
			break;
		}
		
		break;
	}
	}

	return bReturnCode;
}


//	______________________________________________
//
// Load DIB bitmap and associated palette

static HPALETTE 
sCreateDIBPalette (
		LPBITMAPINFO	lpbmi, 
		LPINT			lpiNumColors) 
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8) {
		*lpiNumColors = (1 << lpbi->biBitCount);
	}
	else
		*lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
	if (*lpiNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;
 
		for (i = 0;  i < *lpiNumColors;  i++) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette (lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree (hLogPal);
   }
   return hPal;
}


static HBITMAP 
sLoadResourceBitmap (
		HINSTANCE	hInstance, 
		LPSTR		lpString,
		HPALETTE*	phPalette) 
{
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;
	HDC hdc;
    INT iNumColors;
 
	if (hRsrc = FindResource (hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource (hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)LockResource (hGlobal);
 
		hdc = GetDC(NULL);
		*phPalette =  sCreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
		if (*phPalette) {
			SelectPalette (hdc,*phPalette,FALSE);
			RealizePalette (hdc);
		}
 
		hBitmapFinal = CreateDIBitmap (hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
                   (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
 
		ReleaseDC (NULL,hdc);
		UnlockResource (hGlobal);
		FreeResource (hGlobal);
	}
	return (hBitmapFinal);
}


//	____________________________________
//
//	post the add host wizard

BOOL
PNAddHostWizard (
		HWND		hwndParent,
		PPNCONFIG	ppnconfig,
		INT			iSelectedHost)
{
	BOOL				bReturn					= FALSE;

	ADDHOSTSTRUCT		ahs;
	PGPNetHostEntry		hostEntry;
	PROPSHEETPAGE		psp[NUM_WIZ_PAGES];
	PROPSHEETHEADER		psh;
	HDC					hDC;
	INT					i, iNumBits, iBitmap;

	hostEntry.ipAddress			= 0;
	hostEntry.ipMask			= 0;
	hostEntry.hostName[0]		= '\0';
	hostEntry.identityType		= kPGPike_ID_IPV4_Addr;
	hostEntry.identity[0]		= '\0';
	hostEntry.identityIPAddress	= 0;
	hostEntry.sharedSecret[0]	= '\0';
	hostEntry.authKeyAlg		= kPGPPublicKeyAlgorithm_Invalid;
	hostEntry.authCertIASNLength= 0; 

	ahs.context					= g_context;
	ahs.hwndThreadParent		= NULL;
	ahs.iStatusDirection		= 1;
	ahs.iStatusValue			= -1;
	ahs.bIPValid				= FALSE;
	ahs.bSubnetValid			= FALSE;
	ahs.szHostName[0]			= '\0';
	ahs.hwndHostIPAddress		= NULL;
	ahs.hwndHostIPAddressLookup	= NULL;
	ahs.dwFoundIPAddress		= 0;
	ahs.pheNew					= &hostEntry;
	ahs.ppnconfig				= ppnconfig;
	ahs.iSelectedHost			= iSelectedHost;
	ahs.bBehindGateway			= FALSE;
	ahs.uHostType				= WIZ_HOST;
	ahs.bHideTyping				= TRUE;
	ahs.bSharedSecret			= FALSE;
	ahs.bFirstTime				= TRUE;
	ahs.bFinished				= FALSE;

	ahs.bNeedsAuthKey			= FALSE;
	pgpCopyMemory (ppnconfig, &ahs.pncfg, sizeof(PNCONFIG));
	if (ahs.pncfg.uX509AuthCertIASNLength > 0)
	{
		ahs.pncfg.pX509AuthCertIASN = 
				PGPNewData (PGPGetContextMemoryMgr (g_context),
							ahs.pncfg.uX509AuthCertIASNLength, 
							kPGPMemoryMgrFlags_Clear);

		pgpCopyMemory (	ppnconfig->pX509AuthCertIASN, 
						ahs.pncfg.pX509AuthCertIASN,
						ahs.pncfg.uX509AuthCertIASNLength);
	}
	else
		ahs.pncfg.pX509AuthCertIASN = NULL;

	// Determine which bitmap will be displayed in the wizard
	hDC = GetDC (NULL);
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1)
		iBitmap = IDB_ADDWIZ1;
	else if (iNumBits <= 4) 
		iBitmap = IDB_ADDWIZ4;
	else 
		iBitmap = IDB_ADDWIZ8;

	ahs.hBitmap = sLoadResourceBitmap (g_hinst, 
									  MAKEINTRESOURCE (iBitmap),
									  &ahs.hPalette);

	// Set the values common to all pages
	for (i=0; i<NUM_WIZ_PAGES; i++)
	{
		psp[i].dwSize		= sizeof(PROPSHEETPAGE);
		psp[i].dwFlags		= PSP_DEFAULT;
		psp[i].hInstance	= g_hinst;
		psp[i].pszTemplate  = NULL;
		psp[i].hIcon		= NULL;
		psp[i].pszTitle		= NULL;
		psp[i].pfnDlgProc   = NULL;
		psp[i].lParam		= (LPARAM) &ahs;
		psp[i].pfnCallback  = NULL;
		psp[i].pcRefParent  = NULL;
	}

	// Set up the intro page
	psp[ADDWIZ_INTRO].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_INTRO);
	psp[ADDWIZ_INTRO].pfnDlgProc	
							= sAddWizIntroDlgProc;
	
	psp[ADDWIZ_BEHINDGATEWAY].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_BEHINDGATEWAY);
	psp[ADDWIZ_BEHINDGATEWAY].pfnDlgProc	
							= sAddWizBehindGatewayDlgProc;
	
	psp[ADDWIZ_HOSTTYPE].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_HOSTTYPE);
	psp[ADDWIZ_HOSTTYPE].pfnDlgProc	
							= sAddWizHostTypeDlgProc;
	
	psp[ADDWIZ_SECURE].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_SECURE);
	psp[ADDWIZ_SECURE].pfnDlgProc	
							= sAddWizSecureDlgProc;
	
	psp[ADDWIZ_HOSTNAME].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_HOSTNAME);
	psp[ADDWIZ_HOSTNAME].pfnDlgProc	
							= sAddWizHostNameDlgProc;
	
	psp[ADDWIZ_SUBNETADDRESS].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_SUBNETADDRESS);
	psp[ADDWIZ_SUBNETADDRESS].pfnDlgProc	
							= sAddWizSubnetAddressDlgProc;
		
	psp[ADDWIZ_HOSTADDRESS].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_HOSTADDRESS);
	psp[ADDWIZ_HOSTADDRESS].pfnDlgProc	
							= sAddWizHostAddressDlgProc;
		
	psp[ADDWIZ_ADDRESSLOOKUP].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_ADDRESSLOOKUP);
	psp[ADDWIZ_ADDRESSLOOKUP].pfnDlgProc	
							= sAddWizAddressLookupDlgProc;
		
	psp[ADDWIZ_USESHAREDSECRET].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_USESHAREDSECRET);
	psp[ADDWIZ_USESHAREDSECRET].pfnDlgProc	
							= sAddWizUseSharedSecretDlgProc;
	
	psp[ADDWIZ_SHAREDSECRET].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_SHAREDSECRET);
	psp[ADDWIZ_SHAREDSECRET].pfnDlgProc	
							= sAddWizSharedSecretDlgProc;
	
	psp[ADDWIZ_IDENTITYTYPE].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_IDENTITYTYPE);
	psp[ADDWIZ_IDENTITYTYPE].pfnDlgProc	
							= sAddWizIdentityTypeDlgProc;
	
	psp[ADDWIZ_IDENTITY].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_IDENTITY);
	psp[ADDWIZ_IDENTITY].pfnDlgProc	
							= sAddWizIdentityDlgProc;
	
	psp[ADDWIZ_ADDMORE].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_ADDMORE);
	psp[ADDWIZ_ADDMORE].pfnDlgProc	
							= sAddWizAddMoreDlgProc;
	
	psp[ADDWIZ_AUTHKEYS].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_AUTHKEYS);
	psp[ADDWIZ_AUTHKEYS].pfnDlgProc	
							= sAddWizAuthKeysDlgProc;
	
	psp[ADDWIZ_DONE].pszTemplate	
							= MAKEINTRESOURCE(IDD_ADDWIZ_DONE);
	psp[ADDWIZ_DONE].pfnDlgProc		
							= sAddWizDoneDlgProc;
	

	// Create the header
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_WIZARD | PSH_PROPSHEETPAGE;
	psh.hwndParent	= hwndParent;
	psh.hInstance	= g_hinst;
	psh.hIcon		= NULL;
	psh.pszCaption	= NULL;
	psh.nPages		= NUM_WIZ_PAGES;
	psh.nStartPage	= ADDWIZ_INTRO;
	psh.ppsp		= psp;
	psh.pfnCallback	= NULL;


	// Execute the Wizard - doesn't return until Cancel or Save
	PropertySheet (&psh);


	if (ahs.bFinished)
	{
		sSaveHostEntry (&ahs);

		// copy authentication keys stuff
		ppnconfig->uPGPAuthKeyAlg			= ahs.pncfg.uPGPAuthKeyAlg;
		ppnconfig->uX509AuthKeyAlg			= ahs.pncfg.uX509AuthKeyAlg;
		ppnconfig->uX509AuthCertIASNLength	= ahs.pncfg.uX509AuthCertIASNLength;
		pgpCopyMemory (ahs.pncfg.expkeyidPGPAuthKey, 
						ppnconfig->expkeyidPGPAuthKey,
						kPGPMaxExportedKeyIDSize);
		pgpCopyMemory (ahs.pncfg.expkeyidX509AuthKey, 
						ppnconfig->expkeyidX509AuthKey,
						kPGPMaxExportedKeyIDSize);

		if (ahs.pncfg.uX509AuthCertIASNLength > 0)
		{
			if (IsntNull (ppnconfig->pX509AuthCertIASN))
				PGPFreeData (ppnconfig->pX509AuthCertIASN);
			ppnconfig->pX509AuthCertIASN = ahs.pncfg.pX509AuthCertIASN;
			ahs.pncfg.pX509AuthCertIASN = NULL;
		}

		PNSetAuthKeyFlags (
			g_context, ppnconfig, &g_AOS.bPGPKey, &g_AOS.bX509Key);
		
		bReturn = TRUE;
	}
	else
	{
		if (!ahs.bFirstTime)
			bReturn = TRUE;
	}

	if (IsntNull (ahs.pncfg.pX509AuthCertIASN))
		PGPFreeData (ahs.pncfg.pX509AuthCertIASN);

	return bReturn;
}
