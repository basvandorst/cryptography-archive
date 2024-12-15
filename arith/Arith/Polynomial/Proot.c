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

/* The file @(#)Proot.c 1.1, created on 3/29/90, contains
 * a routine which computes the MODULUS-th root of a polynomial
 * defined over a field with MODULUS elements, as part of the
 * Arithmetic package.
 */

#include "polynomial.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

unsigned short *
proot(p)
unsigned short *p;
{
  register short i, j;
  unsigned short *root;

  root = capture(Z_tmp_WH, *p);
  for (i = 1; i <= *p; ++i) {
    if ((MOD(i-1)) == 0) {
      j = ((i-1) / modulus) + 1;
      *(root + j) = *(p + i);
    } else if (*(p + i) != 0) {
      *root = ZERODEG;
      return (root);
    }
  }
  *root = *p;
  *root = Size(root);
  return (root);
}
