/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

BOOL CALLBACK RecipientDlgProc(HWND hdlg,UINT uMsg,
                               WPARAM wParam,LPARAM lParam);
void GetHelpFile(char *Path);
PRECGBL AllocRecGbl(void);
int InitRecGbl(HWND hwnd,PRECGBL pRecGbl);
PRECGBL RecGbl(HWND hwnd);
int FreeRecGbl(HWND hwnd);
