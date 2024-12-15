/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: FindInstaller.cpp,v 1.2.8.1 1998/11/12 03:13:06 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <shlobj.h>
#include "resource.h"

static char *szClientInstaller = NULL;

BOOL CALLBACK FindInstallerDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam)
{
	BOOL bReturnCode = FALSE;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc;

			// center dialog on screen
			GetWindowRect(GetParent(hwndDlg), &rc);
			SetWindowPos(GetParent(hwndDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);

			szClientInstaller = (char *) lParam;
			SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALL),
				szClientInstaller);

			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALL),
				szClientInstaller, MAX_PATH-1);
			EndDialog(hwndDlg, FALSE);
			bReturnCode = TRUE;
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, TRUE);
			bReturnCode = TRUE;
			break;

		case IDC_CLIENTINSTALL:
			{
				char szFilePath[MAX_PATH];

				GetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALL),
					szFilePath, MAX_PATH-1);

				if (strlen(szFilePath) > 0)
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

				bReturnCode = TRUE;
				break;
			}

		case IDC_BROWSE:
			{
				LPMALLOC pMalloc;
				BROWSEINFO bi;
				LPITEMIDLIST pidlInstallDir;
				char szInstallDir[MAX_PATH];
				int nDirLength;

				SHGetMalloc(&pMalloc);
				
				bi.hwndOwner = hwndDlg;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = szInstallDir;
				bi.lpszTitle = NULL;
				bi.ulFlags = 0;
				bi.lpfn = NULL;
				bi.lParam = 0;

				pidlInstallDir = SHBrowseForFolder(&bi);
				if (pidlInstallDir != NULL)
				{
					SHGetPathFromIDList(pidlInstallDir, szInstallDir);
					nDirLength = strlen(szInstallDir);
					if (szInstallDir[nDirLength-1] != '\\')
						strcat(szInstallDir, "\\");

					SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALL),
						szInstallDir);
					pMalloc->Free(pidlInstallDir);

					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				}

				bReturnCode = TRUE;
				break;
			}
		}
		
		break;
	}

	return(bReturnCode);
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
