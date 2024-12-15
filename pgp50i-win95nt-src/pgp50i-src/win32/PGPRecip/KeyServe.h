/*
 * KeyServe.h  Routines needed to support the keyserver context menu
 *
 * All of these routines are used within the listview subclass handler, but
 * since they deal with the keyserver, they are isolated in this file for
 * clarity.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
#define WM_PGP_GET_LEFT_HELP        WM_USER + 53
#define WM_PGP_DO_CONTEXT_MENU      WM_USER + 54
#define ID_GET_LEFT_HELP            5986

void ListviewKeyserverRoutine(HWND hwnd, UINT msg,
                              WPARAM wParam, LPARAM lParam);
int LookUpNotFoundKeys(HWND hdlg);
