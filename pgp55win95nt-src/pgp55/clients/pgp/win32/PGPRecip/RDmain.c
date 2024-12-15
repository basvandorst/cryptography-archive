/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDmain.c,v 1.23.4.1 1997/11/20 21:17:47 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * main.c  PGPrecipient.dialog entry routine
 *
 * This is the entry routine for the DLL as well as the main entry
 * call for the recipient dialog. The main routine gets the keys off
 * of the disk, tries to match them, and then decides whether 
 * to pop the dialog or do things silently and automatically.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


BOOL GetAdminPrefs(HWND hwnd,PGPContextRef context,
	PGPKeySetRef OrigKeySet,
	PPREFS Prefs)
{
#if PGP_BUSINESS_SECURITY || PGP_ADMIN_BUILD
	PGPPrefRef	AdminPrefRef;
	PGPError	err;
	PGPByte		*rawKeyID;
	PGPSize		rawKeyIDlen;
#endif // PGP_BUSINESS_SECURITY

// default value for non-biz edition
	Prefs->bEnforceRemoteADK=FALSE;
	Prefs->bUseOutgoingADK=FALSE;
	Prefs->bEnforceOutgoingADK=FALSE;
	Prefs->bAllowConventionalEncryption=TRUE;
	Prefs->bWarnNotCorpSigned=FALSE;
	Prefs->CorpKey=0;
	Prefs->outADKKeyAlgorithm=0;
	Prefs->corpKeyAlgorithm=0;
	strcpy(Prefs->outADKKeyID,"");
	strcpy(Prefs->corpKeyID,"");

#if PGP_BUSINESS_SECURITY || PGP_ADMIN_BUILD
#if PGP_ADMIN_BUILD				
	err = PGPcomdlgOpenAdminPrefs (&AdminPrefRef, TRUE);
#else
	err = PGPcomdlgOpenAdminPrefs (&AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY_ADMIN
	if (IsPGPError (err)) 
		return FALSE;	// error-out

	PGPGetPrefBoolean (AdminPrefRef, kPGPPrefEnforceRemoteADKClass , 
		&(Prefs->bEnforceRemoteADK));
	PGPGetPrefBoolean (AdminPrefRef, kPGPPrefUseOutgoingADK , 
		&(Prefs->bUseOutgoingADK));
	PGPGetPrefBoolean (AdminPrefRef, kPGPPrefEnforceOutgoingADK, 
		&(Prefs->bEnforceOutgoingADK));
	PGPGetPrefBoolean (AdminPrefRef, kPGPPrefAllowConventionalEncryption, 
		&(Prefs->bAllowConventionalEncryption));
	if(Prefs->bUseOutgoingADK)
	{
		err=PGPGetPrefData(AdminPrefRef, 
			kPGPPrefOutgoingADKID, 
			&rawKeyIDlen, 
			&rawKeyID);

		strcpy(Prefs->outADKKeyID,rawKeyID);

		PGPGetPrefNumber( AdminPrefRef,
				kPGPPrefOutADKPublicKeyAlgorithm, 
				&(Prefs->outADKKeyAlgorithm));
			
		err = PGPDisposePrefData(AdminPrefRef,rawKeyID);
	}

	PGPGetPrefBoolean(AdminPrefRef,kPGPPrefWarnNotCertByCorp,
		&(Prefs->bWarnNotCorpSigned));

	if(Prefs->bWarnNotCorpSigned)
	{
	// Get the corporate key if we're supposed to verify that all
	// recipients have been signed with it.	
		PGPGetPrefData(AdminPrefRef, 
			kPGPPrefCorpKeyID, 
			&rawKeyIDlen, 
			&rawKeyID);

		strcpy(Prefs->corpKeyID,rawKeyID);

		PGPGetPrefNumber( AdminPrefRef,
				kPGPPrefCorpKeyPublicKeyAlgorithm, 
				&(Prefs->corpKeyAlgorithm));

		PGPDisposePrefData(AdminPrefRef,rawKeyID);
	}
	
	PGPcomdlgCloseAdminPrefs (AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY

	return TRUE;
}
 

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


BOOL GetClientPrefs(PPREFS Prefs)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPError	err;

	Prefs->bMarginalInvalid=FALSE;
	Prefs->bEncToSelf=FALSE;
	Prefs->bSynchNotFound=FALSE;
	Prefs->bWarnOnADK=TRUE;

	err=PGPcomdlgOpenClientPrefs (&PrefRefClient);
	if(IsntPGPError(err))
	{
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefMarginalIsInvalid, &(Prefs->bMarginalInvalid));
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefEncryptToSelf, &(Prefs->bEncToSelf));
		PGPGetPrefBoolean (PrefRefClient,
			kPGPPrefKeyServerSyncUnknownKeys, &(Prefs->bSynchNotFound));
		PGPGetPrefBoolean (PrefRefClient, 
			kPGPPrefWarnOnADK, &(Prefs->bWarnOnADK));

		PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

		return TRUE;
	}
	return FALSE;
}

