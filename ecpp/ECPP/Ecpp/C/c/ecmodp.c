
/* $Log:	ecmodp.c,v $
 * Revision 1.1  91/09/20  14:53:35  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:09  morain
 * Initial revision
 *  */

/**********************************************************************
                 Computing on an elliptic curve modulo p
**********************************************************************/

#include "mod.h"
#include "ecmodp.h"

BigNum ECMRX, ECMRY;

/* (s, *p_sl) gets an upper bound for (sqrt[4](Modu)+1)^2, given by 
   the r.h.s of

   r2 + 2*r4 + 1 <= sqrt(N)+2*sqrt(sqrt(N))+1 < r2+2*r4+6

   where r2 = floor(sqrt(N)), r4 = floor(sqrt(r2)).
*/
EcmComputeBound(s, p_sl, n, nl)
BigNum s, n;
int *p_sl, nl;
{
    BigNum r = BNC(nl+1);
    int l;

    l = BnnSqrt(s, n, nl);
    l = BnnSqrt(r, s, l);
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("%% s="); BnnPrint(s, nl); printf("\n");
	printf("%% r="); BnnPrint(r, nl); printf("\n");
    }
#endif
    BnnAdd(s, nl+1, r, l, 0);
    BnnAdd(s, nl+1, r, l, 0);
    BnnAdd(s, nl+1, (CONSTANTES+6), 1, 0);
    BnFree(r);
    *p_sl = BnnNumDigits(s, nl+1);
#ifdef DEBUG
    if(debuglevel >= 1){printf("%% Smin="); BnnPrint(s, *p_sl); printf("\n");}
#endif
}

EcmFree(E)
Ecm E;
{
    BmFree(E.a); BmFree(E.b);
}

PointFree(P)
Point P;
{
    BmFree(P->x); BmFree(P->y); free((char *)P);
}

PointPrint(P)
Point P;
{
    printf("("); BnnPrint(P->x, Modul); printf(" : ");
    BnnPrint(P->y, Modul); printf(" : ");
    printf("%d)", P->z);
}

EcmPrint(E)
Ecm E;
{
    printf("["); BnnPrint(E.a, Modul); printf(", "); BnnPrint(E.b, Modul); 
    printf("]");
}

int IsPointOnCurve(E, P)
Ecm E;
Point P;
{
    BigNum w = BmCreate(), y2 = BmCreate();
    int ok;

    ModMultiply(w, P->x, P->x);
    ModAdd(w, E.a);
    ModMultiply(w, w, P->x);
    ModAdd(w, E.b);
    ModMultiply(y2, P->y, P->y);
    ok = ModCompare(y2, w);
#ifdef DEBUG
    if(debuglevel >= 8){
	printf("y2="); BnnPrint(y2, Modul); printf("\n");
	printf(" w="); BnnPrint(w, Modul); printf("\n");
    }
#endif
    BmFree(w); BmFree(y2);
    return(!ok);
}

/* Q <- P */
PointAssign(Q, P)
Point Q, P;
{
    ModAssign(Q->x, P->x);
    ModAssign(Q->y, P->y);
    Q->z = P->z;
}

Point PointCreate()
{
    Point Q = PointAlloc();

    Q->x = BmCreate();
    Q->y = BmCreate();
    Q->z = 1; /* licit ? */
    return(Q);
}    

Point PointCopy(P)
Point P;
{
    Point Q = PointCreate();

    PointAssign(Q, P);
    return(Q);
}

/* pt <- O_E = [0, 1, 0] */
PointSetToZero(pt)
Point pt;
{
    ModSetToZero(pt->x);
    ModSetToZero(pt->y);
    BnnSetDigit(pt->y, 1);
    pt->z = 0;
}

EcmInit()
{
    ECMRX = BmCreate();
    ECMRY = BmCreate();
}

EcmClose()
{
    BmFree(ECMRX); BmFree(ECMRY);
}

