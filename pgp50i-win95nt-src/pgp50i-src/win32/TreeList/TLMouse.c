/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:TLMouse.c - handle tree list mouse events
//
//	$Id: TLMouse.c,v 1.6 1997/05/19 14:36:25 pbj Exp $
//

#include <windows.h>
#include "TLIntern.h"

#define TLMF_DBLCLICK	0x0001
#define TLMF_CONTROL	0x0002
#define TLMF_SHIFT		0x0004
#define TLMF_RBUTTON	0x0008

#define TLK_NONE		0x0000
#define TLK_BUTTON		0x0001
#define TLK_SELECT		0x0002


//----------------------------------------------------|
//  Search for object containg given point

TLTreeItem* PointSearch (TLTreeItem* pStart, POINT mpt, int* iType) {
	TLTreeItem* p;
	TLTreeItem* pp;
	
	p = pStart;
	while (p) {
		if (PtInRect (&p->buttonRect, mpt)) {
			*iType = TLK_BUTTON;
			return p;
		}
		if (PtInRect (&p->selectRect, mpt)) {
			*iType = TLK_SELECT;
			return p;
		}
		if (p->state & TLIS_EXPANDED) {
			pp = PointSearch (p->childItem, mpt, iType);
			if (pp) return pp;
		}
		if (p->buttonRect.right < p->buttonRect.left) {
			*iType = TLK_NONE;
			return NULL;
		}
		p = p->nextItem;
	}
	*iType = TLK_NONE;
	return NULL;
}


//----------------------------------------------------|
//  Select range of lines

BOOL TLSelectRange (TLWndData* pWD, TLTreeItem* pFirst, TLTreeItem* pSel) {

	TLTreeItem* p;
	TLTreeItem* pLast;
	BOOL bSel;

	p = pFirst;
	bSel = FALSE;

	if (pSel == pWD->firstSelectItem) {
		pSel->state |= TLIS_SELECTED;
		pWD->iNumberSelected = 1;
		TLInvalidateItem (pWD, pSel);
		return TRUE;
	}

	while (p && !bSel) {
		if ((p->childItem) && (p->state & TLIS_EXPANDED)) {
			if (TLSelectRange (pWD, p->childItem, pSel)) return TRUE;
		}
		if (p == pSel) {
			bSel = TRUE;
			pLast = pWD->firstSelectItem;
			p->state |= TLIS_SELECTED;
			(pWD->iNumberSelected)++;
			TLInvalidateItem (pWD, p);
		}
		else if (p == pWD->firstSelectItem) {
			bSel = TRUE;
			pLast = pSel;
			p->state |= TLIS_SELECTED;
			(pWD->iNumberSelected)++;
			TLInvalidateItem (pWD, p);
		}
		p = p->nextItem;
	}

	if (bSel) {
		while (p && bSel) {
			if (p == pLast) {
				bSel = FALSE;
			}
			p->state |= TLIS_SELECTED;
			(pWD->iNumberSelected)++;
			TLInvalidateItem (pWD, p);
			p = p->nextItem;
		}
		return TRUE;
	}
	return FALSE;
}	
	
//----------------------------------------------------|
//  Handle left mouse button down event

