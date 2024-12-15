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

#ifndef RATIONAL_KRC
#define RATIONAL_KRC

/* The file @(#)rational.h 1.1, created on 3/29/90, contains
 * all the definitions necessary to use the routines for doing Arithmetic
 * with rational functions of arbitrary degree over finite fields.
 */

#ifndef POLYNOMIAL_KRC
#include "polynomial.h"
#endif

#ifndef QSTRUCT_KRC
#include "Qstruct.h"
#endif

extern bool inratl();
extern void ratshowme();
extern ratl ratconst();
extern void Ratcopy();

#define Inratl(a,b,c) inratl(a,b,c,FALSE)
#define Ratshowme(a,b)    ratshowme(stdout, a, b, FALSE)
#define FRatshowme(a,b,c) ratshowme(a, b, c, FALSE)
#define Ratconst(a)   ratconst(a,FALSE)

#endif /* RATIONAL_KRC */
