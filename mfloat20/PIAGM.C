/* This test program shows the calculation of pi using Geometric-    */
/* Arithmetic Mean.                                                  */

extern unsigned _stklen = 40000U;       /* use a large stack segment */

#include <conio.h>
#include <stdio.h>
#include "mfloat.h"                     /* use the mfloat-library */

int main() {
  mfloat y,a;
  mfloat h1,h2,h3;
  int n,m;
  char ch[100];

  mfloatTest80386();
  setmantissawords(16);
  clrscr();
  printf("\n\n");
  printf("              Calculation of Pi using Geometric-Arithmetic Mean\n");
  printf("              =================================================\n");
  printf("\n\n\n");
  printf("Idea from\n");
  printf("Fausto A. A. Barbuto          Email: BJ06@C53000.PETROBRAS.ANRJ.BR\n");
  printf("and\n");
  printf("Jay R. Hill, PhD.             Email: HillJr@Jupiter.SAIC.Com\n\n\n");
  sqrtm(ldtomf(y,0.5));
  ldtomf(a,0.5);
  m=1;
  for (n=0; n <= 8; n++) {
    m=2*m;
    sqrm(divm(y,addm(ldtomf(h1,1),sqrtm(subm(ldtomf(h2,1),sqrm(equm(h3,y)))))));
    subm(multm(a,sqrm(addm(equm(h1,y),ldtomf(h2,1)))),multi(equm(h3,y),m));
	 printf("Pi(%i) = %s\n",n,mftostr(ch,inversm(equm(h1,a)),100,".69F"));
  }
  printf("\nPi = \n%s\n",mftostr(ch,inversm(equm(h1,a)),100,".77F"));
  return 0;
}
