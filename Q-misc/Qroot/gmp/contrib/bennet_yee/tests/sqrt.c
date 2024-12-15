#include <stdio.h>
#include <ctype.h>

#include "gmp.h"
#include "gmp-ext.h"

#define	BITS_PER_RANDOM		31

/* ARGSUSED */
void mpz_rand(state,num,mod)
void		*state;
MP_INT		*num;
const MP_INT	*mod;
{
	int	nrands = BITS_PER_RANDOM*(mpz_size(mod) + 1);

	mpz_set_ui(num,0);
	while (--nrands >= 0) {
		mpz_mul_2exp(num,num,BITS_PER_RANDOM);
		mpz_add_ui(num,num,random());
	}
	mpz_mmod(num,num,mod);
}

skip_whitespace(stream)
FILE	*stream;
{
	int	ch;
	while (isspace(ch = getc(stream)))
		;
	ungetc(ch,stream);
}

main()
{
	int	rv;
	MP_INT	p, x, r;

	mpz_init(&p); mpz_init(&x); mpz_init(&r);

	for (;;) {
		printf("%24s","value: ");
		skip_whitespace(stdin);
		mpz_inp_str(&x,stdin,10);

		if (feof(stdin)) break;

		printf("%24s","Prime: ");
		skip_whitespace(stdin);
		mpz_inp_str(&p,stdin,10);

		if (feof(stdin)) break;

		printf("Legendre(");
		mpz_out_str(stdout,10,&x);
		printf(",");
		mpz_out_str(stdout,10,&p);
		printf(") = "); fflush(stdout);
		rv = mpz_legendre(&x,&p);
		printf("%d\n",rv);
		if (rv != 1) {
			printf("Not a quadratic residue\n");
			continue;
		}
		printf("It is a quadratic residue\n");
#if	1
		printf("Using Berlekamp's algorithm\n");
		mpz_sqrtm_berlekamp(&r,&x,&p,mpz_rand,(void *) 0);
		printf("%24s","root is "); mpz_out_str(stdout,10,&r); putchar('\n');
		mpz_mul(&r,&r,&r);
		mpz_mmod(&r,&r,&p);
		printf("%24s","root squared is "); mpz_out_str(stdout,10,&r); putchar('\n');

		printf("Using algorithm of Adleman, Manders, and Miller\n");
		mpz_sqrtm_amm(&r,&x,&p,mpz_rand,(void *) 0);
		printf("%24s","root is "); mpz_out_str(stdout,10,&r); putchar('\n');
		mpz_mul(&r,&r,&r);
		mpz_mmod(&r,&r,&p);
		printf("%24s","root squared is "); mpz_out_str(stdout,10,&r); putchar('\n');
#endif
	}
	exit(0);
}
