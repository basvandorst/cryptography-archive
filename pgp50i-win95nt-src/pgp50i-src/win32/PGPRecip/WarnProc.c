/*
 * WarnProc.c  Main message loop for the warning dialog
 *
 * This message loop handles all the operations of the warning
 * dialog. Its a bit trickey, since it includes some routines to tack
 * on our "global" variables to the USERDATA portion of the window
 * (for reentrancy)
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 */

#include "precomp.h"

#define WARNREG "Software\\PGP\\PGPlib"
#define WARN_REC 1
#define WARN_SIG 2

static char RSADSArec[]={
"You have mixed RSA and DSS/Diffie-Hellman keys "
"in your recipient list. "
"Users of PGP versions prior to 5.0 may not "
"be able to decrypt this.\n\n"
"Are you sure you want to do this?"};

PWARNGBL AllocWarnGbl(void)
{
    PWARNGBL pWarnGbl;

    pWarnGbl=(PWARNGBL)pgpMemAlloc(sizeof(WARNGBL));
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

    pgpFree(pWarnGbl);

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

int ReadWarnings(void)
{
    DWORD Size,Type;
    HKEY hkey;
    int Warnings;
    int RegValue;

    Warnings=0;

    if(ERROR_SUCCESS ==
                RegOpenKeyEx(HKEY_CURRENT_USER,WARNREG,
                0,KEY_READ,&hkey))
    {
        Size=sizeof(int);
        if(ERROR_SUCCESS==
            RegQueryValueEx(hkey,"Warning_Flags", 0,&Type,
                            (char *)&(RegValue),&Size))
            {
                Warnings=RegValue;
            }
        RegCloseKey(hkey);
    }

    return Warnings;
}

void WriteWarnings(int Warnings)
{
    HKEY hkey;

    if(ERROR_SUCCESS ==
        RegOpenKeyEx(HKEY_CURRENT_USER,WARNREG,
            0,KEY_ALL_ACCESS,&hkey))
    {
    //    vErrorOut(fg_yellow,"Writing\n");

        RegSetValueEx(hkey,"Warning_Flags", 0,REG_DWORD,
             (BYTE*)&Warnings,sizeof(int));
        RegCloseKey(hkey);
    }
}

int RSADSARecWarning(HWND hwnd)
{
    int ReturnValue;
    int Warnings;
    BOOL WarnCheck;
    PWARNGBL pWarnGbl;

    Warnings=ReadWarnings();
    WarnCheck=(BOOL)Warnings&WARN_REC;

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

    if((WarnCheck)&&(ReturnValue))
        WriteWarnings(Warnings|WARN_REC);

    return ReturnValue;
}
