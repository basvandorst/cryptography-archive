//: TreeList internal include file
//
//	$Id: TLintern.h,v 1.17 1997/09/17 15:16:17 pbj Exp $
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "..\include\TreeList.h"

#define TEXTVERTOFF			1
#define NUMIMAGELISTS		2

#define IDC_TREELISTHEADER	100
#define IDC_SCROLLBAR		101

#define INCREMENTINDENT 16
#define BUTTONWIDTH 18
#define BUTTONHEIGHT 16
#define BITMAPWIDTH 16
#define BITMAPHEIGHT 16
#define HTEXTSHIFT 4
#define HBARSHIFT 6
#define VBARSHIFT 3
#define BARCOLUMNDIFF HBARSHIFT*2
#define HBCENT 9
#define BBOX 4
#define BBAR 2

// TreeList linked list list item
typedef struct _TLListItem {
	struct _TLListItem* nextItem;
	UINT state;
	LPSTR pszText;
	LONG lDataValue;
} TLListItem;

// TreeList linked list list item
typedef struct _TLColumnItem {
	struct _TLColumnItem* nextItem;
	INT iSubItem;
	UINT iDataType;
	UINT iDataFmt;
	LONG lMaxValue;
	INT cx;
	BOOL bMouseNotify;
	INT cxImageOffset;
} TLColumnItem;

// TreeList linked list tree item
typedef struct _TLTreeItem {
	struct _TLTreeItem* nextItem;
	struct _TLTreeItem* prevItem;
	struct _TLTreeItem* childItem;
	struct _TLTreeItem* parentItem;
	TLListItem* listItem;
	RECT selectRect;
	RECT hiliteRect;
	RECT buttonRect;
	UINT state;
	LPSTR pszText;
	INT iImage;
	INT iSelImage;
	INT cChildren;
	LPARAM lParam;
} TLTreeItem;

// TreeList window extra data.  Must include window handle field.
typedef struct _TLWndData {
	HWND hWnd;
	HWND hWndParent;
	HWND hWndHeader;
	HDC  hdcMem;
	HBITMAP hbmMem;
	HBITMAP hbmOld;
	HFONT hFont;
	HFONT hFontItalic;
	HFONT hFontBold;
	LONG id;
	UINT style;
	BOOL bTreeFocused;
	INT iNumberSelected;
	NM_TREELIST nmTreeList;
	TL_KEYDOWN nmKeyDown;
	HIMAGELIST hImageList;
	HCURSOR hCursorNormal;
	HCURSOR hCursorDragging;
	HCURSOR hCursorNoDrag;
	TLTreeItem* rootItem;
	TLTreeItem* focusItem;
	TLTreeItem* firstSelectItem;
	TLTreeItem* clickedTreeItem;
	TLListItem* clickedListItem;
	TLColumnItem* columnItem;
	INT iNumberColumns;
	INT iFirstColumnWidth;
	INT iTotalWidth;
	INT iHorizontalPos;
	INT iMaxRows;
	INT iFirstRow;
	INT iExpandedRows;
	INT iRowHeight;
	RECT tlRect;
	RECT tlInval;
	BOOL bDragging;
	BOOL bStillTyping;
	POINT ptButtonDown;
	HBRUSH selbgbrush;
	HBRUSH unselbgbrush;
	HBRUSH selfgbrush;
	HBRUSH unselfgbrush;
	HBRUSH stdbarbrush;
	HBRUSH barbgbrush;
	HBRUSH spcbarbrush;
	DWORD textbgcolor;
	DWORD barcolor;
	DWORD linecolor;
	DWORD selfocuscolor;
	DWORD unselfocuscolor;
	HBITMAP barbgbitmap;
	HPEN seltextpen;
	HPEN unseltextpen;
	HPEN buttonpen;
	HPEN hilightpen;
	HPEN shadowpen;
	INT iBCent;
	INT iBMPos;
} TLWndData;

#define TreeList_GetPtr(hwnd)  (TLWndData*)GetWindowLong((hwnd), 0)
#define TreeList_SetPtr(hwnd, pTL)  \
			(TLWndData*)SetWindowLong((hwnd), 0, (LONG)(pTL))

// function prototypes

