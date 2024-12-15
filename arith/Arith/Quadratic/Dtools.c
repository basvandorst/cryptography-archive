/* This file is @(#)Dtools.c	1.1 created on 3/29/90.
 *
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

/* The file @(#)Dtools.c 1.1, created on 3/29/90, contains
 * the implementation of arbitrary-precision Arithmetic in the ring
 * of integers in a quadratic number field.
 */

#include "Dstruct.h"

static unsigned short _DB_[3]; /* rooom to store a constant */
unsigned short *DB = _DB_;     /* Basic integer */
bool  onemodfour;              /* Is D congruent to one mod 4? */

bool
Deq(a, b)
quad  a;
quad  b;
{
  return(Zeq(a.alpha, b.alpha) && Zeq(a.beta, b.beta));
}

quad
Dadd(a, b)
quad  a;
quad  b;
{
  quad  c;

  c.alpha = Zadd(a.alpha, b.alpha);
  c.beta = Zadd(a.beta, b.beta);
  return (c);
}

quad
Dsub(a, b)
quad  a;
quad  b;
{
  quad  c;

  c.alpha = Zsub(a.alpha, b.alpha);
  c.beta = Zsub(a.beta, b.beta);
  return (c);
}

quad
_OneMult(u, v)
quad  u;
quad  v;
{
  quad  c;
  unsigned short *aa, *ab, *ba, *bb;

  aa = Zmult(u.alpha, v.alpha);
  ab = Zmult(u.alpha, v.beta);
  ba = Zmult(u.beta,  v.alpha);
  bb = Zmult(u.beta,  v.beta);

  c.alpha = Zadd(Zmult(bb, DB), aa);
  c.beta = Zadd(Zadd(ab, ba), bb);

  return(c);
}

quad
_TwoMult(a, b)
quad  a;
quad  b;
{
  quad  c;
  unsigned short *tmp, *tmp2;

  tmp = Zmult(a.beta, b.beta);
  tmp = Zmult(DB, tmp);
  tmp2 = Zmult(a.alpha, b.alpha);
  c.alpha = Zadd(tmp, tmp2);

  tmp = Zmult(a.alpha, b.beta);
  tmp2 = Zmult(a.beta, b.alpha);
  c.beta = Zadd(tmp, tmp2);

  return(c);
}

quad
Dmult(a, b)
quad  a;
quad  b;
{
  quad  c;

  if (onemodfour)
    c = _OneMult(a, b);
  else
    c = _TwoMult(a, b);
  return (c);
}

unsigned short *
_OneNorm(a)
quad  a;
{
  unsigned short *n;
  unsigned short *tmp, *aa, *ab, *bb;

  aa = Zmult(a.alpha, a.alpha);
  ab = Zmult(a.alpha, a.beta);
  bb = Zmult(a.beta,  a.beta);

  tmp = Zmult(bb, DB);
  n = Zsub(Zadd(aa, ab), tmp);

  return(n);
}

unsigned short *
_TwoNorm(a)
quad  a;
{
  unsigned short *n;
  unsigned short *tmp, *tmp2;

  tmp = Zmult(a.alpha, a.alpha);
  tmp2 = Zmult(a.beta, a.beta);
  tmp2 = Zmult(tmp2, DB);
  n = Zsub(tmp, tmp2);
  return(n);
}

unsigned short *
Norm(a)
quad  a;
{
  unsigned short *n;

  if (onemodfour)
    n = _OneNorm(a);
  else
    n = _TwoNorm(a);
  return (n);
}

quad
Dminus(x)
quad  x;
{
  quad  t;

  t.alpha = Zminus(x.alpha);
  t.beta = Zminus(x.beta);
  return (t);
}

void
Dinitial(D)
long D;
{
  long specialcase;

  if (D % 4 == 1) {
    onemodfour = TRUE;
    specialcase = (D-1)/4;
    Shcopy(Constant(specialcase), DB);
  } else {
    onemodfour = FALSE;
    Shcopy(Constant(D), DB);
  }
  return;
}
