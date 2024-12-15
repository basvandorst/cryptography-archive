// LUCAS public key algorithm in C++ (using gcc 2.3, gmp 1.3.1) (file: bignum.cc)
// Mark Henderson - markh@wimsey.bc.ca
// portions of this code are Copyright 1993 Raymond S. Brand, All Rights Reserved

// in particular the entire bignum class, halfluc routine and main()
//  are not covered by this copyright. Of course, use may be restricted 
//  by various patents :-(

extern "C" {
#include <gmp.h>
}
#ifdef TIMETEST
extern "C" {
#include <sys/types.h>
#include <sys/times.h>
}
#include <stdio.h>
#endif
#ifdef TEST
#include <stdio.h>
#endif
#define NN_DIGIT unsigned int
#ifndef NULL
#define NULL (void *)0
#endif
#define GCC

class bignum
{
    public:
        MP_INT mp;
        bignum() {mpz_init(&mp);}
        bignum(int i) {mpz_init_set_si(&mp,i);}
        bignum(long i)  {mpz_init_set_si(&mp,i);}
        bignum(unsigned int i)  {mpz_init_set_ui(&mp,i);}
        bignum(unsigned long i)  {mpz_init_set_ui(&mp,i);}
        bignum(char *s) {mpz_init_set_str(&mp,s,10);}
        bignum(bignum& b) {mpz_init_set(&mp,&(b.mp));}
        ~bignum() {mpz_clear(&mp);}
        unsigned int ui() {
            return mpz_get_ui(&mp);
        }
        int even() {
            return !(mpz_get_ui(&mp) %2);
        }
        int odd() {
            return (mpz_get_ui(&mp) %2);
        }

