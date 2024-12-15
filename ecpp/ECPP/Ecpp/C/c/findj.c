
/* $Log:	findj.c,v $
 * Revision 1.1  91/09/20  14:54:04  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:41  morain
 * Initial revision
 *  */

/**********************************************************************
                Computing j mod (Modu, Modul) 
**********************************************************************/

#include "ecpp.h"
#include "exp.h"
#include "poly.h"

/*
; Pour calculer j, on calcule j(u), avec u racine d'un polyno^me de degre'
; e=h/g a` coefficients dans Krg. 
; invar is defined according to the following table
; Coding invariants
; 0: 	j                    D =    0 mod 3
; 1:	gamma_2              D =    3 mod 8
; 2:	f/sqrt(2)            D =    7 mod 8
; 3:	f1^2/sqrt(2)         d = +/-2 mod 8 (d=D/4)
; 4:	f^4                  d =    5 mod 8 (d=D/4)
; 5:	f^2/sqrt(2)          d =    1 mod 8 (d=D/4)
*/

/* 
  Computes j for D modulo (Modu, Modul) and returns
   -1 if no root has been found (pb in Berlekamp),
    1 if j is a root,
    0 if j is not a root.
*/
int findj(j, p_jl, D, h, g, sqrtd)
BigNum j, sqrtd;
int *p_jl;
long D;
int h, g;
{
    BigNum u = BmCreate();
    BigNumDigit C[1];
    int ul, fac, invcode;

    /* building a string */
    if(h < 0) h = -h;
    fac = findinvariant(u, &ul, &invcode, D, h, g, sqrtd);
#ifdef DEBUG
    if(debuglevel >= 6)	{printf("%% u="); BnnPrint(u, ul); printf("\n");}
#endif
    /* Temporary HORROR */
    if((h == 1) && (g == 1)){
	ModAssign(j, u);
	*p_jl = ModNegate(j);
	BmFree(u);
	return(fac);
    }/* END OF TMP HORROR */
    if(fac == 1){
	if(invcode <= 1){
	    if(!invcode){
		/* u = j */
		ModAssign(j, u);
		*p_jl = ul;
	    }
	    else
		/* u = gamma_2 */
		BmExp(j, p_jl, u, ul, (CONSTANTES+3), 1);
	}
	else{
	    BigNum iu = BmCreate();
	    int iul;

	    if(invcode == 2){
		/* u = f(sqrt(-D))/sqrt(2) = 1/f((1+sqrt(-D))/2) */
		BnnSetDigit(C, 24);
		BmExp(iu, &iul, u, ul, C, 1);
		/* u <- 1/iu */
		EcppEea(u, &ul, iu, iul);
	    }
	    else{
		if(invcode == 4){
		    /* computing f^24 = u^6 */
		    BnnSetDigit(C, 6);
		    BmExp(u, &ul, u, ul, C, 1);
		}
		else{
		    if((invcode == 5) || (invcode == 3)){
			/* u <- 2^6 * u^12 = (2 u^2)^6 */
			BmSquare(u, &ul, u, ul);
			BmAdd(u, &ul, u, ul);
			BnnSetDigit(C, 6);
			BmExp(u, &ul, u, ul, C, 1);
			if(invcode != 5){
			    /* u is in fact f1^24 */
			    ModNegate(u);
			    ul = BnnNumDigits(u, Modul);
			}
		    }
		}
	    }
	    /* at this point, some f is computed in u and j = (u-16)^3/u */
	    ModAssign(j, u);
	    BnnSetDigit(C, 16);
	    *p_jl = ul;
	    BmSubtract(j, p_jl, C, 1);
	    BnnSetDigit(C, 3);
	    /* j <- (u-16)^3 */
	    BmExp(j, p_jl, j, *p_jl, C, 1);
	    if(invcode != 2)
		/* trick since iu already computed */
		EcppEea(iu, &iul, u, ul);
	    /* j <- j * (iu) */
	    BmMultiply(j, p_jl, j, *p_jl, iu, iul);
	    BmFree(iu);
	}
    }
    BmFree(u);
    return(fac);
}

