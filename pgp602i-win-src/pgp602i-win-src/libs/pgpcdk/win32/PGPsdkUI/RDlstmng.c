/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDlstmng.c,v 1.2.10.1 1998/11/12 03:24:26 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


#define EDGEMARGIN 5

#if LISTBOX

void MoveList(LISTSTRUCT *ls,RECT *rc)
{
	int listwidth;
	int xpos;
	float textfloat,ratio;
	int index;

	listwidth=rc->right-rc->left;

	MoveWindow(ls->hwndlist,
		rc->left,rc->top+20,
		rc->right-rc->left,rc->bottom-rc->top-15,
		TRUE);

	xpos=rc->left;

    for(index=0;index<NUMCOLUMNS;index++)
    {
		ratio=ls->colratio[index];

		textfloat=(float)listwidth;
		ls->colwidth[index]=(int)(ratio*textfloat);
		
		MoveWindow(ls->hwndtext[index],
			xpos,rc->top,
			ls->colwidth[index],20,
			TRUE);
	
		xpos=xpos+ls->colwidth[index];
	}
}

BOOL InitList(HWND hwnd,
			  DWORD idc,
			  LISTSTRUCT *ls,
			  char **ColText,
			  float *ColRatio)
{
	int index;
	HFONT hFont;
	DWORD staticFlags;
	PRECGBL prg;

	prg=(PRECGBL)GetWindowLong(hwnd,GWL_USERDATA);

	hFont=prg->ds.hFont;

	ls->hwnd=hwnd;
	ls->colratio=ColRatio;

    ls->hwndlist = CreateWindow ("listbox", NULL,
                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
					  WS_BORDER| LBS_OWNERDRAWFIXED |
					  LBS_SORT | LBS_EXTENDEDSEL|WS_TABSTOP,
                      0,0,0,0,
                      hwnd, (HMENU)idc,
                      (HINSTANCE) g_hinst,
                      NULL) ;

	ShowWindow(ls->hwndlist,SW_SHOW);

    for(index=0;index<NUMCOLUMNS;index++)
    {
		staticFlags=WS_CHILD | WS_VISIBLE;

		if(index!=0)
			staticFlags|=SS_CENTER;

		ls->hwndtext[index]=CreateWindow ("STATIC",ColText[index],
                    staticFlags,
                    0,0,0,0,
                    hwnd, (HMENU)(2000+index),
                    (HINSTANCE) g_hinst,
                    NULL) ;

		SendMessage(ls->hwndtext[index],WM_SETFONT,
				   (WPARAM)hFont,(LPARAM)MAKELPARAM(TRUE,0));
	}

	return TRUE;
}

int AddAnItem(HWND hwndList,PUSERKEYINFO pui)
{
	return SendMessage(hwndList,LB_ADDSTRING,0,(LPARAM)pui);
}

int SetListCursor(HWND hwndList,int index)
{
	return SendMessage(hwndList,LB_SETCURSEL,(WPARAM)index,
							(LPARAM)0);
}

#else // Listview 

int AddListViewItem(HWND hWndLV,int iImage,LPSTR szText,LPARAM lParam)
{
    LV_ITEM  lvItem;

    lvItem.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT ;

    lvItem.lParam        = lParam;
    lvItem.iImage        = iImage;
	lvItem.pszText       = szText;
    lvItem.iSubItem      = 0;
    lvItem.iItem         = ListView_GetItemCount( hWndLV);
	
    return ListView_InsertItem ( hWndLV, &lvItem );
}


BOOL AddListViewColumn(HWND hwnd,char **ColText,int *ColWidth,int num)
{
    LV_COLUMN lvc;
    RECT rect;
    BOOL ReturnValue = TRUE;
    int scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
    int index;

    GetClientRect(hwnd, &rect);

    for(index=0;index<num;index++)
    {
    lvc.mask        = LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT; 
    lvc.pszText     = ColText[index];
    lvc.cx          = ColWidth[index];
    lvc.iSubItem    = index; 
    if(index==0)
        lvc.fmt     = LVCFMT_LEFT;
    else
        lvc.fmt     = LVCFMT_CENTER;

    ListView_InsertColumn(hwnd, index, &lvc);
    }

    return ReturnValue;
}

void MoveList(LISTSTRUCT *ls,RECT *rc)
{
	MoveWindow(ls->hwndlist,
		rc->left,rc->top,
		rc->right-rc->left,rc->bottom-rc->top,
		TRUE);
}

