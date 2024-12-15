
/* $Log:	poly.c,v $
 * Revision 1.1  91/09/20  14:54:56  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:42  morain
 * Initial revision
 *  */

/**********************************************************************
                     Computing with polynomials
**********************************************************************/

#include "mod.h"
#include "poly.h"

/* A polynomial buffer */
Poly TP;

/* An object of type Poly is in fact a pointer on a struct */

/* Creation of a polynomial of "degree" deg */
Poly PolyCreate(length)
int length;
{
    Poly PX = PolyAlloc();
    int i;

    for(i = 0; i <= length; i++)
	PX->coeff[i] = BmCreate();
    PX->length = length;
    PX->deg = 0;
    return(PX);
}

PolyFree(PX)
Poly PX;
{
    int i;
    
    for(i = 0; i <= PX->length; i++){
	if(PX->coeff[i] != NULL)
	    BmFree(PX->coeff[i]);
    }
    free((Poly *)PX);
}

PolyInit(length)
int length;
{
    TP = PolyCreate(length);
}

PolyClose()
{
    PolyFree(TP);
}

/* Clearing coefficients of degree <= dP of PX */
PolySetToZero(PX, dP)
Poly PX;
int dP;
{
    int i;

    for(i = 0; i <= dP; i++)
	ModSetToZero(PX->coeff[i]);
    PX->deg = 0;
}

/* QX <- PX */
PolyAssign(QX, PX)
Poly PX, QX;
{
    int i;

    for(i = 0; i <= PX->deg; i++)
	BnnAssign(QX->coeff[i], PX->coeff[i], Modul);
    QX->deg = PX->deg;
}

/* Returns a copy of R, i.e. a polynomial of same length, same degree 
   and coeffcients. */
Poly PolyCopy(PX)
Poly PX;
{
    Poly RX = PolyCreate(PX->length);

    PolyAssign(RX, PX);
    return(RX);
}
    
/* Equality test. */
int PolyEqual(PX, QX)    
Poly PX, QX;
{
    int i, cmp = 0;
	
    if(PX->deg == QX->deg){
	for(i = PX->deg; i >= 0; i--){
	    if((cmp = ModCompare(PX->coeff[i], QX->coeff[i])))
		break;
	}
	return(cmp);
    }
    else return(0);
}

/* Sets the degree of PX */
PolyUnderflow(PX)
Poly PX;
{
    int i;

    for(i = PX->deg; i > 0; i--){
	if(!BnnIsZero(PX->coeff[i], Modul))
	    break;
    }
    PX->deg = i;
}

/* Printing a nice polynomial in X */
PolyPrint(PX)
Poly PX;
{
    int i;

    for(i=PX->deg; i >= 0; i--){
	printf("+ X^%d * ", i);
	BnnPrint(PX->coeff[i], Modul);
	printf("\n");
    }
}

/* r <- PX(x) mod Modu and returns rl */
int PolyHorner(r, x, xl, PX)
BigMod r, x;
int xl;
Poly PX;
{
    int i, rl = 0;

    MdAssign(r, x, xl);
    rl = xl;
    for(i = (PX->deg-1); i >= 1; i--){
	BmAdd(r, &rl, PX->coeff[i], Modul);
	BmMultiply(r, &rl, r, rl, x, xl);
    }
    BmAdd(r, &rl, PX->coeff[0], Modul);
    return(rl);
}

/* P(X) <- P(X) + Q(X) */
PolyAdd(PX, QX)
Poly PX, QX;
{
    int i, mindeg = (PX->deg < QX->deg) ? PX->deg : QX->deg;

    for(i = 0; i <= mindeg; i++)
	ModAdd(PX->coeff[i], QX->coeff[i]);
    PolyUnderflow(PX);
}

/* P(X) <- P(X) * n modulo Modu */
PolyMultMod(PX, n, nl)
Poly PX;
BigMod n;
int nl;
{
    int i, l;

    for(i = 0; i <= PX->deg; i++)
	BmMultiply(PX->coeff[i], &l, PX->coeff[i], Modul, n, nl);
    PolyUnderflow(PX);
}

/* Forces the leading coeff of P(X) to be 1 */
PolyMakeMonic(PX)
Poly PX;
{
    if(!BnnIsOne(PX->coeff[PX->deg], Modul)){
	/* we must normalize */
	BigNum a = BmCreate();
	int al, gl, pdegl = BnnNumDigits(PX->coeff[PX->deg], Modul);

	if(BnnEeaWithCopy(Gbuf,&gl,a,&al,Modu,Modul,PX->coeff[PX->deg],pdegl)){
	    PolyMultMod(PX, a, al);
	    BmFree(a);
	}
	else{
	    printf("Non invertible in monic\n");
	    exit(0);
	}
    }
}

