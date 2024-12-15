
/* $Log:	findd.c,v $
 * Revision 1.1  91/09/20  14:53:58  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:35  morain
 * Initial revision
 *  */

/**********************************************************************
        Finds a quadratic field K=Q(sqrt(-D)) in which n=N_K(pi) 
**********************************************************************/

#include "ecpp.h"

/* Finds a quadratic field K=Q(sqrt(-D)) in which n=N_K(pi) */
/* goodies are put in ldab */
findD(p_ldab, sqrtd, file_of_DfD)
ListOfDAB *p_ldab;
BigNum sqrtd;
FILE *file_of_DfD;
{
    BigNum A = BmCreate(), B = BmCreate();
    long D, fD[TFMAX];
    int h, g;

    while(1){
	if(!nextD(&D, fD, &h, &g, file_of_DfD)){
	    printf("%%!%% No next D\n");
	    return(0);
	}
	if((D <= 1) || IsANorm(A, B, D, sqrtd)){
	    p_ldab->D = D;
	    p_ldab->h = h;
	    p_ldab->g = g;
	    p_ldab->ABm = NULL;
	    ComputeAllAB(&(p_ldab->ABm), D, A, B);
	    break;
	}
	else{
	    if(abs(h) == g){
		printf("Error: h=g, but N is not a norm\n");
		exit(0);
	    }
	    ModSetToZero(A);
	    ModSetToZero(B);
	}
    }
    return(1);
}

/* Returns the next field for which (Modu, Modul) is represented by A form
   in G_0(-D) */
int nextD(p_D, fD, p_h, p_g, file_of_DfD)
long *p_D, fD[];
int *p_h, *p_g;
FILE *file_of_DfD;
{
    int status;

    do{
#ifdef DISTRIB
	if(distecpp){
	    int i, newrg;

	    newrg = DistGetRgd();
	    for(i = rgd[depth]+1; i < newrg; i++)
		ExtractDfD(p_D, fD, p_h, p_g, file_of_DfD);
	    rgd[depth] = newrg;
	}
	else rgd[depth] = rgd[depth]+1;
#else
	rgd[depth] = rgd[depth]+1;
#endif
	if(rgd[depth] == RGDMAX) return(0);
	if(!(status = ExtractDfD(p_D, fD, p_h, p_g, file_of_DfD)))
	    break;
#ifdef DISTRIB
	if(distecpp && DistAnythingNew()) return(-1);
#endif
    } while(!IsInPrincipalGenus(*p_D, fD));
#ifdef DISTRIB
    if(distecpp && DistAnythingNew()) return(-1);
#endif
    return(status);
}

/* Returns 1 if (Modu, Modul) is represented by the principal form */
int IsANorm(A, B, D, sqrtd)
BigNum A, B, sqrtd;
long D;
{
#ifdef DEBUG
    if(verbose) printf("%% Testing if N is a norm in Q(sqrt(-%ld))\n", D);
#endif
    ModSetToZero(sqrtd);
    return(cornacchia(A, B, D, sqrtd));
}

/* Returns 1 if (Modu, Modul) is represented by a form in G0(-D) */
int IsInPrincipalGenus(D, fD)
long D, fD[];
{
    long q;
    int i;

#ifdef DEBUG
    if(debuglevel >= 10) printf("Trying D[%d]=%ld\n", rgd[depth], D);
#endif
    if(D <= 1)
	return(1);
    else{
	if(LongJacobiBn(-D, Modu, Modul) != 1){
#ifdef DEBUG
	    if(debuglevel >= 10)
		{printf("(-%ld/",D); BnnPrint(Modu,Modul); printf(") <> 1\n");}
#endif
	    return(0);
	}
	else{
	    /* skipping q_1 */
	    for(i = 2; i <= fD[0]; i++){
		q = fD[i];
		if(q % 4 == 3) q = -q;
#ifdef DEBUG
		if(debuglevel >= 10) printf("(%ld/N)\n", q);
#endif
		if(LongJacobiBn(q, Modu, Modul) != 1)
		    return(0);
	    }
	    return(1);
	}
    }
}

/* Computes all solutions of 4 Modu = A^2 + D B^2 */
/* Assumes that the incoming A and B are positive */
ComputeAllAB(p_labm, D, A, B)
ListOfABm *p_labm;
BigNum A, B;
long D;
{
    if(D <= 1)
	/* the N+/-1 cases */
	ComputeAllABfor0(p_labm);
    else{
	if(D == 3)
	    ComputeAllABfor3(p_labm, A, B);
	else{
	    if(D == 4)
		ComputeAllABfor4(p_labm, A, B);
	    else{
		ComputeABm(p_labm, A, 1, B, 1);
	    }
	}
    }
}

/* Case D = 0, corresponding to the N+/-1 case */
ComputeAllABfor0(p_labm)
ListOfABm *p_labm;
{
    ListOfABm l1 = ListOfABmAlloc(), l2 = ListOfABmAlloc();

    l1->lfact = NULL;
    l2->lfact = NULL;
    /* first creating m1 = Modu+1 */
    l1->m = BNC(Modul+2);
    BnnAssign(l1->m, Modu, Modul);
    BnnAddCarry(l1->m, Modul+1, 1);
    /* m2 <- Modu-1 and m2l = Modul since Modu is odd */
    l2->m = BmCreate();
    BnnAssign(l2->m, Modu, Modul);
    BnnSubtractBorrow(l2->m, Modul, 0);
    /* appending all these */
    l1->cdr = *p_labm;
    l2->cdr = l1;
    *p_labm = l2;
}

