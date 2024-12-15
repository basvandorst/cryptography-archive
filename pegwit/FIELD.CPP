#include "vlong.hpp"
#include "field.hpp"

static void assert( int x ){ while (!x) x = x/x; }

GFe::GFe( GF * F ) : f(F), v(0) {}

GFe::GFe() : f(0), v(0) {}

GFe::GFe( GF * F, const vlong & V ) : f(F), v(V) {}

GFe& GFe::operator = ( const GFe & x )
{ v = x.v; f = x.f; return *this; }

int GFe::operator == ( const GFe & x ) const
{ return v == x.v; }

int GFe::operator != ( const GFe & x ) const
{ return v != x.v; }

GFe GFe::operator + ( const GFe & x ) const
{ GFe r(f,f->add( v, x.v )); return r; }

GFe GFe::operator - ( const GFe & x ) const
{ GFe r(f,f->sub( v, x.v )); return r; }

GFe GFe::operator - () const
{ GFe r(f,f->sub( 0, v )); return r; }

GFe GFe::operator * ( const GFe & x ) const
{ GFe r(f,f->mul( v, x.v )); return r; }

GFe GFe::operator / ( const GFe & x ) const
{ GFe r(f,f->div( v, x.v )); return r; }

GFe GFe::operator ^ ( const vlong & n ) const
{ GFe r(f,f->exp( v, n )); return r; }

GFe sq( const GFe & x )
{ GFe r(x.f,x.f->square(x.v)); return r; }

void GFe::set_random( rng & r )
{ v = f->normalise( r.next() ); }

void GFe::set_value( const vlong & x )
{ v = f->normalise(x); }

vlong GFe::get_value() const
{ return v; }

GFe operator * ( const vlong &n, const GFe & x )
{
  GFe result = x-x;
  GFe t = x;
  unsigned nb = n.bits();
  for (unsigned i=0;i<nb;i+=1)
  {
    if (n.bit(i))
      result = result + t;
    t = t + t;
  }
  return result;
}

GF::GF() : one(this),zero(this) {}

vlong GF::exp( const vlong & x, const vlong & n )
{
  vlong result = one.v;
  vlong t = x;
  unsigned nb = n.bits();
  for (unsigned i=0;i<nb;i+=1)
  {
    if (n.bit(i))
      result = mul(result,t);
    t = mul(t,t);
  }
  return result;
}

vlong GF::square( const vlong &x )
{ return mul(x,x); }

/////////////////////////////////////////////
// GFp ( field with prime number of elements)

vlong GFp::add( const vlong & x, const vlong & y )
{
  vlong result = x+y;
  if ( result >= p ) result -= p;
  return result;
}

vlong GFp::sub( const vlong & x, const vlong & y )
{
  vlong result = x-y;
  if ( result < 0 ) result += p;
  return result;
}

vlong GFp::mul( const vlong & x, const vlong & y )
{
  return ( x * y ) % p;
}

vlong GFp::div( const vlong & x, const vlong & y )
{
  return ( x * modinv(y,p)) % p;
}

vlong GFp::normalise( const vlong & x )
{
  vlong result = x % p;
  if ( result < 0 ) result += p;
  return result;
}

vlong GFp::exp( const vlong & x, const vlong & n )
{
  return modexp( x, n, p );
}

GFe GFp::sqrt( const GFe &g )
{
  GFe result(this);
  result.v = ::sqrt( g.v, p );
  return result;
}

int GFp::ybit( const GFe & x )
{
  return x.v.bit(0);
}

GFp::GFp( vlong P ) : p(P)
{
  one.v = 1;
}

//////////////////////////////////////////////////////////////
// Implementation of GF2m

//////////////////////////////////////////////////////////////
// Low-level rotate stuff
// Uses trick to only actually make BPU copies
// by making double length copies and then
// indexing into them

#define BPU (8*sizeof(unsigned))

void GF2m_opt_norm_basis::rota::load( const vlong & x )
{
  vlong y = x ^ ( x << m );
  unsigned i = 0;
  while (1)
  {
    y.store( a[i], mdu );
    i += 1;
    if ( i==BPU || i==m ) break;
    y.ror( m*2 );
  }
}

