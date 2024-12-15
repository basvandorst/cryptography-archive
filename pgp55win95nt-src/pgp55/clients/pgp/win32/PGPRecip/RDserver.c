/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDserver.c,v 1.15 1997/10/16 15:08:53 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * KeyServe.c  Routines needed to support the keyserver context menu
 *
 * All of these routines are used within the listview subclass handler, but
 * since they deal with the keyserver, they are isolated in this file for
 * clarity.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


//	_______________________________________________
//
//  Update selected keys from server


PUSERKEYINFO *MakeServerPUIList(HWND hdlg)
{
	PUSERKEYINFO ListHead;
	PUSERKEYINFO *puilist;
	int listlen,memamt;

    ListHead=(RecGbl(hdlg))->gUserLinkedList;
	listlen=LengthOfList(ListHead);

	memamt=(listlen+1) * sizeof(PUSERKEYINFO);
	puilist = (PUSERKEYINFO *) malloc (memamt);
	memset(puilist,0x00,memamt);

	return puilist;
}

PGPError GetGroupFromServer (PGPContextRef context,
							 HWND hwnd,
							 PUSERKEYINFO pui,
							 PGPKeySetRef *keysetFound)
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	PGPKeyID *pkeyidList;
	int arraycount;
	int listsize;

	arraycount=0;

	listsize=(pui->NumGroupMembers+1)*sizeof(PGPKeyID);

	pkeyidList = (PGPKeyID*)malloc(listsize);
	memset(pkeyidList,0x00,listsize);

	err	= PGPNewGroupItemIter(pui->GroupSet,pui->GroupID,
			kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
			&iter );

	if ( IsntPGPError( err )  )
	{
		PGPGroupItem	item;
		
		while( IsntPGPError( err = PGPGroupItemIterNext(iter,&item)))
		{
			err	= PGPImportKeyID(item.u.key.exportedKeyID,
				&(pkeyidList[arraycount]));

			arraycount++;

			if ( IsPGPError( err ) )
				break;
		}
		if ( err == kPGPError_EndOfIteration )
			err	= kPGPError_NoErr;

		PGPFreeGroupItemIter( iter );
	}

	if(IsntPGPError(err))
	{
		err = PGPcomdlgSearchServerForKeyIDs (context, 
			hwnd,pkeyidList,arraycount,keysetFound);
	}

	free(pkeyidList);

	return( err );
}


void GetSelectedItems(HWND hwnd,PGPKeySetRef kset,
					  PUSERKEYINFO *puilist)
{
    LV_ITEM lvi;
    int iCount, i;
    UINT uState;
    PUSERKEYINFO pui = NULL;
	int selCount,selIndex;

    selCount=ListView_GetSelectedCount( hwnd );

	if(selCount==0)
    {
        return; //  Exit if not
    }

	selIndex=0;

    //  How many items are there?
    iCount = ListView_GetItemCount( hwnd );

    for( i = 0; i < iCount; i++ )            //  Loop through each item
    {     
        uState = ListView_GetItemState( hwnd, i, LVIS_SELECTED );

        if( uState ) //  If this item is selected we wanna move it
        {
            lvi.mask = LVIF_PARAM;
            lvi.iItem        = i;                            
            lvi.iSubItem    = 0;
            
            if(ListView_GetItem( hwnd, &lvi ))
            {
                pui = (PUSERKEYINFO)lvi.lParam;
				puilist[selIndex]=pui;
				selIndex++;
			}
        }
    }
	puilist[selIndex]=0;
}

void GetNotFoundItems(HWND hdlg,PUSERKEYINFO *puilist)
{
	PUSERKEYINFO pui;
	int nfIndex;

    pui=(RecGbl(hdlg))->gUserLinkedList;

	nfIndex=0;

    while(pui!=0)
    {   // Don't try to find multiple match keys. We already have too many!
        if(pui->Flags&PUIF_NOTFOUND)
        {
			puilist[nfIndex]=pui;
			nfIndex++;
		}
		pui=pui->next;
    }
}

BOOL GoQueryAddKeys(HWND hwnd,PGPContextRef context,
					PGPKeySetRef keysetFound,PGPKeySetRef keysetToAddTo)
{
	BOOL RetVal;
	unsigned int uCount;

	RetVal=FALSE;

	if (PGPRefIsValid (keysetFound)) 
	{
		PGPCountKeys (keysetFound, &uCount);
		if (uCount == 0) 
		{
			MessageBox(hwnd,"No keys found",
				"Keyserver Warning",
				MB_OK|MB_SETFOREGROUND|MB_ICONEXCLAMATION);
		}
		else
		{
			if(PGPkmQueryAddKeys (context,hwnd,keysetFound,
				NULL)==kPGPError_NoErr)
				RetVal=TRUE;
		}
		PGPFreeKeySet (keysetFound);
	}

	return RetVal;
}

