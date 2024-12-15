#include "genpari.h"

void printversion(void)
{
  printversionno();
#ifdef __cplusplus
  pariputs("                   (C++ Sparcv7 version)\n");
#else
  pariputs("                     (Sparcv7 version)\n");
#endif
}

#ifndef __cplusplus
long mulmodll(ulong a,ulong b,ulong c)
{
  divll(mulll(a,b),c);
  return hiremainder;
}
#endif
