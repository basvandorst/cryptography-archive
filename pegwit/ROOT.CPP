#include "vlong.hpp"
#include "field.hpp"

static void assert(int x){ if (!x) x = 1/x; }

class GFe_poly // polynomial with coefficients of type GFe
{
  unsigned max;
  unsigned n; // degree + 1
  GF * f;
  GFe * a; // coefficients
public:
  GFe_poly( GF * F, unsigned Max );
  GFe_poly( const GFe_poly & x );
  ~GFe_poly();
  void set( unsigned i, const GFe & x );
  GFe get( unsigned i ) const;
  GFe_poly & operator = ( const GFe_poly & x );
  friend GFe_poly operator +( const GFe_poly & x, const GFe_poly & y );
  friend GFe_poly operator -( const GFe_poly & x, const GFe_poly & y );
  friend GFe_poly operator *( const GFe_poly & x, const GFe_poly & y );
  friend GFe_poly operator /( const GFe_poly & x, const GFe_poly & y );
  friend GFe_poly operator %( const GFe_poly & x, const GFe_poly & y );
  friend unsigned deg( const GFe_poly & x );
  friend GFe_poly gcd( const GFe_poly &x, const GFe_poly &y );
};

void GFe_poly::set( unsigned i, const GFe & x )
{
  assert( i < max );
  a[i] = x;
  if ( x == f->zero )
    while ( n && a[n-1]==f->zero ) n -= 1;
  else if ( i >= n )
    n = i + 1;
}

GFe GFe_poly::get( unsigned i ) const
{
  if ( i < n ) return a[i];
  return f->zero;
}

GFe_poly::GFe_poly( GF * F, unsigned Max )
{
  n = 0;
  max = Max;
  f = F;
  a = new GFe[Max];
  for (unsigned i=0;i<Max;i+=1) a[i] = f->zero;
}

GFe_poly::GFe_poly( const GFe_poly & x )
{
  n = x.n;
  max = x.max;
  f = x.f;
  a = new GFe[max];
  for (unsigned i=0;i<max;i+=1) a[i] = x.a[i];
}

GFe_poly& GFe_poly::operator = ( const GFe_poly & x )
{
  n = 0;
  for (unsigned i=0;i<x.n;i+=1)
    set( i, x.get(i) );
  return *this;
}

GFe_poly::~GFe_poly()
{
  delete [] a;
}

GFe_poly operator +( const GFe_poly & x, const GFe_poly & y )
{
  unsigned n = x.n; if ( n < y.n ) n = y.n;
  GFe_poly z(x.f,n);
  for (unsigned i=0;i<n;i+=1)
    z.set( i, x.get(i) + y.get(i) );
  return z;
}

GFe_poly operator -( const GFe_poly & x, const GFe_poly & y )
{
  unsigned n = x.n; if ( n < y.n ) n = y.n;
  GFe_poly z(x.f,n);
  for (unsigned i=0;i<n;i+=1)
    z.set( i, x.get(i) - y.get(i) );
  return z;
}

GFe_poly operator *( const GFe_poly & x, const GFe_poly & y )
{
  GFe_poly z(x.f,x.n+y.n);
  for (unsigned i=0;i<x.n;i+=1)
    for (unsigned j=0;j<y.n;j+=1)
      z.set( i+j, z.get(i+j) + x.get(i) * y.get(j) );
  return z;
}

GFe_poly operator %( const GFe_poly & x, const GFe_poly & y )
{
  GFe_poly z(x);
  while ( z.n >= y.n )
  {
    GFe d = z.get(z.n-1) / y.get(y.n-1);
    unsigned i = z.n;
    unsigned j = y.n;
    while (i)
    {
      i -= 1;
      j -= 1;
      z.set( i, z.get(i) - d * y.get(j) );
    }
  }
  return z;
}

GFe_poly operator /( const GFe_poly & x, const GFe_poly & y )
{
  GFe_poly result( x.f, y.n-1 );
  GFe_poly z(x);
  while ( z.n >= y.n )
  {
    GFe d = z.get(z.n-1) / y.get(y.n-1);
    result.set( z.n-y.n, d );
    unsigned i = z.n;
    unsigned j = y.n;
    while (i)
    {
      i -= 1;
      j -= 1;
      z.set( i, z.get(i) - d * y.get(j) );
    }
  }
  return result;
}

unsigned deg( const GFe_poly & x )
{
  if ( x.n==0 ) return 0; else return x.n-1;
}

GFe_poly gcd( const GFe_poly &X, const GFe_poly &Y )
{
  GFe_poly a=X, b=Y;
  while ( b.n != 0 )
  {
    GFe_poly c = a % b;
    a = b;
    b = c;
  }
  GFe z = a.f->one / a.get( a.n - 1 );
  for (unsigned i=0;i<a.n;i+=1)
    a.set( i, a.get(i) * z );
  return a;
}

GFe find_root( GF2m & F, const vlong & f )
{
  GFe_poly g(&F,2*F.m);
  for (unsigned i=0;i<f.bits();i+=1)
    if ( f.bit(i) ) g.set( i, F.one );
  GFe_poly fp = g;
  GFe u(&F);
  while ( deg(g) > 1 )
  {
    step1:
    u.set_random( *F.rn );
    GFe_poly z(&F,2); z.set( 1, u );
    GFe_poly c(&F,2*F.m);
    c = z;
    for (unsigned i=1;i<F.m;i+=1)
      c = ( c*c + z ) % g;
    GFe_poly h = gcd( c, g );
    if ( deg(h)<1 || deg(g) == deg(h) ) goto step1;
    if ( 2*deg(h) > deg(g) )
      g = g/h;
    else
      g = h;
  }
  return g.get(0);
}

