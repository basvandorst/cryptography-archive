/*
 * gendata.c  --  Dinky program to generate output for socktest.
 */

#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
  int i, j;
  if (argc != 2)
    return -1;
  j = atoi(argv[1]);
  for (;;)
   {
      for (i = 0; i < j; i++)
        putchar('0' + (i%10)); 
      putchar('\n');
   }
}
