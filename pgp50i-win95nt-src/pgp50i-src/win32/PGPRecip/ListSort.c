/*
 * ListSort.c  List View sorting routine
 *
 * Apparently the sorting algorithm used in PGPlib is somehow different than
 * Microsoft employs in their standard listview control. This makes things
 * sometimes confusing, since PGPkeys follows the PGPlib convention. Albeit
 * bucking PC GUI standards SortEm changes the listview sort to comply with
 * PGPlib and PGPkeys.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

int CALLBACK ListViewCompareProc(LPARAM lParam1,
                                 LPARAM lParam2,LPARAM lParamSort)
{
    PUSERKEYINFO pui1,pui2;
    PRECGBL pRecGbl;
    int result;
    int *SortSub;
    BOOL *SortAscending;
    HWND hwnd;

    hwnd=(HWND)lParamSort;
    pRecGbl=RecGbl(GetParent(hwnd));

    if(hwnd==pRecGbl->hwndRecipients)
    {
         SortSub=&(pRecGbl->RSortSub);
         SortAscending=&(pRecGbl->RSortAscending);
    }
    else
    {
        SortSub=&(pRecGbl->USortSub);
        SortAscending=&(pRecGbl->USortAscending);
    }

    pui1=(PUSERKEYINFO)lParam1;
    pui2=(PUSERKEYINFO)lParam2;

    switch(*SortSub)
    {
        case HEADUSERID:
        {
            if(*SortAscending)
                result=pgpUserIDStrCompare(pui1->UserId,pui2->UserId);
            else
                result=pgpUserIDStrCompare(pui2->UserId,pui1->UserId);
            break;
        }
        case HEADVALIDITY:
        {
            if(*SortAscending)
                result=(pui2->Validity)-(pui1->Validity);
            else
                result=(pui1->Validity)-(pui2->Validity);
            break;
        }
        case HEADTRUST:
        {
            if(*SortAscending)
                result=(pui2->Trust)-(pui1->Trust);
            else
                result=(pui1->Trust)-(pui2->Trust);
            break;
        }
        case HEADSIZE:
        {
            if(*SortAscending)
                result=(atoi(pui2->szSize))-(atoi(pui1->szSize));
            else
                result=(atoi(pui1->szSize))-(atoi(pui2->szSize));
            break;
        }

    }

    return result;
}

void SortEm(HWND hwndLView)
{
     ListView_SortItems(hwndLView,ListViewCompareProc,(LPARAM)hwndLView);
}
