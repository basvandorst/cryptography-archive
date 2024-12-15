
/* $Log:	proof.c,v $
 * Revision 1.1  91/09/20  14:55:05  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:53  morain
 * Initial revision
 *  */

/**********************************************************************
                            Proving part of ECPP
**********************************************************************/

#include "exp.h"
#include "ecpp.h"
#include "ecmodp.h"
#define NPTMAX 5

/* Proving for step i */
int prove(p_cert, maxp, maxpl)
Certif *p_cert;
BigNum maxp;
int maxpl;
{
    BmInit(p_cert->p);
    if(p_cert->D > 1) return(EcmProve(p_cert, maxp, maxpl));
    else return(LucasLehmerProve(p_cert, maxp, maxpl));
}

int LucasLehmerProve(p_cert, maxp, maxpl)
Certif *p_cert;
BigNum maxp;
int maxpl;
{
    if(p_cert->D == -1) return(Nminus1Prove(p_cert, maxp, maxpl));
    else return(Nplus1Prove(p_cert, maxp, maxpl));
}

/* if Th3 applies then
     cert.h      contains 3 (for Th 3)
     cert.lfacto contains (a, 1, 1)
   elif Th1 applies then
     cert.h      contains 1 (for Th 1)
     cert.lfacto contains the a_i's
   fi
*/
int Nminus1Prove(p_cert, maxp, maxpl)
Certif *p_cert;
BigNum maxp;
int maxpl;
{
    ListOfBigNum la = NULL;
    BigNumDigit a[1];
    int done = 0, pr = 0;

    if(maxp != NULL){
	if(LucasApplyTh3(Modu, Modul, maxp, maxpl)){
	    pr = Theorem3(a, Modu, Modul, maxp, maxpl);
	    AppendFactor(&(p_cert->lfacto), a, 1, 1);
	    p_cert->h = 3;
	    done = 1;
	}
	else
	    AppendFactor(&(p_cert->lfact), maxp, maxpl, 1);
    }
    if(!done){
	/* Using complete factorization */
	if(verbose) printf("%% Using complete factorization theorem\n");
	pr = Theorem1(&la, Modu, Modul, p_cert->lfact);
	while(la != NULL){
	    Append1Factor(&(p_cert->lfacto), la->n, 1, 1);
	    la = la->cdr;
	}
	p_cert->h = 1;
    }
    return(pr);
}

/* Here, there is yet some dissymetry */
int Nplus1Prove(p_cert, maxp, maxpl)
Certif *p_cert;
BigNum maxp;
int maxpl;
{
    ListOfFactors lf;
    ListOfBigNum lm = NULL;
    int pr = 0;

    lf = p_cert->lfact;
    if(maxp != NULL){
	AppendFactor(&lf, maxp, maxpl, 1);
    }
    pr = CLLucas(&lm, p_cert->p, p_cert->pl, lf);
    p_cert->ea = BNC(1);
    BnnAssign(p_cert->ea, lm->n, 1);
    lm = lm->cdr;
    while(lm != NULL){
	Append1Factor(&(p_cert->lfacto), lm->n, 1, 1);
	lm = lm->cdr;
    }
    p_cert->h = 1;
    return(pr);
}

/* Let maxp be the largest prime factor of m. Two things can happen:
    1. maxp >  smin: we look for P s.t. (m/maxp)P <> OE, but m P = OE;
    2. maxp <= smin: we hope to find P of large order enough.
   Maxp is ordinarily given by tabp_certif[i+1].p
*/
int EcmProve(p_cert, maxp, maxpl)
Certif *p_cert;
BigNum maxp;
int maxpl;
{
    BigNum m, smin, sqrtd;
    ListOfFactors lfact, lfacto;
    Ecm E;
    Point P;
    long D;
    BigZ A, B;
    int h, g, twist, try, ok, ml, sminl, small;

    D = p_cert->D; h = p_cert->h; g = p_cert->g;
    lfact = p_cert->lfact; m = p_cert->m; ml = p_cert->ml;
    A = p_cert->A; B = p_cert->B; sqrtd = p_cert->sqrtd;
#ifdef DEBUG
    if(debuglevel >= 6)
	{printf("%% #E="); BnnPrint(m, ml); printf("\n");}
#endif
    findE(&E, D, h, g, A, B, sqrtd);
#ifdef DEBUG
    if(debuglevel >= 5)
	{printf("%% Original curve is "); EcmPrint(E); printf("\n");}
#endif
    P = PointCreate();
    twist = WhichC(D, h, A, m);
    if(twist){
	if(verbose) printf("%% Suggested twist(%ld)=%d\n", D, twist);
    }
    smin = BmCreate();
    EcmComputeBound(smin, &sminl, p_cert->p, p_cert->pl);
    if(maxp != NULL){
	small = (BnnCompare(maxp, maxpl, smin, sminl) == -1);
    }
    else small = 1;
    EcmInit();
    BnnSetDigit(P->x, 0);
    P->z = 1;
    for(try = 0; try < NPTMAX; try++){
	lfacto = NULL;
	if(small)
	 ok=EcmProveSmall(&lfacto,m,ml,maxp,maxpl,lfact,smin,sminl,E,P,twist);
	else
	 ok = EcmProveLarge(&lfacto, m, ml, maxp, maxpl, E, P, twist);
	/* at this point, the correct twist is known */
	twist = 1;
	if(ok)
	    break;
	else{
	    ListOfFactorsFree(lfacto);
	}
    }
    p_cert->lfacto = lfacto;
    p_cert->ea = E.a;
    p_cert->eal = BnnNumDigits(E.a, Modul);
    p_cert->eb = E.b;
    p_cert->ebl = BnnNumDigits(E.b, Modul);
    p_cert->px = P->x;
    p_cert->pxl = BnnNumDigits(P->x, Modul);
    p_cert->py = P->y;
    p_cert->pyl = BnnNumDigits(P->y, Modul);
    BmFree(smin);
    EcmClose();
    return(ok);
}

