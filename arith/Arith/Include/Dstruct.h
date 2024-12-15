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

#ifndef DSTRUCT_KRC
#define DSTRUCT_KRC

/* The file @(#)Dstruct.h, version 1.1, created on 3/29/90, 
 * contains the structural information for the part of Arithmetic dealing
 * with rings of integers in quadratic fields.
 */

#ifndef INTEGER_KRC
#include "integer.h"
#endif

extern unsigned short *DB;              /* Basic integer in Q(sqrt(D)). */
                             /* Must be either sqrt(D) or (1+sqrt(D))/2 */
                  /* A general element is of the form   alpha + beta*DB */

typedef struct quad {
  unsigned short *alpha;
  unsigned short *beta;
} quad;

extern bool Deq();
extern quad Dmult();
extern quad Dadd();
extern quad Dsub();
extern quad Dminus();
extern unsigned short *Norm();

#define SHORT(x) (*(x + *(x) - 1))

#endif /* DSTRUCT_KRC */
