
/* LUCAS encryption using gmp 1.3.1
 * this version is faster than my earlier C++ version
 * Mark Henderson - markh@wimsey.bc.ca
 * TEST,TEST2,TEST3 define various tests - define at most one of these
 * STAND - define for "standalone" version, not compiled as a part of L3
 */

#if defined(TEST) || defined(TEST2) || defined(TEST3)
#define STAND
#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>
#endif
#define OGIWARA

#include <gmp.h>
#include "lucas.h"

#if defined(TEST) || defined(TEST2) || defined(TEST3)
char rbuf[2048];
#define chars(label,bignum) {mpz_get_str(rbuf,10,bignum); \
    printf("%s = %s\n", label,rbuf); }
#endif

unsigned char ver[] = "@(#)L3 1.7.5 - Mark C. Henderson - markh@wimsey.bc.ca";

#ifndef STAND
extern void ferr(char *);
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#ifndef STAND
/* you must supply this routine for genkey */
extern unsigned char randombyte(void);
#endif

#ifndef LIMB_BITS
#define LIMB_BITS (sizeof(unsigned long)*8)
#endif

#ifndef init
#define init(g) { g = (MP_INT *)malloc(sizeof(MP_INT));  mpz_init(g); }
#endif
#ifndef clear
#define clear(g) { mpz_clear(g); free(g); }
#endif

struct is {
	int v;
	struct is *next;
};

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

INLINE static void push(int i, struct is **sp)
{
	struct is *tmp;
	tmp = *sp;
	*sp = (struct is *) malloc(sizeof(struct is));
	(*sp)->v = i;
	(*sp)->next = tmp;
}

INLINE static int pop(struct is **sp)
{
	struct is *tmp;
	int i;
	if (!(*sp))
		return (-1);
	tmp = *sp;
	*sp = (*sp)->next;
	i = tmp->v;
	tmp->v = 0;
	free(tmp);
	return i;
}

INLINE static int even(MP_INT * a)
{
	return !(mpz_get_ui(a) & 1);
}

INLINE static int odd(MP_INT * a)
{
	return (mpz_get_ui(a) & 1);
}



void modinv(MP_INT * s, MP_INT * i, MP_INT * n)
{
	MP_INT *g, *h;
	init(g);
	init(h);
	mpz_gcdext(g, h, NULL, i, n);

	/* if (g == 1) */
	if (mpz_cmp_ui(g, 1) == 0) {

		/* s = h % n */
		mpz_mmod(s, h, n);
	}
	else {

		/* s = 0 */
		mpz_set_ui(s, 0);
	}
	clear(g);
	clear(h);
}

/* convert character string a into b */
void cstmp(MP_INT * a, unsigned char *b, unsigned int len)
{
	unsigned int i, u;
	unsigned int t;
	int j;
	MP_INT tt, uu;
	mpz_init(&tt);
	mpz_init(&uu);
	mpz_set_ui(a, 0);
	mpz_set_ui(&uu, 1);
	for (i = 0, j = len - 1; j >= 0; i++) {
		t = 0;
		for (u = 0; j >= 0 && u < LIMB_BITS; j--, u += 8)
			t |= ((unsigned int) b[j]) << u;
		if (t) {
			mpz_set_ui(&tt, t);
			mpz_mul(&tt, &uu, &tt);
			mpz_add(a, &tt, a);
		}
		mpz_mul_2exp(&uu, &uu, LIMB_BITS);
	}
	mpz_clear(&tt);
}

/* convert b to a character string */
void mptcs(unsigned char *a, unsigned int len, MP_INT * b)
{
	unsigned int t;
	int j;
	unsigned int i, u;
	MP_INT bc;
	MP_INT zero;
	mpz_init(&zero);
	mpz_init_set(&bc, b);

	for (i = 0, j = len - 1; mpz_cmp(&bc, &zero); i++) {
		/* t = b[i]; */
		t = mpz_get_ui(&bc);
		mpz_div_2exp(&bc, &bc, LIMB_BITS);
		for (u = 0; j >= 0 && u < LIMB_BITS; j--, u += 8)
			a[j] = (unsigned char) (t >> u);
	}

	for (; j >= 0; j--)
		a[j] = 0;
	mpz_clear(&zero);
	mpz_clear(&bc);
}

static unsigned int tiny_primes[] =
{
	3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
	101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
	191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251};
#define ntiny_primes 53

/* prime, (p-1)/2 prime, (p+1)/2 prime */
int probab_strong_prime(MP_INT *p)
{
	MP_INT *t;
	init(t);
	mpz_div_2exp(t,p,1);  /* t = (p-1)/2 */
	if (!probab_prime(t)) {
		clear(t);
		return 0;
	}
	mpz_add_ui(t,t,1);    /* t=t+1 = (p-1)/2 + 1 = (p+1) / 2 */
	if (!probab_prime(t)) {
		clear(t);
		return 0;
	}
	clear(t);
	return 1;
}

int probab_prime(MP_INT * p)
{
	MP_INT *t;
	int i;
	int status;

	/* is p even */
	if (!(mpz_get_ui(p) % 2))
		return 0;
	init(t);
	for (i = 0; i < ntiny_primes; i++) {

		/* t = p % tiny_primes[i] */
		mpz_mmod_ui(t, p, tiny_primes[i]);

		/* if t = 0,  p sure isn't prime */
		if (!mpz_cmp_ui(t, 0)) {
			clear(t);
			return 0;
		}
	}
	/* t = 2 ^ p mod p */
	mpz_set_ui(t, 2);
	mpz_powm(t, t, p, p);

	/* if (t != 2) */
	if (mpz_cmp_ui(t, 2)) {
		clear(t);
		return 0;
	}

	/* t = 3 ^ p mod p */
	mpz_set_ui(t, 3);
	mpz_powm(t, t, p, p);

	/* if (t != 3) */
	if (mpz_cmp_ui(t, 3)) {
		clear(t);
		return 0;
	}

	/* overkill */
	status = mpz_probab_prime_p(p, 5);
	clear(t);
	return status;
}