        bignum operator+(const bignum& p2) const {
            bignum result;
            mpz_add(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        void operator+=(const bignum &b) {
            mpz_add(&mp,&mp,&b.mp);
        }
        void operator+=(const unsigned int b) {
            mpz_add_ui(&mp,&mp,b);
        }
        void operator-=(const bignum &b) {
            mpz_sub(&mp,&mp,&b.mp);
        }
        void operator-=(const unsigned int &b) {
            mpz_sub_ui(&mp,&mp,b);
        }
        void operator*=(const bignum b) {
            mpz_mul(&mp,&mp,&b.mp);
        }
        void operator*=(const unsigned int b) {
            mpz_mul_ui(&mp,&mp,b);
        }
        void operator/=(const bignum b) {
            mpz_mdiv(&mp,&mp,&b.mp);
        }
        void operator/=(const unsigned int b) {
            mpz_mdiv_ui(&mp,&mp,b);
        }
        void operator%=(const bignum b) {
            mpz_mmod(&mp,&mp,&b.mp);
        }
        void operator%=(const unsigned int b) {
            mpz_mmod_ui(&mp,&mp,b);
        }
        bignum operator+(const unsigned int p) const {
            bignum result;
            mpz_add_ui(&(result.mp), &mp, (unsigned int)p);
            return result;
        }
        bignum operator-(const bignum& p2) const {
            bignum result;
            mpz_sub(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        bignum operator-(const unsigned int p) const {
            bignum result;
            mpz_sub_ui(&(result.mp), &mp, (unsigned int)p);
            return result;
        }
        bignum operator-() const {
            bignum result;
            mpz_neg(&result.mp,&mp);
            return result;
        }
        /* i | n is (1/i) mod n if (i,n)=1, else 0, always 0<=i<n  */
        bignum operator|(const bignum& n) const {
            bignum g,s;
            mpz_gcdext(&g.mp,&s.mp,NULL,&mp,&n.mp);
            if (g == 1) {
                mpz_mmod(&s.mp,&s.mp,&n.mp);
            }
            else {
                s = 0;
            }
            return s;
        }

        bignum operator*(const bignum& p2) const {
            bignum result;
            mpz_mul(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        bignum operator*(const unsigned int p) const {
            bignum result;
            mpz_mul_ui(&(result.mp), &mp, (unsigned int)p);
            return result;
        }
        bignum operator/(const bignum& p2) const {
            bignum result;
            mpz_mdiv(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        bignum operator^(const int p) const {
            bignum result;
            mpz_pow_ui(&(result.mp), &mp, (unsigned int)p);
            return result;
        }
        bignum operator^(const unsigned int p) const {
            bignum result;
            mpz_pow_ui(&(result.mp), &mp, p);
            return result;
        }
        bignum operator%(const bignum& p2) const {
            bignum result;
            mpz_mmod(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        bignum operator%(bignum& p2) {
            bignum result;
            mpz_mmod(&(result.mp), &mp, &(p2.mp));
            return result;
        }
        bignum operator%(const unsigned int p2) const {
            bignum result;
            mpz_mmod_ui(&(result.mp), &mp, p2);
            return result;
        }
        int operator>(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) > 0);
        }
        int operator<(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) < 0);
        }
        int operator<=(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) <= 0);
        }
        int operator>=(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) >= 0);
        }
        int operator==(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) == 0);
        }
        int operator!=(const unsigned int p2) const {
            return (mpz_cmp_ui(&mp,p2) != 0);
        }
        int operator>(const int p2) const {
            return (mpz_cmp_si(&mp,p2) > 0);
        }
        int operator<(const int p2) const {
            return (mpz_cmp_si(&mp,p2) < 0);
        }
        int operator<=(const int p2) const {
            return (mpz_cmp_si(&mp,p2) <= 0);
        }
        int operator>=(const int p2) const {
            return (mpz_cmp_si(&mp,p2) >= 0);
        }
        int operator==(const int p2) const {
            return (mpz_cmp_si(&mp,p2) == 0);
        }
        int operator!=(const int p2) const {
            return (mpz_cmp_si(&mp,p2) != 0);
        }
        int operator>(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) > 0);
        }
        int operator<(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) < 0);
        }
        int operator<=(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) <= 0);
        }
        int operator>=(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) >= 0);
        }
        int operator==(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) == 0);
        }
        int operator!=(const bignum &p2) const {
            return (mpz_cmp(&mp,&p2.mp) != 0);
        }
        operator MP_INT *() {
            return(&mp);
        }
        operator MP_INT *() const {
            return(&mp);
        }
        operator char *() const {
            return mpz_get_str(NULL,10,&mp);
        }
        operator char *() {
            return mpz_get_str(NULL,10,&mp);
        }
        operator unsigned char *() {
            return (unsigned char *)mpz_get_str(NULL,10,&mp);
        }
};

bignum powm(bignum& base, bignum& exp, bignum& mod) {
    bignum result;
    mpz_powm(&result.mp, &base.mp, &exp.mp, &mod.mp);
    return result;
}
    

bignum powm(bignum& base, unsigned int exp, bignum& mod) {
    bignum result;
    mpz_powm_ui(&result.mp, &base.mp, exp, &mod.mp);
    return result;
}

bignum mul_2exp(const bignum &a,unsigned int p) {
    bignum result;
    mpz_mul_2exp(result,a,p);
    return result;
}

bignum Assign2Exp(unsigned int n)
{
    bignum result = 1;
    mpz_mul_2exp(&result.mp,&result.mp,n);
    return result;
}

bignum mod_2exp(const bignum &a,int p) {
    bignum result;
    mpz_mod_2exp(result,a,p);
    return result;
}

bignum div_2exp(const bignum &a,int p) {
    bignum result;
    mpz_div_2exp(result,a,p);
    return result;
}

bignum gcd(const bignum& a,const bignum& b) {
    bignum result;
    mpz_gcd(&result.mp, &a.mp, &b.mp);
    return result;
}

bignum lcm(const bignum& a,const bignum& b) {
    bignum result;
    result = (a*(b/gcd(a,b)));
    return result;
}

bignum abs(const bignum& a) {
    bignum result;
    mpz_abs(&result.mp, &a.mp);
    return result;
}


