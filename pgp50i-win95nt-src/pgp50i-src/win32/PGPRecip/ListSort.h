/*
 * ListSort.h  List View sorting routine
 *
 * Apparently the sorting algorithm used in PGPlib is somehow different than
 * Microsoft employs in their standard listview control. This makes things
 * sometimes confusing, since PGPkeys follows the PGPlib convention. Albeit
 * bucking PC GUI standards SortEm changes the listview sort to comply with
 * PGPlib and PGPkeys.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

void SortEm(HWND hwndLView);

#define HEADUSERID 0
#define HEADVALIDITY 1
#define HEADTRUST 2
#define HEADSIZE 3