/********** Finding the right twist, after Atkin (and the author) **********/

int WhichC(D, h, A, m)
long D;
int h;
BigZ A;
BigNum m;
{
    if(abs(h) != 1) return(0);
    
    if((D == 3) || (D == 4))
	return(1);
    else{
	if(D == 8)
	    return(WhichCFor8(m));
	else
	    return(WhichCForH1(D, A));
    }
}

/* case D == 8 */
int WhichCFor8(m)
BigNum m;
{
    int j2, j3, w;

    w = LongJacobiBn(105, Modu, Modul);
    BnnSetDigit(SCRATCH, 15);
    BnnAndDigits(SCRATCH, *Modu);
    j3 = (int)(BnnGetDigit(SCRATCH));
    if(j3 < 8) w = -w;
    if((j3 % 4) == 3) w = -w;
    BnnSetDigit(SCRATCH, 7);
    BnnAndDigits(SCRATCH, *m);
    j2 = (int)(BnnGetDigit(SCRATCH));
    j2 = j2 - j3 - 1;
    if(j2 > 0)
	j2 = j2 % 8;
    else
	j2 = 8 - ((-j2) % 8);
    if(j2 == 6) w = -w;
    return(w);
}

/* Case h = 1, D odd. Only the "interesting" part of 3*j*sqrt((-j-1728)/(-D))
   is stored. Uses Zbuf. */
int WhichCForH1(D, A)
long D;
BigZ A;
{
    BigNum AA = BmCreate();
    int sign, j2, w, dd = (int)D, zl;

    switch(dd){
      case   7: w = LongJacobiBn(15, Modu, Modul); break;
      case  11: w = LongJacobiBn(21, Modu, Modul); break;
      case  19: w = LongJacobiBn( 3, Modu, Modul); break;
      case  43: w = LongJacobiBn(70, Modu, Modul); break;
      case  67: w = LongJacobiBn(105, Modu, Modul);
	        w = LongJacobiBn(341, Modu, Modul) * w;
	        break;
      case 163: w = LongJacobiBn(2310, Modu, Modul);
	        w = LongJacobiBn(12673, Modu, Modul) * w;
	        w = LongJacobiBn(127, Modu, Modul) * w;
	        break;
    }
    if(D == 7){
	BnnSetDigit(SCRATCH, 3);
	BnnAndDigits(SCRATCH, *Modu);
	if(BnnGetDigit(SCRATCH) == 1) w = -w;
    }
    /* j2 <- (-A / D) */
    sign = BzGetSign(A);
    zl = BzNumDigits(A);
    BnnAssign(AA, BzToBn(A), zl);
    BnnAssign(Zbuf, AA, zl);
    BnnSetDigit((Zbuf+zl), 0);
    j2 = (int)BnnDivideDigit(AA, Zbuf, zl+1, D);
    j2 = LongJacobiLong(j2, D);
    if(sign == 1) j2 = -j2;
    BmFree(AA);
    return(w * j2);
}

