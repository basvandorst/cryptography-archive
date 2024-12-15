/*
 * DragItem.c Routines needed to create images for the ListView drag
 *
 * Since we're using ownerdrawn listboxes, we have to do all this ourselves.
 * These routines build the ImageList used with drag operations.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
#include "precomp.h"

HBITMAP Custom_Merge(HWND hwnd,HBITMAP AccumBmp,HBITMAP NewBmp,
                     int NewY,int RetX,int RetY,int *AccumY)
{
    RECT bmpRect;
    HDC hdc,hdcDest,hdcSrc;
    HBITMAP hBmp;

    bmpRect.left=bmpRect.top=0;
    bmpRect.right=RetX;
    bmpRect.bottom=RetY;

    hdc=GetDC(hwnd);
    hdcSrc=CreateCompatibleDC(hdc);
    hdcDest=CreateCompatibleDC(hdc);
    hBmp=CreateCompatibleBitmap(hdc,RetX,RetY);
    ReleaseDC(hwnd,hdc);

    SelectObject(hdcDest,hBmp);

    FillRect(hdcDest,&bmpRect,(RecGbl(GetParent(hwnd)))->BackBrush);

    SelectObject(hdcSrc,AccumBmp);

    BitBlt(hdcDest,// handle to destination device context
           0,      // x-coordinate of destination rectangle's upper-left
           0,      // x-coordinate of destination rectangle's upper-left
           RetX,   // width of destination rectangle
           *AccumY,// height of destination rectangle
           hdcSrc, // handle to source device context
           0,      // x-coordinate of source rectangle's upper-left
           0,      // y-coordinate of source rectangle's upper-left
           SRCCOPY);

    SelectObject(hdcSrc,NewBmp);

    BitBlt(hdcDest,// handle to destination device context
           0,      // x-coordinate of destination rectangle's upper-left
           NewY,   // x-coordinate of destination rectangle's upper-left
           RetX,   // width of destination rectangle
           RetY-NewY,// height of destination rectangle
           hdcSrc, // handle to source device context
           0,      // x-coordinate of source rectangle's upper-left
           0,      // y-coordinate of source rectangle's upper-left
           SRCCOPY);

    DeleteDC(hdcSrc);
    DeleteDC(hdcDest);

    DeleteObject(AccumBmp);
    DeleteObject(NewBmp);

    *AccumY=RetY;

    return hBmp;
}



HIMAGELIST MakeDragImage(HWND hwndDragFrom,int *left,int *bottom)
{
    int iCount, i, index, uState, result;
    POINT pnt;
    HIMAGELIST hDragImage;
    HBITMAP NewBmp,AccumBmp;
    RECT rcItem;
    int lastx,lasty;
    int DragXsize,DragYsize;

    hDragImage=0;
    index=0;

    // How many items are there?
    iCount = ListView_GetItemCount(hwndDragFrom);

    for( i = 0; i < iCount; i++ )  // Loop through each item
    {
	uState = ListView_GetItemState(hwndDragFrom , i, LVIS_SELECTED);

	if( uState ) //  If this item is selected we want to move it
	{
	       result=ListView_GetItemRect(hwndDragFrom,i,
                                           &rcItem,LVIR_BOUNDS);

               pnt.x=rcItem.left;
               pnt.y=rcItem.top;

               NewBmp = OwnerDraw_CreateDragImage(hwndDragFrom,i);

               if(index==0)
               {
                   AccumBmp=NewBmp;
                   lastx=rcItem.left;
                   lasty=rcItem.top;
                   *left=rcItem.left;
                   *bottom=rcItem.bottom;
                   DragYsize=rcItem.bottom-rcItem.top;
               }
               else
               {
                   AccumBmp=Custom_Merge(hwndDragFrom,AccumBmp,
                      NewBmp,pnt.y-lasty,rcItem.right,rcItem.bottom-lasty,
                      &DragYsize);
               }

               DragXsize=rcItem.right-rcItem.left;
            index++;
        }
    }

    hDragImage = ImageList_Create (DragXsize, DragYsize,
                                   ILC_MASK|ILC_COLORDDB, 1, 1);
    ImageList_AddMasked (hDragImage, AccumBmp, GetSysColor(COLOR_WINDOW));
    DeleteObject(AccumBmp);

    return hDragImage;
}
