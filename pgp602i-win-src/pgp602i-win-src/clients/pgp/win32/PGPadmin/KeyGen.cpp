/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: KeyGen.cpp,v 1.16.2.1.2.1 1998/11/12 03:13:09 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "pgpBuildFlags.h"
#include "PGPcl.h"


BOOL CALLBACK KeyGenDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	char			szMinKeySize[10];

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

					if (pConfig->bAllowKeyGen)
					{
						PGPError err;

						CheckDlgButton(hwndDlg, IDC_ALLOW_KEYGEN, 
							BST_CHECKED);
					
						err = PGPclCheckSDKSupportForPKAlg(
								kPGPPublicKeyAlgorithm_RSA, FALSE, FALSE);

#if NO_RSA_KEYGEN
						err = kPGPError_FeatureNotAvailable;
#endif

						if (IsntPGPError(err))
							EnableWindow(GetDlgItem(hwndDlg, 
								IDC_ALLOW_RSAKEYGEN), TRUE);
						else
							EnableWindow(GetDlgItem(hwndDlg, 
								IDC_ALLOW_RSAKEYGEN), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL),
							TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL2),
							TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINKEYSIZE),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ALLOW_KEYGEN, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALLOW_RSAKEYGEN),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL2),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINKEYSIZE),
							FALSE);
					}

					if (pConfig->bAllowRSAKeyGen)
					{
						CheckDlgButton(hwndDlg, IDC_ALLOW_RSAKEYGEN, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ALLOW_RSAKEYGEN, 
							BST_UNCHECKED);
					}

					wsprintf(szMinKeySize, "%d", pConfig->nMinKeySize);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINKEYSIZE), 
						szMinKeySize);

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					break;
				}

			case PSN_WIZBACK:
				{
					// Determine if we should go to revoker key selection
					// or skip to revoker key options

					if (!pConfig->bAutoAddRevoker)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_REVOKER);

					bReturnCode = TRUE;
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

					if (pConfig->bAllowKeyGen)
					{
						if (pConfig->bAllowRSAKeyGen)
						{
							if ((pConfig->nMinKeySize < 768) || 
								(pConfig->nMinKeySize > 2048))
							{
								LoadString(g_hInstance, 
									IDS_E_MINKEYSIZERANGERSA, szErrorMsg, 254);
								bError = TRUE;
							}
						}
						else
						{
							if ((pConfig->nMinKeySize < 768) || 
								(pConfig->nMinKeySize > 4096))
							{
								LoadString(g_hInstance, 
									IDS_E_MINKEYSIZERANGE, szErrorMsg, 254);
								bError = TRUE;
							}
						}

						if (bError)
						{
							MessageBox(hwndDlg, szErrorMsg, szTitle, MB_OK);
							SetWindowLong(hwndDlg, DWL_MSGRESULT, -1);
						}
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
		case IDC_ALLOW_KEYGEN:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ALLOW_KEYGEN) == 
					BST_CHECKED)
				{
					PGPError err;

					pConfig->bAllowKeyGen = TRUE;
					
					err = PGPclCheckSDKSupportForPKAlg(
							kPGPPublicKeyAlgorithm_RSA, FALSE, FALSE);

#if NO_RSA_KEYGEN
					err = kPGPError_FeatureNotAvailable;
#endif

					if (IsntPGPError(err))
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALLOW_RSAKEYGEN),
							TRUE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALLOW_RSAKEYGEN),
							FALSE);

					EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL),
						TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL2),
						TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINKEYSIZE),
						TRUE);
				}
				else
				{
					pConfig->bAllowKeyGen = FALSE;
					EnableWindow(GetDlgItem(hwndDlg, IDC_ALLOW_RSAKEYGEN),
						FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL),
						FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_LABEL2),
						FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINKEYSIZE),
						FALSE);
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_ALLOW_RSAKEYGEN:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ALLOW_RSAKEYGEN) == 
					BST_CHECKED)
				{
					pConfig->bAllowRSAKeyGen = TRUE;
				}
				else
				{
					pConfig->bAllowRSAKeyGen = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_MINKEYSIZE:
			{
				GetWindowText(GetDlgItem(hwndDlg, IDC_MINKEYSIZE), 
					szMinKeySize, 10);
				pConfig->nMinKeySize = atoi(szMinKeySize);
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
