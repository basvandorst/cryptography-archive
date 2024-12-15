
#include "d3des.h"

unsigned long enkey[32],dekey[32];

main()
{
  char *a;
  char b[100];

  a="thisisatest";
  strcpy(b,a);
  deskey("testing1234",0);
  cpkey(enkey);
  deskey("testing1234",1);
  cpkey(dekey);
  usekey(enkey);
  /* keyinit("testing1234");*/
  des(b,b);
/*  endes(b); */
  write(1,b,8);
  deskey("testing1234",1);
  des(b,b);
 /* dedes(b); */
  b[8]='\0';
  write(1,b,8);
}

