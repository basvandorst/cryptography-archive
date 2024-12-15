// ------- MultiIDW.h  Multiple-precision integer decimal algorithms
/*
  Version : Version 3.13, last revised: 1994/08/20, 0600 hours
  Author  : Copyright (c) 1981-1994 by author: Harry J. Smith,
  Address : 19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.
*/

#ifndef MultiID_H
#define MultiID_H

#include <iostream.h>
#include <alloc.h>
#include <stdio.h>     // FILE
#include <fstream.h>   // ofstream etc.
#include "iomanip.h"   // setw()
#include <dos.h>

// Developed in Turbo C++ 1.0.  Converted to Borland C++ 3.0 for Windows.
// In Turbo C++ 1.0 etc. the max odd integer to store in a float is
// 2**24-1 = 167,77215; for a double is 2**53-1 = 9,00719,92547,40991

// Detect Windows NT or Win32s for 32-bit mode
#ifdef __WIN32__
#define huge
#define farfree free
#define farcalloc calloc
#endif

#define False  0
#define True   1
#define Plus   0
#define Minus  1
#define nL     '\n'
#define dL     "\n\n"
#define MuDMax 7         // Max number of decimal digits in a super digit
#define MuNMax 300000000L// Max number of super digits in a multi-precision #
		  // Each super digit is from 0 to 9999999, i.e. Base = 10**7
typedef float  Mu1Digit; // Can hold + or - integer <= Base
typedef double Mu2Digit; // Can hold + or - integer <= Base squared + Base
typedef int    Bool;     // Boolean, has a value of True = 1 or False = 0
typedef int    Sign;     // Has a value of Minus = 1 or Plus = 0
typedef int    TriS;     // Tristate, has a value of -1: <0. 0: =0, +1: >0
typedef Mu1Digit huge *VPtr;  // Value, LSD at index 0
typedef double Real;          // Real type used in FFT
typedef long double RealPlus; // Extended Real type used in FFT
typedef union {               // Array elements used in FFT
  double v;                   //
  char cV[8];                 //
} bytes8;                     //
typedef union {               // Array elements used in convolution
  long double v;              //   to get 16 byte elements vice 10 for
  char cV[16];                //   long doubles memory fetch problem
} bytes16;                    //   work-around for arrays > 65536 bytes

//extern ostream& OutP[2]; // { cout, cLog } ; // OutP[i] << i << nL; did not
					       // a.WritLn( OutP[i]);  work???

// ------- The following are set when the InitMulti is called
extern Mu2Digit Base;     // Radix of multi-precision numbers, 10**MuDMax
extern Mu1Digit MaxDigit; // Base - 1
extern Mu2Digit BaseSq;   // Base squared
extern Mu2Digit BSMB;     // (BaseSq - Base) for MuDiv and MuSqRt
extern RealPlus Pi;       // Pi
extern RealPlus Pi2;      // 2 * Pi
// ------ The following can be tested by user
extern Bool MuErr;        // Boolean, True if multi-precision error occurred
extern Bool MuAbort;      // Boolean, True if Mu procedure interrupted by user
extern Bool SoftAbort;    // Boolean, True if SoftAbort Flag set by operator
extern Bool KeyHit;       // Boolean, True if KeyPressed has been cleared
extern char KeyCh;        // Key input when KeyPressed was cleared
// ------- The following can be changed by user
extern Bool Echo;         // 1 if in Echo output to printer mode, 0 otherwise
extern Bool MuDOnly;      // True if only raw digits of a number are to be output
extern int  MuDpG;        // Digits per group in number output display >= 1 or 0
extern int  MuLenD;       // Min length in digits to display length in digits
extern Bool MuErrRep;     // True if multi-precision error reports are desired
extern long MMax;         // Max number of super digits <= MuNMax
extern long TracN;        // Number if iterations of an innermost loop
extern long Trace;        // Number of "Digits" left to compute
extern long ShortN;       // If > 0 then Short format desired from x.Writ( Out)
extern Bool DiagOn;       // Diagnostics turned on
extern double TV0;        // Dos time value at time zero for diag
extern double TV1;        // Dos time value for delta time, TV2 - TV1
extern double TV2;        // Dos time value for total time, TV2 - TV0
extern double TV3;        // Dos time value for start of a time-out period
extern double Del0;       // Total time from TV0
extern double Del1;       // Delta time from last Diag call
extern double DosClockP;  // Previous value of Dos clock for day cycle test
extern int DosDays;       // Number of days to add to Dos clock
extern int MuMaxW;        // Max number of characters to write on a line
extern int MuTot;         // Total number of characters on the line so far
extern VPtr huge ProtV;   // Pointer to protected Value being returned from +..
extern ofstream Disk;     // Text file, output
extern ofstream LogF;     // Text file, Log output file
extern struct date da;    // Defined in DOS.h, initialized in InitMulti
// int  da_year;          // current year      use GetDateTime(); to fill this
// char da_day;           // day of the month
// char da_mon;           // month (1 = Jan)
extern struct date StartDa;    // Date of Start date of Pi calculation
extern struct date ItDa;       // Date of last iteration
extern struct  time ti;
// unsigned char ti_min;  // minutes           use GetDateTime(); to fill this
// unsigned char ti_hour; // hours
// unsigned char ti_hund; // hundredths of seconds
// unsigned char ti_sec;  // seconds
extern struct  time StartTi;   // Time of start time of Pi calculation
extern struct  time ItTi;      // Time of last iteration
extern int ItNo;               // Iteration number
extern int ItOf;               // Iterations max

