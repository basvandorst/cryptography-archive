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

#ifndef FRACTION_KRC
#define FRACTION_KRC

/* The file @(#)fraction.h 1.1, created on 3/29/90, contains
 * the definitions necessary to do arbitrary precision Arithmetic
 * with the rational numbers.
 */

#ifndef INTEGER_KRC
#include "integer.h"
#endif

#ifndef QSTRUCT_KRC
#include "Qstruct.h"
#endif

extern bool inratl();
extern void ratshowme();
extern ratl ratconst();
extern void Ratcopy();

#define Inratl(a,b,c)     inratl(a, b, c, TRUE)
#define Ratshowme(a,b)    ratshowme(stdout, a, b, TRUE)
#define FRatshowme(a,b,c) ratshowme(a, b, c, TRUE)
#define Ratconst(a)       ratconst(a, TRUE)

#endif /* FRACTION_KRC */
