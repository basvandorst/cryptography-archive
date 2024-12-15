// ------- PiW.Cpp Compute Pi to a million or so decimal places in Windows

char *Name   = "PiW - Compute Pi to a million or so decimal places in Windows";
char *Version= "Version 1.31, last revised: 1994-07-29, 0600 hours";
char *Author = "Copyright (c) 1981-1994 by author: Harry J. Smith,";
char *Address= "19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.";

#include "MultiFDW.h"

// Developed in Borland C++ 3.1 and 4.0 for Windows.

  char FileSt[13]; // Disk output file name
  char LogSt[13];  // Log output file name
  long Places;     // number of decimal places to compute Pi
  Bool WReset;     // if WReset then Write reset data after each iteration of pi
  Bool ReStart;    // if ReStart then Read reset data before computation of pi
  Bool UseAB;      // if UseAB then use algorithm b else use algorithm a
  Bool DOnly;      // True if only raw digits of a number are to be output
  int  DpG;        // Digits per group in number output display >= 1 or 0

void init() {
  clrscr();
  cout << Name <<nL<< Version <<nL<< Author <<nL<< Address << dL;
  strcpy( FileSt, "PiW.Out");
  strcpy( LogSt,  "PiW.Log");
  cout << "Will write file: " << FileSt
       << " containing Pi." << dL;
  cout << "Will write file: " << LogSt
       << " containing a log of operations." << dL;
  cout << "Will write file: PiReset.Txt"
       << " containing a ReStart file." << dL;
  cout << "Will write file: PiReset.Old"
       << " containing a ReStart file backup." << dL;
  cout << "How many decimal places do you need? (n < 0 to restart) (0 to quit): ";
  cin >> Places;
  if (Places == 0)  exit(0);
  cout << "Use algorithm b? (Y/N): ";
  char YesNo;  cin >> YesNo;
  UseAB = (YesNo == 'Y') || (YesNo == 'y');
  ReStart = (Places < 0);
  Places = labs(Places);
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
  if (!ReStart)
    LogF.open( LogSt, ios::trunc);  // Open Log file for rewrite
  else
    LogF.open( LogSt, ios::app);    // Open Log file for append
  if (LogF.fail())  { cout << "Cannot open " << LogSt << nL;  exit(1); }
  cout << nL;  LogF << nL;  Disk << nL;
  Disk.close();
  GetDateTime();
  cout << "Start of PiW.Cpp ";
  OutDateTime( cout);
  if (Echo) {
    if (!ReStart)
      LogF << Name <<nL<< Version <<nL<< Author <<nL<< Address << dL;
    else
      LogF << "Re";
    LogF << "Start of PiW.Cpp ";
    OutDateTime( LogF);
  }
//  int NRegs = 5;  MuSetMMax( NRegs);
} // --end-- init

// ------- Terminate program
void quit()
{
  GetDateTime();
  cout << "End of PiW.Cpp ";
  OutDateTime( cout);
  if (Echo) {
    LogF << "End of PiW.Cpp ";
    OutDateTime( LogF);
  }
  Disk.close();  LogF.close();
//HeapOk(); // Heap check not supported by MS Windows
  exit(0);
} // --end-- quit

int main( void) // ------- PiW.Cpp
{
  long Tot;
  char Ch;

  init();

  DiagOn = True;  TV3 = TV2 = TV1 = TV0 = DosClock();
  FTn = 3;
  FMC = 5 + (Places + 20) / MuDMax;
  cout << "Trying to compute Pi to about " << (FMC - 4) * MuDMax
       << " decimal places." << nL;
  LogF << "Trying to compute Pi to about " << (FMC - 4) * MuDMax
       << " decimal places." << nL;
  MultiF X( FMC);
  WReset = True;

  if (UseAB)
    PiAB( X, WReset, ReStart); // Compute X = Pi using algorithm b
  else
    PiAA( X, WReset, ReStart); // Compute X = Pi using algorithm a

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
    Diag("Writing Pi to disk");
    cout << nL;
    LogF << nL;
    if (!DOnly) {
      cout       << "Pi = m.n E+0, m.n =" << dL;
      LogF       << "Pi = m.n E+0, m.n =" << dL;
      Disk << nL << "Pi = m.n E+0, m.n =" << dL;
    }
    Tot = 0;
    X.WritLn( Disk, Tot);
    Tot = 0;
    FDn = 700;
    X.WritLn( LogF, Tot);  LogF << nL;
    X.WritLn( cout, Tot);  cout << nL;
    Diag("All done");
  }
  quit();
  return 0;
}

// --end-- PiW.Cpp Compute Pi to a million or so decimal places in Window
