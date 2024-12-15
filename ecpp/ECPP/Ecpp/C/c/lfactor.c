
/* $Log:	lfactor.c,v $
 * Revision 1.1  91/09/20  14:54:30  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:29:09  morain
 * Initial revision
 *  */

#include "ifactor.h"

/* appends (f, fl)^e to lfact */
AppendFactor(p_lfact, f, fl, e)
ListOfFactors *p_lfact;
BigNum f;
int fl, e;
{
    ListOfFactors lf = ListOfFactorsAlloc();

    lf->p = BNC(fl);
    lf->pl = fl;
    BnnAssign(lf->p, f, fl);
    lf->e = e;
    lf->cdr = *p_lfact;
    *p_lfact = lf;
}

Append1Factor(p_lfact, p, pl, e)
ListOfFactors *p_lfact;
BigNum p;
int pl, e;
{
    ListOfFactors bar, foo, tmp;

    foo = ListOfFactorsAlloc();
    foo->p = BNC(pl); BnnAssign(foo->p, p, pl);
    foo->pl = pl; foo->e = 1; foo->cdr = NULL;
    tmp = *p_lfact;
    bar = NULL;
    while(tmp != NULL){
	bar = tmp;
	tmp = tmp->cdr;
    }
    if(bar == NULL) *p_lfact = foo;
    else bar->cdr = foo;
}

PrintListOfFactors(lfact)
ListOfFactors lfact;
{
    ListOfFactors foo = lfact;

    while(foo != NULL){
	printf("%% Factor= "); BnnPrint(foo->p, foo->pl);
	printf("^%d\n", foo->e);
	foo = foo->cdr;
    }
}

ListOfFactorsFree(lfact)
ListOfFactors lfact;
{
    ListOfFactors tmp, bar;

    tmp = lfact;
    /* Freeing all bignums */
    while(tmp != NULL){
	bar = tmp;
	tmp = tmp->cdr;
	BnFree(bar->p);
	bar->cdr = NULL;
	free((ListOfFactors *)bar);
    }
}
