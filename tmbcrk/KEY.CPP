// key.cpp -- Keyboard routines

#include <conio.h>
#include "key.h"

int savedChar;  // Char saved by ungetKey()

// Wait for and return the next keypress. Displays nothing.
// Returns extended PC ASCII values from 0 to 255 for alphanumeric and
// control keys. Returns negative values for function keys such as <F1>,
// <End>, and <Home>. Constants for these keys are defined in key.doc.
// Other similar constants (e.g. for <Alt>-key combinations) can be
// created by taking the second character returned by getch() for that
// key and subtracting 256 from the character's ASCII value.

int getKey(void)
{
  int c;

  if (savedChar != 0) {   // If there's a saved char
    c = savedChar;        // Assign it to c
    savedChar = 0;        // Reset savedChar to none
  } else {
    c = getch();          // Else get next keypress
    if (c == 0)           // Check for function-key lead in 
      c = getch() - 256;  // Return function key value < 0
  }
  return c;  // Return next keypress to caller
}

// Return true if a key was pressed and a character is waiting to be
// read, or if the program called ungetKey and a saved character is
// waiting to be reread.

int keyWaiting(void)
{
  return (kbhit() || (savedChar != 0));
}

// Undo the most recent call to getKey. Saves keypress value k in a
// global variable. That key will then be returned by the next call to
// getKey. Only one key value can be undone. If you call this routine
// without calling getKey, it erases the previously undone key.

void ungetKey(int k)
{
  savedChar = k;
}


// Copyright (c) 1990,1992 by Tom Swan. All rights reserved
// Revision 1.00    Date: 09/26/1990   Time: 07:33 pm
// Revision 2.00    Date: 05/22/1992   Time: 05:48 pm
// 1. Converted for Borland C++ 3, Microsoft C/C++ 7, Zortech C++ 3
// 2. Changed "int ungetKey()" to "void ungetKey()"