void TLLeftButtonDown (TLWndData* pWD, WPARAM wParam, LPARAM lParam) {
	int iHitType;
	TLTreeItem* pHit;
	POINT mpt;
	BOOL bControl, bShift;

	mpt.x = LOWORD (lParam);
	mpt.y = HIWORD (lParam);

	if (wParam & MK_CONTROL) bControl = TRUE;
	else bControl = FALSE;
	if (GetKeyState (VK_SHIFT) & 0x8000) bShift = TRUE;
	else bShift = FALSE;	

	pWD->nmTreeList.flags = 0;
	pWD->nmTreeList.itemNew.hItem = NULL;
	pWD->nmTreeList.itemNew.state = 0;
	pWD->nmTreeList.itemOld.hItem = NULL;

	pHit = PointSearch (pWD->rootItem, mpt, &iHitType);

	switch (iHitType) {
	case TLK_NONE :
		if (bControl || bShift) return;
		TLDeselectTree (pWD, pWD->rootItem, NULL);
		pWD->firstSelectItem = NULL;
		pWD->nmTreeList.flags |= TLC_BYMOUSE;
		if (pWD->focusItem) {
			pHit = pWD->focusItem;
			pWD->focusItem = NULL;
			TLInvalidateItem (pWD, pHit);
		}
		break;

	case TLK_BUTTON :
		if (pHit->state & TLIS_EXPANDED) {
			pHit->state &= ~TLIS_EXPANDED;
		}
		else {
			pHit->state |= TLIS_EXPANDED;
			if (bControl) TLExpandTree (pHit->childItem);
			TLExpandAutoScroll (pWD, pHit);
		}
		InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
		break;

	case TLK_SELECT :
		if (!pWD->firstSelectItem) pWD->firstSelectItem = pHit;
		if (!bControl && !bShift) pWD->firstSelectItem = pHit;
		if ((pHit->parentItem == pWD->firstSelectItem->parentItem) ||
			((pWD->style & TLS_PROMISCUOUS) && !bShift)) {
			if (bShift) {
				TLDeselectTree (pWD, pWD->rootItem, NULL);
				pWD->iNumberSelected = 0;
				TLSelectRange (pWD, pWD->rootItem, pHit);
				pWD->nmTreeList.itemNew.hItem = (HTLITEM)pHit;
				pWD->nmTreeList.itemNew.state = pHit->state;
				pWD->nmTreeList.flags |= TLC_BYMOUSE;
			}
			else {
				if (!(pHit->state & TLIS_SELECTED)) {
					if (!bControl) {
						if (!bControl && !bShift) {
							TLDeselectTree (pWD, pWD->rootItem, pHit);
							pWD->iNumberSelected = 0;
						}
						pHit->state |= TLIS_SELECTED;
						(pWD->iNumberSelected)++;
						if (TLSelectAutoScroll (pWD, pHit))
							InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
						pWD->nmTreeList.itemNew.hItem = (HTLITEM)pHit;
						pWD->nmTreeList.itemNew.state = pHit->state;
						pWD->nmTreeList.flags |= TLC_BYMOUSE;
					}
				}
			}
			if (pWD->focusItem != pHit) {
				if (pWD->focusItem) {
					pWD->focusItem->state &= ~TLIS_FOCUSED;
					pWD->nmTreeList.itemOld.hItem = (HTLITEM)pWD->focusItem;
					pWD->nmTreeList.itemOld.state = pWD->focusItem->state;
					TLInvalidateItem (pWD, pWD->focusItem);
				}
				pHit->state |= TLIS_FOCUSED;
				pWD->focusItem = pHit;
			}
			TLInvalidateItem (pWD, pHit);
		}
		break;
	}
	
	if (pWD->nmTreeList.flags) {
		if (pWD->iNumberSelected > 1) pWD->nmTreeList.flags |= TLC_MULTIPLE;
		pWD->nmTreeList.hdr.code = TLN_SELCHANGED;
		pWD->nmTreeList.ptDrag = mpt;
		SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id,
						(LPARAM)&(pWD->nmTreeList));
	}
}


//----------------------------------------------------|
//  Handle left mouse button up event

