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

#ifndef STORAGE_KRC
#define STORAGE_KRC

/* The file @(#)storage.h 1.1, created on 3/29/90, contains
 * the definitions of structures and external definitions of functions
 * which manipulate them to maintain storage of very long integers and
 * polynomials by the Arithmetic package.
 */

struct WH {
  unsigned short *array;
  int nextin;
  unsigned depth;
  struct WH *newer;
};

typedef struct WH warehouse;

#define BLOCKSIZE 1024

extern warehouse *makeWH();
extern unsigned short *capture();
extern void release();

#endif /* STORAGE_KRC */
