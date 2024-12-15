//:TLMsgProc.c - TreeList message processing routine
//
//	$Id: TLMsgPrc.c,v 1.10 1997/09/16 16:59:57 pbj Exp $
//

#include "TLintern.h"
#include <math.h>

#define KEYTIMEOUT		1000L	//millisec to wait for subsequent keypresses
								//	while autoscrolling 
#define KEYBOARDTIMERID	1111L	//arbitrary numbers used as timer identifiers


//----------------------------------------------------|
//  Message processing function for TreeList class

LRESULT WINAPI TreeListMsgProc (HWND hWnd, UINT msg, WPARAM wParam, 
									LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;

	TLWndData* pWD = TreeList_GetPtr(hWnd);

    if (pWD == NULL) {
        if (msg == WM_NCCREATE) {
        	pWD = (TLWndData*)TLAlloc (sizeof(TLWndData));
        	if (pWD == NULL) return 0L;
        	TreeList_SetPtr (hWnd, pWD);
        }
        else {
        	return DefWindowProc (hWnd, msg, wParam, lParam);
        }
    }
	
    if (msg == WM_NCDESTROY) {
		TLFree (pWD);
        pWD = NULL;
        TreeList_SetPtr (hWnd, NULL);
    }

	switch (msg) {

	case WM_CREATE :
		pWD->hWnd = hWnd;
		pWD->hWndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
		pWD->hdcMem = NULL;
		pWD->id = (LONG)((LPCREATESTRUCT)lParam)->hMenu; 
		pWD->style = (LONG)((LPCREATESTRUCT)lParam)->style;
		pWD->bTreeFocused = FALSE;
		pWD->rootItem = NULL;
		pWD->columnItem = NULL;
		pWD->hImageList = NULL;
		pWD->hCursorNormal = GetCursor ();
		pWD->hCursorDragging = 
			LoadCursor (GetModuleHandle (NULL), "TreeListDraggingCursor");
		if (!(pWD->hCursorDragging)) pWD->hCursorDragging = GetCursor ();
		pWD->hCursorNoDrag = LoadCursor (NULL, IDC_NO);
		if (!(pWD->hCursorNoDrag)) pWD->hCursorNoDrag = GetCursor ();
		pWD->firstSelectItem = NULL;
		pWD->focusItem = NULL;
		pWD->iNumberColumns = 0;
		pWD->iHorizontalPos = 0;
		pWD->iFirstRow = 0;
		pWD->iNumberSelected = 0;
		pWD->clickedTreeItem = NULL;
		pWD->bDragging = FALSE;
		pWD->nmTreeList.hdr.hwndFrom = hWnd;

		TLInitPaint (pWD);

// create a vertical scroll bar
		GetClientRect (pWD->hWnd, &pWD->tlRect);

		pWD->hWndHeader = CreateWindowEx (0L, WC_HEADER, (LPCTSTR) NULL,
			WS_VISIBLE | WS_CHILD | HDS_HORZ | HDS_BUTTONS,
 			0, 0, 30000, pWD->iRowHeight+1,
			hWnd, (HMENU)IDC_TREELISTHEADER, 0, NULL);
		SendMessage (pWD->hWndHeader, WM_SETFONT, (WPARAM)pWD->hFont, FALSE);
		break;

	case WM_ERASEBKGND :
		return 1;

	case WM_KILLFOCUS :
		if (!pWD->bDragging) {
			pWD->bTreeFocused = FALSE;
			InvalidateRect (hWnd, NULL, FALSE);
		}
		return 0;

	case WM_SETFOCUS :
		pWD->bTreeFocused = TRUE;
		if (!pWD->focusItem) {
			if (pWD->rootItem) {
				pWD->focusItem = pWD->rootItem;
				pWD->focusItem->state |= TLIS_FOCUSED;
			}
		}
		else pWD->focusItem->state |= TLIS_FOCUSED;
		InvalidateRect (hWnd, NULL, FALSE);
		return 0;

	case WM_GETDLGCODE :
		return DLGC_WANTARROWS | DLGC_WANTCHARS;

	case WM_PAINT :
		TLPrePaint (pWD);
		hdc = BeginPaint (pWD->hWnd, &ps);
		TLMemPaint (pWD, hdc);
		EndPaint (pWD->hWnd, &ps);
		return 0;

	case TLM_DRAGOVER :
		pWD->bTreeFocused = TRUE;
		return (TLDragOver (pWD, lParam));

	case WM_LBUTTONDOWN :
		SetFocus (hWnd);
		TLEndDrag (pWD, lParam);
		pWD->bDragging = FALSE;
		TLLeftButtonDown (pWD, wParam, lParam);
		pWD->ptButtonDown.x = LOWORD (lParam);
		pWD->ptButtonDown.y = HIWORD (lParam);
		return 0;

	case WM_LBUTTONUP :
		if (pWD->bDragging) TLEndDrag (pWD, lParam);
		else TLLeftButtonUp (pWD, wParam, lParam);
		pWD->bDragging = FALSE;
		break;

	case WM_RBUTTONDOWN :
		SetFocus (hWnd);
		TLEndDrag (pWD, lParam);
		pWD->bDragging = FALSE;
		TLRightButtonDown (pWD, wParam, lParam);
		return 0;

	case WM_LBUTTONDBLCLK :
		SetFocus (hWnd);
		TLEndDrag (pWD, lParam);
		pWD->bDragging = FALSE;
		TLLeftButtonDblClk (pWD, wParam, lParam);
		return 0;

	case WM_RBUTTONDBLCLK :
		SetFocus (hWnd);
		break;

	case WM_MOUSEMOVE :
		TLMouseMove (pWD, wParam, lParam);
		return 0;

	case TLM_INSERTITEM :
		return ((LRESULT)TLInsertTreeItem (pWD, (LPTL_INSERTSTRUCT)lParam));

	case TLM_DELETEITEM :
		return ((LRESULT)TLDeleteTreeItem (pWD, (TL_TREEITEM*)lParam));

	case TLM_GETITEM :
		return ((LRESULT)TLGetTreeItem (pWD, (TL_TREEITEM*)lParam));

	case TLM_SETITEM :
		return ((LRESULT)TLSetTreeItem (pWD, (TL_TREEITEM*)lParam));

	case TLM_GETFIRSTITEM :
		return ((LRESULT)pWD->rootItem);

	case TLM_INSERTCOLUMN :
		return ((LRESULT)TLInsertColumn (pWD, wParam, (TL_COLUMN*)lParam));

	case TLM_SETCOLUMN :
		return ((LRESULT)TLSetColumn (pWD, wParam, (TL_COLUMN*)lParam));

	case TLM_DELETEALLCOLUMNS :
		return (TLDeleteAllColumns (pWD));

	case TLM_GETCOLUMNWIDTH :
		return ((LRESULT)TLGetColumnWidth (pWD, wParam));
		
	case TLM_SETIMAGELIST :
		pWD->hImageList = (HIMAGELIST)lParam;
		return 0;

	case TLM_SETLISTITEM :
		return ((LRESULT)TLSetListItem (pWD, (TL_LISTITEM*)lParam,
										(BOOL)wParam));

	case TLM_GETLISTITEM :
		return ((LRESULT)TLGetListItem (pWD, (TL_LISTITEM*)lParam));

	case TLM_EXPAND :
		return ((LRESULT)TLExpandTreeItem (pWD, (TL_TREEITEM*)lParam, 
												wParam));

	case TLM_SELECTITEM :
		return ((LRESULT)TLSelectTreeItem (pWD, (TL_TREEITEM*)lParam, 
												wParam));

	case TLM_SELECTCHILDREN :
		return ((LRESULT)TLSelectChildren (pWD, (TL_TREEITEM*)lParam));

	case TLM_ENSUREVISIBLE :
		return ((LRESULT)TLEnsureVisible (pWD, (TL_TREEITEM*)lParam));

	case TLM_ITERATESELECTED :
		return ((LRESULT)TLIterateSelected (pWD, lParam));

	case TLM_DELETETREE :
		if (wParam) TLDeleteTreeList (pWD->rootItem);
		pWD->rootItem = NULL;
		pWD->focusItem = NULL;
		pWD->firstSelectItem = NULL;
		pWD->iNumberSelected = 0;
		pWD->iFirstRow = 0;
		return TRUE;

	case WM_TIMER :
		KillTimer (hWnd, wParam);
		pWD->bStillTyping = FALSE;
		break;

	case WM_NOTIFY :
		switch (wParam) {
		case IDC_TREELISTHEADER :
			TLHeaderNotification (pWD, (HD_NOTIFY*)lParam);
			return FALSE;
		}
		break;
		
	case WM_HSCROLL :
		TLHScroll (pWD, LOWORD (wParam), HIWORD (wParam), (HWND)lParam);
		return 0;

	case WM_VSCROLL :
		TLVScroll (pWD, LOWORD (wParam), HIWORD (wParam), (HWND)lParam);
		return 0;

	case WM_WINDOWPOSCHANGED :
		TLResize (pWD, (WINDOWPOS*)lParam);
		return 0;

	case WM_SIZE :
	case WM_MOVE :
		return 0;

	case WM_KEYDOWN :
		TLKeyboardChar (pWD, wParam, lParam);
		break;

	case WM_SYSCOLORCHANGE :
		TLClosePaint (pWD);
		TLInitPaint (pWD);
		SetWindowPos (pWD->hWndHeader, 0, 0, 0, 30000, pWD->iRowHeight+1, 
						SWP_NOZORDER|SWP_NOMOVE);
		SendMessage (pWD->hWndHeader, WM_SETFONT, (WPARAM)pWD->hFont, FALSE);
		break;

	case WM_CONTEXTMENU :
		pWD->nmTreeList.hdr.code = TLN_CONTEXTMENU;
		pWD->nmTreeList.ptDrag.x = LOWORD (lParam);
		pWD->nmTreeList.ptDrag.y = HIWORD (lParam);
		SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id, 
							(LPARAM)&(pWD->nmTreeList));
		return 0;

	case WM_CHAR :
		if ((pWD->style & TLS_AUTOSCROLL) && ~(lParam & 0x80000000)) {
			TLSelectByKeyboard (pWD, wParam, pWD->bStillTyping);
			pWD->bStillTyping = TRUE;
			SetTimer (hWnd, KEYBOARDTIMERID, KEYTIMEOUT, NULL);
		}
		pWD->nmKeyDown.hdr.code = TLN_CHAR;
		pWD->nmKeyDown.wVKey = wParam;
		pWD->nmKeyDown.flags = lParam;
		SendMessage (pWD->hWndParent, WM_NOTIFY, pWD->id, 
							(LPARAM)&(pWD->nmKeyDown));
		break;

	case WM_CLOSE :
		TLDeleteMemDC (pWD);
		TLClosePaint (pWD);
		TLDeleteTreeList (pWD->rootItem);
		pWD->rootItem = NULL;
		TLDeleteAllColumns (pWD);
		pWD->columnItem = NULL;
		break;

	} 
	return (DefWindowProc (hWnd, msg, wParam, lParam));

}
