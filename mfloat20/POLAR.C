/* This program demonstrates the conversion of cartesian coordinates */
/* to polar coordinates with mfloat numbers                          */

extern unsigned _stklen = 40000U;       /* use a large stack segment  */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "mfloat.h"

int main (void) {

  mfloat x,y,r,phi,xr,yr;
  char ch[1000];

  mfloatTest80386();
  clrscr();
  setmantissawords(15);
  printf("          Conversion cartesian coordinates - polar coordinates\n");
  printf("          ====================================================\n\n");
  printf("Cartesian coordinates:\n");
  do {
    printf("x = ");
    scanf("%s",&ch);
  } while (strtomf(x,ch));
  do {
    printf("y = ");
    scanf("%s",&ch);
  } while (strtomf(y,ch));
  printf("\n");
  printf("x   = %s\n", mftoa(ch,x,50));
  printf("y   = %s\n\n", mftoa(ch,y,50));
  printf("Conversion to polar coordinates:\n\n");
  hypotm(equm(r,x),y);
  atan2m(equm(phi,y),x);
  printf("r   = %s\n", mftoa(ch,r,50));
  printf("phi = %s\n\n\n", mftoa(ch,phi,50));
  printf("Conversion back to cartesian coordinates:\n\n");
  cossinm(equm(xr,phi),yr);
  multm(xr,r);
  multm(yr,r);
  printf("x   = %s\n", mftoa(ch,xr,50));
  printf("y   = %s\n", mftoa(ch,yr,50));
  return 0;
}