int good_prime(MP_INT * p, MP_INT * e)
{
	MP_INT *t;
	int i;
	MP_INT *g;
	int status;

	/* is p even */
	if (!(mpz_get_ui(p) % 2))
		return 0;

	init(t);
	for (i = 0; i < ntiny_primes; i++) {

		/* t = p % tiny_primes[i] */
		mpz_mmod_ui(t, p, tiny_primes[i]);

		/* if t = 0,  p sure isn't prime */
		if (!mpz_cmp_ui(t, 0)) {
			clear(t);
			return 0;
		}
	}
	/* t = 2 ^ p mod p */
	mpz_set_ui(t, 2);
	mpz_powm(t, t, p, p);

	/* if (t != 2) */
	if (mpz_cmp_ui(t, 2)) {
		clear(t);
		return 0;
	}

	/* t = 3 ^ p mod p */
	mpz_set_ui(t, 3);
	mpz_powm(t, t, p, p);

	/* if (t != 3) */
	if (mpz_cmp_ui(t, 3)) {
		clear(t);
		return 0;
	}

	init(g);

	/* g = gcd(p+1,e) */
	mpz_add_ui(t, p, 1);
	mpz_gcd(g, t, e);

	/* if (g != 1) */
	if (mpz_cmp_ui(g, 1)) {
		clear(t);
		clear(g);
		return 0;
	}

	/* g = gcd(p-1,e) */
	mpz_sub_ui(t, p, 1);
	mpz_gcd(g, t, e);

	/* if (g != 1) */
	if (mpz_cmp_ui(g, 1)) {
		clear(t);
		clear(g);
		return 0;
	}

	/* g = gcd(p,e) */
	mpz_gcd(g, p, e);

	/* if (g != 1) */
	if (mpz_cmp_ui(g, 1)) {
		clear(t);
		clear(g);
		return 0;
	}

	/* overkill */
	status = mpz_probab_prime_p(p, 5);
	clear(t);
	clear(g);
	return status;
}

/* inputs : r0 and r1 distinct primes
   attempts to find a prime p such that
   r0 | p - 1
   r1 | p + 1
   gcd(e,p) = 1, gcd(e,p-1)=1, gcd(e,p+1)=1
   returns 0 on failure (which means it gave up after limit iterations)
        1 on success
   result goes into p 
 */
int promote(MP_INT *p, MP_INT *r0, MP_INT *r1, MP_INT *e, int limit) 
{
	MP_INT *g,*a,*b,*t; int i;
#ifdef DEBUG
	MP_INT *c;
	init(c);
#endif
	init(g); init(a); init(b); init(t);

	mpz_gcdext(g,a,b,r0,r1);   /* b = r1^{-1} mod r0 */
	/* if g != 1 then return 0 */
	if (mpz_cmp_ui(g,1)) 
		return 0;

	mpz_mmod(b,b,r0);          /* force b into range 0,...,r0 - 1 */

	/* p = 2 * b * r1 - 1 */
	mpz_mul_2exp(p,b,1); mpz_mul(p,r1,p); mpz_sub_ui(p,p,1);

	/* t = 2 * r0 * r1 */
	mpz_mul_2exp(t,r0,1); mpz_mul(t,t,r1);

#ifdef DEBUG
	fprintf(stderr, "debug promote: b = %s\n", mpz_get_str(NULL, 16, b));
	/* check that r0 divides (p - 1) */
	mpz_sub_ui(g,p,1);
	mpz_mod(c,g,r0);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: r0 does not divide p-1\n");
	}
	/* check that r1 divides (p+1) */
	mpz_add_ui(g,p,1);
	mpz_mod(c,g,r1);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: r1 does not divide p+1\n");
	}
#endif
	i =  0;
	while (!good_prime(p,e) && (i < limit)) {
		mpz_add(p,p,t); i++;
	}
	if (i == limit) {
#ifdef DEBUG
		fprintf(stderr, "promote failed\n");
#endif
		return 0;
	}
#ifdef DEBUG
	fprintf(stderr, "debug FINAL : p = %s\n", mpz_get_str(NULL, 16, p));
	clear(c);;
#endif

	clear(g); clear(a); clear(b); clear(t);
	return 1;
}

#ifndef STAND
#ifdef OGIWARA

/* genkey generates a LUCAS private key
    p,q are "returned"
    e is input public key which may be altered
    bits is requested number of bits for each of p and q
	Ogiwara's algorithm for the generation of cryptographically
      strong primes
*/
#define MK(a,n)  \
	cstmp((a),randbuf + (i),(n)); i+=(n)
#define SHIFT 1
int genkey(MP_INT * p, MP_INT * q, MP_INT * e, unsigned int bits)
{
	MP_INT *t,*r[8],*z[4];
	int i,j;
	int bytes = (bits + 7) / 8;
	int fail;
	unsigned char *randbuf;

	init(t);
	for (i=0;i<8;i++)
		init(r[i]);
	for (i=0;i<4;i++)
		init(z[i]);
#ifndef FIXED_E
	/* make sure that e is odd */
	if (even(e))
		mpz_add_ui(e, e, 1);

	/* I'm partial to prime e - although there is no reason we really
     *  need to make e prime. It does make things easier later.
     */
	while (!probab_prime(e))
		mpz_add_ui(e, e, 2);

#endif

	for (;;) {
		fail = 0;
		randbuf = (unsigned char *) malloc(bytes + bytes);

		for (i = 0; i < bytes + bytes; i++)
			*(randbuf + i) = randombyte();

		i = 0;
		MK(r[0],bytes/4-SHIFT);
		MK(r[1],bytes/4-SHIFT);
		MK(r[2],bytes/4);
		MK(r[3],bytes/4);
		MK(r[4],bytes/4);
		MK(r[5],bytes/4);
		MK(r[6],bytes/4 + SHIFT);
		MK(r[7],bytes/4 + SHIFT);
#ifdef DEBUG
		fprintf(stderr, "used %d bytes of random goo\n", i);
#endif

		/* make 8 primes r[0], ... , r[7]*/
		for (i=0; i<8; i++) {
			if (even(r[i]))
				mpz_add_ui(r[i], r[i], 1);
			while (!probab_prime(r[i])) 
				mpz_add_ui(r[i], r[i], 2);
		}

		for (i = 0; i < bytes + bytes; i++)
			*(randbuf + i) = 0;
		free(randbuf);

		/* now r0-r7 are (probably) relatively large primes */
#ifdef DEBUG
		fprintf(stderr, "debug genkey: r[0] = %s\n", mpz_get_str(NULL, 16, r[0]));
		fprintf(stderr, "debug genkey: r[1] = %s\n", mpz_get_str(NULL, 16, r[1]));
		fprintf(stderr, "debug genkey: r[2] = %s\n", mpz_get_str(NULL, 16, r[2]));
		fprintf(stderr, "debug genkey: r[3] = %s\n", mpz_get_str(NULL, 16, r[3]));
		fprintf(stderr, "debug genkey: r[4] = %s\n", mpz_get_str(NULL, 16, r[4]));
		fprintf(stderr, "debug genkey: r[5] = %s\n", mpz_get_str(NULL, 16, r[5]));
		fprintf(stderr, "debug genkey: r[6] = %s\n", mpz_get_str(NULL, 16, r[6]));
		fprintf(stderr, "debug genkey: r[7] = %s\n", mpz_get_str(NULL, 16, r[7]));
#endif
		for (i=0; i<8; i++)
			for (j=i+1; j<8; j++) 
				if (!mpz_cmp(r[i],r[j]))
					fail = 1;
		if (fail)
			continue;  /* try again - should really never happen */

		if (!promote(z[0],r[0],r[1],e,bits*2))
			continue;
		if (!promote(z[1],r[2],r[3],e,bits*2))
			continue;
		if (!promote(z[2],r[4],r[5],e,bits*2))
			continue;
		if (!promote(z[3],r[6],r[7],e,bits*2))
			continue;
		if (!promote(p,z[0],z[1],e,bits * 4))
			continue;
		if (!promote(q,z[2],z[3],e,bits * 4))
			continue;
		break;
	}
#ifdef DEBUG
	{
	MP_INT *g,*c;
	init(g); init(c);
	/* check that z[0] divides (p - 1) */
	mpz_sub_ui(g,p,1);
	mpz_mod(c,g,z[0]);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: z[0] does not divide p-1\n");
	}
	/* check that z[1] divides (p+1) */
	mpz_add_ui(g,p,1);
	mpz_mod(c,g,z[1]);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: z[1] does not divide p+1\n");
	}
	/* check that z[2] divides (q - 1) */
	mpz_sub_ui(g,q,1);
	mpz_mod(c,g,z[2]);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: z[2] does not divide q-1\n");
	}
	/* check that z[3] divides (q+1) */
	mpz_add_ui(g,q,1);
	mpz_mod(c,g,z[3]);
	if (mpz_cmp_ui(c,0)) {
			fprintf(stderr, "warning: z[3] does not divide q+1\n");
	}
	clear(g); clear(c);
	}
