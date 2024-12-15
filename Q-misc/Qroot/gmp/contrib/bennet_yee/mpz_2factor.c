/*
 * HISTORY
 * 92-01-06  Bennet Yee (bsy) at Carnegie-Mellon University
 *	Created.
 */

#include "gmp.h"
#include "gmp-mparam.h"
#include "gmp-ext.h"

/*
 * Common operations in number theoretic code.
 *
 * We often want to compute s, t given n, where n = 2^s t, t odd.
 * This gives s, and mpz_div_2exp(n,s) will give t.
 */

#if	__STDC__
unsigned int	mpz_2factor(const MP_INT	*n)
#else
unsigned int	mpz_2factor(n)
const MP_INT	*n;
#endif
{
	mp_size		nsize;
	mp_srcptr	sp;
	mp_size		nzeros;
	mp_limb		v;

	nsize = n->size;
	if (!nsize) return 0;	/* inf? */
	if (nsize < 0) nsize = -nsize;
	sp = n->d;
	nzeros = 0;
	while (nsize && !(v = *sp++))	{
		nzeros += BITS_PER_MP_LIMB;
		--nsize;
	}
	if (nsize) {
		/* v != 0 */
#if	0
		while (!(v&1)) {
			++nzeros; v >>= 1;
		}
#else
		nzeros += ffs(v) - 1;
#endif
	}
	return nzeros;
}

/*
 * Sometimes we know that n is odd, and we want to decompose it as
 * n-1 = 2^s t, t odd.
 *
 * Pre: n is odd.
 */
#if	__STDC__
unsigned int	mpz_odd_less1_2factor(MP_INT	*n)
#else
unsigned int	mpz_odd_less1_2factor(n)
MP_INT	*n;
#endif
{
	mp_size		nsize;
	mp_srcptr	sp;
	mp_size		nzeros;
	mp_limb		v;

	/* n odd, so nsize > 0 */
	nsize = n->size;
	if (!nsize) {
		gmp_fatal(MP_ERR_INTERN,"mpz_odd_less1_2factor: precodition n odd not satisfied\n");
		/* NOTREACHED */
	}
	if (nsize < 0) nsize = -nsize;
	sp = n->d;
	nzeros = 0;
	if (v = (*sp++ & ~1)) {
		/* v != 0, so will terminate */
#if	0
		while (!(v&1)) {
			++nzeros; v >>= 1;
		}
		return nzeros;
#else
		return ffs(v) - 1;
#endif
	}
	nzeros = BITS_PER_MP_LIMB; --nsize;
	while (nsize && !(v = *sp++)) {
		nzeros += BITS_PER_MP_LIMB;
		--nsize;
	}
	if (nsize) {
		/* v != 0 */
#if	0
		while (!(v&1)) {
			++nzeros; v >>= 1;
		}
#else
		nzeros += ffs(v) - 1;
#endif
	}
	return nzeros;
}
