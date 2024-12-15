/*
 * This program is copyright (c) Alec Muffett 1991 except for certain
 * portions of code ("fdes-crypt.c") copyright (c) Robert Baldwin, Icarus
 * Sparry and Alec Muffett.  The author(s) disclaims all responsibility or
 * liability with respect to it's usage or its effect upon hardware or
 * computer systems.  This software is in the public domain and is freely
 * redistributable PROVIDED that this notice remains intact.
 */

#include "conf.h"

#define STRINGSIZE     255

void Trim ();
char *Reverse ();
char *Uppercase ();
char *Lowercase ();
char *Clone ();

#ifdef FAST_TOCASE
#define toupper(x)     _toupper(x)
#define tolower(x)     _tolower(x)
#endif

#ifdef FCRYPT
#define crypt(a,b)     fcrypt(a,b)
#endif

extern char *fcrypt(char *pw, char *salt);