// TLMisc.c
void* TLAlloc (long size);
void* TLReAlloc (void* p, long size);
void TLFree (void* p);
BOOL TLKeyboardChar (TLWndData* pWD, WPARAM ch, LPARAM lP);
BOOL TLIterateSelected (TLWndData* pWD, LPARAM lParam);
BOOL TLSelectByKeyboard (TLWndData* pWD, LONG c, BOOL bStillTyping);

// TLMouse.c
void TLLeftButtonDown (TLWndData* pWD, WPARAM wParam, LPARAM lParam);
void TLLeftButtonUp (TLWndData* pWD, WPARAM wParam, LPARAM lParam);
void TLRightButtonDown (TLWndData* pWD, WPARAM wParam, LPARAM lParam);
void TLLeftButtonDblClk (TLWndData* pWD, WPARAM wParam, LPARAM lParam);
void TLHeaderNotification (TLWndData* pWD, HD_NOTIFY* hdn);
void TLHScroll (TLWndData* pWD, int code, int pos, HWND hWndBar);
void TLVScroll (TLWndData* pWD, int code, int pos, HWND hWndBar);
void TLResize (TLWndData* pWD, WINDOWPOS* pWP);
BOOL TLSelectRange (TLWndData* pWD, TLTreeItem* pFirst, TLTreeItem* pSel);
void TLMouseMove (TLWndData* pWD, WPARAM wParam, LPARAM lParam);
BOOL TLDragOver (TLWndData* pWD, LPARAM lParam);
BOOL TLDeDragOverTree (TLWndData* pWD, TLTreeItem* p, TLTreeItem* pExcept);
void TLEndDrag (TLWndData* pWD, LPARAM lParam);

// TLMsgProc.c
LRESULT WINAPI TreeListMsgProc (HWND hWnd, UINT msg, WPARAM wParam, 
								LPARAM lParam);

// TLPaint.c
BOOL TLGetRow (TLTreeItem* root, TLTreeItem* p, int* row);
void TLInitPaint (TLWndData* pWD);
void TLClosePaint (TLWndData* pWD);
void TLPrePaint (TLWndData* pWD);
void TLPaint (TLWndData* pWD, HDC hdc);
void TLMemPaint (TLWndData* pWD, HDC hdc);
void TLExpandAutoScroll (TLWndData* pWD, TLTreeItem* p);
BOOL TLSelectAutoScroll (TLWndData* pWD, TLTreeItem* p);
void TLInvalidateItem (TLWndData* pWD, TLTreeItem* p);
void TLDeleteMemDC (TLWndData* pWD);

// TLTree.c
HTLITEM TLInsertTreeItem (TLWndData* pWD, LPTL_INSERTSTRUCT lpInsert);
LONG TLSetListItem (TLWndData* pWD, TL_LISTITEM* pli, BOOL bInvalidate);
LONG TLGetListItem (TLWndData* pWD, TL_LISTITEM* pli);
void TLDeleteTreeList (TLTreeItem* p);
BOOL TLDeleteAllColumns (TLWndData* pWD);
LONG TLInsertColumn (TLWndData* pWD, LONG index, TL_COLUMN* tlc);
LONG TLSetColumn (TLWndData* pWD, LONG index, TL_COLUMN* tlc);
LONG TLGetColumnWidth (TLWndData* pWD, LONG index);
BOOL TLGetTreeItem (TLWndData* pWD, TL_TREEITEM* lpti);
BOOL TLDeselectTree (TLWndData* pWD, TLTreeItem* p, TLTreeItem* pExcept);
BOOL TLExpandTreeItem (TLWndData* pWD, TL_TREEITEM* lpti, int flags);
BOOL TLSelectTreeItem (TLWndData* pWD, TL_TREEITEM* lpti, int flags);
BOOL TLSelectChildren (TLWndData* pWD, TL_TREEITEM* lpti);
BOOL TLSetTreeItem (TLWndData* pWD, TL_TREEITEM* lpti);
BOOL TLDeleteTreeItem (TLWndData* pWD, TL_TREEITEM* lpti);
void TLExpandTree (TLTreeItem* p);
BOOL TLEnsureVisible (TLWndData* pWD, TL_TREEITEM* lpti);