/* Build the correct polynomial from listofcoeff:
px = den*x^e+P1*x^(e-1)+...+Pe,
with Pi in Q(sqrt(qs[1]), ..., sqrt(qs[t])) in the basis alpha[0..g-1]
lP has the shape
u1 ... ut a
a(1,g-1)...a(1,0)
a(2,g-1)...a(2,0)...a(e,g-1)...a(e,0))
den is a small power of 2 (<= g)

This works also when g = 1 ...!
*/
Poly ReadPoly(h, g, ufile)
int h, g;
FILE *ufile;
{
    Poly px = PolyCreate(h/g);
    BigMod y = BmCreate(), z = BmCreate(), c0 = BmCreate(), yy = BmCreate();
    BigNum cy = BNC(1+(Modul << 1)), alpha[GMAX];
    long den, pg, qstar[GMAX];
    int e, i, t, lcy = 1 + (Modul << 1), lden = 0;
    int yl, zl, cyl, c0l, j, ij, yyl, k, gg = g, tmpl;

    e = h/g;
    px->deg = e;
    t = -1; while(gg){t++; gg >>= 1;}
    /* Constructing the alpha base */
    for(k = 0; k < t; k++){
	j = (1 << k);
	fscanf(ufile, "%ld", &qstar[j]);
#ifdef DEBUG
	if(debuglevel >= 10) printf("q*[%d]=%ld ", j, qstar[j]);
#endif
	yl = ModFromLong(y, qstar[j]);
	alpha[j] = BmCreate();
	EcppMdSqrt(alpha[j], &tmpl, y, yl);
#ifdef DEBUG
	if(debuglevel >= 6)
	    {printf("%% Sqrt_%d=", j); BnnPrint(alpha[j], Modul); printf("\n");}
#endif
	for(i = 1; i < j; i++){
	    ij = i+j;
	    qstar[ij] = qstar[i] * qstar[j];
#ifdef DEBUG
	    if(debuglevel >= 10) printf("q*[%d]=%ld\n", ij, qstar[ij]);
#endif
	    alpha[ij] = BmCreate();
	    ModMultiply(alpha[ij], alpha[i], alpha[j]);
	    pg = LongGcdLong(qstar[i], qstar[j]);
	    if(pg != 1){
		/* z <- (A[i], A[j]) */
		qstar[ij] = qstar[ij]/(pg * pg);
		BnnSetDigit(z, pg);
		EcppEea(z, &zl, z, 1);
		ModMultiply(alpha[ij], alpha[ij], z);
	    }
#ifdef DEBUG
	    if(debuglevel >= 8){
		printf("alpha[%d+%d]=", i, j); 
		BnnPrint(alpha[ij], Modul);
		printf("\n");
	    }
#endif
	}
    }
    /* trick enabling the lecture of any polynomial */
    if(g > 1){
	fscanf(ufile, "%ld", &den);
#ifdef DEBUG
	if(debuglevel >= 10) printf("Denominator is %ld\n", den);
#endif
    }
    else den = 1;
    /* computing lden=log_2(den) */
    while(den != 1){den /= 2; lden++;}
    /* reading the Pe(i)'s */
    for(i = e-1; i >= 0; i--){
	/* the leading coefficient */
	BnnSetToZero(cy, lcy);
	ReadBigModFromFile(cy, &cyl, ufile);
#ifdef DEBUG
	if(debuglevel >= 8)
	    {printf("cy="); BnnPrint(cy, cyl); printf("\n");}
#endif
	/* evaluation of Pi=c[0]*a[0]+... */
	for(j = 1; j < g; j++){
	    /* reading the coefficient */
	    BnnSetToZero(c0, Modul+1);
	    ReadBigModFromFile(c0, &c0l, ufile);
#ifdef DEBUG
	    if(debuglevel >= 8)
		{printf("c0="); BnnPrint(c0, c0l); printf("\n");}
#endif
	    /* cy <- cy+c0*a[j] */
	    BnnMultiply(cy, lcy, c0, c0l, alpha[j], Modul);
	    BnnDivide(cy, lcy, Modu, Modul);
	    BnnSetToZero((cy+Modul),  Modul+1);
	    cyl = BnnNumDigits(cy, Modul);
	}
	/* on stocke cy/a = cy/2^la */
	if(lden) cyl = BmShiftRight(cy, cyl, lden);
	BnnAssign(px->coeff[i], cy, Modul);
    }
    BnnSetDigit(px->coeff[e], 1);
    BmFree(y); BmFree(z); BmFree(c0); BmFree(yy); BnFree(cy);
    for(i = 1; i < g; i++) BmFree(alpha[i]);
    return(px);
}

