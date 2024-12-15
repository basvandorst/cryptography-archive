// ------- MultiFDW.h  Multiple-precision floating decimal algorithms
/*
  Version : Version 3.11, last revised: 1994-07-29, 0600 hours
  Author  : Copyright (c) 1981-1994 by author: Harry J. Smith,'
  Address : 19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.
*/

#ifndef MultiFD_H
#define MultiFD_H

#include "MultiIDW.h" // Multiple-precision integer decimal algorithms module
#include <conio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// Developed in Turbo Pascal 5.5.  Converted to Borland C++ 3.1 for Windows.

// ------- The following are set when MultiFInit is called
extern Mu1Digit FRound;   // Floating point rounding constant, Base / 2 or 0
extern long     FMC;      // Current max # of super digits in a result
extern long     FTn;      // Current # of super digits to truncate display
extern long     FDn;      // Current max decimal digits to display
extern Bool     ScieN;    // Force scientific notation on flag
extern Bool     Expanded; // True if running in expanded precision
extern long     FMCB;     // Base FMC when running expanded precision

// ------- A multiple-precision signed floating
class MultiF : public MultiSI {
public:
  long C; // Characteristic, 2 => * Base ** 2

  // ------- CONSTRUCTORS

  // ------- constructor for a null MultiF
  MultiF() : MultiSI() { C = 0; }
  // ------- constructor with just a max size
  MultiF( long NMaxI) : MultiSI( NMaxI) { C = 0; } 


  // ------- DESTRUCTOR

  ~MultiF() {;}

  // ------- MEMBER FUNCTIONS

  virtual void Clear() { MultiSI::Clear(); C = 0; } // Set value = 0
  void SetTo( MultiF& X); // this = X
  void Norm();            // Normalize this
  void RAdd( MultiF& X);  // this = this + X
  void RSub( MultiF& X);  // this = this - X
  void Add( MultiF& X, MultiF& Y); // this = X + Y
  void Sub( MultiF& X, MultiF& Y); // this = X - Y
  void Value( char *St, int& i);
    // Convert String to 'this', return i # of character used
  void SetTo1( Mu1Digit D1); // this = D1 Mod Base
  void SetToD( double Db);   // this = Db Mod Base**4
  void Get1( Mu1Digit& D1);  // D1 = this Mod Base
  void GetD( double& Db);    // Db = this Mod Base**4
  void GetDIn( double& Db, double Lo, double Hi); // Db = this Mod Base**4,
                                                  //   in a range, + or -
  void ShiftR( long R); // this = this, R "digits" lost, unnormalized
  void ShiftL( long L); // this = this, R "digits" more, unnormalized
  void RAdd1( Mu1Digit D1);  // this = this + D1
  void Add1( MultiF& X, Mu1Digit D1); // this = X + D1
  TriS Comp( MultiF& X);// Comp = signum( this - X), signum function = -1,0,+1
  void Writ( ostream& Out, long& Tot); // Output this as a line of text
  void WritLn( ostream& Out, long& Tot); // Output this and a new line
//void ShortWr( ostream& Out, long Short);
    // Output this in short form ... if more than Short digits
//void ShortWrLn( ostream& Out, long Short);
    // Output this short and a new line
  void RMul1( Mu1Digit D1); // this = this * D1
  void Mul1( MultiF& X, Mu1Digit D1); // this = X * D1
//void MulL( MultiF& X, MultiF& Y);  // this = X * Y, Low-level
  void MulSlow( MultiF& X, MultiF& Y); // this = X * Y (Slow)
  void MulFast( MultiF& X, MultiF& Y);  // this = X * Y (Fast)
  void Mul( MultiF& X, MultiF& Y);   // this = X * Y (Fast or Slow)
  void RMul( MultiF& X); // this = this * X
  void Sq( MultiF& X);   // this = X * X
  void RSq(); // this = this * this
  void Pow( MultiF& B, MultiF& P); // this = (B ** P) Mod MuMB
  void RPow( MultiF& P); // this = (this ** P) Mod MuMB
  void RDiv1( Mu1Digit D1); // this = this / D1
  void Div1( MultiF& U, Mu1Digit D1);      // this = U / D1
  void Divi( MultiF& U, MultiF& D);    // this = U / D
//void DivSlow( MultiF& U, MultiF& D); // this = U / D
//void DivFast( MultiF& U, MultiF& D); // this = U / D
  void RDiv( MultiF& D);                // this = this / D
  void Fac( MultiF& X);  // this = (X !) Mod MuMB = 1*2*3*...*X
  void RFac();            // this = (this !) Mod MuMB = 1*2*3*...*X
//void SqRtRemSlow( MultiF& X, MultiF& R); // this = SqRt(X), R = Rem
//void SqRtRemFast( MultiF& X, MultiF& R); // this = SqRt(X), R = Rem
  void SqRoot( MultiF& X);                  // this = SqRt(X)
  void RSqRt();                              // this = SqRt( this)
//void ReadF( FILE* R, char* Name, int& OK); // Read MultiF from a file
  void Inte( MultiF& X); // this = Integer part of X
  void RInt(); // this = Integer part of this
  void Fract( MultiF& X); // this = Fractional part of X
  void RFrac(); // this = Fractional part of this
  void Lop( MultiF& X); // this = X with one less super digit
  void RLop(); // this = this with one less least significant "digit"
//MultiF operator+ (MultiF& X);  // overloaded + infix add operator
//inline MultiF& operator<<= (MultiF& X);//overloaded <<= assignment operator
};

// ------- Multi's inline methods

// --end-- MultiF's inline methods

// ------- The following is initialized in module MultiFW.Cpp
// extern MultiF ????; // ????

// ------- Other services

// ------- Returns max of X, Y
int Max(int X, int Y);

// ------- Returns min of X, Y
int Min(int X, int Y);

// ------- Returns max of X, Y
long MaxL(long X, long Y);

// ------- Returns min of X, Y
long MinL(long X, long Y);

// ------- Returns max of X, Y
double MaxD(double X, double Y);

// ------- Returns min of X, Y
double MinD(double X, double Y);

// ------- Compute X = Pi by algorithm a
void PiAA( MultiF& X, Bool WReset, Bool ReStart);
// Uses 4 large "Registers" X, Y, Z, and T.  L is small.
// if WReset then Write reset data after each iteration
// if ReStart then Read reset data before starting

// ------- Compute X = Pi by algorithm b
void PiAB( MultiF& X, Bool WReset, Bool ReStart);
// Uses 4 large "Registers" X, Y, Z, and T.  L is small.
// if WReset then Write reset data after each iteration
// if ReStart then Read reset data before starting

// ------- Compute X = Pi by algorithm b, set Pi to its computed value
void PiTo( MultiF& X);

// ------- Output String and line feed every 80,
//         Tot = characters on line so far
void Write80( ostream& Out, char *St, long& Tot);

// ------- Output String and line feed every 80,
//         Tot = characters on line so far, followed by line feed
void Write80Ln( ostream& Out, char *St, long& Tot);

// ------- Initialize Multi-precision floating decimal package
void InitMultiF();

// --end-- Other services

#endif  // ifndef MultiF_H

// --end-- MultiFDW.h  Multiple-precision floating decimal algorithms
