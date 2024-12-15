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


/* The file @(#)Ptools.c 1.1, created on 3/29/90, contains
 * the routines which perform arbitrary precision Arithmetic in the
 * ring of polynomials in one variable over the finite field with
 * MODULUS elements.
 */

#include "polynomial.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

void
Zbigadd(a, b, c)
unsigned short *a, *b, *c;
{
  register int   i;

  for (i = 1; i <= *b; ++i)
    *(c + i) = MOD((*(a + i) + *(b + i)));
  for (i = *b + 1; i <= *a; ++i)
    *(c + i) = *(a + i);
}

unsigned short *
Zadd(a, b)
unsigned short *a, *b;
{
  unsigned short *c;

  if (*a == ZERODEG) {
    c = capture(Z_tmp_WH, *b);
    Shcopy(b, c);
  } else if (*b == ZERODEG) {
    c = capture(Z_tmp_WH, *a);
    Shcopy(a, c);
  } else if (*a > *b) {
    c = capture(Z_tmp_WH, *a);
    *c = *a;
    Zbigadd(a, b, c);
  } else if (*b > *a) {
    c = capture(Z_tmp_WH, *b);
    *c = *b;
    Zbigadd(b, a, c);
  } else {
    c = capture(Z_tmp_WH, *a);
    *c = *a;
    Zbigadd(a, b, c);
    *c = Size(c);
  }
  return(c);
}

void
Zshortmult(a, b, c, i)
unsigned short *a, *c;
unsigned short b;
int i;
{
  register int n;

  for (n = 1; n <= *a; ++n)
    *(c + i + n) = MOD((*(c + i + n) + *(a + n) * b));
}

unsigned short *
Zmult(a, b)
unsigned short *a, *b;
{
  unsigned short *c;
  register int i;

  if (*a == ZERODEG || *b == ZERODEG) {
    c = capture(Z_tmp_WH, 0);
    *c = 0;
  } else {
    c = capture(Z_tmp_WH, *a + *b - 1);
    *c = *a + *b - 1;
    for (i = 0; i < *b; ++i)
      Zshortmult(a, *(b + i + 1), c, i);
  }
  return(c);
}

unsigned short *
Zminus(a)
unsigned short *a;
{
  unsigned short *s;
  register int i;

  s = capture(Z_tmp_WH, *a);
  *s = *a;
  for (i = 1; i <= *a; ++i)
    *(s + i) = MOD((modulus - *(a + i)));
  return(s);
}

unsigned short *
Zsub(a, b)
unsigned short *a, *b;
{
  unsigned short *c;

  if (*b == ZERODEG) {
    c = capture(Z_tmp_WH, *a);
    Shcopy(a, c);
  } else {
    c = Zadd(a, Zminus(b));
  }
  return(c);
}

bool
Zeq(a, b)
unsigned short *a, *b;
{
  register int i;
  register int logic = 1;

  if (*a != *b)
    return (NO);
  for (i = 1; i <= *a; ++i)
    logic = logic * (*(a + i) == *(b + i));
  return (logic);
}

bool
Zlessoreq(a, b)
unsigned short *a, *b;
{
  if (*a > *b)
    return (NO);
  else
    return (YES);
}
