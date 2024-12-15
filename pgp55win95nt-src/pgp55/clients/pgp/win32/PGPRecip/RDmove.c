/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDmove.c,v 1.30 1997/10/17 15:57:16 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * MoveItem.c  List View movement routines that support ADK
 *
 * These routines use the algorithm developed by Mark W. to support ADK
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


// ADKWarnPlease
//
// Display this warning if the user removes an ADK'ed key from
// the recipient list.

void ADKWarnPlease(HWND hwnd,char *UserID)
{
	MessageBox(hwnd,
"Removing the Additional Decryption Keys from the recipient list\n"
"may violate the policy established for other keys in the recipient list.",
		UserID,MB_OK|MB_ICONEXCLAMATION);
}

void NonCorpSignedWarnPlease(HWND hwnd,char *UserID)
{
	MessageBox(hwnd,
"Some recipient keys are not signed by the Corporate Signing Key. Encrypting\n"
"to these keys may be a violation of corporate security policy.",
		UserID,MB_OK|MB_ICONEXCLAMATION);
}

// MoveMarkedItems
//
// Use this routine after you have marked all the items you wish to
// move with the PUIF_MOVEME flag

int MoveMarkedItems(HWND src, HWND dest)
{
    LV_ITEM lvi;
    int iCount, i;
    PUSERKEYINFO pui;

    iCount = ListBx_GetItemCount( src );

    if(iCount==0)
        return FALSE;

    for( i = 0; i < iCount; i++ )            //  Loop through each item
    {     
        char szUserId[kPGPMaxUserIDSize];

        lvi.mask    = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;
        lvi.pszText        = szUserId;
        lvi.cchTextMax    = sizeof(szUserId); 
                      
        ListBx_GetItem( src, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);

        if(pui->Flags&PUIF_MOVEME)
        {
            ListBx_InsertItem ( dest, &lvi );
        }
    }

#if !LISTBOX
    SortEm(dest);
#endif

//    InvalidateRect(dest,NULL,TRUE);

    for( i = iCount-1; i >= 0; i-- )            //  now delete them
    {     
        lvi.mask        = LVIF_PARAM;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;

        ListBx_GetItem( src, &lvi );
        pui=(PUSERKEYINFO)(lvi.lParam);

        if(pui->Flags&PUIF_MOVEME)
        {
            ListBx_DeleteItem(src, i);
        }
    }

#if !LISTBOX
    SortEm(src);
#endif

//    InvalidateRect(src,NULL,TRUE);

    return TRUE;
}

// FreeADKLists
// 
// Use this routine to free the ADK lists allocated during startup

void FreeADKLists(PADKINFO ListHead)
{
    PADKINFO ADK,oldADK;

    ADK=ListHead;

    while(ADK!=0)
    {
        oldADK=ADK;
        ADK=ADK->next;
        free(oldADK);
    }
}

// AddADKLink
//
// Use this routine to form ADK linked lists.

PADKINFO AddADKLink(PADKINFO head,PADKINFO ADK)
{
    ADK->next=head;

    return ADK;
}

// GetADKList
//
// For each userID, use this function to form its ADK list

PADKINFO GetADKList(PUSERKEYINFO pui,
					PGPKeyRef basekey, 
					PGPKeySetRef allkeys,
					PADKINFO HeadADK)
{
    PADKINFO ADK;
    PGPError err;
    PGPKeyRef key;
	PGPKeyID keyid;
    int nth;
	BYTE ADKclass;

    nth=0;

    do
    {
		// ADKclass always valid from AdditionalDecryptionKeyID
		err=PGPGetIndexedAdditionalRecipientRequestKeyID(basekey,
				nth,&keyid,&ADKclass);
		err=PGPGetIndexedAdditionalRecipientRequest(basekey,
				allkeys,nth,&key,NULL);

        if((IsntPGPError(err))||
		   (err==kPGPError_AdditionalRecipientRequestKeyNotFound))
        {
            ADK = (PADKINFO) malloc( sizeof(ADKINFO) );
            if(ADK!=0)
            {
                memset(ADK,0x00,sizeof(ADKINFO));
				if(err==kPGPError_AdditionalRecipientRequestKeyNotFound)
				{
					ADK->KeyRef=0;
				}
				else
					ADK->KeyRef=key;
				ADK->KeyID=keyid;
				ADK->ADKclass=ADKclass; // high bit on for warning
				ADK->OwnerPUI=pui;
                HeadADK=AddADKLink(HeadADK,ADK);
            }
        }

        nth++;

    } while((IsntPGPError(err))||
			(err==kPGPError_AdditionalRecipientRequestKeyNotFound));

    return HeadADK;
}
        
