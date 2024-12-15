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

#include <gmp.h>
#include "lucas.h"

#if defined(TEST) || defined(TEST2) || defined(TEST3)
char rbuf[2048];
#define chars(label,bignum) {mpz_get_str(rbuf,10,bignum); \
    printf("%s = %s\n", label,rbuf); }
#endif

unsigned char ver[]="@(#)L3 0.71 - Mark C. Henderson - markh@wimsey.bc.ca";

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

void 
modinv(MP_INT *s, MP_INT *i, MP_INT *n) {
    MP_INT *g,*h;
    init(g);init(h);
    mpz_gcdext(g,h,NULL,i,n);
    
    /* if (g == 1) */
    if (mpz_cmp_ui(g,1) == 0) {

        /* s = h % n */
        mpz_mmod(s,h,n);
    }
    else {

        /* s = 0 */
        mpz_set_ui(s,0);
    }
    clear(g);clear(h);
}

/* convert character string a into b */
void cstmp(MP_INT *a,unsigned char *b,unsigned int len)
{
    unsigned int i,u; 
    unsigned int t;
    int j;
    MP_INT tt,uu;
    mpz_init(&tt); mpz_init(&uu);
    mpz_set_ui(a,0);
    mpz_set_ui(&uu,1);
    for (i=0, j=len-1; j>=0; i++) {
        t=0;
        for (u = 0; j >= 0 && u < LIMB_BITS; j--, u += 8)
            t |= ((unsigned int)b[j]) << u;
        if (t) {
            mpz_set_ui(&tt,t);
            mpz_mul(&tt,&uu,&tt);
            mpz_add(a,&tt,a);
        }
        mpz_mul_2exp(&uu,&uu,LIMB_BITS);
    }
    mpz_clear(&tt);
}
    
/* convert b to a character string */
void mptcs(unsigned char *a, unsigned int len, MP_INT *b)
{
  unsigned int t;
  int j;
  unsigned int i, u;
  MP_INT bc;
  MP_INT zero;
  mpz_init(&zero);
  mpz_init_set(&bc,b);

  for (i = 0, j = len - 1; mpz_cmp(&bc,&zero) ; i++) {
    /* t = b[i]; */
    t=mpz_get_ui(&bc);
    mpz_div_2exp(&bc,&bc,LIMB_BITS);
    for (u = 0; j >= 0 && u < LIMB_BITS; j--, u += 8)
      a[j] = (unsigned char)(t >> u);
  }

  for (; j >= 0; j--)
    a[j] = 0;
  mpz_clear(&zero);
  mpz_clear(&bc);
}

unsigned int tiny_primes[] = {3,5,7,11,13,17};
#define ntiny_primes 6

int
good_prime(MP_INT *p, MP_INT *e)
{
    MP_INT *t; int i; MP_INT *g;
    int status;

    /* is p even */
    i = mpz_get_ui(p);
    if (!(i % 2)) 
        return 0;
        
    init(t);
    for (i=0; i<ntiny_primes; i++) {

        /* t = p % tiny_primes[i] */
        mpz_mmod_ui(t,p,tiny_primes[i]);

        /* if t = 0,  p sure isn't prime */
        if (!mpz_cmp_ui(t,0)) {
            clear(t); return 0;
        }
    }
    /* t = 2 ^ p mod p */
    mpz_set_ui(t,2); mpz_powm(t,t,p,p);

    /* if (t != 2) */
    if (mpz_cmp_ui(t,2)) {
        clear(t); return 0;
    }
    
    init(g);

    /* g = gcd(p+1,e) */
    mpz_add_ui(t,p,1); mpz_gcd(g,t,e);

    /* if (g != 1) */
    if (mpz_cmp_ui(g,1)) {
        clear(t); clear(g); return 0;
    }
        
    /* g = gcd(p-1,e) */
    mpz_sub_ui(t,p,1); mpz_gcd(g,t,e);

    /* if (g != 1) */
    if (mpz_cmp_ui(g,1)) {
        clear(t); clear(g); return 0;
    }

    /* g = gcd(p,e) */
    mpz_gcd(g,p,e);

    /* if (g != 1) */
    if (mpz_cmp_ui(g,1)) {
        clear(t); clear(g); return 0;
    }

    status = mpz_probab_prime_p(p,20);

    clear(t); clear(g); return status;
}

#ifndef STAND

