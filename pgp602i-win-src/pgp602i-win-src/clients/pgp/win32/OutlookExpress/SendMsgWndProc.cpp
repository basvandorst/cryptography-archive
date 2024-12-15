/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: SendMsgWndProc.cpp,v 1.13.8.1 1998/11/12 03:12:48 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
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


BOOL CALLBACK SendMsgWndProc(HWND hDlg, 
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
			szText = GetReplyText();
			if (szText != NULL)
			{
				char *szReplyText;
				char *szReplyHeader;

				GetMessageText(hDlg, FALSE, &szReplyHeader);
				if (FindReplyHeader(szReplyHeader))
				{
					free(szReplyHeader);
					GetMessageText(hDlg, TRUE, &szReplyHeader);
					FindReplyHeader(szReplyHeader);
					szReplyText = (char *) calloc(strlen(szReplyHeader) +
						strlen(szText) * 2 + 1, 1);
					
					strcpy(szReplyText, szReplyHeader);
					AddReply(szReplyText, szText);
					SetMessageText(hDlg, szReplyText);
					free(szReplyText);
					KillTimer(hDlg, PASTE_REPLY_TIMER);
				}
				else
				{
					free(szReplyHeader);
					return 0;
				}
				
				free(szReplyHeader);
				FreeReplyText();
			}
			else
				KillTimer(hDlg, PASTE_REPLY_TIMER);

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
				{
					UIGetString(szText, 254, IDS_STATUS_SMIME_ENCRYPT);
					bPluginMenu = TRUE;
					break;
				}
								
				case IDC_SIGN_SMIME:
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
				{
					if (plugin->bEncrypt || plugin->bSign)
						if (!EncryptSignMessage(hDlg))
							return 0;

					break;
				}

				case IDC_ENCRYPT_PGP:
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

			case IDC_ENCRYPT_PGP:
				UIGetString(szText, 254, IDS_TOOLTIP_PGP_ENCRYPT);
				bTooltip = TRUE;
				break;

			case IDC_SIGN_PGP:
				UIGetString(szText, 254, IDS_TOOLTIP_PGP_SIGN);
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


BOOL EncryptSignMessage(HWND hwnd)
{
	HWND hAttach;
	char *szInput;
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
			
	if (!GetMessageText(hwnd, FALSE, &szInput))
		goto EncryptSignMessageError;

	dwLength = strlen(szInput);
	FixBadSpaces(szInput);

	nError = EncryptSignBuffer(UIGetInstance(), hwnd, plugin->pgpContext, 
				plugin->tlsContext, szExe, szDll, szInput, dwLength, prds, 
				NULL, &signOptions, (void **) &szOutput, &nOutLength, 
				plugin->bEncrypt, plugin->bSign, FALSE);

	free(szInput);

	if (IsntPGPError(nError))
	{
		if (!SetMessageText(hwnd, szOutput))
			PGPFreeData(szOutput);
		else
		{
			PGPFreeData(szOutput);
			plugin->bEncrypt = FALSE;
			plugin->bSign = FALSE;
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


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
