
/* $Log:	clroutines.c,v $
 * Revision 1.1  91/09/20  14:53:16  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:51  morain
 * Initial revision
 *  */

/**********************************************************************
                      Routines for the CL test
**********************************************************************/

#include "mod.h"
#include "exp.h"

#define CLEXP_LIMIT 2

/* Uses Dbuf (= p0, s) and Pbuf (= p1) */

CLPrint(x, y)
BigNum x, y;
{
    printf("(");
    BnnPrint(x, Modul);
    printf(", ");
    BnnPrint(y, Modul);
    printf(")\n");
}

/********** Multiplication routines **********/

/* Case N = 1 mod 4 */
/* x0+x1*alpha <- (x0+x1*alpha)*(y0+y1*alpha) */
CLMul1(x0, x1, y0, y1, a)
BigMod x0, x1, y0, y1, a;
{
    /* p0 <- x0*y0 */
    ModMultiply(Dbuf, x0, y0);
    /* p1 <- x1*y1 */
    ModMultiply(Pbuf, x1, y1);
    /* x0 <- x0+x1 (= s0) */
    ModAdd(x0, x1);
    /* x1 <- y0+y1 (= s1) */
    ModAssign(x1, y0);
    ModAdd(x1, y1);
    /* x1 (= z1) <- s0*s1-p0-p1 */
    ModMultiply(x1, x1, x0);
    ModSubtract(x1, Dbuf);
    ModSubtract(x1, Pbuf);
    /* x0 (= z0) <- p0+a*p1 */
    ModAssign(x0, Pbuf);
    ModMultiplyDigit(x0, a, 0);
    ModAdd(x0, Dbuf);
}

/* Case N = 3 mod 4 */
/* x0+x1*alpha <- (x0+x1*alpha)*(y0+y1*alpha) */
CLMul3(x0, x1, y0, y1, u)
BigMod x0, x1, y0, y1, u;
{
    /* p0 <- x0*y0 */
    ModMultiply(Dbuf, x0, y0);
    /* p1 <- x1*y1 */
    ModMultiply(Pbuf, x1, y1);
    /* x0 <- x0+x1 (= s0) */
    ModAdd(x0, x1);
    /* x1 <- y0+y1 (= s1) */
    ModAssign(x1, y0);
    ModAdd(x1, y1);
    /* x1 (= z1) <- s0*s1+(u-1)*p1-p0 */
    ModMultiply(x1, x1, x0);
    ModSubtract(x1, Dbuf);
    /* x0 (= z0) <- p0+p1 */
    ModAssign(x0, Dbuf);
    ModAdd(x0, Pbuf);
    /* end of x1 */
    BnnSubtractBorrow(u, 1, 0); /* is u necessary or u-1 enough to keep ? */
    ModMultiplyDigit(Pbuf, u, 0);
    BnnAddCarry(u, 1, 1);
    ModAdd(x1, Pbuf);
}

/* Squaring routine for numbers of norm +1 in A */
CLSquare(x0, x1, u)
BigNum x0, x1, u;
{
    /* s <- u*x1+2*x0 */
    ModAssign(Dbuf, x1);
    ModMultiplyDigit(Dbuf, u, 0);
    ModAdd(Dbuf, x0); ModAdd(Dbuf, x0);
    /* x0 <- x0*s-1 */
    ModMultiply(x0, x0, Dbuf);
    ModSubtractDigit(x0, CONSTANTES, 1);
    /* x1 <- x1 * s */
    ModMultiply(x1, x1, Dbuf);
}

/* Exponentiation routine in A for (x0, x1) of norm 1 */
CLExp(y0, y1, x0, x1, u, a, e, el)
BigNum y0, y1, x0, x1, u, a, e;
int el;
{
    int lbloc;
    double tp, runtime();

    if(el <= CLEXP_LIMIT){
	tp = runtime();
	CLExpPlain(y0, y1, x0, x1, u, a, e, el);
#ifdef DEBUG
	if(debuglevel >= 5)
	    printf("%% Time_Clexp(plain)=%lf\n", (runtime()-tp)/1000.);
#endif
    }
    else{
	if(el <= 16) lbloc = 2; else lbloc = 7;
	tp = runtime();
	CLExpBlock(y0, y1, x0, x1, u, a, e, el, lbloc);
#ifdef DEBUG
        if(debuglevel >= 5)
	    printf("%% Time_CLexp(%d)=%lf\n", lbloc, (runtime()-tp)/1000.);
#endif
    }
}