// InitAllItems
//
// Must be called before trying to move anything. Clears the moveme flag,
// and checks whether item is on left or right list

int InitAllItems(HWND left,HWND right)
{
    LV_ITEM lvi;
    int iCount, i;
    PUSERKEYINFO pui;

//  How many items are there?
    iCount = ListBx_GetItemCount( left );

//  Loop through all the source items
    for( i = 0; i < iCount; i++ )            
    {     
        lvi.mask        = LVIF_PARAM;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;
                      
        ListBx_GetItem( left, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);
     
        pui->Flags&=(~PUIF_MOVEME); // Don't move unless we match it
		pui->Flags&=(~PUIF_SELECTED);
        pui->Flags|=PUIF_LEFTLIST;
    }

    iCount = ListBx_GetItemCount( right );

//  Loop through all the source items
    for( i = 0; i < iCount; i++ )            
    {     
        lvi.mask        = LVIF_PARAM;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;
                      
        ListBx_GetItem( right, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);
    
        pui->Flags&=(~PUIF_MOVEME); // Don't move unless we match it
        pui->Flags&=(~PUIF_SELECTED);
        pui->Flags&=(~PUIF_LEFTLIST);
        
    }

    return TRUE;
}

// FindADK
//
// Find an ADK pui. But prefer to get something in the recipient (left) list
// first

PUSERKEYINFO FindADK(HWND hdlg,PUSERKEYINFO headpui,PADKINFO ADK)
{
	PUSERKEYINFO pui;

	if(ADK->KeyRef==0)
		return 0;

	pui=headpui;

    while(pui!=0)
    {
        if((pui->Flags&PUIF_LEFTLIST)&&(ADK->KeyRef==pui->KeyRef))
            return pui;
        pui=pui->next;
    }

	pui=headpui;

    while(pui!=0)
    {
        if(ADK->KeyRef==pui->KeyRef)
            return pui;
        pui=pui->next;
    }

    return 0;
}

// The Following routines are based on Mark W.'s algorithm

//When a UserID "X" gets dragged over to the left side:
void DragADKUserToRec(HWND hdlg,PUSERKEYINFO headpui,PADKINFO ADK)
{
    PUSERKEYINFO pui;

    while(ADK!=0)
    {
        if(pui=FindADK(hdlg,headpui,ADK))        // Found it!
        {
            pui->refCount++; // No drag, but should be there
			if(ADK->ADKclass&ADKCLASS_MOVEWARNING)
			{
				pui->ADKEnforceCount++;
				pui->Flags|=PUIF_ADKWARNPLEASE;
			}
            pui->Flags|=PUIF_MOVEME;
        }
        ADK=ADK->next;
    }
}

void DragUserXUserToRec(HWND hdlg,PUSERKEYINFO headpui,PUSERKEYINFO X)
{
    X->Flags|=PUIF_MANUALLYDRAGGED;  // We manually dragged this entry
    X->refCount++;              // It has priority for being there
    X->Flags|=PUIF_MOVEME;             // Move it
	X->Flags&=(~PUIF_ADKWARNPLEASE);
	X->ADKEnforceCount=0;
    DragADKUserToRec(hdlg,headpui,X->ADK); 
}

//When a UserID "X" gets dragged over to the right side:
void DragADKRecToUser(HWND hdlg,PUSERKEYINFO headpui,PADKINFO ADK)
{
    PUSERKEYINFO pui;

    while(ADK!=0)
    {
        if(pui=FindADK(hdlg,headpui,ADK))         // Found it!
        {
            pui->refCount--;  // No longer priority for being there

			pui->ADKEnforceCount--;

			if(pui->ADKEnforceCount==0)
				pui->Flags&=(~PUIF_ADKWARNPLEASE);

             // ONLY if refcount <=0
            if ((pui->refCount <= 0) && (pui->Flags&PUIF_LEFTLIST)) 
            {
                // Stick it over there.. otherwise no.
                pui->Flags&=(~PUIF_MANUALLYDRAGGED); 
				 
                pui->Flags|=PUIF_MOVEME;
            }
        }
        ADK=ADK->next;
    }
}