// ------- A multiple-precision unsigned integer
class MultiUI {
public:
  long N;      // Current length in super digits, 1..MuNMax
  long M;      // Max number of super digits ever in this number
  VPtr huge V; // string of super digits, Value, LSD at index 0

  // ------- CONSTRUCTORS

  // ------- constructor for a null MultiUI
  MultiUI() { N = 0;  M = 0;  V = NULL; }
  // -------- constructor with just a max size
  MultiUI( long NMaxI);

  // ------- DESTRUCTOR

  ~MultiUI() { if (V == NULL)  ;
	       else {
		 if (V == ProtV)  ProtV = NULL;
	       else {
		 farfree(V);  V = NULL; }}}

  // ------- MEMBER FUNCTIONS

  void NMax( long NMaxI); // Change M = NMaxI
  virtual void Clear() { N = 1; V[0] = 0; } // Clear, set this value = 0
  void SetTo( MultiUI& X); // Set this to X, this = X
  void Norm();             // Normalize this
  void RAdd( MultiUI& X);  // this = this + X
  void RSub( MultiUI& X);  // this = this - X
  void Add( MultiUI& X, MultiUI& Y); // this:= X + Y
  void Sub( MultiUI& X, MultiUI& Y); // this:= X - Y
  TriS AbComp( MultiUI& X); // AbComp = signum( |this| - |X|), signum function
  Bool ZTest();             // Boolean, True if this is Zero
  Bool EQ1( Mu1Digit D1);   // Boolean, True if |this| = D1
  Bool GT1( Mu1Digit D1);   // Boolean, True if |this| > D1
};

// ------- A multiple-precision signed integer
class MultiSI : public MultiUI {
public:
  Sign S; // Has a value of Minus = 1 or Plus = 0

  // ------- CONSTRUCTORS

  // ------- constructor for a null MultiUI
  MultiSI() : MultiUI() { S = Plus; }
  // -------- constructor with just a max size
  MultiSI( long NMaxI) : MultiUI( NMaxI) { S = Plus; }

  // ------- DESTRUCTOR

  ~MultiSI() {;}

  // ------- MEMBER FUNCTIONS

