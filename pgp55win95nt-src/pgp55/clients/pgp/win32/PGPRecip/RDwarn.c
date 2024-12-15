/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDwarn.c,v 1.3 1997/09/03 17:14:59 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

/*
 * WarnProc.c  Main message loop for the warning dialog
 *
 * This message loop handles all the operations of the warning
 * dialog. Its a bit trickey, since it includes some routines to tack
 * on our "global" variables to the USERDATA portion of the window
 * (for reentrancy)
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


#define WARNREG "Software\\PGP\\PGPlib"
#define WARN_REC 1
#define WARN_SIG 2

static char RSADSArec[]={
"You have mixed RSA and Diffie-Hellman/DSS keys "
"in your recipient list. "
"Users of PGP versions prior to 5.0 may not "
"be able to decrypt this.\n\n"
"Are you sure you want to do this?"};

PWARNGBL AllocWarnGbl(void)
{
    PWARNGBL pWarnGbl;

    pWarnGbl=(PWARNGBL)malloc(sizeof(WARNGBL));
    memset (pWarnGbl,0x00,sizeof(WARNGBL));

    return pWarnGbl;
}

int InitWarnGbl(HWND hwnd,PWARNGBL pWarnGbl)
{
    SetWindowLong(hwnd,GWL_USERDATA,(long)pWarnGbl);

    return TRUE;
}

PWARNGBL WarnGbl(HWND hwnd)
{
//    vErrorOut(fg_yellow,"%lX ",hwnd);
    return (PWARNGBL)GetWindowLong(hwnd,GWL_USERDATA);
}

int FreeWarnGbl(HWND hwnd)
{
    PWARNGBL pWarnGbl;

    pWarnGbl=WarnGbl(hwnd);

    free(pWarnGbl);

    return TRUE;
}


BOOL WINAPI WarningDlgProc(HWND hdlg, UINT uMsg, 
                           WPARAM wParam, LPARAM lParam)
{

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            InitWarnGbl(hdlg,(PWARNGBL)lParam);

            SetWindowText(GetDlgItem(hdlg,IDC_WARNINGTEXT),
                (WarnGbl(hdlg))->gWarnText);

            Button_SetCheck(GetDlgItem(hdlg,IDC_WARNCHECK),
                *((WarnGbl(hdlg))->pWarnCheck));
 
            return TRUE;
        }


        case WM_DESTROY:
            {
            FreeWarnGbl(hdlg);

            EndDialog(hdlg, FALSE);
            break;
            }

        case WM_COMMAND:
        {
            switch(wParam)
            {
                case IDC_WARNCHECK:
                {
                    *((WarnGbl(hdlg))->pWarnCheck)=
                        !*((WarnGbl(hdlg))->pWarnCheck);
                    Button_SetCheck(GetDlgItem(hdlg,IDC_WARNCHECK),
                        *((WarnGbl(hdlg))->pWarnCheck));
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
                    char szHelpFile[500];

                    GetHelpFile(szHelpFile);

                    WinHelp (hdlg,szHelpFile, HELP_CONTEXT, 
                        (WarnGbl(hdlg))->gIDH); 
                    break;
                }

            }
            return TRUE;
        }

    }
    return FALSE;
}

BOOL ReadWarnings(PGPBoolean *pbWarnOnMix)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;

	*pbWarnOnMix=TRUE;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefWarnOnMixRSAElGamal, pbWarnOnMix);
		PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

		return TRUE;
	}
	return FALSE;
}

BOOL WriteWarnings(PGPBoolean bWarnOnMix)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	if(IsntPGPError(err))
	{
		PGPSetPrefBoolean (PrefRefClient,
			kPGPPrefWarnOnMixRSAElGamal, bWarnOnMix);
		PGPcomdlgCloseClientPrefs (PrefRefClient, TRUE);

		return TRUE;
	}
	return FALSE;
}


int RSADSARecWarning(HWND hwnd)
{
    int ReturnValue;
    unsigned char bWarnOnMix;
	BOOL WarnCheck;
    PWARNGBL pWarnGbl;

    ReadWarnings(&bWarnOnMix);

	WarnCheck=!bWarnOnMix;

    if(WarnCheck)
        return TRUE;
 
    pWarnGbl=AllocWarnGbl();

    pWarnGbl->gIDH=IDH_RSADSARECWARNING;
    pWarnGbl->pWarnCheck=&WarnCheck;
    pWarnGbl->gWarnText=RSADSArec;

    ReturnValue=DialogBoxParam(    g_hinst, 
                                MAKEINTRESOURCE( IDD_WARNINGDLG ),
                                hwnd, 
                                (DLGPROC) WarningDlgProc, 
                                (LPARAM) pWarnGbl);

	bWarnOnMix=!WarnCheck;

    if((!bWarnOnMix)&&(ReturnValue))
        WriteWarnings(bWarnOnMix);

    return ReturnValue;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

