#include "curve.hpp"
#include "prime.hpp"

static void assert(int x){ if (x==0) x = 1/x; }

// point class

point::point( curve * C, GF * F ) : x(F),y(F),c(C),zero(0) {}

point::point( const point & P ) : x(P.x), y(P.y), c(P.c), zero(P.zero) {}

void point::save( vlstr & s )
{
  s.put( zero );
  if (!zero)
  {
    // point compression
    vlong comp = x.get_value() * 2 +  c->ybit(*this);
    s.put( comp );
  }
}

void point::load( vlstr & s )
{
  zero = to_unsigned( s.get() );
  if (!zero)
  {
    // point compression
    vlong comp = s.get();
    x.set_value( comp / 2 );
    c->expand( *this, comp.bit(0) );
  }
}

// point functions

int operator == ( const point &P, const point &Q )
{
  if ( P.zero || Q.zero ) return P.zero == Q.zero;
  return ( P.x == Q.x ) && ( P.y == Q.y );
}

int operator != ( const point &P, const point &Q )
{
  return !( P == Q );
}

point operator + ( const point &P, const point &Q )
{
  return P.c->add( P, Q );
}

point operator - ( const point &P, const point &Q )
{
  return P + (-Q);
}

point operator - ( const point &P )
{
  return P.c->inverse( P );
}

point operator * ( const vlong &k, const point &P ) // sum of k>0 copies of P
{
  vlong h = k*3;
  point Q = P.c->O;
  point R = P;

  unsigned r = h.bits()-1; // so h.bit(r)==1
  unsigned i=1;
  while (1)
  {
    int hi = h.bit(i);
    int ai = k.bit(i);
    if ( hi == 1 && ai == 0 ) Q = Q + R;
    if ( hi == 0 && ai == 1 ) Q = Q - R;
    if ( i == r ) break;
    i += 1;
    R = R + R;
  }
  return Q;
}

////////////////////////////////////////////////////
// curve implementation

curve::curve( GF * F )
: f(F),a(F),b(F),O(this,F),P(this,F)
{
  O.zero = 1;
  p = pow2(20); // arbitrary default value for p
}

void curve::load( vlstr & s )
{
  p = s.get();
  a.set_value( s.get() );
  b.set_value( s.get() );
  P.load( s );
}

void curve::save( vlstr & s )
{
  s.put( p );
  s.put( a.get_value() );
  s.put( b.get_value() );
  P.save( s );
}

curve::~curve()
{
}

////////////////////////////////////////////////////
// curve implementation (over GFp)

point curve_GFp::inverse( const point &P )
{
  point R(P);
  R.y = - R.y;
  return R;
}

point curve_GFp::add( const point &P, const point &Q )
{
  point R(P);
  if ( P.zero ) return Q;
  if ( Q.zero )
  {
    // nothing to do
  }
  else if ( P == -Q )
    R.zero = 1;
  else if ( P == Q )
  {
    GFe L = ( 3*(P.x^2) + a ) / ( P.y + P.y );
    R.x = (L^2) - 2*P.x;
    R.y = L * ( P.x - R.x ) - P.y;
  }
  else // distinct
  {
    GFe L = ( Q.y - P.y ) / ( Q.x - P.x );
    R.x = (L^2) - P.x - Q.x;
    R.y = L*(P.x-R.x) - P.y;
  }
  return R;
}

void curve_GFp::expand( point & R, int ybit )
{
  R.zero = 0;
  R.y = f1.zero;
  GFe alpha = R.x * ( (R.x^2) + a ) + b;
  if ( alpha != f1.zero )
  {
    if ( (alpha^((f1.p-1)/2)) != f1.one )
      R.zero = 1;
    else
    {
      R.y = f1.sqrt( alpha );
      if ( f1.ybit(R.y) != ybit )
        R.y = -R.y;
    }
  }
}

int curve_GFp::ybit( const point &P )
{
  return f1.ybit( P.y );
}