/* genkey generates a LUCAS private key
    p,q are "returned"
    e is input public key
    bits is requested number of bits for each of p and q
    randbuf is an array of user supplied "random" data
    randbuflen is the length of randbuf

    you should clear randbuf after calling this

    returns 1 on success. 
            0 if randbuf is too short

    expects function randombyte to be defined returning a 
    "random" unsigned char 
*/
int
genkey(MP_INT *p, MP_INT *q,  MP_INT *e, unsigned int bits) {
    MP_INT *t; int i;
    int bytes = (bits + 7)/8;
    unsigned char *randbuf;

    randbuf=(unsigned char *)malloc(bytes+bytes);

    for (i=0;i<bytes+bytes;i++)
        *(randbuf+i) = randombyte();
        
    cstmp(p, randbuf, bytes);
    cstmp(q, randbuf + bytes, bytes);

    for (i=0;i<bytes+bytes;i++)
        *(randbuf+i) = 0;
    free(randbuf);

    init(t);
    mpz_set_ui(t,1);
    mpz_mul_2exp(t,t,bits);
    mpz_div_2exp(t,t,1);

    if (mpz_cmp(p,t) < 0)
        mpz_add(p,t,p);
    if (mpz_cmp(q,t) < 0)
        mpz_add(q,t,q);

    if (even(p))
        mpz_add_ui(p,p,1);
    if (even(q))
        mpz_add_ui(q,q,1);

    while (!good_prime(p,e))
        mpz_add_ui(p,p,2);
    while (!good_prime(q,e))
        mpz_add_ui(q,q,2);

    /* force p > q */
    if (mpz_cmp(p,q) < 0) {
        mpz_set(t,p); mpz_set(p,q); mpz_set(q,t);
    }

    clear(t); 
    return 1;
}
#endif

void 
lcm(MP_INT *l, MP_INT *a, MP_INT *b)
{
    /* l = a*(b/gcd(a,b)) */
    mpz_gcd(l,a,b);
    mpz_div(l,b,l);
    mpz_mul(l,a,l);
}

int 
even(MP_INT *a) {
    return !(mpz_get_ui(a) %2);
}

