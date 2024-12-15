/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDlstsub.c,v 1.14 1997/10/16 15:08:50 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * ListSub.c  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_LBUTTONDBLCLK:
        {
            HWND hwndUserIds = 
                GetDlgItem(GetParent(hwnd), IDC_USER_ID_LIST);
            HWND hwndRecpts = 
                GetDlgItem(GetParent(hwnd), IDC_RECIPIENT_LIST);
                                                                        
            MoveListViewItems(hwndRecpts, hwndUserIds,
                              hwnd == hwndRecpts); 
            break;
        }

        case WM_CONTEXTMENU: 
        {
			POINT pnt;
			HMENU hMenu=NULL;
			HMENU hSubMenu=NULL;

			pnt.x=LOWORD(lParam);
			pnt.y=HIWORD(lParam);
	
			hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_MENU1));
			hSubMenu = GetSubMenu(hMenu,0);

			TrackPopupMenu(hSubMenu,TPM_RIGHTBUTTON,pnt.x,pnt.y,0,hwnd,NULL);
			DestroyMenu(hMenu);
			return 0;
        }

        case WM_COMMAND:
        {
            switch(wParam)
            {
				case IDM_WHATSTHIS:
				{
					char szHelpFile[500];
					PRECGBL prg;

					prg=RecGbl(GetParent(hwnd));
					GetHelpFile(szHelpFile);

					if(hwnd == prg->hwndRecipients)
						WinHelp(hwnd, szHelpFile,
						HELP_CONTEXTPOPUP, IDH_IDC_RECIPIENT_LIST);
					if(hwnd == prg->hwndUserIDs)
						WinHelp(hwnd, szHelpFile,
						HELP_CONTEXTPOPUP, IDH_IDC_USER_ID_LIST);
					return 0;
				}

				case IDM_LOOKUP:
				{
					PRECGBL prg;

					prg=RecGbl(GetParent(hwnd));

					LookUpSelectedKeys(hwnd,
						prg->prds->Context,
						prg->prds->OriginalKeySetRef);
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN:
        {
            int nVirtKey = (int) wParam;    // virtual-key code 
            HWND hwndUserIds = 
                GetDlgItem(GetParent(hwnd), IDC_USER_ID_LIST);
            HWND hwndRecpts = 
                GetDlgItem(GetParent(hwnd), IDC_RECIPIENT_LIST);

            if( ((nVirtKey == VK_RIGHT)||(nVirtKey == VK_LEFT)||
                (nVirtKey == VK_DELETE)) && (hwnd == hwndRecpts))
            {
                MoveListViewItems(hwndRecpts, hwndUserIds,TRUE);
                break;
            }
            else if( ((nVirtKey == VK_RIGHT)||(nVirtKey == VK_LEFT)||
                (nVirtKey == VK_DELETE)) && (hwnd == hwndUserIds))
            {
                MoveListViewItems(hwndRecpts, hwndUserIds,FALSE);
                break;
            }
            break;
        }

    }

    //  Pass all non-custom messages to old window proc
    return CallWindowProc((FARPROC)origListBoxProc, hwnd, 
                           msg, wParam, lParam ) ;
}





void MakeReloadArray(HWND hwnd,PRECIPIENTDIALOGSTRUCT prds)
{
    LV_ITEM lvi;
    int iCount, i;
    PUSERKEYINFO pui = NULL;
    char* cp = NULL;
	int memamt;

	prds->szRecipientArray=0;
	prds->dwNumRecipients=0;

    //  How many items are there?
    iCount = ListBx_GetItemCount( hwnd );

	if(iCount==0)
    {
        return; //  Exit if not
    }

	memamt=(iCount+1) * sizeof(char *);
	prds->szRecipientArray = (char **)malloc (memamt);
	memset(prds->szRecipientArray,0x00,memamt);

	if(iCount)
    {
	   for( i = 0; i < iCount; i++ )  //  Loop through each item
       {         
			lvi.mask = LVIF_PARAM;
            lvi.iItem        = i;                            
            lvi.iSubItem    = 0;
                
            if(ListBx_GetItem( hwnd, &lvi ))
            {
				pui = (PUSERKEYINFO)lvi.lParam;

				if(((pui->Flags&PUIF_SYSTEMKEY)==0)&&
					(pui->Flags&PUIF_MANUALLYDRAGGED))
				{
					prds->szRecipientArray[prds->dwNumRecipients]=
						(char *)malloc(strlen(pui->UserId)+1);

					strcpy(prds->szRecipientArray[prds->dwNumRecipients],
						pui->UserId); // inefficient but easy
                    prds->dwNumRecipients++;	
                }
            }
        }
    }
}

void FreeReloadArray(PRECIPIENTDIALOGSTRUCT prds)
{
	unsigned int i;

	for(i=0;i<prds->dwNumRecipients;i++)
	{
		free(prds->szRecipientArray[i]);
	}
	free(prds->szRecipientArray);
}



/*____________________________________________________________________________
	All all the keys in the group (and its subgroups) to the keyset
____________________________________________________________________________*/
	PGPError
RDNewKeySetFromGroup(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPKeySetRef	masterSet,		/* resolve key IDs using the set */
	PGPKeySetRef *	outKeys,
	PUSERKEYINFO	HeadPUI,
	PGPUInt32 *		numKeysNotFound
	)
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	PGPKeySetRef	workingSet	= kPGPInvalidRef;
	
	err	= PGPNewEmptyKeySet( masterSet, &workingSet );
	if ( IsntPGPError( err ) )
	{
		err	= PGPNewGroupItemIter( set, id,
				kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
				&iter );
	}
	if ( IsntPGPError( err )  )
	{
		PGPGroupItem	item;
		
		while( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyRef	key;
			
			err	= RDGroupItemToKey( &item, masterSet, &key );
			if ( IsntPGPError( err ) )
			{
				if(KeyIsEncryptable(HeadPUI,key))
				{
					PGPKeySetRef	singleKeySet;
				
					err	= PGPNewSingletonKeySet( key, &singleKeySet );
//					PGPFreeKey( key );
					key	= kPGPInvalidRef;
					if ( IsntPGPError( err ) )
					{
						PGPKeySetRef	oldKeys	= workingSet;
					
						err = PGPUnionKeySets( workingSet,
							singleKeySet, &workingSet );
							
						(void) PGPFreeKeySet( singleKeySet );
						(void) PGPFreeKeySet( oldKeys );
					}
				
				
					if ( IsPGPError( err ) )
						break;
				}
			}
			else if ( err == kPGPError_ItemNotFound )
			{
				*numKeysNotFound	+= 1;
			}
			else
			{
				break;
			}
		}
		if ( err == kPGPError_EndOfIteration )
			err	= kPGPError_NoErr;
			
		PGPFreeGroupItemIter( iter );
	}
	
	if ( IsPGPError( err ) && PGPRefIsValid( workingSet ) )
	{
		(void) PGPFreeKeySet( workingSet );
		workingSet	= kPGPInvalidRef;
	}
	
	*outKeys	= workingSet;
	
	return( err );
}


