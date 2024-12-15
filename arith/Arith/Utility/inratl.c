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

/*  @(#)inratl.c	1.1  3/29/90  */

#include <stdio.h>
#include "normal.h"
#include "Qstruct.h"
#include "myinput.h"

extern void bailout();
extern void Shcopy();
extern unsigned short *constant();
extern bool collect();

/* INRATL()
 * reads in a rational structure, using collect to get a very long
 * integer or polynomial.
 */

bool 
inratl(msg, x, toggle, with_sign)
char *msg;            /* description of desired input */
ratl  x;              /* where to store the input - not my problem */
short toggle;         /* expecting integers or fractions ? */
bool with_sign;       /* expecting signed integers ? */
{
  bool reply;         /* were we successful ? */
  bool step;          /* communication with collect */

  step = NUMER_IN;
  reply = YES;
  while (step != FINISH) {
    switch (step) {
    case NUMER_IN:
      step = collect("Numerator of ", msg, x.num, step, with_sign);
      break;
    case DENOM_IN:
      if (toggle == INTEG_IN) {
	step = FINISH;
	Shcopy(constant(1L, with_sign), x.den);
      } else
	step = collect("Denominator of ", msg, x.den, step, with_sign);
      break;
    case REENTER:
      step = NUMER_IN;
      if (printf("reenter:\n") == EOF)
	bailout("Inratl: cannot print message.");
      break;
    case CANCEL:
      *x.den = *x.num = 0; 
      reply = NO;
      step = FINISH;
    }
  }
  return (reply);
}
