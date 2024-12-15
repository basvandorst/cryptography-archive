
/* $Log:	jacobi.c,v $
 * Revision 1.1  91/09/20  14:48:08  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:29  morain
 * Initial revision
 *  */

#include "mod.h"

/********** Jacobi Symbol **********/

/* Supposes q odd and p < q. Uses Gbuf */
/* Supposes p positive */
int BnJacobi(p, pd, pl, q, qd, ql)
BigNum p, q;
int pd, pl, qd, ql;
{
    BigNumDigit C[3];
    int r, al, jac = 1, gpd = 0, gpl = pl, gpf = pl-1;
    int gqd = pl+1, gql = ql, gqf = pl+ql, tmp;

    BnSetDigit(C, 1, 3);
    BnSetDigit(C, 2, 7);
    /* initialization of Gbuf */
    /* with q */
    BnAssign(Gbuf, gqd, q, qd, ql);
    BnSetToZero(Gbuf, gqf+1, 1);
    /* with p */
    BnAssign(Gbuf, gpd, p, pd, pl);
    BnSetToZero(Gbuf, gpf+1, 1);
    /* let's loop till gp <= 1 */
    while(BnnIsGtOne((Gbuf+gpd), gpl)){
	/* let us deal with powers of 2 */
	al = 0;
	while(!BnIsDigitOdd(Gbuf, gpd)){
	    al = 1 - al;
	    BnShiftRight(Gbuf, gpd, gpl, C, 0, 1);
	}
	if(al){
	    /* r <- gq mod 8 */
	    BnAssign(C, 0, Gbuf, gqd, 1);
	    BnAndDigits(C, 0, C, 2);
	    r = (int)(BnnGetDigit(C));
	    if((r == 3) || (r == 5)) jac = -jac;
	}
	/* updating addresses */
	gpl = BnNumDigits(Gbuf, gpd, gpl);
	gpf = gpd + gpl - 1;
	/* gp is odd, so use the reciprocity */
	if(BnnIsGtOne((Gbuf+gpd), gpl)){
	    BnAssign(C, 0, Gbuf, gpd, 1);
	    BnAndDigits(C, 0, C, 1);
	    if(((int)BnnGetDigit(C)) == 3){
		BnAssign(C, 0, Gbuf, gqd, 1);
		BnAndDigits(C, 0, C, 1);
		if(((int)BnnGetDigit(C)) == 3) jac = -jac;
	    }
	    /* let's divide Gbuf[gqd..gqd+gql-1][0] */
	    /* by Gbuf[gpd..gpd+gpl-1] */
	    BnSetDigit(Gbuf, gqf+1, 0);
	    BnDivide(Gbuf, gqd, gql+1, Gbuf, gpd, gpl);
	    /* remainder is on Gbuf[gqd..gqd+gpl-1] */
	    gql = BnNumDigits(Gbuf, gqd, gpl);
	    gqf = gqd + gql - 1;
	    /* permuting all the stuff */
	    tmp = gpd; gpd = gqd; gqd = tmp;
	    tmp = gpl; gpl = gql; gql = tmp;
	    tmp = gpf; gpf = gqf; gqf = tmp;
	}
    }
    /* when (p, q) <> 1, (p/q) = 0 */
    if(BnIsDigitZero(Gbuf, gpd)){
	printf("%%!%% Warning: ("); BnnPrint((p+pd), pl);
	printf("/"); BnnPrint((q+qd), ql); printf(") = 0\n");
	jac = 0;
    }
    return(jac);
}

#if 0
/* 
  Retunrs the jacobi symbol (n/m).
  We must have size(n) > nl, size(m) > ml and m >= n; n and 
  m are destroyed.
*/
int BnJacobiAux(n, nl, m, ml)
BigNum n, m;
int nl, ml;
{
    int tmpl;
    BigNum tmp;

    /* ordinary euclid */
    while(BnIsZero(m, 0, ml) == 0){
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf("m=%s\n", From(m, ml));
	    printf("n=%s\n", From(n, nl));
	}
#endif
	/* division of n[0..nl-1][0] by m[0..ml-1] */
	BnSetDigit(n, nl, 0);
	BnDivide(n, 0, nl+1, m, 0, ml);
	tmpl = BnNumDigits(n, 0, ml);
	nl = ml; ml = tmpl; tmp = n;
	n = m; m = tmp;
    }
    BnAssign(g, 0, n, 0, nl);
#ifdef DEBUG
    if(debuglevel >= 10) printf("g=%s\n", From(g, nl));
#endif
    return(nl);
}

#endif

