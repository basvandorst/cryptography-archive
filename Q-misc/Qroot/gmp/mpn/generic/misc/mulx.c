#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"


#ifdef STAT
#define USIZE 100
#define VSIZE 100
unsigned short mult_counts[USIZE][VSIZE];
#endif

mp_limb
#if __STDC__
mpn_mul (mp_ptr prodp,
	  mp_srcptr up, mp_size usize,
	  mp_srcptr vp, mp_size vsize)
#else
mpn_mul (prodp, up, usize, vp, vsize)
     mp_ptr prodp;
     mp_srcptr up;
     mp_size usize;
     mp_srcptr vp;
     mp_size vsize;
#endif
{
  mp_size n;
  mp_size prod_size;
  mp_limb cy;
  mp_size i, j;
  mp_limb prod_low, prod_high;
  mp_limb cy_limb;
  mp_limb v_limb, c;

#ifdef STAT
  mult_counts[usize][vsize]++;
#endif

  if (vsize == 0)
    return 0;

  /* Multiply by the first limb in V separately, as the result can
     be stored (not added) to PROD.  We also avoid a loop for zeroing.  */
  v_limb = vp[0];
  if (v_limb <= 1)
    {
      if (v_limb == 1)
	MPN_COPY (prodp, up, usize);
      else
	MPN_ZERO (prodp, usize);
      cy_limb = 0;
    }
  else
    cy_limb = mpn_mul_1 (prodp, up, usize, v_limb);

  prodp[usize] = cy_limb;
  prodp++;

  /* For each iteration in the outer loop, multiply one limb from
     U with one limb from V, and add it to PROD.  */
  for (i = 1; i < vsize; i++)
    {
      v_limb = vp[i];
      if (v_limb <= 1)
	{
	  cy_limb = 0;
	  if (v_limb == 1)
	    cy_limb = mpn_add_n (prodp, prodp, up, usize);
	}
      else
	cy_limb = mpn_addmul_1 (prodp, up, usize, v_limb);

      prodp[usize] = cy_limb;
      prodp++;
    }

  return cy_limb;
}
