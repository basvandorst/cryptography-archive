/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ReadMsgWndProc.cpp,v 1.25 1999/04/26 19:13:24 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "TextIO.h"
#include "Prefs.h"
#include "resource.h"
#include "pgpErrors.h"
#include "AddKey.h"
#include "BlockUtils.h"
#include "DecryptVerify.h"
#include "PGPsc.h"

static BOOL DecryptVerifyMessage(HWND hwnd, PluginInfo *plugin);
static void EnableMenus(HWND hwnd, PluginInfo *plugin, BOOL bEnable);


LRESULT CALLBACK ReadMsgWndProc(HWND hDlg, 
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
		if (msg == plugin->nCopyDoneMsg)
		{
			if (wParam)
				DecryptVerifyMessage(hDlg, plugin);

			RestoreClipboardText(hDlg);
			return 0;
		}
	
		if (msg == plugin->nPasteDoneMsg)
			return 0;
	}

	switch(msg)
	{
	case WM_SIZE:
		{
			if (plugin->hEdit)
				if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
					PostMessage(hDlg, WM_EXITSIZEMOVE, 0, 0);

			break;
		}

	case WM_SIZING:
	case WM_EXITSIZEMOVE:
		{
			if (plugin->hEdit)
			{
				RECT rc;

				GetClientRect(plugin->hMessage, &rc);
				SetWindowPos(plugin->hEdit, HWND_TOP, 
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
					SWP_SHOWWINDOW);
			}

			break;
		}

	case WM_TIMER:
		if (wParam == AUTO_DECRYPT_TIMER)
		{
			KillTimer(hDlg, AUTO_DECRYPT_TIMER);
			PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DECRYPT, 0), 0);
			return 0;
		}
		break;

	case WM_MENUSELECT:
		{
			UINT uFlags = (UINT) HIWORD(wParam);
			char szText[255];
			BOOL bPluginMenu = FALSE;

			switch(LOWORD(wParam))
			{
				case IDC_DECRYPT:
				{
					UIGetString(szText, 254, IDS_STATUS_DECRYPT);
					bPluginMenu = TRUE;
					break;
				}
			}	

			if (bPluginMenu)
			{
				CallWindowProc(lpOldProc, hDlg, msg, wParam, lParam);
				SendMessage(plugin->hStatusBar, SB_SETTEXT, 
					255 | SBT_NOBORDERS, (LPARAM) szText);

				return 0;
			}

			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_DECRYPT:
				{
					if (SaveClipboardText(hDlg))
						_beginthread(DoCopyThread, 0, hDlg);

					return 0;
				}

				case IDC_OE5_PRINT:
				{
					if (plugin->bDisableMenus)
					{
						UIDisplayStringID(hDlg, IDS_E_CANTPRINT);
						return 0;
					}
					
					break;
				}

				case IDC_REPLY:
				case IDC_REPLYTOALL:
				case IDC_REPLYTOGROUP:
				case IDC_REPLYTOAUTHOR:
				case IDC_REPLYTOGROUPANDAUTHOR:
				case IDC_FORWARD:
				case IDC_OE5_REPLY:
				case IDC_OE5_REPLYTOALL:
				case IDC_OE5_REPLYTOGROUP:
				case IDC_OE5_FORWARD:
					{
						if (plugin->szOutput != NULL)
						{
							char *szText;

							szText = (char *) 
									 calloc(strlen(plugin->szOutput)+1, 1);

							strcpy(szText, plugin->szOutput);
							free(plugin->szOutput);
							plugin->szOutput = NULL;

							SetReplyText(szText);
						}
						break;
					}

				case IDC_NEXT:
				case IDC_PREVIOUS:
				case IDC_NEXTUNREADARTICLE:
				case IDC_NEXTUNREADTHREAD:
				case IDC_NEXTUNREADMESSAGE:
				case IDC_OE5_NEXT:
				case IDC_OE5_PREVIOUS:
				case IDC_OE5_NEXTUNREADTHREAD:
				case IDC_OE5_NEXTUNREADMESSAGE:
					{
						if (plugin->szOutput != NULL)
						{
							free(plugin->szOutput);
							plugin->szOutput = NULL;
							DestroyWindow(plugin->hEdit);
							ShowWindow(plugin->hMessage, SW_SHOW);
							EnableMenus(hDlg, plugin, TRUE);
						}

						if (AutoDecrypt(plugin->memoryMgr))
							SetTimer(hDlg, AUTO_DECRYPT_TIMER, 500, 0);

						break;
					}
			}
			break;
		}
		
	case WM_INITMENUPOPUP:
		if (plugin->bOE5)
		{
			int nAddrBook;
			MENUITEMINFO menuInfo;
			char szAddrBook[256];
			char szClose[256];
			char szStatusBar[256];
			char szMenu[256];
			LRESULT lResult;
			
			UIGetString(szAddrBook, 254, IDS_MENU_OEADDADDRBOOK);
			UIGetString(szClose, 254, IDS_MENU_OECLOSE);
			UIGetString(szStatusBar, 254, IDS_MENU_OESTATUSBAR);

			nAddrBook = GetMenuItemCount((HMENU) wParam);
			
			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_TYPE;
			menuInfo.dwTypeData = szMenu;
			menuInfo.cch = 255;
			
			GetMenuItemInfo((HMENU) wParam, nAddrBook - 1, TRUE, &menuInfo);
			
			if (strstr(szMenu, szAddrBook))
			{
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				UIGetString(szMenu, 254, IDS_MENU_DECRYPT);

				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_TYPE;
				menuInfo.fType = MFT_SEPARATOR;

				InsertMenuItem((HMENU) wParam, nAddrBook, TRUE, &menuInfo);

				menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
				menuInfo.fType = MFT_STRING;
				menuInfo.wID = IDC_DECRYPT;
				menuInfo.fState = MFS_ENABLED;
				menuInfo.dwTypeData = szMenu;

				InsertMenuItem((HMENU) wParam, nAddrBook+1, TRUE, &menuInfo);

				strcpy(szMenu, "&PGP");
				
				menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | 
									MIIM_STATE;
				menuInfo.fType = MFT_STRING;
				menuInfo.wID = IDC_PGPMENU;
				menuInfo.fState = MFS_ENABLED;
				menuInfo.hSubMenu = plugin->hPGPMenu;
				menuInfo.dwTypeData = szMenu;

				InsertMenuItem((HMENU) wParam, nAddrBook+2, TRUE, &menuInfo);
				plugin->nPGPPosition = nAddrBook + 2;

				return lResult;
			}
			else if (!strcmp(szMenu, "&PGP"))
			{
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_STATE;
				menuInfo.fState = MFS_ENABLED;
				
				SetMenuItemInfo((HMENU) wParam, IDC_DECRYPT, FALSE, 
					&menuInfo);
				
				SetMenuItemInfo((HMENU) wParam, IDC_PGPMENU, FALSE, 
					&menuInfo);
				
				SetMenuItemInfo(plugin->hPGPMenu, IDC_PREFS, FALSE, 
					&menuInfo);
				
				SetMenuItemInfo(plugin->hPGPMenu, IDC_PGPKEYS, FALSE, 
					&menuInfo);
				
				return lResult;
			}
			else if (strstr(szMenu, szClose))
			{
				UINT uFlags = MF_BYPOSITION;
				
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				if (plugin->bDisableMenus)
					uFlags |= MF_GRAYED;
				else
					uFlags |= MF_ENABLED;
				
				EnableMenuItem((HMENU) wParam, OE5_FILESAVEAS_MENU_POS, 
					uFlags);
				EnableMenuItem((HMENU) wParam, OE5_FILESTATIONERY_MENU_POS, 
					uFlags);
				EnableMenuItem((HMENU) wParam, OE5_FILEMOVE_MENU_POS, 
					uFlags);
				EnableMenuItem((HMENU) wParam, OE5_FILECOPY_MENU_POS, 
					uFlags);
				EnableMenuItem((HMENU) wParam, OE5_FILEPRINT_MENU_POS, 
					uFlags);

				return lResult;
			}
			else if (strstr(szMenu, szStatusBar))
			{
				UINT uFlags = MF_BYPOSITION;
				
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				if (plugin->bDisableMenus)
					uFlags |= MF_GRAYED;
				else
					uFlags |= MF_ENABLED;
				
				EnableMenuItem((HMENU) wParam, OE5_VIEWTEXTSIZE_MENU_POS, 
					uFlags);
				EnableMenuItem((HMENU) wParam, OE5_VIEWENCODING_MENU_POS, 
					uFlags);

				return lResult;
			}
		}
		break;

	case WM_NOTIFY:
		if ((((LPNMHDR) lParam)->code) == TTN_GETDISPINFO) 
		{
			int idCtrl;
			NMTTDISPINFO *pInfo;
			BOOL bTooltip = FALSE;

			idCtrl = ((LPNMHDR) lParam)->idFrom;
			pInfo = (NMTTDISPINFO *) lParam;

			switch (idCtrl)
			{
			case IDC_ENCRYPT_SMIME:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_SMIME_ENCRYPT;
				bTooltip = TRUE;
				break;

			case IDC_SIGN_SMIME:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_SMIME_SIGN;
				bTooltip = TRUE;
				break;

			case IDC_DECRYPT:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_DECRYPT;
				bTooltip = TRUE;
				break;

			case IDC_PGPKEYS:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_PGPKEYS;
				bTooltip = TRUE;
				break;
			}

			if (bTooltip)
			{
				pInfo->hinst = UIGetInstance();
				return 0;
			}
		}
		break;
	}

	return CommonWndProc(hDlg, msg, wParam, lParam);
}


