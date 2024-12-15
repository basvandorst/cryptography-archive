#include "gmp.h"

int verbose;

void
compute_pi (ndigits)
     int ndigits;
{
  mpf_t a, b, pi, x, xi;
  mpf_t t, u;
  mpf_t last_pi;
  int i;
  long t1;

  mpf_set_default_prec ((mp_size_t) (ndigits * 3.3219281));

  mpf_init (a);
  mpf_init (b);
  mpf_init (pi);
  mpf_init (x);
  mpf_init (t);
  mpf_init (u);
  mpf_init (xi);

  mpf_init_set_si (last_pi, 0);

  mpf_sqrt_ui (a, 2);		/* a=sqrt(2); */

  mpf_sqrt (x, a);		/* x=sqrt(a); */

  mpf_add_ui (pi, a, 2);	/* pi=2+a; */

  mpf_ui_div (t, 1, x);
  mpf_add (t, t, x);
  mpf_div_ui (a, t, 2);		/* a=(x+1/x)/2; */

  mpf_set (b, x);		/* b=x; */

  mpf_add_ui (t, a, 1);
  mpf_mul (pi, pi, t);
  mpf_add_ui (t, b, 1);
  mpf_div (pi, pi, t);		/* pi=pi*(a+1)/(b+1); */

  /* printf ("%.16f\n", pi); */

  t1 = cputime ();
  for (i = 1;; i++)
    {
      mpf_sqrt (x, a);		/* x=sqrt(a); */

      mpf_ui_div (xi, 1, x);
      mpf_add (t, xi, x);
      mpf_div_ui (a, t, 2);	/* a=(x+1/x)/2; */

      mpf_mul (t, b, x);
      mpf_add (t, t, xi);
      mpf_add_ui (u, b, 1);
      mpf_div (b, t, u);	/* b=(b*x+1/x)/(b+1); */

      mpf_add_ui (t, a, 1);
      mpf_mul (pi, pi, t);
      mpf_add_ui (t, b, 1);
      mpf_div (pi, pi, t);	/* pi=pi*(a+1)/(b+1); */
      if (verbose == 1)
	{
	  mp_exp_t exp;
	  char *str;
	  long t2 = cputime ();
	  str = mpf_get_str (0, &exp, 10, ndigits, pi);
	  printf ("t=.%s*10**%ld\n", str, exp);
	  free (str);
	  t1 += (cputime () - t2);
	}

#define FUSK
#ifdef FUSK
      if (pi->exp == last_pi->exp)
	{
	  mp_size_t size;
	  size = (last_pi->size < pi->size ? last_pi->size : pi->size) - 1;
	  if (memcmp (pi->d + pi->size - size,
		      last_pi->d + last_pi->size - size,
		      size * 4) == 0)
	    break;
	}
#else
      if (mpf_cmp (pi, last_pi) == 0)
	break;
#endif
      mpf_set (last_pi, pi);
   }

  if (!verbose)
    {
      mp_exp_t exp;
      char *str;
      long t2 = cputime ();
      str = mpf_get_str (0, &exp, 10, ndigits, pi);
      printf ("t=.%s*10**%ld\n", str, exp);
      free (str);
      printf ("printing: %ld ms\n", cputime () - t2);
      t1 += (cputime () - t2);
    }

  printf ("%d iterations\n", i);
  printf ("time for computing pi: %ld ms\n", cputime () - t1);
}

main (argc, argv)
     int argc;
     char **argv;
{
  if (strncmp (argv[1], "-v", 2) == 0)
    {
      verbose = 1;
      argv++;
    }
  if (strncmp (argv[1], "-s", 2) == 0)
    {
      verbose = -1;
      argv++;
    }
  compute_pi (atoi (argv[1]));
  exit (0);
}
