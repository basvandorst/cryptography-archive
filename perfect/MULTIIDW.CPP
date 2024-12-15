// ------- MultiIDW.Cpp Multiple-precision integer decimal algorithms
/*
  Version= "Version 3.12, last revised: 1995/08/20, 0600 hours";
  Author = "Copyright (c) 1981-1995 by author: Harry J. Smith,";
  Address= "19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.";
*/

#include "MultiIDW.h" // Multiple-precision integer decimal algorithms module
#include <dos.h>      // gettime()
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <alloc.h>
#include <stdlib.h>
#include <iostream.h>

#define max(a,b) (((a) > (b)) ? (a) : (b))

// char *cgets( char *str); // Not in Windows, use cin.get( St, 81);

// Developed in Turbo C++ 1.0.  Converted to Borland C++ 3.0 for Windows.
// In Turbo C++ 1.0 the max odd integer to store in a float is
// 2**24-1 = 167,77215; for a double is 2**53-1 = 9,00719,92547,40991

//  ostream& Out0 = NULL; // None of this worked???
//  ostream_withassign OutP[2] = {cout, cLog};
//  ostream OutP[2]; // OutP[i] << i << nL;  // a.WritLn( OutP[i]);
//  OutP[0] = cout;
//  ostream& Out = cout; // This worked

// ------- The following are set when the InitMulti is called
  Mu2Digit Base;     // Radix of multi-precision numbers, 10**MuDMax
  Mu1Digit MaxDigit; // Base - 1
  Mu2Digit BaseSq;   // Base squared
  Mu2Digit BSMB;     // (BaseSq - Base) for MuDiv and MuSqRt
  RealPlus Pi;       // Pi
  RealPlus Pi2;      // 2 * Pi
// ------- The following can be tested by user
  Bool MuErr;        // Boolean, True if multi-precision error occurred
  Bool MuAbort;      // Boolean, True if Mu procedure interrupted by operator
  Bool SoftAbort;    // Boolean, True if SoftAbort Flag set by operator
  Bool KeyHit;       // Boolean. True if KeyPressed has been cleared
  char KeyCh;        // Key input when KeyPressed was cleared
// ------- The following can be changed by user
  Bool Echo;         // 1 if in Echo output to log file mode, 0 otherwise
  Bool MuDOnly;      // True if only raw digits of a number are to be output
  int  MuDpG;        // Digits per group in number output display >= 1 or 0
  int  MuLenD;       // Min length in digits to display length in digits
  Bool MuErrRep;     // True if multi-precision error reports are desired
  long MMax;         // Max number of super digits <= MuNMax
  long TracN;        // Number if iterations of an innermost loop
  long Trace;        // Number of "Digits" left to compute
  long ShortN;       // If > 0 then Short format desired from x.Writ( Out)
  Bool DiagOn;       // Diagnostics turned on
  double TV0;        // Dos time value at time zero for diag
  double TV1;        // Dos time value for delta time, TV2 - TV1
  double TV2;        // Dos time value for total time, TV2 - TV0
  double TV3;        // Dos time value for start of a time-out period
  double Del0;       // Total time from TV0
  double Del1;       // Delta time from last Diag call
  double DosClockP;  // Previous value of the Dos clock for day cycle test
  int DosDays;       // Number of days to add to Dos clock
  int MuMaxW;        // Max number of characters to write on a line
  int MuTot;         // Total number of characters on the line so far
  VPtr huge ProtV;   // Pointer to protected Value being returned from +...
  ofstream Disk;     // Text file, output
  ofstream LogF;     // Text file, Log output file
  struct date da;    // Defined in DOS.h, initialized in InitMulti
// int  da_year;          // current year      use GetDateTime(); to fill this
// char da_day;           // day of the month
// char da_mon;           // month (1 = Jan)
  struct date StartDa;    // Date of Start date of Pi calculation
  struct date ItDa;       // Date of last iteration
  struct  time ti;
// unsigned char ti_min;  // minutes           use GetDateTime(); to fill this
// unsigned char ti_hour; // hours
// unsigned char ti_hund; // hundredths of seconds
// unsigned char ti_sec;  // seconds
  struct  time StartTi;   // Time of start time of Pi calculation
  struct  time ItTi;      // Time of last iteration
  int ItNo;               // Iteration number
  int ItOf;               // Iterations max

// ------- The following are set when this module is initialized
  MultiSI MuMB(1); // Multi-precision modulo base, 0 => normal arith.

// ------- Implementation

// ------- Common hidden routines

// ------- |Y| = |Y| + |X|
void IpAdd( MultiUI& X, MultiUI& Y, Mu1Digit& K)
// Multi-precision integer add absolute values in place;  Y is changed
// Y.N Must be >= X.N, X.S and Y.S not used or changed;  X not changed
// Overflow super digit returned not zero if and only if Y overflows
// MuErr flag not set.  Address of X and Y can be the same, then X is changed
{
  Mu2Digit T;
  long i = 0;  K = 0;
  while (i < X.N) {
    T = X.V[i] + Y.V[i] + K; // Add next super digit with carry
    if (T >= Base) {
      K = 1;  Y.V[i] = T - Base;
    } else {
      K = 0;  Y.V[i] = T;
    }
    i++;
  }
  if (!K)  return;
  while (i < Y.N) { // X = 0 from here on
    if (Y.V[i] == MaxDigit) { // K = 1, carry to next super digit of Y
      Y.V[i] = 0;
    } else {
      Y.V[i]++;  K = 0;  return; // Return K = 0}
    }
    i++;
  }
  Y.N++;
  if (Y.N > Y.M) { // If overflow return K = 1
    Y.N = Y.M;
    Y.Norm();
  } else {
    Y.V[Y.N-1] = 1;  K = 0;  return; // Return K = 0
  }
} // --end-- IpAdd

// ------- |Y| = |Y| - |X|
void IpSub( MultiUI& X, MultiUI& Y)
// Multi-precision integer subtract absolute values in place;  Y is changed
// |Y| Must be >= |X|, X.S and Y.S not used or changed;  X not changed
// MuErr flag not set.  Address of X and Y can be the same, then X is changed
{
  Mu1Digit K = 0;
  for (long i = 0; i < X.N; i++) {
    Mu2Digit T = Y.V[i] - X.V[i] + K; // Subtract next 'digit' with borrow
    if (T < 0) {
      K = -1;  Y.V[i] = T + Base;
    } else {
      K = 0;  Y.V[i] = T;
    }
  }
  if (!K || (X.N == Y.N)) { Y.Norm();  return; }
  for ( ; i < Y.N; i++) { // X = 0 from here on
    if (!Y.V[i]) { // K = -1, borrow from next super digit of Y
      Y.V[i] = MaxDigit;
    } else {
      Y.V[i]--;  Y.Norm();  return;
    }
  }
} // --end-- IpSub

// ------- X = |D| * X
void IpMul1( Mu1Digit D, MultiUI& X, Mu1Digit& K)
// Multi-precision one super digit multiply in place;  X is changed
// K = Overflow super digit returned > zero if and only if X overflows
// Signs not used
{
  D = fabs(D);  K = 0;
  for (long i = 0; i < X.N; i++) {
    Mu2Digit T = D * X.V[i] + K; // Multiply next super digit and add carry
    K = floor(T / Base);
    X.V[i] = T - K * Base;
  }
  if (!K)  return;
  X.N++;
  if (X.N > X.M) { // If overflow return K > 0
    X.N = X.M;
  } else {
    X.V[X.N-1] = K;  K = 0;
  }
} // --end-- IpMul1

// ------- U = U / D
void IpDiv1( MultiSI& U, Mu1Digit D, Mu1Digit& R)
// Multi-precision one super digit divide in place, U is changed, R = Remainder
// Allows literal D, MuErr set True if D = 0
{
  R = 0;
  if (!D) {
    MuWriteErr("Cannot divide by zero, continuing...");  return;
  }
  MuInterrupt();
  if (MuAbort) {
    U.Clear();  return;
  }
  Mu1Digit AD = fabs(D);
  for (long i = U.N - 1; i >= 0; i--) {
    Mu2Digit T = R * Base + U.V[i];//Divide next super digit and save remainder
    U.V[i] = floor(T / AD);
    R = T - U.V[i] * AD;
  }
  i = U.S;
  if (U.S && R)  R = -R;
  U.S = (U.S && !(D < 0)) || (!U.S && (D < 0)); // XOR
  U.Norm();
} // --end-- IpDiv1

// ------- Q = U / D, R = Rem, Slow Method (but fast for small numbers)
void IpDivSlow( MultiSI& U, MultiSI& D, MultiSI& Q, MultiSI& R)
// U and D are not changed, U is moved to R and then R is divided
// in place to give Q and a remainder in R, it is OK for U and R
// to have the same location in memory, but then U will be changed
// Sets MuErr True if D = 0
// See Knuth Vol. 2 of "The Art..." Algorithm D, pg. 237.
{
  Mu1Digit V1, V2, D1, K, QH;
  Mu2Digit T;
  long i, j, ij;
  if (D.N == 1) { // Use IpDiv1 for a one super digit divide
    Q.SetTo(U);  D1 = D.V[0];
    if (D.S && D1)  D1 = -D1;
    IpDiv1(Q, D1, K);
    R.S = (K < 0);
    R.V[0] = fabs(K);
    R.N = 1;
  } else {
    R.SetTo(U); // SetTo is No-Op if same address
    if (R.N < D.N) {
      Q.Clear();  return;
    }
    Q.N = R.N - D.N + 1;
    j = R.N;
    R.V[R.N] = 0;
    D.V[D.N] = 0;
    V1 = D.V[D.N-1];
    V2 = D.V[D.N-2];
    D1 = floor( Base / (V1 + 1));    // D1. Normalize
    if (D1 > 1) {
      IpMul1( D1, D, K);  V1 = D.V[D.N-1];  V2 = D.V[D.N-2];
      IpMul1( D1, R, K);
      if (K)  R.V[j] = K;
    }                               // D2. Init j done above
    while (j >= D.N) {              // D3. Calculate Q hat
      T = R.V[j] * Base + R.V[j-1];
      if (V1 == R.V[j])
	QH = MaxDigit;
      else
	QH = floor(T / V1);
      while (V2 * QH > (T - QH * V1) * Base + R.V[j-2])  QH--;
// QH =QH+1; .Diag for add back test
      K = Base;
      for (i = 0; i <= D.N; i++) {     // D4. Multiply and subtract
	ij = i + j - D.N;
	T = R.V[ij] - QH * D.V[i] + K + BSMB; // + BaseSq - Base
	K = floor(T / Base);
	R.V[ij] = T - K * Base;
      }
      Q.V[j - D.N] = QH;            // D5. Test remainder
      if (K == MaxDigit) {
// WriteLn("MuDiv: Doing an add back"); .Diag for add back test
	Q.V[j - D.N] = QH - 1;    // D6. Add back
	K = 0;  ij = j - D.N;
	for (i = 0; i<= D.N; i++) {
	  T = D.V[i] + R.V[ij] + K;
	  K = floor(T / Base);
	  R.V[ij] = T - K * Base;
	  ij++;
	}
      }
      j--;                    // D7. Loop on j
      MuInterrupt();
      if (MuAbort) {
	Q.Clear();  R.Clear();  return;
      }
      if (KeyHit) {
	cout << "Integer Divide: "
	     << floor( 100.0 * (R.N - j) / (R.N - D.N + 1))
	     << " Percent done" << nL;
	KeyHit = False;
      }
    }
    Q.S = (U.S && !D.S) || (!U.S && D.S); // XOR
    R.S = U.S;
    Q.Norm();
    R.Norm();
    if (D1 > 1) {	   // D8. Unnormalize
      IpDiv1(R, D1, K);
      IpDiv1(D, D1, K);
    }
  }
} // --end-- IpDivSlow

