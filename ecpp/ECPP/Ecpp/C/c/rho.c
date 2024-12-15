
/* $Log:	rho.c,v $
 * Revision 1.1  91/09/20  14:55:15  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:04  morain
 * Initial revision
 *  */

/**********************************************************************
                               Pollard rho
**********************************************************************/

#include "ifactor.h"

#ifdef MONTY
#define McfModMultiply(p, n, m) 
#define McfInitialize(n) BmInit((n))
#else
#define McfModMultiply(p, n, m) ModMultiply((p), (n), (m))
#define McfInitialize(n) BmInit((n))
#endif

#define McfG(z, x, cte) {McfModMultiply((z), (x), (x)); \
			 ModAddDigit((z), (cte), 0);}

int rho(p_lfact, itmax, nbgcd)
ListOfFactors *p_lfact;
long itmax;
int nbgcd;
{
    BigNumDigit cte[1];
    BigMod y = BmCreate(), z = BmCreate(), t1 = BmCreate();
    BigMod t2 = BmCreate(), x = BmCreate(), x0 = BmCreate();
    BigMod a1 = BmCreate(), a2 = BmCreate(), a3 = BmCreate();
    BigMod a4 = BmCreate(), w = BmCreate(), g = BmCreate();
    BigMod accu = BmCreate();
    long e = 1, f = 1, ef;
    int i = 1, c = 0, gl, done = 0, accul;
    double tp, runtime();

    tp = runtime();
    printf("%% Entering Rho with itmax=%ld nbgcd=%d\n", itmax, nbgcd);
    BnnSetDigit(cte, 3);
    /* z <- G(x0^2) */
    McfG(z, x0, cte);
    McfModMultiply(y, y, y);
    ModAssign(t1, z);
    ModAssign(t2, x0);
    McfG(x, y, cte);
    BnnSetDigit(accu, 1);
    while(1){
	/* precondition again */
	ModAssign(a1, t1); ModAdd(a1, t2); ModAssign(a2, a1);
	ModAdd(a2, z); McfModMultiply(a4, t1, t2); McfModMultiply(a3, a1, z);
	ModAdd(a3, a4);	ModAssign(a4, y); ModAdd(a4, a3);
	McfModMultiply(a4, a1, a4);
	ef = e+f; e = f; f = ef;
	if(debuglevel >= 8) printf("e=%ld f=%ld\n", e, f);
	ModAssign(t2, t1); ModAssign(t1, z);
	/* main loop */
	if(f > itmax) f = itmax;
	while(i < f){
	    ModAssign(w, x); ModSubtract(w, a2); McfModMultiply(y, x, x);
	    /* TEST */
	    ModAssign(z, y); ModAdd(z, a3); McfModMultiply(z, z, w);
	    ModAdd(z, a4); McfModMultiply(accu, accu, z);
	    c++;
	    if(c == nbgcd){
		accul = BnnNumDigits(accu, Modul);
		if(!IsGcdTrivial(g, &gl, accu, accul, Modu, Modul)){
		    if(done = FactorCheck(g, gl, p_lfact)){
			goto endofrho;
		    }
		    else{
			/* we must reduce all the stuff */
			ModMod(x, Modul); ModMod(y, Modul); 
			ModMod(a2, Modul); ModMod(a3, Modul);
			ModMod(a4, Modul); ModMod(t1, Modul); 
			ModMod(t2, Modul);
		    }
		}
		ModSetToZero(accu);
		BnnSetDigit(accu, 1);
		c = 0;
	    }
	    McfG(x, y, cte);
	    i++;
	}
	if(f >= itmax){
	    if(c){
		accul = BnnNumDigits(accu, Modul);
		if(!IsGcdTrivial(g, &gl, accu, accul, Modu, Modul))
		    done = FactorCheck(g, gl, p_lfact);
	    }
	    goto endofrho;
	}
	else{
	    /* recover last x */
	    ModAssign(z, w);
	    ModAdd(z, a2);
	}
    }
endofrho:
    BnFree(y); BnFree(z); BnFree(t1); BnFree(t2); BnFree(x); BnFree(x0);
    BnFree(a1); BnFree(a2); BnFree(a3); BnFree(a4); BnFree(w); BnFree(g);
    BnFree(accu);
    printf("%% Time for rho is "); 
    printf("%f\n", (runtime()-tp)/1000.);
    return(done);
}
