/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Revoker.cpp,v 1.3 1999/03/10 03:01:59 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <commctrl.h>
#include "resource.h"
#include "Utils.h"
#include "pgpMem.h"
#include "pgpImage.h"

#define ID_TIMER	42

static BOOL bNextEnabled = FALSE;

BOOL CALLBACK RevokerDlgProc(HWND hwndDlg, 
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

					if (pConfig->bAutoAddRevoker)
						CheckDlgButton(hwndDlg, IDC_ENABLEREVOKER, 
							BST_CHECKED);
					else
						CheckDlgButton(hwndDlg, IDC_ENABLEREVOKER, 
							BST_UNCHECKED);

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

			case PSN_WIZBACK:
				{
					// Determine if we should go to corp key selection
					// or skip to corporate key options

					if (!pConfig->bAutoSignTrustCorp)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_CORPKEY);

					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZNEXT:
				{
					// Determine if we should go to revoker key selection
					// or skip to next page

					if (!pConfig->bAutoAddRevoker)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_X509);

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
		case IDC_ENABLEREVOKER:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLEREVOKER) == 
					BST_CHECKED)
				{
					pConfig->bAutoAddRevoker = TRUE;
				}
				else
				{
					pConfig->bAutoAddRevoker = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK RevokerSelectDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	HWND hList;

	g_hCurrentDlgWnd = hwndDlg;

	if (uMsg != WM_INITDIALOG)
		pConfig = (pgpConfigInfo *) GetWindowLong(hwndDlg, GWL_USERDATA);

	hList = GetDlgItem(hwndDlg, IDC_REVOKER_ID);

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

			ShowKeyring(hwndDlg, IDC_REVOKER_ID, pConfig->pgpContext, 
				NULL, FALSE, TRUE, FALSE, TRUE);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_DESTROY:
		FreeKeyring(hwndDlg, IDC_REVOKER_ID);
		break;

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
					int nIndex;

					// Initialize window
					nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					if (nIndex > -1)
					{
						PostMessage(GetParent(hwndDlg),
							PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
						bNextEnabled = TRUE;
					}
					else
					{
						PostMessage(GetParent(hwndDlg),
							PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
						bNextEnabled = FALSE;
					}

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					SetTimer(hwndDlg, ID_TIMER, 500, NULL);
					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					KillTimer(hwndDlg, ID_TIMER);
					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZNEXT:
				{
					int nIndex;
					LV_ITEM lviKey;
					char *szKeyID;
					char szUserID[kPGPMaxUserIDSize+1];

					if (pConfig->szRevokerKeyID != NULL)
					{
						pgpFree(pConfig->szRevokerKeyID);
						pConfig->szRevokerKeyID = NULL;
					}

					if (pConfig->szRevokerKey != NULL)
					{
						pgpFree(pConfig->szRevokerKey);
						pConfig->szRevokerKey = NULL;
					}

					// Save user data for this page
					nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					if (nIndex > -1)
					{
						lviKey.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
						lviKey.iItem = nIndex;
						lviKey.iSubItem = 0;
						lviKey.pszText = szUserID;
						lviKey.cchTextMax = kPGPMaxUserIDSize;
						ListView_GetItem(hList, &lviKey);
						szKeyID = (char *) lviKey.lParam;

						pConfig->szRevokerKeyID = 
							(char *) pgpAlloc(strlen(szKeyID)+1);
						strcpy(pConfig->szRevokerKeyID, szKeyID);

						pConfig->szRevokerKey = 
							(char *) pgpAlloc(strlen(szUserID)+1);
						strcpy(pConfig->szRevokerKey, szUserID);

						if (lviKey.iImage == IDX_RSAPUBKEY)
							pConfig->revokerKeyType = 
												kPGPPublicKeyAlgorithm_RSA;
						if (lviKey.iImage == IDX_DSAPUBKEY)
							pConfig->revokerKeyType = 
												kPGPPublicKeyAlgorithm_DSA;
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

			case NM_CLICK:
			case NM_RCLICK:
			case NM_DBLCLK:
			case NM_RDBLCLK:
				if (pnmh->idFrom == IDC_REVOKER_ID)
				{
					int nIndex;

					nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					if (nIndex > -1)
					{
						SendMessage(GetParent(hwndDlg),
							PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
						bNextEnabled = TRUE;
					}
					bReturnCode = TRUE;
				}
				break;
			}
			
			break;
		}

	case WM_TIMER:
		{
			int nIndex;

			nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
			if (nIndex > -1)
			{
				if (!bNextEnabled)
				{
					SendMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
					bNextEnabled = TRUE;
				}
			}
			else if (bNextEnabled)
			{
				SendMessage(GetParent(hwndDlg),
					PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
				bNextEnabled = FALSE;
			}

			bReturnCode = TRUE;
			SetTimer(hwndDlg, ID_TIMER, 500, NULL);
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
