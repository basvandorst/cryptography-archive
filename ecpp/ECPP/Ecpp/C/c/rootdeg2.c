
/* $Log:	rootdeg2.c,v $
 * Revision 1.1  91/09/20  14:55:18  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:08  morain
 * Initial revision
 *  */

/**********************************************************************
                       Solving degree 2 equations
**********************************************************************/

#include "mod.h"
#include "poly.h"

/* As usual, assumes that PX has two roots modulo (Modu, Modul) */
/* PX is monic */

/* Finds all roots of PX and puts them in tr[0..1]. If there are none, 
   returns 0, else 1.
*/
int AllRootsForDegree2(tr, PX)
BigNum tr[];
Poly PX;
{
    int root, rl;

    if(!OneRootForDegree2(tr[0], &rl, PX))
	return(0);
    else{
	ModAssign(tr[1], PX->coeff[1]);
	ModAdd(tr[1], tr[0]);
	ModNegate(tr[1]);
	return(1);
    }
}

/* Returns ONE solution or yields an error */
int OneRootForDegree2(r, p_rl, PX)
BigNum r;
int *p_rl;
Poly PX;
{
    int al, i;

#ifdef DEBUG
    if(debuglevel >= 4) printf("%% Entering rootdeg2\n");
#endif
    al = ModNumDigits(PX->coeff[1]);
    /* computing the discriminant */
    /* r <- a^2 */
    BmSquare(r, p_rl, PX->coeff[1], al);
    /* r <- r - 4*b */
    for(i = 0; i < 4; i++)
	BmSubtract(r, p_rl, PX->coeff[0], Modul);
#ifdef DEBUG
    if(debuglevel >= 8) 
	{printf("%% Disc="); BnnPrint(r, *p_rl); printf("\n");}
#endif
    if(BnJacobi(r, 0, *p_rl, Modu, 0, Modul) == -1){
	printf("Deg2: no root\n");
	return(0);
    }
    else{
	/* r <- sqrtmod(r, n) */
	if(!EcppMdSqrt(r, p_rl, r, *p_rl)){
	    printf("Discriminant has no squareroot\n");
	    return(0);
	}
	else{
	    /* one root: r <- r - a */
#ifdef DEBUG
	    if(debuglevel >= 8)
		{printf("%% Sqrt(disc)="); BnnPrint(r, *p_rl); printf("\n");}
#endif
	    BmSubtract(r, p_rl, PX->coeff[1], al);
#ifdef DEBUG
	    if(debuglevel >= 8)
		{printf("%% r-a="); BnnPrint(r, *p_rl); printf("\n");}
#endif
	    /* r <- r / 2 mod Modu */
	    *p_rl = BmShiftRight(r, *p_rl, 1);
#ifdef DEBUG
	    if(debuglevel >= 8)
		{printf("%% root="); BnnPrint(r, *p_rl); printf("\n");}
#endif
	    return(1);
	}
    }
}


