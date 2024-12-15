
/* $Log:	pratt.c,v $
 * Revision 1.1  91/09/20  14:55:00  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:45  morain
 * Initial revision
 *  */

/**********************************************************************

                                    PRATT

**********************************************************************/

/* The data structure chosen is quite odd. It is an array in which the
   corresponding tree is "aplati". The elements are thus:
    [p, g, # of prime factors of p-1, []]
*/

#include "ecpp.h"

/* tpratt should be of sufficient length */
int PrattBuild(tpratt, p_i, p)
long tpratt[], p;
int *p_i;
{
    int ok;

    *p_i = 0;
    ok = PrattFill(tpratt, p, p_i);
    tpratt[*p_i] = 0;
    return(ok);
}

int PrattFill(tpratt, p, p_i)
long tpratt[], p;
int *p_i;
{
    long g, q, tn[5];
    int j, nf;

    tpratt[(*p_i)++] = p;
    if(p == 2)
	tpratt[(*p_i)++] = 1;
    else{
	/* find factors of p-1 */
	SmallIfactor(p-1, tn, &nf);
	/* find primitive root mod p */
	if(!SmallFindPrimitiveRoot(&g, p, tn, nf)){
	    printf("%%!%% Error, not a prime\n");
	    return(0);
	}
	else{
	    tpratt[(*p_i)++] = g;
	    tpratt[(*p_i)++] = nf;
	    for(j = 0; j < nf; j++){
		if(!PrattFill(tpratt, tn[j], p_i))
		    return(0);
	    }
	}
    }
    return(1);
}

PrattToList(p_cert, smallp, tpratt, ipratt)
Certif *p_cert;
BigNum smallp;
long tpratt[];
int ipratt;
{
    BigNumDigit n[1];
    int i;

    p_cert->p = smallp;
    p_cert->pl = 1;
    p_cert->D = -1;
    p_cert->h = 0;
    p_cert->lfacto = NULL;
    for(i = ipratt-1; i >= 0; i--){
	BnnSetDigit(n, tpratt[i]);
	AppendFactor(&(p_cert->lfacto), n, 1, 1);
    }
}