int 
odd(MP_INT *a) {
    return (mpz_get_ui(a) %2);
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
int 
jacobi(MP_INT *ac, MP_INT *bc) 
{
    int sgn = 1;
    unsigned long c;
    MP_INT *t,*a,*b; 
    init(t); init(a); init(b);

    /* if ac < 0, then we use the fact that 
     *  (-1/b)= -1 if b mod 4 == 3
     *          +1 if b mod 4 == 1
     */

    if (mpz_cmp_ui(ac,0) < 0 && (mpz_get_ui(bc) % 4) == 3)
        sgn=-sgn;

    mpz_abs(a,ac); mpz_set(b,bc);

    /* while (a > 1) { */
    while(mpz_cmp_ui(a,1) > 0) {

        if (even(a)) {

            /* c = b % 8 */
            c = mpz_get_ui(b) % 8;  

            /* b odd, then (b^2-1)/8 is even iff (b%8 == 3,5) */

            /* if b % 8 == 3 or 5 */
            if (c == 3 || c == 5)
                sgn = -sgn;

            /* a = a / 2 */
            mpz_div_2exp(a,a,1); 

        } 
        else {
            /* note: (a-1)(b-1)/4 odd iff a mod 4==b mod 4==3 */

            /* if a mod 4 == 3 and b mod 4 == 3 */
            if (((mpz_get_ui(a) % 4) == 3) && ((mpz_get_ui(b) % 4) == 3))
                sgn = -sgn;

            /* set (a,b) = (b mod a,a) */
            mpz_set(t,a); mpz_mmod(a,b,t); mpz_set(b,t);
        } 
    }

    /* if a == 0 then sgn = 0 */
    if (!mpz_cmp_ui(a,0))
        sgn=0;

    clear(t); clear(a); clear(b);
    return (sgn);
}

/*
 * Used in calculating private key from public key, message, and primes.
 */
void 
s(MP_INT *result, MP_INT *m, MP_INT *p, MP_INT *q)
{
    MP_INT *d,*p1,*q1;
    init(d);init(p1);init(q1);

    /* d = (m*m)-4; */
    mpz_mul(d,m,m); mpz_sub_ui(d,d,4);                                      

    /* result = lcm(p-jacobi(d,p),q-jacobi(d,q)) */
    mpz_set_si(p1,jacobi(d,p)); 
    mpz_set_si(q1,jacobi(d,q));
    mpz_sub(p1,p,p1); mpz_sub(q1,q,q1);
    lcm(result,p1,q1);

    mpz_clear(p1); mpz_clear(q1); mpz_clear(d);
}


/*
 * Used in calculating private key from pub key and primes.
 */
void
r(MP_INT *result, MP_INT *p, MP_INT *q) {
    MP_INT *a,*b,*ri;
    init(a); init(b); init(ri);

    /* result = (lcm(p-1, lcm(p+1, lcm(q-1, q+1)))); */
    mpz_sub_ui(a,q,1);      /* a = q - 1                */
    mpz_add_ui(b,q,1);      /* b = q + 1                */
    lcm(result,a,b);        /* result = lcm(a,b)        */
    mpz_add_ui(a,p,1);      /* a = p + 1                */
    lcm(ri,result,a);       /* result = lcm(result,a)   */
    mpz_sub_ui(a,p,1);      /* a = p - 1                */
    lcm(result,ri,a);       /* result = lcm(result,a)   */
    clear(a); clear(b);
}


/* returns (1/pub) mod s(message,p,q) */
void
private_from_message(MP_INT *res, MP_INT *message, MP_INT *pub, 
    MP_INT *p, MP_INT *q) 
{
    s(res,message,p,q);     /* res = s(message,p,q) */
    modinv(res,pub,res);    /* res = (1/pub) mod res */
}


/*
 * Calculates LUC private key from pub key, primes. Can be used with any
 *  message but will be VERY big (slow).
 */
void
private_from_primes(MP_INT *res, MP_INT *pub, MP_INT *p, MP_INT *q) {

    /* res = (1/pub) mod r(p,q) */
    r(res,p,q);
    modinv(res,pub,res);
}

/*
 * Lucas function. Calculates V_e(p, q) mod n
 * This could be made considerably faster with a little work - mch
 */
void 
v_pq_n(MP_INT *v, MP_INT *subscriptc, MP_INT *p, MP_INT *q, MP_INT *modulus) 
{
    MP_INT *vm1, *vm1_n, *v_n, *testbit, *qek, *qekm1, *qek_n, *qekm1_n, *t;
    MP_INT *subscript;

    /* if (subscript == 0)  return 2 */
    if (mpz_cmp_ui(subscriptc,0) == 0) {
        mpz_set_ui(v,2);
        return;
    }
    init(vm1); init(vm1_n); init(v_n); init(testbit); init(qek); 
    init(qekm1); init(qek_n); init(qekm1_n); init(t); init(subscript);
    mpz_set(subscript,subscriptc);

    mpz_mmod(p,p,modulus);                        /* p = p % modulus; */
    mpz_set_ui(vm1,2);                            /* vm1 = 2;         */
    mpz_set(v,p);                                 /* v = p;           */
    mpz_set(qek,q);                               /* qek = q;         */
    mpz_set_ui(qekm1,1);                          /* qekm1 = 1;       */
    
    mpz_set_ui(testbit,1);                        /* testbit = 1;     */

    /* while (testbit <= subscript) testbit*=2; */
    while(mpz_cmp(testbit,subscript) <= 0) 
        mpz_mul_2exp(testbit,testbit,1);

    mpz_div_2exp(testbit,testbit,1);              /* testbit /=2 */
    mpz_sub(subscript,subscript,testbit);         /* subscript-=testbit*/

    mpz_div_2exp(testbit,testbit,1);              /* testbit /=2 */

    /* while (testbit >= 1) { */
    while (mpz_cmp_ui(testbit,1) >= 0) {

        /* if (testbit > subscript) */
        if (mpz_cmp(testbit,subscript) > 0) {
            
            /* vm1_n = (v*vm1)%modulus; */
            mpz_mul(t,v,vm1); mpz_mmod(vm1_n, t, modulus);

            /* vm1_n += (modulus-((p*qekm1)%modulus))%modulus; */
            mpz_mul(t,p,qekm1); mpz_mmod(t,t,modulus); mpz_sub(t,modulus,t); 
            mpz_mmod(t,t,modulus); mpz_add(vm1_n,t,vm1_n);

            /* vm1_n = vm1_n % modulus; */
            mpz_mmod(vm1_n,vm1_n,modulus);

            /* v_n = (v*v)%modulus; */
            mpz_mul(t,v,v); mpz_mmod(v_n,t,modulus);

            /* v_n += (modulus-((qek+qek)%modulus))%modulus; */
            mpz_add(t,qek,qek); mpz_mmod(t,t,modulus); mpz_sub(t,modulus,t); 
            mpz_mmod(t,t,modulus); mpz_add(v_n,t,v_n);
    
            /* v_n %= modulus; */
            mpz_mmod(v_n,v_n,modulus);

            /* qek_n = qek*qek; */
            mpz_mul(qek_n,qek,qek);

            /* qekm1_n = qek*qekm1; */
            mpz_mul(qekm1_n,qek,qekm1);;
        } 
        else {

            /* vm1_n = (v*v)%modulus; */
            mpz_mul(vm1_n,v,v); mpz_mmod(vm1_n,vm1_n,modulus);

            /* vm1_n += (modulus-((qek+qek)%modulus))%modulus; */
            mpz_add(t,qek,qek); mpz_mmod(t,t,modulus); mpz_sub(t,modulus,t); 
            mpz_mmod(t,t,modulus); mpz_add(vm1_n,t,vm1_n);

            /* vm1_n %= modulus; */
            mpz_mmod(vm1_n,vm1_n,modulus);

            /* v_n = (p*((v*v)%modulus))%modulus; */
            mpz_mul(t,v,v); mpz_mmod(t,t,modulus); mpz_mul(t,p,t);
            mpz_mmod(v_n,t,modulus);

            /* v_n += (modulus-((q*v*vm1)%modulus))%modulus; */
            mpz_mul(t,q,v); mpz_mul(t,t,vm1); mpz_mmod(t,t,modulus);
            mpz_sub(t,modulus,t); mpz_mmod(t,t,modulus); 
            mpz_add(v_n,v_n,t);

            /* v_n += (modulus-((p*qek)%modulus))%modulus; */
            mpz_mul(t,p,qek); mpz_mmod(t,t,modulus); mpz_sub(t,modulus,t); 
            mpz_mmod(t,t,modulus); mpz_add(v_n,t,v_n);

            /* v_n %= modulus; */
            mpz_mmod(v_n,v_n,modulus);
        
            /* qek_n = q*qek*qek; */
            mpz_mul(t,qek,qek), mpz_mul(qek_n,t,q);

            /* qekm1_n = qek*qek; */
            mpz_set(qekm1_n,t);

            /* subscript -= testbit; */
            mpz_sub(subscript,subscript,testbit);
        }

        /* vm1 = vm1_n; */
        mpz_set(vm1,vm1_n);
        
        /* v = v_n; */
        mpz_set(v,v_n);

        /* qek = qek_n; */
        mpz_set(qek,qek_n);
        
        /* qekm1 = qekm1_n; */
        mpz_set(qekm1,qekm1_n);

        /* testbit /= 2; */
        mpz_div_2exp(testbit,testbit,1);
    }
    clear(vm1); clear(vm1_n); clear(v_n); clear(testbit); clear(qek); 
    clear(qekm1); clear(qek_n); clear(qekm1_n); clear(t);
}


/*
 * Lucas function. Calculates V_e(p, 1) modulus n
 */
void 
v_p1_n(MP_INT *v,MP_INT *subscriptc, MP_INT *p, MP_INT *modulus) {
    MP_INT *vm1, *vm1_next, *v_next, *testbit,*t,*v2,*subscript;
   
    /* if (subscript == 0)  return 2 */
    if (mpz_cmp_ui(subscriptc,0) == 0) {
        mpz_set_ui(v,2);
        return;
    }
    init(vm1);init(vm1_next);init(v_next);init(testbit);init(t);init(v2);
    init(subscript);

    mpz_set(subscript,subscriptc);

    mpz_mmod(p,p,modulus);                   /* p = p%modulus;   */
    mpz_set_ui(vm1,2);                       /* vm1 = 2          */
    mpz_set(v,p);                            /* v = p;           */

    mpz_set_ui(testbit,1);                   /* testbit = 1;     */

    /* while (testbit <= subscript) testbit*=2; */
    while(mpz_cmp(testbit,subscript) <= 0) 
        mpz_mul_2exp(testbit,testbit,1);

    mpz_div_2exp(testbit,testbit,1);         /* testbit /=2 */
    mpz_sub(subscript,subscript,testbit);    /* subscript-=testbit*/
    mpz_div_2exp(testbit,testbit,1);         /* testbit /=2 */

    /* while (testbit >= 1) { */
    while(mpz_cmp_ui(testbit,1) >= 0) {
        /* if (testbit > subscript) { */
        if (mpz_cmp(testbit,subscript) > 0) {

            /* vm1_next = ((v*vm1) - p)) % modulus; */
            mpz_mul(vm1_next,v,vm1); mpz_sub(vm1_next,vm1_next,p);
            mpz_mmod(vm1_next,vm1_next,modulus);

            /* v_next = ((v*v) -2) % modulus; */
            mpz_mul(v_next,v,v); mpz_sub_ui(v_next,v_next,2);
            mpz_mmod(v_next,v_next,modulus);
        } 
        else {
            
            /* vm1_next = v2  = (v*v)%modulus; */
            mpz_mul(v2,v,v); 
            mpz_mmod(v2,v2,modulus);
            mpz_set(vm1_next,v2);

            /* vm1_next -=2; */
            mpz_sub_ui(vm1_next,vm1_next,2);

            /* vm1_next %= modulus; */
            mpz_mmod(vm1_next,vm1_next,modulus);

            /* v_next = ((p*v2) - (v*vm1) - p) % modulus; */
            mpz_mul(v_next,v2,p); mpz_mul(t,v,vm1); 
            mpz_sub(v_next,v_next,t); mpz_sub(v_next,v_next,p);
            mpz_mmod(v_next,v_next,modulus);

            /* subscript -= testbit; */
            mpz_sub(subscript,subscript,testbit);
        }

        /* vm1 = vm1_next; v = v_next; */   
        mpz_set(vm1,vm1_next); mpz_set(v,v_next);

        /* testbit /= 2; */
        mpz_div_2exp(testbit,testbit,1);
#ifdef DEBUG
        fprintf(stderr, "v  ="); mpz_out_str(stderr,16,v); 
        fprintf(stderr, "\n");
        fprintf(stderr, "vm1="); mpz_out_str(stderr,16,vm1); 
        fprintf(stderr, "\n");
        fprintf(stderr, "tb ="); mpz_out_str(stderr,16,testbit); 
        fprintf(stderr, "\n");
        fprintf(stderr, "sub="); mpz_out_str(stderr,16,subscript); 
        fprintf(stderr, "\n\n");
#endif

    }
    clear(vm1);clear(vm1_next);clear(v_next);clear(testbit);clear(t);
    clear(v2);
}

void halfrsa(MP_INT *s, MP_INT *m, MP_INT *e, MP_INT *n, 
    MP_INT *p, MP_INT *q)
{
    MP_INT *p2, *q2, *d1, *d2, *m1, *m2, *u, *v, *t, *d;
    init(p2); init(q2); init(d1); init(d2); init(m1); init(m2); init(t);
    init(u); init(v); init(d);

    /* m = m % n */
    mpz_mmod(m,m,n);

    /* m1 = m mod p */
    mpz_mmod(m1,m,p);
    /* m2 = m mod q */
    mpz_mmod(m2,m,q);

    /* u=p-1, v=q-1 , t = (p-1)(q-1) */
    mpz_sub_ui(u,p,1); mpz_sub_ui(v,q,1); mpz_mul(t,u,v);

    /* d = (1/e) mod (p-1)(q-1) */
    modinv(d,e,t);

    /* d1 = d mod (p-1) */
    mpz_mmod(d1,d,u);
    /* d2 = d mod (q-1) */
    mpz_mmod(d2,d,v);

    /* p2 = m1 ^ d1 mod p */
    mpz_powm(p2,m1,d1,p);

    /* q2 = m2 ^ d2 mod q */
    mpz_powm(q2,m2,d2,q);
    
    /* if (p < q)  (p,q,p2,q2) = (q,p,q2,p2) */
    if (mpz_cmp(p,q) < 0) {
        mpz_set(t,q); mpz_set(q,p); mpz_set(p,t);
        mpz_set(t,q2); mpz_set(q2,p2); mpz_set(p2,t);
    }

    /* now use CRT to get the m in Z_n corresponding to (p2,q2) */
    
    /* return ((((p2 - q2) % p) * ((1/q)mod p)) % p) * q + q2; */
    mpz_sub(t,p2,q2); mpz_mmod(t,t,p); 
    modinv(u,q,p); /* should be precomputed on key generation */
    mpz_mul(t,u,t); mpz_mmod(t,t,p);
    mpz_mul(t,q,t); mpz_add(s,t,q2);

    clear(p2); clear(q2); clear(d1); clear(d2); clear(m1); clear(m2); 
    clear(t); clear(u); clear(v); clear(d);
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
void halfluc(MP_INT *s, MP_INT *m, MP_INT *e, MP_INT *n, 
    MP_INT *p, MP_INT *q)
{
    MP_INT *t,*pa,*qa,*q2,*p2,*di,*u,*d;
    int dip, diq;
    init(t); init(pa); init(qa); init(p2); init(q2); 
    init(di); init(d); init(u);

    /* m = m % n */
    mpz_mmod(m,m,n);

    /* di = (m*m)-4; */
    mpz_mul(di,m,m); mpz_sub_ui(di,di,4);

    dip = jacobi(di,p); diq = jacobi(di,q);

	/* pa = p - jacobi(di,p); qa = q - jacobi(di,q); */ 
	mpz_set_si(pa,dip); mpz_set_si(qa,diq);
	mpz_sub(pa,p,pa); mpz_sub(qa,q,qa);

	/* calculate lcm */
	lcm(t,pa,qa);

	/* d = (1/e) mod lcm(pa,qa) */
	modinv(d,e,t);

	/* p2 = v_p1_n(d % pa, m % p,p); */
	mpz_mmod(t,d,pa); mpz_mmod(u,m,p); v_p1_n(p2,t,u,p);

	/* q2 = v_p1_n(d % qa, m % q,q); */
	mpz_mmod(t,d,qa); mpz_mmod(u,m,q); v_p1_n(q2,t,u,q);

	/* chinese remainder theorem - pull back from Z_p x Z_q to Z_n
	 * the key is the observation that (p2,q2) are the projections
	 *  of the desired result (in Z_n) to Z_p x Z_q
	 * ain't explained worth a damn in the DDJ article (listing 3), 
	 * but pretty obvious if you now how RSA is done in practice
	 */

	/* First force p > q */

	/* if (p < q)  (p,q,p2,q2) = (q,p,q2,p2) */
	if (mpz_cmp(p,q) < 0) {
		mpz_set(t,q); mpz_set(q,p); mpz_set(p,t);
		mpz_set(t,q2); mpz_set(q2,p2); mpz_set(p2,t);
	}

	/* now use CRT to get the m in Z_n corresponding to (p2,q2) */
	
	/* return ((((p2 - q2) % p) * ((1/q)mod p)) % p) * q + q2; */
	mpz_sub(t,p2,q2); mpz_mmod(t,t,p); 
	modinv(u,q,p); /* should be precomputed on key generation */
	mpz_mul(t,u,t); mpz_mmod(t,t,p);
	mpz_mul(t,q,t); mpz_add(s,t,q2);

    clear(t); clear(pa); clear(qa); clear(p2); clear(q2); 
    clear(di); clear(d); clear(u);
}

/* key length is to 48 bytes (really we encrypt KEY+IV, 48=24+16+8) */
#define KEYLEN 48
#define HASHLEN 16

static int bytes(MP_INT *n)
{
    int i = 0;
    MP_INT *t;

    init(t);
    mpz_set(t,n);
    while (mpz_cmp_ui(t,0)) {
        i++;
        mpz_div_2exp(t,t,8);
    }
    clear(t);
    return i;
}
#ifndef STAND
void encrypt_session_key(MP_INT *c, unsigned char *m, MP_INT *n, MP_INT *e,
    int alg)
{
    int nn,i;
    unsigned char ribuf[MAXC];
    MP_INT *mm;

    init(mm);
#ifdef DEBUG
    fprintf(stderr, "encrypt_session_key: session key is:");
    for (i=0;i<KEYLEN;i++)
        fprintf(stderr, "%02x", *(m+i));
    fprintf(stderr, "\n");
#endif
    nn = bytes(n);
    ribuf[0] = 0;
    for (i=1; i <= (nn - KEYLEN)-1; i++)
        ribuf[i] = randombyte();
    memcpy(&ribuf[i], m, KEYLEN);
    cstmp(mm,ribuf,nn);

#ifdef DEBUG
    fprintf(stderr, "padded plaintext = ");
    mpz_out_str(stderr, 16, mm);
    fprintf(stderr, "\n");
#endif

    if (alg == LUC) 
        luc(c,mm,e,n);
    else if (alg == RSA)
        rsa(c,mm,e,n);
    else if (alg == RL) {
        MP_INT *c1;
        init(c1);
        luc(c1,mm,e,n);
        rsa(c,c1,e,n);
        clear(c1);
    }
    else
        ferr("encrypt_session_key: invalid paramaters\n");
    
    zap(ribuf,MAXC);
    clear(mm);
}

void decrypt_session_key(unsigned char *k, MP_INT *c, MP_INT *p, 
    MP_INT *q, MP_INT *e, int alg) {
    MP_INT *m;
    MP_INT *n;
    unsigned char ribuf[MAXC];

    init(m); init(n);
    mpz_mul(n,p,q);
#ifdef DEBUG
    fprintf(stderr, "ciphertext = ");
    mpz_out_str(stderr, 16, c);
    fprintf(stderr, "\n");
#endif
    if (alg == LUC)
        halfluc(m,c,e,n,p,q);
    else if (alg == RSA)
        halfrsa(m,c,e,n,p,q);
    else if (alg == RL) {
        MP_INT *c1;
        init(c1);
        halfrsa(c1,c,e,n,p,q);
        halfluc(m,c1,e,n,p,q);
        clear(c1);
    }
    else
        ferr("decrypt_session_key: invalid paramaters\n");
#ifdef DEBUG
    fprintf(stderr, "decrypted plaintext = ");
    mpz_out_str(stderr, 16, m);
    fprintf(stderr, "\n");
#endif
    mptcs(ribuf,MAXC,m);
    memcpy(k,&ribuf[MAXC-KEYLEN],KEYLEN);
    zap(ribuf,MAXC);
    clear(n);
}

void sign(MP_INT *c, unsigned char *hash, MP_INT *p, MP_INT *q, MP_INT *e, 
    int alg)
{
    int nn,i;
    unsigned char ribuf[MAXC];
    MP_INT *mm, *n;

    init(mm); init(n);
    mpz_mul(n,p,q);
    nn = bytes(n);
    ribuf[0] = 0;
    for (i=1; i <= (nn - HASHLEN)-1; i++)
        ribuf[i] = 0xff;
    memcpy(&ribuf[i], hash, HASHLEN);
    cstmp(mm,ribuf,nn);
    if (alg == LUC)
        halfluc(c,mm,e,n,p,q);
    else if (alg == RSA)
        halfrsa(c,mm,e,n,p,q);
    else if (alg == RL) {
        MP_INT *c1;
        init(c1);
        halfrsa(c1,mm,e,n,p,q);
        halfluc(c,c1,e,n,p,q);
        clear(c1);
    }
    else
        ferr("sign: invalid paramaters\n");

    zap(ribuf,MAXC);
    clear(mm);
}

int vsign(MP_INT *c, unsigned char *hash, MP_INT *n, MP_INT *e, int alg)
{
    int nn,i;
    MP_INT *m;
    unsigned char ribuf[MAXC];
    init(m);

    nn = bytes(n);
    if (alg == LUC)
        luc(m,c,e,n);
    else if (alg == RSA)
        rsa(m,c,e,n);
    else if (alg == RL) {
        MP_INT *c1;
        init(c1);
        luc(c1,c,e,n);
        rsa(m,c1,e,n);
        clear(c1);
    }
    else
        ferr("vsign: invalid paramaters\n");
    mptcs(ribuf,nn,m);
    for (i=0;i<16;i++)
        if (hash[i] != ribuf[nn - HASHLEN + i])
            return 0;
    for (i=1;i < nn-HASHLEN;i++)
        if (ribuf[i] != 0xff)
            return 0;
    if (ribuf[0] != 0)
        return 0;
    return 1;
    clear(m);
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
printf("%d ticks to %s\n",foo.tms_utime - lastc,s);
lastc = foo.tms_utime;
}

test(MP_INT *prime1,MP_INT *prime2, MP_INT *plain, MP_INT *public)
{
    static int number = 0;
    MP_INT *one,*modulus,*c1,*c2,*k1,*k2,*p1,*p2,*p3;
    MP_INT *s1,*s2,*s3,*pp1,*pp2,*pp3,*kk1;

    init(one);init(modulus);
    init(c1); init(c2); init(k1); init(k2); init(p1); 
    init(p2); init(p3); init(kk1);
    init(s1); init(s2); init(s3);
    init(pp1); init(pp2); init(pp3);
    number++;
    printf("TEST NUMBER %d\n",number);
    mpz_set_ui(one,1);
    mpz_mul(modulus,prime1,prime2);

    checkpoint("set up");
    luc(c1,plain, public, modulus);
    checkpoint("encrypt with v_p1_n");
    v_pq_n(c2,public,plain,one,modulus);
    checkpoint("encrypt with v_pq_n");
    if (mpz_cmp(c1,c2)) {
        printf("FAILED: c1 != c2");
        chars("c1",c1);
        chars("c2",c2);
        exit(1);
    }
    checkpoint("verify1");
    private_from_message(k1,c1,public,prime1,prime2);
    checkpoint("compute private from ciphertext");
    halfluc(p3,c1,public,modulus,prime1,prime2);
    checkpoint("decrypt with halfluc (should be fastest way to decrypt)");
    luc(p1,c1,k1,modulus);
    checkpoint("decrypt with private from ciphertext");
    private_from_primes(k2,public,prime1,prime2);
    checkpoint("compute private from primes");
    luc(p2,c1,k2,modulus);
    checkpoint(
     "decrypt with private from primes (should be slowest way to decrypt");
    /* now we must make sure that plain=p1=p2=p3 */
    if (mpz_cmp(plain,p1) || mpz_cmp(p1,p2) || mpz_cmp(p2,p3)) {
        printf("FAILED DECRYPTION TEST\n");
        chars("p1",p1);
        chars("p2",p2);
        chars("p3",p3);
        chars("plain",plain);
        exit(1);
    }
    checkpoint("verify2");
    private_from_message(kk1,plain,public,prime1,prime2);
    checkpoint("compute private from plaintext");
    halfluc(s3,plain,public,modulus,prime1,prime2);
    checkpoint("sign using halfluc (should be fastest way to sign)");
    luc(s1,plain,kk1,modulus);
    checkpoint("sign using private from plaintext");
    luc(s2,plain,k2,modulus);
    checkpoint(
        "sign using private from primes (should be slowest way to sign)");
    if (mpz_cmp(s1,s2) || mpz_cmp(s2,s3)) {
        printf("FAILED SIGNATURE GENERATION TEST\n");
        chars("s1",s1);
        chars("s2",s2);
        chars("s3",s3);
        exit(1);
    }
    checkpoint("verify3");
    luc(pp1,s1,public,modulus);
    checkpoint("verify signature");
    if (mpz_cmp(pp1,plain)) {
        printf("FAILED SIGNATURE VERIFICATION TEST\n"); 
        chars("pp1",pp1);
        exit(1);
    }
    chars("plaintext  ", plain);
    chars("ciphertext ", c1);
    chars("signature  ", s1);
    printf("PASSED TEST NUMBER %d\n",number);

    clear(s1); clear(s2); clear(s3);
    clear(pp1); clear(pp2); clear(pp3);
    clear(one);clear(modulus);
    clear(c1); clear(c2); clear(k1); clear(k2); clear(p1); 
    clear(p2); clear(p3); clear(kk1);
}
        

main() {
MP_INT *prime1,*prime2,*public,*plain,*n;
init(prime1); init(prime2); init(public); init(plain); init(n);
mpz_set_ui(prime1,1949);
mpz_set_ui(prime2,2089);
mpz_set_ui(public,1103);
mpz_set_ui(plain,11111);
test(prime1,prime2,plain,public);

mpz_set_ui(plain,1);
test(prime1,prime2,plain,public);
mpz_set_str(public,"65537",10);
mpz_set_str(prime1,"2229581051182913",10);
mpz_set_str(prime2,"387833844699347",10);
mpz_set_ui(plain,11111);
test(prime1,prime2,plain,public);
mpz_set_ui(plain,1);
test(prime1,prime2,plain,public);

/* expect the following to take a while. */
mpz_set_str(prime1,"12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071",10);
mpz_set_str(prime2,"10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381",10);
mpz_set_ui(public,65537);
mpz_mul(n,prime1,prime2);
mpz_random(plain,32); mpz_mmod(plain,plain,n);
test(prime1,prime2,plain,public);
}
#endif
#ifdef TEST2
main()
{
MP_INT *prime1,*prime2,*public,*plain,*n,*c,*plain2;
init(prime1); init(prime2); init(public); init(plain); init(n);
init(plain); init(plain2);init(c);
mpz_set_str(prime2,"12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071",10);
mpz_set_str(prime1,"10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381",10);
mpz_set_ui(public,65537);
mpz_mul(n,prime1,prime2);
while(1) {
mpz_random(plain,32); mpz_mmod(plain,plain,n);
luc(c,plain,public,n);
halfluc(plain2,c,public,n,prime1,prime2);
if (mpz_cmp(plain,plain2)) {
    printf("failed\n");
    chars("plain",plain); chars("c",c); chars("plain2",plain2);
    exit(1);
}
else {
    printf("passed\n");
}
}
}
#endif
#ifdef TEST3
char vb[2048],eb[2048],Pb[2048],Qb[2048];
#define disp {\
    mpz_get_str(vb,10,v); \
    mpz_get_str(eb,10,e); \
    mpz_get_str(Pb,10,P); \
    mpz_get_str(Qb,10,Q); \
    printf("V_%s(%s,%s) mod 2^68 = %s -- OK\n", eb,Pb,Qb,vb); }

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
    } 

