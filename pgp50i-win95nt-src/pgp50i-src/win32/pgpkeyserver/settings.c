/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#define PGPLIB
#define KEYSERVER_DLL

#include "..\include\config.h"
#include "..\include\pgpkeyserversupport.h"

#include "keyserver.h"
#include "resource.h"
#include "settings.h"
#include "ksconfig.h"

#include "PGPkeyserverHelp.h"

static DWORD aIds[] = {			// Help IDs
	IDC_SERVER,				IDH_PGPKSPREF_SERVER,
	IDC_PORT,				IDH_PGPKSPREF_PORT,
	IDC_CHECK_AUTO_GET,		IDH_PGPKSPREF_AUTOGET,
	IDC_BUTTON_RESET,		IDH_PGPKSPREF_RESET,
    0,0
};


//----------------------------------------------------|
// get PGPkeys path from registry and substitute Help file name

VOID GetHelpFilePath (LPSTR lpszFile, INT iLen) {
	HKEY hKey;
	LONG lResult;
	CHAR sz[MAX_PATH];
	CHAR* p;
	DWORD dwValueType, dwSize;
	CHAR szBuf[256];

	lstrcpy (lpszFile, "");

	LoadString (g_hinst, IDS_REGISTRYKEY, szBuf, sizeof(szBuf));
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, szBuf, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS) {
		dwSize = iLen;
		lResult = RegQueryValueEx (hKey, "ExePath", 0, &dwValueType,
									(LPBYTE)lpszFile, &dwSize);
		RegCloseKey (hKey);
		p = strrchr (lpszFile, '\\');
		if (p) {
			p++;
			*p = '\0';
		}
		else lstrcpy (lpszFile, "");
	}

	LoadString (g_hinst, IDS_HELPFILENAME, sz, sizeof(sz));
	lstrcat (lpszFile, sz);
}


KSERR PGPExport PGPkeyserverSetupKeyServerSettingsPage(PROPSHEETPAGE *pPSP)
{
	KSERR ReturnCode = SUCCESS;

	assert(pPSP);

	if(pPSP)
	{
		pPSP->dwSize = sizeof(PROPSHEETPAGE);
		pPSP->dwFlags = PSP_USETITLE|PSP_HASHELP;
		pPSP->hInstance = g_hinst;
		pPSP->pszTemplate = MAKEINTRESOURCE(IDD_KSPREF);
		pPSP->pszIcon = NULL;
		pPSP->pfnDlgProc = KeyServerPropDlgProc;
		pPSP->pszTitle = "KeyServer";
		pPSP->lParam = 0;
		pPSP->pfnCallback = NULL;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

//----------------------------------------------------|
//  Message processing function for property sheet dialog
LRESULT WINAPI KeyServerPropDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
									LPARAM lParam)
{
	char Keyserver[255];
	unsigned short Port;
	BOOL AutoGet;
	static CHAR szHelpFile[MAX_PATH];

	switch (uMsg)
    {
	
		case WM_INITDIALOG:
			GetHelpFilePath (szHelpFile, sizeof(szHelpFile));
			if(GetKeyserverValues(Keyserver, &Port, &AutoGet) == SUCCESS)
			{
				SetDlgItemText(hDlg, IDC_SERVER, Keyserver);
				SetDlgItemInt(hDlg, IDC_PORT, Port, FALSE);
				if(AutoGet)
					SendDlgItemMessage (hDlg, IDC_CHECK_AUTO_GET, BM_SETCHECK,
										BST_CHECKED, 0);
				else
					SendDlgItemMessage (hDlg, IDC_CHECK_AUTO_GET, BM_SETCHECK,
										BST_UNCHECKED, 0);

			}
			return TRUE;

		case WM_HELP:
			WinHelp (((LPHELPINFO) lParam)->hItemHandle, szHelpFile,
				HELP_WM_HELP, (DWORD) (LPSTR) aIds);
			break;

		case WM_CONTEXTMENU:
			WinHelp ((HWND) wParam, szHelpFile, HELP_CONTEXTMENU,
				(DWORD) (LPVOID) aIds);
			break;

		case WM_COMMAND :
			switch (LOWORD (wParam))
			{
				case IDC_BUTTON_RESET:
				{
					SetKeyserverValues(DEFAULT_KEYSERVER, 11371, FALSE);
					SetDlgItemText(hDlg, IDC_SERVER, DEFAULT_KEYSERVER);
					SetDlgItemInt(hDlg, IDC_PORT, 11371, FALSE);
					SendDlgItemMessage (hDlg, IDC_CHECK_AUTO_GET, BM_SETCHECK,
										BST_UNCHECKED, 0);
					break;
				}
			}
			break;

		case WM_NOTIFY :
			switch (((NMHDR FAR *) lParam)->code) {
			case PSN_SETACTIVE :
				break;

			case PSN_HELP :
				WinHelp (hDlg, szHelpFile, HELP_CONTEXT,
					IDH_PGPKSPREF_DIALOG);
				break;

			case PSN_APPLY :
			{
				BOOL Translated;
				/*Read keyserver stuff here*/
				
//				uReturnValue = PGPCOMDLG_OK;
				GetDlgItemText(hDlg, IDC_SERVER, Keyserver, sizeof(Keyserver));
				Port = GetDlgItemInt(hDlg, IDC_PORT, &Translated, FALSE);
				if(SendDlgItemMessage(hDlg, IDC_CHECK_AUTO_GET, BM_GETCHECK,
										0, 0) == BST_CHECKED)
					AutoGet = TRUE;
				else
					AutoGet = FALSE;

				SetKeyserverValues(Keyserver, Port, AutoGet);
				SetWindowLong (hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				return TRUE;
			}

/*
			case PSN_KILLACTIVE :
				MessageBox(NULL, "KillActive", "Wubba", MB_OK);
				SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
				return TRUE;
*/

			case PSN_RESET :
//				uReturnValue = PGPCOMDLG_CANCEL;
				if(GetKeyserverValues(Keyserver, &Port, &AutoGet) == SUCCESS)
				{
					SetDlgItemText(hDlg, IDC_SERVER, Keyserver);
					SetDlgItemInt(hDlg, IDC_PORT, Port, FALSE);
				}
				SetWindowLong (hDlg, DWL_MSGRESULT, FALSE);
				break;
		}
	}

	return FALSE;

}
