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
/*  @(#)trial.c	1.1  3/29/90  */

#include <stdio.h>
#include "integer.h"
#include "storage.h"

extern void blurb();

unsigned short modulus;

main()
{
  warehouse *nu;
  unsigned short *n;
  unsigned short *d;

  blurb("Trial", "1990", "Kevin R. Coombes and David R. Grant");
  nu = makeWH(10);
  n = capture(nu, 15);
  d = capture(nu,15);
  while (more("another number") == YES) {
    if (Collect("number", n) == NO)
      bailout("Main: we got troubles.");
    Showme("input = ",n);
  }
  Shcopy(n,d);
  Showme("copy of final input = ",d);
  (void) printf("Successful test of input, output, storage, and copying utilities.\n");
}