/* w <- x^3+a(E)*x+b(E) mod Modu */
/* Returns wl */
int EcmYsquare(w, x, xl, E)
BigMod w, x;
int xl;
Ecm E;
{
    int wl = 0;

    /* w <- x^2 mod Modu */
    BmSquare(w, &wl, x, xl);
    /* w <- w + a mod Modu */
    BmAdd(w, &wl, E.a, Modul);
    /* w <- w * x mod Modu */
    BmMultiply(w, &wl, w, wl, x, xl);
    /* w <- w + b mod Modu */
    BmAdd(w, &wl, E.b, Modul);
    return(wl);
}

#if 0
/* Choosing a point on the curve */
int EcmChoose(E, P)
Ecm E;
Point P;
{
    BigNum r = BmCreate();
    int rl, yl;

    BnnAddCarry(P->x, 1, 1);
    rl = EcmYsquare(r, P->x, 1, E);
    while(BnJacobi(r, 0, rl, Modu, 0, Modul) != 1){
	BnnAddCarry(P->x, 1, 1);
	rl = EcmYsquare(r, P->x, 1, E);
    }
    MdSqrt(P->y, &yl, r, rl);
    BmFree(r);
    return(yl);
}
#endif

/* R <- P + Q. Can do R = P */
EcmAdd(R, P, Q, E)
Point R, P, Q;
Ecm E;
{
    if(EcmIsZero(P))
	PointAssign(R, Q);
    else{
	if(EcmIsZero(Q))
	    PointAssign(R, P);
	else{
	    if(EcmEqual(P, Q))
		EcmDouble(R, P, E);
	    else
		EcmPlus(R, P, Q, E);
	}
    }
}

/* R <- P + Q in the case P <> Q. Can do R = P */
EcmPlus(R, P, Q, E)
Point R, P, Q;
Ecm E;
{
    int gl, pl = BnnNumDigits(Q->x, Modul), gpl;

    /*	Pt <- x(Q)-x(P) */
    ModAssign(Pbuf, Q->x);
    BmSubtract(Pbuf, &pl, P->x, Modul);
    if(BnnIsZero(Pbuf, pl)){
	/* Pt = 0, so that x(P) = x(Q) */
	/* Pt <- y(Q)+y(P) */
	ModAssign(Pbuf, Q->y);
	pl = BnnNumDigits(Pbuf, Modul);
	BmAdd(Pbuf, &pl, P->y, Modul);
	if(!BnnIsZero(Pbuf, pl))
	    /* P = Q, we double */
	    EcmDouble(R, P, E);
	else
	    /* P = -Q */
	    PointSetToZero(R);
    }
    else{
	/* Pbuf <- 1/PBuf */
	if(!(BnnEeaWithCopy(Gbuf, &gl, Pbuf, &gpl, Modu, Modul, Pbuf, pl))){
	    printf("Non invertible in EcmPlus\n");
	    exit(0);
	}
	/* Gt <- y(Q) - y(P)*/
	ModAssign(Gbuf, Q->y);
	BnnSetDigit((Gbuf+Modul), 0);
	gl = Modul;
	BmSubtract(Gbuf, &gl, P->y, Modul);
	BmMultiply(Pbuf, &pl, Pbuf, gpl, Gbuf, gl);
	EcmCommon(R, P, Q, pl, E);
    }
}