/*
 *    Recall that a is a quadratic residue mod n if
 *    x^2 = a mod b      has an integer solution x.
 *
 *    J(a,b) = if a==1 then 1 else
 *             if a is even then J(a/2,b) * ((-1)^(b^2-1)/8))
 *             else J(b mod a, a) * ((-1)^((a-1)*(b-1)/4)))
 *
 *
 */
// on entry b must be odd 
int jacobi(bignum a, bignum b) 
{
    int sgn = 1;
    unsigned long c;
    bignum t; 

    while (a > 1) {
        if (a.even()) {

            // c = b % 8
            c = b.ui() % 8;  

            // b odd, then (b^2-1)/8 is even iff (b%8 == 3,5)

            // if b % 8 == 3 or 5 
            if (c == 3 || c == 5)
                sgn = -sgn;

            // a = a / 2
            a=div_2exp(a,1); 

        } 
        else {
            // note: (a-1)(b-1)/4 odd iff a mod 4==b mod 4==3

            // if a mod 4 == 3 and b mod 4 == 3
            if (((a.ui() % 4) == 3) && ((b.ui() % 4) == 3))
                sgn = -sgn;

            // set (a,b) = (b mod a,a)
            t=a; a=b%t; b=t;
        } 
    }
    if (a==0)
        sgn=0;
    return (sgn);
}

/*
 * Used in calculating private key from public key, message, and primes.
 */
bignum s(bignum m, bignum p, bignum q) {
    bignum d;

    d = (m*m)-4;
    return (lcm(p-(bignum)jacobi(d, p), q-(bignum)jacobi(d, q)));
}


/*
 * Used in calculating private key from pub key and primes.
 */
bignum r(bignum p, bignum q) {
    return (lcm(p-1, lcm(p+1, lcm(q-1, q+1))));
}


bignum private_from_message(bignum message, bignum pub, bignum p, bignum q) 
{
    return (pub | s(message,p,q));
}


/*
 * Calculates LUC private key from pub key, primes. Can be used with any
 *  message but will be VERY big (slow).
 */
bignum private_from_primes(bignum pub, bignum p, bignum q) {
    return (pub | r(p, q));
}

/*
 * Lucas function. Calculates V sub e (p, q) mod n
 */
bignum v_pq_n(bignum subscript, bignum p, bignum q, bignum modulus) {
    bignum vm1, v, vm1_n, v_n, testbit, qek, qekm1, qek_n, qekm1_n;

    if (subscript == 0) 
        return ((bignum)2);

    p = p % modulus;
    vm1 = 2;
    v = p;
    qek = q;
    qekm1 = 1;

    testbit = 1;
    while (testbit <= subscript) {
        // testbit *= 2;
        testbit = mul_2exp(testbit,1);
    }

    // testbit /= 2;
    testbit = div_2exp(testbit,1);
    subscript -= testbit;

    // testbit /= 2;
    testbit = div_2exp(testbit,1);

    while (testbit >= 1) {
        if (testbit > subscript) {
            vm1_n = (v*vm1)%modulus;
            vm1_n += (modulus-((p*qekm1)%modulus))%modulus;
            vm1_n = vm1_n % modulus;
            v_n = (v*v)%modulus;
            v_n += (modulus-((qek+qek)%modulus))%modulus;
            v_n %= modulus;
            qek_n = qek*qek;
            qekm1_n = qek*qekm1;
        } 
        else {
            vm1_n = (v*v)%modulus;
            vm1_n += (modulus-((qek+qek)%modulus))%modulus;
            vm1_n %= modulus;
            v_n = (p*((v*v)%modulus))%modulus;
            v_n += (modulus-((q*v*vm1)%modulus))%modulus;
            v_n += (modulus-((p*qek)%modulus))%modulus;
            v_n %= modulus;
            qek_n = q*qek*qek;
            qekm1_n = qek*qek;

            subscript -= testbit;
        }

        vm1 = vm1_n;
        v = v_n;
        qek = qek_n;
        qekm1 = qekm1_n;

        // testbit /= 2;
        testbit = div_2exp(testbit,1);
    }

    return (v);
}


