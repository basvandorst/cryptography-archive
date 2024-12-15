#if	!defined(__GMP_EXT_H__)
#define	__GMP_EXT_H__

#include "gmp_fatal.h"

#if	__STDC__
void		mp_fatal(int mp_errno, const char *free_form_msg);
unsigned int	mpz_2factor(const MP_INT	*n);
unsigned int	mpz_less1_2factor(MP_INT	*n);
int		mpz_invertm(MP_INT *xinv, const MP_INT *x, const MP_INT *n);
int		mpz_jacobi(const MP_INT *pi, const MP_INT *qi);
int		mpz_legendre(const MP_INT *pi, const MP_INT *qi);
int		mpz_size_raw(const MP_INT *x);
int		mpz_set_raw(MP_INT *x, const unsigned char *buf, int bsize);
void		mpz_get_raw(unsigned char *buf, const MP_INT *x);
int		mpz_sqrtm_amm(MP_INT *r, MP_INT *a, MP_INT *p, void (*randfunc)(void *,MP_INT *,const MP_INT *), void *randstate);
int		mpz_sqrtm_berlekamp(MP_INT *r, MP_INT *a, MP_INT *p, void (*randfunc)(void *,MP_INT *,const MP_INT *), void *randstate);

#else	__STDC_

void		mp_fatal();
unsigned int	mpz_2factor();
unsigned int	mpz_less1_2factor();
int		mpz_invertm();
int		mpz_jacobi();
int		mpz_legendre();
int		mpz_size_raw();
int		mpz_set_raw();
void		mpz_get_raw();
int		mpz_sqrtm_amm();
int		mpz_sqrtm_berlekamp();

#endif	__STDC__
#endif	!defined(__GMP_EXT_H__)