/* Q(X) <- P(X) / MX(X) mod :N. Uses Zbuf. 
   Should'nt work if MX is not monic...
*/
PolyQuoPoly(QX, PX, MX)
Poly QX, PX, MX;
{
    Poly RX;
    int zl, hl, i, j, iq, ip, l;

    RX = PolyCopy(PX);
    QX->deg = (PX->deg)-(MX->deg);
    for(ip = PX->deg, iq = QX->deg; ip >= MX->deg; ip--, iq--){
	/* quotient */
	ModAssign(QX->coeff[iq], RX->coeff[ip]);
	zl = BnnNumDigits(QX->coeff[iq], Modul);
	for(i = MX->deg, j = ip; i >= 0; i--, j--){
	    BmMultiply(Hbuf, &hl, MX->coeff[i], Modul, QX->coeff[iq], zl);
	    l = Modul;
	    BmSubtract(RX->coeff[j], &l, Hbuf, hl);
	}
    }
    PolyUnderflow(QX);
    PolyFree(RX);
}

/* R(X) <- P(X) mod MX(X) mod Modu. Stands P = R. Uses Hbuf and Zbuf.*/
PolyModPoly(RX, PX, MX)
Poly RX, PX, MX;
{
    int zl, hl, i, j, ip, l;

    PolyMakeMonic(MX);
    PolyAssign(RX, PX);
    for(ip = PX->deg; ip >= MX->deg; ip--){
	/* copy of the leading coeff of RX */
	BnnAssign(Zbuf, RX->coeff[ip], Modul);
	zl = BnnNumDigits(Zbuf, Modul);
	for(i = MX->deg, j =ip; i >= 0; i--, j--){
	    BmMultiply(Hbuf, &hl, MX->coeff[i], Modul, Zbuf, zl);
	    l = Modul;
	    BmSubtract(RX->coeff[j], &l, Hbuf, hl);
	}
    }
    PolyUnderflow(RX);
}

/* R(X) <- pgcd(P(X), Q(X)) mod Modu. Assumes deg(P)>=deg(Q).
   WARNING: P and Q are destroyed in the process.
*/
PolyPgcd(RX, PX, QX)
Poly RX, PX, QX;
{
    while(QX->deg > 0){
	PolyModPoly(RX, PX, QX);
        PolyAssign(PX, QX);
	PolyAssign(QX, RX);
    }
    if(BnnIsZero(QX->coeff[0], Modul))
        /* gcd is nontrivial */
	PolyAssign(RX, PX);
    else{
        /* le pgcd vaut 1 et on retourne RX = 1 */
	PolySetToZero(RX, 1);
	BnnSetDigit(RX->coeff[0], 1);
    }
}

/* P(X) <- X P(X) mod MX(X) mod Modu. Uses Hbuf and Zbuf.*/
PolyMultXmod(PX, MX)
Poly PX, MX;
{
    int hl, zl, i, l;

    /* copy of the highest coefficient of PX */
    BnnAssign(Hbuf, PX->coeff[PX->deg], Modul);
    hl = BnnNumDigits(Hbuf, Modul);
    /* shifting the leading coeff's */
    for(i = PX->deg; i >= 1; i--)
	BnnAssign(PX->coeff[i], PX->coeff[i-1], Modul);
    /* clearing the first coefficient of PX */
    BnnSetToZero(PX->coeff[0], Modul);
    if(PX->deg < MX->deg-1){
	/* we stop at once */
	PX->deg += 1;
	BnnAssign(PX->coeff[PX->deg], Hbuf, hl);
    }
    else{
	/* otherwise we go on */
	for(i = 0; i <= PX->deg; i++){
	    BmMultiply(Zbuf, &zl, Hbuf, hl, MX->coeff[i], Modul);
	    l = Modul;
	    BmSubtract(PX->coeff[i], &l, Zbuf, zl);
	}
	PolyUnderflow(PX);
    }
}

/* R(X) <- P(X) * Q(X) mod MX(X) mod Modu. P and Q are not modified.*/
/* Stands P = R. Uses Hbuf.*/
PolyMultPolyMod(RX, PX, QX, MX)
Poly RX, PX, QX, MX;
{
    int hl, i, j, l;

    PolyAssign(RX, PX);
    PolySetToZero(TP, MX->deg);
    PolyAssign(TP, PX);
    PolyMultMod(RX, QX->coeff[0], Modul);
    for(i = 1; i <= QX->deg; i++){
	PolyMultXmod(TP, MX);
#ifdef DEBUG
	if(debuglevel >= 10) {printf("X^%d * P=", i); PolyPrint(TP);}
#endif
	for(j = 0; j <= TP->deg; j++){
	    BmMultiply(Hbuf, &hl, TP->coeff[j], Modul, QX->coeff[i], Modul);
	    l = Modul;
	    BmAdd(RX->coeff[j], &l, Hbuf, hl);
	}
    }
    RX->deg = MX->deg - 1;
    PolyUnderflow(RX);
}