/* We look for a point P s.t. (m/maxp)P <> O, but mP = O. */
int EcmProveLarge(p_lfacto, m, ml, maxp, maxpl, E, P, twist0)
ListOfFactors *p_lfacto;
BigNum m, maxp;
Ecm E;
Point P;
int ml, maxpl, twist0;
{
    Point Q = PointCreate(), S = PointCreate();
    BigNum qu = BNC(ml+1);
    int twist, qul, ok = 1;

    /* qu <- m / maxp */
    BnnAssign(qu, m, ml);
    BnnDivide(qu, ml+1, maxp, maxpl);
    if(!BnnIsZero(qu, maxpl)){
	printf("maxp doesn't divide m\n");
	exit(0);
    }
    /* qu = qu[maxpl..ml] */
    qul = BnnNumDigits((qu+maxpl), ml-maxpl+1);
#ifdef DEBUG
    if(debuglevel >= 5)
	{printf("m/maxp="); BnnPrint((qu+maxpl), qul); printf("\n");}
#endif
    twist = ChoosePoint(&E, P, twist0);
    while(1){
	if(debuglevel >= 1){
	    printf("%% E: "); EcmPrint(E); printf("\n");
	    printf("%% P="); PointPrint(P); printf("\n");
	}
	EcmMult(Q, (qu+maxpl), qul, P, E);
#ifdef DEBUG
	if(debuglevel>=1) {printf("%% (m/r)P="); PointPrint(Q); printf("\n");}
#endif
	if(EcmIsZero(Q))
	    /* if z = 0, P is not good, let's find another one */
	    twist = ChoosePoint(&E, P, twist0);
	else{
	    /* computing maxp Q */
	    EcmMult(S, maxp, maxpl, Q, E);
#ifdef DEBUG
	    if(debuglevel>=1) {printf("%% rQ="); PointPrint(S); printf("\n");}
#endif
	    if(EcmIsZero(S)){
		/* P is on E and Q = (m/maxp)P has order maxp */
		AppendFactor(p_lfacto, maxp, maxpl, 1);
		ModAssign(P->x, Q->x);
		ModAssign(P->y, Q->y);
		break;
	    }
	    else{
		if(twist0){
		    /* we should have had the right twist */
		    printf("Error\n");
		    exit(0);
		}
		else{
		    /* new twist */
		    twist = ChoosePoint(&E, P, -1);
		    twist0 = 1;
		}
	    }
	}
    }
    PointFree(Q); PointFree(S); BnFree(qu);
    return(ok);
}

/*
   We are looking for the exact order of P on E. If it is greater than smin, 
   then (Modu, Modul) passes the test.
*/
int EcmProveSmall(p_lfacto, m, ml, maxp, maxpl, lfact, smin, sminl,E,P,twist0)
ListOfFactors *p_lfacto;
BigNum m, maxp, smin;
ListOfFactors lfact;
Ecm E;
Point P;
int twist0, ml, sminl, maxpl;
{
    BigNum qu = BNC(ml+1), mu = BNC(ml+1), tmp, ordP = BNC(ml+1);
    Point Q = PointCreate();
    ListOfFactors foo;
    int twist, qul, ou, ordPl, i, ok;

    twist = ChoosePoint(&E, P, twist0);
    /* checking that P is on E */
    if(debuglevel >= 1) {printf("%% m="); BnnPrint(m, ml); printf("\n");}
    EcmMult(Q, m, ml, P, E);
#ifdef DEBUG
    if(debuglevel >= 1) {printf("%% mP="); PointPrint(Q); printf("\n");}
#endif
    if(!EcmIsZero(Q)){
	/* P is not on E */
	if(twist0){
	    /* we should have the right twist */
	    printf("Error, we should have the right twist\n");
	    exit(0);
	}
	else{
	    twist = ChoosePoint(&E, P, -1);
	    EcmMult(Q, m, ml, P, E);
#ifdef DEBUG
	    if(debuglevel >= 1) 
		{printf("%% mP(twist)="); PointPrint(Q); printf("\n");}
#endif
	    if(!EcmIsZero(Q)){
		printf("mP <> O ...\n");
		exit(0);
	    }
	}
    }
#ifdef DEBUG
    if(debuglevel >= 1){
	printf("%% E="); EcmPrint(E); printf("\n");
	printf("%% P="); PointPrint(P); printf("\n");
    }
#endif
    /* looking for the order of P */
    BnnAssign(ordP, m, ml);
    ordPl = ml;
    foo = lfact;
    /* strange, isn't it ? */
    if(maxp != NULL) AppendFactor(&foo, maxp, maxpl, 1);
    while(foo != NULL){
#ifdef DEBUG
	if(debuglevel >= 1){
	    printf("%% Working with p="); 
	    BnnPrint(foo->p, foo->pl); 
	    printf("\n");
	}
#endif
	/* qu <- p**e */
	BnnSetToZero(qu, ml+1);
	BnnAssign(qu, foo->p, foo->pl);
	qul = foo->pl;
	for(i = 1; i < foo->e; i++){
	    BnnSetToZero(mu, ml+1);
	    BnnMultiply(mu, ml+1, qu, qul, foo->p, foo->pl);
	    tmp = qu; qu = mu; mu = tmp;
	    qul = BnnNumDigits(qu, ml+1);
	}
#ifdef DEBUG
	if(debuglevel >= 1)
	    {printf("%% p**%d=", foo->e); BnnPrint(qu, qul); printf("\n");}
#endif
	BnnSetDigit((ordP+ordPl), 0);
	BnnDivide(ordP, ordPl+1, qu, qul);
	ordPl = BnnNumDigits((ordP+qul), ml-qul+1);
	BnnAssign(ordP, (ordP+qul), ordPl);
	EcmMult(Q, ordP, ordPl, P, E);
#ifdef DEBUG
	if(debuglevel >= 1)
	    {printf("%% (m/p^e)P="); PointPrint(Q); printf("\n");}
#endif
	ou = 0;
	if(!EcmIsZero(Q)){
	    for(ou = 1; ou <= foo->e; ou++){
		BnnSetToZero(mu, ml+1);
		BnnMultiply(mu, ml+1, ordP, ordPl, foo->p, foo->pl);
		tmp = ordP; ordP = mu; mu = tmp;
		ordPl = BnnNumDigits(ordP, ml+1);
#ifdef DEBUG
		if(debuglevel >= 1)
		    {printf("%% "); BnnPrint(foo->p, foo->pl); PointPrint(Q);}
#endif
		EcmMult(Q, foo->p, foo->pl, Q, E);
#ifdef DEBUG
		if(debuglevel >= 1)
		    {printf("="); PointPrint(Q); printf("\n");}
#endif
		if(EcmIsZero(Q))
		    /* (u**ou)(muP)=O(E) */
		    break;
	    }
	}
	if(ou) AppendFactor(p_lfacto, foo->p, foo->pl, ou);
	foo = foo->cdr;
    }
    /* if ordP >= smin, P passes the test */
#ifdef DEBUG
    if(debuglevel >= 1)
	{printf("%% Ordre of P is "); BnnPrint(ordP, ordPl); printf("\n");}
#endif
    ok = (BnnCompare(ordP, ordPl, smin, sminl) != -1);
    BnFree(qu); BnFree(mu); BnFree(ordP); PointFree(Q);
    return(ok);
}

