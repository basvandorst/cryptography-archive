/*
 * A program to help profile the math package
 */
#include <stdio.h>
#include "precision.h"

main(argc, argv)
   int argc;
   char *argv[];
{
   int size, count, icount;
   precision u = pUndef, v = pUndef, w = pUndef, q = pUndef, r = pUndef;

   
   icount = 1024;
   --argc;
   if (argc > 2 || argc == 0) {
usage:
      fprintf(stderr, "usage: mathprof size [ count ]\n");
      exit(1);
   }
   count = sscanf((--argc, *++argv), "%d", &size);
   if (count != 1) goto usage;
   if (argc > 0) {
      count = sscanf((--argc, *++argv), "%d", &icount);
      if (count != 1) goto usage;
   }

   srand(0);
   do {
      pset(&u, prandnum(size));
      pset(&v, prandnum(size));
      pset(&w, padd(u, v));
      pset(&w, psub(u, v));
      pset(&w, pmul(u, v));
      pdivmod(w, u, &q, &r);
   } while (--icount > 0);

   pdestroy(u); pdestroy(v); pdestroy(w); pdestroy(q); pdestroy(r);
   return 0;
}
