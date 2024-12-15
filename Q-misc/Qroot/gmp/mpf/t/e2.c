#include <stdio.h>
#include "gmp.h"

void *malloc ();

main (argc, argv)
     int argc;
     char **argv;
{
  mpf_t e, last_e;
  mpf_t term;
  int ndigits;
  unsigned int div;
  int verbose = 0;
  int i;
  long t;
  int nn;

  if (strncmp (argv[1], "-v", 2) == 0)
    {
      verbose = 1;
      argv++;
    }

  ndigits = atoi (argv[1]);

  mpf_set_default_prec ((mp_size_t) (ndigits * 3.3219281));

  for (nn = 0; nn < 100; nn++)
    {
      printf ("--- %d ---\n", nn);

      mpf_init_set_si (e, 1);
      mpf_init (last_e);
      mpf_init_set_si (term, 1);
      div = 1;

      t = cputime ();
      for (i = 0;; i++)
	{
	  mpf_div_2exp (term, term, nn);
	  mpf_div_ui (term, term, div);
	  mpf_add (e, e, term);
	  div++;

	  if (verbose)
	    {
	      mp_exp_t exp;
	      char *str;
	      long t2 = cputime ();
	      str = mpf_get_str (0, &exp, 10, ndigits, e);
	      printf ("t=.%s*10**%ld\n", str, exp);
	      free (str);
	      t += (cputime () - t2);
	    }

	  if (mpf_cmp (e, last_e) == 0)
	    break;
	  mpf_set (last_e, e);
	}

      printf ("n iterations: %d\n", i);

#if MPF_BUG
      e->d[e->size - 1] = 1;
#endif
      for (i = 0; i < nn; i++)
	mpf_mul (e, e, e);

      if (!verbose)
	{
	  mp_exp_t exp;
	  char *str;
	  long t2 = cputime ();
	  str = mpf_get_str (0, &exp, 10, ndigits, e);
	  printf ("t=.%s*10**%ld\n", str, exp);
	  free (str);
	  printf ("printing: %ld ms\n", cputime () - t2);
	  t += (cputime () - t2);
	}
      printf ("time for computing e: %ld ms\n", cputime () - t);
    }
}

