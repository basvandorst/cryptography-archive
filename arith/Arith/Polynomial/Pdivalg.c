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

/* The file @(#)Pdivalg.c 1.1, created on 3/29/90, contains the implementation of
 * the division algorithm for polynomials over finite fields, as part of
 * the Arithmetic package.
 */

#include "polynomial.h"
#include "storage.h"

#undef NULL
#define NULL (unsigned short *)0

extern warehouse *Z_tmp_WH;

/* ZDIVALG()
 * controls the division algorithm. It returns the quotient and stores
 * the remainder in ptr. If ptr == NULL, then the remainder is ignored.
 */

unsigned short *
Zdivalg(a, b, ptr)
unsigned short *a;          /* dividend */
unsigned short *b;          /* divisor */
unsigned short *ptr;       /* place for the remainder */
{
  extern unsigned short *INVERSE;  /* table of inverses modulo p */

  unsigned short *q;        /* quotient */
  unsigned short *r;        /* remainder */
  register unsigned k;      /* loop variable */
  short rd, bd;             /* degrees of the polynomials */
  register short lead;      /* lead coefficient of divisor */
  register short top;       /* coefficient of quoptient being computed */
  register short fact;      /* multiplicative factor */

  if ((bd = *b) == ZERODEG)
    bailout("Divalg: division by zero.");

  else if (bd == 1) {           /* special case of dividing by constant */
    r = capture(Z_tmp_WH, 0);
    *r = 0;
    lead = INVERSE[*(b+1)];
    q = capture(Z_tmp_WH, *a);
    *q = *a;
    for (k = 1; k <= *q; ++k)
      *(q + k) = MOD((lead * *(a + k)));

  } else if ((rd = *a) < bd) {
    q = capture(Z_tmp_WH, 0);
    *q = 0;
    r = capture(Z_tmp_WH, *a);
    Shcopy(a, r);

  } else {
    q = capture(Z_tmp_WH, *a);
    r = capture(Z_tmp_WH, *a);
    Shcopy(a, r);
    lead = INVERSE[*(b + *b)];
    top = *q = rd - bd + 1;

    while (top >= 1) {
      *(q + top) = MOD((*(r + *r) * lead));
      fact = MOD((modulus - *(q + top)));
      for (k = 1; k <= *b; ++k) {
	*(r + k + top - 1) = MOD((*(r + k + top - 1) + fact * *(b + k)));
      }
      rd = *r = Size(r);
      top = rd - bd + 1;
    }
  }
  if (ptr != NULL) 
    Shcopy(r, ptr);
  return(q);
}
