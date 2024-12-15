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

/*  @(#)ratconst.c	1.1  3/29/90  */

#include "normal.h"
#include "Qstruct.h"

extern void Shcopy();
extern unsigned short *constant();

static unsigned short _numer[3];
static unsigned short _denom[3];
static ratl c = { _numer, _denom };

/* RATCONST()
 *     This function assumes it is given a long number i which can be
 * represented by an unsigned short and returns a rational structure 
 * with that value. In general, the return value should be copied to
 * a safe location.
 */

ratl 
ratconst(i, with_sign)
long i;
bool with_sign;
{
  Shcopy(constant(i, with_sign), _numer);
  Shcopy(constant(1L, with_sign), _denom);
  return (c);
}