#endif

	/* force p > q */
	if (mpz_cmp(p, q) < 0) {
		mpz_set(t, p);
		mpz_set(p, q);
		mpz_set(q, t);
	}
	clear(t); 
	for (i=0; i<8; i++)
		clear(r[i]);
	for (i=0; i<4; i++)
		clear(z[i]);

	return 1;
}
#else
#define SHIFT 1
/* more traditional approach */

int genkey(MP_INT * p, MP_INT * q, MP_INT * e, unsigned int bits)
{
#if 0
	MP_INT *tp;
	MP_INT *tq;
#endif
	MP_INT *t;
	int i;
	int bytes = (bits + 7) / 8;
	unsigned char *randbuf;

	randbuf = (unsigned char *) malloc(bytes + bytes);

	for (i = 0; i < bytes + bytes; i++)
		*(randbuf + i) = randombyte();

	cstmp(p, randbuf, bytes + SHIFT);
	cstmp(q, randbuf + bytes + SHIFT, bytes - SHIFT);

#ifdef DEBUG
	fprintf(stderr, "debug genkey: pi = %s\n", mpz_get_str(NULL, 16, p));
	fprintf(stderr, "debug genkey: qi = %s\n", mpz_get_str(NULL, 16, q));
#endif

	for (i = 0; i < bytes + bytes; i++)
		*(randbuf + i) = 0;
	free(randbuf);

	init(t);
#if 0
	init(tp);
	init(tq);
	mpz_set_ui(t, 1);
	mpz_mul_2exp(t, t, bits);
	mpz_div_2exp(t, t, 1);
	mpz_mul_2exp(tp, t, SHIFT * 8);
	mpz_div_2exp(tq, t, SHIFT * 8);
	if (mpz_cmp(p, tp) < 0)
		mpz_add(p, tp, p);
	if (mpz_cmp(q, tq) < 0)
		mpz_add(q, tq, q);
#endif

	if (even(p))
		mpz_add_ui(p, p, 1);
	if (even(q))
		mpz_add_ui(q, q, 1);

#ifndef FIXED_E
	/* make sure that e is odd */
	if (even(e))
		mpz_add_ui(e, e, 1);

	/* I'm partial to prime e - although there is no reason we really
     *  need to make e prime. It does make things easier later.
     */
	while (!probab_prime(e))
		mpz_add_ui(e, e, 2);

#endif

	while (!good_prime(p, e))
		mpz_add_ui(p, p, 2);
	while (!good_prime(q, e))
		mpz_add_ui(q, q, 2);

	/* force p > q */
	if (mpz_cmp(p, q) < 0) {
		mpz_set(t, p);
		mpz_set(p, q);
		mpz_set(q, t);
	}
	clear(t);

	return 1;
}
#endif
#endif

INLINE void lcm(MP_INT * l, MP_INT * a, MP_INT * b)
{
	/* l = a*(b/gcd(a,b)) */
	mpz_gcd(l, a, b);
	mpz_div(l, b, l);
	mpz_mul(l, a, l);
}

/*
 *    Recall that a is a quadratic residue mod b if
 *    x^2 = a mod b      has an integer solution x.
 *
 *    J(a,b) = if a==1 then 1 else
 *             if a is even then J(a/2,b) * ((-1)^(b^2-1)/8))
 *             else J(b mod a, a) * ((-1)^((a-1)*(b-1)/4)))
 *
 *  b>0  b odd
 *
 *
 */
static int jacobi(MP_INT * ac, MP_INT * bc)
{
	int sgn = 1;
	unsigned long c;
	MP_INT *t, *a, *b;
	init(t);
	init(a);
	init(b);

	/* if ac < 0, then we use the fact that
     *  (-1/b)= -1 if b mod 4 == 3
     *          +1 if b mod 4 == 1
     */

	if (mpz_cmp_ui(ac, 0) < 0 && (mpz_get_ui(bc) % 4) == 3)
		sgn = -sgn;

	mpz_abs(a, ac);
	mpz_set(b, bc);

	/* while (a > 1) { */
	while (mpz_cmp_ui(a, 1) > 0) {

		if (even(a)) {

			/* c = b % 8 */
			c = mpz_get_ui(b) & 0x07;

			/* b odd, then (b^2-1)/8 is even iff (b%8 == 3,5) */

			/* if b % 8 == 3 or 5 */
			if (c == 3 || c == 5)
				sgn = -sgn;

			/* a = a / 2 */
			mpz_div_2exp(a, a, 1);

		}
		else {
			/* note: (a-1)(b-1)/4 odd iff a mod 4==b mod 4==3 */

			/* if a mod 4 == 3 and b mod 4 == 3 */
			if (((mpz_get_ui(a) & 3) == 3) && ((mpz_get_ui(b) & 3) == 3))
				sgn = -sgn;

			/* set (a,b) = (b mod a,a) */
			mpz_set(t, a);
			mpz_mmod(a, b, t);
			mpz_set(b, t);
		}
	}

	/* if a == 0 then sgn = 0 */
	if (!mpz_cmp_ui(a, 0))
		sgn = 0;

	clear(t);
	clear(a);
	clear(b);
	return (sgn);
}

/*
 * Used in calculating private key from public key, message, and primes.
 * result = lcm(p-jacobi(m^2-4,p),q-jacobi(m^2-4,q))
 */
INLINE static void s(MP_INT * result, MP_INT * m, MP_INT * p, MP_INT * q)
{
	MP_INT *d, *p1, *q1;
	init(d);
	init(p1);
	init(q1);

	/* d = (m*m)-4; */
	mpz_mul(d, m, m);
	mpz_sub_ui(d, d, 4);

	/* result = lcm(p-jacobi(d,p),q-jacobi(d,q)) */
	mpz_set_si(p1, jacobi(d, p));
	mpz_set_si(q1, jacobi(d, q));
	mpz_sub(p1, p, p1);
	mpz_sub(q1, q, q1);
	lcm(result, p1, q1);

	mpz_clear(p1);
	mpz_clear(q1);
	mpz_clear(d);
}


