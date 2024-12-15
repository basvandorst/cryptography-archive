#include <stdio.h>

void *malloc ();

main (argc, argv)
     int argc;
     char **argv;
{
  double e, last_e;
  double term;
  double x, xpow;
  int ndigits;
  unsigned int div;
  int verbose = 0;
  int i;
  int nn;

  if (argc != 1 && strncmp (argv[1], "-v", 2) == 0)
    {
      verbose = 1;
      argv++;
    }

  for (nn = 0; nn < 100; nn++)
    {
      printf ("--- %d ---\n", nn);

      e = 1.0;
      term = 1.0;
      x = 1.0;
      for (i = 0; i < nn; i++)
	x /= 2.0;
      div = 1;

      for (i = 0;; i++)
	{
	  term *= x;
	  term /= div;
	  e += term;
	  div++;

	  if (verbose)
	    printf ("t=%.15f\n", e);

	  if (e == last_e)
	    break;
	  last_e = e;
	}

      printf ("n iterations: %d\n", i);

      for (i = 0; i < nn; i++)
	e *= e;

      if (!verbose)
	printf ("t=%.15f\n", e);
    }
}