  virtual void Clear() { MultiUI::Clear(); S = Plus; } // Set value = 0
  void ChS();              // Change sign, this = 0 - this
  void SetTo( MultiSI& X); // this:= X
  void Norm();             // Normalize
  void RAdd( MultiSI& X);  // this:= this + X
  void RSub( MultiSI& X);  // this:= this - X
  void Add( MultiSI& X, MultiSI& Y); // this:= X + Y
  void Sub( MultiSI& X, MultiSI& Y); // this:= X - Y
  void Value( char *St, int& i);
    // Convert String to 'this', returns i = # of character used
  void SetTo1( Mu1Digit D1); // this = D1 Mod Base
  void SetToD( double Db);   // this = Db Mod Base**4
  void Get1( Mu1Digit& D1);  // D1 = this Mod Base
  void GetD( double& Db);    // Db = this Mod Base**4
  void RAdd1( Mu1Digit D1);  // this = this + D1
  void Add1( MultiSI& X, Mu1Digit D1); // this = X + D1
  TriS Comp( MultiSI& X);// Comp = signum( this - X), signum function = -1,0,+1
  void Writ( ostream& Out);   // Output this as a line of text
  void WritLn( ostream& Out); // Output this and a new line
  void ShortWr( ostream& Out, long Short);
    // Output this in short form ... if more than Short digits
  void ShortWrLn( ostream& Out, long Short);
    // Output this short and a new line
  void RMul1( Mu1Digit D1); // this = this * D1
  void Mul1( MultiSI& X, Mu1Digit D1); // this = X * D1
  void Mul( MultiSI& X, MultiSI& Y);   // this = X * Y
  void MulSlow( MultiSI& X, MultiSI& Y); // this = X * Y, Slow
  void MulCon( MultiSI& X, MultiSI& Y);  // this = X * Y, Convolution
  void MulTran( MultiSI& X, MultiSI& Y); // this = X * Y, Transform
  void RMul( MultiSI& X); // this = this * X
  void Sq( MultiSI& X);   // this = X * X
  void RSq(); // this = this * this
  void PowMB( MultiSI& B, MultiSI& P); // this = (B ** P) Mod MuMB
  void RPowMB( MultiSI& P); // this = (this ** P) Mod MuMB
  void ModMB();             // this = this Mod MuMB
  void RDiv1( Mu1Digit D1); // this = this / D1
  void Div1( MultiSI& U, Mu1Digit D1);      // this = U / D1
  void Mod1( Mu1Digit D1, Mu1Digit& R1);    // R1 = Rem( this / D1)
  void RDiv1QR( Mu1Digit D1, Mu1Digit& R1); // this = this / D1, R1 = Rem
  void Div1QR( MultiSI& U, Mu1Digit D1, Mu1Digit& R1);// this = U/D1, R1 = Rem
  void Divi( MultiSI& U, MultiSI& D);    // this = U / D
  void DivSlow( MultiSI& U, MultiSI& D); // this = U / D
  void DivFast( MultiSI& U, MultiSI& D); // this = U / D
  void RDiv( MultiSI& D);                // this = this / D
  void Modu( MultiSI& U, MultiSI& D);    // this = Rem(U / D)
  void RMod( MultiSI& D);                // this = Rem( this / D)
  void DivQR( MultiSI& U, MultiSI& D, MultiSI& R);     // this = U / D, R = Rem
  void DivQRSlow( MultiSI& U, MultiSI& D, MultiSI& R); // this = U / D, R = Rem
  void DivQRFast( MultiSI& U, MultiSI& D, MultiSI& R); // this = U / D, R = Rem
  void RDivQR( MultiSI& D, MultiSI& R);             // this = this / D, R = Rem
  void GCD( MultiSI& A, MultiSI& B);     // this = Greatest common divisor A, B
  void RGCD( MultiSI& A);   // this=Greatest common divisor A, this
  void FacMB( MultiSI& X);  // this = (X !) Mod MuMB = 1*2*3*...*X
  void RFacMB();            // this = (this !) Mod MuMB = 1*2*3*...*X
  void SqRtRem( MultiSI& X, MultiSI& R);     // this = SqRt(X), R = Rem
  void SqRtRemSlow( MultiSI& X, MultiSI& R); // this = SqRt(X), R = Rem
  void SqRtRemFast( MultiSI& X, MultiSI& R); // this = SqRt(X), R = Rem
  void RSqRtRem( MultiSI& R);                // this = SqRt( this), R = Rem
  void SqRoot( MultiSI& X);                  // this = SqRt(X)
  void RSqRt();                              // this = SqRt( this)
  void ReadSI( FILE* R, char* Name, int& OK); // Read MultiSI from a file
  MultiSI operator+ (MultiSI& X);  // overloaded + infix add operator
  MultiSI operator- (MultiSI& X);  // overloaded - infix sub operator
  MultiSI operator* (MultiSI& X);  // overloaded * infix mul operator
  MultiSI operator/ (MultiSI& X);  // overloaded / infix div operator
  MultiSI operator% (MultiSI& X);  // overloaded % infix mod operator
  MultiSI operator^ (MultiSI& X);  // overloaded ^ infix xor operator, => power
  MultiSI operator^ (double p);    // overloaded ^ infix xor operator, => power
  MultiSI operator- ();            // overloaded - unary minus operator
  inline MultiSI& operator<<= (MultiSI& X);//overloaded <<= assignment operator
  inline MultiSI& operator<<= (double D);  //overloaded <<= assignment operator
  inline MultiSI& operator<<= (float F);   //overloaded <<= assignment operator
  inline MultiSI& operator<<= (long L);    //overloaded <<= assignment operator
  inline MultiSI& operator<<= (int i);     //overloaded <<= assignment operator
  inline MultiSI& operator<<= (char *St);  //overloaded <<= assignment operator
  inline MultiSI& operator>>= (MultiSI& X);//overloaded >>= assign. op. => swap
  inline MultiSI& operator|= (MultiSI& X); //overloaded |= minimal memory copy
  inline MultiSI& operator|= (double D);   //overloaded |= min mem assign op
  inline MultiSI& operator|= (float F);    //overloaded |= min mem assign op
  inline MultiSI& operator|= (long L);     //overloaded |= min mem assign op
  inline MultiSI& operator|= (int i);      //overloaded |= min mem assign op
  inline MultiSI& operator|= (char *St);   //overloaded |= min mem assign op
  inline MultiSI& operator&= (long L);     //overloaded &= op. => NMax(L)
  inline MultiSI& operator+= (MultiSI& X); //overloaded += replace add operator
  inline MultiSI& operator-= (MultiSI& X); //overloaded -= replace sub operator
  inline MultiSI& operator*= (MultiSI& X); //overloaded *= replace mul operator
  inline MultiSI& operator/= (MultiSI& X); //overloaded /= replace div operator
  inline MultiSI& operator%= (MultiSI& X); //overloaded %= replace mod operator
  inline MultiSI& operator++ (); // overloaded ++ unary Replace add 1 operator
  inline MultiSI& operator-- (); // overloaded -- unary Replace sub 1 operator
  inline Bool operator<  (MultiSI& X); // overloaded <  .LT. operator
  inline Bool operator<= (MultiSI& X); // overloaded <= .LE. operator
  inline Bool operator>  (MultiSI& X); // overloaded >  .GT. operator
  inline Bool operator>= (MultiSI& X); // overloaded >= .GE. operator
  inline Bool operator== (MultiSI& X); // overloaded == .EQ. operator
  inline Bool operator!= (MultiSI& X); // overloaded != .NE. operator
  inline Bool operator! (); // overloaded ! unary boolean op. not, True iff 0
  inline Bool operator~ (); // overloaded ~ unary boolean op. !!, False iff 0
};

