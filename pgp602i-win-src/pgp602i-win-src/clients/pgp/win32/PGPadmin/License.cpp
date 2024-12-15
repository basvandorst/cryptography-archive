/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: License.cpp,v 1.4.8.1 1998/11/12 03:13:10 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "Utils.h"

BOOL CALLBACK LicenseDlgProc(HWND hwndDlg, 
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
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->szLicenseNum == NULL)
					{
						SetWindowText(GetDlgItem(hwndDlg, IDC_LICENSENUM),
							"");
					}
					else
					{
						SetWindowText(GetDlgItem(hwndDlg, IDC_LICENSENUM), 
							pConfig->szLicenseNum);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					int nLength;

					if (pConfig->szLicenseNum)
					{
						PGPFreeData(pConfig->szLicenseNum);
						pConfig->szLicenseNum = NULL;
					}

					nLength = GetWindowTextLength(GetDlgItem(hwndDlg, 
													IDC_LICENSENUM));

					if (nLength)
					{
						pConfig->szLicenseNum = (char *)
							PGPNewData(pConfig->memoryMgr,
								nLength+1, kPGPMemoryMgrFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, IDC_LICENSENUM),
							pConfig->szLicenseNum, nLength+1);
						pConfig->szLicenseNum[nLength] = 0;
					}

					break;
				}

			case PSN_WIZFINISH:
				{
					// Save user data for all pages
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
