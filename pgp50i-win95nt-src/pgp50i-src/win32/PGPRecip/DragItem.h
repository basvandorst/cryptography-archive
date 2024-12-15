/*
 * DragItem.h  Routines needed to create images for the ListView drag
 *
 * Since we're using ownerdrawn listboxes, we have to do all this ourselves.
 * These routines build the ImageList used with drag operations.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

HIMAGELIST MakeDragImage(HWND hwndDragFrom,int *left,int *bottom);
