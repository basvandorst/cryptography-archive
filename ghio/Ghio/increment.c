#include <stdio.h>

void main(int argc,char *argv[]) {
  FILE *file;
  int x;

  file=fopen(argv[1],"r");
  fscanf(file,"%d",&x);
  fclose(file);

  x=x+1;

  file=fopen(argv[1],"w");
  fprintf(file,"%d",x);
  fclose(file);
}
