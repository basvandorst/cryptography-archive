//:TLPaint.c - paint tree list
//
//	$Id: TLPaint.c,v 1.13 1997/09/17 15:16:17 pbj Exp $
//
#include "TLIntern.h"
#include <math.h>

typedef struct _DOTTEDSTRUCT {
	HDC hDC;
	DWORD dwDottedColor;
} DOTTEDSTRUCT;


//----------------------------------------------------|
// Initialize painting routines -- create pens, brushes, etc.

void TLInitPaint (TLWndData* pWD) {
	DWORD rgb;
	BYTE r, g, b;
	LOGFONT lf;
	HDC hdcMem, hdcScreen;
	HBITMAP hOldBitmap;
	TEXTMETRIC tm;
	RECT rc;
	HPEN pen;
	int i;

	hdcMem = CreateCompatibleDC (NULL);	

	pWD->selbgbrush = CreateSolidBrush (GetSysColor (COLOR_HIGHLIGHT));
	pWD->unselbgbrush = CreateSolidBrush (GetSysColor (COLOR_WINDOW));
	pWD->selfgbrush = CreateSolidBrush (GetSysColor (COLOR_HIGHLIGHTTEXT));
	pWD->unselfgbrush = CreateSolidBrush (GetSysColor (COLOR_WINDOWTEXT));

	pWD->unselfocuscolor = GetSysColor (COLOR_HIGHLIGHT);
	r = GetRValue (pWD->unselfocuscolor);
	if (r > 127) r = 0;
	else r = 255;
	g = GetGValue (pWD->unselfocuscolor);
	if (g > 127) g = 0;
	else g = 255;
	b = GetBValue (pWD->unselfocuscolor);
	if (b > 127) b = 0;
	else b = 255;
	pWD->selfocuscolor = PALETTERGB (r, g, b);

	pWD->seltextpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_HIGHLIGHTTEXT));
	pWD->unseltextpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_WINDOWTEXT));

    pWD->buttonpen = CreatePen(PS_SOLID, 0, GetSysColor (COLOR_3DSHADOW));
	pWD->linecolor = GetSysColor (COLOR_3DSHADOW);

	pWD->barcolor = GetSysColor (COLOR_3DSHADOW);
	pWD->stdbarbrush = CreateSolidBrush (GetSysColor (COLOR_3DSHADOW));
	pWD->spcbarbrush = CreateHatchBrush (HS_BDIAGONAL, 
									GetSysColor (COLOR_HIGHLIGHTTEXT));

	pWD->hilightpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DHILIGHT));
	pWD->shadowpen = CreatePen (PS_SOLID, 0, 
								GetSysColor (COLOR_3DDKSHADOW));

	// GetDeviceCaps returns possitive value for displays with 2-256 colors
	if (GetDeviceCaps (hdcMem, NUMCOLORS) > 0) {
		hdcScreen = GetDC (NULL);
		pWD->barbgbitmap = CreateCompatibleBitmap (hdcScreen, 8, 8);
		hOldBitmap = SelectObject (hdcMem, pWD->barbgbitmap);
		rc.left = 0;
		rc.top = 0;
		rc.bottom = 8;
		rc.right = 8;
		FillRect (hdcMem, &rc, pWD->unselbgbrush);
		pen = CreatePen (PS_SOLID, 0, GetSysColor (COLOR_3DLIGHT));
		SelectObject (hdcMem, pen);
		for (i=-8; i<8; i+=2) {
			MoveToEx (hdcMem, i, 0, NULL);
			LineTo (hdcMem, i+8, 8);
		}
		DeleteObject (pen);
		pWD->barbgbrush = CreatePatternBrush (pWD->barbgbitmap);
		SelectObject (hdcMem, hOldBitmap);
		ReleaseDC (NULL, hdcScreen);
	}
	// GetDeviceCaps returns -1 for displays with more than 8 bit depth
	else {
		rgb = GetSysColor (COLOR_BTNFACE);
		r = GetRValue (rgb);
		r = 255 - ((255 - r)/2);
		g = GetGValue (rgb);
		g = 255 - ((255 - g)/2);
		b = GetBValue (rgb);
		b = 255 - ((255 - b)/2);
		pWD->barbgbitmap = NULL;
		pWD->barbgbrush = CreateSolidBrush (RGB (r,g,b));
	}

	SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);

	pWD->hFont = CreateFontIndirect (&lf);
	lf.lfItalic = !lf.lfItalic;
	pWD->hFontItalic = CreateFontIndirect (&lf);
	lf.lfItalic = !lf.lfItalic;
	lf.lfWeight = FW_BOLD;
	pWD->hFontBold = CreateFontIndirect (&lf);

	SelectObject (hdcMem, pWD->hFont);
	GetTextMetrics (hdcMem, &tm);
	pWD->iRowHeight = tm.tmHeight + (2 * (TEXTVERTOFF+1)) -1;

	DeleteDC (hdcMem);
}


