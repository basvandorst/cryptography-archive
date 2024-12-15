/* LUCAS encryption using gmp 1.3.1
 * this version is faster than my earlier C++ version 
 * Mark Henderson - markh@wimsey.bc.ca
 * Version 0.3
 * 0.21 included a lot of test code
 */

#include <gmp.h>
#include "lucas.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef NULL
#define NULL (void *)0
#endif

/* you must supply this routine for genkey */
extern unsigned char randombyte(void); 

#define LIMB_BITS (sizeof(unsigned long)*8)

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
 *  a,b>0  b odd
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
    mpz_set(a,ac); mpz_set(b,bc);

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

            /* vm1_next = (v*vm1)%modulus; */
            mpz_mul(t,v,vm1); mpz_mmod(vm1_next,t,modulus);

            /* vm1_next += (modulus-p)%modulus; */
            mpz_sub(t,modulus,p); mpz_mmod(t,t,modulus); 
            mpz_add(vm1_next,t,vm1_next);

            /* vm1_next %= modulus; */
            mpz_mmod(vm1_next,vm1_next,modulus);

            /* v_next = (v*v)%modulus; */
            mpz_mul(t,v,v); mpz_mmod(v_next,t,modulus);

            /* v_next += (modulus-2)%modulus; */
            mpz_sub_ui(t,modulus,2); mpz_mmod(t,t,modulus);
            mpz_add(v_next,v_next,t);

            /* v_next %= modulus; */
            mpz_mmod(v_next,v_next,modulus);
        } 
        else {
            
            /* vm1_next = (v*v)%modulus; */
            mpz_mul(v2,v,v); mpz_mmod(v2,v2,modulus);
            mpz_set(vm1_next,v2);

            /* vm1_next += (modulus-2)%modulus; */
            mpz_sub_ui(t,modulus,2); mpz_mmod(t,t,modulus);
            mpz_add(vm1_next,vm1_next,t);

            /* vm1_next %= modulus; */
            mpz_mmod(vm1_next,vm1_next,modulus);

            /* v_next = (p*((v*v)%modulus))%modulus; */
            mpz_mul(v_next,v2,p); mpz_mmod(v_next,v_next,modulus);
            
            /* v_next += (modulus-((v*vm1)%modulus))%modulus; */
            mpz_mul(t,v,vm1); mpz_mmod(t,t,modulus);
            mpz_sub(t,modulus,t); mpz_mmod(t,t,modulus);
            mpz_add(v_next,v_next,t);
            
            /* v_next += (modulus-p)%modulus; */
            mpz_sub(t,modulus,p); mpz_mmod(t,t,modulus);
            mpz_add(v_next,v_next,t);
            
            /* v_next %= modulus; */
            mpz_mmod(v_next,v_next,modulus);

            /* subscript -= testbit; */
            mpz_sub(subscript,subscript,testbit);
        }

        /* vm1 = vm1_next; v = v_next; */   
        mpz_set(vm1,vm1_next); mpz_set(v,v_next);

        /* testbit /= 2; */
        mpz_div_2exp(testbit,testbit,1);
    }
    clear(vm1);clear(vm1_next);clear(v_next);clear(testbit);clear(t);
    clear(v2);
}

/* no need to compute private key from message before calling halfluc
   we do it in here. Note the technique here is similar to that
   used in RSA to decrease time to decrypt/sign messages 

   note in a "real" implementation, we'd precompute the four possible
   private keys and q^{-1} mod p.
   [ Note: it turns out that this precomputation isn't worth the trouble.
          Performance improvement in halfluc is <5% ]
*/
void halfluc(MP_INT *s, MP_INT *m, MP_INT *e, MP_INT *n, 
    MP_INT *p, MP_INT *q)
{
    MP_INT *t,*pa,*qa,*q2,*p2,*di,*u,*d;
    init(t); init(pa); init(qa); init(p2); init(q2); 
    init(di); init(d); init(u);

    /* m = m % n */
    mpz_mmod(m,m,n);

    /* di = (m*m)-4; */
    mpz_mul(di,m,m); mpz_sub_ui(di,di,4);

    /* pa = p - jacobi(di,p); qa = q - jacobi(di,q); */ 
    mpz_set_si(pa,jacobi(di,p)); mpz_set_si(qa,jacobi(di,q));
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
     * ain't explained worth a damn in the DDJ article (listing 3), but 
     * pretty obvious if you now how RSA is done in practice
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

void encrypt_session_key(MP_INT *c, unsigned char *m, MP_INT *n, MP_INT *e)
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

    luc(c,mm,e,n);
	zap(ribuf,MAXC);
    clear(mm);
}

void decrypt_session_key(unsigned char *k, MP_INT *c, MP_INT *p, 
MP_INT *q, MP_INT *e) {
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
    halfluc(m,c,e,n,p,q);
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

void sign(MP_INT *c, unsigned char *hash, MP_INT *p, MP_INT *q, MP_INT *e)
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

    halfluc(c,mm,e,n,p,q);
	zap(ribuf,MAXC);
    clear(mm);
}

int vsign(MP_INT *c, unsigned char *hash, MP_INT *n, MP_INT *e)
{
    int nn,i;
    MP_INT *m;
    unsigned char ribuf[MAXC];
    init(m);

    nn = bytes(n);
    luc(m,c,e,n);
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
