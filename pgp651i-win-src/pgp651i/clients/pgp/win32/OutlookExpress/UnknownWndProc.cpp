/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: UnknownWndProc.cpp,v 1.12 1999/04/26 19:13:24 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "TextIO.h"
#include "Prefs.h"
#include "resource.h"

void CreateMessageWindowGUI(HWND hDlg, PluginInfo *plugin, WNDPROC lpOldProc);

static LRESULT CALLBACK ToolbarWndProc(HWND hDlg, 
									   UINT msg,
									   WPARAM wParam, 
									   LPARAM lParam);


LRESULT CALLBACK UnknownWndProc(HWND hDlg, 
								UINT msg,
								WPARAM wParam, 
								LPARAM lParam)
{
	WNDPROC lpOldProc;
	PluginInfo *plugin;

	lpOldProc = (WNDPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);

	switch(msg)
	{
	case WM_PARENTNOTIFY:
		if (LOWORD(wParam) == WM_CREATE)
		{
			CreateMessageWindowGUI(hDlg, plugin, lpOldProc);
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


void CreateMessageWindowGUI(HWND hDlg, PluginInfo *plugin, WNDPROC lpOldProc)
{
	HWND hSizableRebar;
	HWND hReBar;
	TBBUTTON tbb[4];
	HIMAGELIST hilToolbar;
	HIMAGELIST hilHotToolbar;
	HINSTANCE hInst;
	int nPGPkeysBMP;
	int nPGPkeysSTR = -1;
	HBITMAP hbmpPGPkeys;
	char szText[255];
	MENUITEMINFO menuInfo;
	BOOL bSend = FALSE;
	int nCount;
	HDC	hDC;
	int	iNumBits;
	DWORD dwSize;
	int nX;
	int nY;

	hSizableRebar = FindWindowEx(hDlg, NULL, "SizableRebar", NULL);
	if (hSizableRebar != NULL)
		plugin->bOE5 = TRUE;

	plugin->hStatusBar = FindWindowEx(hDlg, NULL, "msctls_statusbar32", NULL);

	if (plugin->bOE5)
	{
		hReBar = FindWindowEx(hSizableRebar, NULL, "ReBarWindow32", NULL);
		plugin->hToolbar = FindWindowEx(hReBar, NULL, "ToolbarWindow32", 
							NULL);
		plugin->hMenuToolbar = FindWindowEx(hReBar, plugin->hToolbar, 
									"ToolbarWindow32", NULL);

		if (!plugin->hMenuToolbar || !plugin->hToolbar || !plugin->hStatusBar)
			return;

		hDC = GetDC (NULL);		// DC for desktop
		iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * 
					GetDeviceCaps (hDC, PLANES);
		ReleaseDC (NULL, hDC);
	}
	else
	{
		hReBar = FindWindowEx(hDlg, NULL, "ReBarWindow32", NULL);
		plugin->hToolbar = FindWindowEx(hReBar, NULL, "ToolbarWindow32", 
							NULL);
		plugin->hMainMenu = GetMenu(hDlg);
		
		if (!plugin->hMainMenu || !plugin->hToolbar || !plugin->hStatusBar)
			return;
	}
	
	plugin->hEdit = NULL;
	plugin->szReplyText = NULL;
	hInst = UIGetInstance();
	
	// Subclass the Window
	if (plugin->bOE5)
		nCount = SendMessage(plugin->hMenuToolbar, TB_BUTTONCOUNT, 0, 0);
	else
		nCount = GetMenuItemCount(plugin->hMainMenu);

	if (nCount == 8)
	{
		SetProp(plugin->hToolbar, TOOLBAR_TYPE_PROP, (HANDLE) 1);
		SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)SendMsgWndProc);
		plugin->nPGPPosition = 5;
		bSend = TRUE;
	}
	else if (nCount == 6)
	{
		SetProp(plugin->hToolbar, TOOLBAR_TYPE_PROP, (HANDLE) 0);
		SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)ReadMsgWndProc);
		plugin->nPGPPosition = 4;
	}
	else
	{
		SetWindowLong(hDlg, GWL_WNDPROC, (DWORD)lpOldProc);
		FreePluginInfo(plugin);
		return;
	}
	
	if (plugin->bOE5)
	{
		if (plugin->hPGPMenu != NULL)
			DestroyMenu(plugin->hPGPMenu);
		
		plugin->hPGPMenu = CreatePopupMenu();
		plugin->nPGPPosition = -1;
	}
	else
	{
		menuInfo.cbSize = sizeof(MENUITEMINFO);
		menuInfo.fMask = MIIM_SUBMENU;
		GetMenuItemInfo(plugin->hMainMenu, plugin->nPGPPosition, TRUE, 
			&menuInfo);
		plugin->hToolsMenu = menuInfo.hSubMenu;
		
		plugin->hPGPMenu = CreatePopupMenu();
		InsertMenu(plugin->hMainMenu, plugin->nPGPPosition, 
			MF_BYPOSITION | MF_STRING | MF_POPUP, 
			(UINT) plugin->hPGPMenu, "&PGP");
	}

	SendMessage(plugin->hToolbar, TB_BUTTONSTRUCTSIZE,
		(WPARAM) sizeof(TBBUTTON), 0);

	dwSize = SendMessage(plugin->hToolbar, TB_GETBUTTONSIZE, 0, 0);
	nX = LOWORD(dwSize) + 8;
	nY = HIWORD(dwSize);

	hilToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, TB_GETIMAGELIST, 
								0, 0);
	if (plugin->bOE5)
		hilHotToolbar = (HIMAGELIST) SendMessage(plugin->hToolbar, 
										TB_GETHOTIMAGELIST, 0, 0);
	
	if (bSend)
	{
		int nEncryptBMP;
		int nSignBMP;
		HBITMAP hbmpEncrypt;
		HBITMAP hbmpSign;
		int nEncryptSTR = -1;
		int nSignSTR = -1;
		
		if (!(plugin->bOE5))
		{
			GetMenuString(plugin->hToolsMenu, ENCRYPT_SMIME_POS, szText, 254, 
				MF_BYPOSITION);
			
			if (!strcmp(szText, "&Encrypt"))
			{
				UIGetString(szText, 254, IDS_MENU_PGP_ENCRYPT);
				InsertMenu(plugin->hToolsMenu, ENCRYPT_PGP_POS, 
					MF_BYPOSITION | MF_STRING, IDC_ENCRYPT_PGP, szText);
				
				UIGetString(szText, 254, IDS_MENU_PGP_SIGN);
				InsertMenu(plugin->hToolsMenu, SIGN_PGP_POS, 
					MF_BYPOSITION | MF_STRING, IDC_SIGN_PGP, szText);
				
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_TYPE;
				menuInfo.fType = MFT_STRING;
				menuInfo.dwTypeData = szText;
				
				UIGetString(szText, 254, IDS_MENU_SMIME_ENCRYPT);
				SetMenuItemInfo(plugin->hToolsMenu, ENCRYPT_SMIME_POS, TRUE, 
					&menuInfo);
				
				UIGetString(szText, 254, IDS_MENU_SMIME_SIGN);
				SetMenuItemInfo(plugin->hToolsMenu, SIGN_SMIME_POS, TRUE, 
					&menuInfo);
			}
			else
			{
				UIGetString(szText, 254, IDS_MENU_PGP_ENCRYPT);
				AppendMenu(plugin->hToolsMenu, MF_STRING, IDC_ENCRYPT_PGP, 
					szText);
				
				UIGetString(szText, 254, IDS_MENU_PGP_SIGN);
				AppendMenu(plugin->hToolsMenu, MF_STRING, IDC_SIGN_PGP, 
					szText);
			}
		}
		
		if (plugin->bOE5)
		{
			if (iNumBits <= 8)
			{
				hbmpEncrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_ENCRYPTOFF4));
				hbmpSign = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_SIGNOFF4));
			}
			else
			{
				hbmpEncrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_ENCRYPTOFF));
				hbmpSign = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_SIGNOFF));
			}
		}
		else
		{
			hbmpEncrypt = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ENCRYPT));
			hbmpSign = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SIGN));
		}

		nEncryptBMP = ImageList_AddMasked(hilToolbar, hbmpEncrypt, 
						TRANSPARENT_COLOR); 
		DeleteObject(hbmpEncrypt);
		plugin->nEncryptImage = nEncryptBMP;
		
		nSignBMP = ImageList_AddMasked(hilToolbar, hbmpSign, 
						TRANSPARENT_COLOR); 
		DeleteObject(hbmpSign);
		plugin->nSignImage = nSignBMP;
		
		if (plugin->bOE5)
		{
			TBBUTTON tbOE;
			int nIndex;
			int nEncryptSMIME;
			int nSignSMIME;

			if (iNumBits <= 8)
			{
				hbmpEncrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_ENCRYPTLARGE4));
				hbmpSign = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_SIGNLARGE4));
			}
			else
			{
				hbmpEncrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_ENCRYPTLARGE));
				hbmpSign = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_SIGNLARGE));
			}

			ImageList_AddMasked(hilHotToolbar, hbmpEncrypt, 
				TRANSPARENT_COLOR);

			ImageList_AddMasked(hilHotToolbar, hbmpSign, 
				TRANSPARENT_COLOR);

			DeleteObject(hbmpEncrypt);
			DeleteObject(hbmpSign);

			UIGetString(szText, 254, IDS_BUTTON_PGP_ENCRYPT);
			nEncryptSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
							(LPARAM) szText);

			UIGetString(szText, 254, IDS_BUTTON_PGP_SIGN);
			nSignSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
							(LPARAM) szText);
			
			plugin->nEncryptString = nEncryptSTR;
			plugin->nSignString = nSignSTR;

			UIGetString(szText, 254, IDS_BUTTON_SMIME_ENCRYPT);
			nEncryptSMIME = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
								(LPARAM) szText);

			UIGetString(szText, 254, IDS_BUTTON_SMIME_SIGN);
			nSignSMIME = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
							(LPARAM) szText);

			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_OE5_ENCRYPT_SMIME, 0);

			SendMessage(plugin->hToolbar, TB_GETBUTTON, nIndex,
				(LPARAM) &tbOE);

			tbOE.iString = nEncryptSMIME;
			SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
			SendMessage(plugin->hToolbar, TB_INSERTBUTTON, nIndex, 
				(LPARAM) &tbOE); 

			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_OE5_SIGN_SMIME, 0);

			SendMessage(plugin->hToolbar, TB_GETBUTTON, nIndex,
				(LPARAM) &tbOE);

			tbOE.iString = nSignSMIME;
			SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
			SendMessage(plugin->hToolbar, TB_INSERTBUTTON, nIndex, 
				(LPARAM) &tbOE); 
		}

		tbb[1].iBitmap = nEncryptBMP;
		tbb[1].idCommand = IDC_ENCRYPT_PGP;
		tbb[1].fsState = TBSTATE_ENABLED;
		tbb[1].fsStyle = TBSTYLE_CHECK;
		tbb[1].dwData = 0;
		tbb[1].iString = nEncryptSTR;
		
		tbb[2].iBitmap = nSignBMP;
		tbb[2].idCommand = IDC_SIGN_PGP;
		tbb[2].fsState = TBSTATE_ENABLED;
		tbb[2].fsStyle = TBSTYLE_CHECK;
		tbb[2].dwData = 0;
		tbb[2].iString = nSignSTR;
		
		plugin->bEncrypt = ByDefaultEncrypt(plugin->memoryMgr);
		plugin->bSign = ByDefaultSign(plugin->memoryMgr);
		
		SetTimer(hDlg, PASTE_REPLY_TIMER, 250, 0);
	}
	else
	{
		int nDecryptBMP;
		HBITMAP hbmpDecrypt;
		int nDecryptSTR = -1;
			
		if (!(plugin->bOE5))
		{
			UIGetString(szText, 254, IDS_MENU_DECRYPT);
			AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_DECRYPT, szText);
		
			AppendMenu(plugin->hPGPMenu, MF_SEPARATOR, NULL, NULL);
		}
		
		if (plugin->bOE5)
		{
			if (iNumBits <= 8)
				hbmpDecrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_DECRYPTOFF4));
			else
				hbmpDecrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_DECRYPTOFF));

		}
		else
			hbmpDecrypt = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DECRYPT));

		nDecryptBMP = ImageList_AddMasked(hilToolbar, hbmpDecrypt, 
						TRANSPARENT_COLOR);
		DeleteObject(hbmpDecrypt);
		plugin->nDecryptImage = nDecryptBMP;
		
		if (plugin->bOE5)
		{
			if (iNumBits <= 8)
				hbmpDecrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_DECRYPTLARGE4));
			else
				hbmpDecrypt = LoadBitmap(hInst, 
								MAKEINTRESOURCE(IDB_DECRYPTLARGE));

			ImageList_AddMasked(hilHotToolbar, hbmpDecrypt, 
				TRANSPARENT_COLOR);
			DeleteObject(hbmpDecrypt);

			UIGetString(szText, 254, IDS_TOOLTIP_DECRYPT);
			nDecryptSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
							(LPARAM) szText);
			plugin->nDecryptString = nDecryptSTR;
		}

		tbb[1].iBitmap = nDecryptBMP;
		tbb[1].idCommand = IDC_DECRYPT;
		tbb[1].fsState = TBSTATE_ENABLED;
		tbb[1].fsStyle = TBSTYLE_BUTTON;
		tbb[1].dwData = 0;
		tbb[1].iString = nDecryptSTR;
		
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
	AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PREFS, szText);
	
	UIGetString(szText, 254, IDS_MENU_PGPKEYS);
	AppendMenu(plugin->hPGPMenu, MF_STRING, IDC_PGPKEYS, szText);
	
	if (plugin->bOE5)
	{
		if (iNumBits <= 8) 
			hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSOFF4));
		else
			hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYSOFF));
	}
	else
	{
		DrawMenuBar(hDlg);
		hbmpPGPkeys = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PGPKEYS));
	}
	
	nPGPkeysBMP = ImageList_AddMasked(hilToolbar, hbmpPGPkeys, 
					TRANSPARENT_COLOR);
	DeleteObject(hbmpPGPkeys);
	plugin->nPGPKeysImage = nPGPkeysBMP;
	
	SendMessage(plugin->hToolbar, TB_SETIMAGELIST, 0, (LPARAM) hilToolbar);

	if (plugin->bOE5)
	{
		if (iNumBits <= 8)
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSLARGE4));
		else
			hbmpPGPkeys = LoadBitmap(hInst, 
							MAKEINTRESOURCE(IDB_PGPKEYSLARGE));

		ImageList_AddMasked(hilHotToolbar, hbmpPGPkeys, TRANSPARENT_COLOR);
		DeleteObject(hbmpPGPkeys);

		UIGetString(szText, 254, IDS_TOOLTIP_PGPKEYS);
		nPGPkeysSTR = SendMessage(plugin->hToolbar, TB_ADDSTRING, 0,
						(LPARAM) szText);
		plugin->nPGPKeysString = nPGPkeysSTR;
	}
	
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
	tbb[3].iString = nPGPkeysSTR;
	
	SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 4, (LPARAM) &tbb);

	if (plugin->bOE5)
	{
		int nIndex;
		TBBUTTON tb;

		if (bSend)
			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_ENCRYPT_PGP, 0);
		else
			nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
						IDC_DECRYPT, 0);

		SendMessage(plugin->hToolbar, TB_GETBUTTON, nIndex - 2, 
			(LPARAM) &tb);

		if (tb.fsStyle & TBSTYLE_SEP)
			SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex - 2, 0);

		if (nY > 26)
		{
			SendMessage(plugin->hToolbar, TB_SETBUTTONSIZE, 0, 
				MAKELONG(nX, nY));
			plugin->nButtonSizeX = nX;
			plugin->nButtonSizeY = nY;
		}

		if (GetProp(plugin->hToolbar, "oldproc") == NULL)
		{
			// Save away toolbar parent window proc
			SetProp(plugin->hToolbar, "oldproc", 
				(HANDLE) GetWindowLong(plugin->hToolbar, GWL_WNDPROC)); 

			// Save plugin pointer
			SetProp(plugin->hToolbar, PLUGIN_INFO_PROP, (HANDLE) plugin);

			// Subclass toolbar window
			SetWindowLong(plugin->hToolbar, GWL_WNDPROC, 
				(DWORD) ToolbarWndProc);
		}

		if (GetProp(hReBar, "oldproc") == NULL)
		{
			// Save away toolbar parent window proc
			SetProp(hReBar, "oldproc", 
				(HANDLE) GetWindowLong(hReBar, GWL_WNDPROC)); 

			// Save plugin pointer
			SetProp(hReBar, PLUGIN_INFO_PROP, (HANDLE) plugin);

			// Subclass toolbar parent window
			if (bSend)
				SetWindowLong(hReBar, GWL_WNDPROC, 
					(DWORD) SendToolbarParentWndProc);
			else
				SetWindowLong(hReBar, GWL_WNDPROC, 
					(DWORD) ReadToolbarParentWndProc);
		}
	}
	
	if (bSend && !(plugin->bOE5))
		SetTimer(hDlg, CORRECT_TOOLBAR_TIMER, 250, 0);

	return;
}


