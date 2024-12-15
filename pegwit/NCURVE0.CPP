// elliptic curve implementation (curve generation)

#include "vlong.hpp"
#include "ncurve.hpp"
#include "prime.hpp"

#define assert_on        1      // Consistency checking (not expensive)

extern "C" int printf(const char *_format, ...);

#if assert_on
  static void assert0(int x){ if (x==0) while (1/x); }
  #define assert(x) assert0(x)
#else
  #define assert(x)
#endif

int is_prime( unsigned p )
{
  for (unsigned d=2;d*d<=p;d+=1)
    if (p%d==0) return 0;
  return 1;
}

// Routines to perform operations on polynomials
// over F2 represented as bit strings (vlong)

vlong poly_div( const vlong & x, const vlong & y )
{
  unsigned m = y.bits()-1;
  vlong d = 0;
  vlong rem = x;
  while (1)
  {
    unsigned rb = rem.bits();
    if ( rb <= m ) break;
    rem ^= y << (rb-m-1);
    d ^= 1 << (rb-m-1);
  }
  return d;
}

vlong poly_rem( const vlong & x, const vlong & y )
{
  unsigned m = y.bits()-1;
  vlong rem = x;
  while (1)
  {
    unsigned rb = rem.bits();
    if ( rb <= m ) break;
    rem ^= y << (rb-m-1);
  }
  return rem;
}

vlong poly_mul( const vlong & x, const vlong & y )
{
  vlong result=0;
  unsigned yb = y.bits();
  for (unsigned i=0;i<yb;i+=1)
  {
    if (y.bit(i))
      result ^= x << i;
  }
  return result;
}

vlong poly_gcd( const vlong &x, const vlong &y ) // field is F2
{
  vlong a = x;
  vlong b = y;
  while (b != 0)
  {
    vlong c = poly_rem( a, b );
    a = b;
    b = c;
  }
  return a;
}

int poly_irreducible( const vlong & x )
{
  unsigned d = x.bits()-1;
  vlong u = 2;
  for (unsigned i=1;i<d;i+=1)
  {
    u = poly_rem( poly_mul(u,u), x );
    vlong g = poly_gcd( u^2, x );
    if ( g != 1 ) return 0;
  }
  return 1;
}

vlong poly_power( vlong x, vlong k, vlong g )
{
  vlong result = 1;
  for (unsigned i=0;i<k.bits();i+=1)
  {
    if ( k.bit(i) )
    {
      result = poly_rem( poly_mul( result, x ), g );
    }
    x = poly_rem( poly_mul( x, x ), g );
  }
  return result;
}

unsigned calc_root( unsigned L )
{
  // find irreducible poly g such that 2 is primitive
  vlong base = pow2(L);
  for (vlong g=base+1;g<2*base;g+=2) if ( poly_irreducible(g) )
  {
    vlong r = base-1;
    vlong f = 2;
    while (1)
    {
      if ( f != base-1 && r % f == 0 )
      {
        if ( poly_power( 2, (base-1) / f, g ) == 1 ) break;
        do r = r / f; while ( r % f == 0 );
      }
      if ( f >= r ) return to_unsigned(g-base);
      if ( f == 2 ) f = 3; else f += 2;
      if ( f*f > r ) f = r;
    }
  }
  return 0; // should not happen, at least for reasonable L
}

unsigned * small_field::curve_order()
{
  unsigned * result = new unsigned[ BASE ]; // result[0] not defined
  unsigned b,x,y,i;
  for (b=1;b<BASE;b+=1)
    result[b] = 2; /* 0,0 and 0, sqrt(b) */

  for (x=1;x<BASE;x+=1)
  {
    unsigned lx = log[x];
    unsigned x3 = alog[ (3*lx) % BASE_M1 ];
    result[ x3 ] += 2; // y = 0 and y = x
    for (y=1;y<x;y+=1)
    {
      i = log[y] + log[x^y];
      if ( i >= BASE_M1 ) i -= BASE_M1;
      b = x3 ^ alog[i];
      result[ b ] += 1;
    }
    for (y=x+1;y<BASE;y+=1)
    {
      i = log[y] + log[x^y];
      if ( i >= BASE_M1 ) i -= BASE_M1;
      b = x3 ^ alog[i];
      result[ b ] += 1;
    }
  }
  return result;
}

curve_factory::curve_factory( unsigned L ) : L(L)
{
  so_set = 0;
  if ( L > MAXL ) return;

  unsigned BASE = 1u<<L;
  unsigned SQRT_BASE = 1u<<((L+1)/2); // upper bound
  unsigned * ob;
  {
    root = calc_root(L);
    if ( root == 0 ) return;
    small_field f(L,root);
    so_min = BASE - 2*SQRT_BASE;
    so_max = BASE + 2*SQRT_BASE;
    ob = f.curve_order();
  }

  so_set = new unsigned[ so_max - so_min + 1 ];
  for ( unsigned so=so_min; so<=so_max; so+= 1) so_set[so-so_min] = 0;

  for ( unsigned b=1; b<BASE; b+=1 )
  {
    so = ob[ b ];
    assert( so >= so_min );
    assert( so <= so_max );
    assert( (so & 1) == 0 );
    if ( so_set[so-so_min] == 0 )
      so_set[so-so_min] = b;
  }
  delete [] ob;

  {
    prime_factory pf(1000);
    comp = 1;
    for ( unsigned i=0;i<pf.np;i+=1 )
      comp = comp * pf.pl[i];
  }
}

curve_factory::~curve_factory()
{
  delete [] so_set;
}

int curve_factory::find( unsigned K, curve_parameter & result )
{
  if ( !so_set || K < 3 || !is_prime(K) || L % K == 0 ) return 0;

  // calculate T
  unsigned T = 1;
  while (1)
  {
    if ( poly_irreducible( 1 + pow2(T) + pow2(K) ) )
      break;
    T += 1;
    if ( T > K/2 ) return 0;
  }

  vlong best_p = 1000;
  int found = 0;

  for ( unsigned so = so_min; so<= so_max; so += 1 )
  {
    unsigned M = L*K;
    unsigned b = so_set[so-so_min];
    if (b)
    {
      vlong Z = pow2(L);
      vlong p = pow2(M) + 1 - curve::small_lucas( Z-(so-1), Z, K );
      assert( p % so == 0 );
      p = p / so;
      vlong ntf = 1;
      vlong factor = comp;
      while (1)
      {
        factor = gcd(p,factor);
        // printf("L=%u K=%u so=%u factor=%d\n",L,K,so,to_unsigned(factor));
        if ( factor == 1 )
        {
          if ( is_probable_prime(p) && curve::MOV( 10, pow2(M), p ) )
          {
            result.L = L;
            result.K = K;
            result.T = T;
            result.root = root;
            result.b = b;
            result.nso = (so-so_min)/2;
            result.ntf = to_unsigned(ntf);
            found = 1;
            best_p = p;
            if ( result.ntf == 1 ) return 1;
          }
          break;
        }
        else
        {
          ntf = ntf * factor;
          if ( ntf > 0xffff) break; // limit on non-trivial factors
          p = p / factor;
          if ( p <= best_p ) break;
        }
      }
    }
  }
  return found;
}

