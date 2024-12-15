
/* $Log:	pollard.c,v $
 * Revision 1.1  91/09/20  14:54:54  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:38  morain
 * Initial revision
 *  */

/**********************************************************************
                            Pollard p-1 method
**********************************************************************/

#include <stdio.h>
#include "ifactor.h"
#include "exp.h"
#include "gcd.h"

#define DIFNPMAX 60

/* Tries to factor (Modu, Modul) */
/* Uses file which contains d_i(p_(i+1)-p_i)/2 */
/* Computes 2**M where M= prod p_i**a_i, p_i**a_i <= b1 */
/* assumes that b1 is an int */
int pollard(p_lfact, b1, b2, nbgcd, difnp_file)
long b1, b2;
int nbgcd;
char *difnp_file;
ListOfFactors *p_lfact;
{
    FILE *fopen(), *file;
    BigMod a = BmCreate(), g = BmCreate();
    BigNumDigit e[1];
    BigNumCarry br;
    double runtime(), tp;
    int dp, p = 1, m, q, nbp = 0, done, triv;
    int el = 1, al = 1, gl;

    if(!b1) return(0);
    tp = runtime();
    printf("%% Entering P-1 with b1=%ld b2=%ld nbgcd=%d\n", b1, b2, nbgcd);
    BnnSetDigit(a, 2);
    q = 2; m = b1/2; while(q <= m) q <<= 1;
#ifdef DEBUG
    if(debuglevel >= 7) printf("p=2 q=%d\n", q);
#endif
    BnnSetDigit(e, q);
    BmExp(a, &al, a, al, e, el);
    file = fopen(difnp_file, "r");
    while((fscanf(file, "%d", &dp) != EOF)){
	p += (dp << 1);
	if(p <= b1){
	    nbp++;
#ifdef DEBUG
	    if((debuglevel >= 7) && (!(nbp % 100)))
		printf("%d-th prime reached is %d\n", nbp, p);
#endif
	    m = b1/p; q = p;
	    while(q <= m) q *= p;
#ifdef DEBUG
	    if(debuglevel >= 7) printf("p=%d q=%d\n", p, q);
#endif
	    BnnSetDigit(e, q);
	    BmExp(a, &al, a, al, e, el);
#ifdef DEBUG
	    if(debuglevel >= 7)	
		{printf("a="); BnnPrint(a, al); printf("\n");}
#endif
	    if(!(nbp % nbgcd)){
		br = BnnSubtractBorrow(a, al, 0);
		if(!br)
		    printf("Error in pollard, a = -1 since %d\n", (int)br);
#ifdef DEBUG
		if(debuglevel >= 7) 
		    {printf("a-1=[%d]", al); BnnPrint(a, al); printf("\n");}
#endif
		if(!(triv = IsGcdTrivial(g, &gl, a, al, Modu, Modul))){
		    if(done = FactorCheck(g, gl, p_lfact)){
			printf("%% Time for step 1 is "); 
			printf("%f\n", (runtime()-tp)/1000.);
			goto endofpollard;
		    }
		    else{
			BnnAddCarry(a, al, 1);
			al = ModMod(a, al);
		    }
		}
		if(triv){
		    if(BnnAddCarry(a, al, 1))
			printf("Error in pollard, a = 0\n");
		}
	    }
	}
	else break;
    }
    if(nbp % nbgcd){
	BnnSubtractBorrow(a, al, 0);
	if(!(triv = IsGcdTrivial(g, &gl, a, al, Modu, Modul))){
	    if(done = FactorCheck(g, gl, p_lfact)){
		printf("%% Time for step 1 is %f\n", (runtime()-tp)/1000.);
		goto endofpollard;
	    }
	    else{
		BnnAddCarry(a, al, 1);
		al = ModMod(a, al);
	    }
	}
	if(triv) BnnAddCarry(a, al, 1);
    }
    tp = runtime();
    done = pollardstep2(a, al, p_lfact, file, p, b2, nbgcd);
    printf("%% Time for step 2 is %f\n", (runtime()-tp)/1000.);
endofpollard:
    fclose(file);
#ifdef DEBUG
    if(debuglevel >= 10) printf("Done(p-1)=%d\n", done);
#endif
    BnFree(a); BnFree(g);
    return(done);
}

/* Performs the standard 2nd phase */
/* p is the least prime greater than b1 */
int pollardstep2(a, al, p_lfact, file, p, b2, nbgcd)
BigMod a;
FILE *file;
ListOfFactors *p_lfact;
int al, p, b2, nbgcd;
{
    BigMod prod = BmCreate(), g = BmCreate();
    BigMod tabpower[DIFNPMAX];
    BigNumCarry br;
    int bl, i, dp, prodl = 1, nbp = 0, done, gl, triv, tpl;

    printf("%% P-1: entering step 2\n");
    /* tabpower[i] = b**(i+1), where b = a**2 */
    tabpower[0] = BmCreate();
    BmSquare(tabpower[0], &bl, a, al);
    tpl = bl;
    for(i = 1; i < DIFNPMAX; i++){
	tabpower[i] = BmCreate();
	BmMultiply(tabpower[i], &tpl, tabpower[i-1], tpl, tabpower[0], bl);
    }
    BnnSetDigit(prod, p);
    BmExp(a, &al, a, al, prod, 1);
    BnnAssign(prod, a, al);
    prodl = al;
    BnnSubtractBorrow(prod, prodl, 0);
    if(BnnIsDigitZero(*(prod+prodl-1))) prodl--;
    while((fscanf(file, "%d", &dp) != EOF) && (p <= b2)){
	p += (dp << 1);
	if(p <= b2){
	    nbp++;
#ifdef DEBUG
	    if(debuglevel >= 7) printf("p=%d\n", p);
#endif
	    BmMultiply(a, &al, a, al, tabpower[dp-1], Modul);
	    br = BnnSubtractBorrow(a, al, 0);
	    if(!br)
		printf("Error in pollard, a = -1 since %d\n", (int)br);
	    BmMultiply(prod, &prodl, prod, prodl, a, al);
	    if(BnnAddCarry(a, al, 1))
		printf("Error in pollard, a = 0\n");
	    if(!(nbp % nbgcd)){
		if(!(triv=IsGcdTrivial(g, &gl, prod, prodl, Modu, Modul))){
		    bl = Modul;
		    if(done = FactorCheck(g, gl, p_lfact))
			return(done);
		    else{
			BnnSetToZero(prod, prodl);
			BnnSetDigit(prod, 1); 
			prodl = 1;
			al = ModMod(a, al);
			for(i = 0; i < DIFNPMAX; i++)
			    ModMod(tabpower[i], bl);
		    }
		}
	    }
	}
    }
    if(nbp % nbgcd){
	printf("%% Performing last gcd in step 2\n");
	if(!IsGcdTrivial(g, &gl, prod, prodl, Modu, Modul)){
	    return(FactorCheck(g, gl, p_lfact));
	}
    }
    BnFree(prod); BnFree(g);
    for(i = 0; i < DIFNPMAX; i++)
	BnFree(tabpower[i]);
    return(0);
}
