/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: MainWndProc.cpp,v 1.12.8.1 1998/11/12 03:12:44 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "PGPcl.h"
#include "resource.h"

#define PLUGIN_INFO_PROP		"PGP plugin"

void CreateMainWindowGUI(HWND hDlg, PluginInfo *plugin);


BOOL CALLBACK MainWndProc(HWND hDlg, 
						  UINT msg,
						  WPARAM wParam, 
						  LPARAM lParam)
{
	DLGPROC lpOldProc;
	PluginInfo *plugin;

	lpOldProc = (DLGPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);
	
	if (plugin != NULL)
	{
		if (msg == plugin->nPurgeCacheMsg)
		{
			PGPclPurgeCachedPassphrase(wParam);
			return TRUE;
		}
	}

	switch(msg)
	{
	case WM_SETTINGCHANGE:
	case WM_PALETTECHANGED:
	case WM_FONTCHANGE:
	case WM_SYSCOLORCHANGE:
		{
			HINSTANCE hInst;
			HDC	hDC;
			TBBUTTON tbb[1];
			HIMAGELIST hilToolbar;
			HBITMAP hbmpPGPkeys;
			int nPGPkeysBMP;
			int nPGPkeysSTR;
			int	iNumBits;
			int nIndex;
			int nReturn;
			char szText[255];
			
			nReturn = CommonWndProc(hDlg, msg, wParam, lParam);

			hInst = UIGetInstance();
			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_PGPKEYS, 0);

			SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
			plugin->nPGPKeysButton = -1;

			hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
										TB_GETIMAGELIST, 0, 0);
			
			ImageList_Remove(hilToolbar, plugin->nPGPKeysImage);

			hDC = GetDC (NULL);		// DC for desktop
			iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * 
				GetDeviceCaps (hDC, PLANES);
			ReleaseDC (NULL, hDC);
			
			if (iNumBits <= 8) 
				hbmpPGPkeys = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_PGPKEYSOFF4));
			else
				hbmpPGPkeys = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_PGPKEYSOFF));
			
			nPGPkeysBMP = ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
							TRANSPARENT_COLOR);
			DeleteObject(hbmpPGPkeys);
			plugin->nPGPKeysImage = nPGPkeysBMP;
			
			SendMessage(plugin->hToolbar, TB_SETIMAGELIST, 0, 
				(LPARAM) hilToolbar);
			
			hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
										TB_GETHOTIMAGELIST, 0, 0);
			
			if (iNumBits <= 8)
				hbmpPGPkeys = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_PGPKEYSLARGE4));
			else
				hbmpPGPkeys = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_PGPKEYSLARGE));
			
			ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
				TRANSPARENT_COLOR);
			DeleteObject(hbmpPGPkeys);
			
			SendMessage(plugin->hToolbar, TB_SETHOTIMAGELIST, 0, 
				(LPARAM) hilToolbar);
			
			UIGetString(szText, 254, IDS_MENU_PGPKEYS);
			nPGPkeysSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
							(LPARAM) szText);
			plugin->nPGPKeysString = nPGPkeysSTR;
			
			tbb[0].iBitmap = nPGPkeysBMP;
			tbb[0].idCommand = IDC_PGPKEYS;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_BUTTON;
			tbb[0].dwData = 0;
			tbb[0].iString = nPGPkeysSTR;
			
			SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 1, 
				(LPARAM) &tbb);
			
			plugin->nPGPKeysButton = SendMessage(plugin->hToolbar,
										TB_COMMANDTOINDEX,
										IDC_PGPKEYS,
										0);
			return nReturn;
		}

	case WM_DESTROY:
		{
			KillTimer(hDlg, WATCH_TOOLBAR_TIMER);
			break;
		}

	case WM_PARENTNOTIFY:
		if (LOWORD(wParam) == WM_CREATE)
			CreateMainWindowGUI(hDlg, plugin);
		break;
	
	case WM_TIMER:
		if (wParam == WATCH_TOOLBAR_TIMER)
		{
			int nState;

			nState = SendMessage(plugin->hToolbar, TB_GETSTATE, 
						IDC_PGPKEYS, 0);

			if (nState & TBSTATE_ENABLED)
				return 0;

			nState |= TBSTATE_ENABLED;
			SendMessage(plugin->hToolbar, TB_SETSTATE, IDC_PGPKEYS, 
				MAKELONG(nState, 0));

			return 0;
		}
		break;

	case WM_INITMENUPOPUP:
		{
			// For some reason, the old window proc disables the
			// menu items in the PGP popup menu. We'll enable
			// the items after the old proc disables them

			if ((HMENU) wParam == plugin->hPGPMenu)
			{
				CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);
				EnableMenuItem(plugin->hPGPMenu, IDC_PREFS, MF_BYCOMMAND);
				EnableMenuItem(plugin->hPGPMenu, IDC_PGPKEYS, MF_BYCOMMAND);
				return 0;
			}

			break;
		}
	}

	return CommonWndProc(hDlg, msg, wParam, lParam);
}


