/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: AdditionalDecryption.cpp,v 1.5 1999/03/10 02:28:57 heller Exp $
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

BOOL CALLBACK ADKIntroDlgProc(HWND hwndDlg, 
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
					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					// Save user data for this page
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


BOOL CALLBACK ADKIncomingDlgProc(HWND hwndDlg, 
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
					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->bUseIncomingADK)
					{
						CheckDlgButton(hwndDlg, IDC_USE_INADK, BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_USE_INADK, BST_UNCHECKED);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

			case PSN_WIZNEXT:
				{
					// Determine if we should go to incoming ADK key 
					// selection or skip to outgoing ADK

					if (!pConfig->bUseIncomingADK)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, 
							IDD_ADK_OUTGOING);

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
		case IDC_USE_INADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USE_INADK) == BST_CHECKED)
				{
					pConfig->bUseIncomingADK = TRUE;
				}
				else
				{
					pConfig->bUseIncomingADK = FALSE;
					pConfig->bEnforceIncomingADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK ADKInSelectDlgProc(HWND hwndDlg, 
								 UINT uMsg, 
								 WPARAM wParam, 
								 LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	HWND			hList;

	g_hCurrentDlgWnd = hwndDlg;

	if (uMsg != WM_INITDIALOG)
		pConfig = (pgpConfigInfo *) GetWindowLong(hwndDlg, GWL_USERDATA);

	hList = GetDlgItem(hwndDlg, IDC_INADK_ID);

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

			ShowKeyring(hwndDlg, IDC_INADK_ID, pConfig->pgpContext, 
				NULL, FALSE, TRUE, TRUE, FALSE);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_DESTROY:
		FreeKeyring(hwndDlg, IDC_INADK_ID);
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
					int		nIndex;
					LV_ITEM lviKey;
					char *szKeyID;
					char szUserID[kPGPMaxUserIDSize+1];

					if (pConfig->szIncomingADKID != NULL)
					{
						pgpFree(pConfig->szIncomingADKID);
						pConfig->szIncomingADKID = NULL;
					}

					if (pConfig->szIncomingADK != NULL)
					{
						pgpFree(pConfig->szIncomingADK);
						pConfig->szIncomingADK = NULL;
					}

					// Save user data for this page
					nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					if (nIndex > -1)
					{
						lviKey.mask = LVIF_PARAM | LVIF_TEXT;
						lviKey.iItem = nIndex;
						lviKey.iSubItem = 0;
						lviKey.pszText = szUserID;
						lviKey.cchTextMax = kPGPMaxUserIDSize;
						ListView_GetItem(hList, &lviKey);
						szKeyID = (char *) lviKey.lParam;

						pConfig->szIncomingADKID = 
							(char *) pgpAlloc(strlen(szKeyID)+1);
						strcpy(pConfig->szIncomingADKID, szKeyID);

						pConfig->szIncomingADK = 
							(char *) pgpAlloc(strlen(szUserID)+1);
						strcpy(pConfig->szIncomingADK, szUserID);
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
				if (pnmh->idFrom == IDC_INADK_ID)
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


BOOL CALLBACK ADKOutgoingDlgProc(HWND hwndDlg, 
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
					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->bUseOutgoingADK)
					{
						CheckDlgButton(hwndDlg, IDC_USE_OUTADK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_USE_OUTADK, 
							BST_UNCHECKED);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

			case PSN_WIZNEXT:
				{
					// Determine if we should go to RSA key selection
					// or skip past it

					if (!pConfig->bUseOutgoingADK)
//						if (pConfig->bUseIncomingADK)
							SetWindowLong(hwndDlg, DWL_MSGRESULT, 
								IDD_ADK_ENFORCE);
/*
						else
							SetWindowLong(hwndDlg, DWL_MSGRESULT, 
								IDD_PASSPHRASE);
*/

					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZBACK:
				{
					// Determine if we should go to incoming ADK key 
					// selection or skip back to incoming ADK

					if (!pConfig->bUseIncomingADK)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, 
							IDD_ADK_INCOMING);

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
		case IDC_USE_OUTADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USE_OUTADK) == 
					BST_CHECKED)
				{
					pConfig->bUseOutgoingADK = TRUE;
				}
				else
				{
					pConfig->bUseOutgoingADK = FALSE;
					pConfig->bEnforceOutgoingADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK ADKOutSelectDlgProc(HWND hwndDlg, 
									 UINT uMsg, 
									 WPARAM wParam, 
									 LPARAM lParam)
{
	BOOL bReturnCode = FALSE;
	pgpConfigInfo *pConfig = NULL;
	HWND hList;

	g_hCurrentDlgWnd = hwndDlg;

	if (uMsg != WM_INITDIALOG)
		pConfig = (pgpConfigInfo *) GetWindowLong(hwndDlg, GWL_USERDATA);

	hList = GetDlgItem(hwndDlg, IDC_OUTADK_ID);

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

			ShowKeyring(hwndDlg, IDC_OUTADK_ID, pConfig->pgpContext, 
				NULL, TRUE, TRUE, TRUE, FALSE);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_DESTROY:
		FreeKeyring(hwndDlg, IDC_OUTADK_ID);
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

					if (pConfig->szOutgoingADKID != NULL)
					{
						pgpFree(pConfig->szOutgoingADKID);
						pConfig->szOutgoingADKID = NULL;
					}

					if (pConfig->szOutgoingADK != NULL)
					{
						pgpFree(pConfig->szOutgoingADK);
						pConfig->szOutgoingADK = NULL;
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

						pConfig->szOutgoingADKID = 
							(char *) pgpAlloc(strlen(szKeyID)+1);
						strcpy(pConfig->szOutgoingADKID, szKeyID);

						pConfig->szOutgoingADK = 
							(char *) pgpAlloc(strlen(szUserID)+1);
						strcpy(pConfig->szOutgoingADK, szUserID);

						if (lviKey.iImage == IDX_RSAPUBKEY)
							pConfig->outgoingADKKeyType = 
								kPGPPublicKeyAlgorithm_RSA;
						if (lviKey.iImage == IDX_DSAPUBKEY)
							pConfig->outgoingADKKeyType = 
								kPGPPublicKeyAlgorithm_DSA;
					}

					bReturnCode = TRUE;
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

			case NM_CLICK:
			case NM_RCLICK:
			case NM_DBLCLK:
			case NM_RDBLCLK:
				if (pnmh->idFrom == IDC_OUTADK_ID)
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


BOOL CALLBACK ADKEnforceDlgProc(HWND hwndDlg, 
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
					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->bUseIncomingADK)
						EnableWindow(GetDlgItem(hwndDlg, IDC_ENFORCE_INADK),
							TRUE);
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_ENFORCE_INADK),
							FALSE);
						pConfig->bEnforceIncomingADK = FALSE;
					}

					if (pConfig->bUseOutgoingADK)
						EnableWindow(GetDlgItem(hwndDlg, IDC_ENFORCE_OUTADK),
							TRUE);
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_ENFORCE_OUTADK),
							FALSE);
						pConfig->bEnforceOutgoingADK = FALSE;
					}

					if (pConfig->bEnforceIncomingADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_INADK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_INADK, 
							BST_UNCHECKED);
					}

					if (pConfig->bEnforceOutgoingADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_OUTADK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_OUTADK, 
							BST_UNCHECKED);
					}

					if (pConfig->bEnforceRemoteADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_REMOTEADK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_REMOTEADK, 
							BST_UNCHECKED);
					}
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
					// Determine if we should go to outgoing ADK key 
					// selection or skip past it

					if (!pConfig->bUseOutgoingADK)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, 
							IDD_ADK_OUTGOING);

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
		case IDC_ENFORCE_INADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENFORCE_INADK) == 
					BST_CHECKED)
				{
					pConfig->bEnforceIncomingADK = TRUE;
				}
				else
				{
					pConfig->bEnforceIncomingADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		
		case IDC_ENFORCE_OUTADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENFORCE_OUTADK) == 
					BST_CHECKED)
				{
					pConfig->bEnforceOutgoingADK = TRUE;
				}
				else
				{
					pConfig->bEnforceOutgoingADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		
		case IDC_ENFORCE_REMOTEADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENFORCE_REMOTEADK) == 
					BST_CHECKED)
				{
					pConfig->bEnforceRemoteADK = TRUE;
				}
				else
				{
					pConfig->bEnforceRemoteADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK ADKDiskDlgProc(HWND hwndDlg, 
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
					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					if (pConfig->bUseDiskADK)
					{
						CheckDlgButton(hwndDlg, IDC_USE_DISKADK, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_USE_DISKADK, 
							BST_UNCHECKED);
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

			case PSN_WIZNEXT:
				{
					// Determine if we should go to PGPdisk key selection
					// or skip past it

					if (!pConfig->bUseDiskADK)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, 
							IDD_PASSPHRASE);

					bReturnCode = TRUE;
					break;
				}

			case PSN_WIZBACK:
				{
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
		case IDC_USE_DISKADK:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USE_DISKADK) == 
					BST_CHECKED)
				{
					pConfig->bUseDiskADK = TRUE;
				}
				else
				{
					pConfig->bUseDiskADK = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK ADKDiskSelectDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam)
{
	BOOL bReturnCode = FALSE;
	pgpConfigInfo *pConfig = NULL;
	HWND hList;

	g_hCurrentDlgWnd = hwndDlg;

	if (uMsg != WM_INITDIALOG)
		pConfig = (pgpConfigInfo *) GetWindowLong(hwndDlg, GWL_USERDATA);

	hList = GetDlgItem(hwndDlg, IDC_DISKADK_ID);

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

			ShowKeyring(hwndDlg, IDC_DISKADK_ID, pConfig->pgpContext, 
				NULL, TRUE, TRUE, TRUE, FALSE);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_DESTROY:
		FreeKeyring(hwndDlg, IDC_DISKADK_ID);
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

					if (pConfig->szDiskADKID != NULL)
					{
						pgpFree(pConfig->szDiskADKID);
						pConfig->szDiskADKID = NULL;
					}

					if (pConfig->szDiskADK != NULL)
					{
						pgpFree(pConfig->szDiskADK);
						pConfig->szDiskADK = NULL;
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

						pConfig->szDiskADKID = 
							(char *) pgpAlloc(strlen(szKeyID)+1);
						strcpy(pConfig->szDiskADKID, szKeyID);

						pConfig->szDiskADK = 
							(char *) pgpAlloc(strlen(szUserID)+1);
						strcpy(pConfig->szDiskADK, szUserID);

						if (lviKey.iImage == IDX_RSAPUBKEY)
							pConfig->diskADKKeyType = 
								kPGPPublicKeyAlgorithm_RSA;
						if (lviKey.iImage == IDX_DSAPUBKEY)
							pConfig->diskADKKeyType = 
								kPGPPublicKeyAlgorithm_DSA;
					}

					bReturnCode = TRUE;
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

			case NM_CLICK:
			case NM_RCLICK:
			case NM_DBLCLK:
			case NM_RDBLCLK:
				if (pnmh->idFrom == IDC_DISKADK_ID)
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
