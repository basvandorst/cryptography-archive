
/* prvdump.c (dump private key) - Mark Henderson (markh@wimsey.bc.ca) 
   also does a few interesting computations */
#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *p, *q, *c, *n, *d, *pa, *qa;
	char *ps, *qs, *es, *ds;
	char prvfile[127];
	int i;
	unsigned int idlen;
	unsigned char k[16];
	unsigned char iv[8];
	unsigned char idstring[127];
	unsigned char hash[16];
	int alg = getalg();
	unsigned char *ss;
	FILE *private;
	init(e);
	init(p);
	init(q);
	init(c);
	init(n);
	init(d);
	init(pa);
	init(qa);
	if (argc < 2) {
		fprintf(stderr, "usage: prvdump prvfile\n");
		exit(1);
	}
	for (i = 0; i < 8; i++)
		iv[i] = 0;

	strncpy(prvfile, argv[1], 120);
	strcat(prvfile, ".prv");

	if (!(private = mfopen(prvfile, "rb"))) {
		fprintf(stderr, "could not open %s for read\n", prvfile);
		exit(1);
	}

	fprintf(stderr, "Enter password to private key: ");
	ss = getstring(1);
	mdstring(ss, strlen(ss), k);
	for (i = 0; i < 8; i++)
		iv[i] = 0;

	if (!read_numberx(private, p, k, iv) || !read_numberx(private, q, k, iv)) {
		fprintf(stderr, "wrong key to key entered or key corrupt\n");
		zap(k, 16);
		zap(hash, 16);
		zap(ss, strlen(ss));
		exit(1);
	}


	read_number(private, e);
	mpz_mul(n, p, q);
    for (i = 0, mpz_set(pa,n); mpz_cmp_ui(pa, 0); i++, mpz_div_2exp(pa, pa, 1))
        ;
	printf("n has %d bits\n", i);
	ps = mpz_get_str(NULL, 16, p);
	printf("p=%s\n", ps);
    for (i = 0, mpz_set(pa,p); mpz_cmp_ui(pa, 0); i++, mpz_div_2exp(pa, pa, 1))
        ;
	printf("\t%d bits\n", i);
	
	qs = mpz_get_str(NULL, 16, q);
	printf("q=%s\n", qs);
    for (i = 0, mpz_set(pa,q); mpz_cmp_ui(pa, 0); i++, mpz_div_2exp(pa, pa, 1))
        ;
	printf("\t%d bits\n", i);
	es = mpz_get_str(NULL, 16, e);
	printf("e=%s\n", es);
    for (i = 0, mpz_set(pa,e); mpz_cmp_ui(pa, 0); i++, mpz_div_2exp(pa, pa, 1))
        ;
	printf("\t%d bits\n", i);
	mpz_sub_ui(pa, p, 1);
	mpz_sub_ui(qa, q, 1);
	mpz_mul(c, pa, qa);
	modinv(d, e, c);	/* calculate d for RSA for fun */
	ds = mpz_get_str(NULL, 16, d);
	printf("RSA d=%s\n", ds);

	mpz_gcd(c,pa,qa);
	ds = mpz_get_str(NULL, 16, c);
	printf("gcd(p-1,q-1)=%s\n", ds);
	
	mpz_add_ui(pa,p,1);
	mpz_gcd(c,pa,qa);
	ds = mpz_get_str(NULL, 16, c);
	printf("gcd(p+1,q-1)=%s\n", ds);

	mpz_add_ui(qa,q,1);
	mpz_gcd(c,pa,qa);
	ds = mpz_get_str(NULL, 16, c);
	printf("gcd(p+1,q+1)=%s\n", ds);

	mpz_sub_ui(pa,p,1);
	mpz_gcd(c,pa,qa);
	ds = mpz_get_str(NULL, 16, c);
	printf("gcd(p-1,q+1)=%s\n", ds);
	zap(k, 16);
	zap(hash, 16);
	zap(ss, strlen(ss));
	clear(e); clear(p); clear(q); clear(c); clear(n); clear(d); clear(pa);
	clear(qa);
}
