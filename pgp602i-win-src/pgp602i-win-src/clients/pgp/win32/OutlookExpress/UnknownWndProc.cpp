/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: UnknownWndProc.cpp,v 1.7.8.1 1998/11/12 03:12:52 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "TextIO.h"
#include "Prefs.h"
#include "resource.h"


BOOL CALLBACK UnknownWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam)
{
	DLGPROC lpOldProc;
	PluginInfo *plugin;
	MENUITEMINFO menuInfo;
	BOOL bSend = FALSE;

	lpOldProc = (DLGPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);

	switch(msg)
	{
	case WM_PARENTNOTIFY:
		if (LOWORD(wParam) == WM_CREATE)
		{
			HWND hReBar;
			TBBUTTON tbb[4];
			HIMAGELIST hilToolbar;
			HINSTANCE hInst;
			int nPGPkeysBMP;
			HBITMAP hbmpPGPkeys;
			char szText[255];

			plugin->hMainMenu = GetMenu(hDlg);
			hReBar = FindWindowEx(hDlg, NULL, "ReBarWindow32", NULL);
			plugin->hToolbar = FindWindowEx(hReBar, NULL, 
								"ToolbarWindow32", NULL);
			plugin->hStatusBar = FindWindowEx(hDlg, NULL, 
									"msctls_statusbar32", NULL);

			if (!plugin->hMainMenu || !plugin->hToolbar || 
				!plugin->hStatusBar)
				break;

			plugin->hEdit = NULL;
			plugin->szReplyText = NULL;
			hInst = UIGetInstance();

			// Subclass the Window
			if (GetMenuItemCount(plugin->hMainMenu) == 8)
			{
				SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)SendMsgWndProc);
				plugin->nPGPPosition = 5;
				bSend = TRUE;
			}
			else if (GetMenuItemCount(plugin->hMainMenu) == 6)
			{
				SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)ReadMsgWndProc);
				plugin->nPGPPosition = 4;
			}
			else
			{
				SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)lpOldProc);
				FreePluginInfo(plugin);
				break;
			}

			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_SUBMENU;
			GetMenuItemInfo(plugin->hMainMenu, plugin->nPGPPosition, 
				TRUE, &menuInfo);
			plugin->hToolsMenu = menuInfo.hSubMenu;

			plugin->hPGPMenu = CreatePopupMenu();
			InsertMenu(plugin->hMainMenu, plugin->nPGPPosition, 
				MF_BYPOSITION | MF_STRING | MF_POPUP, 
				(UINT) plugin->hPGPMenu, "&PGP");

			SendMessage(plugin->hToolbar, TB_BUTTONSTRUCTSIZE,
				(WPARAM) sizeof(TBBUTTON), 0);
			
			hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
										TB_GETIMAGELIST, 0, 0);

			if (bSend)
			{
				int nEncryptBMP;
				int nSignBMP;
				HBITMAP hbmpEncrypt;
				HBITMAP hbmpSign;

				GetMenuString(plugin->hToolsMenu, ENCRYPT_SMIME_POS,
					szText, 254, MF_BYPOSITION);

				if (!strcmp(szText, "&Encrypt"))
				{
					UIGetString(szText, 254, IDS_MENU_PGP_ENCRYPT);
					InsertMenu(plugin->hToolsMenu, ENCRYPT_PGP_POS, 
						MF_BYPOSITION | MF_STRING, IDC_ENCRYPT_PGP, 
						szText);
					
					UIGetString(szText, 254, IDS_MENU_PGP_SIGN);
					InsertMenu(plugin->hToolsMenu, SIGN_PGP_POS, 
						MF_BYPOSITION | MF_STRING, IDC_SIGN_PGP, 
						szText);
					
					menuInfo.cbSize = sizeof(MENUITEMINFO);
					menuInfo.fMask = MIIM_TYPE;
					menuInfo.fType = MFT_STRING;
					menuInfo.dwTypeData = szText;
					
					UIGetString(szText, 254, IDS_MENU_SMIME_ENCRYPT);
					SetMenuItemInfo(plugin->hToolsMenu, ENCRYPT_SMIME_POS, 
						TRUE, &menuInfo);
					
					UIGetString(szText, 254, IDS_MENU_SMIME_SIGN);
					SetMenuItemInfo(plugin->hToolsMenu, SIGN_SMIME_POS, 
						TRUE, &menuInfo);
				}
				else
				{
					UIGetString(szText, 254, IDS_MENU_PGP_ENCRYPT);
					AppendMenu(plugin->hToolsMenu, MF_STRING, 
						IDC_ENCRYPT_PGP, szText);
					
					UIGetString(szText, 254, IDS_MENU_PGP_SIGN);
					AppendMenu(plugin->hToolsMenu, MF_STRING, 
						IDC_SIGN_PGP, szText);
				}

				hbmpEncrypt = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ENCRYPT));
				nEncryptBMP = ImageList_AddMasked(hilToolbar, hbmpEncrypt, 
								TRANSPARENT_COLOR); 
				DeleteObject(hbmpEncrypt);

				hbmpSign = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SIGN));
				nSignBMP = ImageList_AddMasked(hilToolbar, hbmpSign, 
								TRANSPARENT_COLOR); 
				DeleteObject(hbmpSign);

				tbb[1].iBitmap = nEncryptBMP;
				tbb[1].idCommand = IDC_ENCRYPT_PGP;
				tbb[1].fsState = TBSTATE_ENABLED;
				tbb[1].fsStyle = TBSTYLE_CHECK;
				tbb[1].dwData = 0;
				tbb[1].iString = -1;

				tbb[2].iBitmap = nSignBMP;
				tbb[2].idCommand = IDC_SIGN_PGP;
				tbb[2].fsState = TBSTATE_ENABLED;
				tbb[2].fsStyle = TBSTYLE_CHECK;
				tbb[2].dwData = 0;
				tbb[2].iString = -1;

				plugin->bEncrypt = ByDefaultEncrypt(plugin->memoryMgr);
				plugin->bSign = ByDefaultSign(plugin->memoryMgr);

				SetTimer(hDlg, PASTE_REPLY_TIMER, 250, 0);
			}
			else
			{
				int nDecryptBMP;
				HBITMAP hbmpDecrypt;

				UIGetString(szText, 254, IDS_MENU_DECRYPT);
				AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_DECRYPT, 
					szText);

				AppendMenu(plugin->hPGPMenu, MF_SEPARATOR, NULL, NULL);

				hbmpDecrypt = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DECRYPT));
				nDecryptBMP = ImageList_AddMasked(hilToolbar, hbmpDecrypt, 
									TRANSPARENT_COLOR);
				DeleteObject(hbmpDecrypt);

				tbb[1].iBitmap = nDecryptBMP;
				tbb[1].idCommand = IDC_DECRYPT;
				tbb[1].fsState = TBSTATE_ENABLED;
				tbb[1].fsStyle = TBSTYLE_BUTTON;
				tbb[1].dwData = 0;
				tbb[1].iString = -1;

				tbb[2].iBitmap = -1;
				tbb[2].idCommand = -1;
				tbb[2].fsState = TBSTATE_HIDDEN;
				tbb[2].fsStyle = TBSTYLE_SEP;
				tbb[2].dwData = 0;
				tbb[2].iString = -1;

				if (AutoDecrypt(plugin->memoryMgr))
					SetTimer(hDlg, AUTO_DECRYPT_TIMER, 500, 0);
			}

			UIGetString(szText, 254, IDS_MENU_PREFS);
			AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PREFS, 
				szText);

			UIGetString(szText, 254, IDS_MENU_PGPKEYS);
			AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PGPKEYS, 
				szText);

			DrawMenuBar(hDlg);

			hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYS));
			nPGPkeysBMP = ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
								TRANSPARENT_COLOR);
			DeleteObject(hbmpPGPkeys);

			SendMessage(plugin->hToolbar, TB_SETIMAGELIST, 0, 
				(LPARAM) hilToolbar);

			tbb[0].iBitmap = -1;
			tbb[0].idCommand = -1;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_SEP;
			tbb[0].dwData = 0;
			tbb[0].iString = -1;

			tbb[3].iBitmap = nPGPkeysBMP;
			tbb[3].idCommand = IDC_PGPKEYS;
			tbb[3].fsState = TBSTATE_ENABLED;
			tbb[3].fsStyle = TBSTYLE_BUTTON;
			tbb[3].dwData = 0;
			tbb[3].iString = -1;

			SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 4, (LPARAM) &tbb);

			if (bSend)
				SetTimer(hDlg, CORRECT_TOOLBAR_TIMER, 250, 0);

			break;
		}

	case WM_DESTROY:
		{
			FreePluginInfo(plugin);
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
