
/* $Log:	bztobzz.c,v $
 * Revision 1.1  91/09/20  14:47:50  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:10  morain
 * Initial revision
 *  */

#include "BigZ.h"
#include "bntobnn.h"

#define NULL 0

/* Returns an old BigNum in place of a new one */
BigNum BzToOldBigNum (z, nl)
BigZ         z;
BigNumLength *nl;
{
    BigNum n, m;
    int i;

    if (BzGetSign (z) == BZ_MINUS)
        return NULL;
    *nl = BzNumDigits (z);
    /* [BNHeader | n[0] ... n[*nl-1]] */
    /*           n-^ */
    n = (BigNum) (malloc(sizeof(struct BigNumHeader) +
			 (*nl) * sizeof(BigNumDigit))
		  + sizeof(struct BigNumHeader));
    if (n != NULL)
    {
	BN_LENGTH(n) = *nl; /* set size */
        for (i = 0, m = n; i < *nl; i++, m++)
            *m = z->Digits[i];
    }
    return n;
}
