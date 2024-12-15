#include <stdio.h>
#include "precision.h"

main(argc, argv)
   int argc;
   char *argv[];
{
   precision u = pUndef, v = pUndef, w = pUndef, q = pUndef, r = pUndef;
   int count, size1, size2, res;

   if (--argc != 3) {
usage: fprintf(stderr, "usage: randtest iterationCount, size1 size2\n");
       exit(1);
   }
   res = sscanf(*++argv, "%d", &count);
   if (res != 1) goto usage;
   res = sscanf(*++argv, "%d", &size1);
   if (res != 1) goto usage;
   res = sscanf(*++argv, "%d", &size2);
   if (res != 1) goto usage;
   do {
      pset(&u, prandnum(size1));
      pset(&v, prandnum(size2));
      pset(&q, padd(u, v));
      pset(&w, psub(q, v));
      if (pne(w, u)) {
	 fputp(stdout, q);
         fputs(" - ", stdout);
	 fputp(stdout, v);
	 fputs(" = ", stdout);
	 fputp(stdout, w);
	 fputs(" != ", stdout);
	 fputp(stdout, u);
	 putc('\n', stdout);
      }
      pset(&u, prandnum(size1 * 2));
      pdivmod(u, v, &q, &r);
      pset(&w, padd(pmul(q, v), r));
      if (pne(w, u)) {
	 fputp(stdout, q);
	 fputs(" * ", stdout);
	 fputp(stdout, v);
	 fputs(" + ", stdout);
	 fputp(stdout, r);
	 fputs(" = ", stdout);
	 fputp(stdout, w);
	 fputs(" != ", stdout);
	 fputp(stdout, u);
	 putc('\n', stdout);
      }
   } while (--count > 0);

   pdestroy(u);
   pdestroy(v);
   pdestroy(w);
   pdestroy(q);
   pdestroy(r);
   return 0;
}
