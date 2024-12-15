#include <stdio.h>
#include "pdefs.h"
#include "precision.h"

extern int errno;

int	debug = 0;
char	*myname;

/*
 * BSD returns 32 bit, HPUX 16-bit
 */
#define	randnum(x)	(rand(0) & 0x7fff)

main(argc, argv)
   int argc;
   char *argv[];
{
   extern int optind;
   extern char *optarg;

   int  i, j, k, l, m, n, ch;
   char *chp;

   int  hi = 32;
   int  lo = -hi;

   precision u = pUndef, v = pUndef, w = pUndef; 
   precision x = pUndef, q = pUndef, r = pUndef;

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

   srand(1);			/* set random seed */

   if (argc) {
      chp = *argv++;
      argc--;
   }
   if (argc) {
      sscanf(*argv, "%d", &i);
      argc--, argv++;
   }

   printf("ptoa test\n");
   for (i = lo; i < hi ; i++) {
      pset(&w,  itop(i));
      chp = ptoa(w);
      if (sscanf(chp, "%d", &k) != 1 ||  k != i) {
	 printf("%d = \"%s\"\n", i, chp);
      }
      deallocate(chp);
   }

   printf("random iteration test (itop, ptoi, padd, pmul)\n");
   for (l = 0; l < 1024; l++) {
      i = randnum(0) - 16384;	/* 15 bit signed integer */
      pset(&u, itop(i));
      m = ptoi(u);
      if (m != i) {
	 printf("failed: %ld returned by ptoi, should be %ld\n", m, i);
      }
      j = randnum(0) - 16384;	/* 15 bit signed integer */
      pset(&v, itop(j));
      n = ptoi(v);
      if (n != j) {
	printf("failed: %ld returned by ptoi, should be %ld\n", n, k);
      }
      pset(&w, padd(u, v));
      k = ptoi(w);
      if (k != i + j) {
	 printf("%d + %d = %d\n", i, j, k);
      }
      pset(&w, pmul(u, v));
      k = ptoi(w);
      if (k != i * j) {
	 printf("%d * %d = %d\n", i, j, k);
      }
   }

   printf("hi precision fn/fninverse random iteration test \n");
   for (l = 0; l < 1024; l++) {
      pset(&u, prandnum(4));
      pset(&v, prandnum(4));
      pset(&w, padd(u, v));
      pset(&x, psub(w, v));
      if (pne(u, x)) {
	 fputp(stdout, u);
	 printf(" + ");
	 fputp(stdout, v);
	 printf(" = ");
	 fputp(stdout, w);
	 printf(" inverse = ");
	 fputp(stdout, x);
	 putchar('\n');
      }
      pset(&u, prandnum(4));
      pset(&v, prandnum(4));
      pset(&w, pmul(u, v));
      pdivmod(w, v, &q, &r);
      if (pne(u, q) || pnez(r)) {
	 fputp(stdout, u);
	 printf(" * ");
	 fputp(stdout, v);
	 printf(" = ");
	 fputp(stdout, w);
	 printf(" inverse = ");
	 fputp(stdout, q);
	 printf(" remainder = ");
	 fputp(stdout, r);
	 putchar('\n');
      }
   }

   printf("psqrt test\n");
   for (i = 0; i < hi; i++) {
      pset(&u, itop(i));
      pset(&w, psqrt(u));
      k = ptoi(w);
      if (k * k > i ||  k * (k + 2) < i) {
	 printf("%d^2 = %d\n", k, i);
      }
   }

   printf("pcmp test\n");
   for (i = lo; i < hi ; i++) {
      for (j = lo; j < hi; j++) {
	 pset(&u, itop(i));
	 pset(&v, itop(j));
	 k = pcmp(u, v);
	 if ((k < 0)  != (i < j)
	 ||  (k > 0)  != (i > j)
	 ||  (k == 0) != (i == j)) {
	    printf("%d cmp %d = %d\n", i, j, k);
	 }
      }
   }
   printf("padd test\n");
   for (i = lo; i < hi ; i++) {
      for (j = lo; j < hi; j++) {
	 pset(&u, itop(i));
	 pset(&v, itop(j));
	 pset(&w, padd(u, v));
	 k = ptoi(w);
	 if (k != i + j) {
	    printf("%d + %d = %d\n", i, j, k);
	 }
      }
   }
   printf("psub test\n");
   for (i = lo; i < hi ; i++) {
      for (j = lo; j < hi; j++) {
	 pset(&u, itop(i)); 
	 pset(&v, itop(j));
	 pset(&w, psub(u, v));
	 k = ptoi(w);
	 if (k != i - j) {
	    printf("%d - %d = %d\n", i, j, k);
	 }
      }
   }

   printf("pmul test\n");
   for (i = lo; i < hi ; i++) {
      for (j = lo; j < hi; j++) {
	 pset(&u, itop(i));
	 pset(&v, itop(j));
	 pset(&w, pmul(u, v));
	 k = ptoi(w);
	 if (k != i * j) {
	    printf("%d * %d = %d\n", i, j, k);
	 }
      }
   }

   printf("pdiv test (machine dependent)\n");
   for (i = lo; i < hi ; i++) {
      for (j = lo; j < hi; j++) {
	 if (j != 0) {
	    pset(&u, itop(i));
	    pset(&v, itop(j));
	    pdivmod(u, v, &q, &r);
	    m = ptoi(q);
	    n = ptoi(r);
	    if (i / j != m || i % j != n) {
	       printf("%d / %d = %d + %d\n", i, j, m, n);
	    }
	 }
      }
   }

   pdestroy(r);
   pdestroy(q);
   pdestroy(x);
   pdestroy(w);
   pdestroy(v);
   printf("check storage test\n");
   printf("should show value +0, 1 digit left\n");
   pset(&u, pzero);
   pshow(u);
   pdestroy(u);

   printf("Cache:\nsize length\n");
   phistogram(stdout);

   return 0;
}
