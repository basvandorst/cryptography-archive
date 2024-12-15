// elliptic curve implementation

#include "vlong.hpp"
#include "ncurve.hpp"
#include "prime.hpp"

#define assert_on        0      // Consistency checking (for debugging)
#define use_field_op     0      // Use operators

#if assert_on
  static void assert0(int x){ if (x==0) while (1/x); }
  #define assert(x) assert0(x)
#else
  #define assert(x)
#endif

small_field::small_field( unsigned L, unsigned root )
: L(L),
  BASE( 1u<<L ),
  BASE_M1( (1u<<L)-1 ),
  log( new lunit[ 1u<<L ] ),
  alog( new lunit[ 1u<<L ] )
{
  assert( L <= sizeof(lunit)*8 );
  root |= BASE;
  unsigned j = 1;
  for (unsigned i = 0; i < BASE_M1; i+=1)
  {
    log[j] = i;
    alog[i] = j;
    j *= 2; // 2 is primitive element
    if (j & BASE) j ^= root;
  }
  log[0] = BASE_M1; // used by field::mul
}

small_field::~small_field()
{
  delete [] log;
  delete [] alog;
}

void field::add( const poly a, const poly b, poly c )
{
  if ( a[0] < b[0] )
    add(b,a,c);
  else
  {
    for (unsigned i=0;i<=b[0];i+=1)
      c[i] = a[i] ^ b[i];
    for (;i<=a[0];i+=1) c[i] = a[i];
    c[0] = a[0];
  }
  while ( c[0] && c[c[0]]==0 ) c[0] -= 1;
}

void field::copy ( const poly a, poly b )
{
  for (unsigned i=0;i<=a[0];i+=1)
    b[i] = a[i];
}

int field::equal( const poly a, const poly b )
{
  unsigned i = 0;
  while (1)
  {
    if (a[i] != b[i]) return 0;
    if ( i == a[0] ) return 1;
    i += 1;
  }
}

void field::set_random( poly a )
{
  a[0] = K;
  for (unsigned i=1;i<=K;i+=1)
    a[i] = rand(BASE);
  while ( a[0] && a[a[0]]==0 )
    a[0] -= 1;
}

void field::reduce( poly a )
{
  for (unsigned i=a[0];i>K;i-=1)
  {
    a[i-K] = a[i-K] ^ a[i];
    a[i+T-K] = a[i+T-K] ^ a[i];
    a[i] = 0;
  }
  if ( a[0] > K )
  {
    a[0] = K;
    while (a[0] && a[a[0]]==0) a[0] -= 1;
  }
}

field_element& field_element:: operator = ( const field_element & x )
{
  f = x.f;
  field::copy( x.v, v );
  return *this;
}

field_element::field_element()
{
  f=0;
  v[0]=0;
}

field_element::field_element( field * F )
{
  f=F;
  v[0]=0;
}

field_element::field_element( const field_element & x )
{
  f = x.f;
  field::copy( x.v, v );
}

int field_element::operator == ( const field_element & x ) const
{
  return field::equal( v, x.v );
}

int field_element::operator == ( unsigned x ) const
{
  return ( v[0] == 0 && x == 0 ) || ( v[1] == 1 && x == v[1] );
}

field_element field_element::operator + ( const field_element & x ) const
{
  field_element result(f);
  field::add( v, x.v, result.v );
  return result;
}

field_element field_element::operator * ( const field_element & x ) const
{
  field_element result(f);
  f->mul( v, x.v, result.v );
  f->reduce( result.v );
  return result;
}

field_element curve::sq( const field_element & x )
{
  field_element result(x.f);
  x.f->square( x.v, result.v );
  x.f->reduce( result.v );
  return result;
}

field_element field_element::operator / ( const field_element & x ) const
{
  field_element result(f);
  field::poly tmp;
  f->inverse( x.v, tmp );
  f->mul( v, tmp, result.v );
  f->reduce( result.v );
  return result;
}