/* twist is 0, +/-1, with 0 when we know nothing, +/-1 when we impose the
   twist. Returns the new twist. */
int ChoosePoint(p_E, P, twist)
Ecm *p_E;
Point P;
int twist;
{
    BigNum w = BmCreate();
    long random();
    int xl = 1, wl, tw = 0;

    if(!twist)
	BnnSetDigit(P->x, 0);
    else{
	ModSetToZero(P->x);
	BnnSetDigit(P->x, (random()&0x3f));
    }
#ifdef DEBUG
    if(debuglevel >= 10)
	{printf("First x is "); BnnPrint(P->x, Modul); printf("\n");}
#endif
    /* w <- x^3+E(a)*x+E(b) */
    do{
	BnnAddCarry(P->x, 1, 1);
	wl = EcmYsquare(w, P->x, xl, *p_E);
    } while(BnnIsZero(w, wl));
    if(!twist){
	twist = ModJacobi(w);
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("w="); BnnPrint(w, Modul); printf(" tw=%d\n", twist);}
#endif
    }
    else{
	tw = ModJacobi(w);
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("w="); BnnPrint(w, Modul); printf(" tw=%d\n", tw);}
#endif
	while(tw != twist){
	    do{
		BnnAddCarry(P->x, 1, 1);
		wl = EcmYsquare(w, P->x, xl, *p_E);
	    } while(BnnIsZero(w, wl));
	    tw = ModJacobi(w);
#ifdef DEBUG
	    if(debuglevel >= 10)
		{printf("w="); BnnPrint(w, Modul); printf(" tw=%d\n", tw);}
#endif
	}
    }
    /* P=(w x, w^2) est sur E(w) : Y^2=X^3+E(a)*w^2*X+E(b)*w^3 */
    /* updating P */
    BmMultiply(P->x, &xl, P->x, xl, w, wl);
    BmSquare(P->y, &xl, w, wl);
    /* updating E */
    ModMultiply(p_E->a, p_E->a, P->y);
    ModMultiply(p_E->b, p_E->b, P->y);
    ModMultiply(p_E->b, p_E->b, w);
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("NewE="); EcmPrint(*p_E); printf("\n");
	printf("NewP="); PointPrint(P); printf("\n");
    }
#endif
    BmFree(w);
#ifdef DEBUG
    if(!IsPointOnCurve(*p_E, P))
	printf("P is not on E\n");
#endif
    return(twist);
}
