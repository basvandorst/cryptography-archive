/*--- mberleka.c -- Compute a square root modulo a prime.
 *
 *  Written by Bennet Yee on 12-Jul-1989.
 */

#include <stdio.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"
#include "p.h"

#define  SANITY   1

#define  DEBUG 0
#if   DEBUG
# include   <stdio.h>
#endif

void zrandom P((BIGINT *s,BIGINT *n));

/*-- function mberlekamp ----------------------------------------------
 *
 * Compute a square root, mod a prime number, of a BIGINT.
 *
 *  Entry:  ain   is a quadratic residue BIGINT of p
 *          p     is a BIGINT odd prime.
 *
 *  Exit:   r     is the square root of ain mod p; that is,
 *                r*r mod p = ain mod p
 *
 * Square root algorithm due to Berlekamp.
 *
 * Pre: p odd prime, jacobi(a,p) = 1
 *
 * See pg 49 of
 * Lecture Notes on the Complexity of Some Problems in Number Theory
 * by Dana Angluin, Yale CS TR 243 Aug '82.
 */
void
mberlekamp(ain,p,r)
BIGINT   *ain,*p,/* out */ *r;
{
   BIGINT   gamma, minustwogamma, gamma_sq_minus_a, acopy, *a;
   BIGINT   linear, constterm;
   BIGINT   r_linear, r_const;
   BIGINT   tmp_q, tmp_c, tmp_l;
   BIGINT   p_copy, one;

#if   DEBUG
   puts("Entered mberlekamp");
#endif

   MINIT(&acopy);
   a = &acopy;
   MCOPY(ain,a);
   /* MRR added because precondition a<p is not always met. */
   zmod(a,p,a);

   if (!ztest(a)) {
      MFREE(r);
      MSET(0,r);
      return;
   }
   /* 0 < a < p */
   if ((zlowbits(p) & 0x3) == 3) {
#if   DEBUG
      puts("oh boy! p = 3 mod 4 means -1 is nonresidue");
#endif
      /*
       * we know that -1 is a nonresidue, so -a is a nonreside also
       */
      MSET(0,&gamma);
      MINIT(&gamma_sq_minus_a);
      zsub(p,a,&gamma_sq_minus_a);
   } else {
#if   DEBUG
      puts("Looking for a nonresidue...");
#endif
      /* probabilistically find a quadratic non-residue */
      MINIT(&gamma);
      MINIT(&gamma_sq_minus_a);
      do {
#if   DEBUG2
         putchar('x'); fflush(stdout);
#endif
         zrandom(&gamma,p);
#if   DEBUG
         fputs("trying gamma: ",stdout); zwrite(&gamma); putchar('\n');
#endif
         /* 0 <= gamma < p */
         zmulmod(&gamma,&gamma,p,&gamma_sq_minus_a);
         /* MRR  Avoid negative numbers in computing  gamma^2 - a  */
         if(zcompare(&gamma_sq_minus_a,a) < 0) {
            zsub(a,&gamma_sq_minus_a,&gamma_sq_minus_a);
            zsub(p,&gamma_sq_minus_a,&gamma_sq_minus_a);
         } else {
#if 0
            zsubmod(&gamma_sq_minus_a,a,p,&gamma_sq_minus_a);
#else
            zsub(&gamma_sq_minus_a,a,&gamma_sq_minus_a);
#endif
         }
#if   DEBUG
         fputs("trying gamma_sq_minus_a: ",stdout); zwrite(&gamma_sq_minus_a); putchar('\n');
#endif
      } while (zjacobi(&gamma_sq_minus_a,p) != -1);
   }
#if   DEBUG
   fputs("\ngamma: ",stdout); zwrite(&gamma); putchar('\n');
   fputs("gamma_sq_minus_a: ",stdout); zwrite(&gamma_sq_minus_a); putchar('\n');
#endif
   /*
    * now gamma_sq_minus_a = gamma^2-a is a quadratic nonresidue,
    * and we proceed to compute
    * x^{(p-1)/2}-1 mod ((x-gamma)^2-a).
    *
    * Key observation:  roots of g(x)=((x-gamma)^2-a) are
    * rho and sigma where
    * rho = gamma + sqrt(a), sigma = gamma - sqrt(a).
    * and so rho*sigma = gamma^2-a.
    * Since we know that legendre(gamma^2-a,p)=-1,
    * wlog sigma is a quadratic nonresidue,
    * and rho is a quadratic residue.
    * Now, since f(x)=x^{(p-1)/2}-1 has all quadratic residues as roots,
    * gcd(f(x),g(x)) = delta*(x-rho).
    * Since deg(f(x))>=deg(g(x)),
    * computing f(x) mod g(x) gives delta*(x-rho).
    */
   MCOPY(&gamma,&minustwogamma);
   zshiftl(&minustwogamma,1);
#if 0
   zsub(p,&minustwogamma,&minustwogamma);
#else
   /* MRR */
   zsubmod(p,&minustwogamma,p,&minustwogamma);
#endif
#if   DEBUG
   fputs("-2*gamma: ",stdout); zwrite(&minustwogamma); putchar('\n');
#endif
   MCOPY(p,&p_copy);
   zshiftr(&p_copy,1);/* (p-1)/2 */
   MSET(1,&linear); MSET(0,&constterm);
   MSET(0,&r_linear); MSET(1,&r_const);
   MINIT(&tmp_q); MINIT(&tmp_l); MINIT(&tmp_c);
   /* x */
   if (ztest(&p_copy)) for (;;) {
#if   DEBUG1
      putchar('.'); fflush(stdout);
#endif
      if (zparity(&p_copy)) {
#if   DEBUG
# define SHOW(x) fputs("x is ",stdout); zwrite(x); putchar('\n')
         puts("bit set");
#else
# define SHOW(x)
#endif
         zmulmod(&linear,&r_linear,p,&tmp_q);
         SHOW(&tmp_q);
         zmulmod(&linear,&r_const,p,&tmp_c);
         SHOW(&tmp_c);
         zmulmod(&constterm,&r_linear,p,&tmp_l);
         SHOW(&tmp_l);
         zaddmod(&tmp_l,&tmp_c,p,&tmp_l);
         SHOW(&tmp_l);
         zmulmod(&constterm,&r_const,p,&tmp_c);
         SHOW(&tmp_c);

         zmulmod(&tmp_q,&minustwogamma,p,&r_linear);
         SHOW(&r_linear);
         zmulmod(&tmp_q,&gamma_sq_minus_a,p,&r_const);
         SHOW(&r_const);
         zsubmod(&tmp_l,&r_linear,p,&r_linear);
         SHOW(&r_linear);
         zsubmod(&tmp_c,&r_const,p,&r_const);
         SHOW(&r_const);
      }
#if   DEBUG
      fputs("res linear term is ",stdout); zwrite(&r_linear); putchar('\n');
      fputs("res const term is ",stdout); zwrite(&r_const); putchar('\n');
      fputs("poly linear term is ",stdout); zwrite(&linear); putchar('\n');
      fputs("poly const term is ",stdout); zwrite(&constterm); putchar('\n');
      fputs("p is ",stdout); zwrite(p); putchar('\n');
#endif
      /* drop low bit */
      zshiftr(&p_copy,1);
      /* square the polynomial */
      if (ztest(&p_copy)) {
         zmulmod(&linear,&linear,p,&tmp_q);
         SHOW(&tmp_q);
         zmulmod(&linear,&constterm,p,&tmp_l);
         SHOW(&tmp_l);
         zshiftl(&tmp_l,1);
         zmod(&tmp_l,p,&tmp_l);
         SHOW(&tmp_l);
         zmulmod(&constterm,&constterm,p,&tmp_c);
         SHOW(&tmp_c);

         zmulmod(&tmp_q,&minustwogamma,p,&linear);
         SHOW(&linear);
         zmulmod(&tmp_q,&gamma_sq_minus_a,p,&constterm);
         SHOW(&constterm);
         zsubmod(&tmp_l,&linear,p,&linear);
         SHOW(&linear);
         zsubmod(&tmp_c,&constterm,p,&constterm);
         SHOW(&constterm);
      } else break;
   }
   MSET(1,&one);
   zsubmod(&r_const,&one,p,&r_const);
#if   DEBUG
   fputs("\nlinear term is ",stdout); zwrite(&r_linear); putchar('\n');
   fputs("const term is ",stdout); zwrite(&r_const); putchar('\n');
#endif
   if (zcompare(&one,&r_linear)) {
      zinv(&r_linear,p,&r_linear);
#if   SANITY
      if (!zcompare(&one,&r_linear)) {
         zhalt("mberlekamp:  prime is NOT!");
      }
#endif
      zmulmod(&r_linear,&r_const,p,&r_const);
   }
   zsub(p,&r_const,&r_const);
   /* r_const now contains rho where residue = delta(x-rho) */
#if   DEBUG
   fputs("rho: ",stdout); zwrite(&r_const); putchar('\n');
#endif
   /* rho - gamma is a root of x^2 - a */
   zsubmod(&r_const,&gamma,p,r);
   
   MFREE(&gamma); MFREE(&minustwogamma); MFREE(&gamma_sq_minus_a);
   MFREE(&linear); MFREE(&constterm);
   MFREE(&r_linear); MFREE(&r_const);
   MFREE(&tmp_q); MFREE(&tmp_l); MFREE(&tmp_c);
   MFREE(&p_copy); MFREE(&one);
   MFREE(&acopy);
}

/*
 * HISTORY
 *
 * 12-Jul-89  Bennet Yee (bsy) at CMU
 * Created.  CODE NOT DERIVED FROM MP(3X).
 *
 * 09-Feb-91  Mark Riordan  riordanmr@clvax1.cl.msu.edu  17362MRR@MSU.BITNET
 * Cosmetic changes and conversion to zbigint package.
 */
/*
 * Copyright (C) 1987, Bennet Yee.  You may freely distribute this code as long
 * as this notice is intact.  If you make any modifications in your
 * distribution, you must clearly document them.  If you distribute this code
 * or any derivative thereof, it must be licensed at no charge to all parties
 * on terms identical to those described herein.  All distributions must
 * contain source code.
 *
 * This software is provided with absolutely no warranty.
 *
 * If you have questions, you can contact me at bsy@a.cs.cmu.edu
 */