/* Case D = 3: 4 Modu = A^2 + 3 B^2. If [A, B] is a positive solution,  
   [(A+3*B)/2, -(A-B)/2]  is the second,
   [abs(A-3*B)/2, e*(A+B)/2] is the third (e = sign(A-3*B).
*/
ComputeAllABfor3(p_labm, A, B)
ListOfABm *p_labm;
BigNum A, B;
{
    BigNum U = BmCreate(), V = BmCreate();
    int signofU, signofV;

    /* [A, B] */
    ComputeABm(p_labm, A, 1, B, 1);

    /* [(A+3*B)/2, -(A-B)/2] */
    ModAssign(U, A);
    BnnMultiplyDigit(U, Modul+1, B, Modul, *(CONSTANTES+3));
    BnnShiftRight(U, Modul, 1);
    /* V <- abs((A-B)/2) */
    if(ModCompare(A, B) == 1){
	/* A > B */
	ModAssign(V, A);
	BnnSubtract(V, Modul, B, Modul, 1);
	signofV = -1;
    }
    else{
	ModAssign(V, B);
	BnnSubtract(V, Modul, A, Modul, 1);
	signofV = 1;
    }
    BnnShiftRight(V, Modul, 1);
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("%% U=%d * ", 1); BnnPrint(U, Modul); printf("\n");
	printf("%% V=%d * ", signofV); BnnPrint(V, Modul); printf("\n");
    }
#endif
    ComputeABm(p_labm, U, 1, V, signofV);

    /* [A'', B''] = [abs(A-3*B)/2, e*(A+B)/2] with e = sign(A-3*B) */
    ModSetToZero(V);
    BnnMultiplyDigit(V, Modul+1, B, Modul, *(CONSTANTES+3));
    if(ModCompare(V, A) == 1){
	ModAssign(U, V);
	BnnSubtract(U, Modul, A, Modul, 1);
	signofU = 1;
    }
    else{
	ModAssign(U, A);
	BnnSubtract(U, Modul, V, Modul, 1);
	signofU = -1;
    }
    BnnShiftRight(U, Modul, 1);
    ModAssign(V, A);
    BnnAdd(V, Modul, B, Modul, 0);
    BnnShiftRight(V, Modul, 1);
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("%% U=%d * ", 1); BnnPrint(U, Modul); printf("\n");
	printf("%% V=%d * ", signofU); BnnPrint(V, Modul); printf("\n");
    }
#endif
    ComputeABm(p_labm, U, 1, V, signofU);

    BmFree(U); BmFree(V);
}

/* Case D = 4: 4 Modu = A^2 + 4 B^2. If [A, B] is a positive solution,  
   [2 B, A/2] is the other. */
ComputeAllABfor4(p_labm, A, B)
ListOfABm *p_labm;
BigNum A, B;
{
    ComputeABm(p_labm, A, 1, B, 1);
    BnnShiftRight(A, Modul, 1);
    BnnShiftLeft(B, Modul, 1);
    ComputeABm(p_labm, B, 1, A, 1);
}

/* Appends to labm a */
ComputeABm(p_labm, A, signofA, B, signofB)
ListOfABm *p_labm;
BigNum A, B;
int signofA, signofB;
{
    ListOfABm l1 = ListOfABmAlloc(), l2 = ListOfABmAlloc();
    int m1l, Al = BnnNumDigits(A, Modul), Bl = BnnNumDigits(B, Modul);

    l1->lfact = NULL;
    l2->lfact = NULL;
    /* first creating m1 = Modu+1 */
    l1->m = BNC(Modul+2);
    l2->m = BNC(Modul+2);
    BnnAssign(l1->m, Modu, Modul);
    m1l = Modul;
    if(BnnAddCarry(l1->m, m1l, 1)){
	BnnSetDigit(((l1->m)+m1l), 1);
	m1l++;
    }
#ifdef DEBUG
    if(debuglevel >= 8)	{printf("p+1="); BnnPrint(l1->m, m1l); printf("\n");}
#endif
    BnnAssign(l2->m, l1->m, m1l);
    /* m1 = Modu+1-signofA*A */
    /* m2 = Modu+1+signofA*A */
    if(signofA == 1){
	BnnSubtract(l1->m, m1l, A, Al, 1);
#ifdef DEBUG
	if(debuglevel >= 8)
	    {printf("p+1-A="); BnnPrint(l1->m, m1l); printf("\n");}
#endif
	l1->A = BzFromBigNum(A, Al);
	l1->B = BzFromBigNum(B, Bl); BzSetSign(l1->B, signofB);
	BnnAdd(l2->m, Modul+1, A, Al, 0);
#ifdef DEBUG
	if(debuglevel >= 8)
	    {printf("p+1+A="); BnnPrint(l2->m, Modul+1); printf("\n");}
#endif
	l2->A = BzFromBigNum(A, Al); BzSetSign(l2->A, -1);
	l2->B = BzFromBigNum(B, Bl); BzSetSign(l2->B, -signofB);
    }
    else{
	BnnAdd(l1->m, Modul+1, A, Al, 0);
#ifdef DEBUG
	if(debuglevel >= 8)
	    {printf("p+1-A="); BnnPrint(l1->m, m1l); printf("\n");}
#endif
	l1->A = BzFromBigNum(A, Al); BzSetSign(l1->A, -1);
	l1->B = BzFromBigNum(B, Bl); BzSetSign(l1->B, signofB);
	BnnSubtract(l2->m, m1l, A, Al, 1);
#ifdef DEBUG
	if(debuglevel >= 8)
	    {printf("p+1+A="); BnnPrint(l2->m, m1l); printf("\n");}
#endif
	l2->A = BzFromBigNum(A, Al);
	l2->B = BzFromBigNum(B, Bl); BzSetSign(l2->B, -signofB);
    }
    /* appending all these */
    l2->cdr = *p_labm;
    l1->cdr = l2;
    *p_labm = l1;
}

