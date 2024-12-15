#include "vlong.hpp"
#include "ncurve.hpp"
#include "prime.hpp"

extern "C" {
int printf(const char *_format, ...);
int atoi(const char *s);
}

extern "C" void main( int argc, char * argv[] )
{
  unsigned minm=0, maxm=2000, minl=8, maxl=MAXL, mink=3, maxk=unsigned(-1);

  if (argc>1) minm = atoi(argv[1]);
  if (argc>2) maxm = atoi(argv[2]);
  if (argc>3) minl = atoi(argv[3]);
  if (argc>4) maxl = atoi(argv[4]);
  if (argc>5) mink = atoi(argv[5]);
  if (argc>6) maxk = atoi(argv[6]);

  if (minl<2) minl = 2;
  if (mink<3) mink = 3;
  if (maxl>MAXL) maxl=MAXL;

  printf("/* start minm=%u maxm=%u minl=%u maxl=%u mink=%u maxk=%u*/\n", minm,maxm,minl,maxl,mink,maxk );

  int first = 1;
  for (unsigned L=minl;L<=maxl;L+=1)
  {
    curve_factory C(L);
    for (unsigned K=mink;K<=maxk && K*L<=maxm;K+=1)
    {
      curve_parameter a;
      if ( K*L >= minm && C.find(K,a) )
      {
        if (!first) printf(",\n");
        first = 0;
        printf("/*%u*/ { %u, %u, %u, %u, %u, %u, %u}", L*K,L,K,a.T,a.root,a.b,a.nso,a.ntf);
      }
    }
  }
  printf("\n");
  printf("/* end minm=%u maxm=%u minl=%u maxl=%u mink=%u maxk=%u*/\n", minm,maxm,minl,maxl,mink,maxk );

}

