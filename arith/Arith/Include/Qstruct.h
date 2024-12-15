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

#ifndef QSTRUCT_KRC
#define QSTRUCT_KRC

/* The file @(#)Qstruct.h 1.1, created on 3/29/90, contains
 * the definitions which are common to both fraction.h and rational.h as
 * part of the Arithmetic package. Most higher level routines need
 * a single object file which includes this header file. Exceptions
 * occur with any routines which need  Inratl  or  Ratconst.
 */

#ifndef NORMAL_KRC
#include "normal.h"
#endif

typedef struct {
  unsigned short *num;
  unsigned short *den;
} ratl;

extern void Ratcopy();

extern ratl Qadd();
extern ratl Qmult();
extern ratl Qminus();
extern ratl Qsub();
extern bool Qeq();
extern ratl Qinv();
extern ratl Qdiv();
extern ratl Reduce();

#endif /* QSTRUCT_KRC */