BOOL DecryptVerifyMessage(HWND hwnd, PluginInfo *plugin)
{
	HWND hDocHost;
	HWND hMimeEdit;
	HWND hMessage;
	HWND hAttach;
	HWND hNoteHdr;
	HWND hTo;
	HFONT hFont;
	char *szInput;
	char *szOutput;
	DWORD dwLength;
	PGPSize nOutLength;
	RECT rc;
	char szExe[256];
	char szDll[256];
	PGPError nError = kPGPError_NoErr;
	BOOL FYEO;
	
	UIGetString(szExe, sizeof(szExe), IDS_EXE);
	UIGetString(szDll, sizeof(szDll), IDS_DLL);

	if (plugin->bOE5)
	{
		hDocHost = FindWindowEx(hwnd, NULL, "ME_DocHost", NULL);
		hMimeEdit = FindWindowEx(hDocHost, NULL, "##MimeEdit_Server", NULL);
		hMessage = FindWindowEx(hMimeEdit, NULL, "Internet Explorer_Server", 
					NULL);
	}
	else
	{
		hDocHost = FindWindowEx(hwnd, NULL, "Ath_DocHost", NULL);
		hMessage = FindWindowEx(hDocHost, NULL, "Internet Explorer_Server", 
					NULL);
	}

	plugin->hMessage = hMessage;
	
	hAttach = FindWindowEx(hwnd, NULL, "SysListView32", NULL);
	
	if (plugin->szOutput == NULL)
	{
		if (!GetMessageText(hwnd, FALSE, &szInput))
			return FALSE;

		dwLength = strlen(szInput);
	}
	else
	{
		dwLength = GetWindowTextLength(plugin->hEdit);
		szInput = (char *) calloc(dwLength+1, 1);
		GetWindowText(plugin->hEdit, szInput, dwLength+1);
	}

	FixBadSpaces(szInput);

	nError = DecryptVerifyBuffer(UIGetInstance(), hwnd, plugin->pgpContext, 
				plugin->tlsContext, szExe, szDll,
				szInput, dwLength, FALSE, (void **) &szOutput, &nOutLength,
				&FYEO);

	if (IsntPGPError(nError) && (szOutput != NULL) && (nOutLength > 0))
	{
		if (plugin->szOutput != NULL)
		{
			free(plugin->szOutput);
			plugin->szOutput = NULL;
		}

		if((FYEO)||(GetSecureViewerPref((void *)plugin->pgpContext)))
		{
			TempestViewer((void *)plugin->pgpContext,hwnd,
				szOutput,nOutLength,FYEO);

			PGPFreeData(szOutput);
		}
		else
		{
			plugin->szOutput = (char *) calloc(dwLength + nOutLength + 1, 1);
			strcpy(plugin->szOutput, szOutput);
			PGPFreeData(szOutput);

			GetClientRect(plugin->hMessage, &rc);
			ShowWindow(plugin->hMessage, SW_HIDE);

			if (plugin->hEdit)
				DestroyWindow(plugin->hEdit);

			plugin->hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, 
				"EDIT", plugin->szOutput, 
				WS_VISIBLE | WS_CHILD | 
				WS_VSCROLL | 
				ES_MULTILINE | ES_WANTRETURN | 
				ES_AUTOVSCROLL,
				rc.left, rc.top, 
				rc.right - rc.left, 
				rc.bottom - rc.top,
				hDocHost, NULL, 
				UIGetInstance(), NULL);
		
			hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
			PostMessage(plugin->hEdit, WM_SETFONT, (WPARAM) hFont, 
				MAKELPARAM(TRUE, 0));

			if (plugin->bOE5)
				hNoteHdr = FindWindowEx(hwnd, NULL, "OE_Envelope", NULL);
			else
				hNoteHdr = FindWindowEx(hwnd, NULL, "ATH_NoteHdr", NULL);
		
			SetWindowPos(plugin->hEdit, HWND_TOP, 
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				SWP_SHOWWINDOW);

			hTo = FindWindowEx(hNoteHdr, NULL, "RICHEDIT", NULL);
			SetFocus(hTo);
			SetFocus(plugin->hEdit);

			EnableMenus(hwnd, plugin, FALSE);
		}
	}
	
	free(szInput);