BOOL GetADKsFromServer(PGPContextRef context,
							 HWND hwnd,
							 PUSERKEYINFO pui,
							 PGPKeySetRef keysetToAddTo)
{
	PADKINFO pADK;
	PGPError err;
	int ADKlistlen;
	int arraycount;
	int listsize;
	PGPKeyID *pkeyidList;
	BOOL RetVal;
	PGPKeySetRef keysetFound;

	OleInitialize (NULL);

	RetVal=FALSE;

	pADK=pui->ADK;
	ADKlistlen=0;

	while(pADK!=0)
	{
		ADKlistlen++;
		pADK=pADK->next;
	}

	listsize=(ADKlistlen+1)*sizeof(PGPKeyID);

	pkeyidList = (PGPKeyID*)malloc(listsize);
	memset(pkeyidList,0x00,listsize);

	pADK=pui->ADK;
	arraycount=0;

	while(pADK!=0)
	{
		if(!(pADK->ADKOK))
		{
			pkeyidList[arraycount]=pADK->KeyID;
			arraycount++;
		}
		pADK=pADK->next;
	}

	err = PGPcomdlgSearchServerForKeyIDs (context, 
			hwnd,pkeyidList,arraycount,&keysetFound);

	free(pkeyidList);

	if ((!PGPcomdlgErrorMessage (err))&&(err!=kPGPError_UserAbort))
	{
		RetVal=GoQueryAddKeys(hwnd,context,
					keysetFound,keysetToAddTo);
	}

	OleUninitialize();

	return RetVal;
}

int LoopThroughKeys(HWND hWndParent,
			   PGPContextRef context,
			   PUSERKEYINFO *puilist,
			   PGPKeySetRef keysetToAddTo)
{    
    PGPKeySetRef keysetFound,accumkeyset;
	PGPError err;
	int listIndex;
	int RetVal;

	OleInitialize (NULL);

	if(*puilist==0)
		return FALSE;

	RetVal=FALSE;
	listIndex=0;

	PGPNewKeySet( context, &accumkeyset);

	while(puilist[listIndex]!=0)
	{
		if(puilist[listIndex]->Flags&PUIF_GROUP)
		{
			err=GetGroupFromServer (context,hWndParent,
					puilist[listIndex],&keysetFound);
		}
		else
		{
			err = PGPcomdlgSearchServerForUserID (context, hWndParent, 
					puilist[listIndex]->UserId, &keysetFound);
		}

		if(IsPGPError(err))
			break;

		err = PGPAddKeys(keysetFound,accumkeyset);
		PGPCommitKeyRingChanges (accumkeyset);
		(void) PGPFreeKeySet( keysetFound );

		listIndex++;
	}


	if ((!PGPcomdlgErrorMessage (err))&&(err!=kPGPError_UserAbort))
	{
		RetVal=GoQueryAddKeys(hWndParent,context,
			accumkeyset,keysetToAddTo);
    }
	else
	{
		PGPFreeKeySet(accumkeyset);
	}

	OleUninitialize();

    return RetVal;
}

int	LookUpSelectedKeys(HWND hWndParent,
			   PGPContextRef context,
			   PGPKeySetRef keysetToAddTo)
{    
    PUSERKEYINFO *puilist;
	int RetVal=FALSE;

	puilist=MakeServerPUIList(GetParent(hWndParent));

    GetSelectedItems(hWndParent,keysetToAddTo,
		puilist);

	if(*puilist!=0)
	{
		RetVal=LoopThroughKeys(hWndParent,
			context,
			puilist,
			keysetToAddTo);
	}

	free(puilist);

	return RetVal;
}


int	LookUpNotFoundKeys(HWND hWndParent,
			   PGPContextRef context,
			   PGPKeySetRef keysetToAddTo)
{    
    PUSERKEYINFO *puilist;
	int RetVal=FALSE;

	puilist=MakeServerPUIList(hWndParent);

    GetNotFoundItems(hWndParent,
		puilist);

	if(*puilist!=0)
	{
		RetVal=LoopThroughKeys(hWndParent,
			context,
			puilist,
			keysetToAddTo);
	}

	free(puilist);

	return RetVal;
}

int	LookUpSingleUserID(HWND hWndParent,
			   PGPContextRef context,
			   PUSERKEYINFO pui,
			   PGPKeySetRef keysetToAddTo)
{    
    PUSERKEYINFO *puilist;
	int RetVal=FALSE;

	puilist=MakeServerPUIList(hWndParent);

	puilist[0]=pui;

	if(*puilist!=0)
	{
		RetVal=LoopThroughKeys(hWndParent,
			context,
			puilist,
			keysetToAddTo);
	}

	free(puilist);

	return RetVal;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
