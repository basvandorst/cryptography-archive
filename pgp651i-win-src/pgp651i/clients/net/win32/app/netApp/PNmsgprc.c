/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPmsgprc.c - PGPnet main window message procedure
	

	$Id: PNmsgprc.c,v 1.72.4.2 1999/06/08 16:32:07 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPnetApp.h"

#include "pgpSDKPrefs.h"

#include "PGPcl.h"
#include "pgpNetIPC.h"
#include "pgpNetLog.h"
#include "pgpNetPaths.h"
#include "PGPnetHelp.h"

#define STATUS_STATUS_PANE		0
#define STATUS_NUMSAS_PANE		1
#define STATUS_NUM_PANES		2
#define STATUS_HEIGHT			20
#define STATUS_SA_WIDTH			200
#define IDC_STATUSBAR			5001

#define SERVICETIMER_ID			4321
#define SERVICETIMER_MS			11000
#define STARTUPTIMER_ID			4322
#define STARTUPTIMER_MS			200
#define OPTIONSTIMER_ID			4323
#define OPTIONSTIMER_MS			200

extern HINSTANCE		g_hinst;
extern INT				g_iMinWindowWidth;

extern HWND				g_hwndTab;
extern HWND				g_hwndMain;
extern HWND				g_hwndCurrentDlg;
extern HWND				g_hwndOptions;
extern BOOL				g_bAuthAccess;	

extern PGPContextRef	g_context;
extern INT				g_iCurrentPage;
extern APPOPTIONSSTRUCT	g_AOS;
extern UINT				g_uReloadKeyringMsg;
extern CHAR				g_szHelpFile[];

HWND	g_hwndLogDlg		= NULL;
HWND	g_hwndStatusDlg		= NULL;
HWND	g_hwndHostDlg		= NULL;
HWND	g_hwndStatusBar		= NULL;
INT		g_iStatusBarHeight	= 0;
INT		g_iSACount			= 0;
BOOL	g_bGUIDisabled		= FALSE;
BOOL	g_bWindowShown		= FALSE;
HWND	g_hwndActive		= NULL;
HWND	g_hwndForeground	= NULL;
INT		g_iLogonStatus		= PGPNET_LOGGEDOFF;

#define IDC_MAINTABCONTROL		41000

static DWORD aMainIds[] = {			// Help IDs
	IDC_MAINTABCONTROL,	IDH_PNMAIN_TABS,
	0,0
};

//	____________________________________
//
//	paint the line to separate menu from tab control

static VOID 
sPaintDividerLine (HWND hwnd)
{
	HDC			hdc;
	RECT		rc;
	PAINTSTRUCT ps;

	GetClientRect (hwnd, &rc);
	hdc = BeginPaint (hwnd, &ps);

	DrawEdge (hdc, &rc, EDGE_ETCHED, BF_TOP);

	EndPaint (hwnd, &ps);
}

//	____________________________________
//
//	create the tab control and button controls