GF2m_opt_norm_basis::rota::rota(unsigned M)
{
  typedef unsigned * uptr;
  m = M;
  mdu = (2*m+BPU-1)/BPU;
  a = new uptr[m];
  for (unsigned i=0;i<m;i+=1)
  {
    if (i<BPU)
      a[i] = new unsigned[mdu];
    else
      a[i] = a[i%BPU] + i/BPU;
  }
}

// Routines to perform operations on polynomials
// over F2 represented as bit strings (vlong)

// These are required to for the polynomial basis
// and also to construct optimal normal basis

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


/////////////////////////////////////////////
// GF2m ( field with 2^m elements )

vlong GF2m::normalise( const vlong & x )
{
  return x & (pow2(m)-1);
}

vlong GF2m::add( const vlong & x, const vlong & y )
{
  return x^y;
}

vlong GF2m::sub( const vlong & x, const vlong & y )
{
  return x^y;
}


GF2m::GF2m( unsigned M ) : m(M)
{
  one.v = pow2(m)-1;
}

GF2m::~GF2m()
{
}

/////////////////////////////////////////////
// Optimal normal basis

static vlong typeIpoly( unsigned m )
{
  return pow2(m+1)-1;
}

static vlong typeIIpoly( unsigned m )
{
  vlong f = 1;
  vlong p = 1;
  while (m)
  {
    vlong f1 = p ^ (f<<1); p = f; f = f1;
    m -= 1;
  };
  return f;
}

int is_prime( unsigned p )
{
  for (unsigned d=2;d*d<=p;d+=1)
    if (p%d==0) return 0;
  return 1;
}

int check_order( unsigned g, unsigned k, unsigned p )
{
  if ( modexp(g,k,p) != 1 ) return 0;
  unsigned r = k;
  for (unsigned i=2;i<=r;i+=1)
  {
    if ( r%i==0 )
    {
      if ( modexp(g,k/i,p) == 1 )
        return 0;
      do { r /= i; } while ( r%i == 0 );
    }
  }
  return 1;
}

int GF2m_opt_norm_basis::exists( unsigned m )
{
  return exists(m,0) || exists(m,1);
}

int GF2m_opt_norm_basis::exists( unsigned m, int typeI )
{
  unsigned p = m+1;
  if ( typeI )
  {
    if ( m%8 != 2 && m%8 != 4 ) return 0;
  }
  else
  {
    if ( m%4 == 0 ) return 0;
    p = 2*m+1;
    if ( m%4 != 3 ) m = p-1;
  }
  return is_prime(p) && check_order( 2, m, p );
}

void GF2m_opt_norm_basis::mul( rota & rx, unsigned xr, rota & ry, vlong & result )
{
  // comment : either rx==ry or xr == 0
  // could split these cases out
  // xr is amount of pre-rotation for rx

  unsigned i,j,*a,*b,*c;

  // vlong d = x & ror(y,lambda[0][0]);
  a = rx.a[xr]; b = ry.a[ lambda[0][0] ];
  j = mu; do { j-=1; d[j] = a[j] & b[j]; } while (j);

  i = m-1;
  do
  {
    // d ^= ror(x+xr,m,i) & ( ror(y,m,lambda[0][i]) ^ ror(y,m,lambda[1][i]) );
    {
      unsigned ixr = i+xr;
      if (ixr >= m) ixr-=m; // this step could be eliminated if rx.a were double size
      a = rx.a[ixr];
    }
    b = ry.a[lambda[0][i]];
    c = ry.a[lambda[1][i]];
    j = mu; do { j-=1; d[j] ^= a[j] & ( b[j] ^ c[j] ); } while (j);
    i -= 1;
  } while (i);

  d[mu-1] &= mask; // remove junk caused by rota mechanism
  result.load( d, mu ); // convert d back to vlong
}

vlong GF2m_opt_norm_basis::mul( const vlong & x, const vlong & y )
{
  vlong result;
  r0.load( x );
  r1.load( y );
  mul( r0, 0, r1, result );
  return result;
}

vlong GF2m_opt_norm_basis::div( const vlong & x, const vlong & y )
{
  // vlong w = pow( y, ::pow(2,m)-2 );
  vlong w = y;
  r1.load(w);
  unsigned r = 0; while ( (1u<<r) < m ) r += 1; r -= 1;
  while (r)
  {
    r -= 1;
    unsigned z = (m-1) >> r;
    r0.load(w);
    mul( r0, m-z/2, r0, w );
    if ( z%2 )
    {
      w.rol( m );
      r0.load(w);
      mul( r0, 0, r1, w );
    }
  }
  w.rol( m );
  return mul( x, w );
}

