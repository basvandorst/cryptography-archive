#include <stdio.h>
#include "precision.h"

main(argc, argv)
   int argc;
   char *argv[];
{
   precision u = pUndef, n = pUndef;

   --argc, ++argv;
   if (argc == 0) {
      fprintf(stderr, "usage pow precision integer\n");
      exit(1);
   }
   pset(&u, atop(*argv++));
   pset(&n, atop(*argv++));
   fputp(stdout, u); putchar('^');
   fputp(stdout, n); fputs(" = ", stdout);
   fputp(stdout, ppow(u, n)); putchar('\n');
   pdestroy(u);
   pdestroy(n);
   return 0;
}
