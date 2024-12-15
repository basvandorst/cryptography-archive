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

/* The file @(#)Zinitial.c 1.1, created on 3/29/90, contains
 * the routine which initializes storage for the multi-precision
 * Arithmetic package.
 */

#include "storage.h"

/* ZINITIAL()
 * sets up the storage for scratch computations.
 */

unsigned short modulus;  /* unused fake to pacify polynomial routines */

warehouse *Z_tmp_WH;
warehouse *Q_tmp_WH;

void
Zinitial(m)
unsigned short m;
{
  modulus = m;
  Z_tmp_WH = makeWH(BLOCKSIZE);
  Q_tmp_WH = makeWH(BLOCKSIZE);
  return;
}
