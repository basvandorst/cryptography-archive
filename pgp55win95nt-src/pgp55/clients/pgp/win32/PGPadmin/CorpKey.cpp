/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CorpKey.cpp,v 1.17 1997/10/16 18:32:00 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPadmin.h"
#include "Utils.h"
#include "pgpMem.h"
#include "pgpImage.h"

#define ID_TIMER	42

static BOOL bNextEnabled = FALSE;

BOOL CALLBACK CorpKeyDlgProc(HWND hwndDlg, 
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

					if (pConfig->bAutoSignTrustCorp)
					{
						CheckDlgButton(hwndDlg, IDC_AUTOSIGNTRUST, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_METAINTRODUCER),
							TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_WARN_NOTCERT),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_AUTOSIGNTRUST, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_METAINTRODUCER),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_WARN_NOTCERT),
							FALSE);
					}

					if (pConfig->bMetaIntroducerCorp)
					{
						CheckDlgButton(hwndDlg, IDC_METAINTRODUCER, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_METAINTRODUCER, 
							BST_UNCHECKED);
					}

					if (pConfig->bWarnNotCertByCorp)
					{
						CheckDlgButton(hwndDlg, IDC_WARN_NOTCERT, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_WARN_NOTCERT, 
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
					// Determine if we should go to corp key selection
					// or skip to next page

					if (!pConfig->bAutoSignTrustCorp)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_KEYGEN);

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
		case IDC_AUTOSIGNTRUST:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_AUTOSIGNTRUST) == 
					BST_CHECKED)
				{
					pConfig->bAutoSignTrustCorp = TRUE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_METAINTRODUCER),
						TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_WARN_NOTCERT),
						TRUE);
				}
				else
				{
					pConfig->bAutoSignTrustCorp = FALSE;
					pConfig->bMetaIntroducerCorp = FALSE;
					pConfig->bWarnNotCertByCorp = FALSE;

					EnableWindow(GetDlgItem(hwndDlg, IDC_METAINTRODUCER),
						FALSE);
					CheckDlgButton(hwndDlg, IDC_METAINTRODUCER, 
						BST_UNCHECKED);

					EnableWindow(GetDlgItem(hwndDlg, IDC_WARN_NOTCERT),
						FALSE);
					CheckDlgButton(hwndDlg, IDC_WARN_NOTCERT, 
						BST_UNCHECKED);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_METAINTRODUCER:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_METAINTRODUCER) == 
					BST_CHECKED)
				{
					pConfig->bMetaIntroducerCorp = TRUE;
				}
				else
				{
					pConfig->bMetaIntroducerCorp = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_WARN_NOTCERT:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_WARN_NOTCERT) == 
					BST_CHECKED)
				{
					pConfig->bWarnNotCertByCorp = TRUE;
				}
				else
				{
					pConfig->bWarnNotCertByCorp = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}
		}

		break;
	}

	return(bReturnCode);
}


BOOL CALLBACK CorpKeySelectDlgProc(HWND hwndDlg, 
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

	hList = GetDlgItem(hwndDlg, IDC_CORPKEY_ID);

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

			ShowKeyring(hwndDlg, IDC_CORPKEY_ID, pConfig->pflContext,
				pConfig->pgpContext, NULL, TRUE, TRUE, FALSE, TRUE);
			g_bGotReloadMsg = FALSE;
			break;
		}

	case WM_DESTROY:
		FreeKeyring(hwndDlg, IDC_CORPKEY_ID, pConfig->pflContext);
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

					if (pConfig->szCorpKeyID != NULL)
					{
						pgpFree(pConfig->szCorpKeyID);
						pConfig->szCorpKeyID = NULL;
					}

					if (pConfig->szCorpKey != NULL)
					{
						pgpFree(pConfig->szCorpKey);
						pConfig->szCorpKey = NULL;
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

						pConfig->szCorpKeyID = 
							(char *) pgpAlloc(strlen(szKeyID)+1);
						strcpy(pConfig->szCorpKeyID, szKeyID);

						pConfig->szCorpKey = 
							(char *) pgpAlloc(strlen(szUserID)+1);
						strcpy(pConfig->szCorpKey, szUserID);

						if (lviKey.iImage == IDX_RSAPUBKEY)
							pConfig->corpKeyType = kPGPPublicKeyAlgorithm_RSA;
						if (lviKey.iImage == IDX_DSAPUBKEY)
							pConfig->corpKeyType = kPGPPublicKeyAlgorithm_DSA;
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
				if (pnmh->idFrom == IDC_CORPKEY_ID)
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
