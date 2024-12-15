#include "gmp.h"
#include "gmp-ext.h"

#define	DEBUG	0
#if	DEBUG
# include	<stdio.h>
#endif

/*
 * Square root algorithm due to Berlekamp.
 *
 * Pre: p odd prime, jacobi(a,p) = 1, 0 <= a < p.
 *
 * See pg 49 of
 * Lecture Notes on the Complexity of Some Problems in Number Theory
 * by Dana Angluin, Yale CS TR 243 Aug '82.
 */
#if	__STDC__
int		mpz_sqrtm_berlekamp(MP_INT *r, MP_INT *a, MP_INT *p, void (*randfunc)(void *,MP_INT *,const MP_INT *), void *randstate)
#else
int		mpz_sqrtm_berlekamp(r,a,p,randfunc,randstate)
MP_INT		*r;	/* out */
MP_INT		*a,*p;	/* in */
void		(*randfunc)(), *randstate;
#endif
{
	MP_INT	gamma, minustwogamma, gamma_sq_minus_a;
	MP_INT	linear, constterm;
	MP_INT	r_linear, r_const;
	MP_INT	tmp_q, tmp_c, tmp_l;
	MP_INT	p_copy, one;
	int	rv = 1;

#if	DEBUG
	puts("Entered mberlekamp");
#endif
	if (!mpz_cmp_ui(a,0)) {
		mpz_set_ui(r,0);
		return;
	}
	/* 0 < a < p */
	if ((mpz_get_ui(p) & 0x3) == 3) {
#if	DEBUG
		puts("oh boy! p = 3 mod 4 means -1 is nonresidue");
#endif
		/*
		 * we know that -1 is a nonresidue, so -a is a nonreside also
		 */
		mpz_init_set_ui(&gamma,0);
		mpz_init(&gamma_sq_minus_a);
		mpz_sub(&gamma_sq_minus_a,p,a);
	} else {
#if	DEBUG
		puts("Looking for a nonresidue...");
#endif
		/* probabilistically find a quadratic non-residue */
		mpz_init(&gamma);
		mpz_init(&gamma_sq_minus_a);
		do {
#if	DEBUG2
			putchar('x'); fflush(stdout);
#endif
			(*randfunc)(randstate,&gamma,p);
#if	DEBUG
			fputs("trying gamma: ",stdout);
			mpz_out_str(stdout,10,&gamma);
			putchar('\n');
#endif
			/* 0 <= gamma < p */
			mpz_mul(&gamma_sq_minus_a,&gamma,&gamma);
			mpz_sub(&gamma_sq_minus_a,&gamma_sq_minus_a,a);
			mpz_mmod(&gamma_sq_minus_a,&gamma_sq_minus_a,p);
#if	DEBUG
			fputs("trying gamma_sq_minus_a: ",stdout);
			mpz_out_str(stdout,10,&gamma_sq_minus_a);
			putchar('\n');
#endif
		} while (mpz_legendre(&gamma_sq_minus_a,p) != -1);
	}
#if	DEBUG
	fputs("\ngamma: ",stdout);
	mpz_out_str(stdout,10,&gamma);
	putchar('\n');
	fputs("gamma_sq_minus_a: ",stdout);
	mpz_out_str(stdout,10,&gamma_sq_minus_a);
	putchar('\n');
#endif
	/*
	 * now gamma_sq_minus_a = gamma^2-a is a quadratic nonresidue,
	 * and we proceed to compute
	 *
	 *	x^{(p-1)/2}-1 mod ((x-gamma)^2-a).
	 *
	 * Key observation:  roots of g(x)=((x-gamma)^2-a) are
	 * rho and sigma where
	 *	rho = gamma + sqrt(a), sigma = gamma - sqrt(a).		(1)
	 * and so
	 *	rho*sigma = gamma^2-a.
	 * Since we know that legendre(gamma^2-a,p)=-1,
	 * wlog sigma is a quadratic nonresidue,
	 * and rho is a quadratic residue.
	 *
	 * Now, since f(x)=x^{(p-1)/2}-1 has all quadratic residues as roots,
	 * gcd(f(x),g(x)) = delta*(x-rho).
	 *
	 * Since deg(f(x))>=deg(g(x)),
	 * computing f(x) mod g(x) gives delta*(x-rho).
	 *
	 * rho - gamma is a root of x^2 - a from (1).
	 */

	/*
	 * Implementation details.
	 *
	 * g(x) = (x-gamma)^2-a = x^2 - 2gamma x - gamma^2-a.
	 */
	mpz_init(&minustwogamma);
	mpz_mul_2exp(&minustwogamma,&gamma,1);
	mpz_neg(&minustwogamma,&minustwogamma);
#if	DEBUG
	fputs("-2*gamma: ",stdout);
	mpz_out_str(stdout,10,&minustwogamma);
	putchar('\n');
#endif
	mpz_init(&p_copy);
	mpz_div_2exp(&p_copy,p,1);	/* p_copy == (p-1)/2 */

	/*
	 * Repeatedly square h(x), h(x) <- h(x)^2 mod g(x), and
	 * conditionally performing r(x) = r(x) * h(x) mod g(x).
	 * we get r(x) = f(x) + 1 mod g(x).
	 */
	mpz_init_set_ui(&linear,1); mpz_init_set_ui(&constterm,0); /* h(x)=x */
	mpz_init_set_ui(&r_linear,0); mpz_init_set_ui(&r_const,1); /* r(x)=1 */

	mpz_init(&tmp_q); mpz_init(&tmp_l); mpz_init(&tmp_c);

	if (mpz_cmp_ui(&p_copy,0)) for (;;) {
#if	DEBUG1
		putchar('.'); fflush(stdout);
#endif
		if (mpz_get_ui(&p_copy) & 1) {
#if	DEBUG
# if	__STDC__
#  define SHOW(x) fputs(#x " is ",stdout); mpz_out_str(stdout,10,x); putchar('\n')
# else
#  define SHOW(x) fputs("x is ",stdout); mpz_out_str(stdout,10,x); putchar('\n')
# endif
			puts("bit set");
#else
# define SHOW(x)
#endif
			/*
			 * tmp(x) = h(x) * r(x).
			 */
			mpz_mul(&tmp_q,&linear,&r_linear);
			mpz_mmod(&tmp_q,&tmp_q,p);
			SHOW(&tmp_q);
			mpz_mul(&tmp_c,&linear,&r_const);
			mpz_mmod(&tmp_c,&tmp_c,p);
			SHOW(&tmp_c);
			mpz_mul(&tmp_l,&constterm,&r_linear);
			mpz_mmod(&tmp_l,&tmp_l,p);
			SHOW(&tmp_l);
			mpz_add(&tmp_l,&tmp_l,&tmp_c);
			mpz_mmod(&tmp_l,&tmp_l,p);
			SHOW(&tmp_l);
			mpz_mul(&tmp_c,&constterm,&r_const);
			mpz_mmod(&tmp_c,&tmp_c,p);
			SHOW(&tmp_c);

			/*
			 * r(x) = tmp(x) mod g(x)
			 */ 
			mpz_mul(&r_linear,&tmp_q,&minustwogamma);
			mpz_mmod(&r_linear,&r_linear,p);
			SHOW(&r_linear);
			mpz_mul(&r_const,&tmp_q,&gamma_sq_minus_a);
			mpz_mmod(&r_const,&r_const,p);
			SHOW(&r_const);

			mpz_sub(&r_linear,&tmp_l,&r_linear);
			mpz_mmod(&r_linear,&r_linear,p);
			SHOW(&r_linear);
			mpz_sub(&r_const,&tmp_c,&r_const);
			mpz_mmod(&r_const,&r_const,p);
			SHOW(&r_const);
		}
#if	DEBUG
		fputs("res linear term is ",stdout);
		mpz_out_str(stdout,10,&r_linear); putchar('\n');
		fputs("res const term is ",stdout);
		mpz_out_str(stdout,10,&r_const); putchar('\n');
		fputs("poly linear term is ",stdout);
		mpz_out_str(stdout,10,&linear); putchar('\n');
		fputs("poly const term is ",stdout);
		mpz_out_str(stdout,10,&constterm); putchar('\n');
		fputs("p_copy is ",stdout);
		mpz_out_str(stdout,10,&p_copy); putchar('\n');
#endif
		/* drop low bit */
		mpz_div_2exp(&p_copy,&p_copy,1);
		/*
		 * h(x) = h(x)^2 mod g(x)
		 */
		if (mpz_cmp_ui(&p_copy,0)) {
			mpz_mul(&tmp_q,&linear,&linear);
			mpz_mmod(&tmp_q,&tmp_q,p);
			SHOW(&tmp_q);
			mpz_mul(&tmp_l,&linear,&constterm);
			mpz_mmod(&tmp_l,&tmp_l,p);
			SHOW(&tmp_l);
			mpz_mul_2exp(&tmp_l,&tmp_l,1);
			mpz_mmod(&tmp_l,&tmp_l,p);
			SHOW(&tmp_l);
			mpz_mul(&tmp_c,&constterm,&constterm);
			mpz_mmod(&tmp_c,&tmp_c,p);
			SHOW(&tmp_c);

			mpz_mul(&linear,&tmp_q,&minustwogamma);
			mpz_mmod(&linear,&linear,p);
			SHOW(&linear);
			mpz_mul(&constterm,&tmp_q,&gamma_sq_minus_a);
			mpz_mmod(&constterm,&constterm,p);
			SHOW(&constterm);
			mpz_sub(&linear,&tmp_l,&linear);
			mpz_mmod(&linear,&linear,p);
			SHOW(&linear);
			mpz_sub(&constterm,&tmp_c,&constterm);
			mpz_mmod(&constterm,&constterm,p);
			SHOW(&constterm);
		} else break;
	}
	/*
	 * r(x) = r(x) - 1; so now r(x) = f(x) mod g(x) = delta(x-rho).
	 */
	mpz_sub_ui(&r_const,&r_const,1);
#if	DEBUG
	fputs("\nlinear term is ",stdout);
	mpz_out_str(stdout,10,&r_linear); putchar('\n');
	fputs("const term is ",stdout);
	mpz_out_str(stdout,10,&r_const); putchar('\n');
#endif
	/* rho = -r_const/r_linear mod p */
	if (mpz_cmp_ui(&r_linear,1)) {
		if (!mpz_invertm(&r_linear,&r_linear,p)) {
			rv = 0;
			goto barf;
		}
		mpz_mul(&r_const,&r_linear,&r_const);
		mpz_mmod(&r_const,&r_const,p);
	}
	mpz_neg(&r_const,&r_const);

#if	DEBUG
	fputs("rho: ",stdout);
	mpz_out_str(stdout,10,&r_const); putchar('\n');
#endif
	/* rho - gamma is a root of x^2 - a */
	mpz_sub(r,&r_const,&gamma);
	mpz_mmod(r,r,p);

barf:
	mpz_clear(&tmp_q); mpz_clear(&tmp_l); mpz_clear(&tmp_c);
	mpz_clear(&r_linear); mpz_clear(&r_const);
	mpz_clear(&linear); mpz_clear(&constterm);
	mpz_clear(&p_copy);
	mpz_clear(&minustwogamma);
	mpz_clear(&gamma_sq_minus_a);
	mpz_clear(&gamma);

	return rv;
}

/*
 * HISTORY
 *
 * 12-Jul-89  Bennet Yee (bsy) at CMU
 *	Created.  CODE NOT DERIVED FROM MP(3X).
 *
 * 92-01-15 Bennet Yee (bsy@cs.cmu.edu)
 *	Ported to GMP from CMUMP.
 */
