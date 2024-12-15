/*  This test program shows the advantages of mfloat numbers. */
/*  The bessel function J0(x) is calculated by the series.    */

extern unsigned _stklen = 40000U;       /* use a large stack segment */

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "mfloat.h"

/*------------------------------------------------------------------------*/

long double J0(long double x) {

  mfloat sum,sqrx,prod,mepsi;
  long double epsi;
  int i;

  epsi=1E-20;
  ldtomf(mepsi,epsi);
  sqrm(ldexpm(ldtomf(sqrx,x),-1));
  getonem(sum);
  getonem(prod);
  i=0;
  do {
    i++;
    divi(divi(multm(prod,sqrx),i),i);
    if (i & 1) subm(sum,prod);
    else addm(sum,prod);
  } while (!gtm(mepsi,prod));
  return(mftold(sum));
}

/*------------------------------------------------------------------------*/

long double J0x(long double x) {

  long double sum, sqrx, prod, epsi;
  int  i;

  epsi=1E-20;
  sqrx=x*x/4;
  sum=1;
  prod=1;
  i=0;
  do {
    i++;
    prod*=sqrx/i/i;
    if (i & 1) sum-=prod; /* odd "i"  */
    else sum+=prod;       /* even "i" */
  } while (prod >= epsi);
  return(sum);
}

/*------------------------------------------------------------------------*/

int main (void) {

  int accuracy;
  long double x;

  clrscr();
  mfloatTest80386();
  printf("              Calculation of the bessel function J0(x)\n");
  printf("              ========================================\n\n");
  x = 100.0;
  printf("mantissa words        result     (x = %5.1Lf)\n\n", x);
  for (accuracy=1; accuracy < 16; accuracy++) {
    setmantissawords(accuracy);
    printf("%3i                J0(%5.1Lf) = %25.18Le\n", accuracy, x, J0(x));
  };
  printf("IEEE arithmetic    J0(%5.1Lf) = %25.18Le\n\n", x, J0x(x));
  printf("The accuracy of 12 mantissa words is needed to get a good result!\n");
  return 0;
}
