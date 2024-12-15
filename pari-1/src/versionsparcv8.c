#include "genpari.h"

void printversion(void)
{
  printversionno();
#ifdef __cplusplus
  pariputs("                   (C++ Sparcv8 version)\n");
#else
  pariputs("                     (Sparcv8 version)\n");
#endif
}

#ifndef __cplusplus
long mulmodll(ulong a,ulong b,ulong c)
{
  divll(mulll(a,b),c);
  return hiremainder;
}
#endif
