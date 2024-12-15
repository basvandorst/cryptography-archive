// If you use the mfloat-library use a project file otherwise don't use
// a project file (linking problems).
#define use_mfloat

extern unsigned _stklen = 40000U;       // use a large stack segment

#include <conio.h>
#include <stdio.h>
#include <iomanip.h>
#include <stdlib.h>
#ifdef use_mfloat
#include "mf_cpp.h"                     // use the mfloat-library
#define digits 77
#else
#include <math.h>                       // use the standard library
#define mfloat double                   // use 'normal' float
#define digits 18
#endif

#define maxbernoulli 20

int main() {

  mfloat B[maxbernoulli+1];
  mfloat factorial[2*maxbernoulli+1];
  mfloat R1,R2,delta,maxerr,gamma,remainder,R,S,fac;
  int m,k,i,lu,n;

  clrscr();
  cout << "\n\n";
  cout << "          Calculation of Euler's or Mascheroni's constant â\n";
  cout << "          =================================================\n";
  cout << "\n";
  cout << "                              Ú  n   1            ¿\n";
  cout << "                    â =  lim  ³  ä  ÄÄÄ   - ln(n) ³\n";
  cout << "                       n -> ì À i=1  i            Ù\n\n";
  m=256;  //increase, if necessary
  gamma=0;
  for (i=1; i < m; i++) gamma+=mfloat(1)/i;
  gamma-=log(mfloat(m));
  gamma+=mfloat(1)/(2*m);
#ifdef use_mfloat
  maxerr="1E4000";
#else
  maxerr=1E4000;
#endif
  B[0]=1;
  factorial[0]=1;
  factorial[1]=1;
  for (k=1; k < maxbernoulli; k++) {
    factorial[2*k]=factorial[2*k-1]*(2*k);
    factorial[2*k+1]=factorial[2*k]*(2*k+1);
    B[k]=mfloat(1)/2;
    for (i=0; i < k; i++)
      B[k]-=B[i]*factorial[2*k]/(factorial[2*i]*factorial[2*k+1-2*i]);
    // R1 / remainder > 1 ... upper bound
    R1=B[k]*pow(mfloat(m-1),-2*k)/(2*k);
    // R2 / remainder < 1 ... lower bound
    R2=B[k]*pow(mfloat(m+1),-2*k)/(2*k);
    delta=fabs(R1-R2)/2;
    if (delta < maxerr) {
      maxerr=delta;
      remainder=(R1+R2)/2;
      if (k > 1) gamma+=B[k-1]*pow(mfloat(m),-2*(k-1))/(2*(k-1));
    }
    else break;
  }
  gamma+=remainder;
  cout << setiosflags(ios::uppercase);
  cout << "\ngamma = \n";
  cout << setprecision(digits) << gamma << "\n\n";
  cout << "Error of the calculation due to termination of the series: ";
  cout << setprecision(5) << maxerr << "\n\n";
  // Second method for the calculation:
  // Method of Brent
#ifdef use_mfloat
  n=62;    // this value is optimal for 77 digit arithmetic
#else
  n=12;    // this value is optimal for IEEE-floating point arithmetic
#endif
  // estimate of R
  R=1;
  fac=1;
  for (k=1; k <= n-2; k++) {
    fac*=k;
    fac/=-n;
    R+=fac;
  }
  R*=exp((mfloat)-n)/n;
  // estimate of S
  lu=floor(4.3191*n);
  fac=-1;
  S=0;
  for (k=1; k <= lu; k++) {
    fac*=-n;
    fac/=k;
    S+=fac/k;
  }
  gamma=S-R-log((mfloat)n);
  cout << "\n\nSecond Method (less accurate):\n";
  cout << "\ngamma = \n";
  cout << setprecision(digits) << gamma << "\n\n";
  return 0;
}
