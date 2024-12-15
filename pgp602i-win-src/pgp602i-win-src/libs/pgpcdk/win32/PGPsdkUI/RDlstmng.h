/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDlstmng.h,v 1.1.12.1 1998/11/12 03:24:27 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDLSTMNG_h	/* [ */
#define Included_RDLSTMNG_h


#include "RDdraw.h"

#define LISTBOX 0

BOOL InitList(HWND hwnd,
			  DWORD idc,
			  LISTSTRUCT *ls,
			  char **ColText,
			  float *ColRatio);
int SetListCursor(HWND hwndList,int index);
int AddAnItem(HWND hwndList,PUSERKEYINFO pui);
void MoveList(LISTSTRUCT *ls,RECT *rc);

BOOL ListBx_GetItem(HWND hwnd,LV_ITEM *pitem);
UINT ListBx_GetSelectedCount(HWND hwnd);
UINT ListBx_GetItemCount(HWND hwnd);
UINT ListBx_GetItemState(HWND hwnd,int i,UINT mask);
int ListBx_InsertItem(HWND hwnd,LV_ITEM *pitem);
BOOL ListBx_DeleteItem(HWND hwnd,int iItem);
BOOL ListBx_DeleteAllItems(HWND hwnd);


#endif /* ] Included_RDLSTMNG_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
