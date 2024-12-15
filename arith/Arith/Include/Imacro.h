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

#ifndef IMACRO_KRC
#define IMACRO_KRC

/*
 * The file @(#)Imacro.h, version 1.1, created on 3/29/90,
 * contains the macros used in the routines which do Arithmetic with very
 * long integers.
 */

#define SHORTMASK 00000177777
#define SHORTLENGTH 16
#define NEGSIGNBIT 65535
#define LONGLENGTH 32

#define MINUS(x) ((NEGSIGNBIT-x)+1)
#define SIGNUM(a) (*((a) + *(a)))

#endif /* IMACRO_KRC */
