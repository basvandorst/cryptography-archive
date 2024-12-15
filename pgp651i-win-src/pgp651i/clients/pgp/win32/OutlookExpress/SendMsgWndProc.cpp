/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: SendMsgWndProc.cpp,v 1.18 1999/04/26 19:13:24 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "TextIO.h"
#include "Recipients.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpOptionList.h"
#include "pgpSDKPrefs.h"
#include "resource.h"
#include "EncryptSign.h"

static BOOL EncryptSignMessage(HWND hwnd);
static void DoubleLineFeeds(char *szOriginal, char *szNew);
static void UpdateMenus(HWND hwnd, PluginInfo *plugin);

typedef struct _AttStruct
{
	char *String0;
	char *String1;
	HANDLE hFile0;
	HANDLE hFile1;
} AttStruct;


LRESULT CALLBACK SendMsgWndProc(HWND hDlg, 
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
			{
				if (plugin->nAction == PluginAction_Encrypt)
				{
					if (EncryptSignMessage(hDlg))
						_beginthread(DoPasteThread, 0, hDlg);
					else
						RestoreClipboardText(hDlg);
				}
				else if (plugin->nAction == PluginAction_Reply)
				{
					char *szText;

					szText = GetReplyText();
					if (szText != NULL)
					{
						char *szReplyText;
						char *szReplyHeader;
						
						GetMessageText(hDlg, FALSE, &szReplyHeader);
						if (FindReplyHeader(szReplyHeader))
						{
							szReplyText = (char *) 
								calloc(strlen(szReplyHeader) +
									strlen(szText) * 2 + 1, 1);
							
							strcpy(szReplyText, szReplyHeader);
							AddReply(szReplyText, szText);
							SetMessageText(hDlg, szReplyText);
							free(szReplyText);
							_beginthread(DoPasteThread, 0, hDlg);
						}
						else
							RestoreClipboardText(hDlg);
						
						free(szReplyHeader);
						FreeReplyText();
					}
					else
						RestoreClipboardText(hDlg);
				}
				else
					RestoreClipboardText(hDlg);
			}
			else
				RestoreClipboardText(hDlg);

			return 0;
		}
	
		if (msg == plugin->nPasteDoneMsg)
		{
			RestoreClipboardText(hDlg);
			if (plugin->hDlg != NULL)
				return CallWindowProc(lpOldProc, plugin->hDlg, plugin->nMsg, 
							plugin->wParam, plugin->lParam);
			else
				return 0;
		}
	}

	switch(msg)
	{
	case WM_DESTROY:
		{
			KillTimer(hDlg, WATCH_TOOLBAR_TIMER);
			break;
		}

	case WM_TIMER:
		if (wParam == CORRECT_TOOLBAR_TIMER)
		{
			KillTimer(hDlg, CORRECT_TOOLBAR_TIMER);

			SendMessage(plugin->hToolbar, TB_SETBUTTONWIDTH, 0,
				MAKELONG(24, 24));
			SendMessage(plugin->hToolbar, TB_AUTOSIZE, 0, 0);
			UpdateMenus(hDlg, plugin);

			return 0;
		}
		else if (wParam == PASTE_REPLY_TIMER)
		{
			char *szText;
			
			// Paste any reply text
			KillTimer(hDlg, PASTE_REPLY_TIMER);
			szText = GetReplyText();
			if (szText != NULL)
			{
				HWND hDocHost;
				HWND hMimeEdit;
				HWND hMessage;
				
				if (!SaveClipboardText(hDlg))
					return 0;
				
				plugin->nAction = PluginAction_Reply;
				plugin->hDlg = NULL;
				
				if (plugin->bOE5)
				{
					hDocHost = FindWindowEx(hDlg, NULL, "ME_DocHost", NULL);
					hMimeEdit = FindWindowEx(hDocHost, NULL, 
									"##MimeEdit_Server", NULL);
					hMessage = FindWindowEx(hMimeEdit, NULL, 
									"Internet Explorer_Server", NULL);
					
					SetFocus(hDocHost);
					SetFocus(hMimeEdit);
					SetFocus(hMessage);
				}
				else
				{
					hDocHost = FindWindowEx(hDlg, NULL, "Ath_DocHost", NULL);
					hMessage = FindWindowEx(hDocHost, NULL, 
									"Internet Explorer_Server", NULL);
					
					SetFocus(hDocHost);
					SetFocus(hMessage);
				}
				
				_beginthread(DoCopyThread, 0, hDlg);
			}

			return 0;
		}
		break;

	case WM_MENUSELECT:
		{
			UINT uFlags = (UINT) HIWORD(wParam);
			char szText[255];
			BOOL bPluginMenu = FALSE;

			if (uFlags & MF_POPUP)
			{
				if (LOWORD(wParam) == plugin->nPGPPosition+1)
					UpdateMenus(hDlg, plugin);
			}

			switch(LOWORD(wParam))
			{
				case IDC_ENCRYPT_SMIME:
				case IDC_OE5_ENCRYPT_SMIME:
				{
					UIGetString(szText, 254, IDS_STATUS_SMIME_ENCRYPT);
					bPluginMenu = TRUE;
					break;
				}
								
				case IDC_SIGN_SMIME:
				case IDC_OE5_SIGN_SMIME:
				{
					UIGetString(szText, 254, IDS_STATUS_SMIME_SIGN);
					bPluginMenu = TRUE;
					break;
				}

				case IDC_ENCRYPT_PGP:
				{
					UIGetString(szText, 254, IDS_STATUS_PGP_ENCRYPT);
					bPluginMenu = TRUE;
					break;
				}
								
				case IDC_SIGN_PGP:
				{
					UIGetString(szText, 254, IDS_STATUS_PGP_SIGN);
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
			case IDC_SENDMESSAGE_MENU:
			case IDC_SENDMESSAGE_BUTTON:
			case IDC_SENDLATER:
			case IDC_POSTARTICLE_MENU:
			case IDC_POSTARTICLE_BUTTON:
			case IDC_POSTLATER:
			case IDC_OE5_SENDMESSAGE:
			case IDC_OE5_SENDLATER:
				{
					if (plugin->bEncrypt || plugin->bSign)
					{
						HWND hDocHost;
						HWND hMimeEdit;
						HWND hMessage;

						if (!SaveClipboardText(hDlg))
							return 0;

						plugin->nAction = PluginAction_Encrypt;
						plugin->hDlg = hDlg;
						plugin->nMsg = msg;
						plugin->wParam = wParam;
						plugin->lParam = lParam;
						
						if (plugin->bOE5)
						{
							hDocHost = FindWindowEx(hDlg, NULL, 
											"ME_DocHost", NULL);
							hMimeEdit = FindWindowEx(hDocHost, NULL, 
											"##MimeEdit_Server", NULL);
							hMessage = FindWindowEx(hMimeEdit, NULL, 
											"Internet Explorer_Server", NULL);
							
							SetFocus(hDocHost);
							SetFocus(hMimeEdit);
							SetFocus(hMessage);
						}
						else
						{
							hDocHost = FindWindowEx(hDlg, NULL, 
											"Ath_DocHost", NULL);
							hMessage = FindWindowEx(hDocHost, NULL, 
											"Internet Explorer_Server", NULL);
							
							SetFocus(hDocHost);
							SetFocus(hMessage);
						}
						
						_beginthread(DoCopyThread, 0, hDlg);
						return 0;
					}

					break;
				}

			case IDC_ENCRYPT_PGP:
				if (plugin->bOE5)
				{
					plugin->bEncrypt = !(plugin->bEncrypt);
					SendMessage(plugin->hToolbar, TB_CHECKBUTTON, 
						IDC_ENCRYPT_PGP, MAKELONG(plugin->bEncrypt, 0));
					return 0;
				}
				else
				{
					MENUITEMINFO menuInfo;

					menuInfo.cbSize = sizeof(MENUITEMINFO);
					menuInfo.fMask = MIIM_STATE;
					GetMenuItemInfo(plugin->hToolsMenu, IDC_ENCRYPT_PGP, 
						FALSE, &menuInfo);
					
					if (menuInfo.fState & MFS_CHECKED)
						plugin->bEncrypt = FALSE;
					else
						plugin->bEncrypt = TRUE;

					UpdateMenus(hDlg, plugin);
					return 0;
				}

			case IDC_SIGN_PGP:
				if (plugin->bOE5)
				{
					plugin->bSign = !(plugin->bSign);
					SendMessage(plugin->hToolbar, TB_CHECKBUTTON, 
						IDC_SIGN_PGP, MAKELONG(plugin->bSign, 0));
					return 0;
				}
				else
				{
					MENUITEMINFO menuInfo;

					menuInfo.cbSize = sizeof(MENUITEMINFO);
					menuInfo.fMask = MIIM_STATE;
					GetMenuItemInfo(plugin->hToolsMenu, IDC_SIGN_PGP, 
						FALSE, &menuInfo);
					
					if (menuInfo.fState & MFS_CHECKED)
						plugin->bSign = FALSE;
					else
						plugin->bSign = TRUE;

					UpdateMenus(hDlg, plugin);
					return 0;
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
			char szMenu[256];
			LRESULT lResult;
			
			UIGetString(szAddrBook, 254, IDS_MENU_OEADDRBOOK);
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
				
				strcpy(szMenu, "&PGP");
				
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU | 
									MIIM_STATE;
				menuInfo.fType = MFT_STRING;
				menuInfo.wID = IDC_PGPMENU;
				menuInfo.fState = MFS_ENABLED;
				menuInfo.hSubMenu = plugin->hPGPMenu;
				menuInfo.dwTypeData = szMenu;

				InsertMenuItem((HMENU) wParam, nAddrBook, TRUE, &menuInfo);
				plugin->nPGPPosition = nAddrBook;

				UIGetString(szMenu, 254, IDS_MENU_SMIME_ENCRYPT);
				menuInfo.fMask = MIIM_TYPE;
				menuInfo.fType = MFT_STRING;
				menuInfo.dwTypeData = szMenu;

				SetMenuItemInfo((HMENU) wParam, OE5_ENCRYPT_SMIME_POS, TRUE, 
					&menuInfo);

				UIGetString(szMenu, 254, IDS_MENU_PGP_ENCRYPT);
				menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
				menuInfo.fType = MFT_STRING;
				menuInfo.wID = IDC_ENCRYPT_PGP;
				menuInfo.dwTypeData = szMenu;
				
				if (plugin->bEncrypt)
					menuInfo.fState = MFS_ENABLED | MFS_CHECKED;
				else
					menuInfo.fState = MFS_ENABLED;

				InsertMenuItem((HMENU) wParam, OE5_ENCRYPT_PGP_POS, TRUE, 
					&menuInfo);

				UIGetString(szMenu, 254, IDS_MENU_SMIME_SIGN);
				menuInfo.fMask = MIIM_TYPE;
				menuInfo.fType = MFT_STRING;
				menuInfo.dwTypeData = szMenu;

				SetMenuItemInfo((HMENU) wParam, OE5_SIGN_SMIME_POS, TRUE, 
					&menuInfo);

				UIGetString(szMenu, 254, IDS_MENU_PGP_SIGN);
				menuInfo.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
				menuInfo.fType = MFT_STRING;
				menuInfo.wID = IDC_SIGN_PGP;
				menuInfo.dwTypeData = szMenu;
				
				if (plugin->bSign)
					menuInfo.fState = MFS_ENABLED | MFS_CHECKED;
				else
					menuInfo.fState = MFS_ENABLED;

				InsertMenuItem((HMENU) wParam, OE5_SIGN_PGP_POS, TRUE, 
					&menuInfo);

				return lResult;
			}
			else if (!strcmp(szMenu, "&PGP"))
			{
				lResult = CallWindowProc(lpOldProc, hDlg, msg, wParam, 
							lParam);
				
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_STATE;
				
				if (plugin->bEncrypt)
					menuInfo.fState = MFS_ENABLED | MFS_CHECKED;
				else
					menuInfo.fState = MFS_ENABLED;

				SetMenuItemInfo((HMENU) wParam, IDC_ENCRYPT_PGP, FALSE, 
					&menuInfo);

				if (plugin->bSign)
					menuInfo.fState = MFS_ENABLED | MFS_CHECKED;
				else
					menuInfo.fState = MFS_ENABLED;

				SetMenuItemInfo((HMENU) wParam, IDC_SIGN_PGP, FALSE, 
					&menuInfo);
				
				menuInfo.fState = MFS_ENABLED;

				SetMenuItemInfo((HMENU) wParam, IDC_PGPMENU, FALSE, 
					&menuInfo);
				
				SetMenuItemInfo(plugin->hPGPMenu, IDC_PREFS, FALSE, 
					&menuInfo);
				
				SetMenuItemInfo(plugin->hPGPMenu, IDC_PGPKEYS, FALSE, 
					&menuInfo);
				
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

			case IDC_ENCRYPT_PGP:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_PGP_ENCRYPT;
				bTooltip = TRUE;
				break;

			case IDC_SIGN_PGP:
				pInfo->lpszText = (LPTSTR) IDS_TOOLTIP_PGP_SIGN;
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


BOOL EncryptSignMessage(HWND hwnd)
{
	HWND hAttach;
	char *szInput = NULL;
	char *szOutput;
	DWORD dwLength;
	PGPSize nOutLength;
	RECIPIENTDIALOGSTRUCT *prds = NULL;
	PGPOptionListRef signOptions = NULL;
	PluginInfo *plugin;
	BOOL bSuccess = FALSE;
	char szExe[256];
	char szDll[256];
	PGPError nError = kPGPError_NoErr;
	
	UIGetString(szExe, sizeof(szExe), IDS_EXE);
	UIGetString(szDll, sizeof(szDll), IDS_DLL);
	
	plugin = GetPluginInfo(hwnd);

	hAttach = FindWindowEx(hwnd, NULL, "SysListView32", NULL);
	
	if (hAttach)
		if (ListView_GetItemCount(hAttach) > 0)
			if (!UIWarnUser(hwnd, IDS_W_ATTACHMENT, "Attachment"))
				return FALSE;

	prds = (RECIPIENTDIALOGSTRUCT *) 
			calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

	nError = PGPsdkLoadDefaultPrefs(plugin->pgpContext);
	if (IsPGPError(nError))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, szDll, nError);
		goto EncryptSignMessageError;
	}

	nError = PGPOpenDefaultKeyRings(plugin->pgpContext, 
				(PGPKeyRingOpenFlags)0, &(prds->OriginalKeySetRef));

	if (IsPGPError(nError))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, szDll, nError);
		goto EncryptSignMessageError;
	}

	if (!GetMessageText(hwnd, FALSE, &szInput))
		goto EncryptSignMessageError;

	dwLength = strlen(szInput);
	FixBadSpaces(szInput);

	if (plugin->bEncrypt)
	{
		char *szRecipients;

		if (!GetRecipientText(hwnd, &szRecipients))
			goto EncryptSignMessageError;

		nError = GetRecipients(hwnd, szRecipients, plugin->pgpContext, 
					plugin->tlsContext, prds);
		free(szRecipients);
	}

	if (IsPGPError(nError))
	{
		if (nError != kPGPError_UserAbort)
			UIDisplayErrorCode(__FILE__, __LINE__, szDll, 
				nError);

		goto EncryptSignMessageError; 
	}
			
	nError = EncryptSignBuffer(UIGetInstance(), hwnd, plugin->pgpContext, 
				plugin->tlsContext, szExe, szDll, szInput, dwLength, prds, 
				NULL, &signOptions, (void **) &szOutput, &nOutLength, 
				plugin->bEncrypt, plugin->bSign, FALSE);

	free(szInput);
	szInput = NULL;

	if (IsntPGPError(nError))
	{
		if (!SetMessageText(hwnd, szOutput))
			PGPFreeData(szOutput);
		else
		{
			PGPFreeData(szOutput);
			plugin->bEncrypt = FALSE;
			plugin->bSign = FALSE;
			if (!(plugin->bOE5))
				UpdateMenus(hwnd, plugin);
			bSuccess = TRUE;
		}
	}

EncryptSignMessageError:

	if (signOptions != NULL)
	{
		PGPFreeOptionList(signOptions);
		signOptions = NULL;
	}

	if (prds->OriginalKeySetRef != NULL)
	{
		PGPFreeKeySet(prds->OriginalKeySetRef);
		prds->OriginalKeySetRef = NULL;
	}

	FreeRecipients(prds);
	free(prds);

	if (szInput != NULL)
		free(szInput);

	return bSuccess;
}


void DoubleLineFeeds(char *szOriginal, char *szNew)
{
	int nOrigIndex = 0;
	int nNewIndex = 0;
	BOOL bCRLF = FALSE;

	while (szOriginal[nOrigIndex] != '\0')
	{
		szNew[nNewIndex++] = szOriginal[nOrigIndex++];
		if (szOriginal[nOrigIndex-1] == '\n')
		{
			if (bCRLF)
			{
				szNew[nNewIndex++] = '\r';
				szNew[nNewIndex++] = '\n';
				bCRLF = FALSE;
			}
			else
				bCRLF = TRUE;
		}
	}

	szNew[nNewIndex] = '\0';
	return;
}


static void UpdateMenus(HWND hwnd, PluginInfo *plugin)
{
	if (plugin->bEncrypt)
	{
		CheckMenuItem(plugin->hToolsMenu, IDC_ENCRYPT_PGP, 
			MF_BYCOMMAND | MF_CHECKED);
		SendMessage(plugin->hToolbar, TB_CHECKBUTTON, IDC_ENCRYPT_PGP,
			MAKELONG(TRUE, 0));
	}
	else
	{
		CheckMenuItem(plugin->hToolsMenu, IDC_ENCRYPT_PGP, 
			MF_BYCOMMAND | MF_UNCHECKED);
		SendMessage(plugin->hToolbar, TB_CHECKBUTTON, IDC_ENCRYPT_PGP,
			MAKELONG(FALSE, 0));
	}
	
	if (plugin->bSign)
	{
		CheckMenuItem(plugin->hToolsMenu, IDC_SIGN_PGP, 
			MF_BYCOMMAND | MF_CHECKED);
		SendMessage(plugin->hToolbar, TB_CHECKBUTTON, IDC_SIGN_PGP,
			MAKELONG(TRUE, 0));
	}
	else
	{
		CheckMenuItem(plugin->hToolsMenu, IDC_SIGN_PGP, 
			MF_BYCOMMAND | MF_UNCHECKED);
		SendMessage(plugin->hToolbar, TB_CHECKBUTTON, IDC_SIGN_PGP,
			MAKELONG(FALSE, 0));
	}
	
}


LRESULT CALLBACK SendToolbarParentWndProc(HWND hDlg, 
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
							IDC_SIGN_PGP, 0);

				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);

				nIndex = SendMessage(plugin->hToolbar, TB_COMMANDTOINDEX,
							IDC_ENCRYPT_PGP, 0);

				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex, 0);
				SendMessage(plugin->hToolbar, TB_DELETEBUTTON, nIndex - 1, 0);
			}

			if (lpnm->code == TBN_ENDADJUST)
			{
				DWORD dwSize;
				int nX;
				int nY;
				TBBUTTON tbb[4];
				
				tbb[0].iBitmap = -1;
				tbb[0].idCommand = -1;
				tbb[0].fsState = TBSTATE_ENABLED;
				tbb[0].fsStyle = TBSTYLE_SEP;
				tbb[0].dwData = 0;
				tbb[0].iString = -1;

				tbb[1].iBitmap = plugin->nEncryptImage;
				tbb[1].idCommand = IDC_ENCRYPT_PGP;
				tbb[1].fsState = TBSTATE_ENABLED;
				tbb[1].fsStyle = TBSTYLE_CHECK;
				tbb[1].dwData = 0;
				tbb[1].iString = plugin->nEncryptString;
				
				tbb[2].iBitmap = plugin->nSignImage;
				tbb[2].idCommand = IDC_SIGN_PGP;
				tbb[2].fsState = TBSTATE_ENABLED;
				tbb[2].fsStyle = TBSTYLE_CHECK;
				tbb[2].dwData = 0;
				tbb[2].iString = plugin->nSignString;
				
				tbb[3].iBitmap = plugin->nPGPKeysImage;
				tbb[3].idCommand = IDC_PGPKEYS;
				tbb[3].fsState = TBSTATE_ENABLED;
				tbb[3].fsStyle = TBSTYLE_BUTTON;
				tbb[3].dwData = 0;
				tbb[3].iString = plugin->nPGPKeysString;
				
				dwSize = SendMessage(plugin->hToolbar, TB_GETBUTTONSIZE, 0, 0);
				nX = LOWORD(dwSize);
				nY = HIWORD(dwSize);
				
				SendMessage(plugin->hToolbar, TB_ADDBUTTONS, 4, 
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
