/*
 * ListSub.h  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

BOOL AddImagesToImageList(HWND hdlg,HIMAGELIST* hImageList);
int AddListViewItem(HWND hWndLV, int iImage, LPSTR szText, LPARAM lParam);
BOOL AddListViewColumn(HWND hwnd,char **ColText,int *ColWidth,int num);
LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg,
                                 WPARAM wParam, LPARAM lParam);
int WriteOutKeyIds(PRECIPIENTDIALOGSTRUCT prds,HWND hwnd);
int WriteOutLinkedListKeyIds(PRECIPIENTDIALOGSTRUCT prds,
							 PUSERKEYINFO RecipientLL);
void FreeLinkedLists(PUSERKEYINFO ListHead);
int LengthOfList(PUSERKEYINFO ListHead);
void MakeReloadString(HWND hwnd,DWORD* pdwNumRecpts, char** pszBuffer);
int NoDefaultKeyWarning(HWND parent);
PUSERKEYINFO GetPuiBasedOnComboIndex(PUSERKEYINFO ListHead,int index);