static VOID 
sCreateControls (HWND	hwndParent)
{
	RECT	rc;     
	TCITEM	tie;     
	CHAR	sz[256];
	HFONT	hfont;
	INT		iWidths[STATUS_NUM_PANES];


	// first, create the status bar
	g_hwndStatusBar = CreateStatusWindow (
			WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,
			"", hwndParent, IDC_STATUSBAR);
	iWidths[0] = STATUS_SA_WIDTH;
	iWidths[STATUS_NUM_PANES-1] = -1;
	SendMessage (g_hwndStatusBar, SB_SETPARTS, STATUS_NUM_PANES, 
					(LPARAM)iWidths);
	GetWindowRect (g_hwndStatusBar, &rc);
	g_iStatusBarHeight = rc.bottom - rc.top;

	// Get the dimensions of the parent window's client area, and 
	// create a tab control child window of that size. 
	GetClientRect (hwndParent, &rc);
	rc.bottom -= g_iStatusBarHeight;

	g_hwndTab = CreateWindow (
		WC_TABCONTROL, "", 
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		HOR_TAB_OFFSET, TOP_TAB_OFFSET, 
		rc.right-(2*HOR_TAB_OFFSET), 
		rc.bottom - TOP_TAB_OFFSET - BOTTOM_TAB_OFFSET, 
		hwndParent, (HMENU)IDC_MAINTABCONTROL, g_hinst, NULL);

	if (g_hwndTab != NULL) 
	{
		// The default tab font is UGLY, so use another
		hfont = GetStockObject (DEFAULT_GUI_FONT);
		SendMessage (g_hwndTab, WM_SETFONT, (WPARAM)hfont, 0);

		// Add the tabs
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1; 
		tie.pszText = sz;

		LoadString (g_hinst, IDS_LOG, sz, sizeof(sz)); 
		if (TabCtrl_InsertItem (g_hwndTab, PGPNET_LOGPAGE, &tie) == -1)
			DestroyWindow (g_hwndTab);

		LoadString (g_hinst, IDS_STATUS, sz, sizeof(sz)); 
		if (TabCtrl_InsertItem (g_hwndTab, PGPNET_STATUSPAGE, &tie) == -1)
			DestroyWindow (g_hwndTab);

		LoadString (g_hinst, IDS_HOSTS, sz, sizeof(sz)); 
		if (TabCtrl_InsertItem (g_hwndTab, PGPNET_HOSTPAGE, &tie) == -1)
			DestroyWindow (g_hwndTab);

		TabCtrl_SetCurSel (g_hwndTab, g_iCurrentPage);
		g_iCurrentPage = -1;
	}

	// create the child dialogs
	g_hwndStatusDlg = CreateDialog (g_hinst, 
						MAKEINTRESOURCE(IDD_STATUS), hwndParent,
						PNStatusDlgProc);
	g_hwndLogDlg	= CreateDialog (g_hinst, 
						MAKEINTRESOURCE(IDD_LOG), hwndParent,
						PNLogDlgProc);
	g_hwndHostDlg	= CreateDialogParam (g_hinst, 
						MAKEINTRESOURCE(IDD_HOSTOPTIONS), hwndParent,
						PNHostDlgProc, (LPARAM)&g_AOS);
}


//	____________________________________
//
//	size the tab control and the contained dialog

static VOID
sSizeControls (
		WORD	wXsize,
		WORD	wYsize)
{
	HDWP	hdwp; 
	RECT	rc;

	// set the rect to the desired coords of the tab control
	SetRect (&rc, 
			HOR_TAB_OFFSET, 
			TOP_TAB_OFFSET, 
			wXsize - HOR_TAB_OFFSET,
			wYsize - BOTTOM_TAB_OFFSET - g_iStatusBarHeight); 

	// size the tab control accordingly
	hdwp = BeginDeferWindowPos (2); 
	DeferWindowPos (hdwp, g_hwndTab, NULL, 
		rc.left, rc.right, 
		rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOMOVE|SWP_NOZORDER);  

	// calculate the display area from the tab control window size
	TabCtrl_AdjustRect (g_hwndTab, FALSE, &rc); 
	
	// size the contained dialog box accordingly
	DeferWindowPos (hdwp, 
		g_hwndCurrentDlg, NULL, 
		rc.left, rc.top, 
		rc.right-rc.left, rc.bottom-rc.top, 0); 

	EndDeferWindowPos(hdwp);
}

//	____________________________________
//
//	create and activate new dialog page