void CreateMainWindowGUI(HWND hDlg, PluginInfo *plugin)
{
	HWND hReBar;
	HWND hSizableRebar;
	TBBUTTON tbb[1];
	HIMAGELIST hilToolbar;
	HINSTANCE hInst;
	int nPGPkeysBMP;
	int nPGPkeysSTR;
	int nIndex;
	HBITMAP hbmpPGPkeys;
	MENUITEMINFO menuInfo;
	char szText[255];
	DLGPROC lpOldProc;
	
	plugin->hMainMenu = GetMenu(hDlg);
	hSizableRebar = FindWindowEx(hDlg, NULL, "SizableRebar", NULL);
	hReBar = FindWindowEx(hSizableRebar, NULL, "ReBarWindow32", NULL);
	plugin->hToolbar = FindWindowEx(hReBar, NULL, "ToolbarWindow32", NULL);
	
	if (!plugin->hMainMenu || !plugin->hToolbar)
		return;
	
	hInst = UIGetInstance();
	
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_SUBMENU;
	GetMenuItemInfo(plugin->hMainMenu, 4, TRUE, &menuInfo);
	if (menuInfo.hSubMenu != plugin->hPGPMenu)
	{
		if (plugin->hPGPMenu != NULL)
			DestroyMenu(plugin->hPGPMenu);
		
		plugin->hPGPMenu = CreatePopupMenu();
		plugin->nPGPPosition = 4;
		
		InsertMenu(plugin->hMainMenu, plugin->nPGPPosition, 
			MF_BYPOSITION | MF_STRING | MF_POPUP, 
			(UINT) plugin->hPGPMenu, "&PGP");
		
		UIGetString(szText, 254, IDS_MENU_PREFS);
		AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PREFS, 
			szText);
		
		UIGetString(szText, 254, IDS_MENU_PGPKEYS);
		AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PGPKEYS, 
			szText);
		
		DrawMenuBar(hDlg);
	}
	
	SendMessage(plugin->hToolbar, TB_BUTTONSTRUCTSIZE,
		(WPARAM) sizeof(TBBUTTON), 0);
	
	nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
				IDC_PGPKEYS, 0);

	if (nIndex < 0)
	{
		HDC	hDC;
		int	iNumBits;
		
		hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
			TB_GETIMAGELIST, 0, 0);
		
		hDC = GetDC (NULL);		// DC for desktop
		iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * 
					GetDeviceCaps (hDC, PLANES);
		ReleaseDC (NULL, hDC);
		
		if (iNumBits <= 8) 
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSOFF4));
		else
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSOFF));

		nPGPkeysBMP = ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
			TRANSPARENT_COLOR);
		DeleteObject(hbmpPGPkeys);
		plugin->nPGPKeysImage = nPGPkeysBMP;

		SendMessage(plugin->hToolbar, TB_SETIMAGELIST, 0, 
			(LPARAM) hilToolbar);
		
		hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
			TB_GETHOTIMAGELIST, 0, 0);
		
		if (iNumBits <= 8)
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSLARGE4));
		else
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSLARGE));

		ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
			TRANSPARENT_COLOR);
		DeleteObject(hbmpPGPkeys);
		
		SendMessage(plugin->hToolbar, TB_SETHOTIMAGELIST, 0, 
			(LPARAM) hilToolbar);
		
		UIGetString(szText, 254, IDS_MENU_PGPKEYS);
		nPGPkeysSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
			(LPARAM) szText);
		plugin->nPGPKeysString = nPGPkeysSTR;
		
		tbb[0].iBitmap = nPGPkeysBMP;
		tbb[0].idCommand = IDC_PGPKEYS;
		tbb[0].fsState = TBSTATE_ENABLED;
		tbb[0].fsStyle = TBSTYLE_BUTTON;
		tbb[0].dwData = 0;
		tbb[0].iString = nPGPkeysSTR;
		
		SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 1, 
			(LPARAM) &tbb);
		
		plugin->nPGPKeysButton = SendMessage(plugin->hToolbar,
									TB_COMMANDTOINDEX,
									IDC_PGPKEYS,
									0);

		SetTimer(hDlg, WATCH_TOOLBAR_TIMER, 500, NULL);
	}
	
	// Look for status bar
	plugin->hStatusBar = FindWindowEx(hDlg, NULL, 
		"msctls_statusbar32", NULL);

	lpOldProc = (DLGPROC) GetProp(hReBar, "oldproc");
	if (lpOldProc == NULL)
	{
		// Save away toolbar parent window proc
		SetProp(hReBar, "oldproc", 
			(HANDLE) GetWindowLong(hReBar, GWL_WNDPROC)); 

		// Save plugin pointer
		SetProp(hReBar, PLUGIN_INFO_PROP, (HANDLE) plugin);

		// Subclass toolbar parent window
		SetWindowLong(hReBar, GWL_WNDPROC, (DWORD) ToolbarParentWndProc);
	}

	return;
}


BOOL CALLBACK ToolbarParentWndProc(HWND hDlg, 
								   UINT msg,
								   WPARAM wParam, 
								   LPARAM lParam)
{
	DLGPROC lpOldProc;
	PluginInfo *plugin;

	lpOldProc = (DLGPROC)GetProp( hDlg, "oldproc" );
	plugin = (PluginInfo *) GetProp(hDlg, PLUGIN_INFO_PROP);

	switch(msg)
	{
	case WM_NOTIFY:
		{
			int nIndex;

			if ((((LPNMHDR) lParam)->code) == TBN_BEGINADJUST)
			{
				nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
							IDC_PGPKEYS, 0);

				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
				plugin->nPGPKeysButton = -1;
			}

			if ((((LPNMHDR) lParam)->code) == TBN_ENDADJUST)
			{
				TBBUTTON tbb[1];
				
				tbb[0].iBitmap = plugin->nPGPKeysImage;
				tbb[0].idCommand = IDC_PGPKEYS;
				tbb[0].fsState = TBSTATE_ENABLED;
				tbb[0].fsStyle = TBSTYLE_BUTTON;
				tbb[0].dwData = 0;
				tbb[0].iString = plugin->nPGPKeysString;
				
				SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 1, 
					(LPARAM) &tbb);
				
				plugin->nPGPKeysButton = SendMessage(plugin->hToolbar,
											TB_COMMANDTOINDEX,
											IDC_PGPKEYS,
											0);
			}

			break;
		}
	}

	return CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