#if 0
/* On suppose que abs(p) < q.    */
(de jacobi (p q)
    (let ((gl 0) (pl 0) (ql 0))
         ; coertion
         (when (fixp p) (setq p (fix>>N p 1)))
         (when (fixp q) (setq q (fix>>N q 1)))
         (setq pl (BnNumDigits p 0 (BnGetSize p))
	       ql (BnNumDigits q 0 (BnGetSize q)))
         (setq gl (add 2 (add pl ql)))
         (when (lt (BnGetSize Gbuf) gl)
	       (setq Gbuf (BnCreate '#:R:Q:N gl)))
         (BnJacobi p 0 pl q 0 ql)))

#endif

/* (p/q) with p a small fix and q an ODD Bn.*/
BigNumCmp LongJacobiBn(p, q, ql)
BigNum q;
long p;
int ql;
{
    if(p == 0)
	return(0);
    else{
	long r;
	BigNumCmp s = 1, jac = 1, LongJacobiLong(), al;
    
	if(p < 0){
	    s = -1;
	    p = -p;
	}
	/* r <- q mod 8 */
	r = (long)BnGetDigit(q, 0) & 0x0007;
	if(debuglevel >= 10) printf("q mod 8=%ld\n", r);
	if((s == -1) && (r % 4 == 3))
	    jac = -1;
	if(!(p % 2)){
	    al = 0;
	    while(!(p % 2)){
		al = 1 - al;
		p /= 2;
	    }
	    if(al){
		if((r == 3) || (r == 5))
		    jac = -jac;
	    }
	}
	if(debuglevel >= 10) printf("jac0=%d\n", jac);
	if(p > 1){
	    BigNum C = BNC(1);

	    if((p % 4 == 3) && (r % 4 == 3))
		jac = -jac;
	    BnSetDigit(C, 0, (BigNumDigit)p);
	    BnAssign(Zbuf, 0, q, 0, ql);
	    BnSetDigit(Zbuf, ql, 0);
	    BnDivideDigit(Zbuf, 1, Zbuf, 0, Zbuf, 0, ql+1, C, 0);
	    jac *= LongJacobiLong((int)BnGetDigit(Zbuf, 0), p);
	    BnFree(C);
	}
	return(jac);
    }
}

/* (p/q) with p Bn and q Long. Uses Zbuf. */
BigNumCmp BnJacobiLong(p, pl, q)
BigNum p;
int pl, q;
{
    BigNumDigit r = 0;
    BigNumCmp s = 1;
    
    /* ?????
       (when (equal (BnGetType p) '#:R:Q:Z)
       (setq sign -1))
       */
    BnSetDigit(SCRATCH, 0, q);
    BnAssign(Zbuf, 0, p, 0, pl);
    BnSetDigit(Zbuf, pl, 0);
    BnDivideDigit(Zbuf,1,Zbuf,0,Zbuf, 0, pl + 1, SCRATCH, 0);
    return(LongJacobiLong(BnGetDigit(Zbuf, 0), q)); /* Cf. sign ? */
}

/* (p/q) with p and q two integers.*/
BigNumCmp LongJacobiLong(p, q)
long p, q;
{
    long r;
    BigNumCmp jac = 1, al;
    
    if(p < 0){
	p = - p;
	if(q % 4 == 3)
	    jac = -1;
    }
    if(p > q)
	p = p % q;
    while(p > 1){
	/* let us deal with powers of 2 */
	if(!(p % 2)){
	    al = 0;
	    while(!(p % 2)){
		al = 1 - al;
		p /= 2;
	    }
	    if(al){
		r = q % 8;
		if((r == 3) || (r == 5))
		    jac = - jac;
	    }
	}
	if(p > 1){
	    /* at this point, p is odd and < q */
	    if((p % 4 == 3) && (q % 4 == 3))
		jac = - jac;
	    r = q % p; q = p; p = r;
	}
    }
    if(p == 1)
	return(jac);
    else
	return(0);
}

/* Application: looking for a nonresidue mod (N, Nl) starting from *p_nr. 
   If after nbtrial, none is found, then return 0 otherwise return 1. */
int FindNonresidue(p_nr, N, Nl, nbtrial)
long *p_nr;
BigNum N;
int Nl, nbtrial;
{
    long nr = (*p_nr)+1;
    int i;

    for(i = 0; i < nbtrial; i++){
	if(LongJacobiBn(nr, N, Nl) == -1)
	    break;
	else nr++;
    }
    if(i == nbtrial)
	return(0);
    else{
	*p_nr = nr;
	return(1);
    }
}

