/* mpn_sub_n_rsh -- right-shift difference of low n limbs of 2 pos. integers.

	Ken Weber (kweber@mcs.kent.edu)		07/21/93

*/

#include "gmp.h"
#include "gmp-impl.h"

/*  
   Subtract S2 (pointed to by S2_PTR and SIZE limbs long) from S1,
   shift result CNT bits to the right, and store the SIZE least
   significant limbs of the result at RES_PTR.  Return the low limb
   of the difference shifted left BITS_PER_MP_LIMB - CNT added to
   the borrow out.

   Argument constraints:
   1. 0 < CNT < BITS_PER_MP_LIMB
   2. SIZE > 0.
   3. If the result is to be written over the input, WP must be <= UP.

    I fiddled with this code quite a while.  It works faster if you 
    1.  Make sure the cpu has something to do while the operands are being 
	fetched.
    2.  Use the ugly goto method instead of the nice method from mpn_sub_n.

    This routine is a good candidate for assembly-level code.

	Ken Weber (kweber@mcs.kent.edu)		07/15/93

*/

mp_limb
#if __STDC__
mpn_sub_n_rsh (mp_ptr res_ptr,
	       mp_srcptr s1_ptr, mp_srcptr s2_ptr, mp_size_t size,
	       unsigned int cnt)
#else
mpn_sub_n_rsh (res_ptr, s1_ptr, s2_ptr, size, cnt)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_srcptr s2_ptr;
     mp_size_t size;
     register unsigned int cnt;
#endif
{
  register mp_limb x, y, this, prev, retval;
  register mp_srcptr s1_end = s1_ptr + size;
  register unsigned sh_1, sh_2;

#ifdef DEBUG
  if (size == 0 || cnt == 0)
    abort ();
#endif

  sh_1 = cnt;
  sh_2 = BITS_PER_MP_LIMB - sh_1;

  x = *s1_ptr++;
  y = *s2_ptr++;
  this = x - y;
  retval = this << sh_2;
  res_ptr -= 1;			/*  Preincrementing used here  */


  if (x < y)			/*  BORROW  LOOP  */
    {
      while (s1_ptr != s1_end)
	{
	  x = *s1_ptr++;
	  y = *s2_ptr++;
	  res_ptr += 1;
	  prev = this >> sh_1;
	  this = x - y - 1;
	  if (x > y) goto no_borrow;
	borrow:
	  prev |= this << sh_2;
	  *res_ptr = prev;
	}
	*++res_ptr = this >> sh_1;
	return 1 + retval;
    }
  else				/*  NO BORROW LOOP  */
    {
      while (s1_ptr != s1_end)
	{
	  x = *s1_ptr++;
	  y = *s2_ptr++;
	  res_ptr += 1;
	  prev = this >> sh_1;
	  this = x - y;
	  if (x < y) goto borrow;
	no_borrow:
	  prev |= this << sh_2;
	  *res_ptr = prev;
	}
      *++res_ptr = this >> sh_1;
      return retval;
    }
}
