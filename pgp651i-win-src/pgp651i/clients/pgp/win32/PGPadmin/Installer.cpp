/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Installer.cpp,v 1.3 1999/03/31 22:19:44 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "PGPcl.h"

BOOL CALLBACK InstallerDlgProc(HWND hwndDlg, 
							   UINT uMsg, 
							   WPARAM wParam, 
							   LPARAM lParam)
{
	BOOL			bReturnCode	= FALSE;
	pgpConfigInfo *	pConfig		= NULL;

	g_hCurrentDlgWnd = hwndDlg;

	if (uMsg != WM_INITDIALOG)
		pConfig = (pgpConfigInfo *) GetWindowLong(hwndDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc;
			PROPSHEETPAGE *ppspConfig = (PROPSHEETPAGE *) lParam;

			// center dialog on screen
			GetWindowRect(GetParent(hwndDlg), &rc);
			SetWindowPos(GetParent(hwndDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);

			pConfig = (pgpConfigInfo *) ppspConfig->lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LPARAM) pConfig);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_PAINT:
		if (pConfig->hPalette)
		{
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint (hwndDlg, &ps);
			SelectPalette (hDC, pConfig->hPalette, FALSE);
			RealizePalette (hDC);
			EndPaint (hwndDlg, &ps);
			bReturnCode = TRUE;
		}
		break;
		
	case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
			case PSN_SETACTIVE:
				{
					// Initialize window

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					PostMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
						PSWIZB_NEXT | PSWIZB_BACK);

					if (pConfig->szAdminInstaller == NULL)
						SetWindowText(GetDlgItem(hwndDlg, IDC_ADMININSTALLER),
							"");
					else
						SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER), 
							pConfig->szAdminInstaller);

					if (pConfig->szClientInstaller == NULL)
						SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER),
							"");
					else
						SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER), 
							pConfig->szClientInstaller);

					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZNEXT:
				{
					char szMsg[255];
					char szTitle[255];
					char szAdmin[MAX_PATH];
					char szClient[MAX_PATH];
					HANDLE hFile;
					
					if (!GetWindowTextLength(GetDlgItem(hwndDlg, 
						IDC_ADMININSTALLER)))
					{
						LoadString(g_hInstance, IDS_E_BLANKADMIN,
							szMsg, 254);
						LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
						MessageBox(hwndDlg, szMsg, szTitle, 
							MB_ICONEXCLAMATION);
						
						SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
						bReturnCode = TRUE;
						break;
					}

					if (!GetWindowTextLength(GetDlgItem(hwndDlg, 
						IDC_CLIENTINSTALLER)))
					{
						LoadString(g_hInstance, IDS_E_BLANKCLIENT,
							szMsg, 254);
						LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
						MessageBox(hwndDlg, szMsg, szTitle, 
							MB_ICONEXCLAMATION);
						
						SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
						bReturnCode = TRUE;
						break;
					}

					GetWindowText(GetDlgItem(hwndDlg, IDC_ADMININSTALLER),
						szAdmin, MAX_PATH-1);
					GetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER),
						szClient, MAX_PATH-1);

					if (!strcmp(szAdmin, szClient))
					{
						LoadString(g_hInstance, IDS_E_INSTALLDIR,
							szMsg, 254);
						LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
						MessageBox(hwndDlg, szMsg, szTitle, 
							MB_ICONEXCLAMATION);
						
						SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
					}
					
					hFile = CreateFile(szClient, GENERIC_READ | GENERIC_WRITE,
								0, NULL, CREATE_ALWAYS, 
								FILE_ATTRIBUTE_NORMAL, NULL);

					if ((hFile == INVALID_HANDLE_VALUE) &&
						!SetFileAttributes(szClient, FILE_ATTRIBUTE_NORMAL))
					{

						LoadString(g_hInstance, IDS_E_CANTCOPY, szMsg, 254);
						LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
						MessageBox(hwndDlg, szMsg, szTitle, 
							MB_ICONEXCLAMATION);

						SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
					}
					else
						CloseHandle(hFile);

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					int nDirLength;

					if (pConfig->szAdminInstaller)
					{
						PGPFreeData(pConfig->szAdminInstaller);
						pConfig->szAdminInstaller = NULL;
					}

					nDirLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
														IDC_ADMININSTALLER));

					if (nDirLength > 0)
					{
						pConfig->szAdminInstaller = (char *)
							PGPNewData(pConfig->memoryMgr,
								nDirLength + 1,
								kPGPMemoryMgrFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, IDC_ADMININSTALLER),
							pConfig->szAdminInstaller, nDirLength+1);

						pConfig->szAdminInstaller[nDirLength] = 0;
					}
					
					if (pConfig->szClientInstaller)
					{
						PGPFreeData(pConfig->szClientInstaller);
						pConfig->szClientInstaller = NULL;
					}

					nDirLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
														IDC_CLIENTINSTALLER));

					if (nDirLength > 0)
					{
						pConfig->szClientInstaller = (char *)
							PGPNewData(pConfig->memoryMgr,
								nDirLength + 1,
								kPGPMemoryMgrFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER),
							pConfig->szClientInstaller, nDirLength+1);

						pConfig->szClientInstaller[nDirLength] = 0;
					}
					
					bReturnCode = TRUE;
					break;
				}

			case PSN_HELP:
				{
					// Display help
					break;
				}

			case PSN_QUERYCANCEL:
				{
					// User wants to quit
					g_bGotReloadMsg = FALSE;
					break;
				}
			}
			
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADMININSTALLER:
		case IDC_CLIENTINSTALLER:
			{
				char szAdminFilePath[MAX_PATH];
				char szClientFilePath[MAX_PATH];

				GetWindowText(GetDlgItem(hwndDlg, IDC_ADMININSTALLER),
					szAdminFilePath, MAX_PATH-1);
				GetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER),
					szClientFilePath, MAX_PATH-1);

				bReturnCode = TRUE;
				break;
			}

		case IDC_ADMINBROWSE:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				OPENFILENAME openFile;
				char szAdminFile[MAX_PATH];
				char szTitle[256];

				strcpy(szAdminFile, "\0");
				LoadString(g_hInstance, IDS_GETADMININSTALLER, szTitle, 
					255);

				openFile.lStructSize = sizeof(OPENFILENAME);
				openFile.hwndOwner = hwndDlg;
				openFile.lpstrFilter =	"Executable Files (*.exe)\0"
										"*.exe\0";
				openFile.lpstrCustomFilter = NULL;
				openFile.nFilterIndex = 1;
				openFile.lpstrFile = szAdminFile;
				openFile.nMaxFile = MAX_PATH-1;
				openFile.lpstrFileTitle = NULL;
				openFile.lpstrInitialDir = NULL;
				openFile.lpstrTitle = szTitle;
				openFile.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
									OFN_PATHMUSTEXIST;
				openFile.lpstrDefExt = "exe";

				if (GetOpenFileName(&openFile))
					SetWindowText(GetDlgItem(hwndDlg, IDC_ADMININSTALLER),
						szAdminFile);

				bReturnCode = TRUE;
			}
			break;

		case IDC_CLIENTBROWSE:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				OPENFILENAME openFile;
				char szClientFile[MAX_PATH];
				char szTitle[256];

				strcpy(szClientFile, "\0");
				LoadString(g_hInstance, IDS_GETCLIENTINSTALLER, szTitle, 
					255);

				openFile.lStructSize = sizeof(OPENFILENAME);
				openFile.hwndOwner = hwndDlg;
				openFile.lpstrFilter =	"Executable Files (*.exe)\0"
										"*.exe\0";
				openFile.lpstrCustomFilter = NULL;
				openFile.nFilterIndex = 1;
				openFile.lpstrFile = szClientFile;
				openFile.nMaxFile = MAX_PATH-1;
				openFile.lpstrFileTitle = NULL;
				openFile.lpstrInitialDir = NULL;
				openFile.lpstrTitle = szTitle;
				openFile.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
				openFile.lpstrDefExt = "exe";

				if (GetSaveFileName(&openFile))
					SetWindowText(GetDlgItem(hwndDlg, IDC_CLIENTINSTALLER),
						szClientFile);

				bReturnCode = TRUE;
			}
			break;
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