void DragUserXRecToUser(HWND hdlg,PUSERKEYINFO headpui,PUSERKEYINFO X)
{
    if(X->Flags&PUIF_MANUALLYDRAGGED)
    {
        DragADKRecToUser(hdlg,headpui,X->ADK);
    }

    X->refCount--;  // We had manually dragged it, reduce ref count
    X->Flags&=(~PUIF_MANUALLYDRAGGED); // No longer dragged by hand

//*
    X->Flags|=PUIF_MOVEME;
}

void DragUserX(HWND hdlg,PUSERKEYINFO headpui,PUSERKEYINFO X,BOOL RecToUser)
{
    if(RecToUser)
        DragUserXRecToUser(hdlg,headpui,X);
    else
        DragUserXUserToRec(hdlg,headpui,X);
}

// MoveListViewItems
//
// Use this after the user has selected items and you want to move them.
// Calls the functions above to acheive movement.

int MoveListViewItems(HWND left, HWND right, BOOL RecToUser)
{
    LV_ITEM lvi;
    int iCount, i;
    UINT uState;
    PUSERKEYINFO pui;
    HWND src,dest;
    HWND hdlg;
	BOOL Abort;

    hdlg=GetParent(left);

    if(RecToUser)
    {
        src=left;
        dest=right;
    }
    else
    {
        src=right;
        dest=left;
    }

    if( !ListBx_GetSelectedCount( src ) )
    {
        return FALSE; //  Exit if not
    }

    InitAllItems(left,right);

    //  How many items are there?
    iCount = ListBx_GetItemCount( src );

    for( i = 0; i < iCount; i++ )  //  Loop through each item
    {     
        uState = ListBx_GetItemState( src, i, LVIS_SELECTED );

        if( uState ) //  If this item is selected we wanna move it
        {
            lvi.mask        = LVIF_PARAM;
            lvi.iItem        = i;                            
            lvi.iSubItem    = 0;
                      
            ListBx_GetItem( src, &lvi );
            pui=(PUSERKEYINFO)(lvi.lParam);
			pui->Flags|=PUIF_SELECTED;
		}
	}

	Abort=VetoSelectedKeys(hdlg,
		(RecGbl(hdlg))->Prefs,
		(RecGbl(hdlg))->gUserLinkedList,
		RecToUser);

	if(Abort)
		return TRUE;

	MoveSelectedKeysAndADKs(hdlg,
		(RecGbl(hdlg))->gUserLinkedList,
		RecToUser);

    MoveMarkedItems(src,dest);

    return TRUE;
}

#define CHECKOUTADKS_OK 0
#define CHECKOUTADKS_BADSTRICT 1
#define CHECKOUTADKS_BADNONSTRICT 2

int CheckOutADKs(HWND hdlg,
		 PUSERKEYINFO headpui,
		 PUSERKEYINFO checkpui)
{
	PADKINFO pADK;
	BOOL ADKOK;
	int RetVal;

	RetVal=CHECKOUTADKS_OK;
	pADK=checkpui->ADK;

	while(pADK!=0)
	{
		ADKOK=FALSE;

		if(pADK->KeyRef!=0)
		{
			if(KeyIsEncryptable(headpui,pADK->KeyRef))
				ADKOK=TRUE;
		}

		pADK->ADKOK=ADKOK;

// Strict ADK not found!
		if((!ADKOK)&&(pADK->ADKclass&ADKCLASS_MOVEWARNING))
			RetVal|=CHECKOUTADKS_BADSTRICT;
// Nonstict ADK not found.....
		if((!ADKOK)&&((pADK->ADKclass&ADKCLASS_MOVEWARNING)==0))
			RetVal|=CHECKOUTADKS_BADNONSTRICT;

		pADK=pADK->next;
	}

	return RetVal;
}

// Missing ADKs in a group? Well, we need to tell the user
// which keys are missing the ADKs!

typedef struct _GroupADKInfo
{
	char Text[400];
	PUSERKEYINFO pui;
} GROUPADKINFO;


#define MINDLGX 400
#define MINDLGY 180
#define DLGMARGIN 5

