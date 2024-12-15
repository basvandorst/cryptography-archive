#include "gmp.h"

/*
 * Find multiplicative inverse of x in Z_n.
 * Returns 1 if an inverse exists, else 0 in which case xinv is untouched.
 */
#if	__STDC__
int		mpz_invertm(MP_INT *xinv, const MP_INT *x, const MP_INT *n)
#else
int		mpz_invertm(xinv,x,n)
MP_INT		*xinv;
const MP_INT	*x,*n;
#endif
{
	MP_INT	gcd, inverse;
	int	rv;

	mpz_init(&gcd);
	mpz_init(&inverse);
	mpz_gcdext(&gcd,&inverse,(MP_INT *) 0,x,n);
	rv = mpz_cmp_ui(&gcd,1);
	mpz_clear(&gcd);
	if (rv == 0) {
		mpz_clear(xinv);
		*xinv = inverse;
		return 1;
	}
	mpz_clear(&inverse);
	return 0;
}
