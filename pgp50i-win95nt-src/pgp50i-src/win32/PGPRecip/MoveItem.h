/*
 * MoveItem.h  List View movement routines that support MRK
 *
 * These routines use the algorithm developed by Mark W. to support MRK
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

PMRKINFO GetMRKList(PGPKey *basekey, PGPKeySet *allkeys);
void FreeMRKLists(PMRKINFO ListHead);
int MoveListViewItems(HWND left, HWND right, BOOL LeftToRight);
int MoveMarkedItems(HWND src, HWND dest);
int InitialMoveListViewItems(HWND left, HWND right,PUSERKEYINFO pui);