/* Reads a polynomial in u, *p_inv contains the invariant code
   g0 tells the program that the polynomial is supposed to be factored over
   Krg if it is > 0.*/
int getinvariant(u, p_ul, D, p_inv, file_of_u)
BigNum u;
long D;
char *file_of_u;
int *p_inv, *p_ul;
{
    Poly px;
    FILE *fopen(), *ufile;
    double tpber, runtime();
    long D0, fD[TFMAX];
    int h, g, e, g0, c, qstar[10], t, i, den, fac;

    if((ufile = fopen(file_of_u, "r")) == NULL){
	printf("%% Sorry, file %s does not exist\n", file_of_u);
	exit(0);
    }
    while(1){
	if(!ExtractDfDh(&D0, fD, &h, &g, ufile)){
	    printf("Error, data for %ld not in %s\n", D, file_of_u);
	    return(NULL);
	}
#ifdef DEBUG
	if(debuglevel >= 15) printf("D0=%ld\n", D0);
#endif
	if(D == D0)
	    break;
	else{
	    /* must read till a closing parenthesis */
	    while((c = getc(ufile)) != ')');
	    while((c = getc(ufile)) != '\n');
	}
    }
    /* read invariant code and g0 */
    fscanf(ufile, "%d %d", p_inv, &g0);
#ifdef DEBUG
    if(verbose) 
	printf("%% D=%ld h=%d g=%d invcode=%d g0=%d\n", D, h, g, *p_inv, g0);
#endif
    if(h < 0) h = -h;
    if(g0)
	/* the polynomial has coefficients in a compositum of quadratic
	   fields; there are ... fields in it */
	px = ReadPoly(h, g, ufile);
    else px = ReadPoly(h, 1, ufile);
#ifdef DEBUG
    if(debuglevel >= 1) PolyPrint(px);
#endif
    fclose(ufile);
    PolyInit(px->deg);
    tpber = runtime();
    fac = RootOfPolyModp(u, p_ul, px);
    tpber = (runtime()-tpber)/1000.;
    if(verbose) printf("%% tpber=%lfs\n", tpber);
    PolyFree(px); 
    PolyClose();
    return(fac);
}

/* 
  Computes the invariant for D modulo (Modu, Modul) and returns
   -1 if no root has been found (pb in Berlekamp),
    1 if j is a root,
    0 if j is not a root.
*/
int findinvariant(u, p_ul, p_inv, D, h, g, sqrtd)
BigNum u, sqrtd;
long D;
int *p_ul, *p_inv, h, g;
{
    char jname[200];
    int fac;

    sprintf(jname, "%s%s/h%dg%d.cwdx", path, JPATH, h, g);
#ifdef BENCH
    fac = SolveByRadicals(u, p_ul, D, sqrtd, p_inv, jname);
    sprintf(jname, "%s%s/h%dg%d.wwdx", path, JPATH, h, g);
    fac = getinvariant(u, p_ul, D, p_inv, jname);
#else
    if(!(fac = SolveByRadicals(u, p_ul, D, sqrtd, p_inv, jname))){
	sprintf(jname, "%s%s/h%dg%d.wwdx", path, JPATH, h, g);
#ifdef DEBUG
	if(debuglevel >= 10) printf("Name is %s\n", jname);
#endif
	fac = getinvariant(u, p_ul, D, p_inv, jname);
    }
#endif
    if(verbose) printf("%% j has been computed\n");
    return(fac);
}

