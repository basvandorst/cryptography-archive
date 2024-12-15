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

/*  @(#)makewarehouse.c	1.1  3/29/90  */

#include "storage.h"

#define NUL_WH (warehouse *)0

extern char *malloc();
extern void bailout();

/* MAKEWH()
 * creates a structure of the appropriate type to use with the
 * storage routines capture and release. We think of a warehouse as 
 * consisting of both the structure itself and the array to which its
 * first member points. These will be malloc'ed by a single call, and
 * also free'd by a single call later. The array is cleaned to have 
 * all zero entries; some of the very long integer routines seem to
 * assume this in ways that are no longer clear to me.
 */

warehouse *
makeWH(length)
unsigned length;    /* how many unsigned shorts are needed ? */
{
  warehouse *wptr;
  unsigned total;
  register unsigned short *sptr, *stop;

  if (length == 0)
    bailout("MakeWH: zero length.");

  total = sizeof(warehouse) + length * sizeof(short);
  if ((wptr = (warehouse *) malloc(total)) == NUL_WH)
    bailout("MakeWH: malloc failure.");

  wptr->array = (unsigned short *) (wptr + 1);
  stop = wptr->array + length;
  for (sptr = wptr->array; sptr < stop; ++sptr)
    *sptr = 0;
  wptr->nextin = 0;
  wptr->depth = length;
  wptr->newer = NUL_WH;

  return(wptr);
}
