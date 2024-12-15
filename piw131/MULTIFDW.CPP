// ------- MultiFDW.Cpp  Multiple-precision floating decimal algorithms
/*
  Version : Version 3.11, last revised: 1994-07-29, 0600 hours
  Author  : Copyright (c) 1981-1994 by author: Harry J. Smith,
  Address : 19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.
*/

#include "MultiFDW.h" // Multiple-precision Floating decimal algorithms module
#include <limits.h>

// ------- Convert +Double to a String integer ?????? is this needed ??????
void StrDI(char* St, double D);

// Developed in Turbo Pascal 5.5.  Converted to Borland C++ 3.1 for Windows.

// ------- The following are set when InitMultiF is called
Mu1Digit FRound;   // Floating point rounding constant, Base / 2 or 0
long     FMC;      // Current max # of super digits in a result
long     FTn;      // Current # of super digits to truncate display
long     FDn;      // Current max decimal digits to display
Bool     ScieN;    // Force scientific notation on flag
Bool     Expanded; // True if running in expanded precision
long     FMCB;     // Base FMC when running expanded precision

// ------- Implementation

// ------- Common hidden routines
//         none
// ------- End of common hidden routines

// ------- MultiF's method implementation

// ------- this = X
void MultiF::SetTo( MultiF& X)
{
  long   I, J, K;
  double D;
  Bool   RoundIt;
  long   L;

  K = MinL(M, FMC);
  if (K >= X.N) {
    MultiSI::SetTo(X);
    C = X.C;  return;
  }
  N = K;  S = X.S;
  J = X.N - K;
  for ( I = 0; I < K; I++)  V[I] = X.V[I + J];
  D = X.C + J;
  if (D > LONG_MAX) {
    Clear();  MuWriteErr("Floating SetTo overflow, continuing...");
  }
  else {
    C = X.C + J;
    if ((FRound > 0) && (X.V[J - 1] >= FRound)) {
      RoundIt = True;
      if ((X.V[J - 1] == FRound) && (J == 1)) {
	L = X.V[J];
	if (!(L & 1))  RoundIt = False; // if not odd
      }
      if (RoundIt) {
        MultiF T(1);
	T.V[0] = 1;
	T.S = S;
	T.C = C;
        RAdd(T);
      }
    }
    Norm();
  }
} // --end-- MultiF::SetTo

// ------- Normalize
void MultiF::Norm()
{
  long     I, J;
  Mu1Digit SaveFR;
  long     SaveFMC;
  double   D;
  long     L;
  Bool     RoundIt;

  MultiSI::Norm();
  if (ZTest()) { C = 0;  return; }	
  I = 0;
  if (N > FMC) {
    I = N - FMC;
    D = C + I;
    if (D > LONG_MAX) {
      MuWriteErr("Floating Norm overflow, continuing...");
      Clear();  return;
    }
    if ((FRound > 0) && (V[I - 1] >= FRound)) {
      RoundIt = True;
      if (V[I - 1] == FRound) {
	L = V[I];
	if (!(L & 1)) { // if not odd
	  RoundIt = False;
	  J = I - 2;
	  while (J >= 0) {
	    if (V[J] != 0) {
	      J = 0;  RoundIt = True;
	    }
            J--;
	  }
	}
      }
      if (RoundIt) {
	SaveFR = FRound;
	FRound = 0;
	SaveFMC = FMC;
	FMC = M;
	MultiF T(1);
	T.V[0] = 1;
	T.S = S;
	T.C = C + I;
	if (V[0] == 0)  V[0] = 1; // Prevent Normalizing early
	RAdd(T);
	FRound = SaveFR;
	FMC = SaveFMC;
      }
    }
    N -= I;  C += I;
  }
  while (V[I] == 0) {
    I++;  // I = # of trailing zero super digits
    N = N - 1;
    D = C + 1.0;
    if (D > LONG_MAX) {
      MuWriteErr("Floating Norm overflow, continuing...");
      Clear();  return;
    }
    C = C + 1;
  }
  if (I > 0) {
    for (J = 0; J < N; J++) {
      V[J] = V[I];
      I++;
    }
  }
} // --end-- MultiF::Norm

// ------- this = this, R "digits" lost, unnormalized
void MultiF::ShiftR( long R)
// R < 0 => Shift left, Sets MuErr True if overflow
{
  long I;

  if (R == 0)  return;
  if (R  < 0) { ShiftL(-R);  return; }
  if (R >= N) { Clear();  return; }
  if ((FRound > 0) && (!V[R] || (V[R] == FRound))) {
    I = 0;
    while (I < R) {
      if (V[I] != 0) {
	I = R;  V[R] = V[R] + 1; // Add sticky bit to meet IEEE standard
      }
      I++;
    }
  }
  for (I = R; I < N; I++)  V[I - R] = V[I];
  N = N - R;
  if (C > (LONG_MAX - R)) {
    Clear();  MuWriteErr("Floating shift right underflow, continuing...");
  }
  else
    C = C + R;
} // --end-- MultiF::ShiftR

// ------- this = this,L "digits" more, unnormalized
void MultiF::ShiftL( long L)
// R < 0 => Shift right, Sets MuErr True if overflow
{
  long I;

  if (L == 0)  return;
  if (L < 0) { ShiftR(-L);  return; }
  if ((N + L) > M) {
    MuWriteErr("Floating shift left overflow, continuing...");
    Clear();  return;
  }
  for (I = (N - 1); I >= 0; I--)  V[I + L] = V[I];
  for (I = 0; I < L; I++)  V[I] = 0;
  N = N + L;
  if (C < (L - LONG_MAX))  Clear();
  else
    C = C - L;
} // --end-- MultiF::ShiftL

// ------- this = this + X
void MultiF::RAdd( MultiF& X)
// Sets MuErr True if overflow
{
  Add( *this, X);
} // --end-- MultiF::RAdd

// ------- this = this - X
void MultiF::RSub( MultiF& X)
// Sets MuErr True if overflow
{
  if (V == X.V)
    Clear();
  else {
    X.S = !X.S;
    RAdd(X);
    X.S = !X.S;
  }
} // --end-- MultiF::RSub