/********** The ordinary binary method **********/

/* y0+alpha*y1 <- (x0+alpha*x1)^(e, el) */
CLExpPlain(y0, y1, x0, x1, u, a, e, el)
BigNum y0, y1, x0, x1, u, a, e;
int el;
{
    BigNumDigit C[1];
    int ed = el-1, es = BN_DIGIT_SIZE, i, Nmod4;
    
    ModAssign(y0, x0);
    ModAssign(y1, x1);
    BnnSetDigit(C, 3);
    BnnAndDigits(C, *Modu);
    Nmod4 = BnnGetDigit(C);
    BnnAssign(C, (e+ed), 1);
    /* recherche du bit le plus significatif */
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
	    /* (y0, y1) <- (y0, y1)^2 */
	    CLSquare(y0, y1, u);
#ifdef DEBUG
	    if(debuglevel >= 10)
		{printf("y^2="); CLPrint(y0, y1);}
#endif
	    if(BnnShiftLeft(C, 1, 1)){
		/* (y0, y1) <- (y0, y1)*(x0, x1) */
		if(Nmod4 == 1)
		    CLMul1(y0, y1, x0, x1, a);
		else
		    CLMul3(y0, y1, x0, x1, u);
#ifdef DEBUG
		if(debuglevel >= 10)
		    {printf("y*x="); CLPrint(y0, y1);}
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
}

/* 
   Computing (y0, y1)=(x0, x1)**e in A. We write: 
   e=e[t]*(2**m)**t + ... + e[0] and e[i]=(2**li)*ui, e[t] <> 0.
*/
CLExpBlock(y0, y1, x0, x1, u, a, e, el, lbloc)
BigNum y0, y1, x0, x1, u, a, e;
int el, lbloc;
{
    BigNumDigit C[1];
    BigNum ee = BNC(el), tmp;
    BigNum OP0[TWOPOWERMARYMAX], OP1[TWOPOWERMARYMAX];
    int twopowermminus1 = (1 << (lbloc-1)), es = BN_DIGIT_SIZE, i, j;
    int et = 0, qs, rs, nbits = BN_DIGIT_SIZE+1, nzeros, tl, Nmod4;

    if(lbloc != currentmary){
#ifdef DEBUG
	if(debuglevel >= 8) printf("%% Changing m-ary to %d\n", lbloc);
#endif
	currentmary = lbloc;
	MakeTables(lbloc);
    }
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("%% (x0, x1)=");
	CLPrint(x0, x1);
    }
#endif
    /* copy of e */
    BnnAssign(ee, e, el);
    /* precompute odd powers of a */
    CLExpPrecompute(OP0, OP1, x0, x1, u, a, lbloc);
    /* starting the computation */
    BnnAssign(C, (ee+el-1), 1);
    /* looking for the most significant bit */
    for(nbits = BN_DIGIT_SIZE; nbits; nbits--){
	if(BnnShiftLeft(C, 1, 1))
	    break;
    }
    /* nbits is the number of bits of ee[el-1] */
    if(!nbits){
	 fprintf(stderr, "Error, nbits is 0\n");
        exit(0);
     }
    nzeros = BN_DIGIT_SIZE - nbits;
    nbits = nbits + (el-1) * BN_DIGIT_SIZE;
    qs = nbits / lbloc; rs = nbits % lbloc;
    if(!rs){rs = lbloc; qs--;}
#ifdef DEBUG
    if(debuglevel >= 10) printf("nbits=%d qs=%d rs=%d\n", nbits, qs, rs);
#endif
    /* dealing with the most significant digit */
    /* C[0] <- e[t]=(2**lt)*ut */
    rs += nzeros;
    if(rs < BN_DIGIT_SIZE){
	/* no pb, let's shift */
        et = BnnShiftLeft(ee, el, rs);
    }
    else{
	/* hum, hum... */
	rs -= BN_DIGIT_SIZE;
	BnnShiftLeft(ee, el, rs);
	el--;
	et = BnnGetDigit((ee+el));
    }
    /* (y0, y1) <- (x0, x1)^ut, with ut odd */
    ModAssign(y0, OP0[TableU[et]]);
    ModAssign(y1, OP1[TableU[et]]);
    /* (y0, y1) <- (y0, y1)**(2**lt) */
    for(i = 0; i < TableL[et]; i++)
	CLSquare(y0, y1, u);
#ifdef DEBUG
    if(debuglevel >= 10)
	{printf("(y0, y1)^2="); CLPrint(y0, y1);}
#endif
    /* loop */
    if(BnnIsZero(u, 1)) Nmod4 = 1; else Nmod4 = 3;
    for(i = 0; i < qs; i++){
	/* re'cupe'ration du chiffre courant e[i]=(2**li)*ui */
        et = BnnShiftLeft(ee, el, lbloc);
	/* (y0, y1) <- (y0, y1)**(2**(lbloc-lt)) */
	for(j = 0; j < lbloc-TableL[et]; j++)
	    CLSquare(y0, y1, u);
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("(y0, y1)^2="); CLPrint(y0, y1);}
#endif
	if(et){
	    /* (y0, y1) <- (y0, y1) * (x0, x1)**vt */
	    if(Nmod4 == 1)
		CLMul1(y0, y1, OP0[TableU[et]], OP1[TableU[et]], a);
	    else
		CLMul3(y0, y1, OP0[TableU[et]], OP1[TableU[et]], u);
#ifdef DEBUG
	    if(debuglevel >= 10)
		{printf("(y0, y1)*="); CLPrint(y0, y1);}
#endif
	    /* (y0, y1) <- (y0, y1)**(2**lt) */
	    for(j = 0; j < TableL[et]; j++)
		CLSquare(y0, y1, u);
#ifdef DEBUG
	    if(debuglevel >= 10)
		{printf("(y0, y1)^2="); CLPrint(y0, y1);}
#endif
	}
    }
    BnFree(ee);
}

