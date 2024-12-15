// ------- Perfect.Cpp Compute a perfect numbers from q

char *Name   = "Perfect - Compute a perfect number from an input number q";
char *Version= "Version 1.00, last revised: 1994-08-20, 1600 hours";
char *Author = "Copyright (c) 1981-1994 by author: Harry J. Smith,";
char *Address= "19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.";

// -------- Perfect Numbers and Mersenne Primes --------

// A number is called perfect if it is equal to the sum of its divisors.
// Six is perfect: 6 = 1 + 2 + 3.
// 28 is perfect: 28 = 1 + 2 + 4 + 7 + 14.

// M(n) = 2^n - 1 is called a Mersenne number.
// If M(q) = is prime then it is called a Mersenne prime and q will be prime also.
// If q makes a Mersenne prime then P(q) = 2^(q-1) * (2^q - 1) is a Perfect number.

#include "MultiFDW.h"

// Developed in Borland C++ 3.1 and 4.0 for Windows.

  char FileSt[13]; // Disk output file name
  long Places;     // number of decimal places to compute Perfect
  Bool DOnly;      // True if only raw digits of a number are to be output
  int  DpG;        // Digits per group in number output display >= 1 or 0
  long q;          // q, the prime that makes M(q) = (2^q - 1) prime
                   // and makes 2^(q-1) * (2^q - 1) a Perfect number

void init() {
  clrscr();
  cout << Name <<nL<< Version <<nL<< Author <<nL<< Address << dL;
  strcpy( FileSt, "Perfect.Out");
  cout << "Will write file: " << FileSt
       << " containing the perfect number." << dL;
  cout << "Enter a prime for which 2^q - 1 is a Mersenne prime (or 0 to quit): ";
  cin >> q;
  if (q == 0)  exit(0);
  Places = 1 + (long) floor((q+q-1) * 0.301029995663981195);
  InitMultiF(); // Initialize Multi-precision packages F and SI

  cout <<
   "How many digits between commas in output? (0 for no , < 0 for digits only): ";
  cin >> DpG;
  DOnly = (DpG < 0);
  if (DpG <= -32768L)  DpG = 0;
  DpG = abs( DpG);

  Echo = True;
  Disk.open( FileSt, ios::trunc); // Open output file for rewrite
  if (Disk.fail())  { cout << "Cannot open " << FileSt << nL;  exit(1); }
  cout << nL;  Disk << nL;
  Disk.close();
  GetDateTime();
  cout << "Start of Perfect.Cpp ";
  OutDateTime( cout);
//  int NRegs = 5;  MuSetMMax( NRegs);
} // --end-- init

// ------- Terminate program
void quit()
{
  GetDateTime();
  cout << "End of Perfect.Cpp ";
  OutDateTime( cout);
  Disk.close();
//HeapOk(); // Heap check not supported by MS Windows
  exit(0);
} // --end-- quit

int main( void) // ------- Perfect.Cpp
{
  long Tot;
  char Ch;

  init();

  DiagOn = True;  TV3 = TV2 = TV1 = TV0 = DosClock();
  FTn = 3;
  FMC = 5 + (Places + 20) / MuDMax;
  cout << "Trying to compute Perfect number to about " << (FMC - 4) * MuDMax
       << " decimal places." << nL;
  MultiF A( FMC);
  MultiF B( FMC);
  MultiF X( FMC);
  B.SetTo1( (Mu1Digit) 2);   // B = 2
  X.SetToD( (double) (q-1)); // X = q-1
  A.Pow( B, X);              // A = 2^(q-1)
  B.Add( A, A);              // B = 2 * A = 2^q
  B.RAdd1( (Mu1Digit) -1);   // B = 2^q - 1
  X.Mul( A, B);              // X = 2^(q-1) * (2^q - 1) the perfect number

  if (!MuAbort) {
    do {
      Disk.open( FileSt, ios::trunc); // Open output file for rewrite
      if (Disk.fail()) {
        cout << "Cannot open " << FileSt
             << ", Type a character, Enter to retry" << nL;
	     cin >> Ch;
      }
    } while (Disk.fail());

    MuDOnly = DOnly;
    MuDpG = DpG;
    Diag("Writing Perfect number to disk");
    cout << nL;
    if (!DOnly) {
      cout       << "Perfect Number =" << dL;
      Disk << nL << "For q = " << q << ",  2^(q-1) * (2^q - 1) is:" << nL;
      Disk << nL << "Perfect Number =" << dL;
    }
    Tot = 0;
    X.WritLn( Disk, Tot);
    Tot = 0;
    FDn = 700;
    X.WritLn( cout, Tot);  cout << nL;
    Diag("All done");
  }
  quit();
  return 0;
}

// --end-- Perfect.Cpp Compute a perfect numbers from q

