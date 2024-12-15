#include <stdio.h>
#include <mon.h>
#include "precision.h"

#define NFUNC	1024

extern char *malloc();

extern etext();

main(argc, argv)
   int argc;
   char *argv[];
{
   precision u = pUndef, n = pUndef, m = pUndef, r = pUndef;
   unsigned int bufsize;
   char *buf;

   bufsize = (WORD *) etext - (WORD *) main;

   buf = malloc(bufsize * sizeof (WORD) );
/*   monitor(main, etext, buf, bufsize, NFUNC);  /* */

/*   profil(buf, bufsize * sizeof (WORD), (unsigned int) main, 0xffff); /* */

   printf("bufsize = %u\n", bufsize);
   --argc, ++argv;
   if (argc == 0) {
      fprintf(stderr, "usage pow precision integer\n");
      exit(1);
   }
   pset(&u, atop(*argv++));
   pset(&n, atop(*argv++));
   pset(&m, atop(*argv++));
   pset(&r, ppowmod(u, n, m));
/*
   fputp(stdout, u); putchar('^');
   fputp(stdout, n); fputs(" mod ", stdout);
   fputp(stdout, m); fputs(" = ", stdout);
   fputp(stdout, r); putchar('\n');
*/
   pdestroy(u);
   pdestroy(n);
   pdestroy(m);
   pdestroy(r);
/*   monitor(0); /* */
   return 0;
}
