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

#ifndef MYINPUT_KRC
#define MYINPUT_KRC

/* The file @(#)myinput.h 1.1, created on 3/29/90, contains
 * the constants which control the input routines for Arithmetic; namely,
 * are we inputting integers or fractions, and which step have we reached?
 */

#define NUMER_IN  0
#define DENOM_IN  1
#define FINISH    2
#define REENTER   3
#define CANCEL    4
#define FRAC_IN   5
#define INTEG_IN  6

#endif /* MYINPUT_KRC */