/* R <- 2P. Stands R = P */
EcmDouble(R, P, E)
Point R, P;
Ecm E;
{
    if(EcmIsZero(P))
	PointSetToZero(R);
    else{
	int pl, gpl, gl;

	/* Pt <- 2*y(P) */
	ModAssign(Pbuf, P->y);
	pl = BnnNumDigits(Pbuf, Modul);
	BmAdd(Pbuf, &pl, P->y, Modul);
	if(BnnIsZero(Pbuf, pl))
	    /* P has order 2 */
	    PointSetToZero(R);
	else{
	    /* computing the tangent */
	    /* Pbuf <- 1/Pbuf */
	    if(!(BnnEeaWithCopy(Gbuf,&gl,Pbuf,&gpl, Modu, Modul, Pbuf, pl))){
		printf("Non invertible in EcmDouble\n");
		exit(0);
	    }
	    /* Gt <- 3*x^2+a*/
	    BmSquare(Gbuf, &gl, P->x, Modul);
	    BmMultiply(Gbuf, &gl, Gbuf, gl, (CONSTANTES+3), 1);
	    BmAdd(Gbuf, &gl, E.a, Modul);
	    /* Pt <- Pt * Gt */
	    BmMultiply(Pbuf, &pl, Pbuf, gpl, Gbuf, gl);
	    EcmCommon(R, P, P, pl, E);
	}
    }
}

/* These computations are in common of all routines */
/* Pbuf contains the line and pl is its number of digits */
EcmCommon(R, P, Q, pl, E)
Point R, P, Q;
int pl;
Ecm E;
{
    int rxl, ryl;

    /* computing rx */
    BmSquare(ECMRX, &rxl, Pbuf, pl);
    BmSubtract(ECMRX, &rxl,  P->x, Modul);
    BmSubtract(ECMRX, &rxl,  Q->x, Modul);
    /* computing ry */
    ModAssign(ECMRY, P->x);
    ryl = Modul;
    BmSubtract(ECMRY, &ryl, ECMRX, rxl);
    BmMultiply(ECMRY, &ryl, ECMRY, ryl, Pbuf, pl);
    BmSubtract(ECMRY, &ryl, P->y, Modul);
    ModAssign(R->x, ECMRX);
    ModAssign(R->y, ECMRY);
    R->z = 1;
}

/* R <- kP on E. tands R = P */
EcmMult(R, k, kl, P, E)
Point R, P;
Ecm E;
BigNum k;
int kl;
{
    if(kl <= ECMEXPLIMIT)
	EcmMultPlain(R, k, kl, P, E);
    else{
	int lbloc;

	if(kl <= 3)
	    lbloc = 3;
	else{
	    if(kl <= 9)
		lbloc = 4;
	    else{
		if(kl <= 15)
		    lbloc = 5;
		else
		    lbloc = 6;
	    }
	}
	EcmMultBlock(R, k, kl, P, E, lbloc);
    }
}

/********** Binary method **********

/* R <- kP on E, with maybe R = P.*/
EcmMultPlain(R, k, kl, P, E)
Point R, P;
Ecm E;
BigNum k;
int kl;
{
    BigNumDigit C[1];
    Point Q = PointCopy(P);
    int i, kd, ks;

    kd = kl - 1;
    /* starting the computation */
    BnnAssign(C, (k+kd), 1);
    /* looking for the most significant bit */
    for(ks = BN_DIGIT_SIZE; ks; ks--){
        if(BnnShiftLeft(C, 1, 1))
            break;
    }
    if(debuglevel >= 15) printf("ks=%d\n", ks);
    if(!ks){
        fprintf(stderr, "Error, ks is 0\n");
        exit(0);
    }
    ks--;
    while(kl >= 1){
	for(i = 0; i < ks; i++){
	    EcmDouble(Q, Q, E);
#ifdef DEBUG
	    if(debuglevel >= 10){
		printf("Q^2="); PointPrint(Q); printf("\n");
	    }
#endif
/* I cannot understand what is going on here... !
	    if(EcmEqual(P, Q)){
		printf("HORROR\n");
		exit(0);
	    }
*/
	    if(BnnShiftLeft(C, 1, 1)){
		EcmAdd(Q, P, Q, E);
#ifdef DEBUG
		if(debuglevel >= 10)
		    {printf("Q*P="); PointPrint(Q); printf("\n");}
#endif
	    }
	}
	kl--;
	if(kl){
	    kd--; ks = BN_DIGIT_SIZE;
	    BnnAssign(C, (k+kd), 1);
	}
    }
    PointAssign(R, Q);
    PointFree(Q);
}