void TLLeftButtonUp (TLWndData* pWD, WPARAM wParam, LPARAM lParam) {
	int iHitType;
	TLTreeItem* pHit;
	POINT mpt;
	BOOL bControl, bShift;

	mpt.x = LOWORD (lParam);
	mpt.y = HIWORD (lParam);

	if (wParam & MK_CONTROL) bControl = TRUE;
	else bControl = FALSE;
	if (GetKeyState (VK_SHIFT) & 0x8000) bShift = TRUE;
	else bShift = FALSE;	

	pWD->nmTreeList.flags = 0;
	pWD->nmTreeList.itemNew.hItem = NULL;
	pWD->nmTreeList.itemOld.hItem = NULL;

	pHit = PointSearch (pWD->rootItem, mpt, &iHitType);

	switch (iHitType) {
	case TLK_NONE :
		break;

	case TLK_BUTTON :
		break;

	case TLK_SELECT :
		if (!pWD->firstSelectItem) pWD->firstSelectItem = pHit;
		if (!bControl && !bShift) pWD->firstSelectItem = pHit;
		if ((pHit->parentItem == pWD->firstSelectItem->parentItem) ||
			((pWD->style & TLS_PROMISCUOUS) && !bShift)) {
			if (!bShift) {
				if (pHit->state & TLIS_SELECTED) {
					if (bControl) {
						pHit->state &= ~TLIS_SELECTED;
						(pWD->iNumberSelected)--;
						TLInvalidateItem (pWD, pHit);
						if (pWD->iNumberSelected <= 0) {
							pWD->nmTreeList.flags |= TLC_BYMOUSE;
						}
					}
					else {
						TLDeselectTree (pWD, pWD->rootItem, pHit);
						pHit->state |= TLIS_SELECTED;
						pWD->iNumberSelected = 1;
						if (TLSelectAutoScroll (pWD, pHit))
							InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
						pWD->nmTreeList.itemNew.hItem = (HTLITEM)pHit;
						pWD->nmTreeList.itemNew.state = pHit->state;
						pWD->nmTreeList.flags |= TLC_BYMOUSE;
					}
				}
				else {
					if (bControl) {
						pHit->state |= TLIS_SELECTED;
						(pWD->iNumberSelected)++;
						TLInvalidateItem (pWD, pHit);
						if (TLSelectAutoScroll (pWD, pHit))
							InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
						pWD->nmTreeList.itemNew.hItem = (HTLITEM)pHit;
						pWD->nmTreeList.itemNew.state = pHit->state;
						pWD->nmTreeList.flags |= TLC_BYMOUSE;
					}
				}
			}
			if (bShift || bControl) {
				if (pWD->focusItem != pHit) {
					if (pWD->focusItem) {
						pWD->focusItem->state &= ~TLIS_FOCUSED;
						pWD->nmTreeList.itemOld.hItem =
							(HTLITEM)pWD->focusItem;
						pWD->nmTreeList.itemOld.state =
							pWD->focusItem->state;
						TLInvalidateItem (pWD, pWD->focusItem);
					}
					pHit->state |= TLIS_FOCUSED;
					pWD->focusItem = pHit;
					TLInvalidateItem (pWD, pHit);
				}
			}
		}
		break;
	}
	
	if (pWD->nmTreeList.flags) {
		if (pWD->iNumberSelected > 1) pWD->nmTreeList.flags |= TLC_MULTIPLE;
		pWD->nmTreeList.hdr.code = TLN_SELCHANGED;
		pWD->nmTreeList.ptDrag = mpt;
		SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id,
						(LPARAM)&(pWD->nmTreeList));
	}
}


//----------------------------------------------------|
//  Handle left mouse button double click event

void TLLeftButtonDblClk (TLWndData* pWD, WPARAM wParam, LPARAM lParam) {
	int iHitType;
	TLTreeItem* pHit;
	POINT mpt;
	BOOL bControl;

	mpt.x = LOWORD (lParam);
	mpt.y = HIWORD (lParam);

	if (wParam & MK_CONTROL) bControl = TRUE;
	else bControl = FALSE;

	pHit = PointSearch (pWD->rootItem, mpt, &iHitType);

	switch (iHitType) {
	case TLK_NONE :
		break;

	case TLK_BUTTON :
		if (pHit->state & TLIS_EXPANDED) {
			pHit->state &= ~TLIS_EXPANDED;
		}
		else {
			pHit->state |= TLIS_EXPANDED;
			if (bControl) TLExpandTree (pHit->childItem);
			TLExpandAutoScroll (pWD, pHit);
		}
		InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
		break;

	case TLK_SELECT :
		if (pHit->state & TLIS_EXPANDED) {
			pHit->state &= ~TLIS_EXPANDED;
		}
		else {
			pHit->state |= TLIS_EXPANDED;
			if (bControl) TLExpandTree (pHit->childItem);
			TLExpandAutoScroll (pWD, pHit);
		}
		InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
		break;
	}
}


//----------------------------------------------------|
//  Handle right mouse button down event

