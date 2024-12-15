// key.h -- Header for key.cpp

#ifndef __KEY_H    //
#define __KEY_H         1     // Prevent multiple #includes

// Various key definitions. The values work only with the getkey
// function in the key module; they do not work with the getch and
// other standard key-input library functions.

int getKey(void);
int keyWaiting(void);
void ungetKey(int k);

#endif   // __KEY_H


// Copyright (c) 1990,1992 by Tom Swan. All rights reserved
// Revision 1.00    Date: 09/26/1990   Time: 07:32 pm
// Revision 2.00    Date: 05/22/1992   Time: 05:47 pm
// 1. Converted for Borland C++ 3, Microsoft C/C++ 7, Zortech C++ 3
// 2. Changed "int ungetKey()" to "void ungetKey()"
