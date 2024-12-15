/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ListMng.c,v 1.4 1997/08/28 16:07:30 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

#define EDGEMARGIN 0

#if LISTBOX

void MoveList(LISTSTRUCT *ls,int Width,int Height)
{
	int listwidth;
	int xpos;
	float textfloat,ratio;
	int index;

	listwidth=Width-EDGEMARGIN*2;

	MoveWindow(ls->hwndlist,
		EDGEMARGIN,EDGEMARGIN+15,
		listwidth,Height-EDGEMARGIN*2-15,
		TRUE);

	xpos=EDGEMARGIN;

    for(index=0;index<NUMCOLUMNS;index++)
    {
		ratio=ls->colratio[index];

		textfloat=(float)listwidth;
		ls->colwidth[index]=(int)(ratio*textfloat);
		
		MoveWindow(ls->hwndtext[index],
			xpos,0,
			ls->colwidth[index],20,
			TRUE);
	
		xpos=xpos+ls->colwidth[index];
	}
}

BOOL InitList(HWND hwnd,LISTSTRUCT *ls,char **ColText,float *ColRatio)
{
	RECT rc;
	int index;

	ls->hwnd=hwnd;
	ls->colratio=ColRatio;

    ls->hwndlist = CreateWindow ("listbox", NULL,
                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
					  WS_BORDER| LBS_OWNERDRAWFIXED,
                      0,0,0,0,
                      hwnd, (HMENU)IDC_LOGLIST,
                      (HINSTANCE) g_hinst,
                      NULL) ;

	ShowWindow(ls->hwndlist,SW_SHOW);

    for(index=0;index<NUMCOLUMNS;index++)
    {
		ls->hwndtext[index]=CreateWindow ("STATIC",ColText[index],
                    WS_CHILD | WS_VISIBLE | SS_CENTER,
                    0,0,0,0,
                    hwnd, (HMENU)(2000+index),
                    (HINSTANCE) g_hinst,
                    NULL) ;

		SendMessage(ls->hwndtext[index],WM_SETFONT,
				   (WPARAM)hFont,(LPARAM)MAKELPARAM(TRUE,0));
	}

	GetClientRect(hwnd,&rc);
	MoveList(ls,rc.right-rc.left,rc.bottom-rc.top);

	return TRUE;
}

int AddAnItem(HWND hwndList,DRAWDATA *dd)
{
	return SendMessage(hwndList,LB_ADDSTRING,0,(LPARAM)dd);
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

void MoveList(LISTSTRUCT *ls,int Width,int Height)
{
	MoveWindow(ls->hwndlist,
		EDGEMARGIN,EDGEMARGIN,
		Width-EDGEMARGIN*2,Height-EDGEMARGIN*2,
		TRUE);
}

BOOL InitList(HWND hwnd,LISTSTRUCT *ls,char **ColText,float *ColRatio)
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
					WS_BORDER|LVS_OWNERDRAWFIXED,
                    0,0,0,0,
                    hwnd, (HMENU)IDC_LOGLIST,
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

int AddAnItem(HWND hwndList,DRAWDATA *dd)
{
	int index;

	index=AddListViewItem(hwndList,0,"",(LPARAM)dd);

	return index;
}

int SetListCursor(HWND hwndList,int index)
{
	ListView_EnsureVisible( hwndList,index,FALSE);

	return TRUE;
}

#endif // LISTBOX


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
