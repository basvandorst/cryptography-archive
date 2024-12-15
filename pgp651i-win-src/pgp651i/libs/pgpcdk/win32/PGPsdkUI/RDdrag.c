/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: RDdrag.c,v 1.3 1999/03/10 03:00:51 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

/*
 * DragItem.c  Routines needed to create images for the ListView drag
 *
 * Since we're using ownerdrawn listboxes, we have to do all this ourselves.
 * These routines build the ImageList used with drag operations.
 *
 * Copyright (C) 1996 Network Associates Inc. and affiliated companies.
 * All rights reserved.
 */

  
// OwnerDraw_CreateDragImage
//
// This routine replaces ListView_CreateDragImage since that routine
// breaks (doh!) under ownerdraw. It returns an himagelist associated
// with the selected item and mimics the ListView_CreateDragImage
// routine

HBITMAP OwnerDraw_CreateDragImage(HWND hwnd,int selitem)
{
    DRAWITEMSTRUCT DrawItem;
    HDC hdc,hdcMem;
    HBITMAP hBmp;
    RECT rcEntry;
    int dx,dy;
	LV_ITEM lvi;
	PRECGBL prg;

	prg=(PRECGBL)GetWindowLong(GetParent(hwnd),GWL_USERDATA);

	lvi.mask = LVIF_PARAM;
    lvi.iItem = selitem;
    lvi.iSubItem = 0;
    if(ListView_GetItem(hwnd, &lvi)==FALSE)
        return 0; // just in case we start drawing while we're half setup

    ListView_GetItemRect(hwnd,selitem,&rcEntry,LVIR_BOUNDS); 
    dx=rcEntry.right-rcEntry.left;
    dy=rcEntry.bottom-rcEntry.top;

    hdc=GetDC(hwnd);
    hdcMem=CreateCompatibleDC(hdc);
    hBmp=CreateCompatibleBitmap(hdc,dx,dy);
    ReleaseDC(hwnd,hdc);

    SelectObject(hdcMem,hBmp);

    DrawItem.CtlType=ODT_LISTVIEW; 
    DrawItem.CtlID=0; // Don't really need this
    DrawItem.itemID=selitem; 
    DrawItem.itemAction=ODA_DRAWENTIRE; 
    DrawItem.itemState=0;  // No highlights or anything
    DrawItem.hwndItem=hwnd; 
    DrawItem.hDC=hdcMem; 
    DrawItem.rcItem.left=0;
    DrawItem.rcItem.top=0;
    DrawItem.rcItem.right=dx;
    DrawItem.rcItem.bottom=dy;
    DrawItem.itemData=lvi.lParam;// lParam taken care of in DrawLVitem

    FillRect(hdcMem,&DrawItem.rcItem,
		prg->ds.BackBrush);
 
    DrawListViewItem(&DrawItem);
    DeleteDC(hdcMem);

    return hBmp;
}

HBITMAP Custom_Merge(HWND hwnd,HBITMAP AccumBmp,HBITMAP NewBmp,
                     int NewY,int RetX,int RetY,int *AccumY)
{                    
    RECT bmpRect;
    HDC hdc,hdcDest,hdcSrc;
    HBITMAP hBmp;
 	PRECGBL prg;

	prg=(PRECGBL)GetWindowLong(GetParent(hwnd),GWL_USERDATA);

    bmpRect.left=bmpRect.top=0;
    bmpRect.right=RetX;
    bmpRect.bottom=RetY;
    
    hdc=GetDC(hwnd);
    hdcSrc=CreateCompatibleDC(hdc);
    hdcDest=CreateCompatibleDC(hdc);
    hBmp=CreateCompatibleBitmap(hdc,RetX,RetY);
    ReleaseDC(hwnd,hdc);

    SelectObject(hdcDest,hBmp);        
 
    FillRect(hdcDest,&bmpRect,prg->ds.BackBrush);
                    
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


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

