/*
 * HISTORY
 * 91-01-07  Bennet Yee (bsy) at Carnegie-Mellon University
 *	Created.  Based on mjacobi from cmump.
 */
#include "gmp.h"
#include "gmp-ext.h"

/* precondition:  both p and q are positive */
#if	__STDC__
int		mpz_jacobi(const MP_INT *pi, const MP_INT *qi)
#else
int		mpz_jacobi(pi,qi)
const MP_INT *pi,*qi;
#endif
{
#if	GCDCHECK
	int	retval;
	MP_INT	gcdval;

	mpz_init(&gcdval);
	mpz_gcd(&gcdval,pi,qi);
	if (!mpz_cmp_ui(&gcdval,1L)) {
		/* J(ab,cb) = J(ab,c)J(ab,b) = J(ab,c)J(0,b) = J(ab,c)*0 */
		retval = 0;
	} else	retval = mpz_legendre(pi,qi);
	mpz_clear(&gcdval);
	return retval;
#else
	return mpz_legendre(pi,qi);
#endif
}
