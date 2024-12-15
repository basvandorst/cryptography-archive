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

/* The file @(#)Pgcd.c 1.1, created on 3/29/90, contains
 * the function which compute gcds of arbitrary degree polynomials, as
 * part of the Arithmetic package.
 */

#include "polynomial.h"
#include "storage.h"

extern warehouse *Z_tmp_WH;

#define MAXSTEP 1400

void
zeros(a, b, c, d, e)
unsigned short *a, *b, *c, *d, *e;
{
  Shcopy(a, c);
  Shcopy(Constant(1), d);
  Shcopy(b, e);
  return;
}

void
ones(a, b, c, d, e)
unsigned short *a, *b, *c, *d, *e;
{
  Shcopy(Constant(1), c);
  Shcopy(b, e);
  Shcopy(a, d);
  return;
}

/* ZGCD()
 *     This function computes the gcd of a and b.
 *  The gcd is stored at location c; the values of
 *  a/c and b/c are stored at locations d and e. This
 * is normalized so the gcd is always a monic polynomial.
 */

void
Zgcd(a, b, c, d, e)
unsigned short *a, *b, *c, *d, *e;
{
  extern unsigned short *INVERSE;  /* table of inverses modulo p */

  register int i, j;                         /* loop variables */
  unsigned short *r, *s, *t, *ptr;           /* safe and swappable */
  unsigned short *(q[MAXSTEP]);              /* stored quotients */
  unsigned short *marker;                    /* storage saver */
  short trigger;                             /* degree of remainder */
  short monic;                               /* lead coefficient */
  unsigned short deg;                        /* size of inputs */

  /* first handle trivial special cases */
  if (*b == ZERODEG)
    zeros(a, b, c, d, e);
  else if (*a == ZERODEG)
    zeros(b, a, c, e, d);
  else if (*b == 1)
    ones(a, b, c, d, e);
  else if (*a == 1)
    ones(b, a, c, e, d);
  else {

    /* do initial setup */
    marker = capture(Z_tmp_WH, 0);
    deg = MAX(*a, *b);
    r = capture(Z_tmp_WH, deg);
    s = capture(Z_tmp_WH, deg);
    t = capture(Z_tmp_WH, deg);
    ptr = capture(Z_tmp_WH, deg);
    Shcopy(a, r);
    Shcopy(a, s);
    Shcopy(b, t);
    i = 0;

    /* the main loop */
    trigger = *r;
    while (trigger >= 1) {
      q[i] = Zdivalg(s, t, r);
      trigger = *r;
      ptr = s;
      s = t;
      t = r;
      r = ptr;
      ++i;
      if (i >= MAXSTEP)
	bailout("ZGCD: Steps needed exceeds MAXSTEP.");
    }

    /* normalize the gcd to be monic */
    monic = INVERSE[*(s + *s)];
    *c = *s;
    for (j = 1; j <= *s; ++j)
      *(c + j) = MOD((monic * *(s + j)));

    /* do back substitution */
    Shcopy(Constant(1), s);
    Shcopy(Constant(0), t);
    while (i > 0) {
      r = Zmult(s, q[i - 1]);
      r = Zadd(r, t);
      ptr = t;
      t = s;
      s = r;
      r = ptr;
      --i;
    }

    /* store the answers in the desired places */
    monic = INVERSE[monic];
    *d = *s;
    for (j = 1; j <= *d; ++j)
      *(d + j) = MOD((monic * *(s + j)));
    *e = *t;
    for (j = 1; j <= *e; ++j)
      *(e + j) = MOD((monic * *(t + j)));

    /* cleanup storage area */
    release(Z_tmp_WH, marker);
  }
  return;
}
