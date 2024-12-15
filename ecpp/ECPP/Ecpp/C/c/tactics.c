
/* $Log:	tactics.c,v $
 * Revision 1.1  91/09/20  14:55:36  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:39  morain
 * Initial revision
 *  */

/**********************************************************************
               All kinds of choices of parameters
**********************************************************************/

#include "ecpp.h"

/* when gasp is true, there is something to be done... */
IfactorMethods(p_userho, p_usepollard, gasp)
int *p_userho, *p_usepollard, gasp;
{
    if(!gasp && (Modul <= 10)){
	*p_userho = 0;
	*p_usepollard = 0;
    }
    else{
	*p_userho = 1;
	*p_usepollard = 1;
    }
}

SetSievepmax(nl)
int nl;
{
    FILE *fopen(), *ifile;
    long oldspmax = 0, spmax = 0;
    char name[200];
    int l;

    if(Sievepmax == NULL) Sievepmax = BNC(1);
    BuildFileName(name, FSPMAX);
    ifile = fopen(name, "r");
    while(fscanf(ifile, "%d %ld %*ld %*d %*ld %*ld", &l, &spmax) != EOF){
	if(nl < l) break;
    }
    fclose(ifile);
    if(oldspmax) spmax = oldspmax;
    BnnSetDigit(Sievepmax, (BigNumDigit)spmax);
    if(verbose) printf("%% Pmax=%ld\n", *Sievepmax);
}

SetFactorParameters(p_itmax, p_ngcd, p_b1, p_b2, nl)
long *p_itmax, *p_b1, *p_b2;
int *p_ngcd;
{
    FILE *fopen(), *ifile;
    char name[200];
    int l;

    BuildFileName(name, FSPMAX);
    ifile = fopen(name, "r");
    while(fscanf(ifile, "%d %*ld %ld %d %ld %ld", &l, 
		 p_itmax, p_ngcd, p_b1, p_b2) != EOF){
	if(nl < l) break;
    }
    fclose(ifile);
    if(verbose)
	printf("%% itmax=%ld ngcd=%d b1=%ld b2=%ld\n", *p_itmax, *p_ngcd,
	       *p_b1, *p_b2);
}

