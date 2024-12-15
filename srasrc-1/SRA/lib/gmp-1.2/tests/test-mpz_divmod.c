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
#include <gmp.h>

void debug_mp ();

main (argc, argv)
     int argc;
     char **argv;
{
  MP_INT dividend, divisor;
  MP_INT quotient, remainder;
  MP_INT quotient2, remainder2;
  MP_INT temp;
  mp_size dividend_size, divisor_size;
  int i;
  int reps = 10000;

  if (argc == 2)
     reps = atoi (argv[1]);

  mpz_init (&dividend);
  mpz_init (&divisor);
  mpz_init (&quotient);
  mpz_init (&remainder);
  mpz_init (&quotient2);
  mpz_init (&remainder2);
  mpz_init (&temp);

  for (i = 0; i < reps; i++)
    {
      divisor_size = (random() << 1) % 25;
      dividend_size = (random() << 1) % 25;

      mpz_random2 (&dividend, dividend_size);
      mpz_random2 (&divisor, divisor_size);
      if (!mpz_cmp_ui (&divisor, 0))
	continue;

      mpz_divmod (&quotient, &remainder, &dividend, &divisor);
      mpz_div (&quotient2, &dividend, &divisor);
      mpz_mod (&remainder2, &dividend, &divisor);

      if (mpz_cmp_ui (&quotient, 0) != 0)
      if ((mpz_cmp_ui (&quotient, 0) < 0)
	  != ((mpz_cmp_ui (&dividend, 0) ^ mpz_cmp_ui (&divisor, 0)) < 0))
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr,
	    "incorrect sign of quotient in mpz_divmod and mpz_div\n");
	  abort ();
	}
      if (mpz_cmp_ui (&remainder, 0) != 0)
      if ((mpz_cmp_ui (&remainder, 0) < 0) !=  (mpz_cmp_ui (&dividend, 0) < 0))
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr,
	    "incorrect sign of remainder in mpz_divmod and mpz_mod\n");
	  abort ();
	}

      if (mpz_cmp (&quotient, &quotient2))
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr,
	    "different result in mpz_divmod and mpz_div\n");
	  abort ();
	}
      if (mpz_cmp (&remainder, &remainder2))
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr, "different result in mpz_divmod and mpz_mod\n");
	  abort ();
	}

      mpz_mul (&temp, &quotient, &divisor);
      mpz_add (&temp, &temp, &remainder);
      if (mpz_cmp (&temp, &dividend))
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr,
	    "bad result in mpz_divmod, mpz_mul, mpz_add, or mpz_cmp\n");
	  abort ();
	}

      mpz_abs (&remainder, &remainder);
      mpz_abs (&divisor, &divisor);
      if (mpz_cmp (&remainder, &divisor) >= 0)
	{
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 10);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 10);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 10);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 10);
	  fprintf (stderr, "dividend="); debug_mp (&dividend, 16);
	  fprintf (stderr, "divisor="); debug_mp (&divisor, 16);
	  fprintf (stderr, "quotient="); debug_mp (&quotient, 16);
	  fprintf (stderr, "remainder="); debug_mp (&remainder, 16);
	  fprintf (stderr,
	    "bad result in mpz_divmod or mpz_cmp: remainder too big.\n");
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
