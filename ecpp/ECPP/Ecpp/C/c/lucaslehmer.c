
/* $Log:	lucaslehmer.c,v $
 * Revision 1.1  91/09/20  14:54:33  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:12  morain
 * Initial revision
 *  */

/**********************************************************************
 		A lot of goodies from the article
 
	   J. Brillhart, D. H. Lehmer, J. L. Selfridge

	   New primality criteria and factorizations of 2^m+/-1

	   Math. Comp., vol 29, n 130, April 1975

**********************************************************************/

/* We use exclusively the notations of the paper */

#include "exp.h"
#include "ifactor.h"

/* For the N-1 test */
int LucasApplyTh3(N, Nl, p, pl)
BigNum N, p;
int Nl, pl;
{
    /* quick test for 2 * p + 1 > sqrt(N) */
    if((pl << 1)-1 > Nl){
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% 2 * pl - 1 > Nl\n");
#endif
	return(1);
    }
    else{
	int al = pl+1, cl = (pl+1) << 1, cmp;
	BigNum a = BNC(al), c = BNC(cl);

#ifdef DEBUG
	if(debuglevel >= 4) printf("%% Checking by computing sqrt(N)\n");
#endif
	BnnAssign(a, p, pl);
	BnnShiftLeft(a, al, 1);
	BnnAddCarry(a, al, 1);
	al = BnnNumDigits(a, al);
	if((al << 1) > cl) printf("%%!%% Error in LucasApplyTh3\n");
	BnnMultiply(c, cl, a, al, a, al);
	cmp = (BnnCompare(c, cl, N, Nl) == 1);
	BnFree(a); BnFree(c);
	return(cmp);
    }
}

/* For the N+1 test */
int LucasApplyTh15(N, Nl, p, pl)
BigNum N, p;
int Nl, pl;
{
    /* quick test for 2 * p - 1 > sqrt(N) */
    if((pl << 1)-1 > Nl){
#ifdef DEBUG
	if(debuglevel >= 4) printf("%% 2 * pl - 1 > Nl\n");
#endif
	return(1);
    }
    else{
	int al = pl+1, cl = (pl+1) << 1, cmp;
	BigNum a = BNC(al), c = BNC(cl);

#ifdef DEBUG
	if(debuglevel >= 4) printf("%% Checking by computing sqrt(N)\n");
#endif
	BnnAssign(a, p, pl);
	BnnShiftLeft(a, pl, 1);
	BnnSubtractBorrow(a, pl, 0);
	al = BnnNumDigits(a, pl);
	if((al << 1) > cl) printf("%%!%% Error in LucasApplyTh3\n");
	BnnMultiply(c, cl, a, al, a, al);
	cmp = BnnCompare(p, pl, a, al);
	BnFree(a);
	return(cmp);
    }
}

/* Computes F = prod(lF) and divides R by F */
int ComputeCofactor(R, p_Rl, F, p_Fl, lF)
BigNum R, F;
int *p_Rl, *p_Fl;
ListOfFactors lF;
{
    BigNum buf, acc = BNC(*p_Rl+1);
    ListOfFactors tmp;
    int Rl = *p_Rl, Fl = 1, i, accl;

    BnnSetDigit(F, 1);
    tmp = lF;
    while(tmp != NULL){
	for(i = 1; i <= tmp->e; i++){
	    accl = Fl+(tmp->pl)+1;
	    BnnSetToZero(acc, accl);
	    BnnMultiply(acc, accl, F, Fl, tmp->p, tmp->pl);
	    buf = F; F = acc; acc = buf;
	    Fl = BnnNumDigits(F, accl);
	}
	tmp = tmp->cdr;
    }
#ifdef DEBUG
    printf("F=%s"); BnnPrint(F, Fl); printf("\n");
#endif
    BnnDivide(R, Rl+1, F, Fl);
    if(!BnnIsZero(R, Fl)){
	printf("Error, F does not divide R\n");
	exit(0);
    }
    Rl = Rl - Fl + 1;
    BnnAssign(R, (R+Fl), Rl);
    BnnSetToZero((R+Rl), Fl);
    Rl = BnnNumDigits(R, Rl);
#ifdef DEBUG
    printf("R=%s\n"); BnnPrint(R, Rl); printf("\n");
#endif
    BnFree(acc);
    *p_Rl = Rl; *p_Fl = Fl;
}

/* Tests various conditions of primality using factorization of N-1 */
int LucasNminus1(N, Nl, lF1)
BigNum N;
int Nl;
ListOfFactors lF1;
{
    BigNum F1 = BNC(Nl+1), R1 = BNC(Nl+1);
    ListOfBigNum la;
    int F1l, R1l = Nl, pr;

    BnnAssign(R1, N, Nl);
    /* no underflow since N is odd */
    BnnSubtractBorrow(R1, R1l, 0);
    ComputeCofactor(R1, &R1l, F1, &F1l, lF1);
    if(BnnIsOne(R1, R1l)){
	/* N-1 is completely factorized */
	printf("%% N-1 is completely factored\n");
	pr = Theorem1(&la, N, Nl, lF1);
    }
    else{
	pr = Theorem5(N, Nl, F1, F1l, R1, R1l, lF1);
    }
    BnFree(F1); BnFree(R1);
    return(pr);
}

/* Tests various conditions of primality using factorization of N+1 */
int LucasNplus1(N, Nl, lF2)
BigNum N;
int Nl;
ListOfFactors lF2;
{
    BigNum F2 = BNC(Nl+1), R2 = BNC(Nl+2);
    int F2l, R2l = Nl, pr;

    /* R2 <- N+1 */
    BnnAssign(R2, N, Nl);
    if(BnnAddCarry(R2, R2l, 1)){
	BnnSetDigit((R2+R2l), 1);
	R2l++;
    }
    ComputeCofactor(R2, &R2l, F2, &F2l, lF2);
    if(BnnIsOne(R2, R2l)){
	/* N+1 is completely factorized */
	printf("%% N+1 is completely factored\n");
	pr = Theorem13(N, Nl, lF2);
    }
    else{
	pr = Theorem17(N, Nl, F2, F2l, R2, R2l, lF2);
    }
    BnFree(F2); BnFree(R2);
    return(pr);
}