int ResizeADKDlg(HWND hdlg)
{
    RECT dlgRect;
	int buttx;
	HWND hwndText,hwndList,hwndYes,hwndNo;

    GetClientRect(hdlg, &dlgRect);

    dlgRect.top+=DLGMARGIN;
    dlgRect.bottom-=DLGMARGIN;
    dlgRect.left+=DLGMARGIN;
    dlgRect.right-=DLGMARGIN;
	
	hwndText=GetDlgItem(hdlg,IDC_ADKQUERY);

	InvalidateRect(hwndText,NULL,TRUE);

	MoveWindow(hwndText,
	    dlgRect.left,dlgRect.top,
		dlgRect.right-dlgRect.left,50,TRUE);

	hwndList=GetDlgItem(hdlg,IDC_ADKLIST);

	InvalidateRect(hwndList,NULL,TRUE);

	MoveWindow(hwndList,
		dlgRect.left,60+dlgRect.top,
		dlgRect.right-dlgRect.left,dlgRect.bottom-dlgRect.top-100,TRUE);

	buttx=(dlgRect.right-dlgRect.left)/2-80+dlgRect.left;

	hwndYes=GetDlgItem(hdlg,IDYES);

	InvalidateRect(hwndYes,NULL,TRUE);

	MoveWindow(hwndYes,
	    buttx,dlgRect.bottom-25,
		75,25,TRUE);

	hwndNo=GetDlgItem(hdlg,IDNO);

	InvalidateRect(hwndNo,NULL,TRUE);

	MoveWindow(hwndNo,
	    buttx+85,dlgRect.bottom-25,
		75,25,TRUE);

	return TRUE;
}

