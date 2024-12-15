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

/* The file @(#)blurb.c 1.1, created on 3/29/90, contains
 * routine to ensure that interactive programs display the copyright
 * information.
 */

#include <stdio.h>

void
blurb(name, year, author)
char *name, *year, *author;
{
  (void) fprintf(stderr, "%s Copyright (C) %s %s\n", name, year, author);
  (void) fprintf(stderr, "%s comes with ABSOLUTELY NO WARRANTY.\n"); 
  (void) fprintf(stderr, "This is free software, and you are welcome to\n");
  (void) fprintf(stderr, "redistribute it under certain conditions.\n");
  (void) fprintf(stderr, "For full details, see the COPYRIGHT file which\n");
  (void) fprintf(stderr, "came with this distribution.\n\n");
  return;
}
