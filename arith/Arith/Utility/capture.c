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

/*  @(#)capture.c	1.1  3/29/90  */

#include "storage.h"

#define NUL_WH    (warehouse *)0
#define NUL_SHORT (unsigned short *)0

extern void bailout();

static unsigned WH_block = BLOCKSIZE;

/* CAPTURE()
 * returns a pointer to a block of storage of the desired length.
 * Note that the "length" of a polynomial is one more than its
 * degree, and this is one less than the number of shorts we need
 * to store it.
 */

unsigned short *
capture(where, howmany)
warehouse *where;
unsigned howmany;
{
  register warehouse *w;
  unsigned short *ptr;

  w = where;
/* w->nextin usually holds the offset of the next open block. It is set
 * equal to -1 when the warehouse is full, and a new warehouse is created.
 */
  while (w->nextin < 0) {
    if (w->newer == NUL_WH)
      bailout("Capture: missing warehouse.");
    else
      w = w->newer;
  }

  if (w->array == NUL_SHORT)
    bailout("Capture: vacant warehouse.");
  else if (w->nextin + howmany + 1 > w->depth) {  /* if not enough room */
    w->nextin = -1;                               /* hang up the full sign */
    if (w->newer == NUL_WH) {                     /* then get more */
      while (10 * howmany > WH_block)        /* make the new warehouse */
	WH_block *= 2;                       /* sufficiently roomy */
      w->newer = makeWH(WH_block);
    }
    ptr = capture(w->newer, howmany);
  } else {
    ptr = w->array + w->nextin;
    w->nextin += howmany + 1;                /* set aside requested space */
  }
  return(ptr);
}

