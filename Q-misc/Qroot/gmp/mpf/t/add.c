#include "gmp.h"


main ()
{
  mpf_t x, y, t;

  mpf_set_default_prec (20);

  mpf_init_set_si (x, 10);
  mpf_init_set_si (y, 1110);
  mpf_init (t);
  mpf_add (t, x, y);

  exit (0);
}
