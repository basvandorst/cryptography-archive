
/* $Log:	hacks.c,v $
 * Revision 1.1  91/09/20  14:54:18  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:55  morain
 * Initial revision
 *  */

/**********************************************************************
                   Particular routines for ECPP
**********************************************************************/

#include "ecpp.h"

/* Particular Extended Euclidean Routine with jump in case of factor */
/* Uses Gbuf */
/* y <- 1/x mod (Modu, Modul) */
int EcppEea(y, p_yl, x, xl)
BigNum y, x;
int *p_yl, xl;
{
    int gl;

    if(!BnnEeaWithCopy(Gbuf, &gl, y, p_yl, Modu, Modul, x, xl)){
	printf("Error, non invertible\n");
	printf("Factor is "); BnnPrint(Gbuf, gl); printf("\n");
	printf("Should have jumped at this point, man \n");
	exit(0);
    }
    return(1);
}

EcppMdSqrt(y, p_yl, a, al)
BigNum y, a;
int *p_yl, al;
{
    int tl;

    if(tshanksz[depth] == NULL){
	tshanksz[depth] = BmCreate();
	ModSetToZero(tshanksz[depth]);
	tl = 1;
    }
    else tl = BnnNumDigits(tshanksz[depth], Modul);
#ifdef DEBUG
    if(BnGetSize(tshanksz[depth]) != Modul+1) printf("%%!%% EcppMdSqrt\n");
#endif
    return(MdSqrtZ(y, p_yl, a, al, tshanksz[depth], &tl));
}

BnCheckAndFree(n)
BigNum n;
{
#ifdef DEBUG
    if(debuglevel >= 10)
	printf("%%* %x\n", n);
#endif
    BnFree(n);
}
