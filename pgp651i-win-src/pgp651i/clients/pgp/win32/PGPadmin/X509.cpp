/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: X509.cpp,v 1.8 1999/05/06 19:05:48 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "pgpAdminPrefs.h"
#include "PGPcl.h"


BOOL CALLBACK X509DlgProc(HWND hwndDlg, 
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
					int nIndex;

					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_RESETCONTENT,
						0, 0);

					nIndex = SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
								CB_ADDSTRING, 0, (LPARAM) "None");

					SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_SETITEMDATA,
						nIndex, kPGPKeyServerClass_Invalid);

					if (pConfig->nCAType == kPGPKeyServerClass_Invalid)
						SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
							CB_SETCURSEL, nIndex, 0);

					nIndex = SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
								CB_ADDSTRING, 0, 
								(LPARAM) "Net Tools PKI Server");

					SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_SETITEMDATA,
						nIndex, kPGPKeyServerClass_NetToolsCA);

					if (pConfig->nCAType == kPGPKeyServerClass_NetToolsCA)
						SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
							CB_SETCURSEL, nIndex, 0);

					nIndex = SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
								CB_ADDSTRING, 0, (LPARAM) "VeriSign OnSite");

					SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_SETITEMDATA,
						nIndex, kPGPKeyServerClass_Verisign);

					if (pConfig->nCAType == kPGPKeyServerClass_Verisign)
						SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
							CB_SETCURSEL, nIndex, 0);

					nIndex = SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
								CB_ADDSTRING, 0, (LPARAM) "Entrust");

					SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_SETITEMDATA,
						nIndex, kPGPKeyServerClass_Entrust);

					if (pConfig->nCAType == kPGPKeyServerClass_Entrust)
						SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
							CB_SETCURSEL, nIndex, 0);

					if (pConfig->nCAType == kPGPKeyServerClass_Invalid)
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDATTRIBUTES), 
							FALSE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDATTRIBUTES), 
							TRUE);

					if (pConfig->bKeyGenCertRequest)
					{
						CheckDlgButton(hwndDlg, IDC_KEYGENCERTREQ, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_KEYGENCERTREQ, 
							BST_UNCHECKED);
					}

					if (pConfig->bAllowManualCertRequest)
					{
						CheckDlgButton(hwndDlg, IDC_ALLOWMANREQ, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ALLOWMANREQ, 
							BST_UNCHECKED);
					}

					if (pConfig->bAutoUpdateCRL)
					{
						CheckDlgButton(hwndDlg, IDC_AUTOCRL, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_AUTOCRL, 
							BST_UNCHECKED);
					}

					bReturnCode = TRUE;
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
		case IDC_KEYGENCERTREQ:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_KEYGENCERTREQ) == 
					BST_CHECKED)
				{
					pConfig->bKeyGenCertRequest = TRUE;
				}
				else
				{
					pConfig->bKeyGenCertRequest = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_ALLOWMANREQ:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ALLOWMANREQ) == 
					BST_CHECKED)
				{
					pConfig->bAllowManualCertRequest = TRUE;
				}
				else
				{
					pConfig->bAllowManualCertRequest = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_AUTOCRL:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_AUTOCRL) == 
					BST_CHECKED)
				{
					pConfig->bAutoUpdateCRL = TRUE;
				}
				else
				{
					pConfig->bAutoUpdateCRL = FALSE;
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_CATYPE:
			{
				int nIndex;

				nIndex = SendDlgItemMessage(hwndDlg, IDC_CATYPE, CB_GETCURSEL,
							0, 0);

				pConfig->nCAType = (PGPKeyServerClass) 
									SendDlgItemMessage(hwndDlg, IDC_CATYPE, 
										CB_GETITEMDATA, nIndex, 0);

				if (pConfig->nCAType == kPGPKeyServerClass_Invalid)
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDATTRIBUTES), 
						FALSE);
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDATTRIBUTES), 
						TRUE);

					if (HIWORD(wParam) == CBN_SELENDOK)
					{
						// If you change this, change the code
						// for IDC_ADDATTRIBUTES too

						if (pConfig->pAVList != NULL)
						{
							PGPclFreeCACertRequestAVList(pConfig->pAVList, 
								pConfig->nNumAVs);
							
							pConfig->pAVList = NULL;
							pConfig->nNumAVs = 0;
						}
						
						PGPclGetCACertRequestAVList(hwndDlg, 
							pConfig->pgpContext, TRUE, kInvalidPGPUserIDRef,
							pConfig->nCAType, &(pConfig->pAVList), 
							&(pConfig->nNumAVs));
					}
				}

				bReturnCode = TRUE;
				break;
			}

		case IDC_ADDATTRIBUTES:
			{
				// If you change this, change the code
				// for IDC_CATYPE too

				if (pConfig->pAVList != NULL)
				{
					PGPclFreeCACertRequestAVList(pConfig->pAVList, 
						pConfig->nNumAVs);

					pConfig->pAVList = NULL;
					pConfig->nNumAVs = 0;
				}

				PGPclGetCACertRequestAVList(hwndDlg, pConfig->pgpContext,
					TRUE, kInvalidPGPUserIDRef, pConfig->nCAType,
					&(pConfig->pAVList), &(pConfig->nNumAVs));

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
