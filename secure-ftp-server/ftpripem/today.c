#include <stdio.h>
#include <time.h>

main(argc,argv)
int argc;
char *argv[];
{
  char *s,today[8];
  int i;
  long sec,t;

  t=0;
  if (argc>=1) t=86400*-atoi(argv[1]);
    
  time(&sec);
  sec=sec-t;
  s=ctime(&sec);

  for (i=4;i<=9;i++)
    {
      today[i-4]=s[i];
    }
  today[6]=0;

  fprintf(stdout,"%s\n",today);
}
