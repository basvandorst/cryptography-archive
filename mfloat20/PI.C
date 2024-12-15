/*  Here are some methods demonstrated to calculate the number pi */

extern unsigned _stklen = 40000U;       /* use a large stack segment */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "mfloat.h"

int main (void) {

  mfloat a,b,c,bound;
  int i;
  char ch[1000];

  mfloatTest80386();
  clrscr();
  setmantissawords(16);
  getonem(bound);
  ldexpm(bound,-16*getmantissawords()-16);
  printf("                            Calculation of PI\n");
  printf("                            =================\n\n\n");
  printf("PI = 16 * arctan(1 / 5) - 4 * arctan(1 / 239) :\n");
  equm(b,divi(multi(getonem(a),16),5));
  i=0;
  do {
    i++;
    divi(equm(c,divi(b,25)),2*i+1);
    if (i & 1) subm(a,c);    /* odd "i"  */
    else addm(a,c);          /* even "i" */
  } while (!gtm(bound,c));
  subm(a,divi(multi(getonem(b),4),239));
  i=0;
  do {
    i++;
    divi(equm(c,divi(divi(b,239),239)),2*i+1);
    if (i & 1) addm(a,c);    /* odd "i"  */
    else subm(a,c);          /* even "i" */
  } while (!gtm(bound,c));
  printf("%s\n\n", mftostr(ch,a,80,".F77"));
  printf("*******************************************************************************\n\n");
  printf("PI = 48 * arctan(1 / 18)  + 32 * arctan(1 / 57) - 20 * arctan(1 / 239) :\n");
  equm(b,divi(multi(getonem(a),8),3));
  i=0;
  do {
    i++;
    divi(equm(c,divi(b,324)),2*i+1);
    if (i & 1) subm(a,c);    /* odd "i"  */
    else addm(a,c);          /* even "i" */
  } while (!gtm(bound,c));
  addm(a,divi(multi(getonem(b),32),57));
  i=0;
  do {
    i++;
    divi(equm(c,divi(b,3249)),2*i+1);
    if (i & 1) subm(a,c);    /* odd "i"  */
    else addm(a,c);          /* even "i" */
  } while (!gtm(bound,c));
  subm(a,divi(multi(getonem(b),20),239));
  i=0;
  do {
    i++;
    divi(equm(c,divi(divi(b,239),239)),2*i+1);
    if (i & 1) addm(a,c);   /* odd "i"  */
    else subm(a,c);         /* even "i" */
  } while (!gtm(bound,c));
  printf("%s\n\n", mftostr(ch,a,80,".F77"));
  printf("*******************************************************************************\n\n");
  printf("PI = 6 * arctan(1 / sqrt(3)) :\n");
  equm(b,multi(sqrtm(multi(getonem(a),3)),2));
  i = 0;
  do {
    i++;
    divi(equm(c,divi(b,3)),2*i+1);
    if (i & 1) subm(a,c);    /* odd "i"  */
    else addm(a,c);          /* even "i" */
  } while (!gtm(bound,c));
  printf("%s\n\n", mftostr(ch,a,80,".F77"));
  printf("*******************************************************************************\n\n");
  printf("Internal PI :\n");
  getpim(a);
  printf("%s\n\n", mftostr(ch,a,80,".F77"));
  return 0;
}