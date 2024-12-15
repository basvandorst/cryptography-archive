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

#ifndef UTILITY_KRC
#define UTILITY_KRC

/* The file @(#)utility.h 1.1, created on 3/29/90, contains
 * the external definitions of all of the standard utility functions used
 * with the programs to do Arithmetic with very long integers or polynomials.
 * Normally, this gets included through one of the header files
 *      integer.h, fraction.h, polynomial.h, or rational.h.
 * Applications which require utilities which need additional
 * structures must include additional headers. In particular, 
 * fraction.h and rational.h must define the structures needed
 * to use the utilities  inratl  and  ratconst, and any routines
 * which use the storage utilities  capture  or  release  must 
 * also include  "storage.h".
 */

#ifndef NORMAL_KRC
#include "normal.h"
#endif

/* general utilities */

extern void bailout();
extern void Shcopy();
extern unsigned short size();
extern unsigned short *constant();

/* input - output utilities */

extern bool more();
extern bool collect();
extern void showme();

#endif /* UTILITY_KRC */