/*
 * Used in calculating private key from pub key and primes.
 * result = lcm(p-1,q-1,p+1,q+1)
 */
INLINE static void r(MP_INT * result, MP_INT * p, MP_INT * q)
{
	MP_INT *a, *b, *ri;
	init(a);
	init(b);
	init(ri);

	/* result = (lcm(p-1, lcm(p+1, lcm(q-1, q+1)))); */
	mpz_sub_ui(a, q, 1);	/* a = q - 1                */
	mpz_add_ui(b, q, 1);	/* b = q + 1                */
	lcm(result, a, b);	/* result = lcm(a,b)        */
	mpz_add_ui(a, p, 1);	/* a = p + 1                */
	lcm(ri, result, a);	/* result = lcm(result,a)   */
	mpz_sub_ui(a, p, 1);	/* a = p - 1                */
	lcm(result, ri, a);	/* result = lcm(result,a)   */
	clear(a);
	clear(b);
}


/* returns (1/pub) mod s(message,p,q) */
void private_from_message(MP_INT * res, MP_INT * message, MP_INT * pub,
						  MP_INT * p, MP_INT * q)
{
	s(res, message, p, q);	/* res = s(message,p,q) */
	modinv(res, pub, res);	/* res = (1/pub) mod res */
}


/*
 * Calculates LUC private key from pub key, primes. Can be used with any
 *  message but will be VERY big (slow).
 */
void private_from_primes(MP_INT * res, MP_INT * pub, MP_INT * p, MP_INT * q)
{

	/* res = (1/pub) mod r(p,q) */
	r(res, p, q);
	modinv(res, pub, res);
}

/*
 * Lucas function. Calculates V_e(p, 1) mod n
 * We use the following identities for V_n(P,1)
 *  V_{2n} = (V_n)^2 - 2
 *  V_{2n+1} = V_n*V{n-1} - P
 *  V_{2n-1} = P*(V_n)^2 - V_n * V_{n-1} -P
 * and move from most significant bit to least significant bit
 *  in a manner similar to the usual (modular) exponentiation algorithm
 * x and y are V_r and V_{r-1} where r is the number
 *  with binary representation given by some number of leftmost
 *  bits of e. Every time we add one bit we either need to
 *  obtain V_{2n} from V_{n} and V_{n-1} or V_{2n+1} from
 *  V_{n} and V_{n-1}
 */
void v_p1_n(MP_INT * x, MP_INT * ec, MP_INT * p, MP_INT * n)
{
	MP_INT *y, *xx, *yy, *e, *x2, *t;
	struct is *stack = NULL;
	int k, i;
	if (!mpz_cmp_ui(ec, 0)) {
		mpz_set_ui(x, 2);
		return;
	}
	init(xx);
	init(yy);
	init(y);
	init(e);
	init(x2);
	init(t);

	mpz_set(e, ec);	/* e is a copy of ec which we can munge */
	mpz_set(x, p);	/* x = p */
	mpz_set_ui(y, 2);	/* y = 2 */

	/* push the bits of e onto a stack, starting at the rightmost (LSB) */
	for (k = 0; mpz_cmp_ui(e, 0); k++, mpz_div_2exp(e, e, 1))
		push(mpz_get_ui(e) & 1, &stack);
	k--;

	/*
     * at this point all of the bits of e have been pushed. Now
     * when we pop, we'll get them left-most (most significant) first,
     * and right-most (least significant) last.
     */

	/* discard most significant bit */
	i = pop(&stack);

	/* here we start out with r=1, x=V_1=p, y=V_0=2 */
	for (i = k - 1; i >= 0; i--) {

		mpz_mul(x2, x, x);
		mpz_mmod(x2, x2, n);	/* x2 = x^2 mod n */

		if (pop(&stack)) {	/* odd */

			/* xx = (p*x2 - x*y - p) */
			mpz_mul(t, p, x2);
			mpz_mul(xx, x, y);
			mpz_sub(xx, t, xx);
			mpz_sub(xx, xx, p);

			/* yy = (x2 - 2) */
			mpz_sub_ui(yy, x2, 2);
		}
		else {
			/* xx = (x2 - 2) */
			mpz_sub_ui(xx, x2, 2);

			/* yy = (x * y - p) %n */
			mpz_mul(yy, x, y);
			mpz_sub(yy, yy, p);
		}
		/* x = xx mod n, y = yy mod n */
		mpz_mmod(x, xx, n);
		mpz_mmod(y, yy, n);
	}
	clear(xx);
	clear(yy);
	clear(y);
	clear(e);
	clear(x2);
	clear(t);
}

/*
 * calculate V_e(p,q) mod n
 * same idea here, the identities above have more general forms
 *
 * we also need to keep track of q^r and q^{r-1} on the fly
 */
void v_pq_n(MP_INT * x, MP_INT * ec, MP_INT * p, MP_INT * q, MP_INT * n)
{
	MP_INT *y, *xx, *yy, *e, *x2, *t, *qr, *qr1;
	struct is *stack = NULL;
	int k, i;

	/* if (e == 0) return 2 */
	if (!mpz_cmp_ui(ec, 0)) {
		mpz_set_ui(x, 2);
		return;
	}
	init(xx);
	init(yy);
	init(y);
	init(e);
	init(x2);
	init(t);
	init(qr);
	init(qr1);

	mpz_set(e, ec);	/* e is a copy of ec which we can munge */
	mpz_set(x, p);	/* x = p */
	mpz_set_ui(y, 2);	/* y = 2 */
	mpz_set(qr, q);	/* qr = q */
	mpz_set_ui(qr1, 1);	/* qr1 = 1 */
	for (k = 0; mpz_cmp_ui(e, 0); k++, mpz_div_2exp(e, e, 1))
		push(mpz_get_ui(e) & 1, &stack);
	k--;
	i = pop(&stack);

	for (i = k - 1; i >= 0; i--) {
		mpz_mul(x2, x, x);
		mpz_mmod(x2, x2, n);	/* x2 = x^2 mod n */

		if (pop(&stack)) {	/* odd */

			/* xx = (p*x2 - q*x*y - p*q^r) */
			mpz_mul(t, p, x2);
			mpz_mul(xx, x, y);
			mpz_mul(xx, xx, q);
			mpz_sub(xx, t, xx);
			mpz_mul(t, p, qr);
			mpz_sub(xx, xx, t);

			/* yy = (x2 - 2*q^r) */
			mpz_mul_2exp(t, qr, 1);
			mpz_sub(yy, x2, t);

			/* qr1=(qr*qr) % n */
			mpz_mul(qr1, qr, qr);
			mpz_mmod(qr1, qr1, n);

			/* qr= (qr1*q) % n */
			mpz_mul(qr, qr1, q);
			mpz_mmod(qr, qr, n);
		}
		else {
			/* xx = (x2 - 2*q^r) */
			mpz_mul_2exp(t, qr, 1);
			mpz_sub(xx, x2, t);

			/* yy = (x * y - p*q^(r-1)) */
			mpz_mul(yy, x, y);
			mpz_mul(t, qr1, p);
			mpz_sub(yy, yy, t);

			/* qr1 = (qr*qr1) % n */
			mpz_mul(qr1, qr, qr1);
			mpz_mmod(qr1, qr1, n);

			/* qr= (qr1*q) % n */
			mpz_mul(qr, qr1, q);
			mpz_mmod(qr, qr, n);
		}
		/* x = xx mod n, y = yy mod n */
		mpz_mmod(x, xx, n);
		mpz_mmod(y, yy, n);
	}
	clear(xx);
	clear(yy);
	clear(y);
	clear(e);
	clear(x2);
	clear(t);
	clear(qr);
	clear(qr1);
}

