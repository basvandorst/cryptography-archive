/*
 * OwnDraw.c  ListView draw routines to accommidate italics/strikeout
 *
 * These routines use the ownerdraw characteristic of the standard
 * listview to provide custom drawing routines.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
#include "precomp.h"

void CreateDrawElements(HWND hwnd)
{
    DWORD rgb;
    int r,g,b;

    (RecGbl(hwnd))->g_stdbarbrush =
        CreateSolidBrush (GetSysColor (COLOR_3DSHADOW));
    (RecGbl(hwnd))->g_spcbarbrush =
        CreateHatchBrush(HS_BDIAGONAL, GetSysColor (COLOR_WINDOW));
    (RecGbl(hwnd))->g_seltextpen =
        CreatePen (PS_SOLID, 0, GetSysColor (COLOR_WINDOWTEXT));
    (RecGbl(hwnd))->g_unseltextpen =
        CreatePen (PS_SOLID, 0, GetSysColor (COLOR_WINDOW));

    rgb = GetSysColor (COLOR_BTNFACE);
    r = GetRValue (rgb);
    r = 255 - ((255 - r)/2);
    g = GetGValue (rgb);
    g = 255 - ((255 - g)/2);
    b = GetBValue (rgb);
    b = 255 - ((255 - b)/2);

    (RecGbl(hwnd))->barbgbrush = CreateSolidBrush (RGB (r,g,b));

    (RecGbl(hwnd))->HighBrush=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    (RecGbl(hwnd))->BackBrush=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
}

void DeleteDrawElements(HWND hwnd)
{
    DeleteObject((RecGbl(hwnd))->g_stdbarbrush);
    DeleteObject((RecGbl(hwnd))->g_spcbarbrush);
    DeleteObject((RecGbl(hwnd))->g_seltextpen);
    DeleteObject((RecGbl(hwnd))->g_unseltextpen);
    DeleteObject((RecGbl(hwnd))->barbgbrush);
    DeleteObject((RecGbl(hwnd))->HighBrush);
    DeleteObject((RecGbl(hwnd))->BackBrush);
}

void DrawBar(HWND hwnd,HDC hdc,RECT *ptrBarRect,
             int DataValue,int MaxValue,BOOL Selected)
{
    RECT rc;
    HBRUSH oldbrush,barbrush;
    HPEN oldpen;
    COLORREF oldbkcolor;

    CopyRect(&rc,ptrBarRect);

    if(Selected)
    {
        FillRect (hdc, &rc, (RecGbl(hwnd))->HighBrush);
        oldpen = SelectObject (hdc, (RecGbl(hwnd))->g_unseltextpen);
    }
    else
    {
        FillRect (hdc, &rc, (RecGbl(hwnd))->BackBrush);
        oldpen = SelectObject (hdc, (RecGbl(hwnd))->g_seltextpen);
    }

    oldbrush = SelectObject (hdc, (RecGbl(hwnd))->barbgbrush);

    rc.top+=5;
    rc.left+=5;
    rc.bottom-=5;
    rc.right-=5;

    if (rc.right > rc.left)
    {
        Rectangle (hdc, rc.left, rc.top, rc.right, rc.bottom);
    }

    SelectObject (hdc, oldbrush);
    SelectObject (hdc, oldpen);

    if (DataValue > MaxValue)
    {
        barbrush = (RecGbl(hwnd))->g_spcbarbrush;
    }
    else
    {
        barbrush = (RecGbl(hwnd))->g_stdbarbrush;
        rc.right = rc.left +
            (int)(((float)DataValue / (float)MaxValue)
            * (float)(rc.right-rc.left));
    }

    rc.top++;
    rc.left++;
    rc.bottom--;

    if (rc.right > rc.left)
    {
        rc.right--;

        oldbkcolor=SetBkColor(hdc,GetSysColor (COLOR_3DSHADOW));
        FillRect (hdc, &rc, barbrush);
        SetBkColor(hdc,oldbkcolor);
    }

}


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
    DrawItem.itemData=0;// lParam taken care of in DrawLVitem

    FillRect(hdcMem,&DrawItem.rcItem,(RecGbl(GetParent(hwnd)))->BackBrush);

    DrawListViewItem(&DrawItem);
    DeleteDC(hdcMem);

    return hBmp;
}

//
//  DrawListViewItem
//
//  This routine, given a standard Windows LPDRAWITEMSTRUCT, draws the
//  elements of our custom listview (adapted from a routine in the Microsoft
//  Knowledge base)
//

void DrawListViewItem(LPDRAWITEMSTRUCT lpDrawItem)
{
    HIMAGELIST himl;
    LV_ITEM lvi;
    int cxImage = 0, cyImage = 0;

    RECT rcClip;
    int iColumn = 1;
    UINT uiFlags = ILD_TRANSPARENT;
    PUSERKEYINFO pui;
    HFONT oldFont;
    HWND hdlg;
    int width;

    hdlg=GetParent(lpDrawItem->hwndItem);


    // Get the item image to be displayed
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
    lvi.iItem = lpDrawItem->itemID;
    lvi.iSubItem = 0;
    if(ListView_GetItem(lpDrawItem->hwndItem, &lvi)==FALSE)
        return; // just in case we start drawing while we're half setup
                // (This breaks in 3.1, maybe not win32, but just in case)

    // Check to see if this item is selected
    if (lpDrawItem->itemState & ODS_SELECTED)
    {
        // Set the text background and foreground colors
        SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHT));

        // Also add the ILD_BLEND50 so the images come out selected
        uiFlags |= ILD_BLEND50;
    }
    else
    {
        // Set the text background and foreground colors to the
        // standard window colors
        SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
    }

    // Get the image list and draw the image
    himl = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);

    if (himl)
    {

        ImageList_Draw(himl, lvi.iImage, lpDrawItem->hDC,
                       lpDrawItem->rcItem.left, lpDrawItem->rcItem.top,
                       uiFlags);

        // Find out how big the image we just drew was

        cxImage=cyImage=16;
        lpDrawItem->rcItem.left+=cxImage;
    }

    // Calculate the width of the first column after the image width.  If
    // There was no image, then cxImage will be zero.
    width=ListView_GetColumnWidth(lpDrawItem->hwndItem,0);

    // Set up the new clipping rect for the first column text and draw it
    rcClip.left = lpDrawItem->rcItem.left;
    rcClip.right = lpDrawItem->rcItem.left + width - cxImage;
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;

    pui=(PUSERKEYINFO)(lvi.lParam);

    oldFont=SelectObject(lpDrawItem->hDC,(RecGbl(hdlg))->g_hTLFont);

    if((pui->Flags&PUIF_NOTFOUND)&&((pui->Flags&PUIF_MULTIPLEFOUND)==0))
            oldFont=SelectObject(lpDrawItem->hDC,
                                   (RecGbl(hdlg))->g_hTLFontStrike);

    DrawItemColumn(lpDrawItem->hDC,pui->UserId,
                    &rcClip);

    width=ListView_GetColumnWidth(lpDrawItem->hwndItem,1);

    rcClip.left = rcClip.right;
    rcClip.right = rcClip.right + width;

    DrawBar(hdlg,
            lpDrawItem->hDC,&rcClip,pui->Validity,
            2,(lpDrawItem->itemState & ODS_SELECTED));

    width=ListView_GetColumnWidth(lpDrawItem->hwndItem,2);

    rcClip.left = rcClip.right;
    rcClip.right = rcClip.right + width;

    DrawBar(hdlg,
            lpDrawItem->hDC,&rcClip,pui->Trust,
            2,(lpDrawItem->itemState & ODS_SELECTED));

    width=ListView_GetColumnWidth(lpDrawItem->hwndItem,3);

    rcClip.left = rcClip.right;
    rcClip.right = rcClip.right + width;

    DrawItemColumn(lpDrawItem->hDC,pui->szSize,
                     &rcClip);

    rcClip.left = lpDrawItem->rcItem.left;

    // If we changed the colors for the selected item, undo it
    if (lpDrawItem->itemState & ODS_SELECTED)
    {
        // Set the text background and foreground colors
        SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
    }

    // If the item is focused, now draw a focus rect around the entire row
    if (lpDrawItem->itemState & ODS_FOCUS)
    {
         // Draw the focus rect
         DrawFocusRect(lpDrawItem->hDC, &rcClip);
    }

    SelectObject(lpDrawItem->hDC,oldFont);

    return;
}


// DrawItemColumn
//
// Given a clipping rectange and some text, see how well we can fit
// it in there, and tack on ... if we can't

void DrawItemColumn(HDC hdc, LPTSTR lpsz, LPRECT prcClip)
{
    TCHAR szString[256];

    // Check to see if the string fits in the clip rect.  If not, truncate
    // the string and add "...".
    lstrcpy(szString, lpsz);
    CalcStringEllipsis(hdc, szString, 256, prcClip->right - prcClip->left);

    // print the text

    ExtTextOut(hdc, prcClip->left + 2, prcClip->top + 1,
               ETO_CLIPPED | ETO_OPAQUE,
               prcClip, szString, lstrlen(szString), NULL);
}


// CalcStringEllipsis
//
// Trial and error routine used to see where to put the ... in our string
// to make it fit within a clipping rectangle.

BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString,
                        int cchMax, UINT uColWidth)
{
     const TCHAR szEllipsis[] = TEXT("...");
     SIZE   sizeString;
     SIZE   sizeEllipsis;
     int    cbString;
     LPTSTR lpszTemp;
     BOOL   fSuccess = FALSE;
     BOOL fOnce = TRUE;
     FARPROC pGetTextExtentPoint;

     if (fOnce)
     {
         fOnce = FALSE;

         pGetTextExtentPoint = &GetTextExtentPoint;
     }

     // Adjust the column width to take into account the edges
     uColWidth -= 4;

     {
        // Allocate a string for us to work with.  This way we can mangle the
        // string and still preserve the return value
        lpszTemp = (LPTSTR) pgpMemAlloc(cchMax);
        lstrcpy(lpszTemp, lpszString);

        // Get the width of the string in pixels
        cbString = lstrlen(lpszTemp);
        (pGetTextExtentPoint)(hdc, lpszTemp, cbString, &sizeString);

        // If the width of the string is greater than the column width shave
        // the string and add the ellipsis
        if ((ULONG)sizeString.cx > uColWidth)
        {
            (pGetTextExtentPoint)(hdc, szEllipsis, lstrlen(szEllipsis),
                                       &sizeEllipsis);

            while (cbString > 0)
            {
                lpszTemp[--cbString] = 0;
                !(pGetTextExtentPoint)(hdc, lpszTemp, cbString, &sizeString);

                if ((ULONG)(sizeString.cx + sizeEllipsis.cx) <= uColWidth)
                {
                // The string with the ellipsis finally fits, now make sure
                // there is enough room in the string for the ellipsis
                    if (cchMax >= (cbString + lstrlen(szEllipsis)))
                    {
                    // Concatenate the two strings and break out of the loop
                        lstrcat(lpszTemp, szEllipsis);
                        lstrcpy(lpszString, lpszTemp);
                        fSuccess = TRUE;
                        break;
                    }
                }
            }
        }
        else
        {
            // No need to do anything, everything fits great.
            fSuccess = TRUE;
        }
    }

    // Free the memory
    pgpFree(lpszTemp);
    return (fSuccess);
}


// Main_OnDrawItem
//
// Entry function for the message handler. Basically, we want to draw
// the whole thing no matter what.

BOOL Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{

    // Make sure the control is the listview control
    if (lpDrawItem->CtlType != ODT_LISTVIEW)
        return FALSE;

    switch (lpDrawItem->itemAction)
    {
        case ODA_DRAWENTIRE:
        case ODA_FOCUS:
        case ODA_SELECT:
            DrawListViewItem((LPDRAWITEMSTRUCT)lpDrawItem);
            break;
    }

    return TRUE;
}

// Main_OnMeasureItem
//
// Entry function for the message handler. We need to get the width and
// height of the font we're using.

void Main_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
    TEXTMETRIC tm;
    HDC hdc;
    HWND hwndLV;
    HFONT hFont;

    // Make sure the control is the listview control
    if (lpMeasureItem->CtlType != ODT_LISTVIEW)
         return;

    // Get the handle of the ListView control we're using
    hwndLV = GetDlgItem(hwnd, lpMeasureItem->CtlID);

    // Get the font the control is currently using
    hFont = (HFONT)(DWORD) SendMessage(hwndLV, WM_GETFONT, 0, 0L);

    // Set the font of the DC to the same font the control is using
    hdc = GetDC(hwndLV);
    SelectObject(hdc, hFont);

    // Get the height of the font used by the control
    if (!GetTextMetrics(hdc, &tm))
        return;

    // Add a little extra space between items
    lpMeasureItem->itemHeight = tm.tmHeight + 1;

    // Make sure there is enough room for the images which are CY_SMICON high
    if (lpMeasureItem->itemHeight < (CY_SMICON + 1))
        lpMeasureItem->itemHeight = CY_SMICON + 1;

    // Clean up
    ReleaseDC(hwndLV, hdc);
}
