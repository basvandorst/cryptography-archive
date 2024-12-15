/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "define.h"
#include "resource.h"

#include "errs.h"

#include "EnclyptorViewClipboard.h"
#include "clipboard.h"
#include "viewers.h"

extern HINSTANCE g_hinst;

typedef struct _LaunchViewerProcArgs
{
	char *DecryptionResultsString;
}LAUNCHVIEWERPROCARGS, *PLAUNCHVIEWERPROCARGS;

BOOL WINAPI LaunchViewerProc(HWND hdlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam);

UINT DisplayDecryptionResults(HWND hwnd, char *DecryptionResults)
{
	LAUNCHVIEWERPROCARGS LVPArgs;
	LPCTSTR Resource = MAKEINTRESOURCE(IDD_RESULTS_SMALL);

	assert(DecryptionResults);

	LVPArgs.DecryptionResultsString = DecryptionResults;

	DialogBoxParam(g_hinst,
				  Resource,
				  hwnd,
				  (DLGPROC) LaunchViewerProc,
				  (LPARAM) &LVPArgs);

	return(SUCCESS);
}

BOOL WINAPI LaunchViewerProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PLAUNCHVIEWERPROCARGS pLVPArgs = NULL;

	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			assert(lParam);

			if(lParam)
			{
				pLVPArgs = (PLAUNCHVIEWERPROCARGS) lParam;

				if(pLVPArgs->DecryptionResultsString)
				{
					SetDlgItemText(hdlg,
								   IDC_DECRYPTION_RESULTS,
								   pLVPArgs->DecryptionResultsString);
				}
				SetWindowLong(hdlg, GWL_USERDATA, (LONG) pLVPArgs);
			}

			return FALSE;
		}

		case WM_CLOSE:
		{
			EndDialog(hdlg, TRUE);
			break;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case IDOK:
				{
					EndDialog(hdlg, TRUE);
					break;
				}

				case IDVIEW_EXTERNAL:
				{
					ShowWindow(hdlg, SW_HIDE);
					LaunchExternalViewer(hdlg);
					EndDialog(hdlg, TRUE);
					break;
				}

				case IDVIEW_INTERNAL:
				{
					ShowWindow(hdlg, SW_HIDE);
					LaunchInternalViewer(hdlg);
					EndDialog(hdlg, FALSE);
					break;
				}
			}
			return TRUE;
		}

	}
	return FALSE;
}