/*
 * Lucas function. Calculates V sub e (p, 1) modulus n
 */
bignum v_p1_n(bignum subscript, bignum p, bignum modulus) {
    bignum vm1, v, vm1_next, v_next, testbit;

    if (subscript == 0) {
        return (2);
    }

    p = p%modulus;
    vm1 = 2;
    v = p;

    testbit = 1;
    while (testbit <= subscript) {

        // testbit *= 2;
        testbit = mul_2exp(testbit,1);
    }
    // testbit /= 2;
    testbit = div_2exp(testbit,1);

    subscript -= testbit;

    // testbit /= 2;
    testbit = div_2exp(testbit,1);

    while (testbit >= 1) {
        if (testbit > subscript) {
            vm1_next = (v*vm1)%modulus;
            vm1_next += (modulus-p)%modulus;
            vm1_next %= modulus;
            v_next = (v*v)%modulus;
            v_next += (modulus-2)%modulus;
            v_next %= modulus;
        } 
        else {
            vm1_next = (v*v)%modulus;
            vm1_next += (modulus-2)%modulus;
            vm1_next %= modulus;
            v_next = (p*((v*v)%modulus))%modulus;
            v_next += (modulus-((v*vm1)%modulus))%modulus;
            v_next += (modulus-p)%modulus;
            v_next %= modulus;

            subscript -= testbit;
        }

        vm1 = vm1_next;
        v = v_next;

        // testbit /= 2;
        testbit = div_2exp(testbit,1);
    }

    return (v);
}


bignum luc(bignum message, bignum key, bignum modulus) {
    return (v_p1_n(key, message, modulus));
}

// another algorithm (see Listing 1 in DDJ)
bignum v_p1_n_2(bignum e, bignum p, bignum n) {
bignum d = p*p - 4, ut = 1, vt = p, u = ut, v = vt, c;
    if (e.even()) {
        u=0;v=2;
    }

    // e = e / 2
    e = div_2exp(e,1);

    while (e > 0) {
        ut = (ut*vt) % n; 
        vt = (vt*vt) % n;
        if (vt < 3)
            vt += n;
        vt -= 2;
        if  (e.odd()) {
            c = (ut*v + u*vt) % n;
            v = (vt*v + d*u*ut) % n;
            if (v.odd())  
                v += n; 
            v = div_2exp(v,1); 
            
            if (c.odd()) 
                c+=n; 
            u = div_2exp(c,1);
       }

       // e = e / 2;
       e = div_2exp(e,1);
    }
    return v;
} 

bignum luc2(bignum message, bignum key, bignum modulus) {
    return (v_p1_n_2(key, message, modulus));
}

// no need to compute private key from message before calling this.
// we do it in here
bignum halfluc(bignum m, bignum e, bignum n, bignum p, bignum q)
{
    bignum t,pa,qa,q2,p2,s,di,d;
    int jp,jq;
    di = (m*m)-4; 
    jp = jacobi(di,p); jq=jacobi(di,q);
    pa = p-(bignum)jp; qa = q-(bignum)jq;
    d = e | lcm(pa,qa);

    p2 = v_p1_n(d % pa, m % p,p);
    q2 = v_p1_n(d % qa, m % q,q);

    // chinese remainder theorem - pull back from Z_p x Z_q to Z_n
    // the key is the observation that (p2,q2) are the projections
    //  of the desired result (in Z_n) to Z_p x Z_q
    // ain't explained worth a damn in the DDJ article (listing 3), but 
    // pretty obvious if you now how RSA is done in practice

    if (p < q) {
        t = q; q = p; p = t;
        t = q2; q2 = p2; p2 = t;
    }
    // we've forced p >= q  
    return ((((p2 - q2) % p) * (q|p)) % p) * q + q2;
}