unsigned curve::ybit( const field_element & x )
{
  if ( x.v[0]==0 ) return 0;
  return x.v[1] & 1;
}

int field::slow_trace( const poly a )
{
  field::poly t1;
  copy( a, t1 );
  for (unsigned i=1;i<M;i+=1)
  {
    field::poly t2;
    square( t1, t2 );
    reduce( t2 );
    add( t2, a, t1 );
  }
  return t1[0] != 0;
}

int field::trace( const poly a )
{
  unsigned min = a[0]; if ( min > tm[0] ) min = tm[0];
  unsigned w = 0;
  for (unsigned i=1;i<=min;i+=1)
    w ^= a[i] & tm[i];
  // compute parity of w
  unsigned result = 0;
  while (w)
  {
    result ^= w & 1;
    w >>= 1;
  }
  assert( result == slow_trace(a) );
  return result;
}

void field::quad_solve( const poly a, poly b )
{
  if (M % 2 == 1) // M is odd - compute half-trace
  {
    copy( a, b );
    for (unsigned i=0;i<M/2;i+=1)
    {
      field::poly t1,t2;
      square( b, t1 ); reduce( t1 );
      square( t1, t2 ); reduce( t2 );
      add( t2, a, b );
    }
  }
  else
  {
    field::poly d;
    b[0] = 0;
    copy( a, d );
    for (unsigned i=1;i<M;i+=1)
    {
      field::poly t1,t2;
      square( d, t1 ); reduce( t1 );
      mul( nzt, t1, t2 ); // N.B. make nzt the first parameter, since it is mostly zero
      square( b, t1 );
      add( t1, t2, b ); reduce( b );
      square( d, t1 ); reduce( t1 );
      add( t1, a, d );
    }
  }
}

unsigned field::rand( unsigned base )
{
  // A very bad prng - but should be good enough for initialisation
  // You can use whatever prng you like here
  // but it needs to be standard so that
  // the same point C.P is always generated
  // for a given root,CB
  prng += 1;
  return prng % base;
}

field::field( full_curve_parameter & a )
: small_field( a.L, a.root ), M(a.L*a.K), K(a.K), T(a.T)
{
  assert( K <= MAXK );
  prng = 0;
  if ( a.tm == 0 )
  {
    // Calculate trace mask
    poly x;
    for (unsigned i=1;i<=K;i+=1)
    {
      x[0] = i;
      unsigned m = 1;
      unsigned w = 0;
      for (unsigned j=0;j<L;j+=1)
      {
        x[i] = m;
        if ( slow_trace(x) )
          w ^= m;
        m *= 2;
      }
      x[i] = 0;
      tm[i] = w;
      if (w) tm[0] = i;
    }
    a.tm = pack( tm );
  }
  else
    unpack( a.tm, tm );

  if (M%2==0)
  {
    nzt[0] = 1;
    nzt[1] = 1;
    while ( (nzt[1] & tm[1]) == 0 )
      nzt[1] *= 2;
    assert( trace( nzt ) != 0 );
  }

#if assert_on // test basic field operations
  for (unsigned i=0;i<10;i+=1)
  {
    field_element a(this),b(this),c(this);
    set_random(a.v); set_random(b.v); set_random(c.v);
    assert( a+b == b+a );
    assert( a*b == b*a );
    assert( a*(b+c) == a*b + a*c );
    if ( b.v[0] )
      assert( a == b*(a/b) );
  }
#endif
}

