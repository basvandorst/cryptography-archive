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

#ifndef POLYNOMIAL_KRC
#define POLYNOMIAL_KRC


/* The file @(#)polynomial.h 1.1, created on 3/29/90, contains
 * all the definitions necessary to use the routines which do Arithmetic
 * with polynomials of arbitrary degree over finite fields.
 */

#ifndef UTILITY_KRC
#include "utility.h"
#endif

#define Constant(i)    constant(i, FALSE)
#define Size(n)        size(n, FALSE)
#define Showme(a,b)    showme(stdout, a, b, FALSE)
#define Fshowme(a,b,c) showme(a,b,c,FALSE)
#define Collect(a,b)   collect("",a,b,0,FALSE)

#ifndef PMACRO_KRC
#include "Pmacro.h"
#endif

#ifndef ZTOOLS_KRC
#include "Ztools.h"
#endif

extern unsigned short *deriv();
extern unsigned short *proot();

extern void Pinitial();

#define Initial(a) Pinitial(a)

#endif /* POLYNOMIAL_KRC */
