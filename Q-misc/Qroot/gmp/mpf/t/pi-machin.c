#include <stdio.h>
#include "gmp.h"

/* pi = (16 / (1 *   5^1) - 16 / (3 *   5^3) + 16 / (5 *   5^5) - ... ) +
      - ( 4 / (1 * 239^1) -  4 / (3 * 239^3) +  4 / (5 * 239^5) - ... )  */

void *malloc ();

main (argc, argv)
     int argc;
     char **argv;
{
  mpf_t pi;
  mpf_t dacc, eacc;
  long d, e;
  int ndigits;
  mpf_t t;
  mpf_t last_pi;
  int verbose = 0;

  if (strncmp (argv[1], "-v", 2) == 0)
    {
      verbose = 1;
      argv++;
    }

  ndigits = atoi (argv[1]);

  mpf_set_default_prec ((mp_size_t) (ndigits * 3.3219281));

  mpf_init_set_si (last_pi, 0);

  mpf_init (t);
  d = 1;
  mpf_init_set_si (dacc, 5);
  e = 1;
  mpf_init_set_si (eacc, 239);
  mpf_init_set_si (pi, 0);

  for (;;)
    {
      if (mpf_cmp (dacc, eacc) < 0)
	{
	  /* pi += 16 / (d * dacc); */
	  mpf_mul_ui (t, dacc, d);
	  mpf_ui_div (t, 16, t);
	  mpf_add (pi, pi, t);

	  /* dacc *= 25; */
	  mpf_mul_ui (dacc, dacc, 25);

	  d += 2;

	  /* pi -= 16 / (d * dacc); */
	  mpf_mul_ui (t, dacc, d);
	  mpf_ui_div (t, 16, t);
	  mpf_sub (pi, pi, t);

	  /* dacc *= 25; */
	  mpf_mul_ui (dacc, dacc, 25);

	  d += 2;
	}
      else
	{
	  /* pi += 4 / (e * eacc); */
	  mpf_mul_ui (t, eacc, e);
	  mpf_ui_div (t, 4, t);
	  mpf_sub (pi, pi, t);

	  /* eacc *= 57121; */
	  mpf_mul_ui (eacc, eacc, 57121);

	  e += 2;

	  /* pi -= 4 / (e * eacc); */
	  mpf_mul_ui (t, eacc, e);
	  mpf_ui_div (t, 4, t);
	  mpf_add (pi, pi, t);

	  /* eacc *= 57121; */
	  mpf_mul_ui (eacc, eacc, 57121);
	  e += 2;
	}

      if (verbose)
	{
	  mp_exp_t exp;
	  char *str;
	  str = mpf_get_str (0, &exp, 10, ndigits, pi);
	  printf ("t=.%s*10**%ld\n", str, exp);
	  free (str);
	}
      if (mpf_cmp (pi, last_pi) == 0)
	break;
      mpf_set (last_pi, pi);
    }

  if (!verbose)
    {
      mp_exp_t exp;
      char *str;
      str = mpf_get_str (0, &exp, 10, ndigits, pi);
      printf ("t=.%s*10**%ld\n", str, exp);
      free (str);
    }

  exit (0);
}
