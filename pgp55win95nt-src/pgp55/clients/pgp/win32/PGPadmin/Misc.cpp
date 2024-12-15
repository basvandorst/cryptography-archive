/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Misc.cpp,v 1.9 1997/10/16 18:32:05 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "resource.h"
#include "PGPadmin.h"
#include "pgpMem.h"

BOOL CALLBACK MiscDlgProc(HWND hwndDlg, 
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

					SendDlgItemMessage(hwndDlg, IDC_COMMENTS, EM_SETLIMITTEXT, 
						60, 0);

					if (pConfig->bAllowConventionalEncryption)
					{
						CheckDlgButton(hwndDlg, IDC_ALLOW_CONVENTIONAL, 
							BST_CHECKED);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ALLOW_CONVENTIONAL, 
							BST_UNCHECKED);
					}

					if (pConfig->nCommentLength)
					{
						SetWindowText(GetDlgItem(hwndDlg, IDC_COMMENTS), 
							pConfig->szComments);
					}
					else
					{
						SetWindowText(GetDlgItem(hwndDlg, IDC_COMMENTS), "");
					}

					bReturnCode = TRUE;
					break;
				}

			case PSN_KILLACTIVE:
				{
					if (pConfig->szComments)
					{
						PFLContextMemFree(pConfig->pflContext,
							pConfig->szComments);
						pConfig->szComments = NULL;
					}

					pConfig->nCommentLength = 
						GetWindowTextLength(GetDlgItem(hwndDlg, 
												IDC_COMMENTS));

					if (pConfig->nCommentLength)
					{
						pConfig->szComments = (char *)
							PFLContextMemAlloc(pConfig->pflContext,
								pConfig->nCommentLength+1,
								kPFLMemoryFlags_Clear);

						GetWindowText(GetDlgItem(hwndDlg, IDC_COMMENTS),
							pConfig->szComments, pConfig->nCommentLength+1);
						pConfig->szComments[pConfig->nCommentLength] = 0;
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
		case IDC_ALLOW_CONVENTIONAL:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_ALLOW_CONVENTIONAL) == 
					BST_CHECKED)
				{
					pConfig->bAllowConventionalEncryption = TRUE;
				}
				else
				{
					pConfig->bAllowConventionalEncryption = FALSE;
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
