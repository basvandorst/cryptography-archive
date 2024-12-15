#include "gmp.h"
#include "gmp-ext.h"

#define	DEBUG	0
#if	DEBUG
# include	<stdio.h>
#endif

#if	DEBUG
# define	DBG(arg)	printf arg
#else
# define	DBG(arg)
#endif

/*
 * Square root algorithm due to Adleman, Manders, and Miller.
 *
 * Pre: p odd prime, jacobi(a,p) = 1, 0 <= a < p.
 *
 * See pg 54 of
 *
 * Lecture Notes on the Complexity of Some Problems in Number Theory
 * by Dana Angluin, Yale CS TR 243 Aug '82.
 */

struct towerlist {
	MP_INT	x;
	int	inited;
};

struct tower {
	MP_INT			modulus;
	int			max_sq;
	struct towerlist	cache[1];
};

/* pre max_squarings >= 0 */
static struct tower *pow_tower(val,mod,max_squarings)
MP_INT	*val, *mod;
int	max_squarings;
{
	struct tower		*tp;
	struct towerlist	*t2;
	int			i;

#if	DEBUG
	fputs("Entered pow_tower(",stdout);
	mpz_out_str(stdout,10,val);
	putchar(',');
	mpz_out_str(stdout,10,mod);
	putchar(',');
	printf("%d)\n",max_squarings);
#endif
	tp = (struct tower *) malloc((unsigned) sizeof(struct tower) + max_squarings * sizeof(struct towerlist));
	if (!tp) gmp_fatal(MP_ERR_MEM,"mpz_sqrtm_amm: out of memory");

	DBG(("initializing tbl\n"));
	for (i = max_squarings+1, t2 = tp->cache; --i >= 0; t2++) {
		mpz_init(&t2->x);
		t2->inited = 0;
	}
	DBG(("initializing modulus\n"));
	mpz_init_set(&tp->modulus,mod);

	tp->max_sq = max_squarings;
	DBG(("initializing zero squarings elt\n"));
#if	DEBUG
	fputs("Elt is ",stdout);mpz_out_str(stdout,10,val);putchar('\n');
	fputs("prev is ",stdout);mpz_out_str(stdout,10,&tp->cache[0].x);
	putchar('\n');
#endif
	mpz_set(&tp->cache[0].x,val);
	tp->cache[0].inited = 1;
	DBG(("Leaving pow_tower\n"));
	return tp;
}

static MP_INT *pow_tow_val(tp,squarings)
struct tower	*tp;
int		squarings;
{
	MP_INT	*xx;

#if	DEBUG
	fputs("Entering pow_tow_val(",stdout);
	mpz_out_str(stdout,10,&tp->cache[0].x);
	printf(",%d)\n",squarings);
#endif
	if (squarings > tp->max_sq || squarings < 0)
		gmp_fatal(MP_ERR_INTERN, "mpz_sqrtm_amm: tower of power stack overflow");

	if (tp->cache[squarings].inited) {
#if	DEBUG
		fputs("Found cached copy:",stdout);
		mpz_out_str(stdout,10,&tp->cache[squarings].x);
		putchar('\n');
#endif
		return &tp->cache[squarings].x;
	}
	xx = pow_tow_val(tp,squarings-1);
	mpz_mul(&tp->cache[squarings].x,xx,xx);
	mpz_mmod(&tp->cache[squarings].x,&tp->cache[squarings].x,&tp->modulus);

	tp->cache[squarings].inited = 1;
#if	DEBUG
	fputs("calc'ed: ",stdout);
	mpz_out_str(stdout,10,&tp->cache[squarings].x);
	putchar('\n');
#endif
	return &tp->cache[squarings].x;
}

static pow_tow_free(tp)
struct tower	*tp;
{
	int			i;
	struct towerlist	*ptr;

	mpz_clear(&tp->modulus);
	for (i = tp->max_sq + 1, ptr = tp->cache; --i >= 0; ptr++) {
		mpz_clear(&ptr->x);
	}
	free((char *) tp);
}

/*
 * mpz_sqrtm_amm, square root modulo prime, algorithm
 * of Adleman, Manders, and Miller.  ``sqrtm'' ala ``powm''.
 *
 * The parameter randfunc called as (*randfunc)(randstate,result,modulus)
 * This allows multithreaded code to work right; even if we don't have parallel
 * pseudo-random number generators and are just using random(3) to fill the
 * limbs, we'll need locking to make sure random(3)'s internal state don't get
 * clobbered.  The locks can go in randstate.
 *
 * Pre: p odd prime.
 */