point curve_GFp::rp( rng & r )
{
  point R(this,f);
  retry: R.x.set_random(r);
  GFe alpha = R.x * ( (R.x^2) + a ) + b;
  if ( alpha != f->zero )
  {
    if ( (alpha^((f1.p-1)/2)) != f1.one ) goto retry;
    R.y = f1.sqrt( alpha );
  }
  return R;
}

curve_GFp::curve_GFp( const vlong & p )
: curve( &f1 ), f1( p )
{
}

curve_GFp::curve_GFp( vlstr & s )
: curve( &f1 ), f1( s.get() )
{
  curve::load( s );
}

void curve_GFp::save( vlstr & s )
{
  s.put( f1.p );
  curve::save( s );
}

random_curve_GFp::random_curve_GFp( const vlong & p, rng & r )
: curve_GFp(p)
{
  // We ought to find a,b,P such that P has large prime order
  // Doing this properly is quite complicated
  // For the moment just choose random a,b,P and hope for the best
  do
  {
    a.set_random(r);
    b.set_random(r);
  } while ( 4*(a^3) + 27*(b^2) == f->zero );
  P = rp( r );
}

////////////////////////////////////////////////////
// curve implementation (over GF2m)

static vlong small_lucas( vlong P, vlong Z, unsigned k )
{
  vlong V0 = 2;
  vlong V1 = P;
  while ( k > 1 )
  {
    vlong V2 = P*V1 - Z*V0;
    V0 = V1;
    V1 = V2;
    k -= 1;
  }
  return V1;
}

static vlong dup ( const vlong & x, unsigned m, unsigned l )
{
  vlong y = 0;
  for (unsigned i=0;i<m/l;i+=1)
    y = y*pow2(l) + x;
  return y;
}

point curve_GF2m::inverse( const point &P )
{
  point R(P);
  R.y = R.x + R.y;
  return R;
}

point curve_GF2m::add( const point &P, const point &Q )
{
  point R(P);
  if ( P.zero ) return Q;
  if ( Q.zero )
  {
    // nothing to do
  }
  else if ( P == -Q )
    R.zero = 1;
  else if ( P == Q )
  {
    GFe L = P.x + P.y/P.x;
    R.x = sq(L) + L + P.c->a;
    R.y = sq(P.x) + (L+P.c->f->one)*R.x;
  }
  else
  {
    GFe L = ( P.y + Q.y ) / ( P.x + Q.x );
    R.x = sq(L) + L + P.x + Q.x + P.c->a;
    R.y = L*(P.x+R.x) + R.x + P.y;
  }
  return R;
}

void curve_GF2m::expand( point &R, int ybit )
{
  R.zero = 0;
  R.y = f1->zero;
  if ( R.x == f1->zero )
    R.y = f1->sqrt(b);
  else
  {
    GFe alpha = sq(R.x) * ( R.x + a ) + b;
    if ( alpha != f->zero )
    {
      GFe beta = alpha/sq(R.x);
      if ( f1->trace(beta) != 0 )
        R.zero = 1;
      else
      {
        GFe z = f1->quad_solve(beta);
        if (ybit != f1->ybit(z))
          z = z + f1->one;
        R.y = R.x * z;
      }
    }
  }
}

int curve_GF2m::ybit( const point &R )
{
  if ( R.x == f1->zero ) return 0;
  return f1->ybit( R.y/R.x );
}

point curve_GF2m::rp( rng & r )
{
  point R(this,f);
  retry: R.x.set_random(r);
  if ( R.x == f->zero )
    R.y = f1->sqrt(b);
  else
  {
    GFe alpha = sq(R.x) * ( R.x + a ) + b;
    if ( alpha != f->zero )
    {
      GFe beta = alpha/sq(R.x);
      if ( f1->trace(beta) != 0 ) goto retry;
      R.y = R.x * f1->quad_solve(beta);
    }
  }
  return R;
}

vlong curve_GF2m::order()
{
  vlong result = 2; // count the point O and also (0,sqrt(b))
  GFe x(f1);
  for (vlong X=1;X<pow2(f1->m);X+=1)
  {
    x.set_value(X);
    GFe alpha = sq(x) * ( x + a ) + b;
    if ( alpha == f1->zero || f1->trace( alpha/sq(x) ) == 0 )
      result += 2;
  }
  return result;
}