#ifdef TEST
main() {
bignum prime1, prime2, modulus, publc, plain, sig,private1,private2,cipher;
bignum a,b,prime3,sig2,xx,ww;

printf("\n\nHALFLUC TEST\n");
prime1 = 1949;
prime2 = 2089;
modulus = prime1*prime2;
publc = 1103;
plain = 11111;
cipher = luc(plain, publc, modulus);
xx = halfluc(cipher, publc, modulus, prime1, prime2);
printf("cipher = %s\n",(char *)cipher);
printf("plain = %s\n",(char *)plain);
printf("xx = %s\n",(char *)xx);
if (xx != plain) 
    printf("HALFLUC test failed\n");

printf("\n\nTEST 0\n");
prime1 = 1949;
prime2 = 2089;
modulus = prime1*prime2;
prime3 = 11113;
publc = 65537;
a=v_p1_n(publc, prime3, modulus);
b=v_p1_n_2(publc, prime3, modulus);
printf("a=%s\n", (char *)a);
printf("b=%s\n", (char *)b);
if (a != b) {
printf("failed");
}

printf("\n\nTEST 1\n");
prime1 = 1949;
prime2 = 2089;
modulus = prime1*prime2;
publc = 1103;
plain = 11111;
cipher = luc(plain, publc, modulus);
private1 = private_from_message(cipher, publc, prime1, prime2);
private2 = private_from_primes(publc, prime1, prime2);
sig = luc(plain, private1, modulus);


printf ( "Prime1     = %s\n", (char *)prime1);
printf ( "Prime2     = %s\n", (char *)prime2);
printf ( "Modulus    = %s\n", (char *)modulus);
printf ( "PublicKey  = %s\n", (char *)publc);
printf ( "PlainText  = %s\n", (char *)plain);
printf ( "CipherText = %s\n", (char *)cipher);
printf ( "SignedText = %s\n", (char *)sig);
printf ( " MESSAGE ENCRYPTION\n");
printf ( "  LUC(PlainText, PublicKey, Modulus)       = %s\n", (char *)
    luc(plain,publc,modulus));
printf ( "  PrivateKey calculated with CipherText    = %s\n", (char *) 
    private_from_message(cipher, publc, prime1, prime2));
printf ( "  LUC(CipherText, PrivateKey, Modulus)     = %s\n", (char *)
    luc(cipher, private1, modulus));

printf ( "  PrivateKey calculated without CipherText = %s\n", (char *)
    private_from_primes(publc, prime1, prime2));
printf ( "  LUC(CipherText, PrivateKey, Modulus)     = %s\n", (char *)
    luc(cipher, private2, modulus));

printf("Message Signing\n");
printf ( "  PrivateKey calculated with PlainText     = %s\n", (char *) 
    private_from_message(plain, publc, prime1, prime2));

printf ( "  LUC(PlainText, PrivateKey, Modulus)      = %s\n", (char *)
    luc(plain, private1, modulus));

printf ( "  PrivateKey calculated without PlainText  = %s\n", (char *)
    private_from_primes(publc, prime1, prime2));
printf ( "  LUC(PlainText, PrivateKey, Modulus)      = %s\n", (char *)
    luc(plain, private2, modulus));
printf ( "  LUC(SignedText, PublicKey, Modulus)      = %s\n", (char *)
    luc(sig, publc, modulus));

printf("\n\nTEST 2\n");

prime1="12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071";
prime2="10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381";
// test primality to make sure this is an OK test case
if (!(mpz_probab_prime_p(&prime1.mp,25)))
    printf( "TEST ABORTED, prime1 is not prime\n");
if (!(mpz_probab_prime_p(&prime2.mp,25)))
    printf( "TEST ABORTED, prime2 is not prime\n");
modulus = prime1*prime2;
publc = 65537;
mpz_random(&plain.mp,20);
cipher = luc(plain, publc, modulus);
private1 = private_from_message(cipher, publc, prime1, prime2);
private2 = private_from_primes(publc, prime1, prime2);
sig = luc(plain, private1, modulus);

printf ( "Prime1     = %s\n", (char *)prime1);
printf ( "Prime2     = %s\n", (char *)prime2);
printf ( "Modulus    = %s\n", (char *)modulus);
printf ( "PublicKey  = %s\n", (char *)publc);
printf ( "PlainText  = %s\n", (char *)plain);
printf ( "CipherText = %s\n", (char *)cipher);
printf ( "SignedText = %s\n", (char *)sig);
printf ( " MESSAGE ENCRYPTION\n");
printf ( "  LUC(PlainText, PublicKey, Modulus)       = %s\n", (char *)
    luc(plain,publc,modulus));
printf ( "  PrivateKey calculated with CipherText    = %s\n", (char *) 
    private_from_message(cipher, publc, prime1, prime2));
printf ( "  LUC(CipherText, PrivateKey, Modulus)     = %s\n", (char *)
    luc(cipher, private1, modulus));

printf ( "  PrivateKey calculated without CipherText = %s\n", (char *)
    private_from_primes(publc, prime1, prime2));
printf ( "  LUC(CipherText, PrivateKey, Modulus)     = %s\n", (char *)
    luc(cipher, private2, modulus));
printf ( "  same computation using HALFLUC           = %s\n", 
    (char *) halfluc(cipher, publc, modulus,prime1,prime2));

printf("Message Signing\n");
printf ( "  PrivateKey calculated with PlainText     = %s\n", (char *) 
    private_from_message(plain, publc, prime1, prime2));

printf ( "  LUC(PlainText, PrivateKey, Modulus)      = %s\n", (char *)
    luc(plain, private1, modulus));

printf ( "  PrivateKey calculated without PlainText  = %s\n", (char *)
    private_from_primes(publc, prime1, prime2));
printf ( "  LUC(PlainText, PrivateKey, Modulus)      = %s\n", (char *)
    luc(plain, private2, modulus));
printf ( "  same computation using HALFLUC           = %s\n", 
    (char *) halfluc(plain, publc, modulus,prime1,prime2));
printf ( "  LUC(SignedText, PublicKey, Modulus)      = %s\n", (char *)
    luc(sig, publc, modulus));
}
#endif
#ifdef TIMETEST
void checkpoint(char *s)
{
struct tms foo;
static time_t lastc = 0;
times(&foo);
printf("%s:%d\n",s,foo.tms_utime - lastc);
lastc = foo.tms_utime;
}
    
