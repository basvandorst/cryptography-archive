/*
 * Arithmetic in Global Fields
 * Copyright (C) 1990 Kevin R. Coombes and David R. Grant
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*  @(#)release.c	1.1  3/29/90  */

#include "storage.h"

#define NUL_WH (warehouse *)0

extern void bailout();

/* RELEASE()
 * reclaims storage back to the marked point. Warehouses form a
 * linked list. We first find which warehouse the mark points to. We
 * then free any further warehouses, making the marked warehouse the
 * tail of the list. We then also zero out anything after the mark in
 * the designated warehouse.
 */

void
release(where, mark)
warehouse *where;
unsigned short *mark;
{
  register unsigned short *sptr;       /* to zero out within a warehouse */
  register warehouse *w;               /* to run through the linked list */
  register warehouse *slash, *burn;    /* to free unneeded warehouses */

  w = where;
  while (mark < w->array || mark >= w->array + w->depth) {
    if (w->newer == NUL_WH)
      bailout("Release: bad mark.");
    else
      w = w->newer;
  }

  if (w->nextin < 0) {     /* this is a sign that the current warehouse is */
    w->nextin = w->depth;  /* full, and more warehouses exist */
    slash = w->newer;
    w->newer = NUL_WH;
    while(slash != NUL_WH) {
      burn = slash;
      slash = burn->newer;
      free((char *) burn);
    }
  }

  sptr = w->array + w->nextin;
  do {
    --sptr;
    *sptr = 0;
  } while(sptr > mark);

  w->nextin = sptr - w->array;
  return;
}