static VOID
sNewPage (HWND hwndMain, HWND hwndTab)
{
	RECT	rc;
	INT		iNewPage = TabCtrl_GetCurSel (hwndTab);

	if (iNewPage == g_iCurrentPage) 
		return;

	// hide current dialog
	if (g_hwndCurrentDlg)
		ShowWindow (g_hwndCurrentDlg, SW_HIDE);

	// select new dialog
	g_iCurrentPage = iNewPage;

	switch (iNewPage) {
	case PGPNET_STATUSPAGE :
		g_hwndCurrentDlg = g_hwndStatusDlg;
		break;

	case PGPNET_LOGPAGE :
		g_hwndCurrentDlg = g_hwndLogDlg;
		break;

	case PGPNET_HOSTPAGE :
		g_hwndCurrentDlg = g_hwndHostDlg;
		break;
	}

	// initialize the size of the dialog to fit the tab control
	GetClientRect (hwndMain, &rc);
	sSizeControls ((WORD)rc.right, (WORD)rc.bottom);

	// visualize world peace (and the dialog)
	ShowWindow (g_hwndCurrentDlg, SW_SHOW);
	SendMessage (g_hwndCurrentDlg, PGPNET_M_ACTIVATEPAGE, 0, 0); 
	UpdateWindow (g_hwndCurrentDlg);

	SetFocus (hwndTab);
}

//	____________________________________
//
//	Put PGPnet application window position into registry

