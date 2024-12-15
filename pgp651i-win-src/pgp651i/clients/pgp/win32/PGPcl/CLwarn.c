/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
		

	$Id: CLwarn.c,v 1.2 1998/08/11 14:43:20 pbj Exp $
____________________________________________________________________________*/

#include "pgpPFLConfig.h"
#include <windowsx.h>

// project header files
#include "pgpclx.h"

// external globals
extern HINSTANCE g_hInst;

typedef struct
{
int WarnHelpID;  
PGPBoolean *pWarnCheck;
char *szWarnMessage;
char *szWarnCaption;
} WARNGBL, *PWARNGBL;

#define IDH_RSADSARECWARNING               3009

/*
 * WarnProc.c  Main message loop for the warning dialog
 *
 * This message loop handles all the operations of the warning
 * dialog. Its a bit trickey, since it includes some routines to tack
 * on our "global" variables to the USERDATA portion of the window
 * (for reentrancy)
 *
 */

PWARNGBL AllocWarnGbl(void)
{
    PWARNGBL pWarnGbl;

    pWarnGbl=(PWARNGBL)malloc(sizeof(WARNGBL));
    memset (pWarnGbl,0x00,sizeof(WARNGBL));

    return pWarnGbl;
}

BOOL WINAPI WarningDlgProc(HWND hdlg, UINT uMsg, 
                           WPARAM wParam, LPARAM lParam)
{

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
			PWARNGBL pwg;

			SetWindowLong(hdlg,GWL_USERDATA,lParam);
			pwg=(PWARNGBL)lParam;

            SetWindowText(GetDlgItem(hdlg,IDC_WARNINGTEXT),
                pwg->szWarnMessage);

            SetWindowText(hdlg,pwg->szWarnCaption);

            Button_SetCheck(GetDlgItem(hdlg,IDC_WARNCHECK),
                *(pwg->pWarnCheck));
 
            return TRUE;
        }


        case WM_DESTROY:
        {
			PWARNGBL pwg;

			pwg=(PWARNGBL)GetWindowLong(hdlg,GWL_USERDATA);

            free(pwg);
			
            EndDialog(hdlg, FALSE);
            break;
        }

        case WM_COMMAND:
        {
            switch(wParam)
            {
                case IDC_WARNCHECK:
                {
					PWARNGBL pwg;

					pwg=(PWARNGBL)GetWindowLong(hdlg,GWL_USERDATA);

                    *(pwg->pWarnCheck)=!*(pwg->pWarnCheck);
                    Button_SetCheck(GetDlgItem(hdlg,IDC_WARNCHECK),
                        *(pwg->pWarnCheck));
                    break;
                }

                case IDOK:
                {
                    EndDialog(hdlg, TRUE);
                    break;
                }

                case IDCANCEL:
                {
                    EndDialog(hdlg, FALSE);
                    break;
                }

                case IDHELP:
                {
                    char szHelpFile[MAX_PATH],sz[MAX_PATH];
					PWARNGBL pwg;

					pwg=(PWARNGBL)GetWindowLong(hdlg,GWL_USERDATA);

					PGPclGetPGPPath (szHelpFile, sizeof(szHelpFile));
					LoadString (g_hInst, IDS_HELPFILENAME, sz, sizeof(sz));
					lstrcat (szHelpFile, sz);


                    WinHelp (hdlg,szHelpFile, HELP_CONTEXT,pwg->WarnHelpID); 
                    break;
                }

            }
            return TRUE;
        }

    }
    return FALSE;
}

//	__________________________________________________________
//
//  Post warning about encrypting to RSA and signing w/DSA key

PGPError PGPclExport 
PGPclRSADSAMixWarning (HWND hWnd, PGPBoolean *pbNeverShowAgain) 
{
    int ReturnValue;
    PWARNGBL pWarnGbl;
 	CHAR szCaption [128];
	CHAR szMessage [256];

	LoadString (g_hInst, IDS_CAPTION, szCaption, sizeof(szCaption));
	LoadString (g_hInst, IDS_RSADSAMIXWARNING, szMessage, sizeof(szMessage));

	pWarnGbl=AllocWarnGbl();

    pWarnGbl->WarnHelpID=IDH_RSADSARECWARNING;
    pWarnGbl->pWarnCheck=pbNeverShowAgain;
    pWarnGbl->szWarnMessage=szMessage;
	pWarnGbl->szWarnCaption=szCaption;

    ReturnValue=DialogBoxParam(g_hInst, 
						MAKEINTRESOURCE( IDD_WARNINGDLG ),
						hWnd, 
						(DLGPROC) WarningDlgProc, 
						(LPARAM) pWarnGbl);

	if(ReturnValue)
		return kPGPError_NoErr;

	return kPGPError_UserAbort;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

