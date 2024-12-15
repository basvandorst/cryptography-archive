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

/* The file @(#)Ztools.c 1.1, created on 3/29/90, contains
 * the routines which perform arbitrary precision integer Arithmetic.
 */

#include <stdio.h>
#include "integer.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

void
Zbigadd(a, b, c)  /* assumes size(a) >= size(b) */
unsigned short *a, *b, *c;
{
  unsigned short asign, bsign, csign;
  register unsigned short carry = 0;
  register unsigned long sum = 0;
  register int i;

  asign = SIGNUM(a);
  bsign = SIGNUM(b);
  for (i = 1; i <= *b; ++i) {
    sum = carry + *(a + i) + *(b + i);
    carry = sum >> SHORTLENGTH;
    *(c + i) = sum & SHORTMASK;
  }
  for (i = (*b)+1; i <= *a; ++i) {
    sum = carry + *(a + i) + bsign;    /* the sign propogates */
    carry = sum >> SHORTLENGTH;
    *(c + i) = sum & SHORTMASK;
  }
  *(c + i) = carry + asign + bsign;
  csign = SIGNUM(c);
  if ((csign != 0) && (csign != NEGSIGNBIT)) {
    bailout("sum overflow");
  }
}

unsigned short *
Zadd(a, b)
unsigned short *a, *b;
{
  unsigned short *c;

  if (*a == 0) {
    c = capture(Z_tmp_WH, *b);
    Shcopy(b, c);
  } else if (*b == 0) {
    c = capture(Z_tmp_WH, *a);
    Shcopy(a, c);
  } else if (*a > *b) {
    c = capture(Z_tmp_WH, *a + 1);
    *c = *a + 1;
    Zbigadd(a, b, c);
  } else {
    c = capture(Z_tmp_WH, *b + 1);
    *c = *b + 1;
    Zbigadd(b, a, c);
  }
  *c = Size(c);
  return(c);
}

void 
Zshortmult(a, shortb, c, shift)
unsigned short *a, shortb, *c;
int shift;
{
  register int   i;
  register unsigned short carry = 0;
  register unsigned long prod = 0;

  for (i = 1; i < *a; ++i) {
    prod = carry + *(c + i + shift) + (*(a + i) * shortb);
    carry = prod >> SHORTLENGTH;
    *(c + i + shift) = prod & SHORTMASK;
  }
  *(c + i + shift) += carry;
}

unsigned short *
Zmult(a, b)
unsigned short *a, *b;
{
  unsigned short *c;          /* the product */
  unsigned short *d, *e;      /* tmeporaries to handle negatives */
  register int   i;
  int signflag = 1;

  if (*a == 0 || *b == 0) {
    c = capture(Z_tmp_WH, 0);
    *c = 0;
  } else {
    c = capture(Z_tmp_WH, *a + *b - 1);
    *c = *a + *b - 1;
    d = a;
    if (SIGNUM(a) == NEGSIGNBIT) {
      d = Zminus(a);
      signflag = -signflag;
    }
    e = b;
    if (SIGNUM(b) == NEGSIGNBIT) {
      e = Zminus(b);
      signflag = -signflag;
    }
    for (i = 1; i < *e; ++i)
      Zshortmult(d, *(e + i), c, i - 1);
    *c = Size(c);
    if (signflag == -1)
      c = Zminus(c);
  }
  return(c);
}

unsigned short *
Zminus(a)
unsigned short *a;
{
  register int   i;
  unsigned short *b;

  b = capture(Z_tmp_WH, *a);
  *b = *a;
  if (*b == 0)
    return(b);
  else {
    for (i = *a; i > 0; --i) {
      *(b + i) = ~*(a + i);
    }
    return(Zadd(Constant(1), b));
  }
}

unsigned short *
Zsub(a, b)
unsigned short *a, *b;
{
  return(Zadd(a, Zminus(b)));
}

bool 
Zeq(a, b)
unsigned short *a, *b;
{
  register int i;
  register int logic = 1;

  if (*a != *b)
    return(NO);
  else for (i = *a; i > 0; --i)
    logic = logic * (*(a + i) == *(b + i));
  return (logic);
}

bool 
Zlessoreq(a, b)
unsigned short *a, *b;
{
  register int i;
  unsigned short *mark;
  bool reply;

  if (SIGNUM(a) == NEGSIGNBIT) {
    mark = capture(Z_tmp_WH, 0);
    reply = !Zlessoreq(Zminus(a), Zminus(b));
    release(Z_tmp_WH, mark);
  } else if (SIGNUM(b) == NEGSIGNBIT)
    reply = NO;
  else if (*a > *b)
    reply = NO;
  else if (*a < *b)
    reply = YES;
  else {
    reply = YES;
    for (i = *a; i > 0; --i) {
      if (*(a + i) < *(b + i))
	return (YES);
      else if (*(a + i) > *(b + i))
	return (NO);
    }
  }
  return(reply);
}