UINT PGPrdExport PGPRecipientDialog(PRECIPIENTDIALOGSTRUCT prds)
{
    UINT ReturnValue = FALSE;
    BOOL DialogReturn = FALSE;
    int RSAandDSA;
    int PopDialog;
    PRECGBL pRecGbl;
    PUSERKEYINFO gUserLinkedList;
	UINT AddUserRetVal;
	PPREFS Prefs;
/*
	int i;

	for(i=0;i<prds->dwNumRecipients;i++)
	{
		char temp[256];
		sprintf(temp,"*%s* %d",prds->szRecipientArray[i],
			strlen(prds->szRecipientArray[i]));
		MessageBox(NULL,temp,"test",MB_OK);
	}
*/
    assert(prds);

	prds->SelectedKeySetRef=0;

	if(prds->Version!=CurrentPGPrecipVersion)
	{
		MessageBox(prds->hwndParent,
			"You are using an incompatible version of PGPrecip.dll!\n\n"
			"Please try reinstalling the program.",
			"PGPrecip.dll: Wrong Version",MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	Prefs=(PPREFS)malloc(sizeof(PREFS));

	if(!GetAdminPrefs(
		prds->hwndParent,
		prds->Context,
		prds->OriginalKeySetRef,
		Prefs))
		return FALSE;

	GetClientPrefs(Prefs);

	PGPcomdlgOpenGroupFile(&(Prefs->Group));

/*	// PGP Freeware does not support groups in recipient dialog
#if PGP_FREEWARE
	Prefs->Group=0;
#else
*/
	PGPcomdlgOpenGroupFile(&(Prefs->Group));
//#endif

    PopDialog=TRUE;
    
    PGPM_RELOADKEYRING = RegisterWindowMessage("PGPM_RELOADKEYRING");

	if(!AddUsersToLists(prds->hwndParent,
		&gUserLinkedList,prds,Prefs,&AddUserRetVal))
		return FALSE;

    if(AddUserRetVal==ADDUSER_OK)
    {
		PGPUInt32 NumKeys;

        PopDialog=FALSE;
        DialogReturn=TRUE;

        RSAandDSA=RecipientSetFromList(prds,gUserLinkedList,&NumKeys);

		if(NumKeys==0)
			PopDialog=TRUE;
    
        if(RSAandDSA)
		{
            if(RSADSARecWarning(NULL)==FALSE)
			{
                PopDialog=TRUE;
				PGPFreeKeySet(prds->SelectedKeySetRef);
				prds->SelectedKeySetRef=0;
			}
		}
	}

 //   prds->hwndParent=0; // Because of message notifications....

    if((PopDialog)||(prds->dwDisableFlags&PGPCOMDLG_DISABLE_AUTOMODE))
    {
		WNDCLASS wndclass;

        pRecGbl=AllocRecGbl();

        pRecGbl->prds=prds;
        pRecGbl->gUserLinkedList=gUserLinkedList;
		pRecGbl->AddUserRetVal=AddUserRetVal;
		pRecGbl->Prefs=Prefs;

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

		gUserLinkedList=pRecGbl->gUserLinkedList;
		free(pRecGbl);
    }
 
    if(DialogReturn)
    {
        ReturnValue = TRUE;
    }

    FreeLinkedLists(gUserLinkedList);
	PGPcomdlgCloseGroupFile(Prefs->Group);
	free(Prefs);

    return ReturnValue;
}

// In order for the recipient dialog to be reentrant, these sneaky
// routines are used to put our "global" variables as a structure linked
// to the USERDATA portion of each recipientdialog's window.

PRECGBL AllocRecGbl(void)
{
    PRECGBL pRecGbl;

    pRecGbl=(PRECGBL)malloc(sizeof(RECGBL));
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


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
