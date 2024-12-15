//  This test program shows the advantages of mfloat numbers.
//  The bessel function J0(x) is calculated by the series.

extern unsigned _stklen = 40000U;       // use a large stack segment

#define convert_IEEE_float
#include <conio.h>
#include <iomanip.h>
#include <math.h>
#include "mf_cpp.h"

//---------------------------------------------------------------------------

long double J0(long double x) {

  const mfloat epsi=1e-20;
  mfloat sum,sqrx,prod,mepsi;
  int i;

  sqrx=x*x/4;
  sum=1;
  prod=1;
  i=0;
  do {
    i++;
    prod*=sqrx/i/i;
    if (i & 1) sum-=prod; // odd "i"
    else sum+=prod;       // even "i"
  } while (epsi <= prod);
  return mftold(sum);
};

//---------------------------------------------------------------------------

long double J0x(long double x) {

  const long double epsi=1e-20;
  long double sum,sqrx,prod;
  int i;

  sqrx=x*x/4;
  sum=1;
  prod=1;
  i=0;
  do {
    i++;
    prod *=sqrx/i/i;
    if (i & 1) sum -= prod;
    else sum += prod;
  } while (epsi <= prod);
  return sum;
};

//---------------------------------------------------------------------------

int main() {

int accuracy;
long double x;

  clrscr();
  cout << "              Calculation of the bessel function J0(x)\n"
       << "              ========================================\n"
       << "\n";
  x=100;
  cout << "mantissa words     result     (x = "
       << x << ")\n\n";
  for (accuracy=1; accuracy<16; accuracy++) {
    setmantissawords(accuracy);
    cout << setw(3) << accuracy
	 << "                J0(" << setw(5) << setprecision(1)
	 << x << ") = "
	 << setiosflags(ios::scientific) //use scientific format
	 << setiosflags(ios::uppercase) //use upcase letter: for example 1.2E3
	 << setw(25) << setprecision(18) << J0(x) << "\n"
	 << resetiosflags(ios::scientific); //use fixpoint format
  }
  cout << "IEEE arithmetic    J0(" << setw(5) << x << ") = "
       << setiosflags(ios::scientific)
       << setw(25) << setprecision(18) << J0x(x) << "\n"
       << resetiosflags(ios::scientific);
  cout << "\nThe accuracy of 12 mantissa words is needed to get a good result!\n";
  return 0;
}