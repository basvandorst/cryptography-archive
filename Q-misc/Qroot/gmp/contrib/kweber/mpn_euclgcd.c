/*	File mpn_euclgcd.c
	
	Preconditions:
	1.  ul >= vl.
	2.  (u, ul) and (v, vl) both odd.
	    This is used to lessen the cost of normalizing the denominator.

*/

#include "gmp+.h"
#include "gmp-impl+.h"
#include "longlong+.h"

mp_size_t mpn_euclgcd(mp_ptr g, mp_ptr u, mp_size_t ul, mp_ptr v, mp_size_t vl)

{   unsigned long int d;
    mp_ptr t = (mp_ptr)alloca(sizeof(mp_limb)*(ul+1));

    count_leading_zeros(d, v[vl-1]);
    (void)mpn_lshift(v, v, vl, d), v = t;
    if ((t[ul] = mpn_lshift(t, u, ul, d))) ul++;
    u = t;

    for (;;)
    {   (void)mpn_divmod(u + vl, u, ul, v, vl);
	ul = vl;
	while (u[--ul] == 0)
	    if (ul == 0) 
	    {   while (*v == 0) vl--, v++;
		count_trailing_zeros(d, *v);
		if (d) (void)mpn_rshift(g, v, vl, d);
		return (g[vl-1]) ? vl : vl - 1;
	    }
	ul++;
	if (*u == 0 && *v == 0) ul--, u++, vl--, v++;
	MPN_SWAP(u, ul, v, vl);
	count_leading_zeros(d, v[vl-1]);
	if (d == 0) continue;
	(void)mpn_lshift(v, v, vl, d);
	if ((u[ul] = mpn_lshift(u, u, ul, d))) ul++;
    }
}

/*  End file mpn_euclgcd.c  */
