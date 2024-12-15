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

/* @(#)size.c	1.1  3/29/90  */

#include "normal.h"
#include "Imacro.h"

extern unsigned short *negate();
extern free();

/* SIZE()
 * determines the actual number of nonzero coefficients in a very
 * long integer or polynomial which is stored in little-endian order.
 */

unsigned short 
size(n, with_sign)
unsigned short *n;
bool with_sign;
{
  register unsigned short s;    /* the size */
  unsigned short *neg;          /* minus n, if needed */

  if (*n == 0)
    s = 0;
  else {
    if (with_sign == TRUE && SIGNUM(n) == NEGSIGNBIT) {
      neg = negate(n);
      s = size(neg, with_sign);
      free((char *)neg);
      return(s);
    } else if (with_sign == TRUE) {
      s = *n - 1;
    } else {
      s = *n;
    }

    while (s > 0 && *(n + s) == 0)
      --s;

    if (s != 0 && with_sign == TRUE) ++s;
  }
  return(s);
}
