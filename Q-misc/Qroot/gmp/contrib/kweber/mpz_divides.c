/*	File mpz_divides.c

	mpz_divides(vz, uz)  returns the boolean value (mpz_mod(u, v) != 0), 
			    only calculates it faster.

	Based on mpn_bmod(u, ul, v, vl), a shift remainder operation similar to
	Norton's smod operation.

	Ken Weber (kweber@mcs.kent.edu)		06/08/93

	Refer to 
	    K. Weber, The accelerated integer gcd algorithm, 
	    Dept. of Mathematics and Computer Science, 
	    Kent State University, 1993.
*/

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#define GEQ_AT_HIGH_LIMBS(u, ul, v, vl) \
    ((ul)>(vl) || ((ul)==(vl) && ((ul)<=0 || (u)[(ul)-1]>=(v)[(vl)-1])))

/*  True iff ((u, ul) >> t >= (v, vl) >> t)
    where t = BITS_PER_MP_LIMB*(MIN(ul, vl) - 1)
*/

int mpz_divides(const MP_INT *vz, const MP_INT *uz)

{   mp_ptr u, v;
    mp_size_t ul, vl, uzero_limbs, vzero_limbs;
    int ur, vr;

    /*  vz == 0, so we'll say vz DOES NOT divide uz  */
    if (vz->size == 0) return 0;

    /*  uz == 0  */
    if (uz->size == 0) return !0;

    u = uz->d, v = vz->d, ul = abs(uz->size), vl = abs(vz->size);

    /*  Check quickly to see if uz might be less than vz  */
    if (!GEQ_AT_HIGH_LIMBS(u, ul, v, vl))  return 0;

    /*  If less 2s divide u than v, return 0.
	Otherwise rshift values to temporary storage.
    */
    while (*u == 0) u++;
    while (*v == 0) v++;

    uzero_limbs = u - uz->d, vzero_limbs = v - vz->d;

    if (uzero_limbs < vzero_limbs) return 0;

    count_trailing_zeros(ur, *u);
    count_trailing_zeros(vr, *v);

    if (uzero_limbs == vzero_limbs && ur < vr) return 0;

    ul -= uzero_limbs, vl -= vzero_limbs;

    u = (mp_ptr)alloca(sizeof(mp_limb)*(ul+vl)), v = u + ul;

    if (ur)
    {   (void)mpn_rshift(u, uz->d+uzero_limbs, ul, ur);
	if (u[ul-1] == 0) ul--;
    }
    else
	MPN_COPY(u, uz->d+uzero_limbs, ul);

    if (vr)
    {   (void)mpn_rshift(v, vz->d+vzero_limbs, vl, vr);
	if (v[vl-1] == 0) vl--;
    }
    else
	MPN_COPY(v, vz->d+vzero_limbs, vl);

    /*  Only interested in size of return value -- not shift remainder */
    return (mpn_bmod(u, ul, v, vl) == 0);
}

/*  End mpz_divides.c  */
