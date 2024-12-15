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

#ifndef ZTOOLS_KRC
#define ZTOOLS_KRC

/* The file @(#)Ztools.h 1.1, created on 3/29/90, contains
 * the definitions of functions which are common to integer.h and
 * polynomial.h in the Arithmetic package.
 */

extern unsigned short *Zadd();
extern unsigned short *Zmult();
extern unsigned short *Zminus();
extern unsigned short *Zsub();
extern unsigned short *Zdivalg();
extern void Zgcd();
extern bool Zeq();
extern bool Zlessoreq();
extern void Zshortmult();

#endif /* ZTOOLS_KRC */
