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

#ifndef NORMAL_KRC
#define NORMAL_KRC

/* The file @(#)normal.h 1.1, created on 3/29/90, contains
 * terminology which I standardly use when writing any C program. It
 * should be  #include'd  in every C file and it is not expected to 
 * change any time soon.
 */

/*   Constants used with true-false data.   */

typedef char bool;

#ifndef TRUE
#define TRUE 1
#define FALSE 0

#define YES 1
#define NO 0
#endif

/*   Constants for return without data   */

#ifndef NULL
#define NULL 0
#endif

/*   Constants used with some input routines.   */

#define BOTTOM 1
#define EMPTY (-1)
#define ERROR (-1)

/*   Constants used to control signflags  */

#ifndef POS
#define POS 0
#define NEG 1
#endif

/* Standard stupid unsafe macros */

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

#endif /* NORMAL_KRC */
