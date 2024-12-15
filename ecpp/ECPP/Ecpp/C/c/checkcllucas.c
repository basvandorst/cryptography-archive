
/* $Log:	checkcllucas.c,v $
 * Revision 1.1  91/09/20  14:53:00  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:25  morain
 * Initial revision
 *  */

/**********************************************************************
                  Checking the N+1 test a` la Cohen-Lenstra
**********************************************************************/

#include "ifactor.h"
#include "exp.h"

#define CLIsOne(x0, x1) (BnnIsOne((x0), Modul) && BnnIsZero((x1), Modul))

/*
   We work over A = (Z/NZ)[T]/(T^2-u*T-a) where
    u = 0, (a/N) = -1        if N = 1 mod 4
    a = 1, ((u^2+4)/N) = -1  if N = 3 mod 4
*/

/* 
   lF2 = [[p1, a1], ..., [pk, ak]]
   Returns 1 if for all i, there exists xi in A s.t.
   xi**((N+1)/pi != 1 but xi**(N+1) == 1.
*/
int CheckCLConditionIII(lm, N, Nl, lF2, u, a)
ListOfBigNum lm;
BigNum N, u, a;
int Nl;
ListOfFactors lF2;
{
    BigNum Nplus1 = BNC(Nl), w = BNC(Nl+1);
    BigMod prod = BmCreate();
    BigMod x0 = BmCreate(), x1 = BmCreate();
    BigMod y0 = BmCreate(), y1 = BmCreate();
    BigMod z0 = BmCreate(), z1 = BmCreate();
    ListOfFactors tmp;
    int prodl = 1, wl, Nplus1l = Nl, pr;
    double tp, runtime();

#ifdef DEBUG
    if(debuglevel >= 4)	printf("%% u=%ld a=%ld\n", *u, *a);
#endif
    BnnAssign(Nplus1, N, Nl);
    if(BnnAddCarry(Nplus1, Nl, 1)){
	BnnSetDigit((Nplus1+Nl), 1);
	Nplus1l++;
    }
    BnnSetDigit(prod, 1);
    tmp = lF2;
    tp = runtime();
    while(tmp != NULL){
	/* computes (N+1)/pi */
#ifdef DEBUG
	if(debuglevel >= 2){
	    printf("%% Let's check for ");
	    BnnPrint(tmp->p, tmp->pl);
	    printf("\n");
	}
#endif
	BnnAssign(w, Nplus1, Nplus1l);
	if(Nplus1l == Nl) BnnSetDigit((w+Nl), 0);
	BnnDivide(w, Nl+1, tmp->p, tmp->pl);
	if(!BnnIsZero(w, tmp->pl)){
	    printf("%%!%% Error in ConditionIII: not a divisor\n");
	    pr = 0;
	    break;
	}
	wl = BnnNumDigits((w+tmp->pl), Nl+1-tmp->pl);
#ifdef DEBUG
	if(debuglevel >= 2){
	    printf("%% (N+1)/pi=");
	    BnnPrint(w+tmp->pl, wl);
	    printf("\n");
	}
#endif
	CLComputeXofNorm1(x0, x1, u, a, lm->n);
#ifdef DEBUG
	if(debuglevel >= 4){
	    printf("%% x(%ld)=", *(lm->n));
	    CLPrint(x0, x1);
	}
#endif
	lm = lm->cdr;
	CLExp(y0, y1, x0, x1, u, a, (w+tmp->pl), wl);
	if(CLIsOne(y0, y1)){
	    printf("%%!%% y=1\n");
	    pr = 0;
	    break;
	}
	else{
#ifdef DEBUG
	    if(debuglevel >= 2){
		printf("%% (y0, y1)=");
		CLPrint(y0, y1);
	    }
#endif
	    /* checks that (y0, y1)**pi = 1 */
	    CLExp(z0, z1, y0, y1, u, a, tmp->p, tmp->pl);
	    if(!CLIsOne(z0, z1)){
		printf("%%!%% N is not a Lucas-psp\n");
#ifdef DEBUG
		if(debuglevel >= 2){
		    printf("%% (z0, z1)=");
		    CLPrint(z0, z1);
		}
#endif
		pr = 0;
		break;
	    }
	}
	ModSubtractDigit(y0, CONSTANTES, 1);
	if(BnnIsZero(y0, Modul))
	    ModMultiply(prod, prod, y1);
	else
	    ModMultiply(prod, prod, y0);
	tmp = tmp->cdr;
    }
    if(pr){
	/* final check on (prod, N) = 1 */
	prodl = BnnNumDigits(prod, Modul);
	wl = BnnGcdWithCopy(w, prod, prodl, N, Nl);
	if(BnnIsOne(w, wl)){
	    pr = 1;
	}
	else{
	    printf("%%!%% Gcd nontrivial, please check\n");
	    BnnPrint(w, wl);
	    printf("\n");
	    pr = 0;
	}
    }
    BnFree(Nplus1); BnFree(w);
    BmFree(prod);
    BmFree(x0); BmFree(x1); BmFree(y0); BmFree(y1); BmFree(z0); BmFree(z1);
    return(pr);
}

