/*--- msqrt.c -- Routine to take the 4 square roots of a number
 *  modulus the product of 2 primes.
 *
 *  Mark Riordan  25 Feb 1991  Based on code by Marc Ringuette.
 *  This code is in the public domain.
 */
 
#include <stdio.h>
#include <time.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"
#include "p.h"

#ifdef DEBUG
#ifdef MSDOS
#define TYPTIME clock_t
#else
#include <sys/time.h>
#include <sys/resource.h>
#define TYPTIME struct rusage
#endif
#endif

#ifdef DEBUG
void display_time P((TYPTIME begtime,TYPTIME endtime, char *msg));
void my_get_time P((TYPTIME *mytime));
#endif
void mberlekamp P((BIGINT *ain, BIGINT *p,/* out */ BIGINT *r));
int mcrt P((int num,BIGINT *resids, BIGINT *mods, /* out */ BIGINT *prodmod, BIGINT *cr));

/*--- function msqrt ------------------------------------------------------
 *
 *  Find the four square roots of a number modulus the product of
 *  two primes.
 *
 *  Entry    a       is a BIGINT.
 *           prime1, prime2   are two primes.
 *           sqrts   is an array of four BIGINTs (values unused).
 *
 *  Exit     sqrts   contains four BIGINTs, each of which satisfies:
 *                   sqrts[i]^2 mod (prime1*prime2) == a.
 */ 
void
msqrt(a,prime1,prime2,sqrts)
BIGINT *a;
BIGINT *prime1;
BIGINT *prime2;
BIGINT sqrts[];
{
   BIGINT modul[2], resid[2], psqrt1, psqrt2, garbage;
#ifdef DEBUG
   TYPTIME begtime, endtime;
#endif

   MCOPY(prime1,&modul[0]);
   MCOPY(prime2,&modul[1]);
#ifdef DEBUG
   my_get_time(&begtime);
#endif
   mberlekamp(a,prime1,&psqrt1);
   mberlekamp(a,prime2,&psqrt2);
#ifdef DEBUG
   my_get_time(&endtime);
   display_time(begtime,endtime,"Two calls to mberlekamp");

   my_get_time(&begtime);
#endif
   zcopy(&psqrt1,&resid[0]);
   zcopy(&psqrt2,&resid[1]);
   mcrt(2,resid,modul,&garbage,&sqrts[0]); 
   zsub(prime1,&psqrt1,&resid[0]);
   mcrt(2,resid,modul,&garbage,&sqrts[1]); 
   zsub(prime2,&psqrt2,&resid[1]);
   mcrt(2,resid,modul,&garbage,&sqrts[2]); 

   zcopy(&psqrt1,&resid[0]);
   mcrt(2,resid,modul,&garbage,&sqrts[3]);
#ifdef DEBUG
   my_get_time(&endtime);
   display_time(begtime,endtime,"Four calls to crt");
#endif
}
