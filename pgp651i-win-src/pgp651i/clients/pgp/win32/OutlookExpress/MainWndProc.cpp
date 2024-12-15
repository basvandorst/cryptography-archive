/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: MainWndProc.cpp,v 1.17 1999/04/26 19:40:39 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "PGPcl.h"
#include "resource.h"

void CreateMainWindowGUI(HWND hDlg, PluginInfo *plugin);
void AddMainToolbarButtons(PluginInfo *plugin);

static LRESULT CALLBACK ToolbarParentWndProc(HWND hDlg, 
											 UINT msg,
											 WPARAM wParam, 
											 LPARAM lParam);


LRESULT CALLBACK MainWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam)
{
	WNDPROC lpOldProc;
	PluginInfo *plugin;

	lpOldProc = (WNDPROC)GetProp( hDlg, "oldproc" );
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
			HIMAGELIST hilToolbar;
			int nIndex;
			LRESULT lResult;
			
			lResult = CommonWndProc(hDlg, msg, wParam, lParam);

			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_PGPKEYS, 0);

			SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
			plugin->nPGPKeysButton = -1;

			hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
										TB_GETIMAGELIST, 0, 0);
			
			ImageList_Remove(hilToolbar, plugin->nPGPKeysImage);

			AddMainToolbarButtons(plugin);
			return lResult;
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
			int nOptions;
			MENUITEMINFO menuInfo;
			char szOptions[256];
			char szMenu[256];
			LRESULT lResult;

			// For some reason, the old window proc disables the
			// menu items in the PGP popup menu. We'll enable
			// the items after the old proc disables them

			if ((HMENU) wParam == plugin->hPGPMenu)
			{
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				EnableMenuItem(plugin->hPGPMenu, IDC_PREFS, 
					MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(plugin->hPGPMenu, IDC_PGPKEYS, 
					MF_BYCOMMAND | MF_ENABLED);

				return lResult;
			}
			else if (plugin->bOE5)
			{
				UIGetString(szOptions, 254, IDS_MENU_OEOPTIONS);
				nOptions = GetMenuItemCount((HMENU) wParam);
				
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
				menuInfo.dwTypeData = szMenu;
				menuInfo.cch = 255;
				
				GetMenuItemInfo((HMENU) wParam, nOptions - 1, TRUE, 
					&menuInfo);

				if (!strcmp(szMenu, szOptions))
				{
					lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
								lParam);
					
					strcpy(szMenu, "&PGP");
					
					menuInfo.cbSize = sizeof(MENUITEMINFO);
					menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | 
						MIIM_STATE;
					menuInfo.fType = MFT_STRING;
					menuInfo.fState = MFS_ENABLED;
					menuInfo.wID = IDC_PGPMENU;
					menuInfo.hSubMenu = plugin->hPGPMenu;
					menuInfo.dwTypeData = szMenu;
					
					InsertMenuItem((HMENU) wParam, nOptions, TRUE, &menuInfo);
					plugin->nPGPPosition = nOptions;
					return lResult;
				}
				else if (!strcmp(szMenu, "&PGP"))
				{
					lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
								lParam);
					
					DestroyMenu(plugin->hPGPMenu);
					plugin->hPGPMenu = CreatePopupMenu();
					
					UIGetString(szMenu, 254, IDS_MENU_PREFS);
					AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PREFS, 
						szMenu);
					
					UIGetString(szMenu, 254, IDS_MENU_PGPKEYS);
					AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PGPKEYS, 
						szMenu);
					
					menuInfo.cbSize = sizeof(MENUITEMINFO);
					menuInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
					menuInfo.fState = MFS_ENABLED;
					menuInfo.hSubMenu = plugin->hPGPMenu;

					SetMenuItemInfo((HMENU) wParam, IDC_PGPMENU, FALSE,
						&menuInfo);

					menuInfo.fMask = MIIM_STATE;

					SetMenuItemInfo(plugin->hPGPMenu, IDC_PREFS, FALSE, 
						&menuInfo);
				
					SetMenuItemInfo(plugin->hPGPMenu, IDC_PGPKEYS, FALSE, 
						&menuInfo);

					return lResult;
				}
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
	int nIndex;
	MENUITEMINFO menuInfo;
	WNDPROC lpOldProc;
	char szText[255];
	
	hSizableRebar = FindWindowEx(hDlg, NULL, "SizableRebar", NULL);
	hReBar = FindWindowEx(hSizableRebar, NULL, "ReBarWindow32", NULL);

	if (plugin->bOE5)
	{
		plugin->hToolbar = FindWindowEx(hReBar, NULL, "ToolbarWindow32", 
								NULL);
		plugin->hMenuToolbar = FindWindowEx(hReBar, plugin->hToolbar, 
									"ToolbarWindow32", NULL);

		if (!plugin->hMenuToolbar || !plugin->hToolbar)
			return;
	}
	else
	{
		plugin->hMainMenu = GetMenu(hDlg);
		plugin->hToolbar = FindWindowEx(hReBar, NULL, "ToolbarWindow32", 
							NULL);

		if (!plugin->hMainMenu || !plugin->hToolbar)
			return;
	}
	
	if (plugin->bOE5)
	{
		if (plugin->hPGPMenu != NULL)
			DestroyMenu(plugin->hPGPMenu);
		
		plugin->hPGPMenu = CreatePopupMenu();
		
		UIGetString(szText, 254, IDS_MENU_PREFS);
		AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PREFS, szText);
		
		UIGetString(szText, 254, IDS_MENU_PGPKEYS);
		AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PGPKEYS, szText);
		