GF2m * new_GF2m( unsigned m )
{
  if ( GF2m_opt_norm_basis::exists(m) )
    return new GF2m_opt_norm_basis(m);
  else
    return new GF2m_poly_basis(m);
}

curve_GF2m::curve_GF2m( unsigned M )
: curve( f1 = new_GF2m( M ) )
{
}

curve_GF2m::curve_GF2m( vlstr & s )
: curve( f1 = new_GF2m(to_unsigned(s.get())))
{
  curve::load( s );
}

void curve_GF2m::save( vlstr & s )
{
  s.put( f1->m );
  curve::save( s );
}

random_curve_GF2m::random_curve_GF2m( unsigned m, rng & r )
: curve_GF2m( m )
{
  a.set_random( r );
  do b.set_random(r); while ( b == f->zero );
  P = rp( r );
}

int MOV( unsigned B, const vlong & q, const vlong & r )
{
  vlong t = 1;
  for (unsigned i=1;i<B;i+=1)
  {
    t = ( t*q ) % r;
    if ( t == 1 )
      return 0; // what a pity ...
  }
  return 1;
}

class map
{
  public:
  GF2m * e, *f;
  GFe ge,gf;
  map( GF2m *E, GF2m *F );
  GFe to_f( GFe x );
  int check();
};

GFe map::to_f( GFe x )
{
  if ( x == e->zero ) return f->zero;
  unsigned log = 1;
  GFe p = ge;
  while ( p != x )
  {
    log += 1;
    p = p * ge;
  }
  return gf^log;
}

int map::check()
{
  GFe a = ge;
  GFe A = gf;
  for (vlong i=0;i<e->m;i+=1)
  {
    GFe b = ge;
    GFe B = gf;
    for (vlong j=0;j<e->m;j+=1)
    {
      if ( A + B != to_f(a+b) )
        return 0;
      if ( A * B != to_f(a*b) )
        return 0;
      b = b * ge;
      B = B * gf;
    }
    a = a * ge;
    A = A * gf;
  }
  return 1;
}

map::map( GF2m *E, GF2m *F )
: ge(E), gf(F)
{
  e = E;
  f = F;
  unsigned l = e->m;
  unsigned k = f->m/l;

  // Find generator for f
  for (vlong i=1;i<pow2(l);i+=1)
  {
    gf.set_value( dup(i,l*k,l) );
    GFe x = gf;
    vlong j = 1;
    while ( x != f->one )
    {
      x = x * gf;
      j += 1;
    }
    if ( j == pow2(l)-1 )
      break;
  }

  for (i=1;i<pow2(l);i+=1)
  {
    ge.set_value(i);
    GFe x = ge;
    vlong j = 1;
    while ( x != e->one )
    {
      x = x * ge;
      j += 1;
    }
    if ( j == pow2(l)-1 )
    {
      if ( check() )
        break;
    }
  }
}

weil_curve::weil_curve( rng & r, unsigned l, unsigned k, const vlong & Min_p )
: curve_GF2m(l*k)
{
  unsigned m = l*k;
  vlong min_p = pow2( (l-1)*k ); // default value
  if ( Min_p != 0 && Min_p < pow2(l*k-1) )
    min_p = Min_p; // Use supplied value if it is in range

  prime_factory pf(10000);
  while (1)
  {
    random_curve_GF2m E( l, r );
    vlong Z = pow2(l);
    p = pow2(m) + 1 - small_lucas( Z+1-E.order(), Z, k );
    vlong sf;
    if ( pf.make_prime( p, sf, min_p ) )
    {
      {
        map M( E.f1, f1 );
        a = M.to_f( E.a );
        b = M.to_f( E.b );
        assert( M.to_f(E.a+E.b) == a+b );
        assert( M.to_f(E.a*E.b) == a*b );

      }

      do { P = sf * rp( r ); } while ( P == O );
      assert( p * P == O ); // fingers crossed!

      // Need to check refs to get sensible value for B
      // 10 is just my wild guess
      if ( MOV( 10, pow2(m), p ) )
        break;
    }
  }
}


