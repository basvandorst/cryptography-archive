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

/*  @(#)negate.c	1.1  3/29/90  */

#include "normal.h"
#include "Imacro.h"

#define NUL_SHORT (unsigned short *)0

extern void bailout();
extern char *malloc();

/* NEGATE()
 * takes a very long negative integer in little-endian order and
 * computes its absolute value.
 */

unsigned short *
negate(a)
unsigned short *a;
{
  register unsigned i;
  unsigned short *b;

  if (*a == 0 || SIGNUM(a) == 0)
    bailout("Negate: This can't happen.");

  i = *a;
  b = (unsigned short *) malloc(i*sizeof(unsigned short));
  if (b == NUL_SHORT)
    bailout("Negate: unable to malloc enough memory.");

  *b = *a;                /* set the size */
  SIGNUM(b) = 0;          /* set the signbit */
  for (i = 1; i < *a; ++i)
    *(b+i) = ~ *(a + i);
  for (i = 1; *(b + i) == NEGSIGNBIT; ++i) {
    *(b + i) = 0;
  }
  ++ *(b+i);
  return(b);
}

