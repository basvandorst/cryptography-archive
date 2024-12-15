#include "gmp.h"
#include "gmp-impl.h"

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
#if __STDC__
ref_mpn_add_n (mp_ptr res_ptr,
	       mp_srcptr s1_ptr, mp_srcptr s2_ptr, mp_size size)
#else
ref_mpn_add_n (res_ptr, s1_ptr, s2_ptr, size)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_srcptr s2_ptr;
     mp_size size;
#endif
{
  register mp_limb x, y, cy;
  register mp_size j;

  /* The loop counter and index J goes from -SIZE to -1.  This way
     the loop becomes faster.  */
  j = -size;

  /* Offset the base pointers to compensate for the negative indices.  */
  s1_ptr -= j;
  s2_ptr -= j;
  res_ptr -= j;

  cy = 0;
  do
    {
      y = s2_ptr[j];
      x = s1_ptr[j];
      y += cy;			/* add previous carry to one addend */
      cy = (y < cy);		/* get out carry from that addition */
      y = x + y;		/* add other addend */
      cy = (y < x) + cy;	/* get out carry from that add, combine */
      res_ptr[j] = y;
    }
  while (++j != 0);

  return cy;
}

main (argc, argv)
     int argc;
     char **argv;
{
  mp_limb s1[SIZE];
  mp_limb s2[SIZE];
  mp_limb dx[SIZE+1];
  mp_limb dy[SIZE+1];
  int cyx, cyy;
  int i;
  int size;

  for (;;)
    {
      size = random () % SIZE + 1;

      for (i = 0; i < size; i++)
	{
	  s1[i] = urandom();
	  s2[i] = urandom();
	}
      dx[size] = 0x12345678;
      dy[size] = 0x12345678;

#ifdef PRINT
      mpn_print (s1, size);
      mpn_print (s2, size);
#endif

      for (i = 0; i < TIMES; i++)
	cyx = ref_mpn_add_n (dx, s1, s2, size);

#ifdef PRINT
      mpn_print (dx, size);
#endif

      for (i = 0; i < TIMES; i++)
	cyy = mpn_add_n (dy, s1, s2, size);

#ifdef PRINT
      mpn_print (dy, size);
#endif


      if (cyx != cyy || mpn_cmp (dx, dy, size) != 0
	  || dx[size] != 0x12345678 || dy[size] != 0x12345678)
	{
	  printf ("%d ", cyx); mpn_print (dx, size);
	  printf ("%d ", cyy); mpn_print (dy, size);
	  mpn_print (dx + size, 1);
	  mpn_print (dy + size, 1);
	  abort();
	}
    }
}

mpn_print (mp_ptr p, mp_size size)
{
  mp_size i;

  for (i = size - 1; i >= 0; i--)
    printf ("%08X", p[i]);
  puts ("");
}
