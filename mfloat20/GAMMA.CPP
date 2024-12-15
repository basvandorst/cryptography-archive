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
#define digits 30
#define maxbernoulli 22
#define mingammareal 220
#else
#include <math.h>                       // use the standard library
#include <complex.h>
#define mfloat double                   // use 'normal' float
#define mcomplex complex                // use 'normal' complex
#define digits 10
#define maxbernoulli 10
#define mingammareal 50
double pi=2*acos(0);
#endif

//------------------------------------------------------------------------

mcomplex complexinput(char * name) {

  mcomplex x;
  do {
    cout << name;
    // clear stream buffer of cin, only last "CR" remains
    streambuf * bp=cin.rdbuf();
    while (bp->in_avail() > 1) bp->snextc();
    // clear state of cin
    cin.clear();
    cin >> x;
    if (cin.rdstate() == ios::goodbit) break;
    cout << "Error in input!\n\n";  // error-message
  } while (1);
  return x;
}

//-----------------------------------------------------------------------

int main() {

  mfloat B[maxbernoulli+1];
  mfloat factorial[2*maxbernoulli+1];
  mfloat error,maxerr,Ke;
  mcomplex z,sqz,gamma,fak;
  int k,i;

  clrscr();
  cout << "\n\n";
  cout << "          Calculation of the complex Gamma Function â(z)\n";
  cout << "          ==============================================\n";
  cout << "\n";
  cout << "                           ì\n";
  cout << "                           ô  z-1  -t\n";
  cout << "                    â(z) = ³ t    e   dt\n";
  cout << "                           õ \n";
  cout << "                           0 \n\n";
  z=complexinput("z = ");
  fak=1;
  while (real(z) < mingammareal) {
    fak*=z;
    z+=1;
  }
  fak=1/fak;
  gamma=(z-(mfloat)1/2)*log(z)-z+(mfloat)1/2*log((mfloat)2*pi);
#ifdef use_mfloat
  maxerr="1E300";
#else
  maxerr=1E300;
#endif
  sqz=z*z;
  if (real(sqz) < 0) Ke=abs(sqz)/fabs(imag(sqz));
  else Ke=1;
  B[0]=1;
  factorial[0]=1;
  factorial[1]=1;
  for (k=1; k < maxbernoulli; k++) {
    factorial[2*k]=factorial[2*k-1]*(2*k);
    factorial[2*k+1]=factorial[2*k]*(2*k+1);
    B[k]=(mfloat)1/2;
    for (i=0; i < k; i++)
      B[k]-=B[i]*factorial[2*k]/(factorial[2*i]*factorial[2*k+1-2*i]);
    error=Ke*fabs(B[k])/(2*k)/(2*k-1)*pow(abs(z),1-2*k);
    if (error < maxerr) {
      maxerr=error;
      if (k > 1) gamma+=B[k-1]/(2*k-2)/(2*k-3)*pow(z,3-2*k);
    }
    else break;
  }
  gamma=fak*exp(gamma);
  cout << setiosflags(ios::uppercase);
  cout << setiosflags(ios::scientific);
  cout << setiosflags(ios::showpoint);
  cout << "\ngamma = \n";
  cout << setprecision(digits) << gamma << "\n\n";
  cout << "Relative Error of the calculation due to termination of the series: ";
  cout << setprecision(5) << maxerr << "\n\n";
  return 0;
}
