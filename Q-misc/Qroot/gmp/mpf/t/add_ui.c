#include "gmp.h"


main ()
{
  mpf_t x, t;
  int i;
  char *str;
  mp_exp_t exp;
  int e;

  mpf_set_default_prec (33);

  mpf_init_set_si (x, 0);
  mpf_init (t);
  mpf_add_ui (t, x, 100);

  for (e = 0; e < 100; e++)
    {
      printf ("### %d ###\n", e);
      str = mpf_get_str (0, &exp, 10, 100, t);
      printf ("t=.%s*10**%ld\n", str, exp);

      printf ("size=%ld\n", t[0].size);
      printf ("exp=%ld\n", t[0].exp);
      for (i = t[0].size - 1; i >= 0; i--)
	printf ("%lX ", t[0].d[i]);
      puts ("\n");

      mpf_div_2exp (t, t, 1);
    }

  exit (0);
}