/* calculates V_e(p,q) -- no mod!! - you probably don't want to
 * use this (at least not for cryptography)
 */
void v_pq(MP_INT * x, MP_INT * ec, MP_INT * p, MP_INT * q)
{
	MP_INT *y, *xx, *yy, *e, *x2, *t, *qr, *qr1;
	struct is *stack = NULL;
	int k, i;
	/* if (e == 0) return 2 */
	if (!mpz_cmp_ui(ec, 0)) {
		mpz_set_ui(x, 2);
		return;
	}
	init(xx);
	init(yy);
	init(y);
	init(e);
	init(x2);
	init(t);
	init(qr);
	init(qr1);

	mpz_set(e, ec);	/* e is a copy of ec which we can munge */
	mpz_set(x, p);	/* x = p */
	mpz_set_ui(y, 2);	/* y = 2 */
	mpz_set(qr, q);	/* qr = q */
	mpz_set_ui(qr1, 1);	/* qr1 = 1 */
	for (k = 0; mpz_cmp_ui(e, 0); k++, mpz_div_2exp(e, e, 1))
		push(mpz_get_ui(e) & 1, &stack);
	k--;
	i = pop(&stack);

	for (i = k - 1; i >= 0; i--) {
		/* x2 = x*x */
		mpz_mul(x2, x, x);

		if (pop(&stack)) {	/* odd */

			/* xx = (p*x2 - q*x*y - p*q^r) */
			mpz_mul(t, p, x2);
			mpz_mul(xx, x, y);
			mpz_mul(xx, xx, q);
			mpz_sub(xx, t, xx);
			mpz_mul(t, p, qr);
			mpz_sub(xx, xx, t);

			/* yy = (x2 - 2*q^r) */
			mpz_mul_2exp(t, qr, 1);
			mpz_sub(yy, x2, t);

			/* qr1=(qr*qr) */
			mpz_mul(qr1, qr, qr);

			/* qr= (qr1*q) */
			mpz_mul(qr, qr1, q);
		}
		else {
			/* xx = (x2 - 2*q^r) */
			mpz_mul_2exp(t, qr, 1);
			mpz_sub(xx, x2, t);

			/* yy = (x * y - p*q^(r-1)) */
			mpz_mul(yy, x, y);
			mpz_mul(t, qr1, p);
			mpz_sub(yy, yy, t);

			/* qr1 = (qr*qr1) */
			mpz_mul(qr1, qr, qr1);

			/* qr= (qr1*q) */
			mpz_mul(qr, qr1, q);
		}
		/* x = xx, y = yy */
		mpz_set(x, xx);
		mpz_set(y, yy);
	}
	clear(xx);
	clear(yy);
	clear(y);
	clear(e);
	clear(x2);
	clear(t);
	clear(qr);
	clear(qr1);
}


void halfrsa(MP_INT * s, MP_INT * m, MP_INT * e, MP_INT * n,
			 MP_INT * p, MP_INT * q)
{
	MP_INT *p2, *q2, *d1, *d2, *m1, *m2, *u, *v, *t, *d;
	init(p2);
	init(q2);
	init(d1);
	init(d2);
	init(m1);
	init(m2);
	init(t);
	init(u);
	init(v);
	init(d);

	/* m = m % n */
	mpz_mmod(m, m, n);

	/* m1 = m mod p */
	mpz_mmod(m1, m, p);
	/* m2 = m mod q */
	mpz_mmod(m2, m, q);

	/* u=p-1, v=q-1 , t = (p-1)(q-1) */
	mpz_sub_ui(u, p, 1);
	mpz_sub_ui(v, q, 1);
	mpz_mul(t, u, v);

	/* d = (1/e) mod (p-1)(q-1) */
	modinv(d, e, t);

	/* d1 = d mod (p-1) */
	mpz_mmod(d1, d, u);
	/* d2 = d mod (q-1) */
	mpz_mmod(d2, d, v);

	/* p2 = m1 ^ d1 mod p */
	mpz_powm(p2, m1, d1, p);

	/* q2 = m2 ^ d2 mod q */
	mpz_powm(q2, m2, d2, q);

	/* if (p < q)  (p,q,p2,q2) = (q,p,q2,p2) */
	if (mpz_cmp(p, q) < 0) {
		mpz_set(t, q);
		mpz_set(q, p);
		mpz_set(p, t);
		mpz_set(t, q2);
		mpz_set(q2, p2);
		mpz_set(p2, t);
	}

	/* now use CRT to get the m in Z_n corresponding to (p2,q2) */

	/* return ((((p2 - q2) % p) * ((1/q)mod p)) % p) * q + q2; */
	mpz_sub(t, p2, q2);
	mpz_mmod(t, t, p);
	modinv(u, q, p);	/* should be precomputed on key generation */
	mpz_mul(t, u, t);
	mpz_mmod(t, t, p);
	mpz_mul(t, q, t);
	mpz_add(s, t, q2);

	clear(p2);
	clear(q2);
	clear(d1);
	clear(d2);
	clear(m1);
	clear(m2);
	clear(t);
	clear(u);
	clear(v);
	clear(d);
}

