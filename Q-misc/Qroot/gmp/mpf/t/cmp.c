double atof ();

#include "gmp.h"

main (argc, argv)
     char **argv;
{
  mpf_t x, y;
  double xi, yi;

  xi = atof (argv[1]);
  yi = atof (argv[2]);
  mpf_init_set_d (x, xi);
  mpf_init_set_d (y, yi);
  printf ("mpf_cmp    %d\n", mpf_cmp (x, y));
  if (yi >= 0)
    printf ("mpf_cmp_ui %d\n", mpf_cmp_ui (x, (unsigned) yi));
  printf ("mpf_cmp_si %d\n", mpf_cmp_si (x, (signed) yi));
}