// ------- Q = U / D, R = Rem, Fast method for large numbers
void IpDivFast( MultiSI& U, MultiSI& D, MultiSI& Q, MultiSI& R)
// U and D are not changed, U is divided by D
// to give Q and a remainder R, it is OK for U and R
// to have the same location in memory, but then U will be changed
// Sets MuErr True if D = 0
{
  MultiSI DL;  // Local Divisor scaled down (Points to D.V)
  MultiSI M;   // 1 / D and 2*M
  MultiSI MM;  // M * M
  MultiSI MMD; // (M*M) * D
  MultiSI UL;  // UL = Upper part of U (Points to U.V)
  MultiSI QL;  // QL = U * M
  MultiSI RL;  // Local remainder
  MultiSI E;   // 1.0 for 1 / D and Error in Q
  long i, j, pl, po, ps;

  if (D.N <= 2 || (U.N - D.N) <= 2) {
    IpDivSlow(U, D, Q, R);  return;
  }
  pl = 5;
  if (pl > (U.N - D.N))  pl = (U.N - D.N + 1);
  j = (pl+1 > D.N) ? D.N : pl+1;
  DL.N = DL.M = j;
  DL.V = D.V + (D.N-j);
  E.NMax( pl+1+DL.N);  E.N = E.M;
  for (i = 0; i < E.N; i++)  E.V[i] = 0; // E = 1.0 Shifted
  E.V[ E.N-1] = 1;
  M.NMax( pl+12);
  IpDivSlow(E, DL, M, E);
  E.NMax(5);  DL.V = NULL;

  while (pl <= (U.N - D.N)) { // M = 2*M - D*M^2 = M + M - (M*M) * D
    po = pl;  pl += pl;
    if (pl > (U.N - D.N))  pl = (U.N - D.N) + 1;
    MM.NMax( M.N + M.N);
    MM.Mul(M, M);
    ps = po+po-pl;
    MM.V += ps;
    MM.N -= ps;
    j = (pl+1 > D.N) ? D.N : pl+1;
    DL.N = DL.M = j;
    DL.V = D.V + (D.N-j);
    MMD.NMax( MM.N + DL.N);
    MMD.Mul( MM, DL);
    MMD.V += DL.N;
    MMD.N -= DL.N;
    MM.V -= ps;  MM.NMax(1);  DL.V = NULL;
    M.RAdd(M);
    for (j = M.N-1, i = j + pl-po, M.NMax(i+1), M.N = M.M;
	 i >= 0;  i--, j--) {
      if (j >= 0)
	M.V[i] = M.V[j];
      else
	M.V[i] = 0;
    }
    M.RSub( MMD);
    MMD.V -= DL.N;  MMD.NMax(1);
  }

  UL.N = UL.M = U.N - D.N + 1;
  UL.V = U.V + (D.N-1);
  QL.NMax( UL.N + M.N);
  QL.Mul( UL, M);
  UL.V = NULL;  M.NMax(1);
  QL.N -= pl+1;
  QL.V += pl+1;
  RL.NMax( D.N + QL.N);
  RL.Mul(D, QL);
  RL.Sub(U, RL);
  if (RL.Comp(D) >= 0) {  // if Rem > D
    IpDivSlow( RL, D, E, RL);
    QL.RAdd(E);
  }
  else if (RL.S) {  // if Rem < 0
    RL.S = Plus;
    IpDivSlow( RL, D, E, RL);
    QL.RSub(E);
    if (!RL.ZTest()) {  // if Rem != 0
      RL.Sub(D, RL);
      QL.RAdd1(-1);
    }
  }
  else
  ;
  Q.SetTo( QL);
  QL.V -= pl+1;
  R.SetTo( RL);
} // --end-- IpDivFast

// ------- Q = U / D, R = Rem
void IpDiv( MultiSI& U, MultiSI& D, MultiSI& Q, MultiSI& R)
// U and D are not changed, U is divided by D
// to give Q and a remainder R, it is OK for U and R
// to have the same location in memory, but then U will be changed
// Sets MuErr True if D = 0
{
  float prod = (float) (U.N - D.N) * D.N;
  if (prod / U.N < 344.0)
    IpDivSlow(U, D, Q, R);
  else
    IpDivFast(U, D, Q, R);
} // --end-- IpDiv

// --end-- Common hidden routines

// ------- MultiUI's method implementation

// ------- Constructor Multi-precision unsigned integer
//           with just a max size
MultiUI::MultiUI( long NMaxI)
{
  M = NMaxI;
  if (M > MuNMax)  M = MuNMax;
  long Size = sizeof( Mu1Digit);
  Size = (M+1) * Size;
  if ((V = (float huge*) farcalloc(M + 1, sizeof( Mu1Digit))) == NULL) {
    MuWriteErr("Insufficient memory for Multi-precision number");
    cout << "need " << Size << nL;
    if (Echo)  LogF << "need " << Size << nL;
    M = 1;
    V = (float huge*) farcalloc(M + 1, sizeof( Mu1Digit));
  }
  Clear();
} // --end-- MultiUI::MultuUI( long NMaxI)

// ------- NMax, change M = NMax
void MultiUI::NMax( long NMaxI)
{
  if (NMaxI == M && V != NULL)  return;
  if (NMaxI < 1) {
    if (V != NULL) { farfree(V);  V = NULL; }
    N = 0;  M = 0;  return;
  }
  if (NMaxI > MuNMax)  NMaxI = MuNMax;
  VPtr v = (float huge*) farcalloc( NMaxI + 1, sizeof( Mu1Digit));
  M = NMaxI;
  if (V == NULL) {
    V = v;  Clear();
  } else {
    if (N > NMaxI)  N = NMaxI;
    for (long i = 0; i < N; i++)  v[i] = V[i];
    farfree(V);
    V = v;
  }
  v = NULL;
} // --end-- NMax, change M = NMax

// ------- SetTo, this = X, copy used "digits" only
void MultiUI::SetTo( MultiUI& X)
{
  if (V == X.V)  return;
  if (V == NULL)  NMax(X.N); // Change size of this iff NULL
  N = X.N;
  if (N > M) {
    N = M;
    MuWriteErr("SetTo overflow, continuing...");
  }
  for (long i = 0; i < N; i++)  V[i] = X.V[i];
} // --end-- MultiUI::SetTo, this = X

// ------- Normalize this
void MultiUI::Norm()
{
  if (N < 1)  Clear();
  for (long i = N - 1; !V[i] && i; i--)  ; // Delete leading zeros
  N = i + 1;
} // --end-- MultiUI::Norm()

// ------- AbComp = sign( |this| - |X|)
int MultiUI::AbComp( MultiUI& X)
// Si = -1 if |this| < |X|; Si = 0 if |this| = |X|; Si = +1 if |this| > |X|}
// this.S and X.S not used or changed;  this, X not changed}
{
  if (N < X.N)  return -1;
  if (N > X.N)  return +1;
  for (long i = N - 1; (V[i] == X.V[i]) && i; i--)  ;
  if (V[i] < X.V[i])  return -1;
  if (V[i] > X.V[i])  return +1;
  return 0;
} // --end-- MultiUI::AbComp( MultiUI& X)

// ------- this = this + X
void MultiUI::RAdd( MultiUI& X)
// Sets MuErr True if overflow
{
  if (N < X.N) {  // If Digits in this < Digits in X
    if (M >= X.N) {
      for (long i = N; i < X.N; i++)  V[i] = 0;
      N = X.N;
    } else {
      MuWriteErr("X too big to add, continuing...");
    }
  }
  Mu1Digit K;
  IpAdd(X, *this, K);
  if (K > 0) MuWriteErr("Addition overflow, continuing...");
} // --end-- MultiUI::RAdd( MultiUI& X)

// ------- this = this - X
void MultiUI::RSub( MultiUI& X)
// Sets MuErr True if N > this
{
  int i = AbComp(X);
  if (!i)  Clear();
  else if (i < 0)		// If this < X, Error
    MuWriteErr("Unsigned subtraction error, continuing...");
  else
    IpSub(X, *this);
} // --end-- MultiUI::RSub( MultiUI& X)

// ------- this = X + Y
void MultiUI::Add( MultiUI& X, MultiUI& Y)
// Sets MuErr True if overflow
{
  if (V == X.V)
    RAdd(Y);
  else if (V == Y.V)
    RAdd(X);
  else if (X.N >= Y.N) {
    SetTo(X);  RAdd(Y);
  } else {
    SetTo(Y);  RAdd(X);
  }
} // --end-- MultiUI::Add( MultiUI& X, MultiUI& Y)

// ------- this = X - Y
void MultiUI::Sub( MultiUI& X, MultiUI& Y)
// Sets MuErr True if overflow
{
  MultiUI YL;
  if (X.V == Y.V)  Clear();
  else if (V == X.V)
    RSub(Y);
  else if (V == Y.V) {
    MultiUI YL( Y.N);  YL.SetTo(Y);
    SetTo(X);
    RSub( YL);
  } else {
    SetTo(X);
    RSub(Y);
  }
} // --end-- MultiUI::Sub( MultiUI& X, MultiUI& Y)

// ------- Boolean, True if this is Zero
Bool MultiUI::ZTest()
{
  return (N == 1) && (V[0] == 0);
} // --end-- MultiUI::ZTest()

// ------- Boolean, True if |this| = D1
Bool MultiUI::EQ1( Mu1Digit D1)
// this, D1 are not changed, allows literal D1
{
  return (N == 1) && (V[0] == D1);
} // --end-- MultiUI::EQ1( Mu1Digit D1)

// ------- Boolean, True if |this| > D1
Bool MultiUI::GT1( Mu1Digit D1)
// this, D1 are not changed, allows literal D1
{
  return (N > 1) || (V[0] > D1);
} // --end-- MultiUI::GT1( Mu1Digit D1)

// --end-- MultiUI's method implementation

// ------- MultiSI's method implementation

// ------- Change sign, this = - this
void MultiSI::ChS() {
  if (!ZTest())  S = !S;
} // --end-- MultiSI::ChS()

// ------- this = X
void MultiSI::SetTo( MultiSI& X) {
  MultiUI::SetTo(X);  S = X.S;
} // --end-- MultiSI::SetTo( MultiSI& X)

// ------- Normalize
void MultiSI::Norm() {
  MultiUI::Norm();
  if (ZTest())  S = False; // Prevent -0
} // --end-- MultiSI::Norm()

// ------- this = this + X
void MultiSI::RAdd( MultiSI& X)
// Sets MuErr True if overflow
{
  int i;
  if (S == X.S)		// Same signs
    MultiUI::RAdd(X);
  else {		// Opposite signs
    i = AbComp(X);
    if (i < 0) {	// If |this| < |X|, |this| = |X| - |this|
      MultiUI Temp(N);
      Temp.SetTo( *this);// Temp = Small this
      SetTo(X);		// this = Large X
      IpSub( Temp, *this);
    }
    else if (i > 0)	// If |this| > |X|, |this| = |this| - |X|
      IpSub(X, *this);
    else
      Clear();		// If |this| = |X|, this = 0
  }
} // --end-- MultiSI::RAdd( MultiSI& X)

// ------- this = this - X
void MultiSI::RSub( MultiSI& X)
// Sets MuErr True if overflow
{
  if (V == X.V)
    Clear();
  else {
    X.S = !X.S;
    RAdd(X);
    X.S = !X.S;
  }
} // --end-- MultiSI::RSub( MultiSI& X)