/*
	if (hAttach)
		if (ListView_GetItemCount(hAttach) > 0)
		{
			LV_ITEM lviAttach;
			char szFileName[MAX_PATH];
			
			lviAttach.mask = LVIF_TEXT | LVIF_PARAM;
			lviAttach.iItem = 0;
			lviAttach.iSubItem = 0;
			lviAttach.pszText = szFileName;
			lviAttach.cchTextMax = sizeof(szFileName);
			
			ListView_GetItem(hAttach, &lviAttach);
			MessageBox(hwnd, szFileName, "Attachment", MB_OK);
		}
*/
		
	return TRUE;		
}


void EnableMenus(HWND hwnd, PluginInfo *plugin, BOOL bEnable)
{
	MENUITEMINFO menuInfo;
	UINT uFlags = MF_BYPOSITION;

	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_SUBMENU;

	if (bEnable)
		uFlags |= MF_ENABLED;
	else
		uFlags |= MF_GRAYED;

	if (plugin->bOE5)
		plugin->bDisableMenus = !bEnable;
	else
	{
		GetMenuItemInfo(plugin->hMainMenu, FILE_MENU_POS, TRUE, &menuInfo);
		EnableMenuItem(menuInfo.hSubMenu, FILESAVEAS_MENU_POS, uFlags);
		EnableMenuItem(menuInfo.hSubMenu, FILESTATIONERY_MENU_POS, uFlags);
		EnableMenuItem(menuInfo.hSubMenu, FILEMOVE_MENU_POS, uFlags);
		EnableMenuItem(menuInfo.hSubMenu, FILECOPY_MENU_POS, uFlags);
		EnableMenuItem(menuInfo.hSubMenu, FILEPRINT_MENU_POS, uFlags);
		
		GetMenuItemInfo(plugin->hMainMenu, VIEW_MENU_POS, TRUE, &menuInfo);
		EnableMenuItem(menuInfo.hSubMenu, VIEWFONTS_MENU_POS, uFlags);
		EnableMenuItem(menuInfo.hSubMenu, VIEWLANG_MENU_POS, uFlags);
		
		DrawMenuBar(hwnd);
		
		SendMessage(plugin->hToolbar, TB_ENABLEBUTTON, IDC_SAVE,
			MAKELONG(bEnable, 0));
		SendMessage(plugin->hToolbar, TB_ENABLEBUTTON, IDC_PRINT,
			MAKELONG(bEnable, 0));
	}

	return;
}


