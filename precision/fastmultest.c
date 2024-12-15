#include <stdio.h>
#include "precision.h"

extern long time();

main(argc, argv)
   int argc;
   char *argv[];
{
   precision u = pUndef, v = pUndef, w = pUndef;
   long stime, etime;
   register int count;
   int loopcount = 100;
   unsigned int size, i;

   (--argc, argv++);
   count = sscanf((--argc, *argv++), "%u", &size);
   count = sscanf((--argc, *argv++), "%u", &i);

   pset(&u, prandnum(size));
   pset(&v, prandnum(size));
   count = loopcount;
   stime = time((long  *) 0);
   do {
      pset(&w, pfastmul(u, v, i));
   } while (--count > 0);
   etime = time((long  *) 0) - stime;
   printf("-- %d %lu\n", i, etime);
   pdestroy(u);
   pdestroy(v);
   pdestroy(w);
   return 0;
}