/********** Differentiation **********/

/* dP <- d/dX (P(X)) */
PolyDiff(dP, PX)
Poly dP, PX;
{
    BigNumDigit C[1];
    int i, l;

#ifdef DEBUG
    if(debuglevel >= 10) {printf("P(X)="); PolyPrint(PX);}
#endif
    if(PX->deg){
	ModAssign(dP->coeff[0], PX->coeff[1]);
	BnnSetDigit(C, 1);
	for(i = 1; i < PX->deg; i++){
	    BnnAddCarry(C, 1, 1);
	    /* dP[i] <- (i+1) * P[i+1] */
	    BmMultiply(dP->coeff[i], &l, PX->coeff[i+1], Modul, C, 1);
	}
    }
    ModSetToZero(dP->coeff[PX->deg]);
    /* this is better than dP->deg = P->deg - 1 */
    PolyUnderflow(dP);
#ifdef DEBUG
    if(debuglevel >= 10) {printf("P'(X)="); PolyPrint(dP);}
#endif
}

/* P <- P(X+1) = P(X) + 1/1! P'(X) + ... + 1/n! P^(n)(X) */
OldPolyIncr(PX)
Poly PX;
{
    Poly RX = PolyCreate(PX->deg);
    BigNum f = BmCreate();
    BigNumDigit C[1];
    int gl, fl, i;

#ifdef DEBUG
    if(debuglevel >= 10) {printf("  P(X)="); PolyPrint(PX);}
#endif
    PolyAssign(RX, PX);
    BnnSetDigit(C, 0);
    for(i = 1; i <= PX->deg; i++){
	/* RX <- 1/i! d/dX(R(X)) */
	PolyDiff(RX, RX);
	BnnAddCarry(C, 1, 1);
	/* f <- 1/i */
	BnnEeaWithCopy(Gbuf, &gl, f, &fl, Modu, Modul, C, 1);
	PolyMultMod(RX, f, fl);
	/* P <- P + R */
	PolyAdd(PX, RX);
    }
#ifdef DEBUG
    if(debuglevel >= 10) {printf("P(X+1)="); PolyPrint(PX);}
#endif
    PolyFree(RX); BmFree(f);
}

/* P <- P(X+1) using Horner's rule. Uses TP. */
PolyIncr(PX)
Poly PX;
{
    int i, j, degP = PX->deg, degTP = 0;

#ifdef DEBUG
    if(debuglevel >= 10) {printf("  P(X)="); PolyPrint(PX);}
#endif
    ModAssign(TP->coeff[0], PX->coeff[degP]);
    for(i = degP-1; i >= 0; i--, degTP++){
	/* TP <- TP*(X+1) */
	ModAssign(TP->coeff[degTP+1], TP->coeff[degTP]);
	for(j = degTP; j > 0; j--)
	    ModAdd(TP->coeff[j], TP->coeff[j-1]);
	/* TP <- TP + P[i] */
	ModAdd(TP->coeff[0], PX->coeff[i]);
    }
    TP->deg = degTP;
    PolyAssign(PX, TP);
#ifdef DEBUG
    if(debuglevel >= 10) {printf("P(X+1)="); PolyPrint(PX);}
#endif
}

/********** Exponentiation **********/

/* R(X) <- P(X)**e mod MX(X) mod Modu: left-to-right */
PolyExpMod(RX, PX, e, el, MX)
Poly RX, PX, MX;
BigNum e;
int el;
{
    Poly QX = PolyCreate(MX->deg), S = PolyCreate(MX->deg);
    BigNumDigit C[1];
    int ed, es, i, j;

    /* initialization of the polynomials */
    PolyAssign(QX, PX); 
    PolyAssign(RX, PX);
    /* starting the computation */
    ed = el - 1;
    BnnAssign(C, (e+ed), 1);
    /* looking for the most significant bit */
    for(es = BN_DIGIT_SIZE; es; es--){
	if(BnnShiftLeft(C, 1, 1))
	    break;
    }
#ifdef DEBUG
    if(debuglevel >= 15) printf("es=%d\n", es);
#endif
    if(!es){
	fprintf(stderr, "Error, es is 0\n");
	exit(0);
    }
    es--;
    while(el >= 1){
	for(i = 0; i < es; i++){
	    PolyMultPolyMod(S, RX, RX, MX);
	    PolyAssign(RX, S);
#ifdef DEBUG
	    if(debuglevel >= 10) {printf("R^2="); PolyPrint(RX);}
#endif
	    if(BnnShiftLeft(C, 1, 1)){
		PolyMultPolyMod(RX, RX, QX, MX);
#ifdef DEBUG
		if(debuglevel >= 10) {printf("R*Q="); PolyPrint(RX);}
#endif
	    }
	}
	--el;
	if(el){
	    --ed;
	    es = BN_DIGIT_SIZE;
	    BnnAssign(C, (e+ed), 1);
	}
    }
    PolyFree(QX); PolyFree(S);
}

