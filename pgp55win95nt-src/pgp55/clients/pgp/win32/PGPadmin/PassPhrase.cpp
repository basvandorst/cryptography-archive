/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PassPhrase.cpp,v 1.11 1997/10/16 18:32:07 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "resource.h"
#include "PGPadmin.h"

BOOL CALLBACK PassPhraseDlgProc(HWND hwndDlg, 
								UINT uMsg, 
								WPARAM wParam, 
								LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	char			szMinChars[10];
	char			szMinQuality[10];

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

					if (pConfig->bEnforceMinChars)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_MINCHARS, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL1), 
							TRUE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL2), 
							TRUE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS), 
							TRUE); 
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_MINCHARS, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL1), 
							FALSE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL2), 
							FALSE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS), 
							FALSE); 
					}

					if (pConfig->bEnforceMinQuality)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_MINQUALITY, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL1), 
							TRUE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL2), 
							TRUE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINQUALITY), 
							TRUE); 
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCE_MINQUALITY, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL1), 
							FALSE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL2), 
							FALSE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINQUALITY), 
							FALSE); 
					}

					wsprintf(szMinChars, "%d", pConfig->nMinChars);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINCHARS), 
						szMinChars);

					wsprintf(szMinQuality, "%d", pConfig->nMinQuality);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINQUALITY), 
						szMinQuality);

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

					if (pConfig->bEnforceMinChars)
					{
						if ((pConfig->nMinChars < 8) || 
							(pConfig->nMinChars > 128))
						{
							LoadString(g_hInstance, IDS_E_MINCHARSRANGE,
								szErrorMsg, 254);
							bError = TRUE;
						}
					}

					if (pConfig->bEnforceMinQuality && !bError)
					{
						if ((pConfig->nMinQuality < 20) ||
							(pConfig->nMinQuality > 100))
						{
							LoadString(g_hInstance, IDS_E_MINQUALRANGE,
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

/*
			case PSN_WIZBACK:
				{
					// Determine if we should go to ADK
					// enforcement or skip past it

					if (!pConfig->bUseOutgoingADK && !pConfig->bUseIncomingADK)
						SetWindowLong(hwndDlg, DWL_MSGRESULT, 
							IDD_ADK_OUTGOING);

					bReturnCode = TRUE;
					break;
				}
*/

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
		case IDC_ENFORCE_MINCHARS:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENFORCE_MINCHARS) == 
					BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL1), TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL2), TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS), TRUE); 
					pConfig->bEnforceMinChars = TRUE;
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL1), FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSLABEL2), FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS), FALSE); 
					pConfig->bEnforceMinChars = FALSE;
				}
		
				bReturnCode = TRUE;
				break;
			}

		case IDC_ENFORCE_MINQUALITY:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ENFORCE_MINQUALITY) == 
					BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL1), TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL2), TRUE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINQUALITY), TRUE); 
					pConfig->bEnforceMinQuality = TRUE;
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL1), FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_QUALLABEL2), FALSE); 
					EnableWindow(GetDlgItem(hwndDlg, IDC_MINQUALITY), FALSE); 
					pConfig->bEnforceMinQuality = FALSE;
				}
	
				bReturnCode = TRUE;
				break;
			}

		case IDC_MINCHARS:
			{
				GetWindowText(GetDlgItem(hwndDlg, IDC_MINCHARS), 
					szMinChars, 10);
				pConfig->nMinChars = atoi(szMinChars);
				bReturnCode = TRUE;
				break;
			}

		case IDC_MINQUALITY:
			{
				GetWindowText(GetDlgItem(hwndDlg, IDC_MINQUALITY), 
					szMinQuality, 10);
				pConfig->nMinQuality = atoi(szMinQuality);
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
