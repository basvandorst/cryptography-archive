// version.c -- list of version names and their cyphers.
//
// written by Marc Wallace, 02 May 93.
// last change: version 1.1 (26 May 93).


#include "include.h"


extern Cypher  recurxor, cubicxor, blockxor, quadadd;
extern Cypher  square,   sixteen,  rotate,   dualxor;


char *version[] = 
    {
    "1.0",     // first alpha version
    "1.1",     // first beta version
    0
    };


int verlimit[] =
    {
    3,   // 1.0
    8,   // 1.1
    0
    };


Cypher* cypherlist[] = 
    {
                 // Cypher Name              Ver   Author            
                 // -------------            ---   ------
    &recurxor,   // Recursive XOR            1.0   Merle
    &cubicxor,   // Cubic XOR                1.0   Merle
    &blockxor,   // Block XOR                1.0   Merle
    &quadadd,    // Quadratic Addition       1.1   Merle
    &square,     // Square Mod 251           1.1   Merle
    &sixteen,    // Sixteen Square Rotation  1.1   Merle
    &rotate,     // Byte Rotation            1.1   Merle
    &dualxor,    // Dual XOR                 1.1   Merle

    0            // NULL
    };

