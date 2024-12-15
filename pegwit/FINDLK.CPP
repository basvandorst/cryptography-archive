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

static vlong dup ( const vlong & x, unsigned m, unsigned l )
{
  vlong y = 0;
  for (unsigned i=0;i<m/l;i+=1)
    y = y*pow2(l) + x;
  return y;
}

void pv( vlong x )
{
  if ( x > 1 )
    pv ( x / 2 );
  printf("%d",to_unsigned(x%2));
}

void find_lk( unsigned max_l, unsigned max_m )
{
  prime_factory pf(max_m/2);
  for (unsigned m=2;m<max_m;m+=1)
  {
    for (unsigned i=0;i<pf.np;i+=1)
    {
      unsigned k = pf.pl[i];
      unsigned l = m/k;
      if ( m == l*k && l>=2 && l<max_l && l<k )
      {
        if ( GF2m_opt_norm_basis::exists(m) )
        {
          printf("m=%u,l=%u,k=%u\n",m,l,k);
        }
      }
    }
  }
}

extern "C" int main()
{
  find_lk(18,500);
  return 0;
}