// ------- this = X + Y
void MultiSI::Add( MultiSI& X, MultiSI& Y)
// Sets MuErr True if overflow
{
  if (V == X.V)
    RAdd(Y);
  else if (V == Y.V)
    RAdd(X);
  else if (X.S == Y.S) {
    MultiUI::Add(X, Y);  S = X.S;
  } else {
    int i = X.AbComp(Y);              // Signs are different
    if (!i)  Clear();
    else if (i > 0) {
      SetTo(X);  IpSub(Y, *this);  // |X| > |Y|, so |X| - |Y|
    } else {
      SetTo(Y);  IpSub(X, *this);
    }
  }
} // --end-- // MultiSI::Add( MultiSI& X, MultiSI& Y)


// ------- this = X - Y
void MultiSI::Sub( MultiSI& X, MultiSI& Y)
// Sets MuErr True if overflow
{
  if (X.V == Y.V)  Clear();
  else if (V == X.V)
    RSub(Y);
  else if (V == Y.V) {
    RSub(X);  ChS();
  } else {
    Y.S = !Y.S;
    Add(X, Y);
    Y.S = !Y.S;
  }
} // --end-- MultiSI::Sub( MultiSI& X, MultiSI& Y)

// ------- Convert String to this
void MultiSI::Value( char *St, int& i)
// Returns i = # of character used
// Allows St to be a literal, MuErr set True if overflow
{
  Mu1Digit K;
  int Len;
  int j, D;
  Bool Fini; // Boolean
  Bool OverFlow; // Boolean
  for (i = 0; St[i] == ' '; i++) ;
  if ((St[i] == '-') || (St[i] == '+'))  i++;
  while ((St[i] != '\0') &&
	(((St[i] >= '0') && (St[i] <= '9')) || (St[i] == ',')))
    i++;
  Len = i;
  Clear();  Fini = False;  OverFlow = False;
  if (!Len)  return;
  for (j = 0; St[j] == ' '; j++);
  if (St[j] == '-') { j++;  S = True; }
  else if (St[j] == '+')  j++;
  while (!Fini && (j < Len)) {
    if ((St[j] >= '0') && (St[j] <= '9')) {
      if ((N == M) && (V[N-1] >= Base / 10)) {
	OverFlow = True;
	Fini = True;
      } else {
	D = St[j] - '0';
	IpMul1( 10, *this, K);
	if (K > 0)  OverFlow = True;
	V[0] += D;
      }
    }
    else
      if (St[j] != ',')  Fini = True;
    j++;
  }
  if (OverFlow)  MuWriteErr("Input number overflow, continuing...");
  Norm();
} // --end-- MultiSI::Value()

// ------- this = D1 Mod Base
void MultiSI::SetTo1( Mu1Digit D1)
// Allows a literal D1, D1 an integer, |D1| < Base, D1 not changed
{
  Mu1Digit AbsD = fabs( D1);
  N = 1;  S = (D1 < 0);
  if (AbsD < Base)  V[0] = AbsD;
  else
    V[0] = AbsD - (int)(AbsD / Base) * Base;
} // --end-- MultiSI::SetTo1()

// ------- this = Db Mod Base**4
void MultiSI::SetToD( double Db)
// Allows a literal Db, Db not changed, MuErr set True if overflow
{
  char St[255];
  double AbsD;
  int L, ML;
  AbsD = fabs( Db);
  sprintf( St, "%0.0f", AbsD);
  L = strlen( St);  ML = 4 * MuDMax;
  if (L > ML)
    strcpy( St, &St[L - ML]);
  Value( St, L);
  if (Db < 0)  ChS(); // Prevents -0
} // --end-- MultiSI::SetToD()

// ------- D1 = this Mod Base
void MultiSI::Get1( Mu1Digit& D1)
// this is not changed
{
  D1 = V[0];
  if (S && D1)  D1 = -D1;
} // --end--MultiSI::Get1()

// ------- Db = this Mod Base**4
void MultiSI::GetD( double& Db)
// this is not changed
{
  double T = Base;
  Db = V[0];
  for (long i = 1; (i < N) && (i < 4); i++) {
    Db += V[i] * T; // Add next super digit
    T *= Base;
  }
  if (S && Db)  Db = -Db;
} // --end-- MultiSI::GetD()

// ------- this = this + D1
void MultiSI::RAdd1( Mu1Digit D1)
// Allows literal D1, D1 is not changed, sets MuErr True if overflow
{
  MultiSI DL(1);
  DL.SetTo1( D1);
  RAdd( DL);
} // --end-- // MultiSI::RAdd1()

// ------- this = X + D1
void MultiSI::Add1( MultiSI& X, Mu1Digit D1)
// Allows literal D1, D1 is not changed, sets MuErr True if overflow
{
  SetTo(X);
  RAdd1( D1);
} // --end-- // MultiSI::Add1()

// ------- Comp = sign( this - X)
int MultiSI::Comp( MultiSI& X)
// Sign = -1 if this < X; Sign = 0 if this = X; Sign = +1 if this > X
// this.S and X.S used but not changed;  this, X not changed
{
  if (!S && !X.S)  return AbComp(X);		// Both positive
  else if (S && X.S)  return X.AbComp( *this);	// Both negative
  else if (X.S)  return +1;			// this pos, X neg
  else  return -1;				// this neg, X pos
} // --end-- // MultiSI::Comp()

// ------- Output Character and a new line every MuMaxW,
//         MuTot = characters on line so far
void WriteMax( ostream& Out, char Ch)
{
  if ((&Out == &cout) || (MuMaxW <= 0))  Out << Ch;
  else {
    Out << Ch;
    MuTot++;
    if (MuTot >= MuMaxW) {
      Out << nL;  MuTot = 0;
    }
  }
} // --end-- // WritMaxC()

// ------- Output String and a new line every MuMaxW,
//         MuTot = characters on line so far
void WriteMax( ostream& Out, char *St)
{
  if ((&Out == &cout) || (MuMaxW <= 0))  Out << St;
  else
    for( int i = 0;  St[i] != '\0';  i++) {
      Out << St[i];
      MuTot++;
      if (MuTot >= MuMaxW) {
	Out << nL;  MuTot = 0;
      }
    }
} // --end-- // WritMax()

// ------- Output String and line feed every MuMaxW, and a new line
//         MuTot = characters on line so far
void WriteMaxLn( ostream& Out, char *St)
{
  WriteMax( Out, St);  Out << nL;  MuTot = 0;
} // --end-- // WritMaxLn()

// ------- Output this as a line of text
void MultiSI::Writ( ostream& Out)
// this is not modified
{
  char  Sep = ',';

  if (MuDOnly)  Sep = ' ';
  if (!M)  {WriteMax( Out, '0');  return;}
  if (ShortN && N >= 3 && N > (ShortN / MuDMax))
    { ShortWr( Out, ShortN);  return; }
  char St[9];
  long ToGo = (long)(N) * MuDMax;
  if (S)  WriteMax( Out, '-');
  sprintf( St, "%0.0f", Base + V[N - 1]);
  for (int j = 1; (St[j] == '0') && (j != MuDMax); j++)  ;
  WriteMax( Out, St[j]);
  ToGo -= j;
  long LenD = ToGo + 1;
  for (j++; j <= MuDMax; j++) {
    if (MuDpG && !(ToGo % MuDpG))  WriteMax( Out, Sep);
    WriteMax( Out, St[j]);
    ToGo--;
  }
  for (long i = 2; i <= N; i++) {
    sprintf( St, "%0.0f", Base + V[N - i]);
    for (j = 1; j <= MuDMax; j++) {
      if (MuDpG && !(ToGo % MuDpG))  WriteMax( Out, Sep);
      WriteMax( Out, St[j]);
      ToGo--;
    }
  }
  if (!MuDOnly) {
    if (LenD >= MuLenD) {  //  Out << " (" << LenD << ')';
      sprintf( St, "%ld", LenD);
      if ( (&Out != &cout) && (MuMaxW > 0) &&
        ((strlen( St) + MuTot + 3) > MuMaxW) ) {
        Out << nL;  MuTot = 0;
      }
      if ((&Out == &cout) || (MuMaxW <= 0) || (MuTot != 0))
        WriteMax( Out, ' ');
      WriteMax( Out, '(');  WriteMax( Out, St);  WriteMax( Out, ')');
    }
  }
} // --end-- // MultiSI::Writ()

// ------- Output this and a new line
void MultiSI::WritLn( ostream& Out)
{
  Writ( Out);  Out << nL;  MuTot = 0;
} // --end-- // MultiSI::WritLn()

// ------- Output this in short form ... if more than Short digits
void MultiSI::ShortWr( ostream& Out, long Short)
// this is not modified
{
  if (!M)  {Out << '0';  return;}
  char St[9];
  if ((N < 3) || (N <= (Short / MuDMax))) {
    Writ( Out);  return;
  }
  long ToGo = (long)(N) * MuDMax;
  if (S)  Out << '-';
  sprintf( St, "%0.0f", Base + V[N - 1]);
  for (int j = 1; (St[j] == '0') && (j != MuDMax); j++)  ;
  Out << St[j];  int Did = 1;
  ToGo -= j;
  long LenD = ToGo + 1;
  for (j++; (j <= MuDMax) && (Did < 5); j++) {
    Out << St[j];
    Did++;
  }
  sprintf( St, "%0.0f", Base + V[N - 2]);
  for (j = 1; j < 6-Did; j++) {
    Out << St[j];
  }
  Out << ",...,";
  sprintf( St, "%0.0f", Base + V[0]);
  for (j = MuDMax-4; j < MuDMax+1; j++) {
    Out << St[j];
  }
  Out << " (" << LenD << ')';
} // --end-- // MultiSI::ShortWr()

// ------- Output this short and a new line
void MultiSI::ShortWrLn( ostream& Out, long Short)
{
  ShortWr( Out, Short);  Out << nL;
} // --end-- // MultiSI::ShortWrLn()

// ------- this = this * D1
void MultiSI::RMul1( Mu1Digit D1)
// Multi-precision one super digit multiply; D1 is not changed
// MuErr set true if this overflows
{
  Mu1Digit K;
  S = (S && !(D1 < 0)) || (!S && (D1 < 0)); // XOR
  IpMul1( D1, *this, K);
  if (K > 0)
    MuWriteErr("One digit multiply overflow, continuing...");
} // --end-- // MultiSI::RMul1()

// ------- this = X * D1
void MultiSI::Mul1( MultiSI& X, Mu1Digit D1)
// Multi-precision one super digit multiply; X and D1 are not changed
// MuErr set true if this overflows
{
  SetTo(X);  RMul1( D1);
} // --end-- // MultiSI::Mul1()

// ------- this = X * Y
void MultiSI::Mul( MultiSI& X, MultiSI& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  float prod = (float) X.N * Y.N;
  if (prod < 40.0)
    MulSlow(X, Y);
  else if (prod / (X.N + Y.N) < 344.0)  // 4816 * 4816 dec digs
    MulCon(X, Y);
  else
    MulTran(X, Y);
} // --end-- // MultiSI::Mul()

