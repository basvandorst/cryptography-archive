
/* $Log:	nextprime.c,v $
 * Revision 1.1  91/09/20  14:48:18  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:38  morain
 * Initial revision
 *  */

/**********************************************************************
              Finding primes with various properties
**********************************************************************/

#include "mod.h"

/* Same, but with p large. This time p is a probable prime. */
int nextprprime(p, p_pl, difnp_file)
BigNum p;
int *p_pl;
char difnp_file[];
{
    long why;
    int pl = 1 + *p_pl;

    if(!BnnIsDigitOdd(*p))
	BnnAddCarry(p, pl, 1);
    while(1){
	/* p <- p+2 */
	BnnAdd(p, pl, (CONSTANTES+2), 1, 0);
	if(BnnIsDigitZero(*(p+pl-1))) pl--;
	if(IsProbablePrimeWithSieve(p, pl, difnp_file, 10000, &why))
	    break;
    }
    *p_pl = pl;
#ifdef DEBUG
    if(debuglevel >= 10){
	printf("Next prime is ");
	BnnPrint(p, pl);
	printf("\n");
    }
#endif
}
