/*	tst-div.c:  Test mpz_gcd(), mpz_divides()
	

	Ken Weber (kweber@mcs.kent.edu)		06/18/93

*/

#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"
#include "urandom.h"

void mpz_refgcd (), debug_mp ();
int mpz_refdivides ();

/*  Notice that SIZE is much bigger than intst-gcd.  This is necessary
    to exercise mpn_accelgcd() and mpn_bmodgcd().
*/
#ifndef SIZE
#define SIZE 64
#endif

main (argc, argv)
     int argc;
     char **argv;
{
  MP_INT op1, op2;
  MP_INT refgcd, gcd;
  mp_size_t op1_size, op2_size;
  int ref_div, div;
  int i;
  int reps = 2000;	/*  Since numbers are bigger reps is smaller  */

  if (argc == 2)
     reps = atoi (argv[1]);

  mpz_init (&op1);
  mpz_init (&op2);
  mpz_init (&refgcd);
  mpz_init (&gcd);

  for (i = 0; i < reps; i++)
    {
      op1_size = urandom () % SIZE;
      op2_size = urandom () % SIZE;

      mpz_random2 (&op1, op1_size);
      mpz_random2 (&op2, op2_size);

      mpz_refgcd (&refgcd, &op1, &op2);

      mpz_gcd (&gcd, &op1, &op2);
      if (mpz_cmp (&refgcd, &gcd))
	dump_abort (&op1, &op2);

      ref_div = mpz_refdivides(&op1, &op2);
      div = mpz_divides(&op1, &op2);

      if (!ref_div != !div)
	dump_abort (&op1, &op2);

      mpz_mul(&op1, &op1, &op2);	
      /*  Make sure there are some that divide  */
      
      ref_div = mpz_refdivides(&op1, &op2);
      div = mpz_divides(&op1, &op2);

      if (!ref_div != !div)
	dump_abort (&op1, &op2);


    }

  exit (0);
}

void
mpz_refgcd (g, x, y)
     MP_INT *g;
     MP_INT *x, *y;
{
  MP_INT xx, yy;

  mpz_init (&xx);
  mpz_init (&yy);

  mpz_abs (&xx, x);
  mpz_abs (&yy, y);

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

dump_abort (op1, op2)
     MP_INT *op1, *op2;
{
  fprintf (stderr, "ERROR\n");
  fprintf (stderr, "op1 = "); debug_mp (op1, -16);
  fprintf (stderr, "op2 = "); debug_mp (op2, -16);
  abort();
}

void
debug_mp (x, base)
     MP_INT *x;
{
  mpz_out_str (stderr, base, x); fputc ('\n', stderr);
}

int mpz_refdivides(const MP_INT *v, const MP_INT *u)
{   
    static int init = 1;
    static MP_INT r[1];
    if (mpz_cmp_ui(v, 0) == 0) return 0;
    if (mpz_cmp_ui(u, 0) == 0) return !0;
    if (init) init = 0, mpz_init(r);
    mpz_mod(r,u,v);
    return (mpz_cmp_ui(r, 0) == 0);
}