// ------- MultiSI's inline methods

// ------- overloaded <<= assignment operator for MultiSI <<= MultiSI
inline MultiSI& MultiSI::operator<<= (MultiSI& X)
  { SetTo(X);  return *this; }
// (X <<= Y <<= Z; OK)
// & needed on return type so this->V will not be destructed (freed)
// These did not actually get compiled in-line but may on a later compiler
// Was compiled as a near call vice a far call though

// ------- overloaded <<= assignment operator for MultiSI <<= double
inline MultiSI& MultiSI::operator<<= (double D)
  { SetToD(D);  return *this; }

// ------- overloaded <<= assignment operator for MultiSI <<= float
inline MultiSI& MultiSI::operator<<= (float F)
  { SetToD( (double)F);  return *this; }

// ------- overloaded <<= assignment operator for MultiSI <<= long
inline MultiSI& MultiSI::operator<<= (long L)
  { SetToD( (double)(L));  return *this; }

// ------- overloaded <<= assignment operator for MultiSI <<= int
inline MultiSI& MultiSI::operator<<= (int i)
  { SetTo1( (float)(i));  return *this; }

// ------- overloaded <<= assignment operator for MultiSI <<= string
inline MultiSI& MultiSI::operator<<= (char *St)
  { int i;  Value( St, i);  return *this; }

// ------- overloaded >>= assignment operator for MultiSI swap
inline MultiSI& MultiSI::operator>>= (MultiSI& X)
  { MultiSI T = X;  X = *this;  *this = T;  T.V = NULL;  return *this; }

