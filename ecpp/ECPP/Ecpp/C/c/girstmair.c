
/* $Log:	girstmair.c,v $
 * Revision 1.1  91/09/20  14:54:15  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:51  morain
 * Initial revision
 *  */

/**********************************************************************

              Solving by radicals "a` la" Girstmair

**********************************************************************/

#include <math.h>

#include "ecpp.h"

int SolveByRadicals(u, p_ul, D, sqrtd, p_inv, file_of_u)
BigNum u, sqrtd;
long D;
char *file_of_u;
int *p_inv, *p_ul;
{
    BigNum m = BmCreate(), mm = BmCreate();
    FILE *fopen(), *ufile;
    double runtime(), tpcyc, log();
    long D0, fD[TFMAX];
    int h, g, e, g0, c, qstar[10], t, i, den, cyclic, ul, fac, k, ml, sl;

    if((ufile = fopen(file_of_u, "r")) == NULL){
	if(verbose) printf("%% File %s does not exist\n", file_of_u);
	return(0);
    }
    while(1){
	if(!ExtractDfDh(&D0, fD, &h, &g, ufile)){
	    printf("%% Data for %ld not in %s\n", D, file_of_u);
	    return(0);
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
    /* Is this useful ? */
    ModAssign(mm, Modu);
    BnnSubtractBorrow(mm, Modul, 0);
    k = ComputeValuation(m, &ml, mm, Modul, h);
#if 0
    if(!k || (((double)(k-1))*log((double)h) > LQMAX)){
	printf("%% v_%d(p-1)=%d too high\n", h, k);
	return(0);
    }
#endif
    if(!k){
	BnFree(m); BnFree(mm);
	return(0);
    }
    tpcyc = runtime();
    /* read invariant code, g0, cyclic */
    fscanf(ufile, "%d %d %d", p_inv, &g0, &cyclic);
#ifdef DEBUG
    if(verbose){
	printf("%% D=%ld h=%d g=%d invcode=%d g0=%d", D, h, g, *p_inv, g0);
	printf(" cyclic=%d\n", cyclic);
    }
#endif
    if(verbose) printf("%% Now entering the marvelous Galois solver\n");
    if(h < 0) h = -h;
    ModSetToZero(u);
#ifdef DEBUG
    if(debuglevel >= 6) 
	{printf("%% sqrtd="); BnnPrint(sqrtd, Modul); printf("\n");}
#endif
    fac = ModComputeRadicals(u, sqrtd, h, 1, ufile);
    *p_ul = BnnNumDigits(u, Modul);
    fclose(ufile);
    BnFree(m); BnFree(mm);
    tpcyc = (runtime()-tpcyc)/1000.;
    if(verbose) printf("%% tpcyc=%lfs\n", tpcyc);
    return(fac);
}

/* sqrtd = sqrt(-D) mod Modu */
int ModComputeRadicals(x0, sqrtd, h, g, ufile)
BigNum x0, sqrtd;
int h, g;
FILE *ufile;
{
    BigNum zetapow[HMAX], y = BmCreate(), z1 = BmCreate(), z = BmCreate();
    BigNum yk = BmCreate();
    int j, k, cl, yl, zl, zp0l, zp1l;

    /* finds zeta, a primitive root of 1 mod Modu */
    zetapow[0] = BmCreate();
    zetapow[1] = BmCreate();
    FindPrimitiveRootModp(zetapow[0], &zp0l, zetapow[1], &zp1l, h);
#ifdef DEBUG
    if(debuglevel >= 6)
	{printf("%% zeta="); BnnPrint(zetapow[1], Modul); printf("\n");}
#endif
    /* putting powers of zeta in zetapow */
    for(j = 2; j < h; j++){
	zetapow[j] = BmCreate();
	ModMultiply(zetapow[j], zetapow[j-1], zetapow[1]);
#ifdef DEBUG
	if(debuglevel >= 6){
	    printf("%% zeta^%d=", j); BnnPrint(zetapow[j], Modul); printf("\n");
	}
#endif
    }
    /* computing z1 = z[1] */
    ComputeZ(z1, h, g, sqrtd, zetapow, ufile);
    ModAssign(z, z1);
#ifdef DEBUG
    if(debuglevel >= 6)	{printf("%% z[1]="); BnnPrint(z1, Modul); printf("\n");}
#endif
    /* computing y s.t. y^h = z1/2 */
    BmShiftRight(z, Modul, 1);
    zl = BnnNumDigits(z, Modul);
    if(!ModBinRootOddPrimeZ(y, &yl, z, zl, h, zetapow[0], &zp0l)) return(0);;
#ifdef DEBUG
    if(debuglevel >= 6){printf("%% y="); BnnPrint(y, yl); printf("\n");}
#endif
    /* computing h x_0 = y + sum(z[k]*y^k, k=2..(h-1))/z1 + z[d]/2 */
    ModAssign(yk, y);
    for(k = 2; k < h; k++){
	ModMultiply(yk, yk, y);
	ComputeZ(z, h, g, sqrtd, zetapow, ufile);
#ifdef DEBUG
	if(debuglevel >= 6)
	    {printf("%% z[%d]=", k); BnnPrint(z, Modul); printf("\n");}
#endif
	ModMultiply(z, z, yk);
	ModAdd(x0, z);
    }
#ifdef DEBUG
    if(debuglevel >= 6)
	{printf("%% sum(z[k]*y^k)="); BnnPrint(x0, Modul); printf("\n");}
#endif
    /* x0 <- x0/z1 */
    ModSetToZero(z);
    EcppEea(z, &cl, z1, BnnNumDigits(z1, Modul));
    ModMultiply(x0, x0, z);
#ifdef DEBUG
    if(debuglevel >= 6){printf("%% x0/z1="); BnnPrint(x0, Modul); printf("\n");}
#endif
    ModAdd(x0, y);
    /* z[d] is in fact -(x_1+...+x_h) */
    ReadBigModFromFile(z, &cl, ufile);
    ModAdd(x0, z);
    /* x0 <- x0 / h */
    BnnSetDigit(z1, h);
    EcppEea(z, &cl, z1, 1);
    ModMultiply(x0, x0, z);
#ifdef DEBUG
    if(debuglevel >= 6) {printf("%% x0="); BnnPrint(x0, Modul); printf("\n");}
#endif
    for(k = 0; k < h; k++)
	BnFree(zetapow[k]);
    BnFree(y); BnFree(yk);
    BnFree(z); BnFree(z1); 
    return(1);
}

/* z[k] = sum(c[k, j]*zetapow[h-j], j=1..(h-1)); zetapow[j]=zeta^j mod Modu */
ComputeZ(z, h, g, sqrtd, zetapow, ufile)
BigNum z, sqrtd, zetapow[];
int h, g;
FILE *ufile;
{
    BigNum c = BmCreate(), cy = BmCreate();
    int cyl, cl, j;

    ModSetToZero(z);
    for(j = 1; j < h; j++){
	/* reading the real part of c[k, j] */
	ReadBigModFromFile(cy, &cyl, ufile);
	/* reading the imaginray part */
	ReadBigModFromFile(c, &cl, ufile);
	/* c <- Real(c)+sqrt(-D)*Im(c) */
	ModMultiply(c, c, sqrtd);
	ModAdd(c, cy);
	ModMultiply(c, c, zetapow[h-j]);
	ModAdd(z, c);
    }
    BnFree(c); BnFree(cy); 
}

