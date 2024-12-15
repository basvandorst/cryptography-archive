#include <stdio.h>

void main(int argc,char *argv[]) {
  char input[256];
  int x=0;

  while (fgets(input,255,stdin)!=0) ++x;

  printf("%d",x);
}
