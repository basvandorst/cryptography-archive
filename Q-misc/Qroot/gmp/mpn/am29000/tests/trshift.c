#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

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
ref_mpn_rshift (wp, up, usize, cnt)
     register mp_ptr wp;
     register mp_srcptr up;
     mp_size usize;
     register unsigned int cnt;
{
  register mp_limb high_limb, low_limb;
  register unsigned sh_1, sh_2;
  register mp_size i;
  mp_limb retval;

#ifdef DEBUG
  if (usize == 0 || cnt == 0)
    abort ();
#endif

  sh_1 = cnt;

#if 0
  if (sh_1 == 0)
    {
      if (wp != up)
	{
	  /* Copy from low end to high end, to allow specified input/output
	     overlapping.  */
	  for (i = 0; i < usize; i++)
	    wp[i] = up[i];
	}
      return usize;
    }
#endif

  wp -= 1;
  sh_2 = BITS_PER_MP_LIMB - sh_1;
  high_limb = up[0];
  retval = high_limb << sh_2;
  low_limb = high_limb;

  for (i = 1; i < usize; i++)
    {
      high_limb = up[i];
      wp[i] = (low_limb >> sh_1) | (high_limb << sh_2);
      low_limb = high_limb;
    }
  low_limb >>= sh_1;
  wp[i] = low_limb;

  return retval;
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
  mp_limb dx[SIZE+2];
  mp_limb dy[SIZE+2];
#endif

main (argc, argv)
     int argc;
     char **argv;
{
#ifndef SIM29
  mp_limb s1[SIZE];
  mp_limb dx[SIZE+2];
  mp_limb dy[SIZE+2];
#endif
  int cyx, cyy;
  int i;
  int size;
  int test;
  int cnt = 4;

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

      dx[0] = 0x87654321;
      dy[0] = 0x87654321;
      dx[size+1] = 0x12345678;
      dy[size+1] = 0x12345678;
#endif

#ifdef PRINT
      mpn_print (s1, size);
#endif

#ifdef REF
      for (i = 0; i < TIMES; i++)
	cyx = ref_mpn_rshift (dx + 1, s1, size, cnt);
#endif

#ifdef PRINT
      printf ("%8X ", cyx); mpn_print (dx + 1, size);
#endif

#ifdef NATIVE
      for (i = 0; i < TIMES; i++)
	cyy = mpn_rshift (dy + 1, s1, size, cnt);
#endif

#ifdef PRINT
      printf ("%8X ", cyy); mpn_print (dy + 1, size);
#endif

#if ! defined (SIM29) && defined (REF) && defined (NATIVE)
      if (cyx != cyy || mpn_cmp (dx, dy, size + 2) != 0
	  || dx[size + 1] != 0x12345678 || dx[0] != 0x87654321)
	{
	  printf ("%8X ", cyx); mpn_print (dx, size + 2);
	  printf ("%8X ", cyy); mpn_print (dy, size + 2);
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

