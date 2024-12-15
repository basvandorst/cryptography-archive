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

#ifndef INTEGER_KRC
#define INTEGER_KRC

/* The file @(#)integer.h 1.1, created on 3/29/90, contains
 * the definitions necessary to do arbitrary precision Arithmetic
 * with integers.
 */

#ifndef UTILITY_KRC
#include "utility.h"
#endif

#define Constant(i)    constant(i, TRUE)
#define Size(n)        size(n, TRUE)
#define Showme(a,b)    showme(stdout, a, b, TRUE)
#define Fshowme(a,b,c) showme(a,b,c,TRUE)
#define Collect(a,b)   collect("",a,b,0,TRUE)

#ifndef IMACRO_KRC
#include "Imacro.h"
#endif

#ifndef ZTOOLS_KRC
#include "Ztools.h"
#endif

extern void Zinitial();

#define Initial(a)    Zinitial(a)

#endif /* INTEGER_KRC */

