/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: CommonWndProc.cpp,v 1.7.8.1 1998/11/12 03:12:43 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "PluginInfo.h"
#include "HookProcs.h"
#include "UIutils.h"
#include "PGPcl.h"
#include "resource.h"


BOOL CALLBACK CommonWndProc(HWND hDlg, 
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
	case WM_DESTROY:
		{
			FreePluginInfo(plugin);
			break;
		}

	case WM_MENUSELECT:
		{
			UINT uFlags = (UINT) HIWORD(wParam);
			char szText[255];
			BOOL bPluginMenu = FALSE;

			if (uFlags & MF_POPUP)
			{
				if (LOWORD(wParam) == plugin->nPGPPosition)
				{
					UIGetString(szText, 254, IDS_STATUS_PGP);
					bPluginMenu = TRUE;
				}
			}

			switch(LOWORD(wParam))
			{
				case IDC_PREFS:
				{
					UIGetString(szText, 254, IDS_STATUS_PREFS);
					bPluginMenu = TRUE;
					break;
				}
								
				case IDC_PGPKEYS:
				{
					UIGetString(szText, 254, IDS_STATUS_PGPKEYS);
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
			case IDC_PREFS:
				{
					PGPclPreferences(plugin->pgpContext, hDlg, 0, NULL);
					return 0;
				}

			case IDC_PGPKEYS:
				{
					char szPath[MAX_PATH];
					char szPGPkeys[256];
					PGPError err;
					
					UIGetString(szPGPkeys, sizeof(szPGPkeys), IDS_PGPKEYSEXE);
					err = PGPclGetPGPPath(szPath, MAX_PATH-1);
					if (IsPGPError(err))
					{
						UIDisplayStringID(hDlg, IDS_E_NOPGPKEYS);
						return 0;
					}

					strcat(szPath, szPGPkeys);
					WinExec(szPath, SW_SHOW);
					return 0;
				}
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
