// cypher.c -- class functions for classes "cypher" and "metacypher".
//
// written by Marc Wallace, 26 Apr 93.
// last change: version 1.1 (14 May 93).


#include "data.h"
#include "cypher.h"


void nuthin() {};


Cypher::Cypher(char* n, Cyfunct e, Cyfunct d, void (*i)() = nuthin)
    {
    name = n;
    encode = e;
    decode = d;
    i();
    };


