
#include "des.h"

main()
{
  char *a;
  char b[100];

  a="thisisatest";
  strcpy(b,a);
  keyinit("testing1234");
  endes(b);
  write(1,b,8);
  dedes(b);
  b[8]='\0';
}