// ------- this = X * Y, Slow Method (but fast for small numbers)
void MultiSI::MulSlow( MultiSI& X, MultiSI& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  long i, j, ij;
  Mu1Digit K;
  Mu2Digit T;
  VPtr XVP, YVP, VP;
  MultiSI XL = X;
  MultiSI YL = Y;
  MultiSI TL = XL; // Temp pointer for swap
  if (X.V == V) {
    XL.V = NULL;  XL.NMax(X.N); // Change M = NMax
    XL.SetTo(X);
  }
  else if (Y.V == V) {
    YL.V = NULL;  YL.NMax(Y.N); // Change M = NMax
    YL.SetTo(Y);
  }
  if (X.V == Y.V)  YL = XL;
  else if (XL.N < YL.N) { TL = XL;  XL = YL;  YL = TL; }
  S = (XL.S && !YL.S) || (!XL.S && YL.S); // XOR
  ij = (XL.N < M) ? XL.N : M;
  for (i = 0, VP = V;  i < ij;  i++, VP++)  *VP = 0; // Clear lower of this
  KeyHit = False;
  for (j = 0, YVP = YL.V;  j < YL.N;  j++, YVP++) {
    for (K = 0, i = 0, ij = j, VP = V + j, XVP = XL.V;
	i < XL.N;  i++, ij++, VP++, XVP++) {
      if (ij < M) {		// Mult, add and carry next Digit
	T = (double) *XVP * *YVP + *VP + K;
	K = floor(T / Base);
	*VP = T - K * Base;
      }
      else
	i = XL.N; // Break out of inner loop
    }
    if (ij < M)  V[ij] = K;
    MuInterrupt();
    if (MuAbort) {
      Clear();  TL.V = NULL;
      if ((XL.V == X.V) || (XL.V == Y.V))  XL.V = NULL;
      if ((YL.V == X.V) || (YL.V == Y.V) || (YL.V == XL.V))  YL.V = NULL;
      return;
    }
    if (KeyHit) {
      cout << "Integer Multiply (Slow): "
	   << floor( 100.0 * (j+1) / YL.N) << " Percent done" << nL;
      KeyHit = False;
    }
  }
  if (K)  ij++;
  if (ij > M) {
    ij = M;  MuWriteErr("MulSlow: Multiplication overflow, continuing...");
  }
  N = ij;
  Norm();  TL.V = NULL;
  if ((XL.V == X.V) || (XL.V == Y.V))  XL.V = NULL;
  if ((YL.V == X.V) || (YL.V == Y.V) || (YL.V == XL.V))  YL.V = NULL;
} // --end-- // MultiSI::MulSlow()

// ------- this = X * Y, Convolution method
void MultiSI::MulCon( MultiSI& X, MultiSI& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  long i, j, size;
  long double K;
  long double T;
  MultiSI XL;
  MultiSI YL;
  VPtr XVP, YVP, VP;
  bytes16 huge *b16A;  // bytes16 array
  bytes16 huge *b16P;  // bytes16 pointer
  size = X.N + Y.N - 1;
  if ( (b16A = (bytes16 huge*) farcalloc( size, sizeof( bytes16))) == NULL) {
    MuWriteErr(
      "Integer Multiplication (Con.) not enough memory, continuing...");
    Clear();  return;
  }
  if (Y.N < X.N)  { XL = X;  YL = Y; }
	    else  { XL = Y;  YL = X; }
  S = (XL.S && !YL.S) || (!XL.S && YL.S); // XOR
  for (i = 0, b16P = b16A;  i < size;  i++, b16P++)
    b16P->v = 0; // Clear convolution
  KeyHit = False;
  for (j = 0, YVP = YL.V;  j < YL.N;  j++, YVP++) {
    for (i = 0, b16P = b16A + j, XVP = XL.V;  i < XL.N;  i++, b16P++, XVP++)
      b16P->v += (long double) *XVP * *YVP;
    if (!((j+1) % 184467L)) {  // prevent overflow of long double value
      for (K = 0, i = 0, b16P = b16A;  i < size;  i++, b16P++) {
	T = b16P->v + K;    // carry next Digit
	K = floorl(T / Base);
	b16P->v = T - K * Base;
      }
    }
    MuInterrupt();
    if (MuAbort) {
      XL.V = NULL;  YL.V = NULL;
      farfree( b16A);  Clear();  return;
    }
    if (KeyHit) {
      cout << "Integer Multiply (Con.): "
	   << floor( 100.0 * (j+1) / YL.N) << " Percent done" << nL;
      KeyHit = False;
    }
  }
  for (K = 0, i = 0, b16P = b16A, VP = V;  i < size;  i++, b16P++, VP++) {
    if (i < M) {	   // carry next Digit
      T = b16P->v + K;
      K = floorl(T / Base);
      *VP = T - K * Base;
    }
    else
      i = size; // Break out of loop
  }
  if (i < M)  V[i] = K;
  if (K)  i++;
  if (i > M) {
    i = M;  MuWriteErr("MulCon: Multiplication overflow, continuing...");
  }
  N = i;
  XL.V = NULL;  YL.V = NULL;
  farfree( b16A);  Norm();
} // --end-- // MultiSI::MulCon()

// ------- Generate data for FFT
void FftGenDat( MultiSI& X, bytes8 huge *data, long n, int DDSD)
{
  long i, j, ii;
  long double K;
  long double T;
  long double Base2;
  long double Ten2;
  int d, left;

  for (Base2 = 10.0, i = 1;  i < DDSD; i++)  Base2 *= 10.0; // 10^DDSD
  for (j = 1; j <= n; j++)  data[j].v = 0.0;
  j = 1;  d = DDSD;  left = 7;  K = 0;
  for (i = 0; i < X.N; i++) {
    for (Ten2 = 1.0, ii = 0;  ii < DDSD-d; ii++)  Ten2 *= 10.0; // 10^(DDSD-d)
    T = X.V[i] * Ten2;  left = 7 + (DDSD-d);  d = DDSD;
    while (left > 0) {
      T += K;
      K = floorl(T / Base2);
      data[j].v += T - K * Base2;
      T = 0;
      if (d == DDSD)  j++;
      left -= d;
      if (!left && d != DDSD)  d = DDSD-d;
      if (left && left < d)  d = left;

      if (!(i % 1000)) {
	if (kbhit()) {
	  MuInterrupt();  KeyHit = False;
	  if (MuAbort)  return;
	  cout << "FftGenDat: Generating data, i = " << i <<" of "<< n <<nL;
	}
      }
    }
  }
} // --end-- // FftGenDat

// ------- Store data from FFT
void FftStoDat( MultiSI& X, bytes8 huge *data, long n, int DDSD)
{
  long i, j, ii, size;
  long double K;
  long double T;
  long double Base2;
  long double Ten2;
  int d, left, over;

  for (Base2 = 10.0, i = 1;  i < DDSD; i++)  Base2 *= 10.0; // 10^DDSD
  for (size = n;  size >= 1 && data[size].v < 0.5 ;  size--) ;
  for (K = 0, j = 1;  j <= size;  j++) {
    T = floorl( data[j].v + 0.5) + K;		// carry next Digit
    K = floorl(T / Base2);
    data[j].v = T - K * Base2;
    if (j == size && K != 0.0) { size++;  data[size].v = 0.0; }
  }
  X.N = (size * DDSD + 6) / 7;
  over = (X.N > X.M+1);
  if (X.N > X.M)    X.N = X.M;
  for (i = 0; i < X.N; i++)  X.V[i] = 0;
  X.V[X.M] = 0;

  j = 1;  d = 0;  K = 0;
  for (i = 0; i < X.N; i++) {
    for (Ten2 = 1.0, ii = 0;  ii < d; ii++)  Ten2 *= 10.0; // 10^d
    left = 7;  d += DDSD;
    while (left > 0) {
      if (j > size)  T = K;
      else
	T = data[j].v * Ten2 + K;
      K = floorl(T / Base);
      X.V[i] += T - K * Base;
      Ten2 *= Base2;
      j++;
      left -= d;
      if (left <= 0)  d = -left;  else  d = DDSD;

      if (!(i % 1000)) {
	if (kbhit()) {
	  MuInterrupt();  KeyHit = False;
	  if (MuAbort)  return;
	  cout << "FftGenDat: Generating data, i = " << i <<" of "<< n <<nL;
	}
      }
    }
  }
  if (K != 0.0)  over = True;
  if (over)  MuWriteErr("FftStoDat: Multiplication overflow, continuing...");
  X.Norm();
} // --end-- // FftStoDat

// ------- this = X * Y, Transform method  // Not done yet !!!
void MultiSI::MulTran( MultiSI& X, MultiSI& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  bytes8 huge *data;    long n;
  bytes8 huge *respns;  long m;
  Bool isign;           bytes8 huge *ans;  // All vectors [1.. ]
  bytes8 huge *fft;

  long n0;
  int DDSD;     // Decimal digits per super digit in FFT

// Results of tests:  (n = super digits in FFT, L + M - 1, 99..99 * 99..99)
// Using double data type to store FFT
// -Super digit-  --n--  -MaxErr-    --n--  -MaxErr-
//    9999999      2^4     0.031      2^5     0.125
//     999999      2^9     0.063      2^10    0.156
//      99999      2^15    0.063      2^16    0.141
//       9999      2^20    0.06 est   2^15    0.16 est
//        999      2^25    0.06 est   2^16    0.16 est

  DDSD = 7;
  n0 = X.N + Y.N - 1;
  if (n0 <= 16) ;  // 2^4
  else {
    DDSD = 6;
    n0 = (7 * X.N + 5) / 6 + (7 * Y.N + 5) / 6 - 1;
    if (n0 <= 512) ;  // 2^9
  else {
    DDSD = 5;
    n0 = (7 * X.N + 4) / 5 + (7 * Y.N + 4) / 5 - 1;
    if (n0 <= 32768L) ;  // 2^15
  else {
    DDSD = 4;
    n0 = (7 * X.N + 3) / 4 + (7 * Y.N + 3) / 4 - 1;
    if (n0 <= 1048576L) ;  // 2^20
  else {
    DDSD = 3;
    n0 = (7 * X.N + 2) / 3 + (7 * Y.N + 2) / 3 - 1;
    if (n0 <= 33554432L) ;  // 2^25
  else {
    DDSD = 2;
    n0 = (7 * X.N + 1) / 2 + (7 * Y.N + 1) / 2 - 1;
  } } } } }

  for (n = 2;  n < n0;  n *= 2) ;  // n = 2 ^ p >= n0  (n >= 2 also)
//if (n > 16000)
//  cout << "MulTran: l, m, DDSD, n = " << X.N << ", " << Y.N << ", "
//	 << DDSD << ", " << n << nL;
  m = 0;  isign = 1;
  ans = vector8(1, 2 * n);
  fft = vector8(1, 2 * n);
  if (ans == NULL)  MuWriteErr(
    "Integer Multiplication (Tran.) not enough memory, continuing...");
  if (fft == NULL) {
    MuWriteErr(
    "Integer Multiplication (Tran.) not enough memory, continuing...");
    cout << "MulTran: l, m, DDSD, n = " << X.N << ", " << Y.N << ", "
    	 << DDSD << ", " << n << nL;
  }
  data   = ans;
  respns = ans + n;
  if (!MuAbort && !MuErr)  FftGenDat(X, data, n, DDSD);
  if (!MuAbort && !MuErr)  FftGenDat(Y, respns, n, DDSD);
  if (!MuAbort && !MuErr)  convlv( data, n, respns, m, isign, ans, fft);
  if (!MuAbort && !MuErr)  FftStoDat( *this, ans, n, DDSD);
  free_vector8( fft, 1, 2 * n);
  free_vector8( ans, 1, 2 * n);
  if (MuAbort || MuErr)  Clear();
  Norm();
}  // --end-- // MultiSI::MulTran()