// ------- overloaded |= => minimal memory copy
inline MultiSI& MultiSI::operator|= (MultiSI& X)
  { if (V == X.V)  NMax(X.N);  else  { NMax(0);  SetTo(X); }  return *this; }

// ------- overloaded |= minimal memory assignment op for MultiSI |= double
inline MultiSI& MultiSI::operator|= (double D)
  { NMax(4);  SetToD(D);  NMax(N);  return *this; }

// ------- overloaded |= minimal memory assignment op for MultiSI |= float
inline MultiSI& MultiSI::operator|= (float F)
  { NMax(4);  SetToD( (double)F);  NMax(N);  return *this; }

// ------- overloaded |= minimal memory assignment op for MultiSI |= long
inline MultiSI& MultiSI::operator|= (long L)
  { NMax(2);  SetToD( (double)(L));  NMax(N);  return *this; }

// ------- overloaded |= minimal memory assignment op for MultiSI |= int
inline MultiSI& MultiSI::operator|= (int i)
  { NMax(1);  SetTo1( (float)(i));  NMax(N);  return *this; }

// ------- overloaded |= minimal memory assignment op for MultiSI |= string
inline MultiSI& MultiSI::operator|= (char *St)
  { NMax(40);  int i;  Value( St, i);  NMax(N);  return *this; }

// ------- overloaded &= op. => NMax(L)
inline MultiSI& MultiSI::operator&= (long L)
  { NMax(L);  if (!M) {S = Plus;}  return *this; }

// ------- overloaded += assignment operator for replace add += MultiSI
inline MultiSI& MultiSI::operator+= (MultiSI& X)
  { RAdd(X);  return *this; }

// ------- overloaded -= assignment operator for replace sub -= MultiSI
inline MultiSI& MultiSI::operator-= (MultiSI& X)
  { RSub(X);  return *this; }

// ------- overloaded *= assignment operator for replace mul *= MultiSI
inline MultiSI& MultiSI::operator*= (MultiSI& X)
  { RMul(X);  return *this; }

// ------- overloaded /= assignment operator for replace div /= MultiSI
inline MultiSI& MultiSI::operator/= (MultiSI& X)
  { RDiv(X);  return *this; }

// ------- overloaded %= assignment operator for replace mod %= MultiSI
inline MultiSI& MultiSI::operator%= (MultiSI& X)
  { RMod(X);  return *this; }

// ------- overloaded ++ unary operator for MultiSI ( X++ same as ++X )
inline MultiSI& MultiSI::operator++ () // Replace add 1
{ RAdd1(1.0);  return *this; }

// ------- overloaded -- unary operator for MultiSI ( X-- same as --X )
inline MultiSI& MultiSI::operator-- () // Replace subtract 1
{ RAdd1(-1.0);  return *this; }

// ------- overloaded < less than operator for MultiSI
inline Bool MultiSI::operator< (MultiSI& X)
  { return Comp(X) < 0; }

// ------- overloaded <= operator for MultiSI
inline Bool MultiSI::operator<= (MultiSI& X)
  { return Comp(X) <= 0; }

// ------- overloaded > operator for MultiSI
inline Bool MultiSI::operator> (MultiSI& X)
  { return Comp(X) > 0; }

// ------- overloaded >= operator for MultiSI
inline Bool MultiSI::operator>= (MultiSI& X)
  { return Comp(X) >= 0; }

// ------- overloaded == operator for MultiSI
inline Bool MultiSI::operator== (MultiSI& X)
  { return Comp(X) == 0; }

// ------- overloaded != operator for MultiSI
inline Bool MultiSI::operator!= (MultiSI& X)
  { return Comp(X) != 0; }

// ------- overloaded ! unary boolean operator for MultiSI
inline Bool MultiSI::operator! () // 0 => True, else False
{ return ZTest(); }

// ------- overloaded ~ unary boolean operator for MultiSI
inline Bool MultiSI::operator~ () // 0 => False, else True
{ return !ZTest(); }

// --end-- MultiSI's inline methods

// ------- The following is initialized in module MultiIDW.Cpp
extern MultiSI MuMB; // Multi-precision modulo base, 0 => normal arith.

// ------- Other services

Bool HeapOk(); // ------- Check far heap