void field::mul( const poly a, const poly b, poly c )
{
  assert( a[0] <= K );
  assert( b[0] <= K );

  if ( a[0]==0 || b[0]==0 )
  {
    c[0] = 0;
    return;
  }
  unsigned i,j;
  c[0] = a[0] + b[0] - 1;
  for (i=1;i<=c[0];i+=1)
    c[i] = 0;

  // pre-computation of log[b[j]] (to reduce table lookups)
  unsigned lb[MAXK+1];
  unsigned b0 = b[0];
  for (j=1;j<=b0;j+=1)
    lb[j] = log[b[j]];

  for (i=1;i<=a[0];i+=1)
  {
    unsigned lai = log[a[i]];
    if ( lai != BASE_M1 )
    {
      for (j=1;j<=b0;j+=1)
      {
        unsigned lbj = lb[j];
        if ( lbj != BASE_M1 )
        {
          unsigned x = lai + lbj;
          if ( x >= BASE_M1 ) x -= BASE_M1;
          c[i+j-1] ^= alog[x];
        }
      }
    }
  }
}

void field::square( const poly a, poly c )
{
  if ( a[0]==0 )
  {
    c[0] = 0;
    return;
  }
  unsigned i;
  c[0] = 2*a[0] - 1;
  for (i=1;;i+=1)
  {
    unsigned x = 0;
    if ( a[i] )
    {
      x = 2*log[a[i]];
      if ( x >= BASE_M1 ) x -= BASE_M1;
      x = alog[x];
    }
    c[2*i-1] = x;
    if (i==a[0]) break;
    c[2*i] = 0;
  }
}

void field::addmul( poly a, unsigned alpha, unsigned j, const poly b )
{
  unsigned la = log[alpha];
  for (unsigned i=1;i<=b[0];i+=1)
  {
    while ( a[0] < j+i ){ a[0] += 1; a[a[0]] = 0; }
    unsigned x = b[i];
    if (x)
    {
      x = log[x] + la;
      if ( x >= BASE_M1 ) x -= BASE_M1;
      a[i+j] ^= alog[x];
    }
  }
  while ( a[0] && a[a[0]]==0 ) a[0] -= 1;
}

void field::div( poly a, unsigned b )
{
  unsigned lb = log[b];
  for (unsigned i=1;i<=a[0];i+=1)
  {
    unsigned ix = a[i];
    if (ix)
    {
      ix = log[ix] + BASE_M1 - lb;
      if ( ix >= BASE - 1 ) ix -= BASE_M1;
      a[i] = alog[ix];
    }
  }
}

void field::inverse( const poly a, poly b )
{
  field::poly c,f,g;
  b[0] = 1; b[1] = 1;
  c[0] = 0;
  g[0] = K+1;
  copy( a, f );
  for (unsigned i=2;i<=K;i+=1)
    g[i] = 0;
  g[1] = 1; g[T+1] = 1; g[K+1] = 1;
  while (1)
  {
    if ( f[0]==1 )
    {
      div(b,f[1]);
      return;
    }
    if ( f[0]<g[0] ) // basically same code with b,c,f,g swapped
    {
      while (1)
      {
        unsigned j = g[0] - f[0];
        unsigned ix = log[g[g[0]]] - log[f[f[0]]] + BASE_M1;
        if ( ix >= BASE_M1 ) ix -= BASE_M1;
        unsigned alpha = alog[ix];
        addmul( g, alpha, j, f );
        addmul( c, alpha, j, b );
        if ( g[0]==1 )
        {
          div( c, g[1] );
          copy( c, b );
          return;
        }
        if ( g[0]<f[0] ) break;
      }
    }
    unsigned j = f[0] - g[0];
    unsigned ix = log[f[f[0]]] - log[g[g[0]]] + BASE_M1;
    if ( ix >= BASE_M1 ) ix -= BASE_M1;
    unsigned alpha = alog[ix];
    addmul( f, alpha, j, g );
    addmul( b, alpha, j, c );
  }
}

void field::sqrt( const poly a, poly b )
{
  // b = a^( 2^(M-2) );
  copy(a,b);
  unsigned i = M-2;
  while (i)
  {
    field::poly t1;
    square(b,t1);
    reduce(t1);
    copy(t1,b);
  }
}

