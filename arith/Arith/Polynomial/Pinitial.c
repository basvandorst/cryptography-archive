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

/* The file @(#)Pinitial.c 1.1, created on 3/29/90, contains 
 * the routine which initializes the storage and the modulus of the
 * Arithmetic package which works with arbitrary degree polynomials
 * over finite fields.
 */

#include <stdio.h>
#include "polynomial.h"
#include "storage.h"

unsigned short modulus = MODULUS;
unsigned short *INVERSE;
warehouse *Z_tmp_WH;
warehouse *Q_tmp_WH;

extern char *malloc();
extern char *getenv();
extern long strtol();
extern void bailout();

/*  PINITIAL()
 *  makes sure that the modulus is defined and sets up the table of
 *  inverses. It also sets up the storage arrays needed for scratch 
 *  computations.
 */

void
Pinitial(m)
unsigned short m;
{
  char *environ;
  char **safety, *safe, s;
  long converted;
  register int i, j;

  Z_tmp_WH = makeWH(BLOCKSIZE);
  Q_tmp_WH = makeWH(BLOCKSIZE);

  safe = &s;
  safety = &safe;
  if ( m > 0)
    modulus = m;
  else if ((environ = getenv("MODULUS")) != NULL) {
    converted = strtol(environ, safety, 10);
    if (*safety == environ || converted <= 0 || converted > 65535) {
      (void) fprintf(stderr,
	     "Warning: environment variable MODULUS has unexpected value\n");
    } else
      modulus = (unsigned short) converted;
  }

  INVERSE = (unsigned short *) malloc(modulus*sizeof(unsigned short));
  if (INVERSE == NULL)
    bailout("Pinitial: cannot malloc space for inverses.");
  for (i = 0; i < modulus; ++i)
    INVERSE[i] = 0;
  for (i = 1; i < modulus; ++i)
    if (INVERSE[i] == 0) {
      j = 1;
      while (MOD((i * j)) != 1)
	++j;
      INVERSE[i] = j;
      INVERSE[j] = i;
    }
  return;
}