// ------- Diag for a MultiSI
void DiagSI( char* St, MultiSI& X);

// ------- Fatal error handler
void FatalErr( char error_text[]);

// ------- overloaded sqrt function for MultiSI
// inline
MultiSI sqrt( MultiSI& X); // Could not be done inline, gives warning
//  { MultiSI T, Rem;  T.SqRtRem(X, Rem);  ProtV = T.V;  return T; }

// ------- Allocates a bytes8 vector with range [nl..nh]
bytes8 huge *vector8( long nl,  long nh);

// ------- Frees a bytes8 vector allocated by vector8()
void free_vector8( bytes8 huge *v,  long nl,  long nh);

// ------- FFT of complex type data
void four1( bytes8 huge *data,  long nn,  Bool isign);

// ------- Two FFTs of real data for the price of one
void twofft( bytes8 huge *data1,  bytes8 huge *data2,  bytes8 huge *fft1,
	     bytes8 huge *fft2,   long n);

// ------- FFT of real data
void realft( bytes8 huge *data,  long n,  Bool isign);

// ------- Convolves or deconvolves a real data set (see page 430)
void convlv( bytes8 huge *data,    long n,
	     bytes8 huge *respns,  long m,
	     Bool isign,           bytes8 huge *ans,
	     bytes8 huge *fft);

// ------- MuWriteErr, write error statement and set error flag
void MuWriteErr( char *St);

// ------- Read in string from an istream, allow a blank line
void ReadSt( istream& In, char *St);

// ------- Read in string from the key board, allow a blank line
void ReadSt( char *St);

// ------- Read in a long integer from keyboard
void ReadLInt( char *Mess, long Min, long Max, long Nom,
	       long& II);

// ------- Read in an integer from keyboard
void ReadInt( char *Mess, int Min, int Max, int Nom, int& II);

void MuSetMMax( int NRegs); // ------- Set MMax using MemAvail

void MuInterrupt(); // ------- Test for ESC key pressed to abort operation

// ------- DosClock, get the value of the Dos clock in total seconds
//         Only good to 0.01 seconds, must call earlier once each day
double DosClock();

// ------- Save start time of a "Time-Out" period not to be timed
void TimeOut();

// ------- Adjust TV1 time at end of a "Time-Out" period not to be timed
void TimeIn();

// ------- Output a diagnostic message with delta times
void Diag( char *Mess);

// ------- Log output of diagnostic message with delta times
void DiagL( ostream& LogF, char *Mess);

// ------- Output String and a new line every MuMaxW,
//         MuTot = characters on line so far
void WriteMax( ostream& Out, char *St);

// ------- Output String and line feed every MuMaxW, and a new line
//         MuTot = characters on line so far
void WriteMaxLn( ostream& Out, char *St);

// ------- overloaded << operator for MultiSI output
inline ostream& operator<< (ostream& Out, MultiSI& X)
  { X.Writ( Out);  return Out; }

// ------- overloaded >> operator for MultiSI
inline istream& operator>> (istream& In, MultiSI& X)
  { char St[81];  int i;  ReadSt( In, St);  X.Value( St, i);  return In; }

// ------- overloaded <<= assignment operator for int <<= MultiSI
inline int& operator<<= (int& i, MultiSI& X)
  { Mu1Digit D1;  X.Get1( D1);  i = (int) D1;  return i; }

// ------- overloaded <<= assignment operator for long <<= MultiSI
inline long& operator<<= (long& L, MultiSI& X)
  { double Db;  X.GetD( Db);  L = (long) Db;  return L; }

// ------- overloaded <<= assignment operator for float <<= MultiSI
inline float& operator<<= (float& f, MultiSI& X)
  { Mu1Digit D1;  X.Get1( D1);  f = (float) D1;  return f; }

// ------- overloaded <<= assignment operator for double <<= MultiSI
inline double& operator<<= (double& Db, MultiSI& X)
  { X.GetD( Db);  return Db; }

// ------- Output Date and time output stream
void OutDateTime(ostream& Out);

// ------- Get Date and time
void GetDateTime();

void InitMulti(); // ------- Initialize Multi-precision package

// --end-- Other services

#endif  // ifndef MultiID_H

// --end-- MultiIDW.h  Multiple-precision integer decimal algorithms
