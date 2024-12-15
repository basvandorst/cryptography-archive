/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Review.cpp,v 1.6 1997/10/16 18:32:08 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPadmin.h"
#include "Utils.h"
#include "pgpMem.h"

BOOL CALLBACK ReviewDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	char			szMinChars[10];
	char			szMinQuality[10];
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

					SendDlgItemMessage(hwndDlg, IDC_COMMENTS, EM_SETLIMITTEXT, 
						60, 0);

					if (pConfig->bUseIncomingADK)
					{
						CheckDlgButton(hwndDlg, IDC_INADK_CHECK, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_INADK_CHECK),
							TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_INADK),
							TRUE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_INADK), 
							pConfig->szIncomingADK);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_INADK_CHECK, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_INADK_CHECK),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_INADK),
							FALSE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_INADK), 
							"(disabled)");
					}

					if (pConfig->bUseOutgoingADK)
					{
						CheckDlgButton(hwndDlg, IDC_OUTADK_CHECK, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OUTADK_CHECK),
							TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OUTADK),
							TRUE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_OUTADK), 
							pConfig->szOutgoingADK);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_OUTADK_CHECK, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OUTADK_CHECK),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OUTADK),
							FALSE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_OUTADK), 
							"(disabled)");
					}

					if (pConfig->bEnforceIncomingADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEINADK_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEINADK_CHECK),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEINADK_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEINADK_CHECK),
							FALSE);
					}

					if (pConfig->bEnforceOutgoingADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEOUTADK_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEOUTADK_CHECK),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEOUTADK_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEOUTADK_CHECK),
							FALSE);
					}

					if (pConfig->bEnforceRemoteADK)
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEREMOTEADK_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEREMOTEADK_CHECK),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_ENFORCEREMOTEADK_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCEREMOTEADK_CHECK),
							FALSE);
					}

					if ((!pConfig->bEnforceIncomingADK) &&
						(!pConfig->bEnforceOutgoingADK) &&
						(!pConfig->bEnforceRemoteADK))
					{
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCELABEL),
							FALSE);
					}
					else
					{
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_ENFORCELABEL),
							TRUE);
					}

					if (pConfig->bAutoSignTrustCorp)
					{
						CheckDlgButton(hwndDlg, IDC_CORPKEY_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CORPKEY_CHECK),
							TRUE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CORPKEY),
							TRUE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_CORPKEY), 
							pConfig->szCorpKey);

						if (pConfig->bMetaIntroducerCorp)
						{
							CheckDlgButton(hwndDlg, IDC_METAINTRODUCER_CHECK, 
								BST_CHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_METAINTRODUCER_CHECK),
								TRUE);
						}
						else
						{
							CheckDlgButton(hwndDlg, IDC_METAINTRODUCER_CHECK, 
								BST_UNCHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_METAINTRODUCER_CHECK),
								FALSE);
						}
						
						if (pConfig->bWarnNotCertByCorp)
						{
							CheckDlgButton(hwndDlg, IDC_WARN_NOTCERT_CHECK, 
								BST_CHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_WARN_NOTCERT_CHECK),
								TRUE);
						}
						else
						{
							CheckDlgButton(hwndDlg, IDC_WARN_NOTCERT_CHECK, 
								BST_UNCHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_WARN_NOTCERT_CHECK),
								FALSE);
						}
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_CORPKEY_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CORPKEY_CHECK),
							FALSE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CORPKEY),
							FALSE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_METAINTRODUCER_CHECK),
							FALSE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_WARN_NOTCERT_CHECK),
							FALSE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_CORPKEY), 
							"(disabled)");
					}

					if (pConfig->bEnforceMinChars)
					{
						CheckDlgButton(hwndDlg, IDC_PASSPHRASE_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_PASSPHRASE_CHECK), 
							TRUE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS_CHECK), 
							TRUE); 
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_PASSPHRASE_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_PASSPHRASE_CHECK), 
							FALSE); 
						EnableWindow(GetDlgItem(hwndDlg, IDC_MINCHARS_CHECK), 
							FALSE); 
					}

					if (pConfig->bEnforceMinQuality)
					{
						CheckDlgButton(hwndDlg, IDC_QUALITY_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_QUALITY_CHECK), 
							TRUE); 
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_MINQUALITY_CHECK), 
							TRUE); 
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_QUALITY_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_QUALITY_CHECK), 
							FALSE); 
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_MINQUALITY_CHECK), 
							FALSE); 
					}

					wsprintf(szMinChars, "%d", pConfig->nMinChars);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINCHARS_CHECK), 
						szMinChars);

					wsprintf(szMinQuality, "%d", pConfig->nMinQuality);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINQUALITY_CHECK), 
						szMinQuality);

					if (pConfig->bAllowKeyGen)
					{
						CheckDlgButton(hwndDlg, IDC_KEYGEN_CHECK, 
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYGEN_CHECK),
							TRUE);

						CheckDlgButton(hwndDlg, IDC_KEYSIZE_CHECK,
							BST_CHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_CHECK),
							TRUE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_MINKEYSIZE_CHECK),
							TRUE);

						if (pConfig->bAllowRSAKeyGen)
						{
							CheckDlgButton(hwndDlg, IDC_RSAKEYGEN_CHECK, 
								BST_CHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_RSAKEYGEN_CHECK),
								TRUE);
						}
						else
						{
							CheckDlgButton(hwndDlg, IDC_RSAKEYGEN_CHECK, 
								BST_UNCHECKED);
							EnableWindow(
								GetDlgItem(hwndDlg, IDC_RSAKEYGEN_CHECK),
								FALSE);
						}
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_KEYGEN_CHECK, 
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYGEN_CHECK),
							FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_RSAKEYGEN_CHECK),
							FALSE);

						CheckDlgButton(hwndDlg, IDC_KEYSIZE_CHECK,
							BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_KEYSIZE_CHECK),
							FALSE);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_MINKEYSIZE_CHECK),
							FALSE);
					}

					wsprintf(szMinKeySize, "%d", pConfig->nMinKeySize);
					SetWindowText(GetDlgItem(hwndDlg, IDC_MINKEYSIZE_CHECK), 
						szMinKeySize);

					if (pConfig->bAllowConventionalEncryption)
					{
						CheckDlgButton(hwndDlg, IDC_CONVENTIONAL_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CONVENTIONAL_CHECK),
							TRUE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_CONVENTIONAL_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_CONVENTIONAL_CHECK),
							FALSE);
					}

					if (pConfig->nCommentLength)
					{
						CheckDlgButton(hwndDlg, IDC_COMMENT_CHECK, 
							BST_CHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_COMMENT_CHECK),
							TRUE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_COMMENT), 
							pConfig->szComments);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_COMMENT_CHECK, 
							BST_UNCHECKED);
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_COMMENT_CHECK),
							FALSE);
						SetWindowText(GetDlgItem(hwndDlg, IDC_COMMENT), "");
					}

					if (pConfig->defaultKeySet != NULL)
					{
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_DEFKEYS),
							TRUE);
						ShowKeyring(hwndDlg, IDC_DEFKEYS, pConfig->pflContext,
							pConfig->pgpContext, pConfig->defaultKeySet, 
							TRUE, TRUE, FALSE, FALSE);
					}
					else
					{
						ListView_DeleteAllItems(
							GetDlgItem(hwndDlg, IDC_DEFKEYS));
						EnableWindow(
							GetDlgItem(hwndDlg, IDC_DEFKEYS),
							FALSE);
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
