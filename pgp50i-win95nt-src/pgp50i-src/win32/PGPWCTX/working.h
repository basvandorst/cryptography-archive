/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
BOOL CALLBACK WorkingDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
							LPARAM lParam);
int WorkingCallback (void* arg, INT SoFar, INT Total) ;
HWND WorkingDlgProcThread(HMODULE hMod,
						  HWND hParent,
						  char *Action,
						  char *FileName);