/********** Using blocks **********/

EcmMultBlock(R, k, kl, P, E, lbloc)
Point R, P;
BigNum k;
Ecm E;
int kl, lbloc;
{
    EcmMultPlain(R, k, kl, P, E);
}

#if 0
/*; Calcul de kP.*/
(de EcmMultBlock (R k kd kl P E lbloc)
   (let ((2**m-1 (** 2 (sub1 lbloc))))
    (let ((C (BnCreate 'N 2)) (ks BN_DIGIT_SIZE) (kk (BnCreate '#:R:Q:N kl))
	  (et 0) (qs 0) (rs 0) (OddPower (makevector (add1 2**m-1) ()))
	  (nbits (add1 BN_DIGIT_SIZE)) (nzeros 0) (Q (copy P)) )
	 (BnAssign kk 0 k kd kl)
/*	 ; precompute odd powers of P*/
	 (EcmPrecompute Oddpower P E)
/*	 ; calcul du nombre de bits de k*/
	 (BnAssign C 0 kk (sub1 kl) 1)
	 (until (neq (BnIsDigitOdd C 1) 0)
	        (BnShiftLeft C 0 1 C 1 1)
	        (setq nbits (sub1 nbits)))
	 (setq nzeros (sub BN_DIGIT_SIZE nbits))
	 (setq nbits (add (mul (sub1 kl) BN_DIGIT_SIZE) nbits))
	 (setq qs (div nbits lbloc) rs (rem nbits lbloc))
	 (when (eqn rs 0) (setq rs lbloc qs (sub1 qs)))
/*	; traitement du premier chiffre*/
/*	 ; re'cupe'ration du premier chiffre : C[0] <- k[t]=(2**lt)*ut*/
	 (setq rs (add rs nzeros))
	 (if (lt rs BN_DIGIT_SIZE)
/*	     ; pas de pb, on shifte*/
	     (progn
		 (BnShiftLeft kk 0 kl C 0 rs)
		 (setq et (BnGetDigit C 0)))
/*	     ; hum, hum...*/
	     (setq rs (sub rs BN_DIGIT_SIZE))
	     (BnShiftLeft kk 0 kl C 0 rs)
	     (setq kl (sub1 kl))
	     (setq et (BnGetDigit kk kl)))
/*	 ; Q <- ut P, avec ut impair*/
	 (PointAssign Q (vref OddPower (vref :TableU et)))
/*	 ; Q <- (2**lt)Q*/
	 (repeat (vref :TableL et)
	    (EcmDouble Q Q E))
/*	; boucle principale*/
         (repeat qs
/*	     ; re'cupe'ration du chiffre courant k[i]=(2**li)*ui*/
	     (BnShiftLeft kk 0 kl C 0 lbloc)
	     (setq et (BnGetDigit C 0))
/*	     ; Q <- (2**(m-lt))Q*/
	     (repeat (sub lbloc (vref :TableL et))
	         (EcmDouble Q Q E))
	     (when (neq et 0)
/*	        ; Q <- (a**vt)Q*/
	 	(EcmAdd Q (vref OddPower (vref :TableU et)) Q E))
/*	        ; Q <- (2**lt)Q*/
	        (repeat (vref :TableL et)
	            (EcmDouble Q Q E)))
	 (PointAssign R Q))))

/*; Pre'calcul des puissances impaires de P.*/
(de EcmPrecompute (OddPower P E)
/*; (step*/
    (let ((i 1) (2P (copy P)) (2**m-1 (** 2 (sub1 lbloc))))
	 (EcmDouble 2P 2P E)
/*	 ; calcul des puissances impaires*/
	 (vset OddPower 0 (copy P))
	 (while (le i 2**m-1)
/*	    ; cre'ation de P**...*/
	    (vset OddPower i (#:point:make))
	    (EcmAdd (vref OddPower i) (vref OddPower (sub1 i)) 2P E)
	    (setq i (add1 i)))))
)

#endif
