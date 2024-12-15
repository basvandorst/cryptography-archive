//  This program demonstrates the conversion of cartesian coordinates
//  to polar coordinates with mfloat numbers

// If you use the mfloat-library use a project file otherwise don't use
// a project file (linking problems).
#define use_mfloat

extern unsigned _stklen = 40000U;       // use a large stack segment

#include <conio.h>
#include <stdio.h>
#include <iomanip.h>
#ifdef use_mfloat
#include "mf_cpp.h"                     // use the mfloat-library
#else
#include <math.h>                       // use the standard library
#define mfloat double                   // use 'normal' float
#endif

mfloat x,y,r,phi,xr,yr;

//-----------------------------------------------------------------------

mfloat realinput(char * name) {

  mfloat x;
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

  clrscr();
  cout << setiosflags(ios::uppercase); // use upcase letter: for example 1.2E3
  cout << setiosflags(ios::scientific); // use scientific format
  cout << setiosflags(ios::showpoint); // don't remove zeros at the end
  cout << "          Conversion cartesian coorinates - polar coordinates\n";
  cout << "          ===================================================\n";
  cout << "\n\n";
  cout << "Cartesian coordinates:\n";
  x=realinput("x = ");
  y=realinput("y = ");
  cout << "\nx   = " << setprecision(20) << x
       << "\ny   = " << setprecision(20) << y;
  r=hypot(x,y);    // hypot(x,y)=sqrt(x*x+y*y)
  phi=atan2(y,x);  // atan2(y,x)=atan(y/x)
  cout << "\n\n\nConversion to polar coordinates:\n"
       << "\nr   = " << setprecision(20) << r
       << "\nphi = " << setprecision(20) << phi;
  xr=r*cos(phi);
  yr=r*sin(phi);
  cout << "\n\n\nConversion back to cartesian coordinates:\n"
       << "\nx   = " << setprecision(20) << xr
       << "\ny   = " << setprecision(20) << yr
       << "\n";
  return 0;
}