vlong GF2m_opt_norm_basis::square( const vlong &x )
{
  vlong result = x;
  result.rol( m );
  return result;
}

GFe GF2m_opt_norm_basis::sqrt( const GFe &x )
{
  GFe result(x);
  result.v.ror( m );
  return result;
}

int GF2m_opt_norm_basis::trace( const GFe &x )
{
  int result = 0;
  for (unsigned i=0;i<m;i+=1)
    result ^= x.v.bit(i);
  return result;
}

GFe GF2m_opt_norm_basis::quad_solve( const GFe &x )
{
  // It may be necessary to optimise this
  // since it is used when computing size of small elliptic curves
  vlong t;
  int w = 0;
  for (unsigned i=1;i<m;i+=1)
  {
    w = w ^ x.v.bit(i);
    if (w) t.setbit(i);
  }
  GFe result(this,t);
  return result;
}

int GF2m_opt_norm_basis::ybit( const GFe & x )
{
  return x.v.bit(m-1); // could be any bit, but this is standard
}

static void invert( vlong * A, unsigned N )
{
  vlong * W = new vlong[N];
  unsigned i,j,t;
  i = N; while (i)
  {
    i -= 1;
    W[i] = A[N-1-i] << N;
    W[i].setbit(i);
    A[N-1-i] = 0;
  }
  i = N; while (i)
  {
    i -= 1;
    if (!W[i].bit(i+N))
    {
       t = i - 1;
       while (W[t].bit(i+N) == 0) { if(!t) assert(0); t-=1; }
       vlong tmp = W[i];
       W[i] = W[t];
       W[t] = tmp;
    }
    t = N; while (t)
    {
      t -= 1;
      if ( i != t )
      {
        if ( W[t].bit(i+N) )
        {
          W[t] = W[t] ^ ( W[i] & (pow2(N+i)-1) );
          W[t].clearbit(i+N);
        }
      }
    }
  }
  i = N; while (i)
  {
    i -= 1;
    j = N; while (j)
    {
      j -= 1;
      if ( W[j].bit(i) )
        A[N-1-i].setbit(j);
    }
  }
  delete [] W;
}

vlong * GF2m_opt_norm_basis::matrix( GF2m & to )
{
  GFe gamma = find_root( to, g );
  vlong * M = new vlong[ m ];
  for (unsigned i=0;i<m;i+=1)
  {
    M[i] = gamma.get_value();
    gamma = sq(gamma);
  }
  return M;
}

GF2m_opt_norm_basis::GF2m_opt_norm_basis( unsigned M )
: GF2m( M ), r0(M), r1(M)
{
  int typeI = 0;
  if (!exists(M,0))
  {
    typeI = 1;
    assert( exists(M,1) );
  }
  { // construct lamda
    if (typeI) g = typeIpoly(m); else g = typeIIpoly(m);
    unsigned i,j,k;
    vlong * A = new vlong[m];
    vlong * V = new vlong[m];
    vlong t = 2;
    for (i=0;i<m;i+=1)
    {
      A[i] = t;
      V[i] = poly_rem( poly_mul(t,2), g );
      t = poly_rem( poly_mul(t,t), g );
    }
    invert(A,m);
    lambda[0] = new unsigned[m];
    lambda[1] = new unsigned[m];
    lambda[1][0] = -1; // not used
    for (i=0;i<m;i+=1)
    {
      k = 0;
      j = 0;
      while (1)
      {
        if ( product( V[(m+j-i)%m], A[(m-i)%m] ) )
        {
          lambda[k][i] = j;
          k += 1;
          if (k == 2||i==0) break;
        }
        j += 1;
      }
    }
    delete [] V;
    delete [] A;
  }

  { // initialise rotate stuff
    mu = (m+BPU-1)/BPU;
    unsigned unused = mu*BPU - m;
    mask = -1; mask >>= unused;
    d = new unsigned[mu];
  }
}

GF2m_opt_norm_basis::~GF2m_opt_norm_basis()
{
  delete [] d;
  delete [] lambda[0];
  delete [] lambda[1];
}

///////////////////////////////////////////////////////////////////////
// Polynomial basis
//
// This is rather slow at present and not well optimised :
// poly_rem and poly_mul need optimising.
// Maybe compute pre-shifted copies (similar to rota class).
// Then need to make inner loop of poly_rem fast.

