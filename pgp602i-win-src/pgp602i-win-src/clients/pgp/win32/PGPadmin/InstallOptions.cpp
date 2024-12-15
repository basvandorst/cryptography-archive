/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: InstallOptions.cpp,v 1.4.8.1 1998/11/12 03:13:08 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "pgpClientPrefs.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"

BOOL CALLBACK InstallOptionsDlgProc(HWND hwndDlg, 
									UINT uMsg, 
									WPARAM wParam, 
									LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
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
			SetWindowPos (GetParent(hwndDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);

			pConfig = (pgpConfigInfo *) ppspConfig->lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, (LPARAM) pConfig);
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
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->bPreselectInstall)
					{
						CheckDlgButton(hwndDlg, IDC_PRESELECT, BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_USERINSTALLDIR), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLPROGRAMS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLDISK), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLEUDORA), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLOUTLOOK), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLOUTEXPRESS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLMANUAL), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_UNINSTALL), TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_PRESELECT, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_USERINSTALLDIR), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLPROGRAMS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLDISK), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLEUDORA), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLOUTLOOK), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLOUTEXPRESS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_INSTALLMANUAL), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, 
										IDC_UNINSTALL), FALSE);
					}

					if (pConfig->szUserInstallDir == NULL)
					{
						SetWindowText(GetDlgItem(hwndDlg, 
										IDC_USERINSTALLDIR), "");
					}
					else
					{
						SetWindowText(GetDlgItem(hwndDlg, 
										IDC_USERINSTALLDIR), 
										pConfig->szUserInstallDir);
					}

					if (pConfig->bInstallPrograms)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
							BST_UNCHECKED);
					}

					if (pConfig->bInstallDisk)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLDISK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLDISK, 
							BST_UNCHECKED);
					}

					if (pConfig->bInstallEudora)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLEUDORA, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLEUDORA, 
							BST_UNCHECKED);
					}

					if (pConfig->bInstallOutlook)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLOUTLOOK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLOUTLOOK, 
							BST_UNCHECKED);
					}

					if (pConfig->bInstallOutExpress)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLOUTEXPRESS, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLOUTEXPRESS, 
							BST_UNCHECKED);
					}

					if (pConfig->bInstallManual)
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLMANUAL, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INSTALLMANUAL, 
							BST_UNCHECKED);
					}

					if (pConfig->bUninstallOld)
					{
						CheckDlgButton(hwndDlg, IDC_UNINSTALL, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_UNINSTALL, 
							BST_UNCHECKED);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					int nLength;

					if (pConfig->szUserInstallDir)
					{
						PGPFreeData(pConfig->szUserInstallDir);
						pConfig->szUserInstallDir = NULL;
					}

					nLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
													IDC_USERINSTALLDIR));

					if (nLength)
					{
						pConfig->szUserInstallDir = (char *)
							PGPNewData(pConfig->memoryMgr,
								nLength+1, kPGPMemoryMgrFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, 
										IDC_USERINSTALLDIR),
										pConfig->szUserInstallDir, 
										nLength+1);
						pConfig->szUserInstallDir[nLength] = 0;
					}

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
		case IDC_PRESELECT:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_PRESELECT) == 
					BST_CHECKED)
				{
					pConfig->bPreselectInstall = TRUE;
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_USERINSTALLDIR), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLPROGRAMS), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLDISK), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLEUDORA), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLOUTLOOK), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLOUTEXPRESS), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLMANUAL), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_UNINSTALL), TRUE);
				}
				else
				{
					pConfig->bPreselectInstall = FALSE;
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_USERINSTALLDIR), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLPROGRAMS), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLDISK), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLEUDORA), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLOUTLOOK), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLOUTEXPRESS), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_INSTALLMANUAL), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, 
									IDC_UNINSTALL), FALSE);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLPROGRAMS:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLPROGRAMS) == 
					BST_CHECKED)
				{
					pConfig->bInstallPrograms = TRUE;
				}
				else
				{
					pConfig->bInstallPrograms = FALSE;
					pConfig->bInstallEudora = FALSE;
					pConfig->bInstallOutlook = FALSE;
					pConfig->bInstallOutExpress = FALSE;
					pConfig->bUninstallOld = FALSE;

					CheckDlgButton(hwndDlg, IDC_INSTALLEUDORA, 
						BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_INSTALLOUTLOOK, 
						BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_INSTALLOUTEXPRESS, 
						BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_UNINSTALL, 
						BST_UNCHECKED);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLEUDORA:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLEUDORA) == 
					BST_CHECKED)
				{
					pConfig->bInstallEudora = TRUE;
					pConfig->bInstallPrograms = TRUE;
					CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
						BST_CHECKED);
				}
				else
				{
					pConfig->bInstallEudora = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLOUTLOOK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLOUTLOOK) == 
					BST_CHECKED)
				{
					pConfig->bInstallOutlook = TRUE;
					pConfig->bInstallPrograms = TRUE;
					CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
						BST_CHECKED);
				}
				else
				{
					pConfig->bInstallOutlook = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLOUTEXPRESS:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLOUTEXPRESS) == 
					BST_CHECKED)
				{
					pConfig->bInstallOutExpress = TRUE;
					pConfig->bInstallPrograms = TRUE;
					CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
						BST_CHECKED);
				}
				else
				{
					pConfig->bInstallOutExpress = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLMANUAL:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLMANUAL) == 
					BST_CHECKED)
				{
					pConfig->bInstallManual = TRUE;
				}
				else
				{
					pConfig->bInstallManual = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_INSTALLDISK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_INSTALLDISK) == 
					BST_CHECKED)
				{
					pConfig->bInstallDisk = TRUE;
				}
				else
				{
					pConfig->bInstallDisk = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_UNINSTALL:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_UNINSTALL) == 
					BST_CHECKED)
				{
					pConfig->bUninstallOld = TRUE;
					pConfig->bInstallPrograms = TRUE;
					CheckDlgButton(hwndDlg, IDC_INSTALLPROGRAMS, 
						BST_CHECKED);
				}
				else
				{
					pConfig->bUninstallOld = FALSE;
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