void TLRightButtonDown (TLWndData* pWD, WPARAM wParam, LPARAM lParam) {
	int iHitType;
	TLTreeItem* pHit;
	POINT mpt;

	if (wParam & MK_CONTROL) return;
	if (GetKeyState (VK_SHIFT) & 0x8000) return;

	mpt.x = LOWORD (lParam);
	mpt.y = HIWORD (lParam);

	pWD->nmTreeList.flags = 0;
	pWD->nmTreeList.itemNew.hItem = NULL;
	pWD->nmTreeList.itemOld.hItem = NULL;

	pHit = PointSearch (pWD->rootItem, mpt, &iHitType);

	switch (iHitType) {
	case TLK_NONE :
		TLDeselectTree (pWD, pWD->rootItem, NULL);
		pWD->nmTreeList.flags |= TLC_BYMOUSE;
		break;

	case TLK_BUTTON :
		break;

	case TLK_SELECT :
		if (!(pHit->state & TLIS_SELECTED)) {
			TLDeselectTree (pWD, pWD->rootItem, pHit);
			pHit->state |= TLIS_SELECTED;
			pWD->iNumberSelected = 1;
			if (TLSelectAutoScroll (pWD, pHit))
				InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
			pWD->nmTreeList.itemNew.hItem = (HTLITEM)pHit;
			pWD->nmTreeList.itemNew.state = pHit->state;
			pWD->nmTreeList.flags |= TLC_BYMOUSE;
		}
		if (pWD->focusItem) {
			pWD->focusItem->state &= ~TLIS_FOCUSED;
			pWD->nmTreeList.itemOld.hItem = (HTLITEM)pWD->focusItem;
			pWD->nmTreeList.itemOld.state = pWD->focusItem->state;
			TLInvalidateItem (pWD, pWD->focusItem);
		}
		pHit->state |= TLIS_FOCUSED;
		pWD->focusItem = pHit;
		TLInvalidateItem (pWD, pHit);
		break;
	}
	
	if (pWD->nmTreeList.flags) {
		if (pWD->iNumberSelected > 1) pWD->nmTreeList.flags |= TLC_MULTIPLE;
		pWD->nmTreeList.hdr.code = TLN_SELCHANGED;
		pWD->nmTreeList.ptDrag = mpt;
		SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id,
						(LPARAM)&(pWD->nmTreeList));
	}
}


//----------------------------------------------------|
//  Handle column resizing

void ColumnResize (TLWndData* pWD, HD_NOTIFY* hdn) {
	TLColumnItem* pci;
	int i, scx, left;
	HD_ITEM hdi;
	RECT rcInval;

	if (hdn->iButton == 0) {
		rcInval = pWD->tlInval;
		left = pWD->iFirstColumnWidth;
		if (hdn->iItem == 0) {
			scx = hdn->pitem->cxy - pWD->iFirstColumnWidth;
			pWD->iFirstColumnWidth = hdn->pitem->cxy;
			rcInval.right = pWD->iFirstColumnWidth + scx +1;
			if (scx < 0) rcInval.left = rcInval.right - 24;
			else rcInval.left = pWD->iFirstColumnWidth - 24;
		}
		else {
			pci = pWD->columnItem;
			left += pci->cx;
			rcInval.left = pWD->iFirstColumnWidth;
			for (i=1; i<hdn->iItem; i++) {
				rcInval.left += pci->cx;
				pci = pci->nextItem;
				left += pci->cx;
			}
			scx = hdn->pitem->cxy - pci->cx;
			pci->cx = hdn->pitem->cxy;
			rcInval.right = left + scx +1;
		}

		pci = pWD->columnItem;
		pWD->iTotalWidth = pWD->iFirstColumnWidth;
		for (i=1; i<pWD->iNumberColumns; i++) {
			pWD->iTotalWidth += pci->cx;
			pci = pci->nextItem;
		}

		hdi.mask = HDI_WIDTH;
		hdi.cxy = hdn->pitem->cxy;
		Header_SetItem (pWD->hWndHeader, hdn->iItem, &hdi);
		rcInval.left -= pWD->iHorizontalPos;
		rcInval.right -= pWD->iHorizontalPos;
		InvalidateRect (pWD->hWnd, &pWD->tlInval, FALSE);
		if ((scx < 0) && (pWD->iHorizontalPos > 0)) {
			i = pWD->iHorizontalPos + scx;
			if (i < 0) i = 0;
			SendMessage (pWD->hWnd, WM_HSCROLL,
				MAKEWPARAM (SB_THUMBPOSITION, i), 0);
			InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
		}
	}
}


//----------------------------------------------------|
//  Handle header notifications

void TLHeaderNotification (TLWndData* pWD, HD_NOTIFY* hdn) {

	switch (hdn->hdr.code) {
	case HDN_ENDTRACK :
	case HDN_TRACK :
		if (hdn->iButton == 0) {
			ColumnResize (pWD, hdn);
		}
		break;

	case HDN_ITEMCLICK :
		if (hdn->iButton == 0) {
			pWD->nmTreeList.hdr.code = TLN_HEADERCLICKED;
			pWD->nmTreeList.index = hdn->iItem;
			SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id,
							(LPARAM)&(pWD->nmTreeList));
		}
		break;
	}
}


//----------------------------------------------------|
//  Handle horizontal scrolling

