// Here are some methods demonstrated to calculate the number pi

// If you use the mfloat-library use a project file, otherwise don't use
// a project file (linking problems).
#define use_mfloat

extern unsigned _stklen = 40000U;       // use a large stack segment

#include <conio.h>
#include <iomanip.h>

#ifdef use_mfloat
#include "mf_cpp.h"                     // use the mfloat-library
#define digits 77                       // # of output digits
#else
#include <math.h>                       // use the standard library
#define mfloat double                   // use 'normal' float
#define setmantissawords(v)             // ignore setmantissawords()
#define getmantissawords() 5            // replacement for getmantissawords()
#define digits 40                       // # of output digits
double pi=2*acos(0);
#endif

mfloat a,b,c,bound;
int i;

int main(void) {

  clrscr();
  setmantissawords(16);
  bound =ldexp((mfloat)1,-16*getmantissawords()-16);
  cout << "                            Calculation of PI\n";
  cout << "                            =================\n";
  cout << "\n\n";
  cout << "PI = 16 * arctan(1 / 5) - 4 * arctan(1 / 239) :\n";
  a=(mfloat)16/5;
  b=a;
  i=0;
  do {
    i++;
    b/=25;
    c=b/(2*i+1);
    if (i & 1) a-=c;   // odd "i"
    else a+=c;         // even "i"
  } while (bound <= c);
  b=(mfloat)4/239;
  a-=b;
  i=0;
  do {
    i++;
    b/=239;
    b/=239;
    c=b/(2*i+1);
    if (i & 1) a+=c;  // odd "i"
    else a-=c;        // even "i"
  } while (bound <= c);
  cout << setprecision(digits) << a << "\n\n";
  cout << "*******************************************************************************\n\n";
  cout << "PI = 48 * arctan(1 / 18)  + 32 * arctan(1 / 57) - 20 * arctan(1 / 239) :\n";
  a=(mfloat)8/3;
  b=a;
  i=0;
  do {
    i++;
    b/=324;           // 18*18
    c=b/(2*i+1);
    if (i & 1) a-=c;  // odd "i"
    else  a+=c;       // even "i"
  } while (bound <= c);
  b=(mfloat)32/57;
  a+=b;
  i=0;
  do {
    i++;
    b/=3249;          // 57*57;
    c=b/(2*i+1);
    if (i & 1) a-=c;  // odd "i"
    else a+=c;        // even "i"
  } while (bound <= c);
  b=(mfloat)20/239;
  a-=b;
  i=0;
  do {
    i++;
    b/=239;
    b/=239;
    c=b/(2*i+1);
    if (i & 1) a+=c; // odd "i"
    else a-=c;       // even "i"
  } while (bound <= c);
  cout << setprecision(digits) << a << "\n\n";
  cout << "*******************************************************************************\n\n";
  cout << "PI = 6 * arctan(1 / sqrt(3)) :\n";
  a=2*sqrt((mfloat)3);
  b=a;
  i=0;
  do {
    i++;
    b/=3;
    c=b/(2*i+1);
    if (i & 1) a-=c;
    else a+=c;
  } while (bound <= c);
  cout << setprecision(digits) << a << "\n\n";
  cout << "*******************************************************************************\n\n";
  cout << "Internal Pi:\n";
  cout << setprecision(digits) << pi << "\n\n";
  return 0;
}
