/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Finish.cpp,v 1.16.8.1 1998/11/12 03:13:07 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <shlobj.h>
#include "resource.h"
#include "pgpKeys.h"
#include "pgpMem.h"
#include "PGPcl.h"

BOOL CALLBACK FinishDlgProc(HWND hwndDlg, 
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
					PostMessage(GetParent(hwndDlg),
						PSM_SETFINISHTEXT, 0, (LPARAM) "&Save");

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->szInstallDir == NULL)
					{
						PostMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
							PSWIZB_DISABLEDFINISH | PSWIZB_BACK);
						SetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR),
							"");
					}
					else
					{
						PostMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
							PSWIZB_FINISH | PSWIZB_BACK);
						SetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR), 
							pConfig->szInstallDir);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZFINISH:
				{
					int nDirLength;
					char szPGPPath[MAX_PATH];
					char szClientInstaller[255];
					char szMsg[255];
					char szTitle[255];
					
					if (g_bGotReloadMsg == TRUE)
						break;

					if (pConfig->szInstallDir)
					{
						PGPFreeData(pConfig->szInstallDir);
						pConfig->szInstallDir = NULL;
					}
					
					nDirLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
						IDC_INSTALLDIR));
					
					if (nDirLength > 0)
					{
						pConfig->szInstallDir = (char *)
							PGPNewData(pConfig->memoryMgr,
							nDirLength + 2,
							kPGPMemoryMgrFlags_Clear);
						
						GetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR),
							pConfig->szInstallDir, nDirLength+1);

						if (pConfig->szInstallDir[nDirLength-1] != '\\')
						{
							strcat(pConfig->szInstallDir, "\\");
							nDirLength++;
						}

						pConfig->szInstallDir[nDirLength] = 0;

						PGPclGetPGPPath(szPGPPath, MAX_PATH-1);
						LoadString(g_hInstance, IDS_CLIENTINSTALLER,
							szClientInstaller, 254);
						strcat(szPGPPath, szClientInstaller);

						if (strstr(pConfig->szInstallDir, szPGPPath) != NULL)
						{
							LoadString(g_hInstance, IDS_E_INSTALLDIR,
								szMsg, 254);
							LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
							MessageBox(hwndDlg, szMsg, szTitle, 
								MB_ICONEXCLAMATION);
	
							SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
						}
						else
							pConfig->bSave = TRUE;
					}
					
					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					int nDirLength;

					if (pConfig->szInstallDir)
					{
						PGPFreeData(pConfig->szInstallDir);
						pConfig->szInstallDir = NULL;
					}

					nDirLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
														IDC_INSTALLDIR));

					if (nDirLength > 0)
					{
						pConfig->szInstallDir = (char *)
							PGPNewData(pConfig->memoryMgr,
								nDirLength + 1,
								kPGPMemoryMgrFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR),
							pConfig->szInstallDir, nDirLength+1);

						if (pConfig->szInstallDir[nDirLength-1] != '\\')
						{
							strcat(pConfig->szInstallDir, "\\");
							nDirLength++;
						}

						pConfig->szInstallDir[nDirLength] = 0;
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
		case IDC_INSTALLDIR:
			{
				char szFilePath[MAX_PATH];

				GetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR),
					szFilePath, MAX_PATH-1);

				if (strlen(szFilePath) > 0)
					SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
						PSWIZB_FINISH | PSWIZB_BACK);
				else
					SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
						PSWIZB_DISABLEDFINISH | PSWIZB_BACK);

				bReturnCode = TRUE;
				break;
			}

		case IDC_BROWSE:
			{
				LPMALLOC pMalloc;
				BROWSEINFO bi;
				LPITEMIDLIST pidlInstallDir;
				char szInstallDir[MAX_PATH];
				char szInstallTitle[255];
				int nDirLength;

				SHGetMalloc(&pMalloc);
				LoadString(g_hInstance, IDS_CHOOSEDIR, szInstallTitle, 
					254);
				
				bi.hwndOwner = hwndDlg;
				bi.pidlRoot = NULL;
				bi.pszDisplayName = szInstallDir;
				bi.lpszTitle = szInstallTitle;
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

					SetWindowText(GetDlgItem(hwndDlg, IDC_INSTALLDIR),
						szInstallDir);
					pMalloc->Free(pidlInstallDir);

					SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, 0, 
						PSWIZB_FINISH | PSWIZB_BACK);
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
