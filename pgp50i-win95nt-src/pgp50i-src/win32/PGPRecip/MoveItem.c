/*
 * MoveItem.c  List View movement routines that support MRK
 *
 * These routines use the algorithm developed by Mark W. to support MRK
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

// MRKWarnPlease
//
// Display this warning if the user removes an MRK'ed key from
// the recipient list.

void MRKWarnPlease(void)
{
	MessageBox(NULL,
"Removing the message recovery agent key from the recipient list may\n"
"violate the policy established for other keys in the recipient list.",
		"PGP 5.0 Warning",MB_OK|MB_ICONEXCLAMATION);
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

    iCount = ListView_GetItemCount( src );

    if(iCount==0)
         return FALSE;

    for( i = 0; i < iCount; i++ )          //  Loop through each item
    {
        char szUserId[USERIDLENGTH];

        lvi.mask    = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
        lvi.iItem         = i;
        lvi.iSubItem    = 0;
        lvi.pszText         = szUserId;
        lvi.cchTextMax    = sizeof(szUserId);

        ListView_GetItem( src, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);

        if(pui->Flags&PUIF_MOVEME)
        {
             ListView_InsertItem ( dest, &lvi );
        }
    }

    SortEm(dest);

//    InvalidateRect(dest,NULL,TRUE);

    for( i = iCount-1; i >= 0; i-- )           //  now delete them
    {
         lvi.mask       = LVIF_PARAM;
         lvi.iItem       = i;
         lvi.iSubItem   = 0;

         ListView_GetItem( src, &lvi );
         pui=(PUSERKEYINFO)(lvi.lParam);

         if(pui->Flags&PUIF_MOVEME)
         {
             ListView_DeleteItem(src, i);
         }
    }

    SortEm(src);

//    InvalidateRect(src,NULL,TRUE);

    return TRUE;
}

// FreeMRKLists
//
// Use this routine to free the MRK lists allocated during startup

void FreeMRKLists(PMRKINFO ListHead)
{
    PMRKINFO mrk,oldmrk;

    mrk=ListHead;

    while(mrk!=0)
    {
         oldmrk=mrk;
         mrk=mrk->next;
         free(oldmrk);
    }
}

// AddMRKLink
//
// Use this routine to form MRK linked lists.

PMRKINFO AddMRKLink(PMRKINFO head,PMRKINFO mrk)
{
    mrk->next=head;

    return mrk;
}

// FillInPMRK
//
// Use this routine to fill out a newly allocated MRK structure

int FillInPMRK(PGPKey *key,PMRKINFO mrk)
{
    size_t len;
    char szKeyId[KEYIDLENGTH];

    len = KEYIDLENGTH;
    pgpGetKeyString(key, kPGPKeyPropKeyId, szKeyId, &len);
    KMConvertStringKeyID (szKeyId);
    wsprintf(mrk->KeyId, "0x%s",szKeyId);

    return TRUE;
}

// GetMRKList
//
// For each userID, use this function to form its MRK list

PMRKINFO GetMRKList(PGPKey *basekey, PGPKeySet *allkeys)
{
    PMRKINFO ret_mrk,mrk;
    PGPError ret_val;
    PGPKey *key;
    int nth;
	byte mrkclass;

    ret_mrk=0;
    nth=0;

    do
    {
        ret_val=pgpGetMessageRecoveryKey(basekey,allkeys,nth,&key,&mrkclass,
										NULL);

        if(key!=0)
        {
            mrk = (PMRKINFO) malloc( sizeof(MRKINFO) );
            if(mrk!=0)
            {
                memset(mrk,0x00,sizeof(MRKINFO));
                FillInPMRK(key,mrk);
				mrk->mrkclass=mrkclass; // high bit on for warning
                ret_mrk=AddMRKLink(ret_mrk,mrk);
            }
        }

        nth++;

    } while(key!=0);

    return ret_mrk;
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
    iCount = ListView_GetItemCount( left );

//  Loop through all the source items
    for( i = 0; i < iCount; i++ )
    {
        lvi.mask        = LVIF_PARAM;
        lvi.iItem         = i;
        lvi.iSubItem    = 0;

        ListView_GetItem( left, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);

        pui->Flags&=(~PUIF_MOVEME); // Don't move unless we match it
        pui->Flags|=PUIF_LEFTLIST;
    }

    iCount = ListView_GetItemCount( right );

//  Loop through all the source items
    for( i = 0; i < iCount; i++ )
    {
        lvi.mask        = LVIF_PARAM;
        lvi.iItem        = i;
        lvi.iSubItem    = 0;

        ListView_GetItem( right, &lvi );

        pui=(PUSERKEYINFO)(lvi.lParam);

        pui->Flags&=(~PUIF_MOVEME); // Don't move unless we match it

        pui->Flags&=(~PUIF_LEFTLIST);

    }

    return TRUE;
}

// FindMRK
//
// Find an MRK pui. But prefer to get something in the recipient (left) list
// first

PUSERKEYINFO FindMRK(HWND hdlg,PMRKINFO MRK)
{
    PUSERKEYINFO pui;

    pui=(RecGbl(hdlg))->gUserLinkedList;

    while(pui!=0)
    {
        if((pui->Flags&PUIF_LEFTLIST)&&(!strcmp(MRK->KeyId,pui->KeyId)))
             return pui;
        pui=pui->next;
    }

    pui=(RecGbl(hdlg))->gUserLinkedList;

    while(pui!=0)
    {
        if(!strcmp(MRK->KeyId,pui->KeyId))
            return pui;
        pui=pui->next;
    }

    return 0;
}

// The Following routines are based on Mark W.'s algorithm

//When a UserID "X" gets dragged over to the left side:
void DragMRKRightToLeft(HWND hdlg,PMRKINFO MRK)
{
    PUSERKEYINFO pui;

    while(MRK!=0)
    {
        if(pui=FindMRK(hdlg,MRK))        // Found it!
        {
            pui->refCount++; // No drag, but should be there
			if(MRK->mrkclass&MRKCLASS_MOVEWARNING)
				pui->Flags|=PUIF_MRKWARNPLEASE;
            pui->Flags|=PUIF_MOVEME;
        }
        MRK=MRK->next;
    }
}

void DragUserXRightToLeft(HWND hdlg,PUSERKEYINFO X)
{
    X->Flags|=PUIF_MANUALLYDRAGGED;  // We manually dragged this entry
    X->refCount++;              // It has priority for being there
    X->Flags|=PUIF_MOVEME;             // Move it
	X->Flags&=(~PUIF_MRKWARNPLEASE);
    DragMRKRightToLeft(hdlg,X->mrk);
}

//When a UserID "X" gets dragged over to the right side:
void DragMRKLeftToRight(HWND hdlg,PMRKINFO MRK)
{
     PUSERKEYINFO pui;

     while(MRK!=0)
     {
         if(pui=FindMRK(hdlg,MRK))        // Found it!
         {
            pui->refCount--;  // No longer priority for being there
            pui->Flags&=(~PUIF_MRKWARNPLEASE);

             // ONLY if refcount <=0
            if ((pui->refCount <= 0) && (pui->Flags&PUIF_LEFTLIST))
            {
                 // Stick it over there.. otherwise no.
                 pui->Flags&=(~PUIF_MANUALLYDRAGGED);
				
                 pui->Flags|=PUIF_MOVEME;
            }
        }
        MRK=MRK->next;
    }
}

void DragUserXLeftToRight(HWND hdlg,PUSERKEYINFO X)
{
    if(X->Flags&PUIF_MANUALLYDRAGGED)
    {
        DragMRKLeftToRight(hdlg,X->mrk);
    }

    X->refCount--;  // We had manually dragged it, reduce ref count
    X->Flags&=(~PUIF_MANUALLYDRAGGED); // No longer dragged by hand
	if(X->Flags&PUIF_MRKWARNPLEASE)
	{
		X->Flags&=(~PUIF_MRKWARNPLEASE);
		MRKWarnPlease();
	}
    X->Flags|=PUIF_MOVEME;
}

void DragUserX(HWND hdlg,PUSERKEYINFO X,BOOL LeftToRight)
{
    if(LeftToRight)
        DragUserXLeftToRight(hdlg,X);
    else
        DragUserXRightToLeft(hdlg,X);
}

// MoveListViewItems
//
// Use this after the user has selected items and you want to move them.
// Calls the functions above to acheive movement.

int MoveListViewItems(HWND left, HWND right, BOOL LeftToRight)
{
    LV_ITEM lvi;
    int iCount, i;
    UINT uState;
    PUSERKEYINFO pui;
    HWND src,dest;
    HWND hdlg;

    hdlg=GetParent(left);

    if(LeftToRight)
    {
        src=left;
        dest=right;
    }
    else
    {
        src=right;
        dest=left;
    }

    if( !ListView_GetSelectedCount( src ) )
    {
        return FALSE; //  Exit if not
    }

    InitAllItems(left,right);

    //  How many items are there?
    iCount = ListView_GetItemCount( src );

    for( i = 0; i < iCount; i++ )  //  Loop through each item
    {
        uState = ListView_GetItemState( src, i, LVIS_SELECTED );

        if( uState ) //  If this item is selected we wanna move it
        {
            lvi.mask        = LVIF_PARAM;
            lvi.iItem        = i;
            lvi.iSubItem    = 0;

            ListView_GetItem( src, &lvi );
            pui=(PUSERKEYINFO)(lvi.lParam);

            DragUserX(hdlg,pui,LeftToRight);
        }
    }

    MoveMarkedItems(src,dest);

    return TRUE;
}

int InitialMoveListViewItems(HWND left, HWND right,PUSERKEYINFO pui)
{
    HWND src,dest;
    HWND hdlg;

    hdlg=GetParent(left);

    src=right;
    dest=left;

    InitAllItems(left,right);

    while(pui!=0)
    {
        if(pui->Flags&PUIF_MATCHEDWITHRECIPIENT)
        {
            DragUserX(hdlg,pui,FALSE);
        }
        pui=pui->next;
    }

    MoveMarkedItems(src,dest);

    return TRUE;
}
