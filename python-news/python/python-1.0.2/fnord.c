#include <stdio.h>

int
main(int argc, char * argv[])
{
char ch;
FILE * fdin, *fdout;
fdin = fopen(argv[1], "r");
fdout = fopen(argv[2], "w");

while ((ch = fgetc(fdin)) != -1) {
  if (ch == '\r') {
    fputc('\r',fdout);
    fputc('\n',fdout);
  }
  else
    fputc(ch, fdout);
   
 }

}