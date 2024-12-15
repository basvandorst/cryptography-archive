/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: ReadMsgWndProc.cpp,v 1.17.2.1.2.1 1998/11/12 03:12:46 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
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
#include "pgpSC.h"

static BOOL DecryptVerifyMessage(HWND hwnd, PluginInfo *plugin);
static void EnableMenus(HWND hwnd, PluginInfo *plugin, BOOL bEnable);


BOOL CALLBACK ReadMsgWndProc(HWND hDlg, 
							 UINT msg,
							 WPARAM wParam, 
							 LPARAM lParam)
{
	DLGPROC lpOldProc;
	PluginInfo *plugin;

	lpOldProc = (DLGPROC)GetProp( hDlg, "oldproc" );
	plugin = GetPluginInfo(hDlg);

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
				SetWindowPos(plugin->hEdit, NULL, 
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
					SWP_NOZORDER | SWP_SHOWWINDOW);
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
					DecryptVerifyMessage(hDlg, plugin);
					break;
				}

				case IDC_REPLY:
				case IDC_REPLYTOALL:
				case IDC_REPLYTOGROUP:
				case IDC_REPLYTOAUTHOR:
				case IDC_REPLYTOGROUPANDAUTHOR:
				case IDC_FORWARD:
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
		
	case WM_NOTIFY:
		if ((((LPNMHDR) lParam)->code) == TTN_NEEDTEXT) 
		{
			int idCtrl;
			TOOLTIPTEXT *pText;
			BOOL bTooltip = FALSE;
			char szText[255];

			idCtrl = ((LPNMHDR) lParam)->idFrom;
			pText = (LPTOOLTIPTEXT) lParam;

			switch (idCtrl)
			{
			case IDC_ENCRYPT_SMIME:
				UIGetString(szText, 254, IDS_TOOLTIP_SMIME_ENCRYPT);
				bTooltip = TRUE;
				break;

			case IDC_SIGN_SMIME:
				UIGetString(szText, 254, IDS_TOOLTIP_SMIME_SIGN);
				bTooltip = TRUE;
				break;

			case IDC_DECRYPT:
				UIGetString(szText, 254, IDS_TOOLTIP_DECRYPT);
				bTooltip = TRUE;
				break;

			case IDC_PGPKEYS:
				UIGetString(szText, 254, IDS_TOOLTIP_PGPKEYS);
				bTooltip = TRUE;
				break;
			}

			if (bTooltip)
			{
				strcpy(pText->szText, szText);
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

	hDocHost = FindWindowEx(hwnd, NULL, "Ath_DocHost", NULL);
	hMessage = FindWindowEx(hDocHost, NULL, "Internet Explorer_Server", NULL);
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

		if(FYEO)
		{
			TempestViewer((void *)plugin->pgpContext,hwnd,
				szOutput,nOutLength);

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
		
			hNoteHdr = FindWindowEx(hwnd, NULL, "ATH_NoteHdr", NULL);
			hTo = FindWindowEx(hNoteHdr, NULL, "RICHEDIT", NULL);
			SetFocus(hTo);
			SetFocus(plugin->hEdit);

			EnableMenus(hwnd, plugin, FALSE);
		}
	}
	
	free(szInput);

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

	GetMenuItemInfo(plugin->hMainMenu, FILE_MENU_POS, TRUE, &menuInfo);
	EnableMenuItem(menuInfo.hSubMenu, FILESAVEAS_MENU_POS, uFlags);
	EnableMenuItem(menuInfo.hSubMenu, FILESTATIONERY_MENU_POS, uFlags);
	EnableMenuItem(menuInfo.hSubMenu, FILEMOVE_MENU_POS, uFlags);
	EnableMenuItem(menuInfo.hSubMenu, FILECOPY_MENU_POS, uFlags);
	EnableMenuItem(menuInfo.hSubMenu, FILEPRINT_MENU_POS, uFlags);
	
	EnableMenuItem(plugin->hMainMenu, EDIT_MENU_POS, uFlags);
	
	GetMenuItemInfo(plugin->hMainMenu, VIEW_MENU_POS, TRUE, &menuInfo);
	EnableMenuItem(menuInfo.hSubMenu, VIEWFONTS_MENU_POS, uFlags);
	EnableMenuItem(menuInfo.hSubMenu, VIEWLANG_MENU_POS, uFlags);

	DrawMenuBar(hwnd);

	SendMessage(plugin->hToolbar, TB_ENABLEBUTTON, IDC_SAVE,
		MAKELONG(bEnable, 0));
	SendMessage(plugin->hToolbar, TB_ENABLEBUTTON, IDC_PRINT,
		MAKELONG(bEnable, 0));
	SendMessage(plugin->hToolbar, TB_ENABLEBUTTON, IDC_COPY,
		MAKELONG(bEnable, 0));

	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
