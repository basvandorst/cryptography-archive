/*
 * OwnDraw.c  ListView draw routines to accommidate italics/strikeout
 *
 * These routines use the ownerdraw characteristic of the standard
 * listview to provide custom drawing routines.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#define CX_SMICON   16              // Size of the small images
#define CY_SMICON   16

HBITMAP OwnerDraw_CreateDragImage(HWND hwnd,int selitem);
void DrawListViewItem(LPDRAWITEMSTRUCT);
BOOL CalcStringEllipsis(HDC hdc, LPTSTR lpszString,
                        int cchMax, UINT uColWidth);
void DrawItemColumn(HDC hdc, LPTSTR lpsz, LPRECT prcClip);
BOOL Main_OnDrawItem(HWND, const DRAWITEMSTRUCT *);
void Main_OnMeasureItem(HWND, MEASUREITEMSTRUCT *);
void CreateDrawElements(HWND hwnd);
void DeleteDrawElements(HWND hwnd);