/* Precomputing odd powers of (x0, x1) */
int CLExpPrecompute(OP0, OP1, x0, x1, u, a, lbloc)
BigMod OP0[], OP1[];
BigNum x0, x1, u, a;
int lbloc;
{
    BigNum s0 = BmCreate(), s1 = BmCreate();
    int i, twopowermminus1 = (1 << (lbloc-1)), Nmod4;
    double tp, runtime();

    tp = runtime();
    if(BnnIsZero(u, 1)) Nmod4 = 1; else Nmod4 = 3;
    ModAssign(s0, x0); ModAssign(s1, x1);
    CLSquare(s0, s1, u);
    OP0[0] = BmCreate(); ModAssign(OP0[0], x0);
    OP1[0] = BmCreate(); ModAssign(OP1[0], x1);
    for(i = 1; i <= twopowermminus1; i++){
	OP0[i] = BmCreate(); ModAssign(OP0[i], s0);
	OP1[i] = BmCreate(); ModAssign(OP1[i], s1);
	if(Nmod4 == 1)
	    CLMul1(OP0[i], OP1[i], OP0[i-1], OP1[i-1], a);
	else
	    CLMul3(OP0[i], OP1[i], OP0[i-1], OP1[i-1], u);
    }
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("%% Time for precomputation(%d) is ", lbloc);
	printf("%lf s\n", (runtime()-tp)/1000.);
    }
#endif
}

/* N[alpha](x0, x1) = x0^2 - a * x1^2 + x0 * x1 * u mod N */
CLNorm(norm, x0, x1, u, a)
BigNum norm, x0, x1, u, a;
{
    if(BnnIsDigitZero(*u))
	CLNorm1(norm, x0, x1, a);
    else
	CLNorm3(norm, x0, x1, u);
}

CLNorm1(norm, x0, x1, a)
BigNum norm, x0, x1, a;
{
    ModAssign(Dbuf, x1);
    ModMultiply(Dbuf, Dbuf, Dbuf);
    ModMultiplyDigit(Dbuf, a, 0);
    ModMultiply(norm, x0, x0);
    ModSubtract(norm, Dbuf);
}

CLNorm3(norm, x0, x1, u)
BigNum norm, x0, x1, u;
{
    ModAssign(norm, x0);
    ModMultiply(norm, norm, norm);
    ModAssign(Dbuf, x0);
    ModMultiplyDigit(Dbuf, u, 0);
    ModSubtract(Dbuf, x1);
    ModMultiply(Dbuf, Dbuf, x1);
    ModAdd(norm, Dbuf);
}
