
/* $Log:	chinese.c,v $
 * Revision 1.1  91/09/20  14:47:54  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:14  morain
 * Initial revision
 *  */

/**********************************************************************

                        Chinese remainder theorem

**********************************************************************/

#include "mod.h"
#include "chinese.h"

/* computes i = 1/a mod p */
SmallInvMod(p_i, a, p)
long *p_i, a, p;
{
    long u0 = 1, u = 0, d = -1, r = p, m;
    
    while(r > 1){
	d = -d;
	m = a % r;
	a = u0 + u * (a / r);
	u0 = u; u = a; a = r; r = m;
    }
    if(d < 0) u = p - u;
    *p_i = u;
}

ChinesePrecompute(tp, tc, k)
long tp[], tc[];
int k;
{
    BigNum z = BnCreate((BigNumType)1, k+1), w = BnCreate((BigNumType)1, k+1);
    BigNum tmp;
    int j, zl;

    tc[0] = 1;
    BnnSetDigit(z, 1);
    zl = 1;
    for(j = 1; j < k; j++){
	/* prod <- prod * tp[j-1] mod tp[j] */
	BnnSetToZero(w, zl+1);
	BnnMultiplyDigit(w, zl+1, z, zl, tp[j-1]);
	BnnSetDigit(z, BnnDivideDigit(z+1, w, zl+1, tp[j]));
	tmp = w; w = z; z = tmp;
	zl = BnnNumDigits(z, zl+1);
	SmallInvMod(&(tc[j]), BnnGetDigit(w), tp[j]);
#ifdef DEBUG
	if(debuglevel >= 10) printf("tc[%d]=%ld\n", j, tc[j]);
#endif
    }
    BnFree(z); BnFree(w);
}

/* Returns x[0..xl-1] s.t. x = tx[i] for all i. Assumes that x is a bignum
   large enough (typically, size(x) > k. */
ChineseRecover(x, p_xl, tp, tc, tx, k)
BigNum x;
long tp[], tc[], tx[];
int *p_xl;
{
    BigNum xx =  BnCreate((BigNumType)1, k+1);
    BigNum z = BnCreate((BigNumType)1, k+1), w = BnCreate((BigNumType)1, 2);
    BigNum tmp, zy = BnCreate((BigNumType)1, 2);
    long y[CHMAX];
    int j, r, xl, zl;

#ifdef DEBUG
    if(debuglevel >= 10){
	for(j = 0; j < k; j++)
	    printf("tx[%d]=%ld\n", j, tx[j]);
    }
#endif
    y[0] = tx[0];
#ifdef DEBUG
    if(debuglevel >= 10) printf("y[0]=%ld\n", y[0]);
#endif
    for(j = 1; j < k; j++){
	BnnSetToZero(zy+1, 0);
	BnnSetDigit(zy, y[j-1]);
	for(r = j-2; r >= 0; r--){
	    BnnSetDigit(w, y[r]);
	    BnnSetDigit(w+1, 0);
	    BnnMultiplyDigit(w, 2, zy, 1, tp[r]);
	    BnnSetDigit(zy, BnnDivideDigit(zy, w, 2, tp[j]));
	}
	y[j] = tx[j] - BnnGetDigit(zy);
	if(y[j] < 0) y[j] += tp[j];
	BnnSetDigit(zy, y[j]);
	BnnSetToZero(w, 2);
	BnnMultiplyDigit(w, 2, zy, 1, tc[j]);
	y[j] = BnnDivideDigit(zy, w, 2, tp[j]);
#ifdef DEBUG
	if(debuglevel >= 10) printf("y[%d]=%ld\n", j, y[j]);
#endif
    }
    xl = 1;
    BnnSetDigit(xx, y[k-1]);
    for(j = k-2; j >= 0; j--){
	/* z <- x * tp[j] + y [j] */
	zl = xl+1;
	BnnSetToZero(z, zl);
	BnnSetDigit(z, y[j]);
	BnnMultiplyDigit(z, zl, xx, xl, tp[j]);
	/* xx <-> z */
	tmp = xx; xx = z; z = tmp;
	xl = BnnNumDigits(xx, xl+1);
#ifdef DEBUG
	if(debuglevel >= 10)
	    {printf("x[%d]=", j); BnnPrint(xx, xl); printf("\n");}
#endif
    }
    BnnAssign(x, xx, xl);
    *p_xl = xl;
    BnFree(xx); BnFree(z); BnFree(w); BnFree(zy);
}