LRESULT CALLBACK ReadToolbarParentWndProc(HWND hDlg, 
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

				nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
							IDC_DECRYPT, 0);

				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex - 1, 0);
			}

			if (lpnm->code == TBN_ENDADJUST)
			{
				DWORD dwSize;
				int nX;
				int nY;
				TBBUTTON tbb[3];
				
				tbb[0].iBitmap = -1;
				tbb[0].idCommand = -1;
				tbb[0].fsState = TBSTATE_ENABLED;
				tbb[0].fsStyle = TBSTYLE_SEP;
				tbb[0].dwData = 0;
				tbb[0].iString = -1;

				tbb[1].iBitmap = plugin->nDecryptImage;
				tbb[1].idCommand = IDC_DECRYPT;
				tbb[1].fsState = TBSTATE_ENABLED;
				tbb[1].fsStyle = TBSTYLE_BUTTON;
				tbb[1].dwData = 0;
				tbb[1].iString = plugin->nDecryptString;
				
				tbb[2].iBitmap = plugin->nPGPKeysImage;
				tbb[2].idCommand = IDC_PGPKEYS;
				tbb[2].fsState = TBSTATE_ENABLED;
				tbb[2].fsStyle = TBSTYLE_BUTTON;
				tbb[2].dwData = 0;
				tbb[2].iString = plugin->nPGPKeysString;
				
				dwSize = SendMessage(plugin->hToolbar, TB_GETBUTTONSIZE, 0, 0);
				nX = LOWORD(dwSize);
				nY = HIWORD(dwSize);
				
				SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 3, 
					(LPARAM) &tbb);

				SendMessage(plugin->hToolbar, TB_SETBUTTONSIZE, 0, 
					MAKELONG(nX, nY));
		
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