//----------------------------------------------------|
// Close painting routines -- delete pens, brushes, etc.

void TLClosePaint (TLWndData* pWD) {
	DeleteObject (pWD->selbgbrush);
	DeleteObject (pWD->unselbgbrush);
	DeleteObject (pWD->selfgbrush);
	DeleteObject (pWD->unselfgbrush);
	DeleteObject (pWD->stdbarbrush);
	DeleteObject (pWD->spcbarbrush);
	DeleteObject (pWD->barbgbrush);
	DeleteObject (pWD->seltextpen);
	DeleteObject (pWD->unseltextpen);
	DeleteObject (pWD->buttonpen);
	DeleteObject (pWD->hilightpen);
	DeleteObject (pWD->shadowpen);
	DeleteObject (pWD->hFont);
	DeleteObject (pWD->hFontItalic);
	DeleteObject (pWD->hFontBold);
	if (pWD->barbgbitmap) DeleteObject (pWD->barbgbitmap);
}


//----------------------------------------------------|
//  Truncate text string

void TruncateText (HDC hdc, char * szorig, int xmax, char * sztrunc) {
	SIZE s;
	int l, w;
	lstrcpy (sztrunc, szorig);
	GetTextExtentPoint32 (hdc, szorig, lstrlen (szorig), &s);
	w = s.cx + 4;

	l = lstrlen (szorig);
	if (l < 3) {
		if (w > xmax) lstrcpy (sztrunc, "");
	}
	else {
		l = lstrlen (szorig) - 3;
		while ((w > xmax) && (l >= 0)) {
			lstrcpy (&sztrunc[l], "...");
			GetTextExtentPoint32 (hdc, sztrunc, lstrlen (sztrunc), &s);
			w = s.cx + 4;
			l--;
		}	
		if (l < 0) lstrcpy (sztrunc, "");
	}
}


//----------------------------------------------------|
//  Calculate how many lines are needed

int CountRows (TLTreeItem* p) {
	int i = 0;
	
	while (p) {
		i++;
		if (p->childItem && (p->state & TLIS_EXPANDED)) 
			i += CountRows (p->childItem);
		p = p->nextItem;
	}
	return i;
}


//----------------------------------------------------|
//  Get row number of item