/* R(X) <- X**e mod MX(X) mod Modu: left-to-right */
PolyXPowerMod(RX, e, el, MX)
Poly RX, MX;
BigNum e;
int el;
{
    Poly S = PolyCreate(MX->deg);
    BigNumDigit C[1];
    int ed, es, i, j;

    /* initialization of the polynomials */
    PolySetToZero(RX, MX->deg);
    /* RX <- X */
    RX->deg = 1;
    BnnSetDigit(RX->coeff[1], 1);
    /* starting the computation */
    ed = el - 1;
    BnnAssign(C, (e+ed), 1);
    BnnSetDigit((C+1), 0);
    /* looking for the most significant bit */
    for(es = BN_DIGIT_SIZE; es; es--){
	if(BnnShiftLeft(C, 1, 1))
	    break;
    }
#ifdef DEBUG
    if(debuglevel >= 15) printf("es=%d\n", es);
#endif
    if(!es){
	fprintf(stderr, "Error, es is 0\n");
	exit(0);
    }
    es--;
    while(el >= 1){
	for(i = 0; i < es; i++){
	    PolyMultPolyMod(S, RX, RX, MX);
	    PolyAssign(RX, S);
#ifdef DEBUG
	    if(debuglevel >= 10) {printf("R^2="); PolyPrint(RX);}
#endif
	    if(BnnShiftLeft(C, 1, 1)){
		PolyMultXmod(RX, MX);
#ifdef DEBUG
		if(debuglevel >= 10) {printf("R * X="); PolyPrint(RX);}
#endif
	    }
	}
	--el;
	if(el){
	    --ed;
	    es = BN_DIGIT_SIZE;
	    BnnAssign(C, (e+ed), 1);
	}
    }
    PolyFree(S);
}

/**********************************************************************

                       New code for multiplication

**********************************************************************/

#if 0

/* TX[i] <- X^(i+n) mod MX, for i=0..n-2, with degree(MX) = n.
   Assume that MX is monic. */
PolyComputePowerOfX(TX, MX)
Poly MX, TX[];
{
    int degM, i, j;

    degM = MX->degree;
    /* TX[0] <- X^n mod MX = -(MX - X^n) */
    for(j = 0; j < degM; j++){
	ModAssign(TX[0]->coeff[j], MX->coeff[j]);
	ModNegate(TX[0]->coeff[j]);
    }
#ifdef DEBUG
    printf("X^%d=", degM); PolyPrint(TX[0]); printf("\n");
#endif
    for(i = 1; i <= n-2; i++){
	/* TX[i] <- TX[i-1] * X mod MX */
	PolyAssign(TX[i], TX[i-1]);
	PolyMultXmod(TX[i], MX);
#ifdef DEBUG
	printf("X^%d=", i+degM); PolyPrint(TX[0]); printf("\n");
#endif
    }
}

/* TX is the same as above. Uses Dbuf */
PolySquareMod(RX, PX, MX, TX)
Poly PX, RX, MX;
{
    int dl, i, degM = MX->degree;

    /* TP <- PX^2 */
    PolySquare(TP, PX);
    for(i = 0; i <= n; i++){
	BnnAssign(Dbuf, TP->coeff[i], Modul);
	BnnSetToZero(Dbuf+Modul, Modul+1);
	dl = 2*Modul+1;
	/* Dbuf <- TP[i] + sum(TP[j+degM] * TX[j], j=0..degM-2) */
	for(j = 0; j < degM-1; j++){
	    BnnMultiply(Dbuf, dl, TP->coeff[j+degM], Modul, TX[j], Modul);
	}
	/* Only one reduction */
	BnnDivide(Dbuf, dl, Modu, Modul);
	BnnAssign(RX->coeff[i], Dbuf, Modul);
    }
}

/* RX is of degree 2*degM-2 */
PolySquare(RX, PX)
Poly RX, PX;
{
    int i, j, l;

    for(i = 0; i < PX->deg; i++){
	for(j = 0; j < PX->deg; j++)
	   BmMultiply(RX->coeff[i+j],&l,PX->coeff[j],Modul,PX->coeff[i],Modul);
    }
}
#endif
