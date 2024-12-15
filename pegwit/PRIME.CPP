// prime factory implementation

#include "vlong.hpp"
#include "prime.hpp"

int is_probable_prime( const vlong &p )
{
  // Test based on Fermats theorem a**(p-1) = 1 mod p for prime p
  // For 1000 bit numbers this can take quite a while
  const rep = 4;
  const unsigned any[rep] = { 2,3,5,7 /*,11,13,17,19,23,29,31,37..*/ };
  for ( unsigned i=0; i<rep; i+=1 )
    if ( modexp( any[i], p-1, p ) != 1 )
      return 0;
  return 1;
}

prime_factory::prime_factory( unsigned MP )
{
  np = 0;

  // Initialise pl
  char * b = new char[MP+1]; // one extra to stop search
  for (unsigned i=0;i<=MP;i+=1) b[i] = 1;
  unsigned p = 2;
  while (1)
  {
    // skip composites
    while ( b[p] == 0 ) p += 1;
    if ( p == MP ) break;
    np += 1;
    // cross off multiples
    unsigned c = p*2;
    while ( c < MP )
    {
      b[c] = 0;
      c += p;
    }
    p += 1;
  }
  pl = new unsigned[np];
  np = 0; for (p=2;p<MP;p+=1) if ( b[p] ) { pl[np] = p; np += 1; }
  delete [] b;
}

prime_factory::~prime_factory()
{
  delete [] pl;
}

vlong prime_factory::find_prime( vlong & start )
{
  unsigned SS = 1000; // should be enough unless we are unlucky
  char * b = new char[SS]; // bitset of candidate primes
  unsigned tested = 0;
  while (1)
  {
    unsigned i;
    for (i=0;i<SS;i+=1)
      b[i] = 1;
    for (i=0;i<np;i+=1)
    {
      unsigned p = pl[i];
      unsigned r = to_unsigned(start % p); // not as fast as it should be - could do with special routine
      if (r) r = p - r;
      // cross off multiples of p
      while ( r < SS )
      {
        b[r] = 0;
        r += p;
      }
    }
    // now test candidates
    for (i=0;i<SS;i+=1)
    {
      if ( b[i] )
      {
        tested += 1;
        if ( is_probable_prime(start) )
          return start;
      }
      start += 1;
    }
  }
  delete [] b;
}

int prime_factory::make_prime( vlong & r, vlong &k, const vlong & min_p )
// Divide out small factors or r
{
  k = 1;
  for (unsigned i=0;i<np;i+=1)
  {
     unsigned p = pl[i];
     // maybe pre-computing product of several primes
     // and then GCD(r,p) would be faster ?
     while ( r % p == 0 )
     {
       if ( r == p )
         return 1; // can only happen if min_p is small
       r = r / p;
       k = k * p;
       if ( r < min_p )
         return 0;
     }
  }
  return is_probable_prime( r );
}