/* no need to compute private key from message before calling halfluc
   we do it in here. Note the technique here is similar to that
   used in RSA to decrease time to decrypt/sign messages

   in a "real" implementation, we'd precompute the four possible
   private keys and q^{-1} mod p.
   [ Note: it turns out that this precomputation isn't worth the trouble.
          Performance improvement in halfluc is <5% ]
   Precondition: of course, m^2 - 4 must be coprime to n
*/
void halfluc(MP_INT * s, MP_INT * m, MP_INT * e, MP_INT * n,
			 MP_INT * p, MP_INT * q)
{
	MP_INT *t, *pa, *qa, *q2, *p2, *di, *u, *d;
	int dip, diq;
	init(t);
	init(pa);
	init(qa);
	init(p2);
	init(q2);
	init(di);
	init(d);
	init(u);

	/* m = m % n */
	mpz_mmod(m, m, n);

	/* di = (m*m)-4; */
	mpz_mul(di, m, m);
	mpz_sub_ui(di, di, 4);

	dip = jacobi(di, p);
	diq = jacobi(di, q);

	/* pa = p - jacobi(di,p); qa = q - jacobi(di,q); */
	mpz_set_si(pa, dip);
	mpz_set_si(qa, diq);
	mpz_sub(pa, p, pa);
	mpz_sub(qa, q, qa);

	/* calculate lcm */
	lcm(t, pa, qa);

	/* d = (1/e) mod lcm(pa,qa) */
	modinv(d, e, t);

	/* p2 = v_p1_n(d % pa, m % p,p); */
	mpz_mmod(t, d, pa);
	mpz_mmod(u, m, p);
	v_p1_n(p2, t, u, p);

	/* q2 = v_p1_n(d % qa, m % q,q); */
	mpz_mmod(t, d, qa);
	mpz_mmod(u, m, q);
	v_p1_n(q2, t, u, q);

	/* chinese remainder theorem - pull back from Z_p x Z_q to Z_n
     * the key is the observation that (p2,q2) are the projections
     *  of the desired result (in Z_n) to Z_p x Z_q
     * ain't explained worth a damn in the DDJ article (listing 3),
     * but pretty obvious if you now how RSA is done in practice
     */

	/* First force p > q */

	/* if (p < q)  (p,q,p2,q2) = (q,p,q2,p2) */
	if (mpz_cmp(p, q) < 0) {
		mpz_set(t, q);
		mpz_set(q, p);
		mpz_set(p, t);
		mpz_set(t, q2);
		mpz_set(q2, p2);
		mpz_set(p2, t);
	}

	/* now use CRT to get the m in Z_n corresponding to (p2,q2) */

	/* return ((((p2 - q2) % p) * ((1/q)mod p)) % p) * q + q2; */
	mpz_sub(t, p2, q2);
	mpz_mmod(t, t, p);
	modinv(u, q, p);	/* should be precomputed on key generation */
	mpz_mul(t, u, t);
	mpz_mmod(t, t, p);
	mpz_mul(t, q, t);
	mpz_add(s, t, q2);

	clear(t);
	clear(pa);
	clear(qa);
	clear(p2);
	clear(q2);
	clear(di);
	clear(d);
	clear(u);
}

/* key length is to 80 bytes (really we encrypt KEY+IV
   80 = 24  3DES
        16  IDEA
         8  DES
	     4  tran1
         4  tran2
        24  3 IVs
really, I shouldn't bother encrypting the IVs
*/
/* LKEYLEN and HASHLEN must match value defined in l3.h */
#define LKEYLEN 96
#define HASHLEN 32

static INLINE int bytes(MP_INT * n)
{
	int i = 0;
	MP_INT *t;

	init(t);
	mpz_set(t, n);
	while (mpz_cmp_ui(t, 0)) {
		i++;
		mpz_div_2exp(t, t, 8);
	}
	clear(t);
	return i;
}

#ifndef STAND
void encrypt_session_key(MP_INT * c, unsigned char *m, MP_INT * n, MP_INT * e,
						 int alg)
{
	int nn, i;
	unsigned char ribuf[MAXC];
	MP_INT *mm;

	init(mm);
#ifdef DEBUG
	fprintf(stderr, "encrypt_session_key: session key is:");
	for (i = 0; i < LKEYLEN; i++)
		fprintf(stderr, "%02x", *(m + i));
	fprintf(stderr, "\n");
#endif
	nn = bytes(n);
	ribuf[0] = 0;
	for (i = 1; i <= (nn - LKEYLEN) - 1; i++)
		ribuf[i] = randombyte();
	memcpy(&ribuf[i], m, LKEYLEN);
	cstmp(mm, ribuf, nn);

#ifdef DEBUG
	fprintf(stderr, "padded plaintext = ");
	mpz_out_str(stderr, 16, mm);
	fprintf(stderr, "\n");
#endif

	if (alg == LUC)
		luc(c, mm, e, n);
	else if (alg == RSA)
		rsa(c, mm, e, n);
	else if (alg == RL) {
		MP_INT *c1;
		init(c1);
		luc(c1, mm, e, n);
		rsa(c, c1, e, n);
		clear(c1);
	}
	else
		ferr("encrypt_session_key: invalid paramaters\n");

	zap(ribuf, MAXC);
	clear(mm);
}

void decrypt_session_key(unsigned char *k, MP_INT * c, MP_INT * p,
						 MP_INT * q, MP_INT * e, int alg)
{
	MP_INT *m;
	MP_INT *n;
	unsigned char ribuf[MAXC];

	init(m);
	init(n);
	mpz_mul(n, p, q);
#ifdef DEBUG
	fprintf(stderr, "ciphertext = ");
	mpz_out_str(stderr, 16, c);
	fprintf(stderr, "\n");
#endif
	if (alg == LUC)
		halfluc(m, c, e, n, p, q);
	else if (alg == RSA)
		halfrsa(m, c, e, n, p, q);
	else if (alg == RL) {
		MP_INT *c1;
		init(c1);
		halfrsa(c1, c, e, n, p, q);
		halfluc(m, c1, e, n, p, q);
		clear(c1);
	}
	else
		ferr("decrypt_session_key: invalid paramaters\n");
#ifdef DEBUG
	fprintf(stderr, "decrypted plaintext = ");
	mpz_out_str(stderr, 16, m);
	fprintf(stderr, "\n");
#endif
	mptcs(ribuf, MAXC, m);
	memcpy(k, &ribuf[MAXC - LKEYLEN], LKEYLEN);
	zap(ribuf, MAXC);
	clear(n);
}

void sign(MP_INT * c, unsigned char *hash, MP_INT * p, MP_INT * q, MP_INT * e,
		  int alg)
{
	int nn, i;
	unsigned char ribuf[MAXC];
	MP_INT *mm, *n;

	init(mm);
	init(n);
	mpz_mul(n, p, q);
	nn = bytes(n);
	ribuf[0] = 0;
	for (i = 1; i <= (nn - HASHLEN) - 1; i++)
		ribuf[i] = 0xff;
	memcpy(&ribuf[i], hash, HASHLEN);
	cstmp(mm, ribuf, nn);
	if (alg == LUC)
		halfluc(c, mm, e, n, p, q);
	else if (alg == RSA)
		halfrsa(c, mm, e, n, p, q);
	else if (alg == RL) {
		MP_INT *c1;
		init(c1);
		halfrsa(c1, mm, e, n, p, q);
		halfluc(c, c1, e, n, p, q);
		clear(c1);
	}
	else
		ferr("sign: invalid paramaters\n");

	zap(ribuf, MAXC);
	clear(mm);
}

