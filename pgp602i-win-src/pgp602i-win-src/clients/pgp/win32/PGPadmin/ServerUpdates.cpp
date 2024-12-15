/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: ServerUpdates.cpp,v 1.1.8.1 1998/11/12 03:13:14 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <commctrl.h>
#include "resource.h"

BOOL CALLBACK ServerUpdatesDlgProc(HWND hwndDlg, 
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
			SetWindowPos(GetParent(hwndDlg), NULL,
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
					char szBuffer[4];

					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					SendMessage(GetDlgItem(hwndDlg, IDC_ALLSPIN), 
						UDM_SETRANGE, 0, MAKELONG(999, 1));
					SendMessage(GetDlgItem(hwndDlg, IDC_TRUSTEDSPIN), 
						UDM_SETRANGE, 0, MAKELONG(999, 1));

					if (pConfig->bUpdateAllKeys)
					{
						CheckDlgButton(hwndDlg, IDC_UPDATEALL, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALLDAYS),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_UPDATEALL, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALLDAYS),
							FALSE);
					}

					if (pConfig->bUpdateTrustedIntroducers)
					{
						CheckDlgButton(hwndDlg, IDC_UPDATETRUSTED, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_UPDATETRUSTED, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS),
							FALSE);
					}

					wsprintf(szBuffer, "%d", pConfig->nDaysUpdateAllKeys);
					SetWindowText(GetDlgItem(hwndDlg, IDC_ALLDAYS), 
						szBuffer);

					wsprintf(szBuffer, "%d", 
						pConfig->nDaysUpdateTrustedIntroducers);
					SetWindowText(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS), 
						szBuffer);

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					break;
				}

			case PSN_WIZBACK:
				{
					break;
				}

			case PSN_WIZNEXT:
				{
					char szErrorMsg[255];
					char szTitle[255];
					BOOL bError = FALSE;

					bReturnCode = TRUE;
					LoadString(g_hInstance, IDS_TITLE, szTitle, 254);

					// Check data validity

					if (pConfig->bUpdateAllKeys)
					{
						if ((pConfig->nDaysUpdateAllKeys < 1) || 
							(pConfig->nDaysUpdateAllKeys > 999))
						{
							LoadString(g_hInstance, IDS_E_DAYSRANGE, 
								szErrorMsg, 254);
							bError = TRUE;
						}
					}

					if (pConfig->bUpdateTrustedIntroducers)
					{
						if ((pConfig->nDaysUpdateTrustedIntroducers < 1) || 
							(pConfig->nDaysUpdateTrustedIntroducers > 999))
						{
							LoadString(g_hInstance, IDS_E_DAYSRANGE, 
								szErrorMsg, 254);
							bError = TRUE;
						}
					}

					if (bError)
					{
						MessageBox(hwndDlg, szErrorMsg, szTitle, MB_OK);
						SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
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
		if (pConfig == NULL)
			break;

		switch (LOWORD(wParam))
		{
		case IDC_UPDATEALL:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_UPDATEALL) == 
					BST_CHECKED)
				{
					pConfig->bUpdateAllKeys = TRUE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_ALLDAYS),
						TRUE);
				}
				else
				{
					pConfig->bUpdateAllKeys = FALSE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_ALLDAYS),
						FALSE);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_UPDATETRUSTED:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_UPDATETRUSTED) == 
					BST_CHECKED)
				{
					pConfig->bUpdateTrustedIntroducers = TRUE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS),
						TRUE);
				}
				else
				{
					pConfig->bUpdateTrustedIntroducers = FALSE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS),
						FALSE);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_ALLDAYS:
			{
				char szBuffer[4];

				GetWindowText(GetDlgItem(hwndDlg, IDC_ALLDAYS), 
					szBuffer, 4);
				pConfig->nDaysUpdateAllKeys = atoi(szBuffer);
				bReturnCode = TRUE;
				break;
			}

		case IDC_TRUSTEDDAYS:
			{
				char szBuffer[4];

				GetWindowText(GetDlgItem(hwndDlg, IDC_TRUSTEDDAYS), 
					szBuffer, 4);
				pConfig->nDaysUpdateTrustedIntroducers = atoi(szBuffer);
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
