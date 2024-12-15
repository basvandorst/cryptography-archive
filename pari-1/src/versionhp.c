#include "genpari.h"

void printversion(void)
{
  printversionno();
#ifdef __cplusplus
  pariputs("                   (C++ hppa version)\n");
#else
  pariputs("                     (hppa version)\n");
#endif
}

#ifndef __cplusplus
long mulmodll(ulong a, ulong b, ulong c)
{
  divll(mulll(a,b),c);
  return hiremainder;
}
#endif