static VOID 
sPutWindowPosToRegistry (
		HWND	hwnd)
{	
	WINDOWPLACEMENT	wp;
	HKEY			hKey;
	LONG			lResult;
	DWORD			dw;
	CHAR			sz[256];
	INT				iX, iY, iWidth, iHeight;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement (hwnd, &wp);

	iX = wp.rcNormalPosition.left;
	iY = wp.rcNormalPosition.top;
	iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

	LoadString (g_hinst, IDS_PGPNETREGKEY, sz, sizeof(sz));
	lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, sz, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

	if (lResult == ERROR_SUCCESS) {
		LoadString (g_hinst, IDS_WINDOWPOS, sz, sizeof(sz));
		dw = MAKELONG (iX, iY);
		RegSetValueEx (hKey, sz, 0, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

		LoadString (g_hinst, IDS_WINDOWSIZE, sz, sizeof(sz));
		dw = MAKELONG (iWidth, iHeight);
		RegSetValueEx (hKey, sz, 0, REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

		RegCloseKey (hKey);
	}
}

//	____________________________________
//
//  display status bar text

UINT
PNDisplayStatusBarText (VOID)
{
	CHAR	sz1[64];
	CHAR	sz2[64];
	UINT	uStatus;

	uStatus = PNGetServiceStatus (g_hwndMain);
	switch (uStatus) {
	case PGPNET_SERVICENOTRESPONDING :
		LoadString (g_hinst, IDS_STATUSSERVICENOTRESPOND, sz2, sizeof(sz2));
		break;

	case PGPNET_SERVICENOTAVAILABLE :
		LoadString (g_hinst, IDS_STATUSSERVICENOTAVAIL, sz2, sizeof(sz2));
		break;

	case PGPNET_DRIVERNOTAVAILABLE :
		LoadString (g_hinst, IDS_STATUSDRIVERNOTAVAIL, sz2, sizeof(sz2));
		break;

	case PGPNET_DRIVERDISABLED :
		LoadString (g_hinst, IDS_STATUSDRIVERDISABLED, sz2, sizeof(sz2));
		break;

	case PGPNET_DRIVERENABLED :
		switch (g_iLogonStatus) {
		case PGPNET_LOGGEDOFF :
			LoadString (g_hinst, IDS_STATUSLOGGEDOFF, sz2, sizeof(sz2));
			break;

		case PGPNET_LOGGEDON :
			LoadString (g_hinst, IDS_STATUSLOGGEDON, sz2, sizeof(sz2));
			break;

		case PGPNET_NOLOGONREQUIRED :
			LoadString (g_hinst, IDS_STATUSNOLOGONREQUIRED, sz2, sizeof(sz2));
			break;

		case PGPNET_LOGONINPROCESS :
			LoadString (g_hinst, IDS_STATUSLOGONINPROCESS, sz2, sizeof(sz2));
			break;
		}
		break;
	}
	SendMessage (g_hwndStatusBar, SB_SETTEXT, 
						STATUS_STATUS_PANE, (LPARAM)sz2);


	LoadString (g_hinst, IDS_NUMSASFORMAT, sz2, sizeof(sz2));
	wsprintf (sz1, sz2, g_iSACount);
	SendMessage (g_hwndStatusBar, SB_SETTEXT, 
						STATUS_NUMSAS_PANE, (LPARAM)sz1);

	return uStatus;
}

VOID
PNSetStatusSACount (
		INT		iCount)
{
	g_iSACount = iCount;
	PNDisplayStatusBarText ();
}

VOID
PNIncStatusSACount (VOID)
{
	g_iSACount++;
	PNDisplayStatusBarText ();
}

VOID
PNDecStatusSACount (VOID)
{
	g_iSACount--;
	PNDisplayStatusBarText ();
}

//	____________________________________
//
//	relay the WM_COPYDATA message to the appropriate window

static LRESULT 
sRelayCopyDataMessage (
		WPARAM		wParam,
		LPARAM		lParam)
{	
	PCOPYDATASTRUCT	pcds = (PCOPYDATASTRUCT)lParam;

	switch (pcds->dwData) {
	case PGPNET_LOGONSTATUS :
		g_iLogonStatus = *((DWORD*)(pcds->lpData));
		PNDisplayStatusBarText ();
		return TRUE;

	case PGPNET_STATUSFULL:
	case PGPNET_STATUSNEWSA :
	case PGPNET_STATUSREMOVESA :
		SendMessage (g_hwndStatusDlg, 
						PGPNET_M_INTERNALCOPYDATA, wParam, lParam);
		SendMessage (g_hwndHostDlg, 
						PGPNET_M_UPDATELIST, wParam, lParam);
		return TRUE;

	case PGPNET_LOGUPDATE :
		SendMessage (g_hwndLogDlg, 
						PGPNET_M_INTERNALCOPYDATA, wParam, lParam);
		return TRUE;
	}

	return FALSE;
}


//	____________________________________
//
//	display messagebox on basis of service/driver status

static UINT
sDisplayStatusMessageBox (
		HWND	hwnd)
{
	CHAR	sz1[128];
	CHAR	sz2[128];
	UINT	uStatus;

	uStatus = PNDisplayStatusBarText ();

	switch (uStatus) {
	case PGPNET_DRIVERENABLED :
	case PGPNET_DRIVERDISABLED :
		return uStatus;

	case PGPNET_SERVICENOTRESPONDING :
	case PGPNET_SERVICENOTAVAILABLE :
		LoadString (g_hinst, IDS_SERVICENOTRUNNING, sz2, sizeof(sz2));
		break;

	case PGPNET_DRIVERNOTAVAILABLE :
		LoadString (g_hinst, IDS_DRIVERNOTAVAIL, sz2, sizeof(sz2));
		break;

	default :
		return uStatus;
	}

	LoadString (g_hinst, IDS_CAPTION, sz1, sizeof(sz1));
	MessageBox (hwnd, sz2, sz1, MB_OK|MB_ICONEXCLAMATION);

	return uStatus;
}


//	____________________________________
//
//	load application options

VOID
PNLoadAppOptions (
		HWND	hwnd,
		BOOL	bInitialTime)
{
	PGPKeyRef			keyPGP		= kInvalidPGPKeyRef;
	PGPKeyRef			keyX509		= kInvalidPGPKeyRef;
	PGPSigRef			sigX509		= kInvalidPGPSigRef;

	// first time initialization
	if (bInitialTime) 
	{
		pgpClearMemory (&g_AOS, sizeof(g_AOS));
		g_AOS.keysetMain = kInvalidPGPKeySetRef;
	}

	// open keyring files
	PGPnetLoadSDKPrefs (g_context);
	PNLoadKeyRings (hwnd, &g_AOS.keysetMain);

	// load config from prefs file
	PGPnetLoadConfiguration (g_context, &g_AOS.pnconfig);

	// if no hosts in list, pop the "add host" dialog/wizard
	if (g_AOS.pnconfig.bFirstExecution)
	{
		if (bInitialTime && 
			(g_AOS.pnconfig.uHostCount == 0))
		{
			PNAddHostEntry (hwnd, &g_AOS.pnconfig, FALSE, -1, NULL);
		}
		g_AOS.pnconfig.bFirstExecution = FALSE;
		PNSaveConfiguration (hwnd, &g_AOS.pnconfig, FALSE);
	}

	// determine if authentication keys are specified
	PNSetAuthKeyFlags (
			g_context, &g_AOS.pnconfig, &g_AOS.bPGPKey, &g_AOS.bX509Key);

	if (g_AOS.pnconfig.uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (!g_AOS.bPGPKey)
		{
			PNMessageBox (hwnd, IDS_CAPTION, IDS_MISSINGPGPKEY, 
							MB_ICONSTOP|MB_OK);
		}
	}

	if (g_AOS.pnconfig.uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (!g_AOS.bX509Key)
		{
			PNMessageBox (hwnd, IDS_CAPTION, IDS_MISSINGX509KEY, 
							MB_ICONSTOP|MB_OK);
		}
	}
}


//	____________________________________
//
//	window proc of main window

LRESULT CALLBACK 
PGPnetWndProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{

	switch (uMsg)
	{
	case WM_CREATE:
		if (g_iCurrentPage == PGPNET_OPTIONSHEET)
		{
			if (!g_hwndOptions)
			{
				PNLoadAppOptions (hwnd, TRUE);
				PNOptionsPropSheet (hwnd, PGPNET_GENOPTIONS);
				PGPnetFreeConfiguration (&g_AOS.pnconfig);
			}
			else
				SetForegroundWindow (g_hwndOptions);
			return -1;
		}

		PostMessage (hwnd, WM_APP, PGPNET_STARTUP, 0);
		SetTimer (hwnd, SERVICETIMER_ID, SERVICETIMER_MS, NULL);
		return 0;

	case WM_APP :
		switch (wParam) 
		{
		case PGPNET_STARTUP :
			PNLoadAppOptions (hwnd, TRUE);

			sCreateControls (hwnd);
			sNewPage (hwnd, g_hwndTab);
			g_AOS.hwndStatusDlg = g_hwndStatusDlg;

			// delay to allow service time to disable GUI 
			SetTimer (hwnd, STARTUPTIMER_ID, STARTUPTIMER_MS, NULL);
			break;

		case PGPNET_SHOWWINDOW :
			g_bWindowShown = TRUE;

			ShowWindow (hwnd, SW_SHOW); 
			UpdateWindow (hwnd); 
			EnableWindow (hwnd, TRUE);

			PNSendServiceMessage (PGPNET_M_APPMESSAGE,
						(WPARAM)PGPNET_QUERYLOGONSTATUS, (LPARAM)hwnd);

			PNDisplayStatusBarText ();
			sDisplayStatusMessageBox (hwnd);

			if (lParam)
			{
				TabCtrl_SetCurSel (g_hwndTab, PGPNET_HOSTPAGE);
				sNewPage (hwnd, g_hwndTab);
			}

			if (!g_bAuthAccess)
				PNMessageBox (hwnd, IDS_CAPTION, IDS_NOAUTHPRIVS, 
									MB_OK|MB_ICONEXCLAMATION);
			break;

		case PGPNET_PAGESELECT :
			if (g_hwndOptions)
				return ((LRESULT)g_hwndOptions);
			else
			{
				if (lParam == PGPNET_OPTIONSHEET)
					SetTimer (hwnd, OPTIONSTIMER_ID, OPTIONSTIMER_MS, NULL);
				else
				{
					TabCtrl_SetCurSel (g_hwndTab, lParam);
					sNewPage (hwnd, g_hwndTab);
				}
				return ((LRESULT)hwnd);
			}
			break;
		}
		break;

	case PGPNET_M_APPMESSAGE :
		switch (wParam) {
		case PGPNET_CLEARLOGFILE :
			SendMessage (g_hwndLogDlg, uMsg, wParam, lParam);
			break;

		case PGPNET_DISABLEGUI :
			if (!g_bGUIDisabled)
			{
				HWND	hwndForeground;

				g_bGUIDisabled = TRUE;

				if (g_bWindowShown)
				{
					hwndForeground = GetForegroundWindow ();
					if (hwndForeground)
						g_hwndForeground = hwndForeground;

					g_hwndActive = GetLastActivePopup (hwnd);
					if (!g_hwndActive)
						g_hwndActive = hwnd;

					if (g_hwndForeground != g_hwndActive)
						g_hwndForeground = NULL;

					// disable our own window
					EnableWindow (g_hwndActive, FALSE);
				}

				// relenquish the foreground window to the PGPnet service
				SetForegroundWindow ((HWND)lParam);
			}
			break;

		case PGPNET_ENABLEGUI :
			if (g_bGUIDisabled)
			{
				g_bGUIDisabled = FALSE;

				if (!g_bWindowShown)
					PostMessage (hwnd, WM_APP, PGPNET_SHOWWINDOW, TRUE);
				else
				{
					EnableWindow (g_hwndActive, TRUE);

					if (g_hwndForeground)
					{
						SetForegroundWindow (g_hwndForeground);
						g_hwndForeground = NULL;
					}
				}
			}
			break;

		case PGPNET_PROMISCUOUSHOSTADDED :
			SendMessage (g_hwndHostDlg, uMsg, wParam, lParam);
			break;
		}
		break;

	case WM_TIMER :
		if (wParam == SERVICETIMER_ID)
		{
			UINT uStatus = PNDisplayStatusBarText ();
			if ((uStatus == PGPNET_SERVICENOTRESPONDING) ||
				(uStatus == PGPNET_SERVICENOTAVAILABLE))
			{
				if (g_bGUIDisabled)
					EnableWindow (g_hwndActive, TRUE);
			}
		}
		else if (wParam == STARTUPTIMER_ID)
		{
			KillTimer (hwnd, STARTUPTIMER_ID);

			if (!g_bGUIDisabled)
				PostMessage (hwnd, WM_APP, PGPNET_SHOWWINDOW, FALSE);
		}
		else if (wParam == OPTIONSTIMER_ID)
		{
			KillTimer (hwnd, OPTIONSTIMER_ID);
			PostMessage (hwnd, WM_COMMAND, IDM_OPTIONS, 0);
		}
		break;

	case WM_SIZING :
		{
			LPRECT prc = (LPRECT)lParam;
			switch (wParam) {
			case WMSZ_TOPLEFT :
			case WMSZ_LEFT :
			case WMSZ_BOTTOMLEFT :
				if ((prc->right - prc->left) < g_iMinWindowWidth)
					prc->left = prc->right - g_iMinWindowWidth;
				break;

			case WMSZ_TOPRIGHT :
			case WMSZ_RIGHT :
			case WMSZ_BOTTOMRIGHT :
				if ((prc->right - prc->left) < g_iMinWindowWidth)
					prc->right = prc->left + g_iMinWindowWidth;
				break;
			}
		}
		break;

	case WM_SETFOCUS :
		SetFocus (g_hwndTab);
		break;

	case WM_PAINT :
		sPaintDividerLine (hwnd);
		return 0;

	case WM_SIZE :
		sSizeControls (LOWORD(lParam), HIWORD(lParam));
		SendMessage (g_hwndStatusBar, WM_SIZE, wParam, lParam);
		return 0;

	case WM_NOTIFY :
		{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code) {
			case TCN_SELCHANGE :
				sNewPage (hwnd, g_hwndTab);
				break;

			case TCN_KEYDOWN :
			{
				TC_KEYDOWN*	ptckd = (TC_KEYDOWN*)lParam;
				if (ptckd->wVKey == VK_TAB)
					SendMessage (g_hwndCurrentDlg, PGPNET_M_SETFOCUS, 0, 0);
				break;
			}
			}
		}
		break;

	case WM_COPYDATA :
		return sRelayCopyDataMessage (wParam, lParam);

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPSTR) aMainIds); 
		return TRUE; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aMainIds); 
		return TRUE; 

	case WM_COMMAND :
		switch (LOWORD (wParam)) {
		case IDM_OPTIONS :
			if (!g_hwndOptions)
			{
				PNOptionsPropSheet (hwnd, PGPNET_GENOPTIONS);
				SendMessage (g_hwndCurrentDlg, PGPNET_M_ACTIVATEPAGE, 0, 0);
			}
			break;

		case IDM_EXIT :
			PostMessage (hwnd, WM_CLOSE, 0, 0);
			break;

		case IDM_HELPABOUT :
			PGPclHelpAbout (g_context, hwnd, NULL, NULL, NULL);
			break;

		case IDM_STATUS :
			TabCtrl_SetCurSel (g_hwndTab, PGPNET_STATUSPAGE);
			sNewPage (hwnd, g_hwndTab);
			break;

		case IDM_LOG :
			TabCtrl_SetCurSel (g_hwndTab, PGPNET_LOGPAGE);
			sNewPage (hwnd, g_hwndTab);
			break;

		case IDM_HOSTS :
			TabCtrl_SetCurSel (g_hwndTab, PGPNET_HOSTPAGE);
			sNewPage (hwnd, g_hwndTab);
			break;

		case IDM_HELPCONTENTS :
			WinHelp (hwnd, g_szHelpFile, HELP_FINDER, 0);
			break;

		default :
			SendMessage (g_hwndCurrentDlg, uMsg, wParam, lParam);
			break;
		}
		return 0;

	case WM_CLOSE:
		WinHelp (hwnd, g_szHelpFile, HELP_QUIT, 0);
		sPutWindowPosToRegistry (hwnd);
		DestroyWindow (hwnd);
		return 0;

	case WM_DESTROY:
		KillTimer (hwnd, SERVICETIMER_ID);

		if (g_hwndLogDlg)
			DestroyWindow (g_hwndLogDlg);
		if (g_hwndStatusDlg)
			DestroyWindow (g_hwndStatusDlg);
		if (g_hwndTab)
			DestroyWindow (g_hwndTab);

		PGPnetFreeConfiguration (&g_AOS.pnconfig);
		if (PGPKeySetRefIsValid (g_AOS.keysetMain))
			PGPFreeKeySet (g_AOS.keysetMain);

		PostQuitMessage (0);
		return 0;

	default :
		if (uMsg == g_uReloadKeyringMsg)
		{
			PGPnetFreeConfiguration (&g_AOS.pnconfig);
			PNLoadAppOptions (hwnd, FALSE);

			g_AOS.bUpdateHostList = TRUE;
			SendMessage (g_hwndHostDlg, PGPNET_M_UPDATELIST, 0, 0);

			if (g_AOS.hwndAuthDlg != NULL)
			{
				PostMessage (
					g_AOS.hwndAuthDlg, PGPNET_M_RELOADKEYRINGS, 0, 0);
			}

			if (g_AOS.hwndHostDlg != NULL)
			{
				PostMessage (
					g_AOS.hwndHostDlg, PGPNET_M_RELOADKEYRINGS, 0, 0);
			}
			return 0;
		}
		break;
	}

	return DefWindowProc (hwnd, uMsg, wParam, lParam);
}


