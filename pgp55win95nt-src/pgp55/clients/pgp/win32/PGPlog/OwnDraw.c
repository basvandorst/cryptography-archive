/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: OwnDraw.c,v 1.8 1997/10/20 14:53:54 wjb Exp $
____________________________________________________________________________*/
/*
 * OwnDraw.c  ListView draw routines to accommidate italics/strikeout
 *
 * These routines use the ownerdraw characteristic of the standard 
 * listview to provide custom drawing routines.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */
#include "precomp.h"



void CreateDrawElements(DRAWSTRUCT *ds)
{
	LOGFONT lf;
	int iNumBits;
	HDC hDC;
	HBITMAP hBmp;

	ds->DisplayMarginal=FALSE;

	GetValidityDrawPrefs(&(ds->DisplayMarginal));
	GetMarginalInvalidPref(&(ds->MarginalInvalid));

    ds->stdbarbrush = 
        CreateSolidBrush (GetSysColor (COLOR_3DSHADOW));
    ds->spcbarbrush = 
        CreateHatchBrush(HS_BDIAGONAL, GetSysColor (COLOR_WINDOW));
    ds->g_seltextpen = 
        CreatePen (PS_SOLID, 0, GetSysColor (COLOR_WINDOWTEXT));
    ds->g_unseltextpen =
        CreatePen (PS_SOLID, 0, GetSysColor (COLOR_WINDOW));

	ds->barcolor = GetSysColor (COLOR_3DSHADOW);

    ds->buttonpen = CreatePen(PS_SOLID, 0, GetSysColor (COLOR_3DSHADOW));
	ds->hilightpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DHILIGHT));
	ds->shadowpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DDKSHADOW));

    ds->barbgbrush = CreateSolidBrush (GetSysColor (COLOR_3DFACE));

    ds->HighBrush=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    ds->BackBrush=CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    SystemParametersInfo (SPI_GETICONTITLELOGFONT, 
		sizeof(LOGFONT), &lf, 0);

    ds->hFont=CreateFontIndirect (&lf);
    lf.lfItalic = !lf.lfItalic;
    ds->hItalic=CreateFontIndirect (&lf);
    lf.lfItalic = !lf.lfItalic;
    lf.lfStrikeOut=TRUE;
    ds->hStrikeOut = CreateFontIndirect (&lf);

	// ImageList Init

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		ds->hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (ds->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		ds->hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (ds->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
}

void DeleteDrawElements(DRAWSTRUCT *ds)
{
    DeleteObject(ds->stdbarbrush);
    DeleteObject(ds->spcbarbrush);
    DeleteObject(ds->g_seltextpen);
    DeleteObject(ds->g_unseltextpen);
	DeleteObject(ds->hilightpen);
	DeleteObject(ds->shadowpen);
	DeleteObject(ds->buttonpen);

    DeleteObject(ds->barbgbrush); 
    DeleteObject(ds->HighBrush); 
    DeleteObject(ds->BackBrush); 
	DeleteObject(ds->hFont);
	DeleteObject(ds->hItalic);
	DeleteObject(ds->hStrikeOut);
	ImageList_Destroy(ds->hIml);
}

void DrawBar(DRAWSTRUCT *ds,HDC hdc,RECT *ptrBarRect,
             int DataValue,int MaxValue,BOOL Selected)
{
    RECT rc;
    HBRUSH oldbrush,barbrush;
    HPEN oldpen;
    COLORREF oldbkcolor;
	int cx;

	CopyRect(&rc,ptrBarRect);

    if(Selected)
    {
        FillRect (hdc, &rc, ds->HighBrush);
        oldpen = SelectObject (hdc, ds->g_unseltextpen);
    }
    else
    {
        FillRect (hdc, &rc, ds->BackBrush);
        oldpen = SelectObject (hdc, ds->g_seltextpen);
    }

    rc.top+=5;
    rc.left+=5;
    rc.bottom-=5;
    rc.right-=5;

	if(rc.right<=rc.left)
		return;

	cx=rc.right-rc.left;

	oldbrush = SelectObject (hdc, ds->barbgbrush);
	SelectObject (hdc, ds->buttonpen);

	Rectangle (hdc, rc.left - 1, rc.top - 1, 
		rc.right + 2, rc.bottom + 2);

	SelectObject (hdc, ds->shadowpen);
	MoveToEx (hdc, rc.left, rc.bottom, NULL);
	LineTo (hdc, rc.left, rc.top);
	LineTo (hdc, rc.right, rc.top);

	SelectObject (hdc, ds->hilightpen);
	LineTo (hdc, rc.right, rc.bottom);
	LineTo (hdc, rc.left, rc.bottom);
														
	if (MaxValue != 0) 
	{
		if (DataValue > MaxValue) 
		{
			barbrush = ds->spcbarbrush;
			rc.right = rc.left + cx;
		}
		else 
		{
			barbrush = ds->stdbarbrush;
			rc.right = rc.left + 
				(int)(((float)DataValue / 
				(float)MaxValue)
				* (float)cx);
		}
	}
	else 
		rc.right = rc.left;

	rc.top++;
	rc.left++;

	if (rc.right > rc.left) 
	{
		oldbkcolor=SetBkColor (hdc, ds->barcolor);
		FillRect (hdc, &rc, barbrush); 

		rc.top--;
		rc.left--;

		// hilight pen already selected 
		MoveToEx (hdc, rc.right, rc.top, NULL);
		LineTo (hdc, rc.left, rc.top);
		LineTo (hdc, rc.left,rc.bottom);

		SelectObject (hdc, ds->shadowpen);
		LineTo (hdc, rc.right, rc.bottom);
		LineTo (hdc, rc.right, rc.top);
		SetBkColor(hdc,oldbkcolor);
	}

	SelectObject (hdc, oldbrush);
	SelectObject (hdc, oldpen);
}

void DrawNoviceButton(DRAWSTRUCT *ds,HDC hdc,RECT *ptrBarRect,
             int DataValue,int MaxValue,BOOL Selected)
{
	DWORD NoviceIcon;
//	HICON hIcon;

    if(Selected)
    {
        FillRect (hdc, ptrBarRect, ds->HighBrush);
    }
    else
    {
        FillRect (hdc, ptrBarRect, ds->BackBrush);
    }

	if((ptrBarRect->right-ptrBarRect->left)>=CX_SMICON)
	{
		NoviceIcon=IDX_INVALID;

		if(DataValue>2)
			NoviceIcon=IDX_AXIOMATIC;

		if(DataValue==2)
			NoviceIcon=IDX_VALID;

		if((!ds->MarginalInvalid)&&(DataValue==1))
			NoviceIcon=IDX_VALID;

#ifdef _WIN32
		ImageList_Draw(ds->hIml,NoviceIcon,
			hdc,
			ptrBarRect->left+((ptrBarRect->right-ptrBarRect->left-16)/2),
			ptrBarRect->top+((ptrBarRect->bottom-ptrBarRect->top-16)/2),
			ILD_TRANSPARENT);
#else
		hIcon=LoadIcon(g_hinst,MAKEINTRESOURCE(NoviceIcon));
	
		DrawIcon(hdc,
			ptrBarRect->left+((ptrBarRect->right-ptrBarRect->left-32)/2),
			ptrBarRect->top+((ptrBarRect->bottom-ptrBarRect->top-32)/2),
			hIcon);
	
		DeleteObject(hIcon);
#endif
	}
}

LISTSTRUCT *GetListStruct(HWND hwndList)
{
	return &logList;
}

DRAWSTRUCT *GetDrawStruct(HWND hDlg)
{
	return &logDraw;
}

void DrawStuff(LPDRAWITEMSTRUCT lpDrawItem)
{
	DRAWDATA *dd;
	DRAWSTRUCT *ds;
	LISTSTRUCT *ls;
	int index;
	RECT rc;
//	HICON hIcon;
	HWND hDlg,hwndList;
	int max;

	hwndList=lpDrawItem->hwndItem;
	hDlg=GetParent(hwndList);
	
	ls=GetListStruct(hwndList);
	ds=GetDrawStruct(hDlg);

	dd=(DRAWDATA *)(lpDrawItem->itemData);
	CopyRect(&rc,&(lpDrawItem->rcItem));
/*
	hIcon=LoadIcon(g_hinst,MAKEINTRESOURCE(dd->icon));
	
	DrawIcon(lpDrawItem->hDC,
		rc.left+((CX_SMICON-32)/2),
		rc.top+((CY_SMICON-32)/2),
		hIcon);
	
	DeleteObject(hIcon);
*/
	ImageList_Draw(ds->hIml,dd->icon,
		lpDrawItem->hDC,
		rc.left+((CX_SMICON-16)/2),
		rc.top+((CY_SMICON-16)/2),
		ILD_TRANSPARENT);

	rc.right=rc.left;
	rc.left=rc.left+16;

	max=0;

	for(index=0;index<NUMCOLUMNS;index++)
	{
#if LISTBOX
		max=max+ls->colwidth[index];
#else
		max=max+
			ListView_GetColumnWidth(lpDrawItem->hwndItem,index);
#endif
	}

	for(index=0;index<NUMCOLUMNS;index++)
	{
#if LISTBOX
		rc.right=rc.right+ls->colwidth[index];
#else
		rc.right=rc.right+
			ListView_GetColumnWidth(lpDrawItem->hwndItem,index);
#endif
		switch(dd->type[index])
		{
			case PGP_DDERROR:
			{
				rc.right=max;
				DrawItemColumn(lpDrawItem->hDC,
					(char *)dd->data1[index],
					&rc);
				break;
			}

			case PGP_DDTEXT:
			{
				DrawItemColumn(lpDrawItem->hDC,
					(char *)dd->data1[index],
					&rc);
				break;
			}

			case PGP_DDBAR:
			{
				if(ds->DisplayMarginal)
					DrawBar(ds,lpDrawItem->hDC,&rc,
						(int)dd->data1[index],(int)dd->data2[index],
						(lpDrawItem->itemState & ODS_SELECTED));
				else
					DrawNoviceButton(ds,lpDrawItem->hDC,&rc,
					(int)dd->data1[index],(int)dd->data2[index],
					(lpDrawItem->itemState & ODS_SELECTED));
				break;
			}

		}
		rc.left=rc.right;
	}
	lpDrawItem->rcItem.right=max;
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
    UINT uiFlags;

	uiFlags=ILD_TRANSPARENT;

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

	DrawStuff(lpDrawItem);

	lpDrawItem->rcItem.left=lpDrawItem->rcItem.left+CX_SMICON;

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
        DrawFocusRect(lpDrawItem->hDC, &(lpDrawItem->rcItem));
    }

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
        lpszTemp = (LPTSTR) malloc(cchMax);
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
    free(lpszTemp);
    return (fSuccess);
}


// Main_OnDrawItem
//
// Entry function for the message handler. Basically, we want to draw
// the whole thing no matter what.

BOOL Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{

    // Make sure the control is the listview control
    if ((lpDrawItem->CtlType != ODT_LISTVIEW)&&
		(lpDrawItem->CtlType != ODT_LISTBOX))
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
    if ((lpMeasureItem->CtlType != ODT_LISTVIEW)&&
		(lpMeasureItem->CtlType != ODT_LISTBOX))
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


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

