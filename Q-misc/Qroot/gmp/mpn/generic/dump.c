#include "gmp.h"

void
mpn_dump (ptr, size)
     mp_srcptr ptr;
     mp_size_t size;
{
  if (size == 0)
    printf ("0\n");
  {
    while (size)
      {
	size--;
	printf ("%0*X", 2 * sizeof (mp_limb), ptr[size]);
      }
    printf ("\n");
  }
}
