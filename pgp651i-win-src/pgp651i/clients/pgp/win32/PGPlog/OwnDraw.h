/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: OwnDraw.h,v 1.7 1999/03/10 02:46:37 heller Exp $
____________________________________________________________________________*/
#ifndef Included_OWNDRAW_h	/* [ */
#define Included_OWNDRAW_h

/*
 * OwnDraw.c  ListView draw routines to accommidate italics/strikeout
 *
 * These routines use the ownerdraw characteristic of the standard 
 * listview to provide custom drawing routines.
 *
 * Copyright (C) 1996 Network Associates Inc. and affiliated companies.
 * All rights reserved.
 */

#define CX_SMICON   16              // Size of the small images
#define CY_SMICON   16


typedef struct _drawstruct
{
	BOOL DisplayMarginal;
	BOOL MarginalInvalid;
	DWORD barcolor;
	HBRUSH stdbarbrush;
	HBRUSH spcbarbrush;
	HPEN g_seltextpen;
	HPEN g_unseltextpen;
	HPEN hilightpen;
	HPEN shadowpen;
	HPEN buttonpen;

	HBRUSH barbgbrush;
	HBRUSH HighBrush;
	HBRUSH BackBrush;
	HFONT	hFont;
	HFONT	hItalic;
	HFONT	hStrikeOut;
	HIMAGELIST hIml;
} DRAWSTRUCT;


#define PGP_DDERROR 1
#define PGP_DDTEXT 2
#define PGP_DDBAR 3

#define MAXCOLS 10

typedef struct _drawdata
{
	int numcols;
	int icon;
	int type[MAXCOLS];
	void *data1[MAXCOLS];
	void *data2[MAXCOLS];
	int chunksize;
	char *chunk;
	struct _drawdata *next;
} DRAWDATA;

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

#endif /* ] Included_OWNDRAW_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