vlong field::pack( const poly a )
{
  vlong result;
  unsigned i=a[0];
  while (i)
  {
    result = ( result << L ) + a[i];
    i -= 1;
  }
  return result;
}

void field::unpack( const vlong & X, poly a )
{
  vlong x = X;
  unsigned n = 0;
  while ( x != 0 )
  {
    n += 1;
    a[n] = to_unsigned( x & BASE_M1 );
    x >>= L;
  }
  a[0] = n;
}

point::point()
{
  c = 0;
}

point::point( curve * C ) : x(C), y(C)
{
  c = C;
}

point::point( const point & P )
{
  c = P.c;
  x = P.x;
  y = P.y;
}

point operator * ( const vlong & k, const point & P )
{
  point result( P.c );
  P.c->mul( P, k, result );
  return result;
}

point point::operator + ( const point &P ) const
{
  point result(P.c);
  c->add( *this, P, result );
  return result;
}

point point::operator - ( const point & P ) const
{
  point result(P.c);
  c->sub( *this, P, result );
  return result;
}

point & point::operator = ( const point & P )
{
  c = P.c;
  x = P.x;
  y = P.y;
  return *this;
}

vlong curve::to_vlong( const point & P )
{
  return P.c->field::pack( P.x.v );
}

vlong curve::pack( const point & P )
{
  vlong result = 0;
  if ( P.x.v[0] )
  {
    result = to_vlong(P);
    result = result * 2 + ybit( P.y/P.x );
  }
  else if ( P.y.v[0] )
    result = 1;
  else
    result = 0;
  return result;
}

point curve::unpack( const vlong & X )
{
  vlong x = X;
  unsigned yb = to_unsigned( x & 1 ); x >>= 1;
  point R( this );
  field::unpack( x, R.x.v );
  if ( R.x.v[0] || yb )
    calc_y( R, yb ); // will return 1 provided X is valid
  else
    R.y = R.x; // zero
  return R;
}

int curve::MOV( unsigned B, const vlong & q, const vlong & r )
{
  vlong x = 1;
  for (unsigned i=1;i<B;i+=1)
  {
    x = ( x*q ) % r;
    if ( x == 1 )
      return 0; // what a pity ...
  }
  return 1;
}

vlong curve::small_lucas( vlong P, vlong Z, unsigned ik )
{
  vlong V0 = 2;
  vlong V1 = P;
  while ( ik > 1 )
  {
    vlong V2 = P*V1 - Z*V0;
    V0 = V1;
    V1 = V2;
    ik -= 1;
  }
  return V1;
}

#if use_field_op

void curve::sub( const point & P, const point & Q, point & R )
{
  point t1 = Q;
  t1.y = t1.y + t1.x;
  add( P, t1, R );
}

void curve::add( const point & P, const point & Q, point & R )
{
  if ( P.x == 0 && P.y == 0 )
    R = Q;
  else if ( Q.x == 0 && Q.y == 0 )
    R = P;
  else
  {
    if ( P.x == Q.x )
    {
      if ( P.y == Q.y ) // points are equal
      {
        field_element LD = P.x + P.y/P.x;
        R.x = sq(LD) + LD /*+ a*/;
        R.y = sq(P.x) + LD*R.x + R.x;
      }
      else // must be inverse - result is zero
      {
        R.x.v[0] = 0;
        R.y.v[0] = 0;
      }
    }
    else // P,Q are distinct, non-zero, and P != -Q
    {
      field_element LD = ( P.y + Q.y ) / ( P.x + Q.x );
      R.x = sq(LD) + LD + P.x + Q.x /*+ a*/;
      R.y = LD*(P.x+R.x) + R.x + P.y;
    }
  }
}

#else

// optimised version - about 5% faster ?
// could still do better

void curve::sub( const point & P, const point & Q, point & R )
{
  point t1;
  t1.c = this;
  copy( Q.x.v, t1.x.v );
  field::add( Q.x.v, Q.y.v, t1.y.v );
  add( P, t1, R );
}

