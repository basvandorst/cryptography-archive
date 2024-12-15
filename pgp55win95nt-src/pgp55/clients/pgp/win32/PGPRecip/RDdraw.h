/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDdraw.h,v 1.1 1997/08/11 20:41:52 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_RDDRAW_h	/* [ */
#define Included_RDDRAW_h

/*
 * OwnDraw.c  ListView draw routines to accommidate italics/strikeout
 *
 * These routines use the ownerdraw characteristic of the standard 
 * listview to provide custom drawing routines.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

#define CX_SMICON   16              // Size of the small images
#define CY_SMICON   16

#define PGP_DDERROR 1
#define PGP_DDTEXT 2
#define PGP_DDBAR 3

#define MAXCOLS 10

void CreateDrawElements(DRAWSTRUCT *ds);
void DeleteDrawElements(DRAWSTRUCT *ds);
void DrawBar(DRAWSTRUCT *ds,HDC hdc,RECT *ptrBarRect,
             int DataValue,int MaxValue,BOOL Selected);
void DrawListViewItem(LPDRAWITEMSTRUCT lpDrawItem);
void DrawItemColumn(HDC hdc, LPTSTR lpsz, LPRECT prcClip);
BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString, 
                        int cchMax, UINT uColWidth);
BOOL Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
void Main_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);

#endif /* ] Included_RDDRAW_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