// ------- this = X + Y
void MultiF::Add( MultiF& X, MultiF& Y)
// Sets MuErr True if overflow
{
  double   XF,  YF,  TF; // Length of mantissa
  MultiF  *XL, *YL, *TL; // Local pointers
  long    MaxS;

  if (X.ZTest()) { SetTo(Y);  return; }
  if (Y.ZTest()) { SetTo(X);  return; }
  XL = &X;  XF = (double)(X.C) + X.N;
  YL = &Y;  YF = (double)(Y.C) + Y.N;
  if (YF > XF) {
    TL = XL;  XL = YL;  YL = TL;
    TF = XF;  XF = YF;  YF = TF;
  }
  if ((XF - YF - 3) >= M)  SetTo( *XL);
  else {
    XF = XF - MinL( X.C, Y.C);
    MaxS = MinD( XF, M) + 1 + 3; // 3 guard + 1 overflow "Digits"
    MultiF XT( MaxS);  XT.SetTo( *XL);
    MultiF YT( MaxS);  YT.SetTo( *YL); // XT, YT are Temp on heap
    XT.ShiftL( XT.M - XT.N - 1);
    YT.ShiftR( XT.C - YT.C);
    XT.MultiSI::RAdd( YT);
    XT.Norm();
    SetTo( XT);
  }
} // --end-- MultiF::Add

// ------- this = X - Y
void MultiF::Sub( MultiF& X, MultiF& Y)
// Sets MuErr True if overflow
{
  if (&X == &Y)  Clear();
  else if (V == X.V) 
    RSub(Y);
  else if (V == Y.V) {
    RSub(X);  ChS();
  }
  else {
    Y.S = !Y.S;
    Add(X, Y);
    Y.S = !Y.S;
  }
} // --end-- MultiF::Sub

// -------
void MultiF::Value( char* St, int& I)
// Convert String to this, returns I = # of character used
// Allows St to be a literal, MuErr set True if overflow
{
  int      J, K, L;
  Mu1Digit D1;
  double   Db, DI;
  Bool     Sign;

  Sign = (St[0] == '-');
  MultiSI::Value( St, J);
  I= J;
  C = 0;
  Norm();
  if (J) strcpy( St, St+J); // Delete first J characters
  if (strlen( St) == 0)  return;
  if ((strlen( St) >= 2) && (St[0] == '.') &&
	 (St[1] >= '0') && (St[1] <= '9')) {
    strcpy( St, St+1); // Delete first character
    MultiF X(1 + strlen( St) / MuDMax); // Temp on heap
    X.MultiSI::Value( St, J);
    I += J + 1;
    K = 0;
    for (L = 0; L < J; L++)  if ((St[L] >= '0') && (St[L] <= '9'))  K++;
    X.C = -((K - 1) / MuDMax) - 1;
    K = MuDMax - ((K - 1) % MuDMax) - 1;
    D1 = 1;
    for (L = 0; L < K; L++)  D1 *= 10;
    X.MultiSI::RMul1( D1);
    X.S = Sign;
    RAdd(X);
    if (J) strcpy( St, St+J); // Delete first J characters
  }
  if (strlen( St) < 2)  return;

  if ( ((St[0] == 'E') || (St[0] == 'e')) &&    (
	 ((St[1] >= '0') && (St[1] <= '9')) ||
	 ((strlen(St) >= 3) &&
	  ((St[1] == '+') || (St[1] == '-')) &&
	  (St[2] >= '0') && (St[2] <= '9'))     ) ) {

    strcpy( St, St+1); // Delete first character
    MultiF X(1 + strlen( St) / MuDMax); // Temp on heap
    X.MultiSI::Value( St, J);
    I += J + 1;
    X.MultiSI::GetD( Db);
    modf( Db / MuDMax, &DI); // DI = Integer part
    if (Db < 0.0)  DI--;
    if ((DI + C) > LONG_MAX) {
      MuWriteErr("Floating Value overflow, continuing...");
      Clear();  return;
    }
    if ((DI + C) < -LONG_MAX) {
      Clear();  return;
    }
    C = DI + C;
    K = Db - DI * MuDMax;
    D1 = 1;
    for (L = 1; L <= K; L++)  D1 *= 10;
    RMul1( D1);
  }
  Norm();
} // --end-- MultiF::Value

// ------- this = D1 Mod Base
void MultiF::SetTo1( Mu1Digit D1)
// Allows a literal D1, D1 an integer, |D1| < Base, D1 not changed
{
  MultiSI::SetTo1( D1);
  C = 0;
} // --end-- MultiF::SetTo1

// ------- this = Db Mod Base**4
void MultiF::SetToD( double Db)
// Allows a literal Db, Db not changed, MuErr set True if overflow
{
  MultiSI::SetToD( Db);
  C = 0;
  Norm();
} // --end-- MultiF::SetToD

// ------- D1 = this Mod Base
void MultiF::Get1( Mu1Digit& D1)
// this is not changed
{
  Bool    SaveB;

  SaveB = MuErrRep;  MuErrRep = False; // Suppress error messages
  MultiF X(5);  X.SetTo( *this);       // X is a temp on the heap
  if (X.C > 4) {
    MuErrRep = SaveB;
    X.Clear();
    MuWriteErr("Get1 overflow, continuing...");
  }
  else  X.ShiftL(X.C);
  X.MultiSI::Get1( D1);
  MuErrRep = SaveB;
} // --end-- MultiF::Get1

// ------- Db = this Mod Base**4
void MultiF::GetD( double& Db)
// this is not changed
{
  Bool    SaveB;

  SaveB = MuErrRep;  MuErrRep = False; // Suppress error messages
  MultiF X(5);  X.SetTo( *this);
  if (X.C > 4) {
    MuErrRep = SaveB;
    X.Clear();  MuWriteErr("GetD overflow, continuing...");
  }
  else  X.ShiftL(X.C);
  X.MultiSI::GetD( Db);
  MuErrRep = SaveB;
} // --end-- MultiF::GetD

