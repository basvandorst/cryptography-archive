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

#ifndef MSTRUCT_KRC
#define MSTRUCT_KRC

/* The file @(#)Mstruct.h 1.1, created on 3/29/90, contains
 * the standard definitions used throughout the program multiples
 * and other studies of the arithmetic of genus two curves.
 *
 */

#ifndef QSTRUCT_KRC
#include "Qstruct.h"
#endif

/* constants to distinguish types of points on the Jacobian */

#define ORIGIN 0
#define THETA  1
#define PAIR   2

/*   Special data structures created for multiples.   */

typedef struct {
  ratl  x;
  ratl  y;
} point;

typedef struct {
  ratl  xsum;		/* x1+x2  */
  ratl  xprod;		/* x1*x2  */
  ratl  ydiff;		/* (y1-y2)/(x1-x2)  */
  ratl  mix;		/* (x2y1-x1y2)/(x1-x2)   */
} twopoint;

union jacobian {
  short origin;
  point theta;
  twopoint pair;
} ;

typedef struct {	/* general element of the jacobian */
  short utype;
  union jacobian jac;
} point_pair;

#endif /* MSTRUCT_KRC */