BOOL TLGetRow (TLTreeItem* root, TLTreeItem* p, int* row) {
	
	while (root) {
		if (p == root) return TRUE;
		(*row)++;
		if (root->childItem && (root->state & TLIS_EXPANDED)) {
			if (TLGetRow (root->childItem, p, row)) return TRUE;
		}
		root = root->nextItem;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Item has just been expanded -- scroll window appropriately

void TLExpandAutoScroll (TLWndData* pWD, TLTreeItem* p) {
	int itemRow = 0;

	if (TLGetRow (pWD->rootItem, p, &itemRow)) {
		if (((itemRow-pWD->iFirstRow) + p->cChildren) >= pWD->iMaxRows) {
			if (p->cChildren >= pWD->iMaxRows) pWD->iFirstRow = itemRow;
			else pWD->iFirstRow = itemRow + p->cChildren - pWD->iMaxRows +1;
			if (pWD->iFirstRow < 0) pWD->iFirstRow = 0;
		}
	}
	return;
}
	

//----------------------------------------------------|
//  Item has just been selected -- scroll window appropriately

BOOL TLSelectAutoScroll (TLWndData* pWD, TLTreeItem* p) {
	int itemRow = 0;

	if (TLGetRow (pWD->rootItem, p, &itemRow)) {
		if ((itemRow-pWD->iFirstRow) >= pWD->iMaxRows) {
			pWD->iFirstRow = itemRow - pWD->iMaxRows +1;
			if (pWD->iFirstRow < 0) pWD->iFirstRow = 0;
			return TRUE;
		}
		if (itemRow < pWD->iFirstRow) {
			pWD->iFirstRow = itemRow;
			return TRUE;
		}
	}
	return FALSE;
}
	

//----------------------------------------------------|
//  Invalidate the specified item

void TLInvalidateItem (TLWndData* pWD, TLTreeItem* p) {
	RECT rc;
	rc = p->hiliteRect;
	rc.left = pWD->tlInval.left;
	InvalidateRect (pWD->hWnd, &rc, TRUE);	
}
	

//----------------------------------------------------|
//  draw a dotted line by setting alternate pixels
//  (callback to LineDDA)

void CALLBACK DottedLineProc (INT iX, INT iY, LPARAM lpData) {
	DOTTEDSTRUCT *pDottedStruct;

	pDottedStruct = (DOTTEDSTRUCT*)lpData;
	if (!((iX+iY) & 0x0001)) 
		SetPixel (pDottedStruct->hDC, iX, iY, pDottedStruct->dwDottedColor);
}
	

//----------------------------------------------------|
//  Paint one level of list

void PaintItems (HDC hdc, TLWndData* pWD, int *item, int* y, int indent, 
							TLTreeItem* p) {
	HBRUSH oldbrush, bgbrush, fgbrush, barbrush;
	INT iBCent, iBMPos;
	HPEN oldpen, textpen;
	SIZE s;
	INT ix, iy, cx, xoffset;
	TLColumnItem* pci;
	TLListItem* pli;
	RECT rc;
	DOTTEDSTRUCT DottedStruct;
	double f;
	char sz256[256];

	iBCent = pWD->iBCent;
	iBMPos = pWD->iBMPos;
	DottedStruct.hDC = hdc;

	xoffset = indent - pWD->iHorizontalPos;

	while (p) {

//  increment item count
		(*item)++;
//  check if items are visible in window
		if ((*item) > (pWD->iFirstRow + pWD->iMaxRows + 2)) break;
		if ((*item) > pWD->iFirstRow) {

//  first paint the tree items  ...
//  select colors based on state of tree item
			if (pWD->bTreeFocused && 
				(p->state & (TLIS_SELECTED|TLIS_DRAGGEDOVER))) {
				bgbrush = pWD->selbgbrush;
				fgbrush = pWD->selfgbrush;
				textpen = pWD->seltextpen;
				DottedStruct.dwDottedColor = pWD->selfocuscolor;
				pWD->textbgcolor = GetSysColor (COLOR_HIGHLIGHT);
				SetTextColor (hdc, GetSysColor (COLOR_HIGHLIGHTTEXT));
			}
			else {
				bgbrush = pWD->unselbgbrush;
				fgbrush = pWD->unselfgbrush;
				textpen = pWD->unseltextpen;
				DottedStruct.dwDottedColor = pWD->unselfocuscolor;
				pWD->textbgcolor = GetSysColor (COLOR_WINDOW);
				SetTextColor (hdc, GetSysColor (COLOR_WINDOWTEXT));
			}
			SetBkColor (hdc, pWD->textbgcolor);
		
//  set rectangle coordinates 
//  rectangle sensitive to "selection" mouse clicks
			p->selectRect.left = xoffset + BUTTONWIDTH;
			p->selectRect.right = pWD->tlInval.right;
			p->selectRect.top = *y+1;
			p->selectRect.bottom = p->selectRect.top + pWD->iRowHeight;

//  rectangle sensitive to "button" mouse clicks
			if (p->cChildren && (pWD->style & TLS_HASBUTTONS)) {
				p->buttonRect.left = xoffset;
				p->buttonRect.right = p->buttonRect.left + BUTTONWIDTH;
				p->buttonRect.top = *y;
				p->buttonRect.bottom = p->buttonRect.top + pWD->iRowHeight;
			}
			else {
				p->buttonRect.left = xoffset + BUTTONWIDTH;
				p->buttonRect.right = xoffset + BUTTONWIDTH;
				p->buttonRect.top = *y;
				p->buttonRect.bottom = 0;
			}

//  rectangle which is highlighted when item is selected
			p->hiliteRect.left = 
				min (xoffset + BUTTONWIDTH+BITMAPWIDTH+HTEXTSHIFT,
					 pWD->iFirstColumnWidth);
			p->hiliteRect.top = *y+1;
			p->hiliteRect.bottom = p->selectRect.top + pWD->iRowHeight;
			if (p->state & TLIS_BOLD) SelectObject (hdc, pWD->hFontBold);
			else if (p->state & TLIS_ITALICS) 
				SelectObject (hdc, pWD->hFontItalic);
			else SelectObject (hdc, pWD->hFont);
			TruncateText (hdc, p->pszText, 
				pWD->iFirstColumnWidth-
				(indent + BUTTONWIDTH+BITMAPWIDTH+HTEXTSHIFT), 
				sz256);
			GetTextExtentPoint32 (hdc, sz256, lstrlen (sz256), &s);
			p->hiliteRect.right = p->hiliteRect.left + s.cx + HTEXTSHIFT;
			pli = p->listItem;
			pci = pWD->columnItem;
			ix = pWD->iFirstColumnWidth;
			while (pci && pli) {
				ix += pci->cx;
				if (pli->state & TLIS_VISIBLE) p->hiliteRect.right = ix;
				pci = pci->nextItem;
				pli = pli->nextItem;
			}

//  if focused, draw box
			FillRect (hdc, &p->hiliteRect, bgbrush);
			if (pWD->bTreeFocused && (p->state & TLIS_FOCUSED)) {
				LineDDA (p->hiliteRect.left, p->hiliteRect.top,
						 p->hiliteRect.right-1, p->hiliteRect.top,
						 (LINEDDAPROC)DottedLineProc, (LPARAM)&DottedStruct);
				LineDDA (p->hiliteRect.right-1, p->hiliteRect.top,
						 p->hiliteRect.right-1, p->hiliteRect.bottom-1,
						 (LINEDDAPROC)DottedLineProc, (LPARAM)&DottedStruct);
				LineDDA (p->hiliteRect.right-1, p->hiliteRect.bottom-1,
						 p->hiliteRect.left, p->hiliteRect.bottom-1,
						 (LINEDDAPROC)DottedLineProc, (LPARAM)&DottedStruct);
				LineDDA (p->hiliteRect.left, p->hiliteRect.bottom-1,
						 p->hiliteRect.left, p->hiliteRect.top,
						 (LINEDDAPROC)DottedLineProc, (LPARAM)&DottedStruct);
			}

//  draw tree item text 
			iy = p->hiliteRect.top + TEXTVERTOFF;
			TextOut (hdc, p->hiliteRect.left+2, iy, sz256, lstrlen(sz256));

//  draw tree item bitmap, if there is space
			if (pWD->iFirstColumnWidth >= (p->buttonRect.right+BITMAPWIDTH)) {
				ImageList_Draw (pWD->hImageList, p->iImage, hdc,
								p->buttonRect.right, p->hiliteRect.top+iBMPos,
								ILD_TRANSPARENT);
			}
		
//  paint list items based on their data type
			SelectObject (hdc, pWD->hFont);
			pli = p->listItem;
			pci = pWD->columnItem;
			ix = pWD->iFirstColumnWidth - pWD->iHorizontalPos;
			while (pci && pli) {

				if (pli->state & TLIS_VISIBLE) {

					switch (pci->iDataType) {
					case TLC_DATANONE :
						break;

					case TLC_DATASTRING :
						TruncateText (hdc, pli->pszText, pci->cx, sz256);
						SetBkColor (hdc, pWD->textbgcolor);
						TextOut (hdc, ix+6, iy, sz256, lstrlen(sz256));
						break;

					case TLC_DATALONG :
						switch (pci->iDataFmt) {
						case TLCFMT_LINBAR :
							cx = pci->cx - BARCOLUMNDIFF;
							if (cx <= 0) break;
							rc.left = ix + HBARSHIFT;
							rc.top = iy + VBARSHIFT +1;
							rc.right = ix + pci->cx - HBARSHIFT -1;
							rc.bottom = iy+pWD->iRowHeight-(VBARSHIFT+2) -1;
							oldbrush = SelectObject (hdc, pWD->barbgbrush);
							oldpen = SelectObject (hdc, pWD->buttonpen);
							Rectangle (hdc, rc.left - 1, rc.top - 1, 
											rc.right + 2, rc.bottom + 2);

							SelectObject (hdc, pWD->shadowpen);
							MoveToEx (hdc, rc.left, rc.bottom, NULL);
							LineTo (hdc, rc.left, rc.top);
							LineTo (hdc, rc.right, rc.top);

							SelectObject (hdc, pWD->hilightpen);
							LineTo (hdc, rc.right, rc.bottom);
							LineTo (hdc, rc.left, rc.bottom);
														
							if (pci->lMaxValue != 0) {
								if (pli->lDataValue > pci->lMaxValue) {
									barbrush = pWD->spcbarbrush;
									rc.right = rc.left + cx;
								}
								else {
									barbrush = pWD->stdbarbrush;
									rc.right = rc.left + 
										(int)(((float)pli->lDataValue / 
											(float)pci->lMaxValue)
											* (float)cx);
								}
							}
							else 
								rc.right = rc.left;

							rc.top++;
							rc.left++;

							if (rc.right > rc.left) {
								SetBkColor (hdc, pWD->barcolor);
								FillRect (hdc, &rc, barbrush); 

								rc.top--;
								rc.left--;

								// hilight pen already selected 
								MoveToEx (hdc, rc.right, rc.top, NULL);
								LineTo (hdc, rc.left, rc.top);
								LineTo (hdc, rc.left,rc.bottom);

								SelectObject (hdc, pWD->shadowpen);
								LineTo (hdc, rc.right, rc.bottom);
								LineTo (hdc, rc.right, rc.top);
							}

							SelectObject (hdc, oldbrush);
							SelectObject (hdc, oldpen);
							break;

						case TLCFMT_LOGBAR :
							rc.left = ix + HBARSHIFT;
							rc.top = iy + VBARSHIFT +1;
							rc.right = ix + pci->cx - HBARSHIFT -1;
							rc.bottom = iy+pWD->iRowHeight-(VBARSHIFT+2) -1;
							oldbrush = SelectObject (hdc, pWD->barbgbrush);
							oldpen = SelectObject (hdc, pWD->buttonpen);
							Rectangle (hdc, rc.left - 1, rc.top - 1, 
											rc.right + 2, rc.bottom + 2);

							SelectObject (hdc, pWD->shadowpen);
							MoveToEx (hdc, rc.left, rc.bottom, NULL);
							LineTo (hdc, rc.left, rc.top);
							LineTo (hdc, rc.right, rc.top);

							SelectObject (hdc, pWD->hilightpen);
							LineTo (hdc, rc.right, rc.bottom);
							LineTo (hdc, rc.left, rc.bottom);
														
							if (pci->lMaxValue != 0) {
								if (pli->lDataValue > pci->lMaxValue) {
									barbrush = pWD->spcbarbrush;
									rc.right = rc.left + cx;
								}
								else {
									barbrush = pWD->stdbarbrush;
									f = (float)pli->lDataValue / 
										(float)pci->lMaxValue;
									f += 1.0;
									f = log (f) / log (2.0);
									rc.right = rc.left + (int)(f * (float)cx);
								}
							}
							else 
								rc.right = rc.left;

							rc.top++;
							rc.left++;

							if (rc.right > rc.left) {
								SetBkColor (hdc, pWD->barcolor);
								FillRect (hdc, &rc, barbrush); 

								rc.top--;
								rc.left--;

								// hilight pen already selected 
								MoveToEx (hdc, rc.right, rc.top, NULL);
								LineTo (hdc, rc.left, rc.top);
								LineTo (hdc, rc.left,rc.bottom);

								SelectObject (hdc, pWD->shadowpen);
								LineTo (hdc, rc.right, rc.bottom);
								LineTo (hdc, rc.right, rc.top);
							}

							SelectObject (hdc, oldbrush);
							SelectObject (hdc, oldpen);
							break;

						case TLCFMT_IMAGE :
							if (pci->cx >= (BITMAPWIDTH)) {
								int i = 0;
								if (pli->state & TLIS_CLICKED) i = 1;
								ImageList_Draw (pWD->hImageList, 
									(pli->lDataValue + i), hdc, 
									ix + pci->cxImageOffset, 
									p->hiliteRect.top+iBMPos,
									ILD_TRANSPARENT);
							}
							break;

						default :
							wsprintf (sz256, "%i", pli->lDataValue);
							SetBkColor (hdc, pWD->textbgcolor);
							TextOut (hdc, ix+6, iy, sz256, lstrlen(sz256));
							break;
						}
						break; 
					}
				}
				ix += pci->cx;

				pci = pci->nextItem;
				pli = pli->nextItem;
			}

//  increment y coordinate to next row	
			*y += pWD->iRowHeight;
	
//  if there is space, paint button and lines
			if (pWD->iFirstColumnWidth >= p->buttonRect.right) {
				if (p->cChildren) {
					oldpen = SelectObject (hdc, pWD->buttonpen);
					MoveToEx (hdc, p->buttonRect.left+HBCENT-BBOX, 
								p->buttonRect.top+iBCent-BBOX, NULL);
					LineTo (hdc, p->buttonRect.left+HBCENT+BBOX, 
								p->buttonRect.top+iBCent-BBOX);
					LineTo (hdc, p->buttonRect.left+HBCENT+BBOX, 
								p->buttonRect.top+iBCent+BBOX);
					LineTo (hdc, p->buttonRect.left+HBCENT-BBOX, 
								p->buttonRect.top+iBCent+BBOX);
					LineTo (hdc, p->buttonRect.left+HBCENT-BBOX, 
								p->buttonRect.top+iBCent-BBOX);
					SelectObject (hdc, oldpen);
					if (p->state & TLIS_EXPANDED) {
						oldpen = SelectObject (hdc, pWD->unseltextpen);
						MoveToEx (hdc, p->buttonRect.left+HBCENT-BBAR, 
									p->buttonRect.top+iBCent, NULL);
						LineTo (hdc, p->buttonRect.left+HBCENT+BBAR+1, 
									p->buttonRect.top+iBCent);
						SelectObject (hdc, oldpen);
						// paint children
						PaintItems (hdc, pWD, item, y, 
									indent+INCREMENTINDENT, p->childItem);
					}
					else {
						oldpen = SelectObject (hdc, pWD->unseltextpen);
						MoveToEx (hdc, p->buttonRect.left+HBCENT-BBAR, 
									p->buttonRect.top+iBCent, NULL);
						LineTo (hdc, p->buttonRect.left+HBCENT+BBAR+1, 
									p->buttonRect.top+iBCent);
						MoveToEx (hdc, p->buttonRect.left+HBCENT, 
									p->buttonRect.top+iBCent-BBAR, NULL);
						LineTo (hdc, p->buttonRect.left+HBCENT, 
									p->buttonRect.top+iBCent+BBAR+1);
						SelectObject (hdc, oldpen);
					}
					if ((pWD->style & TLS_HASLINES) && (p->parentItem ||
						(!p->parentItem && (pWD->style & TLS_LINESATROOT)))) {
						DottedStruct.dwDottedColor = pWD->linecolor;
						if (p->prevItem || p->parentItem) {
							if (p->prevItem) 
								LineDDA (xoffset+HBCENT, 
										 p->prevItem->selectRect.bottom,
										 xoffset+HBCENT, 
										 p->buttonRect.top+iBCent-BBOX,
										 (LINEDDAPROC)DottedLineProc, 
										 (LPARAM)&DottedStruct);
							else 
								LineDDA (xoffset+HBCENT, p->buttonRect.top,
										 xoffset+HBCENT, 
										 p->buttonRect.top+iBCent-BBOX,
										 (LINEDDAPROC)DottedLineProc, 
										 (LPARAM)&DottedStruct);
						}
						if (p->nextItem) {
							LineDDA (xoffset+HBCENT, 
									 p->buttonRect.top+iBCent+BBOX,
									 xoffset+HBCENT, *y +1,
									 (LINEDDAPROC)DottedLineProc, 
									 (LPARAM)&DottedStruct);
						}
						LineDDA (xoffset+HBCENT+BBOX, 
								 p->buttonRect.top+iBCent,
								 xoffset+BUTTONWIDTH, 
								 p->buttonRect.top+iBCent,
								 (LINEDDAPROC)DottedLineProc, 
								 (LPARAM)&DottedStruct);
					}
				}
	
//  if item has no children, only paint lines
				else {
					if ((pWD->style & TLS_HASLINES) && (p->parentItem ||
					    (!p->parentItem && (pWD->style & TLS_LINESATROOT)))) {
						DottedStruct.dwDottedColor = pWD->linecolor;
						if (p->prevItem) {
							ix = xoffset+HBCENT;
							iy = p->prevItem->selectRect.bottom;
						}
						else {
							ix = xoffset+HBCENT;
							iy = p->buttonRect.top;
						}
						if (p->nextItem) {
							LineDDA (ix, iy, xoffset+HBCENT, *y +1, 
									 (LINEDDAPROC)DottedLineProc, 
									 (LPARAM)&DottedStruct);
						}
						else LineDDA (ix, iy, xoffset+HBCENT, 
									  p->buttonRect.top+iBCent,
									  (LINEDDAPROC)DottedLineProc, 
									  (LPARAM)&DottedStruct);
						LineDDA (xoffset+HBCENT, p->buttonRect.top+iBCent,
								 xoffset+BUTTONWIDTH, 
								 p->buttonRect.top+iBCent,
								 (LINEDDAPROC)DottedLineProc, 
								 (LPARAM)&DottedStruct);
					}
				}
			}

//  if no space, we still need to paint children
			else {
				if (p->cChildren) {
					if (p->state & TLIS_EXPANDED) {
						PaintItems (hdc, pWD, item, y, 
							indent+INCREMENTINDENT, p->childItem);
					}
				}
			}

		}
		else {		// ((*item) <= pWD->iFirstRow) {
			p->selectRect.left = 0;
			p->selectRect.right = 0;
			p->selectRect.top = 0;
			p->selectRect.bottom = 0;

			p->buttonRect.left = 0;
			p->buttonRect.right = 0;
			p->buttonRect.top = 0;
			p->buttonRect.bottom = 0;

			if (p->cChildren) {
				if (p->state & TLIS_EXPANDED) {
					PaintItems (hdc, pWD, item, y, 
						indent+INCREMENTINDENT, p->childItem);
				}
			}
		}
//  step to next tree item
		p = p->nextItem;
	}

//	set next item to unusual value to flag end of list
	if (p) {
		p->selectRect.left = 0;
		p->selectRect.right = -1;
		p->selectRect.top = 0;
		p->selectRect.bottom = -1;

		p->buttonRect.left = 0;
		p->buttonRect.right = -1;
		p->buttonRect.top = 0;
		p->buttonRect.bottom = -1;
	}
}


//----------------------------------------------------|
//  Calculations before painting -- called in response to WM_PAINT

void TLPrePaint (TLWndData* pWD) {

	SCROLLINFO si;
	INT iY;

	pWD->iBCent = (pWD->iRowHeight / 2) +2;
	pWD->iBMPos = pWD->iBCent - (BITMAPHEIGHT/2) -1; 

	pWD->tlInval = pWD->tlRect;
	pWD->tlInval.top += pWD->iRowHeight+1;

	pWD->iExpandedRows = CountRows (pWD->rootItem);
	pWD->iMaxRows = (pWD->tlInval.bottom-pWD->tlInval.top-
					(pWD->iRowHeight/2)) / pWD->iRowHeight;
	if (pWD->iMaxRows <= pWD->iExpandedRows) pWD->tlInval.right = 
		pWD->tlRect.right - GetSystemMetrics (SM_CXVSCROLL);
	else {
		pWD->iFirstRow = 0;
		pWD->tlInval.right = pWD->tlRect.right;
	}

	iY = pWD->tlInval.right - pWD->tlInval.left;
	if (pWD->iTotalWidth > iY) pWD->tlInval.bottom = 
		pWD->tlRect.bottom - GetSystemMetrics (SM_CYHSCROLL);
	else pWD->tlInval.bottom = pWD->tlRect.bottom;

	pWD->iMaxRows = (pWD->tlInval.bottom-pWD->tlInval.top-
					(pWD->iRowHeight/2)) / pWD->iRowHeight;
	si.cbSize = sizeof (SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nMin = 0;
	si.nMax = pWD->iExpandedRows;
	si.nPage = pWD->iMaxRows;
	si.nPos = pWD->iFirstRow;
	SetScrollInfo (pWD->hWnd, SB_VERT, &si, TRUE);

	si.nMax = pWD->iTotalWidth;
	si.nPage = pWD->tlInval.right - pWD->tlInval.left;
	si.nPos = pWD->iHorizontalPos;
	SetScrollInfo (pWD->hWnd, SB_HORZ, &si, TRUE);
}


//----------------------------------------------------|
//  Paint a tree list -- called in response to WM_PAINT

void TLPaint (TLWndData* pWD, HDC hdc) {

	TLTreeItem* p;
	HFONT hOldFont;
	int y, item;

	hOldFont = SelectObject (hdc, pWD->hFont);

	p = pWD->rootItem;
	y = pWD->tlInval.top;
	item = 0;
	PaintItems (hdc, pWD, &item, &y, 0, p);

	SelectObject (hdc, hOldFont);
}


//----------------------------------------------------|
//  create mem dc and pass it to routine to render treelist

void TLDeleteMemDC (TLWndData* pWD) {
	if (pWD->hdcMem) {
		SelectObject (pWD->hdcMem, pWD->hbmOld);
		DeleteObject (pWD->hbmMem);
		DeleteDC (pWD->hdcMem);
		pWD->hdcMem = NULL;
	}
}


//----------------------------------------------------|
//  create mem dc and pass it to routine to render treelist

void TLMemPaint (TLWndData* pWD, HDC hdc) {
	RECT rc;

	GetClientRect (pWD->hWnd, &rc);

	if (!pWD->hdcMem) {
		pWD->hdcMem = CreateCompatibleDC (hdc);
		pWD->hbmMem = CreateCompatibleBitmap (hdc, rc.right-rc.left, 
												rc.bottom-rc.top);
		pWD->hbmOld = SelectObject (pWD->hdcMem, pWD->hbmMem);
		SetBkMode (pWD->hdcMem, TRANSPARENT);
	}

	FillRect (pWD->hdcMem, &rc, pWD->unselbgbrush);
	TLPaint (pWD, pWD->hdcMem);
    
	BitBlt (hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
			pWD->hdcMem, 0, 0, SRCCOPY);
}
  

