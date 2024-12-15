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

/*  @(#)constant.c	1.1  3/29/90  */

#include "normal.h"
#include "Imacro.h"
#include "Pmacro.h"

extern void bailout();

static unsigned short theconstant[3];
static unsigned short *c = theconstant;

/* CONSTANT()
 *     This function assumes it is given a long number i which can be 
 * represented as a single unsigned short and returns a very long integer
 * with that value. In the case of polynomials, the input is computed
 * modulo the usual prime. In general, the return value should be copied 
 * to a safe location.
 */

unsigned short *
constant(i, with_sign)
long i;
bool with_sign;
{
  if (i == 0) {
    *c = 0;
  } else if (with_sign == TRUE) {
    *c = 2;
    if (i > NEGSIGNBIT || 1-i > NEGSIGNBIT)
      bailout("Constant: out of range.");
    else if (i < 0) {
      *(c+1) = MINUS((-i));
      *(c+2) = NEGSIGNBIT;
    } else {
      *(c+1) = i;
      *(c+2) = 0;
    }
  } else {
    *c = 1;
    *(c+1) = MOD(i);
  }
  return (c);
}
