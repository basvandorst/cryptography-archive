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

/*  @(#)more.c	1.1  3/29/90  */

#include <stdio.h>
#include "normal.h"

extern void bailout();

/* MORE()
 * reads in the answer to the question :
 * Do you want to enter another variable of the form _____?
 * Only recognizes the first occurrence of n, N, y, or Y, and
 * flushes the rest of the line. 
 */

bool 
more(msg)
char *msg;
{
  register int reply;

  if (printf(" %s? ", msg) == EOF)
    bailout("More: cannot print message.");

  do {
    if ((reply = getchar()) == EOF) {
      clearerr(stdin);
      bailout("More: end of file encountered.");

    } else if (reply == '\n') {
      break;

    } else if (reply == 'y' || reply == 'Y') {
      while (getchar() != '\n')
	;
      return (YES);

    } else if (reply == 'n' || reply == 'N') {
      while (getchar() != '\n')
	;
      return (NO);
    }
  } while (1);

  if (printf("Yes or no answer required.\n") == EOF)
    bailout("More: cannot print message.");

  return (more(msg));
}