// ------- Db = this Mod Base**4, in a range, + or -
void MultiF::GetDIn( double& Db, double Lo, double Hi)
{
  Db = N;  Db = Db + C;
  if (Db > 4.0) {
    if (S)  Db = Lo;
    else  Db = Hi;
    return;
  }
  GetD( Db);
  if (Db < Lo)  Db = Lo;
  if (Db > Hi)  Db = Hi;
} // --end-- MultiF::GrtDIn

// ------- this = this + D1
void MultiF::RAdd1( Mu1Digit D1)
// Allows literal D1, D1 is not changed, sets MuErr True if overflow
{
  MultiF X(1);  X.SetTo1( D1);
  RAdd(X);
} // --end-- MultiF::RAdd1

// ------- this = X + D1
void MultiF::Add1( MultiF& X, Mu1Digit D1)
// Allows literal D1, D1 is not changed, sets MuErr True if overflow
{
  SetTo(X);
  RAdd1( D1);
} // --end-- MultiF::Add1

// ------- Comp = sign( this - X)
int MultiF::Comp( MultiF& X)
// Sign = -1 if this < X; Sign = 0 if this = X; Sign = +1 if this > X
// this.S and X.S used but not changed;  this, X not changed
{
  MultiF Dif(1 + MaxL( N, X.N));
  Dif.Sub( *this, X);
  if      (Dif.ZTest())  return 0;
  else if (Dif.S)        return -1;
  else  return +1;
} // --end-- MultiF::Comp

// ------- Output String and line feed every 80, Tot = characters on line so far
void Write80( ostream& Out, char* St, long& Tot)
{
  if (&Out == &cout)  Out << St;
  else
    for ( int i = 0; St[i] != '\0'; i++) {
      Out << St[i];
      Tot++;
      if (Tot % 80 == 0)  Out << nL;
    }
} // --end-- Write80

// ------- Output String and line feed every 80, Tot = characters on line so far
void Write80Ln( ostream& Out, char* St, long& Tot)
{
  Write80( Out, St, Tot);  Out << nL;
} // --end-- Write80Ln

// ------- Convert +Double to a String integer
void StrDI(char* St, double D);

// ------- Convert +Double to a String integer
void StrDI(char* St, double D)
   // Kludge around problem with Windows 3.0 386 Mode, Str( Single, St)
   //  or Write( Single) or Double or Real can hang system }
{
  long I1, I2;
  char St1[ 11];
  char St2[ 11];
  double DI;

  D = D + 0.5;
  modf( D / 1.0E9, &DI);  I1 = DI; // integer part
  if (!I1) {
    modf( D, &DI);  I2 = DI; // integer part
    sprintf( St, "%ld", I2);
  }
  else {
    modf(D - 1.0E9 * I1, &DI);  I2 = DI + 1000000000L;
    sprintf( St1, "%ld", I1);
    sprintf( St2, "%ld", I2);
    St = strcat( St1, St2+1);
  }
} // --end--  StrDI


// -------
void Write1( ostream& Out, long& Tot, int J, int K, long& Did,
             long DidZ, char* St) // Part of WriteFixed
{
  char* StL = " ";
  while (J < K) {
    J++;
    if ((Did != DidZ) && (MuDpG) && !(Did % MuDpG))
       Write80( Out, ",", Tot);
    StL[0] = St[J];
    Write80( Out, StL, Tot);
    Did++;
  }
} // --end--  Write1

// -------
void WriteFixed( MultiF& T, ostream& Out, long& Tot)
{
  int J, K;
  long Did, DidZ;
  char St[ 15];

  Did = 0;
  if (T.S)  Write80( Out, "-", Tot);
  if (T.N + T.C > 0) {
    sprintf( St, "%.0f", Base + T.V[ T.N - 1]);
    J = 0;
    do
      J++;
    while ((St[J] == '0') && (J != MuDMax));
    Did = J - 1 - MuDMax;
    if (T.N + T.C > 1)  Did -= MuDMax;
    if (MuDpG != 0)     Did  = Did % MuDpG;
    if (Did < 0)        Did += MuDpG;
    DidZ = Did;
    K = MuDMax;  J--;
    Write1( Out, Tot, J, K, Did, DidZ, St);
    if ((T.N + T.C) > 1) {
      if (T.N > 1)
	sprintf( St, "%.0f", Base + T.V[ T.N - 2]);
      else
	sprintf( St, "%.0f", Base);
      K = MuDMax;  J = 0;
      Write1( Out, Tot, J, K, Did, DidZ, St);
    }
  }
  else
    Write80( Out, "0", Tot);
  Write80( Out, ".", Tot);
  DidZ = Did;
  if (-T.C == 2) {
    sprintf( St, "%.0f", Base + T.V[1]);
    K = MuDMax;  J = 0;
    Write1( Out, Tot, J, K, Did, DidZ, St);
  }
  if (-T.C > 0) {
    sprintf( St, "%.0f", Base + T.V[0]);
    J = MuDMax +1;
    do
      J--;
    while (St[J] == '0');
    K = J;  J = 0;
    Write1( Out, Tot, J, K, Did, DidZ, St);
  }
  else
    Write80( Out, "0", Tot);
} // --end--  WriteFixed

