// random.c -- randomization routines.
//
// written by Marc Wallace, 30 Apr 93.
// last change: version 1.0 (02 May 93).


#include "include.h"
#include <stdlib.h>
#include <time.h>


void
randinit()
    {
    long t;
    t = time(&t);
    srand(int(t % 65536));
    srand(rand());
    };


char
randc()
    {
    // return a random char value (0-255)
    if (rand() < 10)
        srand(rand());
    return char(rand() % 256);
    };


char
randc(char lo, char hi)
    {
    // return a random char value (between lo and hi)
    if (rand() < 10)
        srand(rand());
    return char(lo + rand() % (hi - lo + 1));
    };



