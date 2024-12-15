/*
 * An integer replacement for dc
 *
 * NOTE:
 *    Watch out for function argument evaluation order
 */
#include <ctype.h>
#include <stdio.h>
#include "precision.h"
#ifdef FACTOR
#include "pfactor.h"
#endif

#define STACKSIZE 1024

precision stack[STACKSIZE];
precision *sp = stack - 1;

extern precision psmallFactor();
extern char *malloc();

char hexmap[] = "0123456789abcdef";

precision pop()
{
   precision res = pUndef;

   pset(&res, *sp);
   if (sp < stack) return pzero;
   --sp;
   return presult(res);
}

precision push(p)
   precision p;
{
   *++sp = pUndef;
   pset(sp, p);
   return p;
}

precision xchg(p)
   precision p;
{
   precision temp = pUndef;

   pparm(p);
   pset(&temp, pop());
   push(p);
   pdestroy(p);
   return presult(temp);
}


void clear()
{
   while (sp >= stack) {
      pvoid(pop());
   }
}

main(argc, argv)
   int argc;
   char *argv[];
{
   int ch;
   char *chp;
   unsigned int factCount, size;

#ifdef FACTOR
   pfactorType factors[256];
#endif
   precision temp = pUndef;
   precision rn = pUndef;
   precision b  = pUndef;

   do {
   ch = getchar();
   if (ch == EOF) return 0;
   if (isdigit(ch)) {
      ungetc(ch, stdin);
      push(fgetp(stdin));
   } else if (isspace(ch)) {
      ;
   } else {
      switch (ch) {
      case '@': pshow(push(pop()));
      case '+': push(padd(pop(), pop())); break;
      case '-':
	 ch = getchar();
	 ungetc(ch, stdin);
	 if (isdigit(ch)) {
	    push(pneg(fgetp(stdin)));
	 } else {
	    pset(&temp, pop()); push(psub(pop(), temp));
	 }
	 break;
      case '*': push(pmul(pop(), pop())); break;
      case '/': pset(&temp, pop()); push(pdiv(pop(), temp)); break;
      case '%': pset(&temp, pop()); push(pmod(pop(), temp)); break;
      case '^': pset(&temp, pop()); push(ppow(pop(), temp)); break;
      case 'b': pset(&b, pop());
	   pset(&temp, pop());
	   push(temp);
	   if (pgt(b, utop(16)) || plt(b, ptwo)) {
	      fprintf(stderr, "invalid radix\n");
	   } else {
	      if (peqz(temp)) {
		 putp(temp);
	      } else {
		 if (pltz(temp)) {
		    pset(&temp, pneg(temp));
		    putchar('-');
		 }
		 size = plogb(temp, b) + 1;
		 chp = malloc(size + 1);
		 ptob(temp, chp, size, hexmap, ptou(b));
		 chp[size] = '\0';
		 puts(chp);
		 free(chp);
	      }
	   }
	   break;
      case 'C': clear(); break;
      case 'c': pop(); break;
      case 'd': 
	 pset(&temp, pop()); 
	 push(temp);
	 push(temp);
	 break;
#ifdef FACTOR
      case 'f': pset(&rn, psmallFactor(push(pop()), 0, factors, &factCount));
	   fputp(stdout, push(pop()));
	   fputs(" = ", stdout);
	   pputFactors(stdout, factors, factCount);
	   if pne(rn, pone) {
	      fputs("*(", stdout);
	      fputp(stdout, rn);
	      fputs(")", stdout);
	   }
	   putc('\n', stdout);
	   break;
#endif
      case 'g': pset(&temp, pop()); push(utop(plogb(pop(), temp))); break;
      case 'p': fputp(stdout, push(pop())); putc('\n', stdout); break;
      case 'q': return 0;
      case 'x': push(xchg(pop())); break;
      case 'v': push(psqrt(pop())); break;
      case '!': push(factorial(pop())); break;
      case '?':
	 puts("+ - * / %   2->1   add subtract multiply divide remainder");
	 puts("^ g         2->1   power logb");
	 puts("! v         1->1   factorial sqrt");
	 puts("C *->0   clear stack");
	 puts("b 2->2   print base b");
	 puts("c 1->0   pop");
	 puts("d 1->2   duplicate");
#ifdef FACTOR
	 puts("f 1      factor");
#endif
	 puts("p 1      print");
	 puts("@ 1      pshow");
	 puts("x 2->2   xchange");
	 puts("?        help");
	 puts("q        quit");
	 break;
      }
   }
   } while (1);
}