BOOL WINAPI MissingADKDlgProc(HWND hdlg, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
		{
			GROUPADKINFO *GADKI;
			ADKINFO *ADK;
			PUSERKEYINFO LastPUI;

			GADKI=(GROUPADKINFO *)lParam;
			ADK=GADKI->pui->ADK;
			LastPUI=0;

			while(ADK!=0)
			{
				if((ADK->ADKOK==FALSE)&&(ADK->OwnerPUI!=LastPUI))
				{
					LastPUI=ADK->OwnerPUI;

					SendMessage(GetDlgItem(hdlg,IDC_ADKLIST),
						LB_ADDSTRING,0,(LPARAM)LastPUI->UserId);
				}
				ADK=ADK->next;
			}
	
			SetWindowText(GetDlgItem(hdlg,IDC_ADKQUERY),
				GADKI->Text);
			ResizeADKDlg(hdlg);

			ShowWindow(hdlg,SW_SHOW);
			SetForegroundWindow(hdlg);

			return TRUE;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = MINDLGX;
    		lpmmi->ptMinTrackSize.y = MINDLGY;
            break;
		}

        case WM_SIZE:
        {
            ResizeADKDlg(hdlg);
            break;
        }

		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
		{
			EndDialog(hdlg, FALSE);
			break;
		}

        case WM_COMMAND:
        {
            switch(wParam)
            {
				case IDYES:
				{
					EndDialog(hdlg, TRUE);
					break;
				}

				case IDNO:
				{
					EndDialog(hdlg, FALSE);
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

UINT QueryMissingADKs(HWND hwnd,PUSERKEYINFO pui,BOOL Strict)
{
	GROUPADKINFO GADKI;

	if(pui->Flags&PUIF_GROUP)
	{
		strcpy(GADKI.Text,
			"The keys listed below from the group ");
		strcat(GADKI.Text,
			pui->UserId);
		strcat(GADKI.Text,
			" are missing Additional Decryption Keys. ");
	}
	else
	{
		strcpy(GADKI.Text,
			"The key listed below is missing at least one "
			"Additional Decryption Key. ");
	}
	
	if(Strict)
	{
		strcat(GADKI.Text,
			"Some of these Additional Decryption Keys are "
			"enforced. ");
	}
	else
	{
		strcat(GADKI.Text,
			"None of these Additional Decryption Keys are "
			"enforced. ");
	}

	strcat(GADKI.Text,
		"Would you like to update the Additional Decryption Keys "
		"from the server?");
	
	GADKI.pui=pui;
	
	return DialogBoxParam(g_hinst, 
			MAKEINTRESOURCE( IDD_MISSINGADKDLG),
            hwnd, 
            (DLGPROC) MissingADKDlgProc, 
            (LPARAM) &GADKI);
}

// Ah, the power of veto. Last chance before we move any keys,
// so make your decisions here.

BOOL VetoSelectedKeys(HWND hdlg,
					PPREFS Prefs,
					PUSERKEYINFO headpui,
					BOOL RecToUser)
{
	PUSERKEYINFO pui;
	int ADKcheck;
	BOOL Abort;
	int MsgRet;

	Abort=FALSE;

	pui=headpui;

    while(pui!=0)
    {
        if(pui->Flags&PUIF_SELECTED)
        {
// Check on moving stuff OUT of recipients
// like ADKS....
			if(RecToUser)
			{
					// Don't move notfound items.
				if(pui->Flags&PUIF_NOTFOUND)
				{
					pui->Flags&=(~PUIF_SELECTED);
						
					MsgRet=MessageBox(hdlg,
		"This user name was not matched with any of the\n"
		"keys on the keyring. Would you like to search for\n"
		"this key on the server?",
						pui->UserId,
						MB_YESNO|MB_SETFOREGROUND|MB_ICONSTOP);

						if(MsgRet==IDYES)
						{
							LookUpSingleUserID(hdlg,
								(RecGbl(hdlg))->prds->Context,
								pui,
								(RecGbl(hdlg))->prds->OriginalKeySetRef);

							Abort=TRUE;
							break;
						}
				}

				if(pui->Flags&PUIF_ADKWARNPLEASE)
				{
					if(Prefs->bEnforceRemoteADK)
					{
						pui->Flags&=(~PUIF_SELECTED); 
						// We enforce incoming ADKS!
					}
					else
					{
						// Personal edition or we don't enforce
						pui->ADKEnforceCount--;

						if(pui->ADKEnforceCount==0)
							pui->Flags&=(~PUIF_ADKWARNPLEASE);

						ADKWarnPlease(hdlg,pui->UserId);
					}
				}

				if(Prefs->bUseOutgoingADK)
				{
					if(pui->KeyRef==(Prefs->RSAADKKey))
					{
						if(Prefs->bEnforceOutgoingADK)
							pui->Flags&=(~PUIF_SELECTED); 
					// We enforce outgoing ADKS!
						else
							ADKWarnPlease(hdlg,pui->UserId);
					}
				}
			}
// Check on moving stuff into recipients, like ADKless keys and keys
// not signed by corp key
			if(!RecToUser)
			{
				if(hdlg!=NULL)
				{
				// Annoy them if it must be signed by corp
					if(Prefs->bWarnNotCorpSigned)
					{
						if((pui->Flags&PUIF_SIGNEDBYCORP)==0)
							NonCorpSignedWarnPlease(hdlg,pui->UserId);
					}
				}

				ADKcheck=CheckOutADKs(hdlg,headpui,pui);

				if(ADKcheck&CHECKOUTADKS_BADSTRICT)
				{
					if(Prefs->bEnforceRemoteADK)
					{
// Only Business versions enforce the strict bit, but they can decide
						pui->Flags&=(~PUIF_SELECTED);
					}

					if(hdlg!=0)
					{
						MsgRet=QueryMissingADKs(hdlg,
							pui,TRUE);

						if(MsgRet)
						{
							GetADKsFromServer(
								(RecGbl(hdlg))->prds->Context,
								hdlg,
								pui,
								(RecGbl(hdlg))->prds->OriginalKeySetRef);
							Abort=TRUE;
							break;
						}
					}
					else // Auto-don't select, and pop dialog
					{
						Abort=TRUE;
					}
				}
				else if (ADKcheck&CHECKOUTADKS_BADNONSTRICT)
				{
					if(hdlg!=0)
					{
						MsgRet=QueryMissingADKs(hdlg,
							pui,FALSE);

						if(MsgRet)
						{
							GetADKsFromServer(
								(RecGbl(hdlg))->prds->Context,
								hdlg,
								pui,
								(RecGbl(hdlg))->prds->OriginalKeySetRef);
							Abort=TRUE;
							break;
						}
					}
					else // Auto-don't select, and pop dialog
					{
						pui->Flags&=(~PUIF_SELECTED);
						Abort=TRUE;
					}
				}
			}
		}
		pui=pui->next;
	}
    return Abort;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