int vsign(MP_INT * c, unsigned char *hash, MP_INT * n, MP_INT * e, int alg)
{
	int nn, i;
	MP_INT *m;
	unsigned char ribuf[MAXC];
	init(m);

	nn = bytes(n);
	if (alg == LUC)
		luc(m, c, e, n);
	else if (alg == RSA)
		rsa(m, c, e, n);
	else if (alg == RL) {
		MP_INT *c1;
		init(c1);
		luc(c1, c, e, n);
		rsa(m, c1, e, n);
		clear(c1);
	}
	else
		ferr("vsign: invalid paramaters\n");
	mptcs(ribuf, nn, m);
	for (i = 0; i < HASHLEN; i++)
		if (hash[i] != ribuf[nn - HASHLEN + i])
			return 0;
	for (i = 1; i < nn - HASHLEN; i++)
		if (ribuf[i] != 0xff) {
			clear(m);
			return 0;
		}
	if (ribuf[0] != 0) {
		clear(m);
		return 0;
	}
	clear(m);
	return 1;
}

#endif


/* TEST CODE BEGINS HERE -- this tests the raw LUC functions
 * only. You can probably safely ignore the rest of this.
 */

#ifdef TEST

void checkpoint(char *s)
{
	struct tms foo;
	static time_t lastc = 0;
	times(&foo);
	printf("%d ticks to %s\n", foo.tms_utime - lastc, s);
	lastc = foo.tms_utime;
}

test(MP_INT * prime1, MP_INT * prime2, MP_INT * plain, MP_INT * public)
{
	static int number = 0;
	MP_INT *one, *modulus, *c1, *c2, *k1, *k2, *p1, *p2, *p3;
	MP_INT *s1, *s2, *s3, *pp1, *pp2, *pp3, *kk1;

	init(one);
	init(modulus);
	init(c1);
	init(c2);
	init(k1);
	init(k2);
	init(p1);
	init(p2);
	init(p3);
	init(kk1);
	init(s1);
	init(s2);
	init(s3);
	init(pp1);
	init(pp2);
	init(pp3);
	number++;
	printf("TEST NUMBER %d\n", number);
	mpz_set_ui(one, 1);
	mpz_mul(modulus, prime1, prime2);

	checkpoint("set up");
	luc(c1, plain, public, modulus);
	checkpoint("encrypt with v_p1_n");
	v_pq_n(c2, public, plain, one, modulus);
	checkpoint("encrypt with v_pq_n");
	if (mpz_cmp(c1, c2)) {
		printf("FAILED: c1 != c2");
		chars("c1", c1);
		chars("c2", c2);
		exit(1);
	}
	checkpoint("verify1");
	private_from_message(k1, c1, public, prime1, prime2);
	checkpoint("compute private from ciphertext");
	halfluc(p3, c1, public, modulus, prime1, prime2);
	checkpoint("decrypt with halfluc (should be fastest way to decrypt)");
	luc(p1, c1, k1, modulus);
	checkpoint("decrypt with private from ciphertext");
	private_from_primes(k2, public, prime1, prime2);
	checkpoint("compute private from primes");
	luc(p2, c1, k2, modulus);
	checkpoint(
	"decrypt with private from primes (should be slowest way to decrypt");
	/* now we must make sure that plain=p1=p2=p3 */
	if (mpz_cmp(plain, p1) || mpz_cmp(p1, p2) || mpz_cmp(p2, p3)) {
		printf("FAILED DECRYPTION TEST\n");
		chars("p1", p1);
		chars("p2", p2);
		chars("p3", p3);
		chars("plain", plain);
		exit(1);
	}
	checkpoint("verify2");
	private_from_message(kk1, plain, public, prime1, prime2);
	checkpoint("compute private from plaintext");
	halfluc(s3, plain, public, modulus, prime1, prime2);
	checkpoint("sign using halfluc (should be fastest way to sign w/LUC)");
	halfrsa(p3, plain, public, modulus, prime1, prime2);
	checkpoint("sign using halfrsa");
	luc(s1, plain, kk1, modulus);
	checkpoint("sign using private from plaintext");
	luc(s2, plain, k2, modulus);
	checkpoint(
	   "sign using private from primes (should be slowest way to sign)");
	if (mpz_cmp(s1, s2) || mpz_cmp(s2, s3)) {
		printf("FAILED SIGNATURE GENERATION TEST\n");
		chars("s1", s1);
		chars("s2", s2);
		chars("s3", s3);
		exit(1);
	}
	checkpoint("verify3");
	luc(pp1, s1, public, modulus);
	checkpoint("verify LUC signature");
	if (mpz_cmp(pp1, plain)) {
		printf("FAILED SIGNATURE VERIFICATION TEST\n");
		chars("pp1", pp1);
		exit(1);
	}
	rsa(pp1, p3, public, modulus);
	checkpoint("verify RSA signature");
	if (mpz_cmp(pp1, plain)) {
		printf("FAILED SIGNATURE VERIFICATION TEST\n");
		chars("pp1", pp1);
		exit(1);
	}
	chars("plaintext  ", plain);
	chars("ciphertext ", c1);
	chars("signature  ", s1);
	printf("PASSED TEST NUMBER %d\n", number);

	clear(s1);
	clear(s2);
	clear(s3);
	clear(pp1);
	clear(pp2);
	clear(pp3);
	clear(one);
	clear(modulus);
	clear(c1);
	clear(c2);
	clear(k1);
	clear(k2);
	clear(p1);
	clear(p2);
	clear(p3);
	clear(kk1);
}


main()
{
	MP_INT *prime1, *prime2, *public, *plain, *n;
	init(prime1);
	init(prime2);
	init(public);
	init(plain);
	init(n);
	mpz_set_ui(prime1, 1949);
	mpz_set_ui(prime2, 2089);
	mpz_set_ui(public, 1103);
	mpz_set_ui(plain, 11111);
	test(prime1, prime2, plain, public);

	mpz_set_ui(plain, 1);
	test(prime1, prime2, plain, public);
	mpz_set_str(public, "65537", 10);
	mpz_set_str(prime1, "2229581051182913", 10);
	mpz_set_str(prime2, "387833844699347", 10);
	mpz_set_ui(plain, 11111);
	test(prime1, prime2, plain, public);
	mpz_set_ui(plain, 1);
	test(prime1, prime2, plain, public);

/* expect the following to take a while. */
	mpz_set_str(prime1, "12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071", 10);
	mpz_set_str(prime2, "10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381", 10);
	mpz_set_ui(public, 65537);
	mpz_mul(n, prime1, prime2);
	mpz_random(plain, 32);
	mpz_mmod(plain, plain, n);
	test(prime1, prime2, plain, public);
}