void curve::add( const point & P, const point & Q, point & R )
{
  if ( P.x.v[0] + P.y.v[0] == 0 )
    R = Q;
  else if ( Q.x.v[0] + Q.y.v[0] == 0 )
    R = P;
  else
  {
    field::poly LD,t1,t2,t3;
    field::add( P.x.v, Q.x.v, t1 );
    if ( t1[0] == 0 )
    {
      if ( equal( P.y.v, Q.y.v ) ) // points are equal
      {
        // field_element LD = P.x + P.y/P.x;
        inverse( P.x.v, t1 );
        field::mul( t1, P.y.v, t2 ); reduce( t2 );
        field::add( t2, P.x.v, LD );
        // R.x = sq(LD) + LD /*+ a*/;
        square( LD, t1 ); reduce( t1 );
        field::add( t1, LD, R.x.v );
        // R.y = sq(P.x) + LD*R.x + R.x;
        square( P.x.v, t1 );
        field::mul( LD, R.x.v, t2 );
        field::add( t1, t2, t3 );
        reduce( t3 );
        field::add( t3, R.x.v, R.y.v );
      }
      else // must be inverse - result is zero
      {
        R.x.v[0] = 0;
        R.y.v[0] = 0;
      }
    }
    else // P,Q are distinct, non-zero, and P != -Q
    {
      //field_element LD = ( P.y + Q.y ) / ( P.x + Q.x );
      inverse( t1, t2 );
      field::add( P.y.v, Q.y.v, t1 );
      field::mul( t1, t2, LD ); reduce(LD);
      //R.x = sq(LD) + LD + P.x + Q.x /*+ a*/;
      square( LD, t1 ); reduce( t1 );
      field::add( t1, LD, t2 );
      field::add( t2, P.x.v, t1 );
      field::add( t1, Q.x.v, R.x.v );
      //R.y = LD*(P.x+R.x) + R.x + P.y;
      field::add( P.x.v, R.x.v, t1 );
      field::mul( LD, t1, t2 ); reduce( t2 );
      field::add( t2, R.x.v, t1 );
      field::add( t1, P.y.v, R.y.v );
    }
  }
}

#endif

point curve::random_point()
{
  point R(this);
  do
    field::set_random(R.x.v);
  while ( calc_y(R) == 0 );
  return R;
}

int curve::calc_y( point & R, unsigned yb )
{
  if ( R.x.v[0] == 0 )
    sqrt( b.v, R.y.v );
  else
  {
    field_element alpha = sq(R.x) * ( R.x /*+ a*/ ) + b;
    R.y.v[0] = 0;
    if ( alpha.v[0] != 0 )
    {
      field_element t1(this);
      field_element beta = alpha/sq(R.x);
      if ( trace(beta.v) != 0 ) return 0;
      quad_solve(beta.v,t1.v);
      assert( t1*t1 + t1 == beta );
      if ( ybit(t1) != yb )
        t1.v[1] ^= 1;
      R.y = R.x * t1;
    }
  }
  assert( sq(R.y) + R.x*R.y == R.x*R.x*R.x + b );
  return 1;
}

void curve::mul( const point & P, const vlong & x, point & Q )
{
  vlong h = x*3;
  Q.x.v[0] = 0;
  Q.y.v[0] = 0;

  point R = P;

  unsigned r = h.bits()-1; // so h.bit(r)==1
  unsigned i=1;
  while (1)
  {
    int hi = h.bit(i);
    int ai = x.bit(i);
    if ( hi == 1 && ai == 0 )
      Q = Q + R;
    if ( hi == 0 && ai == 1 )
      Q = Q - R;
    if ( i == r ) break;
    i += 1;
    R = R+R;
  }
}

