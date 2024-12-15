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

void mpz_refgcd (), debug_mp ();

main (argc, argv)
     int argc;
     char **argv;
{
  MP_INT op1, op2;
  MP_INT gcd1, gcd2, gcd, s, t;
  MP_INT temp1, temp2;
  mp_size op1_size, op2_size;
  int i;
  int reps = 1000;

  if (argc == 2)
     reps = atoi (argv[1]);

  mpz_init (&op1);
  mpz_init (&op2);
  mpz_init (&gcd1);
  mpz_init (&gcd2);
  mpz_init (&gcd);
  mpz_init (&temp1);
  mpz_init (&temp2);
  mpz_init (&s);
  mpz_init (&t);

  for (i = 0; i < reps; i++)
    {
      op1_size = random() % 25;
      op2_size = random() % 25;

      mpz_random2 (&op1, op1_size);
      mpz_random2 (&op2, op2_size);

      mpz_gcd (&gcd1, &op1, &op2);
      mpz_gcdext (&gcd2, &s, &t, &op1, &op2);
      mpz_mul (&temp1, &s, &op1);
      mpz_mul (&temp2, &t, &op2);
      mpz_add (&gcd2, &temp1, &temp2);
      mpz_refgcd (&gcd, &op1, &op2);

      if (mpz_cmp (&gcd, &gcd1))
	{
	  debug_mp(&op1, 10);
	  debug_mp(&op2, 10);
	  debug_mp(&gcd, 10);
	  debug_mp(&gcd1, 10);
	  fprintf (stderr,
	    "bad result in mpz_gcd or in function called by mpz_refgcd\n");
	  abort ();
	}
      if (mpz_cmp (&gcd, &gcd2))
	{
	  fprintf (stderr,
	    "bad result in mpz_gcdext or in function called by mpz_refgcd\n");
	  abort ();
	}
      if (mpz_cmp (&gcd, &gcd2))
	{
	  fprintf (stderr,
	    "bad result in mpz_gcdext\n");
	  abort ();
	}
    }

  exit (0);
}

void
mpz_refgcd (g, x, y)
     MP_INT *g;
     MP_INT *x, *y;
{
  MP_INT xx, yy;

  mpz_init_set (&xx, x);
  mpz_init_set (&yy, y);

  for (;;)
    {
      if (mpz_cmp_ui (&yy, 0) == 0)
	{
	  mpz_set (g, &xx);
	  break;
	}
      mpz_mod (&xx, &xx, &yy);
      if (mpz_cmp_ui (&xx, 0) == 0)
	{
	  mpz_set (g, &yy);
	  break;
	}
      mpz_mod (&yy, &yy, &xx);
    }

  mpz_clear (&xx);
  mpz_clear (&yy);
}

void
debug_mp (x, base)
     MP_INT *x;
{
  mpz_out_str (stderr, base, x); fputc ('\n', stderr);
}