BOOL InitList(HWND hwnd,
			  DWORD idc,
			  LISTSTRUCT *ls,
			  char **ColText,
			  float *ColRatio)
{
	RECT rc;
	int index;
	int listwidth;
	int xpos;
	float ratio;
	float textfloat;

	ls->hwnd=hwnd;

    GetClientRect(hwnd,&rc);

	listwidth=rc.right-rc.left-EDGEMARGIN*2;

	ls->hwndlist = CreateWindowEx (
					WS_EX_CLIENTEDGE,
					WC_LISTVIEW,
					"",
					WS_VISIBLE|WS_CHILD|LVS_REPORT|
					LVS_OWNERDRAWFIXED|WS_TABSTOP,
                    rc.left+EDGEMARGIN,rc.top+EDGEMARGIN,
					listwidth,rc.bottom-rc.top-EDGEMARGIN*2,
                    hwnd, (HMENU)idc,
                    (HINSTANCE) g_hinst,
                    NULL) ;

	ShowWindow(ls->hwndlist,SW_SHOW);

	listwidth=listwidth; // For icon
	xpos=rc.left+10;

    for(index=0;index<NUMCOLUMNS;index++)
    {
		ratio=ColRatio[index];

		if(ratio>1.0)
			ls->colwidth[index]=(int)ratio;
		else
		{
			textfloat=(float)listwidth;
			ls->colwidth[index]=(int)(ratio*textfloat);
		}
	}

	AddListViewColumn(ls->hwndlist,ColText,
			ls->colwidth,NUMCOLUMNS);

	return TRUE;
}

int AddAnItem(HWND hwndList,PUSERKEYINFO pui)
{
	int index;

	index=AddListViewItem(hwndList,0,pui->UserId,(LPARAM)pui);

	return index;
}

int SetListCursor(HWND hwndList,int index)
{
	ListView_EnsureVisible( hwndList,index,FALSE);

	return TRUE;
}

#endif // LISTBOX


BOOL ListBx_GetItem(HWND hwnd,LV_ITEM *pitem)
{
#if LISTBOX
	PUSERKEYINFO pui;

	pui=(PUSERKEYINFO)SendMessage(hwnd,LB_GETITEMDATA,(WPARAM)pitem->iItem,0);

    if(pui==0)
    	return FALSE;
    	
	if(pitem->mask&LVIF_PARAM)
		pitem->lParam=(LPARAM)pui;
	
	if(pitem->mask&LVIF_TEXT)
		strcpy(pitem->pszText,pui->UserId);

	if(pitem->mask&LVIF_STATE)
		pitem->state=(UINT)SendMessage(hwnd,
			LB_GETSEL,(WPARAM)pitem->iItem,(LPARAM)0);

	return TRUE;
#else
	return ListView_GetItem(hwnd,pitem);
#endif
}

UINT ListBx_GetSelectedCount(HWND hwnd)
{
#if LISTBOX
	return (UINT)SendMessage(hwnd,LB_GETSELCOUNT,(WPARAM)0,(LPARAM)0);
#else
	return ListView_GetSelectedCount(hwnd);
#endif
}

UINT ListBx_GetItemCount(HWND hwnd)
{
#if LISTBOX
	return (UINT)SendMessage(hwnd,LB_GETCOUNT,(WPARAM)0,(LPARAM)0);
#else
	return ListView_GetItemCount(hwnd);
#endif
}

UINT ListBx_GetItemState(HWND hwnd,int i,UINT mask)
{
#if LISTBOX
	return (UINT)SendMessage(hwnd,LB_GETSEL,(WPARAM)i,(LPARAM)0);
#else
	return ListView_GetItemState(hwnd,i,mask);
#endif
}

int ListBx_InsertItem(HWND hwnd,LV_ITEM *pitem)
{
#if LISTBOX
	PUSERKEYINFO pui;
	pui=(PUSERKEYINFO)pitem->lParam;

	return (int)SendMessage(hwnd,LB_ADDSTRING,(WPARAM)0,(LPARAM)pui);
#else
	return ListView_InsertItem(hwnd,pitem);
#endif
}

BOOL ListBx_DeleteItem(HWND hwnd,int iItem)
{
#if LISTBOX
	return (BOOL)SendMessage(hwnd,LB_DELETESTRING,(WPARAM)iItem,(LPARAM)0);
#else
	return ListView_DeleteItem(hwnd,iItem);
#endif
}


BOOL ListBx_DeleteAllItems(HWND hwnd)
{
#if LISTBOX
	return (BOOL)SendMessage(hwnd,LB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
#else
	return ListView_DeleteAllItems(hwnd);
#endif
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
