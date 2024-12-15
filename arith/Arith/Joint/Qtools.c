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

/* The file @(#)Qtools.c 1.1, created on 3/29/90, contains
 * the routines which perform rational Arithmetic either with arbitrary
 * precision integers or with polynomials over finite fields.
 */

#include <stdio.h>
#include "normal.h"
#include "Ztools.h"
#include "storage.h"

extern void Shcopy();

extern warehouse *Z_tmp_WH;
extern warehouse *Q_tmp_WH;

typedef struct {
  unsigned short *num;
  unsigned short *den;
} ratl;

/* REDUCE(r)
 *     This function takes a rational structure r and
 *  divides its numerator and denominator by their Zgcd
 */

ratl
Reduce(r)
ratl  r;
{
  ratl  c;
  unsigned short *dummy;

  c.num = capture(Q_tmp_WH, *r.num);
  c.den = capture(Q_tmp_WH, *r.den);
  dummy = capture(Z_tmp_WH, MAX(*r.num, *r.den));
  Zgcd(r.num, r.den, dummy, c.num, c.den);
  release(Z_tmp_WH, dummy);
  return (c);
}

ratl
Qadd(a, b)
ratl  a, b;
{
  ratl  c;
  unsigned short *d, *e;
  unsigned short *mark;

  mark = capture(Z_tmp_WH, 0);
  d = Zmult(a.num, b.den);
  e = Zmult(a.den, b.num);
  c.num = Zadd(d,e);
  c.den = Zmult(a.den, b.den);
  c = Reduce(c);
  release(Z_tmp_WH, mark);
  return(c);
}

ratl
Qmult(a, b)
ratl  a, b;
{
  ratl c;
  unsigned short *mark;

  mark = capture(Z_tmp_WH, 0);
  c.num = Zmult(a.num, b.num);
  c.den = Zmult(a.den, b.den);
  c = Reduce(c);
  release(Z_tmp_WH, mark);
  return(c);
}

ratl
Qminus(a)
ratl  a;
{
  ratl  b;
  unsigned short *mark;

  mark = capture(Z_tmp_WH, 0);
  b.num = capture(Q_tmp_WH, *a.num);
  Shcopy(Zminus(a.num), b.num);
  b.den = capture(Q_tmp_WH, *a.den);
  Shcopy(a.den, b.den);
  release(Z_tmp_WH, mark);
  return(b);
}

ratl
Qsub(a, b)
ratl  a, b;
{
  return(Qadd(a, Qminus(b)));
}

bool
Qeq(a, b)
ratl  a, b;
{
  unsigned short *d, *e;
  unsigned short *mark;
  bool ok;

  mark = capture(Z_tmp_WH, 0);
  d = Zmult(a.den, b.num);
  e = Zmult(a.num, b.den);
  ok = Zeq(d,e);
  release(Z_tmp_WH, mark);
  return(ok);
}

ratl
Qinv(a)
ratl  a;
{
  ratl  c;

  c.num = capture(Q_tmp_WH, *a.den);
  Shcopy(a.den, c.num);
  c.den = capture(Q_tmp_WH, *a.num);
  Shcopy(a.num, c.den);
  return(c);
}

ratl
Qdiv(a, b)
ratl  a, b;
{
  return(Qmult(a, Qinv(b)));
}