#endif
#ifdef TEST2
main()
{
	MP_INT *prime1, *prime2, *public, *plain, *n, *c, *plain2;
	init(prime1);
	init(prime2);
	init(public);
	init(plain);
	init(n);
	init(plain);
	init(plain2);
	init(c);
	mpz_set_str(prime2, "12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071", 10);
	mpz_set_str(prime1, "10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381", 10);
	mpz_set_ui(public, 65537);
	mpz_mul(n, prime1, prime2);
	while (1) {
		mpz_random(plain, 32);
		mpz_mmod(plain, plain, n);
		luc(c, plain, public, n);
		halfluc(plain2, c, public, n, prime1, prime2);
		if (mpz_cmp(plain, plain2)) {
			printf("failed\n");
			chars("plain", plain);
			chars("c", c);
			chars("plain2", plain2);
			exit(1);
		}
		else {
			printf("passed\n");
		}
	}
}

#endif
#ifdef TEST3
char vb9[2048], eb9[2048], Pb9[2048], Qb9[2048];
#define disp {\
    mpz_get_str(vb9,10,v); \
    mpz_get_str(eb9,10,e); \
    mpz_get_str(Pb9,10,P); \
    mpz_get_str(Qb9,10,Q); \
    printf("V_%s(%s,%s) mod 2^68 = %s -- OK\n", eb9,Pb9,Qb9,vb9); }
#define disp2 {\
    mpz_get_str(vb9,16,v); \
    mpz_get_str(eb9,10,e); \
    mpz_get_str(Pb9,10,P); \
    mpz_get_str(Qb9,10,Q); \
    printf("V_%s(%s,%s) = 0x%s -- OK\n", eb9,Pb9,Qb9,vb9); }

#define test3(ip,iq,ie,is)  \
    mpz_set_si(P,ip); \
    mpz_set_si(Q,iq); \
    mpz_set_ui(e,ie); \
    mpz_set_str(w,is,10) ; \
    v_pq_n(v,e,P,Q,n) ; \
    if (mpz_cmp(v,w)) { \
        printf("failed\n"); \
        chars("v",v); \
        chars("w",w); \
        chars("P",P); \
        chars("Q",Q); \
        chars("e",e); \
        exit(1); \
    } \
    else { \
        disp; \
    }  \
    v_pq(v,e,P,Q) ; \
    if (mpz_cmp(v,w)) { \
        printf("failed\n"); \
        chars("v",v); \
        chars("w",w); \
        chars("P",P); \
        chars("Q",Q); \
        chars("e",e); \
        exit(1); \
    } \
    else { \
        disp2; \
    }

int smallprimes[] =
{2, 3, 5, 7, 11, 13, 17, 65537};
int nsmallprimes = 8;


main()
{
	MP_INT *P;
	MP_INT *Q;
	MP_INT *v;
	MP_INT *e;
	MP_INT *w;
	MP_INT *n;
	MP_INT *x;
	MP_INT *vn, *vn1;
	unsigned int i, j, k;
	init(P);
	init(Q);
	init(v);
	init(e);
	init(w);
	init(n);
	init(x);
	init(vn);
	init(vn1);

	mpz_set_ui(n, 1);
	mpz_mul_2exp(n, n, 68);
	/* check that V_38(1,-1) = 87403803 */
	test3(1, -1, 38, "87403803");
	/* V_37(1,-1) = 54018521 */
	test3(1, -1, 37, "54018521");

	/* some Companion Pell Numbers */
	test3(2, -1, 17, "3215042");
	test3(2, -1, 25, "3710155682");
	test3(2, -1, 37, "145445522951122");
	test3(2, -1, 38, "351136554095046");

	/* some for P=4 Q=3 */
	test3(4, 3, 33, "5559060566555524");
	test3(4, 3, 27, "7625597484988");

	for (i = 64; i < 256; i++) {
		/* w = 2^i + 1 */
		mpz_set_ui(w, 1);
		mpz_mul_2exp(w, w, i);
		mpz_add_ui(w, w, 1);

		/* v = V_i(3,2) */
		mpz_set_si(P, 3);
		mpz_set_si(Q, 2);
		mpz_set_ui(e, i);
		v_pq(v, e, P, Q);

		/* if v != w , we have a problem */
		if (mpz_cmp(v, w)) {
			printf("test failed\n");
			printf("i=%d\n", i);
			chars("v", v);
			chars("w", w);
			exit(1);
		}
		else {
			disp2;
		}
	}

	for (i = 0; i < 64; i++) {
		/* w = 2^i + 1 */
		mpz_set_ui(w, 1);
		mpz_mul_2exp(w, w, i);
		mpz_add_ui(w, w, 1);

		/* v = V_i(3,2) */
		mpz_set_si(P, 3);
		mpz_set_si(Q, 2);
		mpz_set_ui(e, i);
		v_pq_n(v, e, P, Q, n);

		/* if v != w , we have a problem */
		if (mpz_cmp(v, w)) {
			printf("test failed\n");
			printf("i=%d\n", i);
			chars("v", v);
			chars("w", w);
			exit(1);
		}
		else {
			disp;
		}
	}

	for (i = 0; i < nsmallprimes; i++) {
		for (j = 0; i < nsmallprimes; i++) {
			if (i != j) {
				for (k = 2; k < 65; k++) {
					int a = smallprimes[i], b = smallprimes[j];

					/* P = a+b, Q=ab */
					mpz_set_ui(P, a);
					mpz_add_ui(P, P, b);
					mpz_set_ui(Q, a);
					mpz_mul_ui(Q, Q, b);

					/* set w = (P^k + Q^k) mod n*/
					mpz_set_ui(w, a);
					mpz_powm_ui(w, w, k, n);
					mpz_set_ui(x, b);
					mpz_powm_ui(x, x, k, n);
					mpz_add(w, w, x);
					mpz_mod(w, w, n);

					mpz_set_ui(e, k);
					v_pq_n(v, e, P, Q, n);
					if (mpz_cmp(w, v)) {
						printf("failed\n");
						chars("v", v);
						chars("w", w);
						chars("P", P);
						chars("Q", Q);
						chars("e", e);
						exit(1);
					}
					else {
						disp;
					}
				}
			}
		}
	}

	mpz_set_str(P, "3172873123443364723647236476432723641", 10);
	mpz_set_str(Q, "64638746327463278463278463278463287463637", 10);
	mpz_set(vn, P);
	mpz_set_ui(vn1, 2);
	for (i = 2; i < 255; i++) {

		/* x = P*vn - Q*vn1 */
		mpz_mul(x, P, vn);
		mpz_mul(w, Q, vn1);
		mpz_sub(x, x, w);

		/* vn1 = vn, vn = x */
		mpz_set(vn1, vn);
		mpz_set(vn, x);

		mpz_set_ui(x, i);
		v_pq(v, x, P, Q);
		if (mpz_cmp(v, vn)) {
			printf("error\n");
			exit(1);
		}
		else {
			chars("v", v);
		}
		v_pq_n(v, x, P, Q, n);
		mpz_mmod(x, vn, n);
		if (mpz_cmp(x, v)) {
			printf("error\n");
			exit(1);
		}
		else {
			chars("x", x);
		}
	}
}

#endif
