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

/*  @(#)showme.c	1.1  3/29/90  */

#include <stdio.h>
#include "normal.h"
#include "Imacro.h"

#define DIGITLIST 30

extern free();
extern void bailout();
extern unsigned short *negate();

/* SHOWME()
 * displays a very long integer or polynomial. The parameter  a  points
 * to a string of digits. The first digit is the length of the string.
 * Both integers and polynomials are stored in little-endian order. In the
 * case of integers, the highest order digit is the signbit.
 */

void
showme(outfile, msg, a, with_sign)
FILE *outfile;
char *msg;
unsigned short *a;
bool with_sign;
{
  register short j;    /* loop variable */
  short count;         /* how many digits have been printed on this line */
  unsigned short *b;   /* temporary pointer for negation */

  if (fprintf(outfile, "%s ", msg) == EOF)
    bailout("Showme: cannot print message.");

  if (*a == 0) {                           /* special cases */
    if (fprintf(outfile, "0\n") == EOF)
      bailout("Showme: cannot print.");
  } else if (with_sign == TRUE && SIGNUM(a) == NEGSIGNBIT) {
    b = negate(a);
    showme(outfile, "MINUS", b, with_sign);
    free((char *)b);
  } else {

    if (with_sign == TRUE)
      j = *a - 1;
    else
      j = *a;
    count = 0;
    while (j > 1 && *(a+j) == 0)   /* don't print leading zeros */
      --j;
    while (j > 0) {
      ++count;
      if (count > DIGITLIST) {
	if (fprintf(outfile, " &\n(more %s) ", msg) == EOF)
	  bailout("Showme: Cannot print message.");
	count = 0;
      }
      if (fprintf(outfile, " %d", *(a + j)) == EOF)
	bailout("Showme: Cannot print data.");
      --j;
    }
    if (fprintf(outfile, "\n") == EOF)
      bailout("Showme: Cannot complete printing.");
  }
  return;
}