// ------- Output this as a line of text, Tot = characters on line so far
void MultiF::Writ( ostream& Out, long& Tot)
// this is not modified
{
  MultiF T; // Temp on heap
  long   LZ;
  char*  Ch = " ";
  int    J, K;
  long   I, Did;
  char   St[ 15];
  char   St2[ 15];
  char*  StL = " ";
  char*  StP = "0.";
  char*  Sep = ",";
  double DE;
  Bool   NewT;
  Bool   Fixed;

  if (ZTest())  { Write80( Out, "0.0 (False)", Tot);  return; }
  if (!C && !S && EQ1(1)) {
    Write80( Out, "1.0 (True) ", Tot);   return;
  }
  Fixed = (!ScieN) && (N + C <= 2) && (-C <= 2);
  if (Fixed) {
    WriteFixed( *this, Out, Tot);  // format <= 99999999999999.99999999999999
    return;
  }
  T = *this;
  NewT = False;
  if ((FTn > 0) && (FTn < FMC) && (N > FMC - FTn)) {
    MultiF TL( FMC - FTn);  NewT = True;
    TL.SetTo( *this);  T = TL;  TL.V = NULL;
  }
  if (T.S)  Write80( Out, "-", Tot);
//Str( Base + T.V[ T.N - 1] :0:0, St);
  sprintf( St, "%.0f", Base + T.V[ T.N - 1]);
  J = 0;
  do
    J++;
  while ((St[J] == '0') && (J != MuDMax));
  if (MuDOnly) {
    Sep = " ";  StP = "0 ";
    if (!MuDpG)  StP = "0";
  }
  StP[0] = St[J];
  Write80( Out, StP, Tot);
  LZ = J - 1;
  Did = 0;
  K = MuDMax;
  if (T.N == 1)  while (St[K] == '0')  K--;
  if ((J >= K) && (T.N == 1 ))  Write80( Out, "0", Tot);
  while (J < K) {
    J++;
    if (Did && MuDpG && !(Did % MuDpG))
       Write80( Out, Sep, Tot);
    StL[0] = St[J];
    Write80( Out, StL, Tot);
    Did++;
    if (Did >= FDn-1)  K = 0;
  }
  if (Did < FDn-1)  I = 2;  else  I = T.N + 1;
  while (I <= T.N) {
//  Str( Base + T.V[ T.N - I] :0:0, St);
    sprintf( St, "%.0f", Base + T.V[ T.N - I]);
    K = MuDMax;
    if (I == T.N)  while (St[K] == '0')  K--;
    J = 1;
    while (J <= K) {
      if (Did && MuDpG && !(Did % MuDpG))
        Write80( Out, Sep, Tot);
      StL[0] = St[J];
      Write80( Out, StL, Tot);
      Did++;
      if (Did >= FDn-1) { K = 0;  I = T.N; }
      J++;
    }
    MuInterrupt;
    if (MuAbort)  I = T.N;
    I++;
  }
  DE = T.C;
  DE = MuDMax * (DE + T.N) - LZ - 1.0;
  if (DE >= 0)  Ch = "+";
  else  Ch = "-";
  if (!MuDOnly) {
    Write80( Out, " E", Tot);
    Write80( Out,   Ch, Tot);
//Str( Abs( DE) :0:0, St);
//StrDi( St, DE);
    sprintf( St, "%.0f", fabs( DE));
    K = strlen( St);
    if ((K > MuDpG) && MuDpG) {
      J = K + 1;
      for (I = 1; I <= ((K-1) / MuDpG); I++) {
        J -= MuDpG;
        strcpy( St2, St+J-1);  St[J-1] = ',';  strcpy(St+J, St2);
      }
    }
    Write80( Out, St, Tot);
    if (Did >= MuLenD) {
      sprintf( St, "%ld", Did + 1);
      Write80( Out, " (", Tot);
      Write80( Out,   St, Tot);
      Write80( Out,  ")", Tot);
    }
    sprintf( St, "%ld", N * (long)( MuDMax));
    Write80( Out, " [", Tot);
    Write80( Out,   St, Tot);
    Write80( Out,  "]", Tot);
  }
  if (!NewT) T.V = NULL;
} // --end-- MultiF::Writ

// ------- Output this as a line of text, Tot = characters on line so far
void MultiF::WritLn( ostream& Out, long& Tot)
{
  Writ( Out, Tot);  Out << nL;
} // --end-- MultiF::WritLn

// ------- this = this * D1
void MultiF::RMul1( Mu1Digit D1)
// Multi-precision one super digit multiply; D1 is not changed
// MuErr set true if this overflows
{
  MultiSI::RMul1( D1);
  Norm();
} // --end-- MultiF::RMul1

// ------- this = X * D1
void MultiF::Mul1( MultiF& X, Mu1Digit D1)
// Multi-precision one super digit multiply; X and D1 are not changed
// MuErr set true if this overflows
{
  SetTo(X);
  RMul1( D1);
} // --end-- MultiF::Mul1

// ------- this = X * Y
void MultiF::MulSlow( MultiF& X, MultiF& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  long     MM;
  long     I, J, II, IJ;
  long     Bias;
  Mu1Digit K;
  Mu2Digit T;
  double   D;

  MultiF XL = X; // Local pointer to X (or Y) so XL.N >= YL.N
  MultiF YL = Y; // Local pointer to Y (or X)
  if (XL.N < YL.N) {
    XL = Y;  YL = X;
  }
  I = 1;
  if (!Expanded)  I = 4; // Four guard "digits", no sticky bit
  MM = MinL( XL.N + YL.N, M + I);
  MM = MinL( MM, FMC + I);
  MM = MinL( MM, MuNMax);
  MultiF Z(MM); // Temp on heap
  Z.S = (XL.S && !YL.S) || (!XL.S && YL.S); // XOR
  Bias = XL.N + YL.N - Z.M;  if (Bias < 0)  Bias = 0;
  for (I = 0; I < (XL.N - Bias); I++)  Z.V[I] = 0; // Clear lower of Z
  KeyHit = False;
  for (J = 0; J < YL.N; J++) {
    II = Bias - J;  if (II < 0)  II = 0;
    IJ = II + J - Bias;  K = 0;
    for (I = II; I < XL.N; I++) {  // Mult, add and carry next Digit
      T = XL.V[I] * YL.V[J] + Z.V[ IJ] + K;
      modf( (double)(T / Base), &D);  K = D;  // integer part
      Z.V[ IJ] = T - K * Base;
      IJ++;
    }
    Z.V[ IJ] = K;
    MuInterrupt;
    if (MuAbort) {
      Clear();  XL.V = YL.V = NULL;  return;
    }
    if (KeyHit) {
      if (Echo)  LogF << "Floating Multiply: " 
                      << (int)((100.0 * (J+1)) / YL.N) << " Percent done" << nL;
      cout << "Floating Multiply: " 
           << (int)((100.0 * (J+1)) / YL.N) << " Percent done" << nL;
      KeyHit = False;
    }
  }
  Z.N = IJ;
  if (K)  Z.N++;
  D = XL.C;  D = D + YL.C;  D = D + Bias;
  if (D > LONG_MAX) {
    Z.Clear();
    MuWriteErr("Floating multiply overflow, continuing...");
  }
  else
    if (-D > LONG_MAX) {
      Z.Clear();
    }
    else
      Z.C = D;
  SetTo(Z);
  Norm();
  XL.V = YL.V = NULL;
} // --end-- MultiFSlow::Mul

