#include "vlong.hpp"
#include "ncurve.hpp"

extern "C" {
int rand(void);
int printf(const char *_format, ...);
long clock();
}

#ifdef noio
int printf(const char *_format, ...){ return 0; }
#endif

static void assert(int x){ if (x==0) { printf("assert failed"); while (1/x); } }

class ec_crypt1 : public ec_crypt
{
  unsigned rand( unsigned range )
  {
    // This needs replacing with a proper true (not pseudo)
    // random number generator
    return ::rand() % range;
  }
  public:
  ec_crypt1( full_curve_parameter & a ) : ec_crypt( a ) {}
};

class timing
{
  unsigned current;
  long start;
  long total [10];
  long count [10];
  char * name[10];
public:
  timing()
  {
    for (unsigned ix=0;ix<10;ix+=1)
    {
      total[ix] = 0;
      count[ix] = 0;
    }
    current = 0;
    start = clock();
  }
  void set(unsigned ix,char * Name)
  {
    long now = clock();
    count[current] += 1;
    total[current] += now-start;
    current = ix;
    name[ix] = Name;
    start = now;
  }
  ~timing()
  {
    set(0,"");
    for (unsigned ix=1;ix<10;ix+=1)
      if (count[ix])
      {
        printf("  %s %ld",name[ix],total[ix]/(count[ix]*10));
      }
  }
};

void test( const curve_parameter & bp )
{
  if ( bp.L > MAXL || bp.K > MAXK ) return;
  full_curve_parameter a( bp );

  ec_crypt1 C0(a); // pre-calculate full parameters

  // Full parameters should be stored either as compiled data
  // or in a data file / database (if initialisation time is important).

  {
    timing tim;

    tim.set(1,"I");
    ec_crypt1 C(a);
    tim.set(0,"");

    printf("[%u,%u=%u*%u]", C.bits, a.L*a.K, a.L, a.K );

    // Alice computes private/public key pair (sk,pk)
    vlong sk = C.make_private_key();
    vlong pk = C.make_public_key( sk );

    {
      { // Example 1 : Bob sends Alice a secret random number

        tim.set(2,"E"); // Bob computes secret and message
        pair msg;
        vlong secret = C.make_secret( pk, msg );

        // msg is transmitted

        tim.set(3,"D"); // Alice decodes message
        vlong secret2 = C.decode_secret( sk, msg ); // Alice's copy
        assert( secret2 == secret );

      }

      { // Example 2 : Alice signs a document, Bob verifies signature
        // (DSA version)
        tim.set(4,"DSAs");

        vlong e = pk; // e would be a crypto hash of document to be signed (use pk for test)
        pair sig = C.dsa_sign( e, sk );

        // sig is transmitted

        tim.set(5,"DSAv");
        int ok = C.dsa_verify( e, sig, pk ); // Bob verifies signature
        assert( ok );
      }

      { // Example 3 : Alice signs a document, Bob verifies signature
        // (NR version)
        tim.set(6,"NRs");

        vlong e = pk & (pow2(C.bits-1)-1); // e would be a crypto hash of document to be signed (use pk for test)
        pair sig = C.nr_sign( e, sk );

        // sig is transmitted

        tim.set(7,"NRv");
        vlong e2 = C.nr_verify( sig, pk ); // Bob's copy of e
        assert( e2 == e );
      }
    }
  }
  printf("\n");
}

curve_parameter const cdata [] =
{
  #include "ncdatas.hpp"
};

unsigned const num_curve = sizeof(cdata)/sizeof(curve_parameter);

extern "C" void main()
{
  // test curves
  for ( unsigned ci=0; ci < num_curve; ci +=1 )
  {
    unsigned K = cdata[ci].K;
    unsigned L = cdata[ci].L;
    unsigned M = L*K;
    //if ( M >= 100 ) // exclude small curves
      if ( M <= 400 ) // exclude large curves
      {
        test(cdata[ci]);
      }
  }
}

