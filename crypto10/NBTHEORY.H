#ifndef NBTHEORY_H
#define NBTHEORY_H

#include "bignum.h"

// the following functions do not depend on the details of bignum implementation

// greatest common divisor
bignum Gcd(const bignum &a, const bignum &b);

// multiplicative inverse of a modulus m
bignum Inverse(const bignum &a, const bignum &m);

boolean IsSmallPrime(const bignum &p);

boolean SmallDivisorsTest(const bignum &p);

// use Fermat's Little Theorem with a = {first rounds number of primes} to test for primality
boolean FermatTest(const bignum &p, unsigned int rounds);

// one round of the Rabin-Miller primality test
boolean RabinMillerTest(RandomNumberGenerator &rng, const bignum &w, unsigned int rounds);

// small divisors test + Fermat test
// should be good enough for most practical purposes
// but feel free to change this to suit your fancy
inline boolean IsPrime(const bignum &p)
{
    return (IsSmallPrime(p) || (SmallDivisorsTest(p) && FermatTest(p, 2)));
}

// use a fast sieve to find the next prime after p
// returns TRUE iff successful
boolean NextPrime(bignum &p, const bignum &max, boolean blumInt=FALSE);

// exponentiation using the Chinese Remainder Theorem
bignum a_exp_b_mod_pq(const bignum &a, const bignum &ep, const bignum &eq,
                      const bignum &p, const bignum &q, const bignum &u);

class PrimeAndGenerator
{
public:
    // generate random prime of pbits (with maximal subprime) and primitive g
    PrimeAndGenerator(RandomNumberGenerator &rng, unsigned int pbits);
    // generate random prime of pbits (with subprime of qbits) and g of order q
    PrimeAndGenerator(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);

    const bignum& Prime() const {return p;}
    const bignum& SubPrime() const {return q;}
    const bignum& Generator() const {return g;}

private:
    bignum p, q, g;
};

#endif