// ------- this = X * Y (Fast)
void MultiF::MulFast( MultiF& X, MultiF& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
//  MulSlow( X, Y); // Stub for now
  double CD = (double)X.C + Y.C;
  MultiF Z(X.N + Y.N); // Temp on heap
  Z.MultiSI::Mul(X, Y);  
  Z.Norm();
  SetTo(Z);
  CD += C;
  if (CD > LONG_MAX) {
    Clear();  MuWriteErr("Floating multiply overflow, continuing...");  return;
  }
  if (-CD > LONG_MAX) {
    Clear();  return;  
  }
  C = CD;
} // --end-- MultiF::MulFast

// ------- this = X * Y (Fast or Slow)
void MultiF::Mul( MultiF& X, MultiF& Y)
// X or Y or both may have the same address in memory as this
// X, Y are not changed unless they share memory with this
// Sets MuErr True if overflow
{
  float prod = (float) X.N * Y.N;
  if (prod < 20.0)
    MulSlow(X, Y);
  else
    MulFast(X, Y);
} // --end-- MultiF::Mul

// ------- this = this * X
void MultiF::RMul( MultiF& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow
{
  Mul( *this, X);
} // --end-- MultiF::RMul

// ------- this = X * X
void MultiF::Sq( MultiF& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow
{
  Mul(X, X);
} // --end-- MultiF::Sq

// ------- this = this * this
void MultiF::RSq()
// Sets MuErr True if overflow
{
  Mul( *this, *this);
} // --end-- MultiF::RSq

// ------- this = B ** P
void MultiF::Pow( MultiF& B, MultiF& P)
// B or P or both may have the same address in memory as this
// B, P are not changed unless they share memory with this
// Sets MuErr True if overflow or error
{
  double D, DI;

  MultiF PL( P.N + P.C);  PL.SetTo(P);
  PL.ShiftL( PL.C);
  MultiF Z(M);
  Z.PowMB(B, PL);
  Z.Norm();
  PL.GetD(D);
  modf( D, &DI); // Integer part
  D = DI * B.C + Z.C;
  if (D > LONG_MAX) {
    Z.Clear();  MuWriteErr("B ** P overflow, continuing...");
  }
  else {
    if (-D > LONG_MAX) {
      Z.Clear();
    }
    else  Z.C = D;
  }
  SetTo(Z);
} // --end-- MultiF::Pow

// ------- this = this ** P
void MultiF::RPow( MultiF& P)
// P may have the same address in memory as this
// P is not changed unless it shares memory with this
// Sets MuErr True if overflow or error
{
  Pow( *this, P);
} // --end-- MultiF::RPow

// ------- this = this / D1
void MultiF::RDiv1( Mu1Digit D1)
// Multi-precision one super digit divide, D1 is not changed
// Allows literal D1, MuErr set True if D1 = 0
{
  MultiF X(1);  X.SetTo1( D1);  // Temp on heap
  RDiv(X);
} // --end-- MultiF::RDiv1

// ------- this = U / D1
void MultiF::Div1( MultiF& U, Mu1Digit D1)
// Multi-precision one super digit divide, D1 and U not changed
// U and this may have same address in memory, then U is changed
// Allows literal D1, MuErr set True if D1 = 0
{
  SetTo( U);
  RDiv1( D1);
} // --end-- MultiF::Div1

// ------- Adjust .C, part of MultiF::Divi
void AdjustC( MultiF& Z, MultiF& D)
{
  if ((Z.C > 0) && (D.C < 0))
    if (Z.C > (LONG_MAX + D.C)) {
      Z.Clear();  MuWriteErr("Floating divide overflow, continuing...");
    }
    else
      Z.C -= D.C;
  else
    if ((Z.C < 0) && (D.C > 0)) 
      if (-Z.C > (LONG_MAX - D.C)) {
	Z.Clear();
      }
      else
	Z.C -= D.C;
    else
      if (!Z.ZTest())  Z.C -= D.C;
} // --end-- AdjustC

// ------- this = U / D
void MultiF::Divi( MultiF& U, MultiF& D)
// U or D or both may have the same address in memory as this
// U, D are not changed unless they share memory with this
// Sets MuErr True if D = 0
{
  long   K, ML;
  MultiF Z;  // Temp on heap

  K = MinL(M, FMC);
  if (!Expanded)  K += 4; // Four guard "digits", no sticky bit
  if (K > MuNMax)  K = MuNMax;

  if ((K < 13) || (D.N < (K/3))) {
    Z.NMax(K + D.N);  Z.SetTo(U);
    Z.ShiftL(K - U.N + D.N );
    Z.MultiSI::RDiv(D);
    Z.Norm();
    AdjustC(Z, D);
    SetTo(Z);
  } else {
    long KL = 1 + K / 2;
    long DN = MinL( KL, D.N);
    if ((V != U.V) && (V != D.V)) { ML = M;  NMax(0); }
    Z.NMax(KL + DN);  Z.SetTo1(1);
    Z.ShiftL(KL - 1 + DN );
    long DV = D.N - DN;
    D.V += DV;  D.N -= DV;
    Z.MultiSI::RDiv(D);
    D.V -= DV;  D.N += DV;
    Z.C = -(KL - 1 + DN + DV);
    Z.Norm();
    AdjustC(Z, D);  // Z ~= 1 / D
    Z.NMax( Z.N);

    MultiF ZZ(K);  // ZZ = Z + Z - (Z*Z)*D
    ZZ.Sq(Z);
    ZZ.RMul(D);
    ZZ.ChS();
    ZZ.RAdd(Z);
    ZZ.RAdd(Z);
    Z.NMax(0);
    ZZ.RMul(U);
    if ((V != U.V) && (V != D.V))  NMax( ML);
    SetTo(ZZ);
  }
} // --end-- MultiF::Divi

// ------- this = this / D
void MultiF::RDiv( MultiF& D)
// D may have the same address in memory as this
// D is not changed unless it shares memory with this
// Sets MuErr True if D = 0
{
  Divi( *this, D);
} // --end-- MultiF::RDiv

// ------- this = X ! = 1*2*3*...*X
void MultiF::Fac( MultiF& X)
// X may have the same address in memory as this
// X is not changed unless it shares memory with this
// Sets MuErr True if overflow or error
{
  double NN;

  if (X.S) {
    MuWriteErr("Cannot take factorial of number < zero");
    Clear();  return;
  }
  if ((X.N + X.C) <= 2)  X.GetD( NN);
  else  NN = 1.0 + LONG_MAX;
  if (NN > LONG_MAX) {
    MuWriteErr("Number too large for factorial function");
    Clear();  return;
  }
  MultiF XL(2);
  SetTo1(1);
  TracN = 0;
  while (NN > 1) {
    XL.SetToD( NN);
    Mul( XL, *this);
    if (MuAbort) {
      if (Echo)  LogF << "MultiF::Fac abort, N = " << NN << nL;
      cout << "MultiF::Fac abort, N = " << NN << nL;
      Clear();  NN = 0;
    }
    NN = NN - 1;
    Trace = NN;
    TracN++;
  }
  Trace = 0;
} // --end-- MultiF::Fac

// ------- this = this ! = 1*2*3*...*X
void MultiF::RFac()
// Sets MuErr True if overflow or error
{
  Fac( *this);
} // --end-- MultiF::RFac

// ------- this = SqRt(X)
void MultiF::SqRoot( MultiF& X)
// X is not changed, it is ok for X and this to have the same location in memory
// but then X will be changed.  Sets MuErr True if error
{
  long Sh, ZC, ML, SaveFMC;

  if (X.ZTest()) { Clear();  return; }
  Sh = MinL(M, FMC) + 1;
  if (!Expanded)  Sh += 4; // Four guard "digits", no sticky bit
  if (Sh > MuNMax)  Sh = MuNMax;
  if ((Sh - X.N - X.C) & 1)  Sh--; // if odd
  if (V != X.V) { ML = M;  NMax(0); }
  MultiF Z( Sh);  Z.SetTo(X);
  Z.ShiftL( Sh - Z.N);
  Z.C = Z.C / 2;  ZC = Z.C;
  Z.MultiSI::RSqRt();  Z.C = ZC;
  Z.Norm();
  if (Z.ZTest()) {
    if (V != X.V) { NMax( ML); }
    Clear();
  }
  else {
    Z.NMax( Z.N); 
    MultiF Y( Sh);
    SaveFMC = FMC;
    FMC = Sh;
    Y.Divi(X, Z);
    Y.RSub(Z);
    Y.RDiv1(2);
    Y.RAdd(Z); // (X/Z - Z) / 2  + Z
    Z.NMax( 0);
    if (V != X.V)  NMax( ML);
    SetTo(Y);
    FMC = SaveFMC;
    Norm();
  }
} // --end-- MultiF::SqRoot

// ------- this = SqRt( this)
void MultiF::RSqRt()
// Sets MuErr True if error
{
  SqRoot( *this);
} // --end-- MultiF::RSqRt

// ------- this = Integer part of X
void MultiF::Inte( MultiF& X)
{
  long I, J;
  Mu1Digit SvFRound; // Save Floating point rounding constant, Base / 2 or 0

  SvFRound = FRound;
  FRound = 0;
  SetTo(X);
  if (-C >= N) {
    FRound = SvFRound;
    Clear();  return;
  }
  J = -C - 1;
  for (I = 0; I <= J; I++)  V[I] = 0;
  Norm();
  FRound = SvFRound;
} // --end-- MultiF::Inte

// ------- this = Integer part of this
void MultiF::RInt()
{
  Inte( *this);
} // --end-- MultiF::RInt

// ------- this = Fractional part of X
void MultiF::Fract( MultiF& X)
{
  long I, J;
  Mu1Digit SvFRound; // Save Floating point rounding constant, Base / 2 or 0

  SvFRound = FRound;
  FRound = 0;
  SetTo(X);
  J = -C;
  if (J < 0)  J = 0;
  for (I = J; I < N; I++)  V[I] = 0;
  Norm();
  FRound = SvFRound;
} // --end-- MultiF::Fract

// ------- this = Fractional part of this
void MultiF::RFrac()
{
  Fract( *this);
} // --end-- MultiF::RFrac

// ------- this = X with one less super digit
void MultiF::Lop( MultiF& X)
{
  long L;
  Bool RoundIt;

  SetTo(X);
  if ((FRound > 0) && (V[0] >= FRound)) {
    RoundIt = True;
    if (V[0] == FRound) {
      L = V[1];
      if (!(L & 1))  RoundIt = False; // if not odd
    }
    if (RoundIt) {
      MultiF T(2);
      T.V[0] = 0;
      T.V[1] = 1;
      T.N = 2;
      T.S = S;
      T.C = C;
      V[0] = 0;
      RAdd(T);
      return;
    }
  }
  V[0] = 0;
  Norm();
} // --end-- MultiF::Lop

// ------- this = this with one less least significant "digit"
void MultiF::RLop()
{
  Lop( *this);
} // --end-- MultiF::RLop

// --end-- MultiF's methods implementation

// --end-- Method implementation

// ------- Other services

// ------- Returns max of X, Y
int Max(int X, int Y)
{
  if (X > Y)  return X;
  else  return Y;
} // --end-- Max

// ------- Returns min of X, Y
int Min(int X, int Y)
{
  if (X < Y)  return X;
  else  return Y;
} // --end-- Min

// ------- Returns max of X, Y
long MaxL(long X, long Y)
{
  if (X > Y)  return X;
  else  return Y;
} // --end-- MaxL

// ------- Returns min of X, Y
long MinL(long X, long Y)
{
  if (X < Y)  return X;
  else  return Y;
} // --end-- MinL

// ------- Returns max of X, Y
double MaxD( double X, double Y) 
{
  if (X > Y)  return X;
  else  return Y;
} // --end-- MaxD

// ------- Returns min of X, Y
double MinD( double X, double Y)
{
  if (X < Y)  return X;
  else  return Y;
} // --end-- MinD

// ------- Compute X = Pi by algorithm a
void PiAA( MultiF& X, Bool WReset, Bool ReStart)
// Uses 3 large "Registers" X, Y, and Z.  L is small.
// if WReset then Write reset data after each iteration
// if ReStart then Read reset data before starting
{
  int J;
  int It; // Number of iterations(...
  char* ResetSt = "PiReset.Txt";
  char* OldReSt = "PiReset.Old";
  ofstream WritF;  // Reset text file, output
  FILE*    ReadF; // Reset text file, input
  char Ch;
  char Name[21];
  Bool OK;

  MultiF Z( FMC); // Temp
  MultiF Y( FMC); // y0, y1, ...
  MultiF L( 2);   // -(2**N)
  It = log( 1024.0 * (FMC * (long)( MuDMax) + 3) / 2788.0 ) / log( 2.0) + 2;
  cout << "FMC * MuDMax = " << FMC * MuDMax << "  It = " << It << nL;
  if (!ReStart) {
    Diag("Starting Pi");
    if (Echo)  LogF << "  Starting to compute Pi by algorithm a" << nL;
    cout << "  Starting to compute Pi by algorithm a" << nL;
    int i;
    X.Value("0.5", i); // X0 = 1/2 = 0.5
    Diag("Start Y.SqRoot(0.5);");
    Y.SqRoot(X);       // Y0 = 1/SqRt(2) = SqRt(0.5)
    Diag("End Y.SqRoot(0.5);");
    L.SetTo1(-1);      // L  = -1
    J = 1;
  } else {
    Diag("Restarting Pi");
    do {
      if ((ReadF = fopen( ResetSt, "rt")) == NULL) { // Open Reset file for read
        cout << "Cannot open " << ResetSt
             << ", Type a character, Enter to retry" << nL;
        cin >> Ch;
      }
    } while (ReadF == NULL);
    fscanf( ReadF, "%d", &J);
    L.MultiSI::ReadSI( ReadF, Name, OK);
    Y.MultiSI::ReadSI( ReadF, Name, OK);
    fscanf( ReadF, " (%*ld) %ld", &Y.C);
    X.MultiSI::ReadSI( ReadF, Name, OK);
    fscanf( ReadF, " (%*ld) %ld", &X.C);
    fclose( ReadF);
    J++;
  }
  for ( ; J <= It; J++) {
    Diag("Starting Iteration");
    if (Echo) LogF << "  Start of iteration number " << J << " of " << It << nL;
    cout << "  Start of iteration number " << J << " of " << It << nL;
    // Y = (1 - SqRoot(1 - Y**2)) / (1 + SqRoot(1 - Y**2))
    Diag("Start Y.RSq();");
    Y.RSq();
    Diag("End Y.RSq();");
    Y.ChS();
    Y.RAdd1(1);
    Diag("Start Y.RSqRt();");
    Y.RSqRt();
    Diag("End Y.RSqRt();");
    Z.Add1(Y, 1);          // Z = 1 + SqRoot(1 - Y**2)
    Y.ChS();
    Y.RAdd1(1);            // = 1 - SqRoot(1 - Y**2)
    Diag("Start Y.RDiv();");
    Y.RDiv(Z);
    Diag("End Y.RDiv();");

    // X = (1 + Y)**2 * X - 2**N * Y
    L.RAdd(L);            // L = -2**1, -2**2, ...
    Z.Add1(Y, 1);         // Z = (1 + Y) 
    Diag("Start Z.RSq();");
    Z.RSq();
    Diag("Start X.RMul(Z);");
    X.RMul(Z);            // X = (1 + Y)**2 * X
    Diag("Start Z.Mul(Y, L);");
    Z.Mul(Y, L);          // Z = - 2**N * Y
    Diag("Start X.RAdd(Z);");
    X.RAdd(Z);
    Diag("End X.RAdd(Z);");

    if (MuAbort) {
      Diag("Operator abort");  break;
    }
    if (WReset && !MuAbort) {
      unlink( OldReSt);         // Delete backup file
      rename(ResetSt, OldReSt); // Rename the Reset file
      do {
	WritF.open( ResetSt, ios::trunc); // Open Reset file for rewrite
	if (WritF.fail()) {
	  cout << "Cannot open " << ResetSt
               << ", Type a character, Enter to retry" << nL;
	       cin >> Ch;
        }
      } while (WritF.fail());
      WritF << nL << J << " : J" << dL << "L =" << nL;
      MuTot = 0; L.MultiSI::Writ( WritF); WritF << dL << "Y =" << nL;
      MuTot = 0; Y.MultiSI::Writ( WritF); WritF << dL << Y.C << dL
	<< "X =" << nL;
      MuTot = 0; X.MultiSI::Writ( WritF); WritF << dL << X.C << dL;
      WritF.close();
    }
  }
  if (!MuAbort) {
    Diag("All done but...");
    if (Echo)  LogF << "  All done but inverting alpha" << nL;
    cout << "  All done but inverting alpha" << nL;
    Z.SetTo1(1);         // PI ~= 1 / X 
    X.Divi(Z, X);
    Diag("Pi is done");
    if (Echo)  LogF << "  All done computing Pi" << dL;
    cout << "  All done computing Pi" << dL;
  }
} // --end-- PiAA

// ------- Compute X = Pi by algorithm b
void PiAB( MultiF& X, Bool WReset, Bool ReStart)
// Uses 4 large "Registers" X, Y, Z, and T.  L is small.
// if WReset then Write reset data after each iteration
// if ReStart then Read reset data before starting
{
  int J;
  int It; // Number of iterations(...
  char* ResetSt = "PiReset.Txt";
  char* OldReSt = "PiReset.Old";
  ofstream WritF;  // Reset text file, output
  FILE*    ReadF; // Reset text file, input
  char Ch;
  char Name[21];
  Bool OK;

  MultiF T( FMC); // Temp
  MultiF Z( FMC); // Temp
  MultiF Y( FMC); // y0, y1, ...
  MultiF L( 2);   // -(2**(2 * N + 1))
  It = log( 1024.0 * (FMC * (long)( MuDMax) + 3) / 2788.0 ) / log( 4.0) + 1;
  cout << "FMC * MuDMax = " << FMC * MuDMax << "  It = " << It << nL;
  if (!ReStart) {
    Diag("Starting Pi");
    if (Echo)  LogF << "  Starting to compute Pi by algorithm b" << nL;
    cout << "  Starting to compute Pi by algorithm b" << nL;
    Y.SetTo1(2); // Y0 = SqRt(2) - 1
    Y.RSqRt();
    X.SetTo(Y);
    Y.RAdd1(-1);
    X.RMul1(-4); // X0 = 6 - 4 * SqRt(2);
    X.RAdd1(6);
    L.SetTo1(-2);
    J = 1;
  } else {
    Diag("Restarting Pi");
    do {
      if ((ReadF = fopen( ResetSt, "rt")) == NULL) { // Open Reset file for read
        cout << "Cannot open " << ResetSt
             << ", Type a character, Enter to retry" << nL;
        cin >> Ch;
      }
    } while (ReadF == NULL);
    fscanf( ReadF, "%d", &J);
    L.MultiSI::ReadSI( ReadF, Name, OK);
    Y.MultiSI::ReadSI( ReadF, Name, OK);
    fscanf( ReadF, " (%*ld) %ld", &Y.C);
    X.MultiSI::ReadSI( ReadF, Name, OK);
    fscanf( ReadF, " (%*ld) %ld", &X.C);
    fclose( ReadF);
    J++;
  }
  for ( ; J <= It; J++) {
    Diag("Starting Iteration");
    if (Echo) LogF << "  Start of iteration number " << J << " of " << It << nL;
    cout << "  Start of iteration number " << J << " of " << It << nL;
    // Y = (1 - 4thRoot(1 - Y**4)) / (1 + 4thRoot(1 - Y**4))
    Y.RSq();  Y.RSq();
    Y.ChS();
    Y.RAdd1(1);
    Y.RSqRt();  Y.RSqRt();
    Z.Add1(Y, 1);          // Z = 1 + 4thRoot(1 - Y**4 
    Y.ChS();
    Y.RAdd1(1);            // = 1 - 4thRoot(1 - Y**4
    Y.RDiv(Z);

    // X = (1 + Y)**4 * X - 2**(2*N + 1) * Y * (1 + (1 + Y)*Y) 
    L.RMul1(4);            // L = -2**3, -2**5, ...
    Z.Add1(Y, 1);          // Z = (1 + Y) 
    T.Mul(Z, Y);
    T.RAdd1(1);
    T.RMul(Y);
    T.RMul(L);             // T = - 2**(2*N + 1) * Y * (1 + (1 + Y)*Y 
    Z.RSq();  Z.RSq();
    X.RMul(Z);             // = (1 + Y)**4 * X 
    X.RAdd(T);

    if (MuAbort) {
      Diag("Operator abort");  break;
    }
    if (WReset && !MuAbort) {
      unlink( OldReSt);         // Delete backup file
      rename(ResetSt, OldReSt); // Rename the Reset file
      do {
	WritF.open( ResetSt, ios::trunc); // Open Reset file for rewrite
	if (WritF.fail()) {
	  cout << "Cannot open " << ResetSt
               << ", Type a character, Enter to retry" << nL;
	       cin >> Ch;
        }
      } while (WritF.fail());
      WritF << nL << J << " : J" << dL << "L =" << nL;
      MuTot = 0; L.MultiSI::Writ( WritF); WritF << dL << "Y =" << nL;
      MuTot = 0; Y.MultiSI::Writ( WritF); WritF << dL << Y.C << dL
	<< "X =" << nL;
      MuTot = 0; X.MultiSI::Writ( WritF); WritF << dL << X.C << dL;
      WritF.close();
    }
  }
  if (!MuAbort) {
    Diag("All done but...");
    if (Echo)  LogF << "  All done but inverting alpha" << nL;
    cout << "  All done but inverting alpha" << nL;
    T.SetTo1(1);         // PI ~= 1 / X 
    X.Divi(T, X);
    Diag("Pi is done");
    if (Echo)  LogF << "  All done computing Pi" << dL;
    cout << "  All done computing Pi" << dL;
  }
} // --end-- PiAB

// ------- Compute X = Pi by algorithm b, set Pi to its computed value
void PiTo( MultiF& X)
{
  Bool WReset = False;
  Bool ReStart = False;
  PiAB( X, WReset, ReStart); // Compute X = Pi by algorithm b
} // --end-- PiTo

// ------- End of other services

// ------- Initialization section

// ------- Initialize Multi-precision floating decimal package
void InitMultiF()
{
  InitMulti(); // Initialize Multi-precision package
  FRound = Base / 2; // Set floating point rounding constant to round}
  FMC = 2 + 23 / MuDMax; // Set for at least 25 decimal digits}
  FTn = 1; // Set to truncate 1 super digits in display}
  FDn = (MuNMax - 2) * MuDMax; // Set max digits to display to max}
  ScieN = False;
  Expanded = False;
  FMCB = FMC;
} // --end-- InitMultiF

/*
Revisions made -
--------
Converted from Turbo Pascal 6.0 to Borland C++ 3.1 for Windows  HJS 1992-12-03
--------
*/
// --end-- MultiFDW.Cpp  Multiple-precision floating decimal algorithms
