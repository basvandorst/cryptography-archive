extern unsigned _stklen = 40000U;       /* use a large stack segment */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>                       /* use the standard library */
#include "mfloat.h"

#define maxbernoulli 20

int main(void) {

  mfloat B[maxbernoulli+1];
  mfloat factorial[2*maxbernoulli+1];
  mfloat R1,R2,delta,maxerr,gamma,remainder,R,S,fac;
  mfloat h1,h2;
  int m,k,i,n,lu;
  char ch[100];

  mfloatTest80386();
  setmantissawords(16);
  clrscr();
  printf("\n\n");
  printf("          Calculation of Euler's or Mascheroni's constant â\n");
  printf("          =================================================\n");
  printf("\n");
  printf("                              Ú  n   1            ¿\n");
  printf("                    â =  lim  ³  ä  ÄÄÄ   - ln(n) ³\n");
  printf("                       n -> ì À i=1  i            Ù\n\n");
  m=256;  /* increase, if necessary */
  ldtomf(gamma,0);
  for (i=1; i < m; i++) addm(gamma,divi(ldtomf(h1,1),i));
  subm(gamma,logm(ldtomf(h1,m)));
  addm(gamma,divi(ldtomf(h1,1),2*m));
  ldtomf(maxerr,1E4000);
  ldtomf(B[0],1);
  ldtomf(factorial[0],1);
  ldtomf(factorial[1],1);
  for (k=1; k < maxbernoulli; k++) {
    multi(equm(factorial[2*k],factorial[2*k-1]),2*k);
    multi(equm(factorial[2*k+1],factorial[2*k]),2*k+1);
    ldtomf(B[k],0.5);
    for (i=0; i < k; i++) {
      multm(equm(h1,factorial[2*k]),B[i]);
      multm(equm(h2,factorial[2*i]),factorial[2*k+1-2*i]);
      subm(B[k],divm(h1,h2));
    }
    /* R1 / remainder > 1 ... upper bound */
    divi(multm(equm(R1,B[k]),powi(ldtomf(h1,m-1),-2*k)),2*k);
    /* R2 / remainder < 1 ... lower bound */
    divi(multm(equm(R2,B[k]),powi(ldtomf(h1,m+1),-2*k)),2*k);
    divi(fabsm(subm(equm(delta,R1),R2)),2);
    if (gtm(maxerr,delta)) {
      equm(maxerr,delta);
      divi(addm(equm(remainder,R1),R2),2);
      if (k > 1)
	addm(gamma,divi(multm(powi(ldtomf(h1,m),-2*(k-1)),B[k-1]),2*(k-1)));
    }
    else break;
  }
  addm(gamma,remainder);
  printf("\ngamma = \n");
  printf("%s\n\n", mftostr(ch,gamma,100,".77F"));
  printf("Error of the calculation due to termination of the series: ");
  printf("%s\n\n", mftostr(ch,maxerr,100,".5E"));
  // Second method for the calculation:
  // Method of Brent
  n=62;    // this value is optimal for 77 digit arithmetic
  // estimate of R
  ldtomf(R,1);
  ldtomf(fac,1);
  for (k=1; k <= n-2; k++) {
    multi(fac,k);
    divi(fac,-n);
    addm(R,fac);
  }
  multm(R,divi(expm(ldtomf(h1,-n)),n));
  // estimate of S
  lu=floor(4.3191*n);
  ldtomf(fac,-1);
  ldtomf(S,0);
  for (k=1; k <= lu; k++) {
    multi(fac,-n);
    divi(fac,k);
    addm(S,divi(equm(h1,fac),k));
  }
  subm(addm(negm(logm(ldtomf(gamma,n))),S),R);
  printf("\n\nSecond Method (less accurate):\n");
  printf("\ngamma = \n");
  printf("%s\n\n", mftostr(ch,gamma,100,".77F"));
  return 0;
}
