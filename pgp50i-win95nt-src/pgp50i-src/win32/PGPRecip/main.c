/*
 * main.c  PGPrecipient.dialog entry routine
 *
 * This is the entry routine for the DLL as well as the main entry
 * call for the recipient dialog. The main routine gets the keys off
 * of the disk, tries to match them, and then decides whether
 * to pop the dialog or do things silently and automatically.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason,LPVOID lpvReserved)
{
    switch (fdwReason) {
       case DLL_PROCESS_ATTACH:
          g_hinst = hinstDll;
          //vErrorOut(fg_white,"DLL_PROCESS_ATTACH\n");
          break;
        case DLL_THREAD_ATTACH:
            //vErrorOut(fg_white,"DLL_THREAD_ATTACH\n");
           break;
        case DLL_THREAD_DETACH:
            //vErrorOut(fg_red,"DLL_THREAD_DETACH\n");
           break;
        case DLL_PROCESS_DETACH:
            //vErrorOut(fg_red,"DLL_PROCESS_DETACH\n");
           break;
    }
    return(TRUE);
}

UINT PGPExport PGPRecipientDialog(PRECIPIENTDIALOGSTRUCT prds)
{
    UINT ReturnValue = RCPT_DLG_ERROR_FAILURE;
    BOOL DialogReturn = FALSE;
    int RSAandDSA;
    int PopDialog;
    PRECGBL pRecGbl;
    PUSERKEYINFO gUserLinkedList;

    assert(prds);

    LoadKeyserverSupport();

    PopDialog=TRUE;

    if((prds->szRecipients==0)&&(prds->dwNumRecipients!=0))
        return RCPT_DLG_ERROR_FAILURE;

    if(prds->sVersion != 0x0101)
    {
        return RCPT_DLG_ERROR_VERSION;
    }

    PGPM_RELOADKEYRING = RegisterWindowMessage("PGPM_RELOADKEYRING");

    if(AddUsersToLists(&gUserLinkedList, prds->dwNumRecipients,
                       prds->szRecipients,TRUE))
    {
        PopDialog=FALSE;
        DialogReturn=TRUE;

        RSAandDSA=WriteOutLinkedListKeyIds(prds,gUserLinkedList);

        if(RSAandDSA)
            if(RSADSARecWarning(NULL)==FALSE)
                PopDialog=TRUE;
    }

    prds->hwndParent=0;

    if((PopDialog)||(prds->dwDisableFlags&DISABLE_AUTO_POP_DIALOG))
    {
		WNDCLASS wndclass;

        pRecGbl=AllocRecGbl();

        pRecGbl->prds=prds;
        pRecGbl->gUserLinkedList=gUserLinkedList;

	    wndclass.style = 0;
	    wndclass.lpfnWndProc = (WNDPROC)DefDlgProc;
	    wndclass.cbClsExtra = 0;
	    wndclass.cbWndExtra = DLGWINDOWEXTRA;
	    wndclass.hInstance = g_hinst;
	    wndclass.hIcon = LoadIcon (g_hinst, MAKEINTRESOURCE(IDI_DLGICON));
	    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	    wndclass.hbrBackground = NULL;
        wndclass.lpszMenuName = NULL;
	    wndclass.lpszClassName = "PGPrecipClass";

        RegisterClass (&wndclass);

        DialogReturn = DialogBoxParam(g_hinst,
                                    MAKEINTRESOURCE( IDD_RECIPIENTDLG ),
                                    prds->hwndParent,
                                    (DLGPROC) RecipientDlgProc,
                                    (LPARAM) pRecGbl);
    }

    if(DialogReturn)
    {
        ReturnValue = RCPT_DLG_ERROR_SUCCESS;
    }

    FreeLinkedLists(gUserLinkedList);

    UnLoadKeyserverSupport();

    return ReturnValue;
}

// In order for the recipient dialog to be reentrant, these sneaky
// routines are used to put our "global" variables as a structure linked
// to the USERDATA portion of each recipientdialog's window.

PRECGBL AllocRecGbl(void)
{
    PRECGBL pRecGbl;

    pRecGbl=(PRECGBL)pgpMemAlloc(sizeof(RECGBL));
    memset (pRecGbl,0x00,sizeof(RECGBL));

    return pRecGbl;
}

int InitRecGbl(HWND hwnd,PRECGBL pRecGbl)
{
    SetWindowLong(hwnd,GWL_USERDATA,(long)pRecGbl);

    return TRUE;
}

PRECGBL RecGbl(HWND hwnd)
{
//    vErrorOut(fg_yellow,"%lX ",hwnd);
    return (PRECGBL)GetWindowLong(hwnd,GWL_USERDATA);
}

int FreeRecGbl(HWND hwnd)
{
    PRECGBL pRecGbl;

    pRecGbl=RecGbl(hwnd);

    pgpFree(pRecGbl);

    return TRUE;
}
