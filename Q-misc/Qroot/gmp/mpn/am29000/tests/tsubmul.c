#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

urandom()
{
  return random() ^ (random() << 1);
}

#ifndef OPS
#define OPS 4000000
#endif
#ifndef SIZE
#define SIZE 1000
#endif
#ifndef TIMES
#define TIMES OPS/SIZE
#endif


mp_limb
ref_mpn_submul_1 (res_ptr, s1_ptr, s1_size, s2_limb)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     mp_size s1_size;
     register mp_limb s2_limb;
{
  register mp_limb cy_limb;
  register mp_size j;
  register mp_limb prod_high, prod_low;
  register mp_limb x;

  /* The loop counter and index J goes from -SIZE to -1.  This way
     the loop becomes faster.  */
  j = -s1_size;

  /* Offset the base pointers to compensate for the negative indices.  */
  res_ptr -= j;
  s1_ptr -= j;

  cy_limb = 0;
  do
    {
      umul_ppmm (prod_high, prod_low, s1_ptr[j], s2_limb);

      prod_low += cy_limb;
      cy_limb = (prod_low < cy_limb) + prod_high;

      x = res_ptr[j];
      prod_low = x - prod_low;
      cy_limb += (prod_low > x);
      res_ptr[j] = prod_low;
    }
  while (++j != 0);

  return cy_limb;
}

#ifdef SIM29
mp_limb s1[SIZE] = {
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,0x72438923,
  };
  mp_limb dx[SIZE+1];
  mp_limb dy[SIZE+1];
#endif

main (argc, argv)
     int argc;
     char **argv;
{
#ifndef SIM29
  mp_limb s1[SIZE];
  mp_limb dx[SIZE+1];
  mp_limb dy[SIZE+1];
#endif
  int cyx, cyy;
  int i;
  int size;
  mp_limb s2_limb;
  int test;

#ifndef TESTS
#define TESTS 100000000
#endif
  for (test = 0; test < TESTS; test++)
    {
#ifdef RANDOM
      size = random () % SIZE + 1;
#else
      size = SIZE;
#endif

#ifndef SIM29
      mpn_random2 (s1, size);
      mpn_random2 (dx, size);
      MPN_COPY (dy, dx, size);
      mpn_random2 (&s2_limb, 1);

      dx[size] = 0x12345678;
      dy[size] = 0x12345678;
#else
      s2_limb = 0x12345678;
#endif

#ifdef PRINT
      mpn_print (s1, size);
#endif

#ifdef REF
      for (i = 0; i < TIMES; i++)
	cyx = ref_mpn_submul_1 (dx, s1, size, s2_limb);
#endif

#ifdef PRINT
      printf ("%8X ", cyx); mpn_print (dx, size);
#endif

#ifdef NATIVE
      for (i = 0; i < TIMES; i++)
	cyy = mpn_submul_1 (dy, s1, size, s2_limb);
#endif

#ifdef PRINT
      printf ("%8X ", cyy); mpn_print (dy, size);
#endif

#ifndef SIM29
      if (cyx != cyy || mpn_cmp (dx, dy, size) != 0
	  || dx[size] != 0x12345678 || dy[size] != 0x12345678)
	{
	  printf ("%d ", cyx); mpn_print (dx, size);
	  printf ("%d ", cyy); mpn_print (dy, size);
	  mpn_print (dx + size, 1);
	  mpn_print (dy + size, 1);
	  abort();
	}
#endif
    }

  exit (0);
}

mpn_print (mp_ptr p, mp_size size)
{
  mp_size i;

  for (i = size - 1; i >= 0; i--)
    printf ("%08X", p[i]);
  puts ("");
}
