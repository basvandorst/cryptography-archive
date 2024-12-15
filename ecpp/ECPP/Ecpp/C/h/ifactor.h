
/* $Log:	ifactor.h,v $
 * Revision 1.1  91/09/20  15:01:08  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:33:06  morain
 * Initial revision
 *  */

#include "mod.h"

extern int IsGcdTrivial();

/* a list containing the factorization of a BigNum */
struct list_of_factors{
	BigNum p;
	int pl;
	int e;
	struct list_of_factors *cdr;
       };

typedef struct list_of_factors FCELL;
typedef FCELL *ListOfFactors;

#define ListOfFactorsAlloc() ((ListOfFactors)malloc(sizeof(FCELL)))

/* Lists of BigNum */
struct listn{
    BigNum n;
    struct listn *cdr;
};

typedef struct listn LLN;
typedef LLN *ListOfBigNum;
#define ListOfBigNumAlloc() ((ListOfBigNum)malloc(sizeof(LLN)))

extern FreeListOfFactors();
