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

/* The file @(#)Zgcd.c 1.1, created on 3/29/90, contains
 * the functions which compute gcds of arbitrary length integers in the
 * Arithmetic package.
 */

#include "integer.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

#define MAXSTEP 1400

/* ZGCD()
 *     This function computes the gcd of a and b.
 *  The gcd is stored at location c; the values of
 *  a/c and b/c are stored at locations d and e.
 */

void 
Zgcd(a, b, c, d, e)
unsigned short *a, *b, *c, *d, *e;
{
  unsigned short *r, *s, *t, *ptr, *(q[MAXSTEP]), *mark;
  short trigger;
  unsigned short sz;
  unsigned short asign, bsign;
  register int   i;

  if (*b == 0) {
    Shcopy(a, c);
    Shcopy(Constant(1), d);
    Shcopy(b,e);
    return;
  } else if (*a == 0) {
    Zgcd(b, a, c, e, d);
    return;
  }

  mark = capture(Z_tmp_WH, 0);
  sz = MAX(*a, *b);
  r = capture(Z_tmp_WH, sz);
  s = capture(Z_tmp_WH, sz);
  t = capture(Z_tmp_WH, sz);
  ptr = capture(Z_tmp_WH, sz);
  if ((asign = SIGNUM(a)) == NEGSIGNBIT)
    Shcopy(Zminus(a), r);
  else
    Shcopy(a, r);
  if ((bsign = SIGNUM(b)) == NEGSIGNBIT)
    Shcopy(Zminus(b), t);
  else
    Shcopy(b, t);
  Shcopy(r, s);
  i = 0;

  trigger = *r;
  while (trigger != 0) {
    q[i] = Zdivalg(s, t, r);
    if (SIGNUM(r) != 0)
      bailout("ZGCD: remainder sign error.");
    if (SIGNUM(q[i]) != 0)
      bailout("ZGCD: quotient sign error.");
    if (!Zlessoreq(r, t))
      bailout("ZGCD: invalid remainder.");
    trigger = *r;
    ptr = s;
    s = t;
    t = r;
    r = ptr;
    ++i;
    if (i >= MAXSTEP)
      bailout("Zgcd: Steps needed exceed MAXSTEP");
  }

  Shcopy(s, c);
  Shcopy(Constant(1), s);
  Shcopy(Constant(0), t);
  while (i > 0) {
    r = Zmult(s, q[i-1]);
    r = Zadd(r,t);
    ptr = t;
    t = s;
    s = r;
    r = ptr;
    --i;
  }

  if (asign == NEGSIGNBIT)
    Shcopy(Zminus(s), d);
  else
    Shcopy(s, d);
  if (bsign == NEGSIGNBIT)
    Shcopy(Zminus(t), e);
  else
    Shcopy(t, e);

  release(Z_tmp_WH, mark);
  return;
}