#if	__STDC__
int		mpz_sqrtm_amm(MP_INT *r, MP_INT *a, MP_INT *p, void (*randfunc)(void *,MP_INT *,const MP_INT *), void *randstate)
#else
int		mpz_sqrtm_amm(r,a,p,randfunc,randstate)
MP_INT		*r;	/* out */
MP_INT		*a,*p;	/* in */
void		(*randfunc)(), *randstate;
#endif
{
	int		k, j, m;
	MP_INT		q, a_exp_q;
	MP_INT		s, t, two_to_k, gcd;
	MP_INT		gamma, l, phi_of_p, *ptr;
	struct tower	*l_tp = 0, *a_exp_q_tp = 0;
	int		rv = 1;

#if	DEBUG
	printf("mpz_sqrtm_amm(r,");
	mpz_out_str(stdout,10,a);
	putchar(',');
	mpz_out_str(stdout,10,p);
	printf(")\n");
#endif
	if (!mpz_cmp_ui(a,0)) {
		mpz_set_ui(r,0);
		goto simply_done;
	}
	k = mpz_odd_less1_2factor(p);
	DBG(("k is %d\n",k));
	mpz_init(&q);
	mpz_div_2exp(&q,p,k);
#if	DEBUG
	printf("q = "); mpz_out_str(stdout,10,&q); putchar('\n');
#endif
	mpz_init(&a_exp_q);
	mpz_powm(&a_exp_q,a,&q,p);
#if	DEBUG
	printf("a^q = "); mpz_out_str(stdout,10,&a_exp_q); putchar('\n');
#endif
	if (!mpz_cmp_ui(&a_exp_q,1)) {
		mpz_add_ui(&q,&q,1);
		mpz_div_2exp(&q,&q,1);
		mpz_powm(r,a,&q,p);
		goto quite_easily_done;
	}
	mpz_init(&s); mpz_init(&t);
	mpz_init_set_ui(&two_to_k,1); mpz_mul_2exp(&two_to_k,&two_to_k,k);
	mpz_init(&gcd);
	mpz_gcdext(&gcd,&s,&t,&two_to_k,&q);

#if	DEBUG
	printf("s = "); mpz_out_str(stdout,10,&s);
	printf("\nt= "); mpz_out_str(stdout,10,&t); putchar('\n');
	{
		MP_INT x,y;
		mpz_init(&x); mpz_mul(&x,&two_to_k,&s);
		mpz_init(&y); mpz_mul(&y,&q,&t);
		mpz_add(&x,&y,&x);
		printf("sum is "); mpz_out_str(stdout,10,&x); putchar('\n');
		mpz_clear(&x); mpz_clear(&y);
	}
#endif

	if (mpz_cmp_ui(&gcd,1)) {
		rv = 0;
		goto barf;
	}
	mpz_init(&gamma);
	mpz_init(&l);
	if ((mpz_get_ui(p) & 0x3) == 0x3) {
#if	0
		mpz_init_set(&gamma,p);
		mpz_sub_ui(&gamma,1);	/* -1 */
#endif
		if (mpz_get_ui(&q) & 1)	{
			mpz_set(&l,p); mpz_sub_ui(&l,&l,1);
		} else			mpz_set_ui(&l,1);
	} else {
		do {
			(*randfunc)(randstate,&gamma,p);
		} while (mpz_legendre(&gamma,p) != -1);
		mpz_powm(&l,&gamma,&q,p);
	}

	a_exp_q_tp = pow_tower(&a_exp_q,p,k-1);

	for (;;) {
#if	DEBUG
		fputs("Step 7: l = ",stdout);
		mpz_out_str(stdout,10,&l);
		putchar('\n');
#endif
		l_tp = pow_tower(&l,p,k-1);
		for (j = 0; j < k; j++) {
			ptr = pow_tow_val(a_exp_q_tp,j);
#if	DEBUG
			printf("j is %d, a_exp_q^2^j is ",j);
			mpz_out_str(stdout,10,ptr); putchar('\n');
#endif
			for (m = j+1; m < k; m++) {
				if (!mpz_cmp(ptr,pow_tow_val(l_tp,m))) {
					goto found;
				}
			}
		}
		/* should never happen */
		gmp_fatal(MP_ERR_INTERN,"mpz_sqrtm_amm: interal error (no j found!)");
found:
		DBG(("m is %d\n",m));
		if (j > 0) {
			mpz_mul(&l,pow_tow_val(l_tp,k-m),&l);
			mpz_mmod(&l,&l,p);
			pow_tow_free(l_tp);
			/* next iter will generate another power tower */
		} else break;
	}
	/* j == 0 */
	mpz_init_set(&phi_of_p,p);
	mpz_sub_ui(&phi_of_p,&phi_of_p,1);
	/*
	 * We canonicalize s and t -- mpow can not handle negative
	 * exponents since it does not know phi(p).
	 */
	mpz_mmod(&s,&s,&phi_of_p);
	mpz_mmod(&t,&t,&phi_of_p);
	/*
	 * We know x^(p-1) == 1.
	 * mpow does not factor and so can not do this optimization.
	 */
	mpz_mul_2exp(&s,&s,k-1);
	mpz_mmod(&s,&s,&phi_of_p);
	mpz_mul_2exp(&t,&t,m-1);
	mpz_mmod(&t,&t,&phi_of_p);
	mpz_powm(&s,a,&s,p);
	mpz_powm(&t,&l,&t,p);
	mpz_mul(r,&s,&t);
	mpz_mmod(r,r,p);
	/*
	 * free up trash
	 */
	mpz_clear(&phi_of_p);
	pow_tow_free(l_tp);
	pow_tow_free(a_exp_q_tp);
	mpz_clear(&l);
	mpz_clear(&gamma);
barf:
	mpz_clear(&gcd);
	mpz_clear(&two_to_k);
	mpz_clear(&s); mpz_clear(&t);
quite_easily_done:
	mpz_clear(&a_exp_q);
	mpz_clear(&q);
simply_done:
	return rv;
}