curve::curve( full_curve_parameter & a ) : field( a )
{
  b.f = this;
  b.v[0] = 1;
  b.v[1] = a.b;

  unsigned BASE = 1u<<a.L;
  unsigned SQRT_BASE = 1u<<((a.L+1)/2); // upper bound
  unsigned so_min = BASE - 2*SQRT_BASE;
  unsigned so = so_min + 2*a.nso;
  vlong sf = so * a.ntf;

  p = a.p0;
  if (p==0)
  {
    vlong Z = pow2(L);
    p = pow2(M) + 1 - small_lucas( Z-(so-1), Z, K );
    p = p / sf;
    a.p0 = p;
  }
  assert( is_probable_prime(p) && MOV( 10, pow2(M), p ) );

  if (a.P0==0)
  {
    prng = 0; // to make sure P is reproducible
    do P = sf * random_point();
      while ( P.x.v[0]+P.y.v[0] == 0 );
    a.P0 = pack(P);
  }
  else
    P = unpack(a.P0);

  #if assert_on
    point t2 = p*P;
    assert( t2.x.v[0] + t2.y.v[0] == 0 ); // fingers crossed!
  #endif
};

full_curve_parameter::full_curve_parameter( const curve_parameter & bp )
{
  L = bp.L;
  K = bp.K;
  T = bp.T;
  root = bp.root;
  b = bp.b;
  nso = bp.nso;
  ntf = bp.ntf;
  p0 = 0;
  P0 = 0;
  tm = 0;
}

vlong ec_crypt::make_private_key()
{
  vlong x;
  while ( x < p )
    x = (x << 15) + rand(1u<<15);
  x = 2 + x % (p-3);
  return x;
}

vlong ec_crypt::make_public_key( const vlong & private_key )
{
  return pack( private_key * P );
}

vlong ec_crypt::make_secret( const vlong & public_key, pair & message )
{
  point S = random_point();
  vlong sr = make_private_key();
  message.s = pack( sr * P );
  message.r = pack( sr * unpack(public_key) + S );
  return to_vlong(S);
}

vlong ec_crypt::decode_secret( const vlong & private_key, const pair & message )
{
  point D = unpack(message.r) - private_key * unpack(message.s);
  return to_vlong(D);
}

pair ec_crypt::nr_sign( const vlong & data, const vlong & private_key )
{
  pair sig;
  vlong k;
  do
  {
    k = make_private_key();
    vlong z = to_vlong( k*P );
    sig.r = ( z + data ) % p;
  } while ( sig.r==0 );
  sig.s = ( k - private_key * sig.r ) % p; if ( sig.s < 0 ) sig.s += p;
  return sig;
}

vlong ec_crypt::nr_verify( const pair & sig, const vlong & public_key )
{
  vlong bz = to_vlong( sig.s * P + sig.r * unpack(public_key) );
  vlong data = ( sig.r - bz ) % p; if ( data < 0 ) data += p;
  return data;
}

pair ec_crypt::dsa_sign( const vlong & data, const vlong & private_key )
{
  pair sig;
  vlong k;
  do
  {
    k = make_private_key();
    vlong z = to_vlong( k*P );
    sig.r = z % p;
  } while ( sig.r==0 );
  sig.s = (modinv(k,p) * ( data + (private_key * sig.r)%p )) % p;
  return sig;
}

int ec_crypt::dsa_verify( const vlong & e, const pair & sig, const vlong & public_key )
{
  if ( sig.r < 1 || sig.r >= p || sig.s < 1 || sig.s >= p )
    return 0;
  vlong c = modinv(sig.s,p);
  vlong u1 = ( e*c ) % p;
  vlong u2 = ( sig.r*c ) % p;
  vlong z = to_vlong( u1 * P + u2 * unpack(public_key) ) % p;
  return z == sig.r;
}

ec_crypt::ec_crypt( full_curve_parameter & a )
: curve( a ), bits( p.bits() )
{
}
