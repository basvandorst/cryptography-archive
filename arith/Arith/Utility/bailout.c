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

/* @(#)bailout.c	1.1  3/29/90  */

#include <stdio.h>

/* BAILOUT()
 * is a library function to handle various error conditions that
 * might occur doing very long arithmetic. Programs will often
 * implement their own version that supercedes this - especially if
 * they want to recover and continue processing data automatically
 * from a file.
 */

void
bailout(msg)
char *msg;
{
  (void) fprintf(stderr, "\nError detected -> %s\n", msg);
  exit(1);
}