/*
		lpOldProc = (WNDPROC) GetProp(plugin->hMenuToolbar, "oldproc");
		if (lpOldProc == NULL)
		{
			// Save away menu toolbar window proc
			SetProp(plugin->hMenuToolbar, "oldproc", 
				(HANDLE) GetWindowLong(plugin->hMenuToolbar, GWL_WNDPROC)); 
			
			// Save plugin pointer
			SetProp(plugin->hMenuToolbar, PLUGIN_INFO_PROP, 
				(HANDLE) plugin);
			
			// Subclass menu toolbar window
			SetWindowLong(plugin->hMenuToolbar, GWL_WNDPROC, 
				(DWORD) MenuToolbarWndProc);
		}
*/
	}
	else
	{
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
	}
	
	SendMessage(plugin->hToolbar, TB_BUTTONSTRUCTSIZE,
		(WPARAM) sizeof(TBBUTTON), 0);
	
	nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
				IDC_PGPKEYS, 0);

	if (nIndex < 0)
	{
		AddMainToolbarButtons(plugin); 
		SetTimer(hDlg, WATCH_TOOLBAR_TIMER, 500, NULL);
	}
	
	// Look for status bar
	plugin->hStatusBar = FindWindowEx(hDlg, NULL, 
		"msctls_statusbar32", NULL);

	lpOldProc = (WNDPROC) GetProp(hReBar, "oldproc");
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


void AddMainToolbarButtons(PluginInfo *plugin)
{
	HDC	hDC;
	int	iNumBits;
	HIMAGELIST hilToolbar;
	HBITMAP hbmpPGPkeys;
	int nPGPkeysSTR;
	int nPGPkeysBMP;
	TBBUTTON tbb[1];
	char szText[255];
	HINSTANCE hInst;
	
	hInst = UIGetInstance();
	
	hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, TB_GETIMAGELIST, 
								0, 0);
	
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);
	
	if (iNumBits <= 8) 
		hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSOFF4));
	else
		hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSOFF));
	
	nPGPkeysBMP = ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
					TRANSPARENT_COLOR);
	DeleteObject(hbmpPGPkeys);
	plugin->nPGPKeysImage = nPGPkeysBMP;
	
	SendMessage(plugin->hToolbar, TB_SETIMAGELIST, 0, (LPARAM) hilToolbar);
	
	hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
								TB_GETHOTIMAGELIST, 0, 0);
	
	if (iNumBits <= 8)
		hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSLARGE4));
	else
		hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSLARGE));
	
	ImageList_AddMasked(hilToolbar, hbmpPGPkeys, TRANSPARENT_COLOR);
	DeleteObject(hbmpPGPkeys);
	
	SendMessage(plugin->hToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM) hilToolbar);
	
	UIGetString(szText, 254, IDS_TOOLTIP_PGPKEYS);
	nPGPkeysSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
					(LPARAM) szText);
	plugin->nPGPKeysString = nPGPkeysSTR;
	
	tbb[0].iBitmap = nPGPkeysBMP;
	tbb[0].idCommand = IDC_PGPKEYS;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;
	tbb[0].dwData = 0;
	tbb[0].iString = nPGPkeysSTR;
	
	SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 1, (LPARAM) &tbb);
	
	plugin->nPGPKeysButton = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
								IDC_PGPKEYS, 0);
	
	return;
}


static LRESULT CALLBACK ToolbarParentWndProc(HWND hDlg, 
											 UINT msg,
											 WPARAM wParam, 
											 LPARAM lParam)
{
	WNDPROC lpOldProc;
	PluginInfo *plugin;
	LPNMHDR lpnm;
	LPNMTOOLBAR lptb;

	lpOldProc = (WNDPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);

	lpnm = (LPNMHDR) lParam;
	lptb = (LPNMTOOLBAR) lParam;

	switch(msg)
	{
	case WM_NOTIFY:
		{
			int nIndex;

			if (lpnm->code == TBN_BEGINADJUST)
			{
				nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
							IDC_PGPKEYS, 0);

				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
				plugin->nPGPKeysButton = -1;
			}

			if (lpnm->code == TBN_ENDADJUST)
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


/*
LRESULT CALLBACK MenuToolbarWndProc(HWND hDlg, 
									UINT msg,
									WPARAM wParam, 
									LPARAM lParam)
{
	WNDPROC lpOldProc;
	PluginInfo *plugin;
	LPNMHDR lpnm;
	LPNMTOOLBAR lptb;
	LRESULT lResult;

	lpOldProc = (WNDPROC)GetProp( hDlg, "oldproc" );
	plugin = (PluginInfo *) GetProp(hDlg, PLUGIN_INFO_PROP);

	lpnm = (LPNMHDR) lParam;
	lptb = (LPNMTOOLBAR) lParam;

	switch(msg)
	{
	}

	return CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);
}
*/


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