int RecipientSetFromList(PRECIPIENTDIALOGSTRUCT prds,
						PUSERKEYINFO RecipientLL,
						PGPUInt32 *pNumKeys)
{
    PUSERKEYINFO pui;
    int iCount;
    int RSA,DSA;
	PGPError err;
	PGPKeySetRef oldKeys,additionalKeys;

    RSA=DSA=FALSE;

    iCount=LengthOfList(RecipientLL);
    
	PGPNewEmptyKeySet(prds->OriginalKeySetRef,&(prds->SelectedKeySetRef));

    if(iCount)
    {
        pui=RecipientLL;

        while(pui!=0)
        {
            if((pui->Flags&PUIF_MOVEME)&&((pui->Flags&PUIF_NOTFOUND)==0))
            {
				if(pui->Flags&PUIF_HASRSA)
					RSA=TRUE;

				if(pui->Flags&PUIF_HASDSA)
					DSA=TRUE;

   				if (pui->Flags&PUIF_GROUP)
				{
					PGPUInt32	numMissing;
					
					err	= RDNewKeySetFromGroup(pui->GroupSet,
							pui->GroupID,prds->OriginalKeySetRef,
							&additionalKeys, RecipientLL,&numMissing );
					// what about missing keys
				}
				else
				{
					err = PGPNewSingletonKeySet(pui->KeyRef,
								&additionalKeys );
				}
			
				if ( IsPGPError( err ) )
					break;
					
				oldKeys = prds->SelectedKeySetRef;
				err = PGPUnionKeySets( prds->SelectedKeySetRef, 
						additionalKeys, &prds->SelectedKeySetRef);
				(void) PGPFreeKeySet( additionalKeys );
				(void) PGPFreeKeySet( oldKeys );
            }
            pui=pui->next;
        }
    }

	PGPCountKeys(prds->SelectedKeySetRef, pNumKeys);

	if((*pNumKeys==0)||(IsPGPError(err)))
	{
		PGPFreeKeySet( prds->SelectedKeySetRef );
		prds->SelectedKeySetRef=0;
	}

	if(RSA)
		prds->dwFlags|=PGPCOMDLG_RSAENCRYPT;
	else
		prds->dwFlags&=~PGPCOMDLG_RSAENCRYPT;

    if(RSA && DSA)
        return TRUE;

    return FALSE;
}

int RecipientSetFromGUI(HWND hwnd,
				   PRECIPIENTDIALOGSTRUCT prds,
				   PUSERKEYINFO RecipientLL,
				   PGPUInt32 *pNumKeys)
{
    LV_ITEM lvi;
    int iCount, i;
	PUSERKEYINFO pui;

	pui=RecipientLL;

	while(pui!=0)
	{
		pui->Flags&=(~PUIF_MOVEME);
		pui=pui->next;
	}

    //  How many items are there?
    iCount = ListBx_GetItemCount( hwnd );

    if(iCount)
    {
            for( i = 0; i < iCount; i++ )  //  Loop through each item
            {         
                lvi.mask = LVIF_PARAM;
                lvi.iItem        = i;                            
                lvi.iSubItem    = 0;
                
                if(ListBx_GetItem( hwnd, &lvi ))
                {
                    pui = (PUSERKEYINFO)lvi.lParam;
					pui->Flags|=PUIF_MOVEME;
                }
            }
    }

    return RecipientSetFromList(prds,RecipientLL,pNumKeys);
}

int LengthOfList(PUSERKEYINFO ListHead)
{
    PUSERKEYINFO pui;
    int length;

    length=0;
    pui=ListHead;

    while(pui!=0)
    {
        pui=pui->next;
        length++;
    }
    return length;
}


void FreeLinkedLists(PUSERKEYINFO ListHead)
{
    PUSERKEYINFO pui,oldpui;

    pui=ListHead;

    while(pui!=0)
    {
        oldpui=pui;
        pui=pui->next;

        FreeADKLists(oldpui->ADK); // Remove the ADK links
        free(oldpui);
    }
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
