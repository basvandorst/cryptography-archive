/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Review.cpp,v 1.20 1999/03/12 22:00:28 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include <commctrl.h>
#include "resource.h"
#include "pgpMem.h"
#include "pgpAdminPrefs.h"
#include "PGPcl.h"

BOOL CALLBACK ReviewDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL			bReturnCode = FALSE;
	pgpConfigInfo *	pConfig		= NULL;
	DWORD			dwStart;
	DWORD			dwEnd;

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

		SendDlgItemMessage(hwndDlg, IDC_PANEL, EM_GETSEL, (WPARAM) &dwStart,
			(LPARAM) &dwEnd);

		if (dwStart != -1)
			SendDlgItemMessage(hwndDlg, IDC_PANEL, EM_SETSEL, -1, 0);
		break;
		
	case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
			case PSN_SETACTIVE:
				{
					char *szPanel;
					char szTemp[256];
					int nTabSize;
					PGPUInt32 nIndex;

					szPanel = (char *) calloc(1, 8192);
					if (szPanel == NULL)
						return FALSE;

					// Initialize window
					PostMessage(GetParent(hwndDlg),
						PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(hwndDlg, IDC_WIZBITMAP, STM_SETIMAGE, 
						IMAGE_BITMAP, (LPARAM) pConfig->hBitmap);

					nTabSize = 20;
					SendDlgItemMessage(hwndDlg, IDC_PANEL, EM_SETTABSTOPS, 
						1, (LPARAM) &nTabSize);

					strcpy(szPanel, "Company Name:\r\n");
					
					if (pConfig->szLicenseNum != NULL)
						wsprintf(szTemp, "\t%s\r\n\r\n", 
							pConfig->szLicenseNum);
					else
						strcpy(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Incoming ADK:\r\n");
					
					if (pConfig->bUseIncomingADK)
					{
						wsprintf(szTemp, "\t%s\r\n", pConfig->szIncomingADK);
						if (pConfig->bEnforceIncomingADK)
							strcat(szTemp, "\tEnforced\r\n\r\n");
						else
							strcat(szTemp, "\tNot Enforced\r\n\r\n");
					}
					else
						strcpy(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Outgoing ADK:\r\n");
					
					if (pConfig->bUseOutgoingADK)
					{
						wsprintf(szTemp, "\t%s\r\n", pConfig->szOutgoingADK);
						if (pConfig->bEnforceOutgoingADK)
							strcat(szTemp, "\tEnforced\r\n\r\n");
						else
							strcat(szTemp, "\tNot Enforced\r\n\r\n");
					}
					else
						strcpy(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Enforce Remote ADK:\r\n");
					
					if (pConfig->bEnforceRemoteADK)
						strcat(szPanel, "\tYes\r\n\r\n");
					else
						strcat(szPanel, "\tNo\r\n\r\n");

					strcat(szPanel, "PGPdisk ADK:\r\n");
					
					if (pConfig->bUseDiskADK)
						wsprintf(szTemp, "\t%s\r\n\r\n", pConfig->szDiskADK);
					else
						strcpy(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Enforce Minimum Passphrase "
									"Length:\r\n");

					if (pConfig->bEnforceMinChars)
						wsprintf(szTemp, "\tYes\t%d characters\r\n\r\n",
							pConfig->nMinChars);
					else
						strcpy(szTemp, "\tNo\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Enforce Minimum Passphrase "
									"Quality:\r\n");

					if (pConfig->bEnforceMinQuality)
						wsprintf(szTemp, "\tYes\t%d%%\r\n\r\n",
							pConfig->nMinQuality);
					else
						strcpy(szTemp, "\tNo\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Corporate Key:\r\n");

					if (pConfig->bAutoSignTrustCorp)
					{
						wsprintf(szTemp, "\t%s\r\n", pConfig->szCorpKey);
						strcat(szTemp, "\tAuto-Sign\r\n");
						if (pConfig->bMetaIntroducerCorp)
							strcat(szTemp, "\tMeta-Introducer\r\n");
						strcat(szTemp, "\r\n");
					}
					else
						wsprintf(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);

					if (pConfig->bAutoSignTrustCorp)
					{
						strcat(szPanel, "Encrypt to Keys Not Signed by "
										"Corporate Key:\r\n");

						if (pConfig->bWarnNotCertByCorp)
							strcat(szPanel, "\tWarn\r\n\r\n");
						else
							strcat(szPanel, "\tDo Not Warn\r\n\r\n");
					}

					strcat(szPanel, "Designated Revoker key:\r\n");

					if (pConfig->bAutoAddRevoker)
						wsprintf(szTemp, "\t%s\r\n\r\n", 
							pConfig->szRevokerKey);
					else
						wsprintf(szTemp, "\t<none>\r\n\r\n");

					strcat(szPanel, szTemp);
					strcat(szPanel, "Key Generation performs "
									"X.509 Certificate Requests:\r\n");
					
					if (pConfig->bKeyGenCertRequest)
						strcat(szPanel, "\tYes\r\n\r\n");
					else
						strcat(szPanel, "\tNo\r\n\r\n");

					strcat(szPanel, "Allow manual X.509 "
									"Certificate Requests:\r\n");
					
					if (pConfig->bAllowManualCertRequest)
						strcat(szPanel, "\tYes\r\n\r\n");
					else
						strcat(szPanel, "\tNo\r\n\r\n");

					strcat(szPanel, "Automatically update X.509 "
									"Certificate Revocation Lists:\r\n");

					if (pConfig->bAutoUpdateCRL)
						strcat(szPanel, "\tYes\r\n\r\n");
					else
						strcat(szPanel, "\tNo\r\n\r\n");

					strcat(szPanel, "Default CA:\r\n");

					switch (pConfig->nCAType)
					{
					case kPGPKeyServerClass_Invalid:
						strcat(szPanel, "\tNone\r\n\r\n");
						break;

					case kPGPKeyServerClass_NetToolsCA:
						strcat(szPanel, "\tNet Tools PKI Server\r\n\r\n");
						break;

					case kPGPKeyServerClass_Verisign:
						strcat(szPanel, "\tVeriSign OnSite\r\n\r\n");
						break;

					case kPGPKeyServerClass_Entrust:
						strcat(szPanel, "\tEntrust\r\n\r\n");
						break;
					}

					if ((pConfig->nCAType != kPGPKeyServerClass_Invalid) &&
						(pConfig->pAVList != NULL) && 
						(pConfig->nNumAVs > 0))
					{
						strcat(szPanel, "Default CA Attributes:\r\n");

						for (nIndex=0; nIndex<pConfig->nNumAVs; nIndex++)
						{
							PGPclGetAVListAttributeString(
								pConfig->pAVList[nIndex].attribute,
								szTemp,
								254);

							strcat(szPanel, "\t");
							strcat(szPanel, szTemp);
							strcat(szPanel, " = ");

							strncpy(szTemp, (char *) 
								pConfig->pAVList[nIndex].value.pointervalue,
								pConfig->pAVList[nIndex].size);
							szTemp[pConfig->pAVList[nIndex].size] = 0;

							strcat(szPanel, szTemp);
							strcat(szPanel, "\r\n");
						}

						strcat(szPanel, "\r\n");
					}

					strcat(szPanel, "Automatic Updates:\r\n");

					if (pConfig->bUpdateTrustedIntroducers)
					{
						wsprintf(szTemp, "\tEvery %d days, "
							"Trusted Introducers\r\n", 
							pConfig->nDaysUpdateTrustedIntroducers);

						strcat(szPanel, szTemp);
					}

					if (pConfig->bUpdateAllKeys)
					{
						wsprintf(szTemp, "\tEvery %d days, All Keys\r\n\r\n",
							pConfig->nDaysUpdateAllKeys);

						strcat(szPanel, szTemp);
					}

					if (!pConfig->bUpdateAllKeys && 
						!pConfig->bUpdateTrustedIntroducers)
						strcat(szPanel, "\t<none>\r\n\r\n");
					else
						strcat(szPanel, "\r\n");

					strcat(szPanel, "Key Generation:\r\n");

					if (pConfig->bAllowKeyGen)
					{
						strcat(szPanel, "\tAllowed");
						
						if (pConfig->bAllowRSAKeyGen)
							strcat(szPanel, "\tRSA Algorithm "
											"Enabled\r\n\r\n");
						else
							strcat(szPanel, "\tRSA Algorithm "
											"Disabled\r\n\r\n");

						strcat(szPanel, "Minimum Key Generation Size:\r\n");
						wsprintf(szTemp, "\t%d\r\n\r\n", 
							pConfig->nMinKeySize);
						strcat(szPanel, szTemp);
					}
					else
						strcat(szPanel, "\tNot Allowed\r\n\r\n");

					strcat(szPanel, "Allow Conventional Encryption:\r\n");

					if (pConfig->bAllowConventionalEncryption)
						strcat(szPanel, "\tYes\r\n\r\n");
					else
						strcat(szPanel, "\tNo\r\n\r\n");

					strcat(szPanel, "Message Header Comment:\r\n");

					if (pConfig->nCommentLength)
					{
						strcat(szPanel, "\t");
						strcat(szPanel, pConfig->szComments);
						strcat(szPanel, "\r\n\r\n");
					}
					else
						strcat(szPanel, "\t<none>\r\n\r\n");

					strcat(szPanel, "Pre-Loaded Keys:\r\n");

					if (pConfig->defaultKeySet != NULL)
					{
						PGPKeyListRef	keyList;
						PGPKeyIterRef	keyIter;
						PGPKeyRef		key;
						PGPUInt32		nLength;
						char *			szUserID[kPGPMaxUserIDSize+1];

						PGPOrderKeySet(pConfig->defaultKeySet,
							kPGPUserIDOrdering, &keyList);
					    PGPNewKeyIter(keyList, &keyIter);
						PGPKeyIterNext(keyIter, &key);
						while (key)
						{
							PGPGetPrimaryUserIDNameBuffer(key,
								kPGPMaxUserIDSize, (char*)szUserID,
								&nLength);

							wsprintf(szTemp, "\t%s\r\n", szUserID);
							strcat(szPanel, szTemp);
							PGPKeyIterNext(keyIter, &key);
						}

						PGPFreeKeyIter(keyIter);
						PGPFreeKeyList(keyList);
						strcat(szPanel, "\r\n");
					}
					else
						strcat(szPanel, "\t<none>\r\n\r\n");

					SetDlgItemText(hwndDlg, IDC_PANEL, szPanel);
					free(szPanel);
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