// ------- this = this * X
void MultiSI::RMul( MultiSI& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow
{
  Mul( *this, X);
} // --end-- // MultiSI::RMul()

// ------- this = X * X
void MultiSI::Sq( MultiSI& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow
{
  Mul(X, X);
} // --end-- // MultiSI::Sq()

// ------- this = this * this
void MultiSI::RSq()
// Sets MuErr True if overflow
{
  Mul( *this, *this);
} // --end-- // MultiSI::RSq()

// ------- this = this Mod MuMB
void MultiSI::ModMB()
// this is changed
{
  if (MuMB.ZTest())  return;
  int i = AbComp( MuMB);
  if (i >= 0) { // if |this| >= |MuMB|
    MultiSI QL(N);
    IpDiv( *this, MuMB, QL, *this);
  }
} // --end-- // MultiSI::ModMB()

// ------- this = this / D1
void MultiSI::RDiv1( Mu1Digit D1)
// Multi-precision one super digit divide, D1 is not changed
// Allows literal D1, MuErr set True if D1 = 0
{
  Mu1Digit RL;
  IpDiv1( *this, D1, RL);
} // --end-- // MultiSI::RDiv1()

// ------- this = U / D1
void MultiSI::Div1( MultiSI& U, Mu1Digit D1)
// Multi-precision one super digit divide, D1 and U not changed
// U and this may have same address in memory, then U is changed
// Allows literal D1, MuErr set True if D1 = 0
{
  Mu1Digit RL;
  SetTo(U);
  IpDiv1( *this, D1, RL);
} // --end-- // MultiSI::Div1()

// ------- R1 = Rem( this / D1)
void MultiSI::Mod1( Mu1Digit D1, Mu1Digit& R1)
// Multi-precision one super digit Modulo, D1 and this not changed
// D1 and R1 may have same address in memory, then D1 is changed
// Allows literal D1, MuErr set True if D1 = 0
{
  MultiSI QL(N);
  QL.SetTo( *this);
  IpDiv1( QL, D1, R1);
} // --end-- // MultiSI::Mod1()

// ------- this = this / D1, R1 = Rem
void MultiSI::RDiv1QR( Mu1Digit D1, Mu1Digit& R1)
// Multi-precision one super digit Divide Q & R.  D1 and U not changed
// D1 and R1 may have same address in memory, then D1 is changed
// Allows literal D1, MuErr set True if D1 = 0
{
  IpDiv1( *this, D1, R1);
} // --end-- // MultiSI::RDiv1QR()

// ------- this = U / D1, R1 = Rem
void MultiSI::Div1QR( MultiSI& U, Mu1Digit D1, Mu1Digit& R1)
// Multi-precision one super digit Divide Q & R.  D1 and U not changed
// D1 and R1 may have same address in memory, then D1 is changed
// Allows literal D1, MuErr set True if D1 = 0
{
  SetTo(U);
  IpDiv1( *this, D1, R1);
} // --end-- // MultiSI::Div1QR()

// ------- this = U / D
void MultiSI::Divi( MultiSI& U, MultiSI& D)
// U or D or both may have the same address in memory as this
// U, D are not changed unless they share memory with this
// Sets MuErr True if D = 0
{
  MultiSI DL = D;
  if (D.V == V) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  MultiSI RL(U.N);  RL.SetTo(U);
  IpDiv( RL, DL, *this, RL);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::Divi()

// ------- this = U / D, Slow Method (but fast for small numbers)
void MultiSI::DivSlow( MultiSI& U, MultiSI& D)
// U or D or both may have the same address in memory as this
// U, D are not changed unless they share memory with this
// Sets MuErr True if D = 0
{
  MultiSI DL = D;
  if (D.V == V) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  MultiSI RL(U.N);  RL.SetTo(U);
  IpDivSlow( RL, DL, *this, RL);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::DivSlow()

// ------- this = U / D, Fast method for large numbers
void MultiSI::DivFast( MultiSI& U, MultiSI& D)
// U or D or both may have the same address in memory as this
// U, D are not changed unless they share memory with this
// Sets MuErr True if D = 0
{
  MultiSI DL = D;
  if (D.V == V) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  MultiSI RL(U.N);  RL.SetTo(U);
  IpDivFast( RL, DL, *this, RL);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::DivFast()

// ------- this = this / D
void MultiSI::RDiv( MultiSI& D)
// D may have the same address in memory as this
// D is not changed unless it shares memory with this
// Sets MuErr True if D = 0
{
  Divi( *this, D);
} // --end-- // MultiSI::RDiv()

// ------- this = Rem(U / D)
void MultiSI::Modu( MultiSI& U, MultiSI& D)
// U or D or both may have the same address in memory as this
// U, D are not changed unless they share memory with this
// Sets MuErr True if D = 0
{
  MultiSI DL = D;
  if (D.V == V) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  MultiSI QL(U.N);
  SetTo(U);
  IpDiv( *this, DL, QL, *this);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::Modu()

// ------- this = Rem( this / D)
void MultiSI::RMod( MultiSI& D)
// D may have the same address in memory as this
// D is not changed unless it shares memory with this
// Sets MuErr True if D = 0
{
  Modu( *this, D);
} // --end-- // MultiSI::RMod()

// ------- this = U / D, R = Rem
void MultiSI::DivQR( MultiSI& U, MultiSI& D, MultiSI& R)
// U or D or both may have the same address in memory as this or R
// U, D are not changed unless they share memory with this or R
// this and R may not have the same location in memory. Sets MuErr True if D = 0
{
  if (V == R.V) {
    MuWriteErr(
     "Cannot set same location to both quotient and remainder, continuing...");
    return;
  }
  MultiSI DL = D;
  if ((D.V == V) || (D.V == R.V)) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  R.SetTo(U);
  IpDiv(R, DL, *this, R);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::DivQR()

// ------- this = U / D, R = Rem, Slow Method (but fast for small numbers)
void MultiSI::DivQRSlow( MultiSI& U, MultiSI& D, MultiSI& R)
// U or D or both may have the same address in memory as this or R
// U, D are not changed unless they share memory with this or R
// this and R may not have the same location in memory. Sets MuErr True if D=0
{
  if (V == R.V) {
    MuWriteErr(
     "Cannot set same location to both quotient and remainder, continuing...");
    return;
  }
  MultiSI DL = D;
  if ((D.V == V) || (D.V == R.V)) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  R.SetTo(U);
  IpDivSlow(R, DL, *this, R);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::DivQRSlow()

// ------- this = U / D, R = Rem, Fast method for large numbers
void MultiSI::DivQRFast( MultiSI& U, MultiSI& D, MultiSI& R)
// U or D or both may have the same address in memory as this or R
// U, D are not changed unless they share memory with this or R
// this and R may not have the same location in memory. Sets MuErr True if D=0
{
  if (V == R.V) {
    MuWriteErr(
     "Cannot set same location to both quotient and remainder, continuing...");
    return;
  }
  MultiSI DL = D;
  if ((D.V == V) || (D.V == R.V)) {
    DL.V = NULL;  DL.NMax(D.N); // Change M = NMax
    DL.SetTo(D);
  }
  R.SetTo(U);
  IpDivFast(R, DL, *this, R);
  if (DL.V == D.V)  DL.V = NULL;
} // --end-- // MultiSI::DivQRFast()

// ------- this = this / D, R = Rem
void MultiSI::RDivQR( MultiSI& D, MultiSI& R)
// this may have the same address in memory as D or R
// D is not changed unless it shares memory with this or R
// Sets MuErr True if D = 0
{
  DivQR( *this, D, R);
} // --end-- // MultiSI::RDivQR()

// ------- this = (B ** P) Mod MuMB
void MultiSI::PowMB( MultiSI& B, MultiSI& P)
// B or P or both may have the same address in memory as this
// B, P are not changed unless they share memory with this
// Sets MuErr True if overflow or error
{
  long L;
  Mu1Digit R;
  if (B.ZTest()) { // B = 0
    if (P.S)
      MuWriteErr("Cannot raise zero to a negative power");
    else if (P.ZTest()) // P = 0
      MuWriteErr("Cannot raise zero to the zero power");
    Clear();  return;
  }
  if ((P.S) && ((B.N > 1) || (B.V[0] > 1))) { // (>1) ** -P
    Clear();  return;
  }
  if ((B.N == 1) && (B.V[0] == 1)) { // B = +/-1
    L = floor(P.V[0]);
    S = B.S && (L % 2);
    N = 1;  V[0] = 1;  return;
  }
  MultiSI BL(M);    BL.SetTo(B);
  MultiSI PL(P.N);  PL.SetTo(P);
  SetTo1(1);
  Bool Fini = False;
  while (!Fini) {
    L = floor( PL.V[0]);
    if (L % 2) {
      Mul( BL, *this);  ModMB();
    }
    IpDiv1( PL, 2, R);
    if (PL.ZTest())  Fini = True;
    else {
      BL.Mul( BL, BL);  BL.ModMB();
    }
    if (MuAbort) {
      Clear();  Fini = True;
    }
  }
} // --end-- // MultiSI::PowMB()

// ------- this = (this ** P) Mod MuMB
void MultiSI::RPowMB( MultiSI& P)
// P may have the same address in memory as this
// P is not changed unless it shares memory with this
// Sets MuErr True if overflow or error
{
  PowMB( *this, P);
} // --end-- // MultiSI::RPowMB()

// ------- this = Greatest common divisor A, B
void MultiSI::GCD( MultiSI& A, MultiSI& B)
// A, B are not modified unless they share memory with this
// Will write to standard output if MuErrRep is True
{
  MultiSI AL(A.N);  AL.SetTo(A);
  MultiSI BL(B.N);  BL.SetTo(B);
  Bool Fini = False;
  AL.S = False;  BL.S = False;
  if (AL.ZTest()) { SetTo( BL);  Fini = True; }
  if (BL.ZTest()) { SetTo( AL);  Fini = True; }
  int NN = 0;
  if (MuErrRep && !Fini)  cout << nL;
  while (!Fini) {
    NN++;
    if (NN % 2) {
      AL.Modu( AL, BL);
      if (MuErrRep) {
	cout << NN << ' ';  AL.WritLn( cout);
      }
      if (AL.ZTest()) { SetTo( BL);  Fini = True; }
    } else {
      BL.Modu( BL, AL);
      if (MuErrRep) {
	cout << NN << ' ';  BL.WritLn( cout);
      }
      if (BL.ZTest()) { SetTo( AL);  Fini = True; }
    }
  }
} // --end-- // MultiSI::GCD()

// ------- this=Greatest common divisor A, this
void MultiSI::RGCD( MultiSI& A)
// A is not modified unless it shares memory with this
// Will write to standard output if MuErrRep is True
{
  GCD(A, *this);
} // --end-- // MultiSI::RGCD()

// ------- this = (X !) Mod MuMB = 1*2*3*...*X
void MultiSI::FacMB( MultiSI& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow or error
{
  double NN;
  if (X.S) {
    MuWriteErr("Cannot take factorial of number < zero");
    Clear();  return;
  }
  if (X.N > 2) {
    MuWriteErr("Number too large for factorial function");
    Clear();  return;
  }
  MultiSI XL(X.N);  XL.SetTo(X);
  SetTo1(1);
  XL.GetD( NN);
  while (NN > 1) {
    Mul( XL, *this);
    ModMB();
    if (MuAbort) {
      Clear();  return;
    }
    NN--;
    XL.SetToD( NN);
  }
} // --end-- // MultiSI::FacMB()

// ------- this = (this !) Mod MuMB = 1*2*3*...*X
void MultiSI::RFacMB()
// Sets MuErr True if overflow or error
{
  FacMB( *this);
} // --end-- // MultiSI::RFacMB()

// ------- this = SqRt(X), R = Rem, Slow Method (but fast for small numbers)
void MultiSI::SqRtRemSlow( MultiSI& X, MultiSI& R)
// X is not changed, X is moved to R and then R is "divided"
// in place to give the SqRt in this and a remainder in R, it is OK for X and R
// or X and this to have the same location in memory, but then X will be changed
// this and R may not have the same location in memory. Sets MuErr True if error
{
  Mu1Digit D0, QH;
  Mu2Digit T, T2, D4, K;
  long i, j, RI0, DI0, RI, DI, ij;
  if (V == NULL)  NMax( max( 1+X.N/2, 3));  // V and R.V may be NULL
  if (V == R.V) {
    MuWriteErr(
  "Cannot set same location to both square root and remainder, continuing...");
    return;
  }
  R.SetTo(X);
  Clear();
  if (R.S) {
    MuWriteErr("Cannot take square root of negative number, continuing...");
    R.Clear();  return;
  }
  if (M < 3)  NMax(3);
  if (R.M < 4)  R.NMax(4);
  if (R.N < 4) {
    for (i = R.N; i <= 3; i++)  R.V[i] = 0;
    R.N = 4;
  }
  if (R.N % 2) {
    R.V[R.N] = 0;
    R.N++;
  }
  N = R.N / 2 + 1;
  T = 0;
  for (i = 1; i <= 4; i++)  T = T * Base + R.V[R.N - i];
  T2 = floor( sqrt(T));
  do {
    D0 = floor( T2 / Base);
    QH = 1 + T2 - D0 * Base; // +1 for safety
    T = R.V[R.N - 1] * Base + R.V[R.N - 2] - D0 * D0;
    if (T < 0)  T2--;
  } while (T < 0);
  K = floor(T / Base);
  R.V[R.N - 2] = T - K * Base;
  R.V[R.N - 1] = K;
  if (N >= 4)  V[N - 4] = 0;
  V[N - 3] = QH;
  T = 2 * D0;
  K = floor(T / Base);
  V[N - 2] = T - K * Base;
  V[N - 1] = K;
  KeyHit = False;
  for (j = 2; j <= R.N / 2; j++) {
    K = Base;  RI0 = R.N - j - j;  DI0 = N - j - 1;
    RI = RI0;   DI = DI0;   ij = RI0 + j;
    while (RI <= ij) {	// Multiply and subtract
      T = R.V[RI] - QH * V[DI] + K + BSMB; // + BaseSq - Base
      K = floor(T / Base);
      T -= K * Base;
      R.V[RI] = T;
      RI++;  DI++;
    }
    T = R.V[RI] + K + BSMB; // + BaseSq - Base
    K = floor(T / Base);
    R.V[RI] = T - K * Base;
    while (K == MaxDigit) { // Test remainder
// WriteLn("MuSqRt: Doing an add back"); .Diag for add back test
      QH--;	// Add back
      K = QH;  RI = RI0;  DI = DI0;
      while (RI <= ij) {
	T = R.V[RI] + V[DI] + K;
	K = floor(T / Base);
	R.V[RI] = T - K * Base;
	RI++;  DI++;
      }
      T = R.V[RI] + K;
      K = floor(T / Base);
      R.V[RI] = T - K * Base;
      K += MaxDigit;
      V[DI0] = QH;
    }
    i = DI0;  ij = i + j;  K = QH;
    while (i <= ij) {
      T = V[i] + K;         // Carry to next super digit
      K = floor(T / Base);
      V[i] = T - K * Base;
      if (!K)  i = ij;	// Break out of loop
      i++;
    }
    if (j < R.N / 2) {
      T = 0;
      for (i = 0; i <= 4; i++) {
	T = T * Base + R.V[R.N - j - i];
      }
      D4 = 0;
      for (i = 1; i <= 4; i++) {
	D4 = D4 * Base + V[N - i];
      }
      QH = 1 + floor(T / D4);
      V[N - j - 2] = QH;
    }
    MuInterrupt();
    if (MuAbort) {
      Clear();  R.Clear();  return;
    }
    if (KeyHit) {
      cout << "Integer Square Root: "
	   << floor( 400.0 * j * j / R.N / R.N) << " Percent done" << nL;
      KeyHit = False;
    }
  }
  R.N = R.N / 2 + 1;
  R.Norm();
  Norm();
  IpDiv1( *this, 2, QH);
} // --end-- // MultiSI::SqRtRemSlow()

// ------- this = SqRt(X), R = Rem, Fast method for large numbers
void MultiSI::SqRtRemFast( MultiSI& X, MultiSI& R)
// X is not changed, X is moved to R and then R is "divided"
// in place to give the SqRt in this and a remainder in R, it is OK for X and R
// or X and this to have the same location in memory, but then X will be changed
// this and R may not have the same location in memory. Sets MuErr True if error
{
  MultiSI XL;  // Local X
  MultiSI YL;  // Local Y = SqRt(X)
  MultiSI Q;   // Q = XL / YL
  MultiSI RL;  // Local remainder
  MultiSI T;   // 2 * YL
  MultiSI D;   // RL / T
  MultiSI TD;  // T * D
  long    i, j, pl, po, ps, xs, ys, qs; //Scalings, X = XL * Base^xs
  Bool adjusted; // True iff an adjustment was needed

  pl = 5;
  ps = pl+pl+2;
  if (X.N % 2)  ps--; // ps = pl+pl+1
  if (X.N < ps || X.S) { SqRtRemSlow(X, R);  return; }
  XL.N = XL.M = ps;
  XL.V = X.V + (X.N-ps);
  YL.NMax( pl+2);
  RL.NMax( ps);
  YL.SqRtRemSlow( XL, RL);  RL.NMax(1);
  XL.V = NULL;
  ys = (X.N - ps) / 2;

  while (pl <= (X.N / 2)) { // y = (y + x / y) / 2
    po = pl;  pl += pl;
    if (pl > (X.N / 2))  pl = 1 + X.N / 2;
    j = pl+po+2;
    if (j > X.N)  j = X.N;
    XL.N = XL.M = j;
    XL.V = X.V + (X.N-j);
    xs = X.N - XL.N;
    Q.NMax( 1 + XL.N - YL.N);
    RL.NMax( XL.N);
    IpDiv( XL, YL, Q, RL);  XL.V = NULL;  RL.NMax(1);
    qs = xs - ys;
    if (ys > qs) {
      for (j = YL.N-1, i = j + ys-qs,  YL.NMax( i+2), YL.N = i+1;
       i >= 0;  i--, j--) {
	if (j >= 0)
      YL.V[i] = YL.V[j];
	else
      YL.V[i] = 0;
      }
      ys = qs;
    }
    if (ys < qs)
      FatalErr("ys < qs in MultiSI::SqRtRemFast()");
    YL.RAdd(Q);  Q.NMax(1);
    YL.RDiv1(2);
    if (ys < 0) {
      for (i = 0, j = -ys, YL.N -= j; i < YL.N;  i++, j++)  YL.V[i] = YL.V[j];
      ys = 0;
    }
  }

  XL.NMax( YL.N + YL.N);
  XL.Mul( YL, YL);
  RL.NMax( X.N + 1);
  RL.Sub( X, XL);  XL.NMax(1);
  T.NMax( YL.N + 1);
  T.Add( YL, YL);

  adjusted = False;
  while (RL.S || RL.Comp(T) > 0) {
    adjusted = True;
    if (RL.S) {
      if (T.N > RL.N)  { D.NMax(1);  D.Clear(); }
      else {
	RL.S = Plus;
	D.NMax(1 + RL.N - T.N);
	D.Divi( RL, T);
	RL.S = Minus;
      }
      D.RAdd1(1);
      T.RSub( D);
      TD.NMax( T.N + D.N);
      TD.Mul( T, D);
      RL.RAdd( TD);  TD.NMax(1);
      T.RSub( D);
    }

    else if (RL.Comp(T) > 0) {
      D.NMax( 1 + RL.N - T.N);
      D.Divi( RL, T);
      T.RAdd( D);
      TD.NMax( T.N + D.N);
      TD.Mul( T, D);
      RL.RSub( TD);  TD.NMax(1);
      T.RAdd( D);
    }
  }

  if (adjusted)  YL.Div1(T, 2);
  if (R.V == NULL)  R >>= RL;  // If R not allocated, swap with RL
    else            R <<= RL;  // else set to RL (copy)
  if (V == NULL)  *this >>= YL;
    else          *this <<= YL;
} // --end-- // MultiSI::SqRtRemFast()

// ------- this = SqRt(X), R = Rem
void MultiSI::SqRtRem( MultiSI& X, MultiSI& R)
// X is not changed, X is moved to R and then R is "divided"
// in place to give the SqRt in this and a remainder in R, it is OK for X and R
// or X and this to have the same location in memory, but then X will be changed
// this and R may not have the same location in memory. Sets MuErr True if error
{
  if (X.N <= 2024)
    SqRtRemSlow(X, R);
  else
    SqRtRemFast(X, R);
} // --end-- // MultiSI::SqRtRem()

// ------- this = SqRt( this), R = Rem
void MultiSI::RSqRtRem( MultiSI& R)
// this is moved to R and then R is "divided" in place to give the SqRt in this
// and a remainder in R, this and R may not have the same location in memory
// Sets MuErr True if error
{
  SqRtRem( *this, R);
} // --end-- // MultiSI::RSqRtRem()

// ------- this = SqRt(X)
void MultiSI::SqRoot( MultiSI& X)
// X is not changed, it is OK for X and this to have the same location in memory
// but then X will be changed.  Sets MuErr True if error
{
  MultiSI RL(X.N);
  SqRtRem(X, RL);
} // --end-- // MultiSI::SqRoot()

// ------- this = SqRt( this)
void MultiSI::RSqRt()
// Sets MuErr True if error
{
  SqRoot( *this);
} // --end-- // MultiSI::RSqRt()

// ------- this = Read MultiSI from a file
void MultiSI::ReadSI( FILE* R, char* Name, Bool& OK)
// Returns Name and OK != 0 if read is OK
// MuErr set True if overflow
{
  char ch;
  long Li, Lj, Len = 0;
  int j = 0;
  Bool S = False; // Boolean
  Bool OverFlow = False; // Boolean
  Mu1Digit Dig, SDig = 0;
  Clear();  N = 0;  Name[0] = NULL;  OK = False;  TracN = 0;
  do {
    ch = fgetc(R);
    if (ch == '=')  break;
    if (ch == '\n') { j = 0;  Name[0] = NULL; }
    else
      Name[j++] = ch;
  } while (ch != EOF);
  Name[j] = NULL;
  if (ch == EOF)  return;
  do {
    ch = fgetc(R);
    if ((ch == ' ') || (ch == '\n') || (ch == '\t'))  ;
    else if (ch == '-')  { S = True;  break; }
    else if (ch == '+')  break;
    else if ((ch >= '0') && (ch <= '9')) {
      ungetc( ch, R);  break;
    }
  } while (ch != EOF);
  if (ch == EOF)  return;
  do {
    ch = fgetc(R);
    if ((ch == ',') || (ch == '\n'))  ;
    else if ((ch >= '0') && (ch <= '9')) {
      OK = True;
      TracN++;  Trace = ++Len;
      Dig = (ch - '0');
      SDig = 10 * SDig + Dig;
      if (!(Len % MuDMax)) {
	N++;
	if ((Li = M - N) < 0) // HJS 1992-12-13 (was <= 0)
	  { OverFlow = True;  N = M;  Li = 0;  SDig = 0;  break; }
	V[Li] = SDig;  SDig = 0;
      }
      MuInterrupt();  if (MuAbort)  break;
    }
    else  break;
  } while (ch != EOF);
  for (Lj = 0;  Lj < N;  Li++, Lj++)  V[Lj] = V[Li];
  for (j = (int)(Len % MuDMax), Dig = 1;  j > 0;  j--)  Dig = Dig * 10;
  RMul1( Dig);  RAdd1( SDig);
  if (S)  ChS();
  if (OverFlow)  MuWriteErr("Input number overflow, continuing...");
  Norm();
} // --end-- MultiSI::ReadSI()

// --end-- MultiSI's method implementation

// --end-- Method implementation

// ------- Other services

// ------- Check far heap  // Heap check not supported by MS Windows
Bool HeapOk() {
//  if( farheapcheck() == _HEAPCORRUPT ) ???
//    { cout << "Heap is corrupted.\n";  return False; }
//  else
//    { cout << "Heap is OK.\n";  return True; }
  return True;
} // --end-- HeapOk()

// ------- Diag for a MultiSI
void DiagSI( char* St, MultiSI& X)
{
  if (LogF.good()) {
    if (MuErr)  LogF << "MuErr = True\n";
    LogF << St << " (" << X.M << ", " << X.N << ") =\n" << X << dL; MuTot = 0;
    LogF << flush;
  }
} // --end-- DiagSI

// ------- Fatal error handler
//           from Numerical Recipes standard error handler (see page 705)
void FatalErr( char error_text[])
{
  fprintf( stderr, "Fatal run-time error...\n");
  fprintf( stderr, "%s\n", error_text);
  fprintf( stderr, "...now exiting to system...\n");
  exit(1);
} // --end-- FatalErr()

// ------- overloaded sqrt function for MultiSI
MultiSI sqrt( MultiSI& X)  // Could not be done inline, gives warning
{ MultiSI T, Rem;  T.SqRtRem(X, Rem);  ProtV = T.V;  return T; }

// ------- Allocates a bytes8 vector with range [nl..nh] (see page 705)
bytes8 huge *vector8( long nl,  long nh)
{
  bytes8 huge *v;
  v = (bytes8 huge *) farcalloc( (nh-nl+1), sizeof( bytes8));
  if (!v)  return v;
  if (kbhit()) {
    MuInterrupt();  KeyHit = False;
    cout << "vector8: Allocated array[" << nl << ".." << nh << ']' << nL;
  }
  v = v-nl;
  return v;
} // --end-- vector8()

// ------- Frees a bytes8 vector allocated by vector8() (see page 707)
void free_vector8( bytes8 huge *v,  long nl,  long nh)
{
  bytes8 huge *vl;
  vl = v+nl;
  if (v) farfree( vl);  nh;
} // --end-- free_vector8()

// ------- FFT of complex type data (see page 411)
void four1( bytes8 huge *data,  long nn,  Bool isign)
/*
   Replace data by its discrete Fourier transform, if isign is input as 1;
   or replace data by nn times its inverse discrete Fourier transform, if
   isign is input as -1.  data is a complex array of length nn, input as a
   real array data[1..2*nn].  nn must be an integer power of 2 (this is not
   checked for!).
*/
{
  #define SWAP(a, b)  tempr = (a);  (a) = (b);  (b) = tempr;
  long n, mmax, m, j, istep, i;
			   // Double precision for trigonometric recurrences.
  RealPlus wtemp, wr, wpr, wpi, wi, theta;
  Real tempr, tempi;

  n = nn << 1;
  j = 1;
  for (i = 1; i < n; i += 2) {
    if (j > i) {          // This is the bit-reversal section of the routine.
      SWAP( data[j].v,   data[i].v);     // Exchange the two complex numbers.
      SWAP( data[j+1].v, data[i+1].v);
    }
    m = n >> 1;
    while (m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;

    if (!((i+1) % 1000)) {
      if (kbhit()) {
	MuInterrupt();  KeyHit = False;
	if (MuAbort)  return;
	cout << "four1: bit-reversal section, i = " << (i+1) << " of " << n
	     << nL;
      }
    }
  }
  mmax = 2;      // Here begins the Danielson-Lanczos section of the routine.
  while (mmax < n) {                    // Outer loop executed log2 nn times.
    istep = 2 * mmax;
    theta = Pi2 / (isign * mmax);//Initialize for the trigonometric recurrence.
    wtemp = sinl( 0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sinl( theta);
    wr = 1.0;
    wi = 0.0;
    for (m = 1; m < mmax; m += 2) { // Here are the two nested inner loops.
      for (i = m; i <= n; i += istep) {
	j = i + mmax;               // This is the Danielson-Lanczos formula.
	tempr = wr * data[j].v   - wi * data[j+1].v;
	tempi = wr * data[j+1].v + wi * data[j].v;
	data[j].v   = data[i].v   - tempr;
	data[j+1].v = data[i+1].v - tempi;
	data[i].v   += tempr;
	data[i+1].v += tempi;
      }                                          // Trigonometric recurrence.
      wr = (wtemp = wr) * wpr - wi * wpi + wr;
      wi = wi * wpr + wtemp * wpi + wi;
    }
    mmax = istep;

    if (kbhit()) {
      MuInterrupt();  KeyHit = False;
      if (MuAbort)  return;
      cout << "four1: istep = " << istep << " of " << n << nL;
    }
  }
} // --end-- four1()

// ------- Two FFTs of real data for the price of one (see page 415)
void twofft( bytes8 huge *data1,  bytes8 huge *data2,  bytes8 huge *fft1,
	     bytes8 huge *fft2,   long n)
/*
   Given two input arrays data1[1..n] and data2[1..n], this routine calls
   four1 and returns two complex output arrays, fft1 and fft2, each of
   complex length n (i.e. real dimensions [1..2n]), which contains the
   discrete Fourier transform of the respective data.  n MUST be an integer
   power of 2.
*/
{
  long nn3, nn2, jj, j;
  Real rep, rem, aip, aim;

  nn3 = 1 + (nn2 = 2 + n + n);
  for (j= 1, jj = 2;  j <= n;  j++, jj += 2) {
    fft1[jj-1].v = data1[j].v; //Pack the 2 real arrays into 1 complex array.
    fft1[jj  ].v = data2[j].v;

    if (!(j % 1000)) {
      if (kbhit()) {
	MuInterrupt();  KeyHit = False;
	if (MuAbort)  return;
	cout << "twoft: Pack the 2 real arrays into 1 complex array, j = "
	     << j << " of " << n << nL;
      }
    }
  }
  four1( fft1, n, 1);      // Transform the complex array
  if (MuAbort)  return;
  fft2[1].v = fft1[2].v;
  fft1[2].v = fft2[2].v = 0.0;
  for (j = 3; j <= n+1; j += 2) {
    rep = 0.5 * (fft1[j].v   + fft1[ nn2 - j].v);//Use symmetries to separate
    rem = 0.5 * (fft1[j].v   - fft1[ nn2 - j].v);//  the two transforms.
    aip = 0.5 * (fft1[j+1].v + fft1[ nn3 - j].v);
    aim = 0.5 * (fft1[j+1].v - fft1[ nn3 - j].v);
    fft1[j].v     = rep;
    fft1[j+1].v   = aim;
    fft1[nn2-j].v = rep;
    fft1[nn3-j].v = -aim;
    fft2[j].v     = aip;
    fft2[j+1].v   = -rem;
    fft2[nn2-j].v = aip;
    fft2[nn3-j].v = rem;

    if (!((j+1) % 1000)) {
      if (kbhit()) {
	MuInterrupt();  KeyHit = False;
	if (MuAbort)  return;
	cout << "twoft: Separate the two transforms, j = "
	     << (j+1) << " of " << n << nL;
      }
    }
  }
} // --end-- twofft()

// ------- FFT of real data (see page 417)
void realft( bytes8 huge *data,  long n,  Bool isign)
/*
   Calculates the Fourier Transform of a set of 2n real-valued data points.
   Replaces this data (which is stored in array data[1..2n]) by the positive
   frequency half of its complex Fourier Transform.  The real-valued first
   and last components of the complex transform are returned as elements
   data[1] and data[2] respectively.  n must be a power of 2.  This routine
   also calculates the inverse transform of a complex data array if it is the
   transform of real data.  (Result in this case must be multiplied by 1/n.)
*/
{
  long i, i1, i2, i3, i4, n2p3;
  Real c1 = 0.5, c2, h1r, h1i, h2r, h2i;
  RealPlus wr, wi, wpr, wpi, wtemp, theta; // Extra precision for
					   //   trigonometric recurrences.
  theta = Pi / (double) n;                 // Initialize the recurrence.
  if (isign == 1) {
    c2 = -0.5;
    four1( data, n, 1);                  // The forward transform is here
    if (MuAbort)  return;
  } else {
    c2 = 0.5;                      // Otherwise set up the inverse transform.
    theta = -theta;
  }
  wtemp = sinl( 0.5 * theta);
  wpr = -2.0 * wtemp * wtemp;
  wpi = sinl( theta);
  wr = 1.0 + wpr;
  wi = wpi;
  n2p3 = 2 * n + 3;
  for (i = 2; i <= n / 2; i++) {         // Case i = 1 done separately below.
    i4 = 1 + (i3 = n2p3 - (i2 = 1 + (i1 = i + i - 1)));
    h1r = c1 * (data[i1].v + data[i3].v);  // The two separate transforms are
    h1i = c1 * (data[i2].v - data[i4].v);  //   separated out of data.
    h2r =-c2 * (data[i2].v + data[i4].v);
    h2i = c2 * (data[i1].v - data[i3].v);
    data[i1].v = h1r + wr * h2r - wi * h2i;//Here they are recombined to form
    data[i2].v = h1i + wr * h2i + wi * h2r;//  the true transform of the
    data[i3].v = h1r - wr * h2r + wi * h2i;//  original real data.
    data[i4].v =-h1i + wr * h2i + wi * h2r;
    wr = (wtemp = wr) * wpr - wi * wpi + wr;   // The recurrence.
    wi = wi * wpr + wtemp * wpi + wi;

    if (!(i % 1000)) {
      if (kbhit()) {
	MuInterrupt();  KeyHit = False;
	if (MuAbort)  return;
	cout << "realft: separate and recombine transforms, i = "
	    << i << " of " << n/2 << nL;
      }
    }
  }
  if (isign == 1) {
    data[1].v = (h1r = data[1].v) + data[2].v; // Squeeze the first and last data
    data[2].v =  h1r - data[2].v;        //   together to get them all within
					 //   the original array.
  } else {
    data[1].v = c1 * ((h1r = data[1].v) + data[2].v);
    data[2].v = c1 *  (h1r - data[2].v);
    four1( data, n, -1);                 // This is the inverse transform for
					 //   the case isign = -1.
    if (MuAbort)  return;
  }
} // --end-- realft()

// ------- Convolves or deconvolves a real data set (see page 430)
void convlv( bytes8 huge *data,    long n,
	     bytes8 huge *respns,  long m,
	     Bool isign,           bytes8 huge *ans,
	     bytes8 huge *fft)
/*
   Convolves or deconvolves a real data set data[1..n] (including any user-
   supplied zero padding) with a response function respns[1..n].  The
   response function must be stored in wrap around order in the first m
   elements of respns, where m is an odd integer <= n.  Wrap around order
   means that the first half of the array respns contains the impulse
   response function at positive times, while the second half of the array
   contains the impulse response function at negative times, counting down
   from the highest element respns[m].  On input isign is +1 for convolution,
   -1 for deconvolution.  The answer is returned in the first n components of
   ans.  However, ans must be supplied in the calling program with dimensions
   [1..2*n], for consistency with twofft.  n MUST be an integer power of two.
   Note: if (m <= 0) respns is assumed to have user supplied zero padding.
     Must allocate fft[1..2n] for temp storage before calling:
       fft = vector8(1, 2 * n);
     data[] and respns[] can be overlaid with ans[] by allocating them
     as the same memory locations:
	ans    = vector8(1, 2 * n);
	fft    = vector8(1, 2 * n);
	data   = ans;
	respns = ans + n;
*/
{
  #define SQR(a)	(sqrarg=(a), sqrarg*sqrarg)
  static Real sqrarg;
  long i, no2;
  Real dum, mag2;

  if (m > 0) {
    for (i = 1; i <= (m-1) / 2; i++)     // Put respns in array of length n.
      respns[n + 1 - i].v = respns[m + 1 - i].v;
    for (i = (m + 3) / 2; i <= n - (m - 1) / 2; i++)    // Pad with zeros.
      respns[i].v = 0.0;
  }
  twofft( data, respns, fft, ans, n);    // FFT both at once
  if (MuAbort)  return;
  no2 = n / 2;
  for (i = 2; i <= n + 2; i += 2) {
    if (isign == 1) {                    // Multiply FFTs to convolve.
      ans[i-1].v = (fft[i-1].v * (dum=ans[i-1].v) - fft[i].v * ans[i].v)/no2;
      ans[i].v = (fft[i].v * dum + fft[i-1].v * ans[i].v) / no2;
    }
    else if (isign == -1) {
      if ((mag2 = SQR( ans[i-1].v) + SQR( ans[i].v)) == 0.0)
	FatalErr("Deconvolving at response zero in CONVLV");
					 // Divide FFTs to deconvolve.
      ans[i-1].v = (fft[i-1].v*(dum=ans[i-1].v) + fft[i].v*ans[i].v)/mag2/no2;
      ans[i].v = (fft[i].v * dum - fft[i-1].v * ans[i].v) / mag2 / no2;
    }
    else FatalErr("No meaning for ISIGN in CONVLV");

    if (!(i % 1000)) {
      if (kbhit()) {
	MuInterrupt();  KeyHit = False;
	if (MuAbort)  return;
	cout << "convov: Multiply FFTs to convolve, i = "
	     << i << " of " << n << nL;
      }
    }
  }
  ans[2].v = ans[n+1].v;          // Pack last element with first for realft.
  realft( ans, no2, -1);          // Inverse transform back to time domain.
  if (MuAbort)  return;
} // --end-- convlv()

// ------- MuWriteErr, write error statement and set error flag
void MuWriteErr( char *St)
// Allows St to be a literal string
{
  MuErr = True;
  if (MuErrRep) {
    cout << St << nL;
    if (Echo)
      LogF << St << nL;
  }
} // --end-- MuWriteErr

// ------- Read in string from an istream, allow a blank line
void ReadSt( istream& In, char *St)
{
  In.get( St, 81);
  if (In.peek() == '\n')  In.get(); // Clear '\n' from input
} // --end-- // ReadSt istream

// ------- Read in string from the key board, allow a blank line
void ReadSt( char *St)
{
  cin.get( St, 81);
  if (cin.peek() == '\n')  cin.get(); // Clear '\n' from input
} // --end-- // ReadSt

// ------- Read in a long integer from keyboard
void ReadLInt( char *Mess, long Min, long Max, long Nom,
	       long& II)
{
  char St[81];
  int  Stat;
  long LI;
  do {
    cout << Mess << nL;
    cout << "  [" << Min << ", " << Max << "] (ENTER => " << Nom << "): ";
    ReadSt( St);
    Stat = sscanf( St, "%ld", &LI);
  } while (((Stat != 1) || (LI < Min) || (LI > Max)) && (St[0] != '\0'));
  if (St[0] == '\0')  LI = Nom;
  II = LI;
  cout << "Input = " << II << nL;
} // --end-- // ReadLInt

// ------- // Read in an integer from keyboard
void ReadInt( char *Mess, int Min, int Max, int Nom, int& II)
{
  char St[81];
  int Stat;
  long LI;
  do { 
    cout << Mess << nL;
    cout << "  [" << Min << ", " << Max << "] (ENTER => " << Nom << "): ";
    ReadSt( St);
    Stat = sscanf( St, "%ld", &LI);
  } while (((Stat != 1) || (LI < Min) || (LI > Max)) && (St[0] != '\0'));
  if (St[0] == '\0')  LI = Nom;
  II = (int)LI;
  cout << "Input = " << II << nL;
} // --end-- // ReadInt

// ------- Set MMax
void MuSetMMax( int NRegs)
{
  int Percent;
  long Max = 300000L; // Arbitrary number of max bytes to be used
  cout << "MemAvail = " << Max << " bytes" << nL;
  if (Echo)  LogF << "MemAvail = " << Max << " bytes" << nL;
  ReadInt("Input % of available memory to use:", 0, 100, 100, Percent);
  Max = ((Max - 256) * Percent) / (100 * NRegs); // Max per register
  MMax = (Max - sizeof( MultiSI)) / sizeof( Mu1Digit) - 1;
  if (MMax > MuNMax)  MMax = MuNMax;
  if (MMax < 5)  MMax = 5;
} // --end-- MuSetMMax

// ------- Test for ESC key pressed to abort operation
void MuInterrupt()
// MuAbort set TRUE if ESC pressed twice, unchanged if no ESC or ESC, SPACE
{
  char Esc = 27; // ESCAPE
  char Ret = 13; // RETURN
  char Ch;
  if (!kbhit())  return;
  while (kbhit())  Ch = getch();
  KeyHit = True;  KeyCh = Ch;  DosClock();
  if (Trace > 0) {
    cout << '<' << TracN << ' ' << Trace << '>' << nL;
    Trace = 0;
  }
  if ((Ch != Esc) || MuAbort)  return;
  cout << "*** INTERRUPT:  To Continue Press RETURN Key;\n";
  cout << "To Abort Computation Press ESCAPE Key;\n";
  cout << "To Set SoftAbort Flag Press SPACE Bar.\n\n";
  do {Ch = getch();} while ((Ch != Esc) && (Ch != Ret) && (Ch != ' '));
  if (Ch == Ret)  cout << "Continuing...\n";
  if (Ch == Esc) {
    MuAbort = True;
    cout << "Computation aborted by operator!\n\n";
  }
  if (Ch == ' ') {
    SoftAbort = True;
    cout << "SoftAbort flag set by operator!\n\n";
  }
} // --end-- MuInterrupt()

// ------- Get the value of the Dos clock in total seconds
double DosClock() {
// Only good to 0.01 seconds, must call earlier once each day
  double DC; // Local value of DosClock
  struct  time t;
  gettime( &t);
  DC = DosDays * 86400.0 + t.ti_hour * 3600.0
       + t.ti_min * 60.0 + t.ti_sec + t.ti_hund / 100.0;
  if (DC < DosClockP) {
    DosDays++;  DC += 86400.0;
  }
  DosClockP = DC;
  return DC;
} // --end-- DosClock

// ------- Save start time of a "Time-Out" period not to be timed
void TimeOut()
{
  if (DiagOn)  TV3 = DosClock();
} // --end-- // TimeOut

// ------- Adjust TV1 time at end of a "Time-Out" period not to be timed
void TimeIn()
{
  if (DiagOn)  TV2 += DosClock() - TV3;
} // --end-- // TimeIn

// ------- Output a diagnostic message with delta times
void Diag( char *Mess)
{
  if (DiagOn) {
    TV1 = TV2;  TV2 = DosClock();
    Del0 = TV2 - TV0;
    Del1 = TV2 - TV1;
    cout << "T = " << Del0 << "  DT = " << Del1 << " sec.";
    if (Mess[0])  cout << "  " << Mess;
    cout << nL;
    DiagL( LogF, Mess);
  }
} // --end-- // Diag

// ------- Log output of diagnostic message with delta times
void DiagL( ostream& LogF, char *Mess)
{
  if (DiagOn && LogF.good()) {
    LogF << "T = " << Del0 << "  DT = " << Del1 << " sec.";
    if (Mess[0])  LogF << "  " << Mess;
    LogF << nL;
  }
} // --end-- // DiagL

// ------- overloaded + infix add operator for MultiSI
MultiSI MultiSI::operator+ (MultiSI& X) {
  MultiSI T(1 + max( this->N, X.N)); // Temp variable
  T.Add( *this, X);
  ProtV = T.V; // Prevent T.V from being freed
  return T;
}
// & not allowed on return type because got error: attempting
//   to return a reference to a variable 'T'

// ------- overloaded - infix sub operator for MultiSI
MultiSI MultiSI::operator- (MultiSI& X) {
  MultiSI T(1 + max( this->N, X.N));
  T.Sub( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded * infix mul operator for MultiSI
MultiSI MultiSI::operator* (MultiSI& X) {
  MultiSI T( this->N + X.N);
  T.Mul( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded / infix div operator for MultiSI
MultiSI MultiSI::operator/ (MultiSI& X) {
  MultiSI T( this->N);
  T.Divi( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded % infix mod operator for MultiSI
MultiSI MultiSI::operator% (MultiSI& X) {
  MultiSI T(X.N);
  T.Modu( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded ^ infix xor operator, -> power
  MultiSI MultiSI::operator^ (MultiSI& X) {
  long n;

  if (~MuMB)  n = MuMB.N;
  else {
    MultiSI NN(9), Max(2);
    NN <<= this->N;
    NN <<= NN * X;
    Max <<= MMax;
    if (NN >= Max)  n = MMax;
    else
      n <<= NN;
  }
  MultiSI T(n);
  T.PowMB( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded ^ infix xor operator, -> power
  MultiSI MultiSI::operator^ (double p) {
  MultiSI X(2);
  double d;
  long n;

  X.SetToD(p);
  if (~MuMB)  n = MuMB.N;
  else {
    d = this->N * p;
    if (d >= MMax)  n = MMax;
    else  n = (long) d;
  }
  MultiSI T(n);
  T.PowMB( *this, X);
  ProtV = T.V;  return T;
}

// ------- overloaded - unary minus operator for MultiSI
MultiSI MultiSI::operator- () {
  MultiSI T( this->N);
  T.SetTo( *this);  T.ChS();
  ProtV = T.V;  return T;
}

// ------- Output Date and time output stream
void OutDateTime(ostream& Out) {
  Out.fill('0');
  Out << setw(4) << da.da_year << '-'
      << setw(2) << (int)da.da_mon << '-'
      << setw(2) << (int)da.da_day << ' '
      << setw(2) << (int)ti.ti_hour << ':'
      << setw(2) << (int)ti.ti_min << ':'
      << setw(2) << (int)ti.ti_sec << '.'
      << setw(2) << (int)ti.ti_hund << nL;
} // --end-- OutDateTime

// ------- Get Date and time
void GetDateTime() {
  gettime(&ti);  getdate(&da);
} // --end-- GetDateTime

// ------- Initialize Multi-precision package
void InitMulti() {
// Input: MuDMax, MuNMax
// Output: Base, BaseSq, MaxDigit, BSMB, Pi, Pi2
// Initialize: MuDpG=5, MuLenD=21, MuErr=False, MuErrRep=True, MuAbort=False,
// Echo=0, MMax=MuNMax,  TracN=0, Trace=0, DiagOn=False, TV0=TV1=TV2=DosClock,
// MuDOnly=False
  GetDateTime();
  Base = 10;
  for (int i = 2; i <= MuDMax; i++)  Base *= 10;
  MaxDigit = Base - 1;
  BaseSq   = Base * Base;
  BSMB     = BaseSq - Base;
  Pi       = atan2l(0.0, -1.0);
  Pi2      = Pi + Pi;
  MuDpG    = 5;
  MuDOnly  = False;
  MuLenD   = 21;
  MuErr    = False;
  MuErrRep = True;
  MuAbort  = False;
  SoftAbort= False;
  KeyHit   = False;
  KeyCh    = ' ';
  Echo     = 0;
  MMax     = MuNMax;
  TracN    = 0;
  Trace    = 0;
  ShortN   = 0;
  DiagOn   = False;
  DosClockP= 86400.0;
  DosDays  = 0;
  MuMaxW   = 65;
  MuTot    = 0;
  ProtV    = NULL;
  Disk.close();
  LogF.close();
  TV3 = TV2 = TV1 = TV0 = DosClock();
//cout.setf( 0x1000 /*fixed*/);  cout.precision( 20); ???
//  ostream& Out = cout; // This worked
//  OutP[0]  = cout;  // OutP[i] << i << nL;  // a.WritLn( OutP[i]);
//  OutP[1]  = cLog;  // This did not work:
//  Error 'ios::operator =(ios near&)' is not accessible in function
//     ostream::operator =(ostream near&)
} // --end-- InitMulti Initialize Multi-precision package

// --end-- Other services

// Revisions made -
// Fixed an error in ReadSI: 
//	if ((Li = M - N) < 0) // HJS 1992-12-13 (was <= 0)
//	  { OverFlow = True;  N = M;  Li = 0;  SDig = 0;  break; }

// --end-- MultiIDW.Cpp Multiple-precision integer decimal algorithms
