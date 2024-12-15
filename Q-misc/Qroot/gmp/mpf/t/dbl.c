#include "gmp.h"

double atof ();

main (argc, argv)
     int argc;
     char **argv;
{
  mpf_t t;
  char *str;
  mp_exp_t exp;
  double d;
  int i;


  mpf_set_default_prec (100);

  mpf_init (t);

  for (i = 1; i < argc; i++)
    {
      d = atof (argv[i]);
      mpf_set_d (t, d);

      str = mpf_get_str (0, &exp, 10, 5, t);
      printf ("t=.%s*10**%ld (%.16g)\n", str, exp, d);
    }

  exit (0);
}
