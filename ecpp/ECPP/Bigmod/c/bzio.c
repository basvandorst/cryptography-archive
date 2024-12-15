
/* $Log:	bzio.c,v $
 * Revision 1.1  91/09/20  14:47:46  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:07  morain
 * Initial revision
 *  */

/**********************************************************************
          Some I/O routines for reading/writing bignums
**********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "BigNum.h"
#include "BigZ.h"

#include "bntobnn.h"
#include "bzio.h"

#ifdef pyramid
#include <strings.h>
#else
#include <string.h>
#endif

char bzstrbuf[BZIOSTRMAX];

/********** Some I/O **********/

/* Creates a string representing (n, nl) */
char *BnnToString(n, nl, base)
BigNum n;
int nl;
BigNumDigit base;
{
    BigZ z;
    char *s;

    z = BzFromBigNum(n, nl);
    s = BzToString(z, base);
    BzFree(z);
    return(s);
}

BzPrint(z)
BigZ z;
{
    char *s;

    s = BzToString(z, 10);
    printf("%s", s);
    free(s);
}

PrintBzToFile(ofile, z)
FILE *ofile;
BigZ z;
{
    char *s = BzToString(z, 10);

    fprintf(ofile, "%s", s);
    free(s);
}

/* Prints a bignum without too much extra space (>> From) */
BnnPrint(n, nl)
BigNum n;
int nl;
{
    char *s, *BnnToString();

    s = BnnToString(n, nl, 10);
    printf("%s", s);
    free(s);
}

BnnPrintToFile(file, n, nl, base)
FILE *file;
BigNum n;
int nl;
BigNumDigit base;
{
    char *s;

    s = BnnToString(n, nl, base);
    fprintf(file, "%s", s);
    free(s);
}

/* Printing (n, nl) by blocks of MAXCOL characters followed by a \ */
BnnPrintByBlockToFile(ofile, n, nl, base, maxcol, bol, eol)
FILE *ofile;
BigNum n;
int nl, maxcol;
BigNumDigit base;
char bol[], eol[];
{
    BigZ z;
    char *s, *t;
    int col, sl, q, r;
    
    z = BzFromBigNum(n, nl);
    s = BzToString(z, base);
    if(!maxcol)	fprintf(ofile, "%s%s%s", bol, s, eol);
    else{
	t = s;
	col = 0;
	fprintf(ofile, "%s", bol);
	while(*t != '\0'){
	    fprintf(ofile, "%c", *t);
	    t++;
	    col++;
	    if(col == maxcol){
		col = 0;
		if(*t != '\0')
		    fprintf(ofile, "%s\n%s", eol, bol);
	    }
	}
    }
    BzFree(z); free(s);
}

/* Reading by blocks of MAXCOL characters followed by a \ */
char *StrReadByBlockFromFile(ifile, base)
FILE *ifile;
BigNumDigit base;
{
    char *s, t[MAXCOL+1], *occ, *strchr();
    int pos = 0, sd = 0;

    s = (char *)malloc(BZIOSTRMAX * sizeof(char));
    while(1){
	fscanf(ifile, "%s", t);
	occ = strchr(t, '\\');
	if(occ != NULL){
	    pos = occ - t;
	    strncpy(s+sd, t, pos);
	    sd += pos;
	}
	else break;
    }
    if(pos = strlen(t)){
	strncpy(s+sd, t, pos);
	sd += pos;
    }
    s[sd] = '\0';
    return(s);
}

/* Reading by blocks of MAXCOL characters followed by a \ */
int ReadByBlockFromFile(s, ifile)
char *s;
FILE *ifile;
{
    char t[MAXCOL+2], *occ, *strchr();
    int pos = 0, sd = 0;

    while(1){
	fscanf(ifile, "%s", t);
	occ = strchr(t, '\\');
	if(occ != NULL){
	    pos = occ - t;
	    strncpy(s+sd, t, pos);
	    sd += pos;
	}
	else break;
    }
    if(pos = strlen(t)){
	strncpy(s+sd, t, pos);
	sd += pos;
    }
    *(s+sd) = '\0';
    return(sd);
}