void TLHScroll (TLWndData* pWD, int code, int pos, HWND hWndBar) {
	int pagesize;
	int newpos;
	int scrollamount;

	pagesize = pWD->tlInval.right - pWD->tlInval.left;
	newpos = pWD->iHorizontalPos;

	switch (code) {
	case SB_TOP :
		newpos = 0;
		break;

	case SB_BOTTOM :
		newpos = (pWD->iTotalWidth-pagesize);
		break;

	case SB_PAGELEFT :
		newpos -= pagesize;
		if (newpos < 0) newpos = 0;
		break;

	case SB_PAGERIGHT :
		newpos += pagesize;
		if (newpos > (pWD->iTotalWidth-pagesize))
			newpos = (pWD->iTotalWidth-pagesize);
		break;

	case SB_LINELEFT :
		newpos-=3;
		if (newpos < 0) newpos = 0;
		break;

	case SB_LINERIGHT :
		newpos+=3;
		if (newpos > (pWD->iTotalWidth-pagesize))
			newpos = (pWD->iTotalWidth-pagesize);
		break;

	case SB_THUMBPOSITION :
	case SB_THUMBTRACK :
		newpos = pos;
		break;

	default :
		return;
	}
	scrollamount = pWD->iHorizontalPos - newpos;
	pWD->iHorizontalPos = newpos;
	if (scrollamount) {
		ScrollWindow (pWD->hWnd, scrollamount, 0, NULL, NULL);
		UpdateWindow (pWD->hWnd);
	}
}


//----------------------------------------------------|
//  Handle window resizing

void TLResize (TLWndData* pWD, WINDOWPOS* pWP) {
	int dx, dy;
	
	dx = pWD->tlRect.right - pWP->cx;
	if ((dx < 0) && (pWD->iHorizontalPos > 0)) {
		dx += pWD->iHorizontalPos;
		if (dx < 0) dx = 0;
		SendMessage (pWD->hWnd, WM_HSCROLL,
						MAKEWPARAM (SB_THUMBPOSITION, dx), 0);
	}
	pWD->tlRect.right = pWP->cx;
		
	dy = pWD->tlRect.bottom - pWP->cy;
	if ((dy < 0) && (pWD->iFirstRow > 0)) {
		dy += (pWD->iFirstRow * pWD->iRowHeight);
		dy /= pWD->iRowHeight;
		if (dy < 0) dy = 0;
		SendMessage (pWD->hWnd, WM_VSCROLL,
						MAKEWPARAM (SB_THUMBPOSITION, dy), 0);
	}
	pWD->tlRect.bottom = pWP->cy;

	TLDeleteMemDC (pWD);

	pWD->tlInval = pWD->tlRect;
	pWD->tlInval.top += pWD->iRowHeight+1;

	InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
}


//----------------------------------------------------|
//  Handle vertical scrolling

void TLVScroll (TLWndData* pWD, int code, int pos, HWND hWndBar) {
	int pagesize;
	int scrollamount;
	int newrow;

	pagesize = pWD->tlInval.right - pWD->tlInval.left;
	newrow = pWD->iFirstRow;

	switch (code) {
	case SB_TOP :
		newrow = 0;
		break;

	case SB_BOTTOM :
		newrow = (pWD->iExpandedRows-pWD->iMaxRows);
		break;

	case SB_PAGEUP :
		newrow -= pWD->iMaxRows;
		if (newrow < 0) newrow = 0;
		break;

	case SB_PAGEDOWN :
		newrow += pWD->iMaxRows;
		if (newrow > (pWD->iExpandedRows-pWD->iMaxRows))
			newrow = (pWD->iExpandedRows-pWD->iMaxRows) +1;
		break;

	case SB_LINEUP :
		newrow--;
		if (newrow < 0) newrow = 0;
		break;

	case SB_LINEDOWN :
		newrow++;
		if (newrow > (pWD->iExpandedRows-pWD->iMaxRows))
			newrow = (pWD->iExpandedRows-pWD->iMaxRows) +1;
		break;

	case SB_THUMBPOSITION :
	case SB_THUMBTRACK :
		newrow = pos;
		break;

	default :
		return;
	}

	scrollamount = pWD->iFirstRow - newrow;
	pWD->iFirstRow = newrow;

	if (scrollamount) {
		InvalidateRect (pWD->hWnd, &pWD->tlInval, TRUE);
	}
}
