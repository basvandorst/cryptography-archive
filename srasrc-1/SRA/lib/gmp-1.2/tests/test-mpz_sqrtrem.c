/* Test mpz_divmod, mpz_mul, mpz_add, and mpz_cmp.

Copyright (C) 1991 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU MP Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU MP Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include "gmp.h"

void debug_mp ();
mp_size _mpn_mul_classic ();
void mpz_refmul ();

main (argc, argv)
     int argc;
     char **argv;
{
  MP_INT x2;
  MP_INT x, rem;
  MP_INT temp;
  mp_size x2_size;
  int i;
  int reps = 10000;

  if (argc == 2)
     reps = atoi (argv[1]);

  mpz_init (&x2);
  mpz_init (&x);
  mpz_init (&rem);
  mpz_init (&temp);

  for (i = 0; i < reps; i++)
    {
      x2_size = random() % 25;

      mpz_random2 (&x2, x2_size);

      mpz_sqrtrem (&x, &rem, &x2);
      mpz_mul (&temp, &x, &x);
      mpz_add (&temp, &temp, &rem);

      if (mpz_cmp (&x2, &temp))
	{
	  fprintf (stderr, "x2="); debug_mp (&x2, 16);
	  fprintf (stderr, "x="); debug_mp (&x, 16);
	  fprintf (stderr, "rem="); debug_mp (&rem, 16);
	  fprintf (stderr, "x*x+rem="); debug_mp (&temp, 16);
	  fprintf (stderr,
	    "bad result in mpz_sqrtrem, mpz_mul, mpz_add\n, or mpz_cmp\n");
	  abort ();
	}
    }

  exit (0);
}

void
debug_mp (x, base)
     MP_INT *x;
{
  mpz_out_str (stderr, base, x); fputc ('\n', stderr);
}