static LRESULT CALLBACK ToolbarWndProc(HWND hDlg, 
									   UINT msg,
									   WPARAM wParam, 
									   LPARAM lParam)
{
	WNDPROC lpOldProc;
	PluginInfo *plugin;
	UINT nState;
	UINT nOldState;
	LRESULT lResult;
	BOOL bSend;

	lpOldProc = (WNDPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);
	bSend = (BOOL) GetProp(hDlg, TOOLBAR_TYPE_PROP);

	if ((msg == WM_USER+59) && (lParam == 0x00460000) && 
		(plugin->nButtonSizeX > 0) && (plugin->nButtonSizeY > 0))
	{
		DWORD dwSize;
		INT nX;
		INT nY;
		
		dwSize = SendMessage(hDlg, TB_GETBUTTONSIZE, 0, 0);
		nX = LOWORD(dwSize);
		nY = HIWORD(dwSize);
		
		if (!dwSize)
			return CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);

		if ((nX != plugin->nButtonSizeX) || 
			(nY != plugin->nButtonSizeY))
		{
			lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);

			SendMessage(hDlg, TB_SETBUTTONSIZE, 0, 
				MAKELONG(plugin->nButtonSizeX, plugin->nButtonSizeY));

			return lResult;
		}
	}

	lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);

	nState = CallWindowProc(lpOldProc, hDlg, TB_GETSTATE, IDC_PGPKEYS, 0);
	if (!(nState & TBSTATE_ENABLED))
	{
		nState |= TBSTATE_ENABLED;
		CallWindowProc(lpOldProc, hDlg, TB_SETSTATE, IDC_PGPKEYS,
			MAKELONG(nState, 0));
	}

	if (bSend)
	{
		nState = CallWindowProc(lpOldProc, hDlg, TB_GETSTATE, IDC_SIGN_PGP, 
					0);
		nOldState = nState;

		if (!(nState & TBSTATE_ENABLED))
			nState |= TBSTATE_ENABLED;

		if (plugin->bSign)
		{
			if (!(nState & TBSTATE_CHECKED))
				nState |= TBSTATE_CHECKED;
		}
		else
		{
			if (nState & TBSTATE_CHECKED)
				nState ^= TBSTATE_CHECKED;
		}

		if (nState != nOldState)
		{
			CallWindowProc(lpOldProc, hDlg, TB_SETSTATE, IDC_SIGN_PGP,
				MAKELONG(nState, 0));
		}

		nState = CallWindowProc(lpOldProc, hDlg, TB_GETSTATE, IDC_ENCRYPT_PGP, 
					0);
		nOldState = nState;

		if (!(nState & TBSTATE_ENABLED))
			nState |= TBSTATE_ENABLED;

		if (plugin->bEncrypt)
		{
			if (!(nState & TBSTATE_CHECKED))
				nState |= TBSTATE_CHECKED;
		}
		else
		{
			if (nState & TBSTATE_CHECKED)
				nState ^= TBSTATE_CHECKED;
		}

		if (nState != nOldState)
		{
			CallWindowProc(lpOldProc, hDlg, TB_SETSTATE, IDC_ENCRYPT_PGP,
				MAKELONG(nState, 0));
		}
	}
	else
	{
		TBBUTTON tb;

		nState = CallWindowProc(lpOldProc, hDlg, TB_GETSTATE, IDC_DECRYPT, 0);
		if (!(nState & TBSTATE_ENABLED))
		{
			nState |= TBSTATE_ENABLED;
			CallWindowProc(lpOldProc, hDlg, TB_SETSTATE, IDC_DECRYPT,
				MAKELONG(nState, 0));
		}

		nState = CallWindowProc(lpOldProc, hDlg, TB_COMMANDTOINDEX,
					IDC_DECRYPT, 0);

		CallWindowProc(lpOldProc, hDlg, TB_GETBUTTON, nState - 2, 
			(LPARAM) &tb);

		if (tb.fsStyle & TBSTYLE_SEP)
			CallWindowProc(lpOldProc, hDlg, TB_DELETEBUTTON, nState - 2, 0);
	}

	return lResult;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
