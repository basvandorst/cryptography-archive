// This test program shows the calculation of pi using Geometric-Arithmetic
// Mean.

// If you use the mfloat-library use a project file otherwise don't use
// a project file (linking problems).

#define use_mfloat

extern unsigned _stklen = 40000U;       // use a large stack segment

#include <conio.h>
#include <stdio.h>
#include <iomanip.h>

#ifdef use_mfloat
#include "mf_cpp.h"                     // use the mfloat-library
#define digits 69
#else
#include <math.h>                       // use the standard library
#define mfloat double                   // use 'normal' float
#define sqr(x) (x)*(x)
#define digits 18
#endif

int main() {
  mfloat y,a,z;
  int n,m;

  clrscr();
  cout << "\n\n";
  cout << "              Calculation of Pi using Geometric-Arithmetic Mean\n";
  cout << "              =================================================\n";
  cout << "\n\n\n";
  cout << "Idea from\n";
  cout << "Fausto A. A. Barbuto          Email: BJ06@C53000.PETROBRAS.ANRJ.BR\n";
  cout << "and\n";
  cout << "Jay R. Hill, PhD.             Email: HillJr@Jupiter.SAIC.Com\n\n\n";
  y=sqrt((mfloat)1/2);
  a=mfloat(1)/2;
  m=1;
  for (n=0; n <= 8; n++) {
    m=2*m;
    y=sqr(y/(1+sqrt(1-sqr(y))));
    a=a*sqr(1+y)-m*y;
    cout << setprecision(digits) << "Pi(" << n << ") = " << 1 / a << '\n';
  }
  cout << "\nPi = \n" << setprecision(digits+8) << 1/a << "\n";
  return 0;
}
