
/* $Log:	ecppio.c,v $
 * Revision 1.1  91/09/20  14:53:43  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:17  morain
 * Initial revision
 *  */

/**********************************************************************
                          I/O routines for ecpp
**********************************************************************/

#include "ecpp.h"

/* Complete name */
BuildFileName(nameo, namei)
char nameo[], namei[];
{
    sprintf(nameo, "%s%s", path, namei);
}

/* A data in file_of_fD is ((q1 ... qt) D h ...) */
/* Returns 0 in case of EOF */
int ExtractDfD(p_D, fD, p_h, p_g, file_of_DfD)
long *p_D;
long fD[];
int *p_h, *p_g;
FILE *file_of_DfD;
{
    int c;

    if(ExtractDfDh(p_D, fD, p_h, p_g, file_of_DfD)){
	while((c = getc(file_of_DfD)) != '\n');
	return(1);
    }
    else return(0);
}

/* Same as above, but doesn't try to read the end of the line: This is useful 
   for reading the invariant code and genus number in findj */
int ExtractDfDh(p_D, fD, p_h, p_g, file_of_DfD)
long *p_D;
long fD[];
int *p_h, *p_g;
FILE *file_of_DfD;
{
    long l;
    int g, i, c;

    if(fscanf(file_of_DfD, "((%ld", &l) == EOF)
	return(0);
    i = 0; g = 1;
    while((c = getc(file_of_DfD)) != EOF){
	if(c == ')')
	    break;
	else{
	    /* works even for the first case of q_1 ! */
	    if(c != ' ')
		l = l*10+(c-'0');
	    else{
		fD[++i] = l;
		l = 0; 
		g <<= 1;
	    }
	}
    }
    if(c == EOF)
	return(0);
    else{
	if(l){
	    fD[++i] = l;
	    g <<= 1;
	}
	*p_g = g >> 1;
	fD[0] = i;
	if(fscanf(file_of_DfD, "%ld %d", p_D, p_h) != EOF){
#ifdef DEBUG
	    if(debuglevel >= 10)
		printf("Value of D extracted is %ld\n", *p_D);
#endif
	    return(1);
	}
	else return(0);
    }
}



