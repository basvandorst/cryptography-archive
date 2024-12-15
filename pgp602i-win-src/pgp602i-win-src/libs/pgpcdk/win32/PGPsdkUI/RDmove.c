/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDmove.c,v 1.7.10.1 1998/11/12 03:24:31 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

// MoveMarkedItems
//
// Use this routine after you have marked all the items you wish to
// move with the PUIF_MOVEME flag

int MoveMarkedItems(HWND src, HWND dest,
					PGPRecipientUserLocation destinationList)
{
    LV_ITEM lvi;
    int iCount, i;
    PUSERKEYINFO pui;

    iCount = ListBx_GetItemCount( src );

    if(iCount==0)
        return FALSE;

	//  Loop through each item and insert
    for( i = 0; i < iCount; i++ )   
    {     
        char szUserId[kPGPMaxUserIDSize];

        lvi.mask    = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;
        lvi.pszText        = szUserId;
        lvi.cchTextMax    = sizeof(szUserId); 
                      
        ListBx_GetItem( src, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);

        if(pui->pru->location==destinationList)
        {
            ListBx_InsertItem ( dest, &lvi );
        }
    }

#if !LISTBOX
    SortEm(dest);
#endif

	 //  now delete them
    for( i = iCount-1; i >= 0; i-- )           
    {     
        lvi.mask        = LVIF_PARAM;
        lvi.iItem        = i;                            
        lvi.iSubItem    = 0;

        ListBx_GetItem( src, &lvi );
        pui=(PUSERKEYINFO)(lvi.lParam);

        if((pui->pru->location==destinationList)||
		   (pui->pru->location==kPGPRecipientUserLocation_Hidden))
        {
            ListBx_DeleteItem(src, i);
        }
    }

#if !LISTBOX
    SortEm(src);
#endif

    return TRUE;
}

// MoveListViewItems
//
// Use this after the user has selected items and you want to move them.
// Calls the functions above to acheive movement.

int MoveListViewItems(HWND left, HWND right, BOOL RecToUser)
{
    LV_ITEM lvi;
    int userCount,iCount, i;
    UINT uState;
    PUSERKEYINFO pui;
    HWND src,dest;
    HWND hdlg;
	PGPRecipientUser **userList;
	int userIndex;
	int memamt;
	PRECGBL prg;
	PGPRecipientUserLocation destinationList;
	PGPUInt32 numMovedUsers;
	PGPError err;
	PGPBoolean movedARRs;

    hdlg=GetParent(left);

	prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

    if(RecToUser)
    {
        src=left;
        dest=right;
		destinationList=kPGPRecipientUserLocation_UserList;
    }
    else
    {
        src=right;
        dest=left;
		destinationList=kPGPRecipientUserLocation_RecipientList;
    }

    userCount=ListBx_GetSelectedCount( src );
	if(userCount==0)
        return FALSE; //  Exit if not

	memamt=sizeof(PGPRecipientUser *)*userCount;

	userList=(PGPRecipientUser **)malloc(memamt);
	memset(userList,0x00,memamt);

	userIndex=0;

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
			userList[userIndex]=pui->pru;
			userIndex++;
		}
	}

	err=PGPMoveRecipients(&(prg->mRecipients),
		destinationList,
		userCount, userList,
		&numMovedUsers, &movedARRs);

	free(userList);

    MoveMarkedItems(src,dest,destinationList);

    return TRUE;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