int smallprimes[]={2,3,5,7,11,13,17,65537};
int nsmallprimes=8;


main()
{
    MP_INT *P; MP_INT *Q; MP_INT *v; MP_INT *e; MP_INT *w; MP_INT *n;
    MP_INT *x;
    int i,j,k;
    init(P); init(Q); init(v); init(e); init(w); init(n); init(x);

    mpz_set_ui(n,1);
    mpz_mul_2exp(n,n,68);

    for (i=0;i<64;i++) {
        /* w = 2^i + 1 */
        mpz_set_ui(w,1); mpz_mul_2exp(w,w,i); mpz_add_ui(w,w,1);

        /* v = V_i(3,2) */
        mpz_set_si(P,3); mpz_set_si(Q,2); mpz_set_ui(e,i);
        v_pq_n(v,e,P,Q,n);

        /* if v != w , we have a problem */
        if (mpz_cmp(v,w)) {
            printf("test failed\n");
            exit(1);
        }
        else {
            disp;
        }
    }
    
    for (i=0;i<nsmallprimes;i++) {
        for (j=0;i<nsmallprimes;i++) {
            if (i != j) {
                for (k=2;k<65;k++) {
                    int a=smallprimes[i], b=smallprimes[j];

                    /* P = a+b, Q=ab */
                    mpz_set_ui(P,a); mpz_add_ui(P,P,b);
                    mpz_set_ui(Q,a); mpz_mul_ui(Q,Q,b);

                    /* set w = (P^k + Q^k) mod n*/
                    mpz_set_ui(w,a); mpz_powm_ui(w,w,k,n);
                    mpz_set_ui(x,b); mpz_powm_ui(x,x,k,n);
                    mpz_add(w,w,x); mpz_mod(w,w,n);

                    mpz_set_ui(e,k);
                    v_pq_n(v,e,P,Q,n);
                    if (mpz_cmp(w,v)) {
                        printf("failed\n"); 
                        chars("v",v); 
                        chars("w",w); 
                        chars("P",P); 
                        chars("Q",Q); 
                        chars("e",e); 
                        exit(1); 
                    }
                    else {
                        disp;
                    }
                }
            }
        }
    }
                        
    
    /* check that V_38(1,-1) = 87403803 */
    test3(1,-1,38,"87403803");
    /* V_37(1,-1) = 54018521 */
    test3(1,-1,37,"54018521");

    /* some Companion Pell Numbers */
    test3(2,-1,17,"3215042");
    test3(2,-1,25,"3710155682");
    test3(2,-1,37,"145445522951122");
    test3(2,-1,38,"351136554095046");

    /* some for P=4 Q=3 */
    test3(4,3,33,"5559060566555524");
    test3(4,3,27,"7625597484988");
}
#endif
    