main() {
bignum prime1, prime2, modulus, publc, plain, sig,private1,private2,cipher,xx,yy;
bignum zz,ww;
printf("\n\nTEST 2\n");

prime1="12922244505866957444614411074149878850813162782555916990165279745481230764642284945473557923346255859169679558092546245275716942381703676448951672729262071";
prime2="10574927133503540793209256833101417634035690799704584951086330929700469640717251976719240114456906872310713500218283726863253627915091407446716493633283381";
modulus = prime1*prime2;
publc = 65537;
mpz_random(&plain.mp,20);
checkpoint("setup");
cipher = luc(plain, publc, modulus);
checkpoint("encryption");
private1 = private_from_message(cipher, publc, prime1, prime2);
checkpoint("private key calculation");
yy = luc(cipher, private1, modulus);
checkpoint("decryption-1");
ww = halfluc(cipher, publc, modulus, prime1, prime2);
checkpoint("decryption-H");
zz = luc2(cipher, private1, modulus);
checkpoint("decryption-2");
sig = luc(plain, private1, modulus);
checkpoint("sign time");
xx = luc(sig,publc,modulus);
checkpoint("verify sig");
printf("plaintext = %s\n", (char *)plain);
printf("decrypts  = %s\n", (char *)yy);
printf("decrypts-H= %s\n", (char *)ww);
printf("sigv      = %s\n", (char *)xx);
}
#endif
