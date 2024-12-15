#include <stdio.h>
#include "precision.h"

extern int errno;

int	debug = 0;
char	*myname;

main(argc, argv)
   int argc;
   char *argv[];
{
   extern int optind;
   extern char *optarg;
   
   precision factorial();
   precision p = pUndef;

   int  i, ch;
   char *chp;

   myname = *argv;

   while ((ch = getopt(argc, argv, "d")) != EOF) {
      switch (ch) {
      case 'd':
	 debug = ! debug;
	 break;
      default:
	 fprintf(stderr, "usage: %s [ -d ]\n", myname);
	 exit(1);
      }
   }
   argc -= optind; argv += optind;

   if (argc) {
      pset(&p ,atop(*argv));
      argc--, argv++;
   }

   fputp(stdout, factorial(p));
   putchar('\n');
   pdestroy(p);
   return 0;
}

/*
 * Rules (for precision objects):
 *   - local variables must be initialized to pUndef
 *   - formal parameters must be pparm'd on entry to procedure
 *   - local variables AND parameters must be pdestroy'd prior to return, 
 *     unless it's a ...
 *   - function results must be returned by calling presult
 */
precision factorial(n)
   precision n;
{
   precision res = pUndef;

   pparm(n);
   if (peqz(n)) {
      pset(&res, pone);
   } else {
      pset(&res, pmul(n, factorial(psub(n, pone))));
   }
   pdestroy(n);
   return presult(res);
}
