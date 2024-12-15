#include <stdio.h>
#include <errno.h>
extern int errno;
#include "getopt.h"

#define TAB '	' /*XXX*/

int flagcheck = 0;
int flagin = 0;
int flaghin = 0;
int flagbinary = 1;

int flagallmatch = 1;
int flagreaderr = 0;

char *oldhash; /* defined for the duration of doit, if flagcheck */

void doit(fi,fn)
FILE *fi;
char *fn;
{
 static char out[100];

 sum(fi,out);

 if (flagcheck)
  {
   if (strcmp(oldhash,out))
    {
     fprintf(stderr,"%s: warning: %s hash does not match\n"
       ,optprogname,fn);
     flagallmatch = 0;
    }
  }
 else
  {
   fputs(out,stdout);
   putchar(TAB);
   puts(fn);
  }
}

void handle(x)
char *x;
{
 FILE *fi;

 fi = fopen(x,flagbinary ? "rb" : "r");
 if (!fi)
  {
   int errnobak;
   errnobak = errno;
   fprintf(stderr,"%s: warning: cannot open %s for "
     ,optprogname,x);
   errno = errnobak;
   perror("reading");
   flagreaderr = 1;
  }
 else
  {
   doit(fi,x);
   fclose(fi);
  }
}

char line[1000];

main(argc,argv)
int argc;
char **argv;
{
 int opt;

 while ((opt = getopt(argc,argv,"cgfFiIbB")) != opteof)
   switch(opt)
    {
     case 'c': flagcheck = 1; break;
     case 'g': flagcheck = 0; break;
     case 'f': flaghin = 0; break;
     case 'F': flaghin = 1; break;
     case 'i': flagin = 1; break;
     case 'I': flagin = 0; break;
     case 'b': flagbinary = 1; break;
     case 'B': flagbinary = 0; break;
     case '?': default: exit(1);
    }
 argc -= optind;
 argv += optind;

 if (flagcheck)
   flagin = flaghin = 1;

 if (flagin)
   while (fgets(line,sizeof(line),stdin))
    {
     char *x;
     int len;
     x = line;
     if (flaghin)
      {
       while (*x && (*x != '\n') && (*x != TAB)) ++x;
       if (*x == TAB) *x++ = 0;
      }
     if (!*x) continue; /*XXX*/
     len = strlen(x);
     if (x[len - 1] == '\n') x[len - 1] = 0;
     if (!*x) continue; /*XXX*/
     if (flagcheck)
       oldhash = line;
     handle(x);
    }
 else
   if (*argv)
     while (*argv)
      {
       handle(*argv);
       ++argv;
      }
   else
     doit(stdin,""); /*XXX*/

 if (!flagallmatch)
   exit(3);
 if (flagreaderr)
   exit(2);
 exit(0);
}
