
/* $Log:	bntobnn.c,v $
 * Revision 1.1  91/09/20  14:47:43  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:03  morain
 * Initial revision
 *  */

		/* old functions of Bn */
#include "BigNum.h"
#include "bntobnn.h"

/*
 *	Creation and access to type and length fields.
 */
extern char *malloc();
/* Allocates a BigNum structure and returns a pointer to it */
BigNum BnAlloc(size) int size; {
	register BigNum n;
 
	n = (BigNum) (malloc(sizeof(struct BigNumHeader) +
				size * sizeof(BigNumDigit))
			+ sizeof(struct BigNumHeader));
	BN_LENGTH(n) = size;
	return(n);
}
 
/* Allocates a BigNum, inserts its Type, and returns a pointer to it */
BigNum BnCreate(type, size) BigNumType type; int size; {
	register BigNum n;
 
	n = BnAlloc(size);
	BN_TYPE(n) = type;
	BnSetToZero(n, 0, size);
	return(n);
}
 
/* Frees a BigNum structure */
int BnFree(n) BigNum n; {
	free(((struct BigNumHeader *) n) - 1);
        return 1; 
}
 
/* Returns the BigNum's Type */
BigNumType BnGetType(n) BigNum n; {
        return(BN_TYPE(n));
}
 
/* Sets the BigNum's Type */
void BnSetType(n, type) BigNum n; BigNumType type; {
        BN_TYPE(n) = type;
}
 
/* Returns the number of digits allocated for the BigNum */
int BnGetSize(n) BigNum n; {
	return(BN_LENGTH(n));
}
 

