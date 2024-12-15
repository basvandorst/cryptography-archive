#include "curve.hpp"
#include "prime.hpp"

extern "C" {
int           rand(void);
#define random(num)    (rand() % (num))
int printf(const char *_format, ...);
}

#ifdef noio
int printf(const char *_format, ...) { return 0; }
#endif


void check(int x)
{
  if (x==0)
    x = 1/x; // force abort ( put debug break here )
}

class bad_rng : public rng
// This needs to be replaced by a good proper (not pseudo) random
// number generator - i.e. one which derives entropy from keystroke
// timing or some other private external random source.
{
  unsigned seed;
public:
  virtual vlong next()
  {
    vlong result = random(10000);
    while ( result < pow2(128) )
    {
      result = result * 256 + rand();
    }
    return result;
  }
};

class string : public vlstr
// very simple implementation of vlstr
{
public:
  char buffer[500];
  unsigned n,i;
  virtual void put( const vlong & x );
  virtual vlong get();
  void putc( char c );
  string();
};

void string::putc( char c )
{
  check( n < 500 );
  buffer[n] = c;
  n += 1;
}

void string::put( const vlong & x )
{
  vlong t = x;
  if ( t < 0 ) { putc('-'); t = 0-t; }
  if ( t > 9 )
  {
    put( t/10 );
    n -= 1; // remove trailing space
  }
  putc( '0' + to_unsigned(t%10) );
  putc( ' ' );
}

vlong string::get()
{
  vlong x = 0;
  while ( buffer[i]!=' ' )
  {
    x = x * 10 + buffer[i] - '0';
    i += 1;
    if ( i == n ) i = 0; //wrap round
  }
  // skip the space
  i += 1;
  if ( i == n ) i = 0; //wrap round
  return x;
}

string::string(){ i=0; n=0; }

//////////////////////////////////////////////////////////////////////////

extern "C" void testC( curve & C, int order_known )
{
  string sPub,sPri;
  // Alice computes key pair (d,Q)
  {
    bad_rng rg; // should be good quality rng
    vlong d = rg.next() % C.p;  // secret portion
    point Q = d * C.P;          // public portion
    sPri.put( d );              // save secret portion
    Q.save( sPub );             // save public portion
  }

  // Setup complete : Alice publishes sPub and sCurve
  // and stores sPri securely.
  // End of setup

  // Example 1 : Bob sends Alice a secret random number
  // Bob computes message sMsg
  string sMsg;
  vlong value;
  {
    point Q(&C,C.f); Q.load( sPub );
    bad_rng rg;                 // should be good quality rng
    point S = C.rp(rg);         // shared secret
    vlong sr = rg.next() % C.p; // secret random multiplier
    point M1 = sr * C.P;        // first half of message (independent of Q)
    point M2 = S + sr * Q;      // second half of message (depends on Q)
    M1.save( sMsg );
    M2.save( sMsg );
    value = S.x.get_value();
  }

  // Alice decodes message
  {
    point AM1(&C,C.f); AM1.load(sMsg);
    point AM2(&C,C.f); AM2.load(sMsg);
    point D = AM2 - sPri.get()*AM1;    // decode secret
    check ( D.x.get_value() == value );
  }

  // Alice and Bob could now communicate using the secret
  // number as a key for a stream cipher.
  // End of example

  if (!order_known) return; // need to know order to do authentication

  // Example 2 : Alice signs a document, Bob verifies signature

  // e would be a crypto hash of document to be signed
  vlong e = 12345 % C.p;

  // Alice computes signature sSig
  string sSig;
  {
    bad_rng rg; // should be good quality rng
    retry:
    vlong k = rg.next() % C.p;
    point R = k * C.P;
    vlong z = R.x.get_value();
    vlong r = ( z + e ) % C.p; if ( r==0 ) goto retry;
    vlong s = ( k - sPri.get() * r ) % C.p; if ( s < 0 ) s += C.p;
    sSig.put(r);
    sSig.put(s);
  }

  // Bob checks signature
  {
    vlong r = sSig.get();
    vlong s = sSig.get();
    point Q(&C,C.f); Q.load( sPub );
    point W = s*C.P + r*Q;
    vlong bz = W.x.get_value();
    vlong e1 = ( r - bz ) % C.p; if ( e1 < 0 ) e1 += C.p;
    check( e1 == e );
  }
  // End of example
}

void test_weil( unsigned l, unsigned k )
{
  printf("weil(%u,%u)\n",l,k);

  string sCurve;
  {
    bad_rng rg;
    weil_curve C0( rg, l, k );
    C0.save( sCurve );
  }

  sCurve.putc(0);
  printf(sCurve.buffer);
  printf("\n");

  curve_GF2m C( sCurve ); // Note : Weil curve is over GF2m
  testC( C, 1 );
}

void test_rand_GFp( unsigned bits )
{
  string sCurve;
  {
    bad_rng rg;
    prime_factory pf;
    vlong p = pf.find_prime( rg.next() % pow2(bits) );
    random_curve_GFp C0( p, rg );
    C0.save( sCurve );
  }
  curve_GFp C( sCurve );
  testC( C, 0 );
}

void test_rand_GF2m( unsigned m )
{
  // Check random curve over GF(2^m) poly basis is working
  string sCurve;
  {
    bad_rng rg;
    random_curve_GF2m C0( m, rg );
    C0.save( sCurve );
  }
  curve_GF2m C( sCurve );
  testC( C, 0 );
}

void change_basis_test(unsigned m)
{
  printf("basis test %d test\n",m);

  bad_rng rn;
  GF2m_poly_basis B1(m);
  GF2m_opt_norm_basis B2(m);

  B1.rn = &rn; // needed for conversion!
  B2.rn = &rn; // needed for conversion!

  GFe a(&B2),b(&B2);
  a.set_random(rn);
  b.set_random(rn);
  GFe c = a*b;

  vlong * M = B2.matrix(B1);
  GFe A = convert( a, B1, M );
  GFe B = convert( b, B1, M );
  GFe C = convert( c, B1, M );

  check( C == A*B );

  vlong * IM = inverse( M, m );
  GFe a1 = convert( A, B2, IM ); check( a1 == a );
  GFe b1 = convert( B, B2, IM ); check( b1 == b );
  GFe c1 = convert( C, B2, IM ); check( c1 == c );

  delete [] M;
  delete [] IM;
}

extern "C" int main()
{
  test_weil( 5,7 );
  test_weil( 5,13 );
  test_weil( 9,11 );
  test_weil( 5,31 );

  test_weil( 10,13 );
  test_weil( 6,23 );
  test_weil( 6,29 );
  test_weil( 11,19 );

  change_basis_test(4);
  change_basis_test(23);

  test_rand_GF2m( 40 ); // No ONB
  test_rand_GFp( 30 );
  return 0;
}