// However, maybe a normal basis should be used
// (even if it is "non-optimal")

vlong GF2m_poly_basis::mul( const vlong & x, const vlong & y )
{
  return poly_rem( poly_mul(x,y), g );
}

vlong GF2m_poly_basis::square( const vlong & x )
{
  return mul(x,x); // could do more efficient version ?
}

vlong GF2m_poly_basis::div( const vlong & x, const vlong & y ) // y != 0
{
  // Extended GCD
  vlong r0 = g;
  vlong r1 = y;
  vlong s0 = 0;
  vlong s1 = x;
  while (1)
  {
    vlong q = poly_div( r0, r1 );
    vlong r2 = r0 ^ poly_mul( q, r1 );
    if (r2==0) break;
    r0 = r1;
    r1 = r2;
    vlong s2 = s0 ^ poly_mul( q, s1 );
    s0 = s1;
    s1 = s2;
  }
  return poly_rem( s1, g );
}

int GF2m_poly_basis::trace( const GFe &x )
{
  GFe T = x;
  for (unsigned i=1;i<m;i+=1)
    T = sq(T) + x;
  return T.v != 0;
}

GFe GF2m_poly_basis::quad_solve( const GFe &x )
{
  unsigned i;
  if (m&1)
  {
    GFe T = x;
    for (i=0;i<(m-1)/2;i+=1)
      T = sq(sq(T))+x;
    return T;
  }
  else
  {
    GFe c = zero;
    GFe d = x;
    for (i=1;i<m;i+=1)
    {
      c = sq(c) + sq(d)*t;
      d = sq(d) + x;
    }
    return c;
  }
}

int GF2m_poly_basis::ybit( const GFe & x )
{
  return x.v.bit(0); // has to be bit 0 (the only with which changes when we add 1)
}

GFe GF2m_poly_basis::sqrt( const GFe &x )
{
  return x^( 2^(m-2) );
}

vlong * GF2m_poly_basis::matrix( GF2m & to )
{
  GFe gamma = find_root( to, g );
  vlong * M = new vlong[ m ];
  GFe y = to.one;
  for (unsigned i=0;i<m;i+=1)
  {
    M[i] = y.get_value();
    y = y * gamma;
  }
  return M;
}

GF2m_poly_basis::GF2m_poly_basis( unsigned M ) : GF2m(M), t(this)
{
  one.v = 1;
  g = pow2(m);
  while ( !poly_irreducible(g) )
    g += 1;

  if (m%2==0) // m is even
  {
    t.v = pow2(m-1);
    while ( trace(t) == 0 )
    {
      t.v = t.v / 2;
      if ( t.v==0 )
      {
        while ( trace(t)==0 )
          t.v += 1;
        break;
      }
    }
  }
}

GFe convert( const GFe & X, GF2m & to, vlong * M )
{
  vlong x = X.get_value();
  vlong y = 0;
  for (unsigned i=0;i<x.bits();i+=1)
    if ( x.bit(i) )
      y = y ^ M[i];
  GFe result(&to);
  result.set_value(y);
  return result;
}

vlong * inverse( vlong * A, unsigned N )
{
  vlong * W = new vlong[N];
  unsigned i,j,t;
  i = N; while (i)
  {
    i -= 1;
    W[i] = A[N-1-i] << N;
    W[i].setbit(i);
    A[N-1-i] = 0;
  }
  i = N; while (i)
  {
    i -= 1;
    if (!W[i].bit(i+N))
    {
       t = i - 1;
       while (W[t].bit(i+N) == 0) { if(!t) assert(0); t-=1; }
       vlong tmp = W[i];
       W[i] = W[t];
       W[t] = tmp;
    }
    t = N; while (t)
    {
      t -= 1;
      if ( i != t )
      {
        if ( W[t].bit(i+N) )
        {
          W[t] = W[t] ^ ( W[i] & (pow2(N+i)-1) );
          W[t].clearbit(i+N);
        }
      }
    }
  }
  vlong * R = new vlong[N];
  i = N; while (i)
  {
    i -= 1;
    j = N; while (j)
    {
      j -= 1;
      if ( W[j].bit(i) )
        R[j].setbit(N-i-1);
    }
  }
  delete [] W;
  return R;
}
