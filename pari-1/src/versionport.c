#include "genpari.h"

void printversion(void)
{
  printversionno();
#ifdef __cplusplus
  pariputs("             (portable C++ 32/64-bit version)\n");
#else
  pariputs("               (portable 32/64-bit version)\n");
#endif
}

ulong overflow,hiremainder;

#ifdef __cplusplus
#define IL inline
#else
#define IL
#endif

IL long addll(long x, long y)
{
  long z;

  z=x+y;
  if((x>=0)&&(y>=0)) {overflow=0;return z;}
  if((x<0)&&(y<0)) {overflow=1;return z;}
  overflow=(z>=0)?1:0; return z;
}

IL long addllx(long x, long y)
{
  long z;

  z=x+y+overflow;
  if((x>=0)&&(y>=0)) {overflow=0;return z;}
  if((x<0)&&(y<0)) {overflow=1;return z;}
  overflow=(z>=0)?1:0; return z;
}

IL long subll(long x, long y)
{
  long z;

  z=x-y;
  if((x>=0)&&(y<0)) {overflow=1;return z;}
  if((x<0)&&(y>=0)) {overflow=0;return z;}
  overflow=(z>=0)?0:1; return z;
}

IL long subllx(long x, long y)
{
  long z;

  z=x-y-overflow;
  if((x>=0)&&(y<0)) {overflow=1;return z;}
  if((x<0)&&(y>=0)) {overflow=0;return z;}
  overflow=(z>=0)?0:1; return z;
}

IL long shiftl(ulong x, ulong y)
{
  hiremainder=x>>(BITS_IN_LONG-y);return (x<<y);
}

IL long shiftlr(ulong x, ulong y)
{
  hiremainder=x<<(BITS_IN_LONG-y);return (x>>y);
}

int bfffo(ulong x)
{
  int sc;
  static int tabshi[16]={4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};

#ifdef LONG_IS_64BIT
  if(x&(0xffffffff00000000)) sc=0; else {sc=32;x<<=32;}
  if(!(x&(0xffff000000000000))) {sc+=16;x<<=16;}
  if(!(x&(0xff00000000000000))) {sc+=8;x<<=8;}
  if(x&(0xf000000000000000)) x>>=60; else {sc+=4;x>>=56;}
  sc+=tabshi[x];return sc;
#else
  if(x&(0xffff0000)) sc=0; else {sc=16;x<<=16;}
  if(!(x&(0xff000000))) {sc+=8;x<<=8;}
  if(x&(0xf0000000)) x>>=28; else {sc+=4;x>>=24;}
  sc+=tabshi[x];return sc;
#endif
}

IL long mulll(ulong x, ulong y)
{
  ulong xlo,xhi,ylo,yhi;
  ulong z;

  xlo=x&MAXHALFULONG;xhi=x>>(BITS_IN_LONG/2);
  ylo=y&MAXHALFULONG;yhi=y>>(BITS_IN_LONG/2);
  z=addll(xlo*yhi,xhi*ylo);
  hiremainder=(overflow)?(xhi*yhi+(MAXHALFULONG+1)+(z>>(BITS_IN_LONG/2)))
    :(xhi*yhi+(z>>(BITS_IN_LONG/2)));
  z=addll(xlo*ylo,(z<<(BITS_IN_LONG/2)));hiremainder+=overflow;
  return z;
}

IL long addmul(ulong x, ulong y)
{
  ulong xlo,xhi,ylo,yhi;
  ulong z,z2;

  xlo=x&MAXHALFULONG;xhi=x>>(BITS_IN_LONG/2);
  ylo=y&MAXHALFULONG;yhi=y>>(BITS_IN_LONG/2);
  z=addll(xlo*yhi,xhi*ylo);
  z2=(overflow)?(xhi*yhi+(MAXHALFULONG+1)+(z>>(BITS_IN_LONG/2)))
    :(xhi*yhi+(z>>(BITS_IN_LONG/2)));
  z=addll(xlo*ylo,(z<<(BITS_IN_LONG/2)));z2+=overflow;
  z=addll(z,hiremainder);hiremainder=z2+overflow;
  return z;
}

long divll(ulong x, ulong y)
{
#define HIGHWORD(a) (a >> (BITS_IN_LONG/2))
/* si le compilateur est bugge, il faut mettre
 (a >> (BITS_IN_LONG)/2) & LOWMASK) */
#define LOWWORD(a) (a & LOWMASK)

#define GLUE(hi, lo) ((hi << (BITS_IN_LONG/2)) + lo)
#define SPLIT(a, b, c) b = HIGHWORD(a); c = LOWWORD(a)

  ulong v1, v2, u3, u4, q1, q2, aux, aux1, aux2;
  int k;
  
  for(k = 0; !(y & HIGHBIT); k++)
    {
      hiremainder <<= 1;
      if (x & HIGHBIT) hiremainder++;
      x <<= 1;
      y <<= 1;
    }
  
  SPLIT(y, v1, v2);
  SPLIT(x, u3, u4);
  
  q1 = hiremainder / v1; if (q1 & HIGHMASK) q1 = LOWMASK;
  hiremainder -= q1 * v1;
  aux = v2 * q1;
 again:
  SPLIT(aux, aux1, aux2);
  if (aux2 > u3) aux1++;
  if (aux1 > hiremainder) {q1--; hiremainder += v1; aux -= v2; goto again;}
  u3 -= aux2;
  hiremainder -= aux1;
  hiremainder <<= (BITS_IN_LONG/2); hiremainder += u3 & LOWMASK;
  
  q2 = hiremainder / v1; if (q2 & HIGHMASK) q2 = LOWMASK;
  hiremainder -= q2 * v1;
  aux = v2 * q2;
 again2:
  SPLIT(aux, aux1, aux2);
  if (aux2 > u4) aux1++;
  if (aux1 > hiremainder) {q2--; hiremainder += v1; aux -= v2; goto again2;}
  u4 -= aux2;
  hiremainder -= aux1;
  hiremainder <<= (BITS_IN_LONG/2); hiremainder += u4 & LOWMASK;
  hiremainder >>= k;
  return GLUE(q1, q2);
}

#undef IL

#ifndef __cplusplus
long mulmodll(ulong a,ulong b,ulong c)
{
  divll(mulll(a,b),c);
  return hiremainder;
}
#endif
