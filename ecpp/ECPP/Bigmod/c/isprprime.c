
/* $Log:	isprprime.c,v $
 * Revision 1.1  91/09/20  14:48:04  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:26  morain
 * Initial revision
 *  */

#include "mod.h"

#define STRMAX 1000

main(argc, argv)
int argc;
char *argv[];
{
    FILE *fopen(), *ifile;
    BigZ z;
    BigNum n;
    int nl;
    char s[STRMAX];
    double tp, runtime();

    if(argc == 1){
	printf("Enter number to test for pseudoprimality:\n");
	scanf("%s", s);
	z = BzFromString(s, 10);
	n = BzToOldBigNum(z, &nl);
	tp = runtime();
	BmInit(n);
	if(IsProbablePrime(n, nl))
	    printf("Time for proving the pseudoprimality");
	else
	    printf("Time for proving the compositness   ");
	printf(" of N is %lf s\n", (runtime()-tp)/1000);
    }
    else{
	ifile = fopen(argv[1], "r");
	if(argc == 3){
	    sscanf(argv[2], "%d", &debuglevel);
	    printf("debuglvel=%d\n", debuglevel);
	}
	while(fscanf(ifile, "%s", s) != EOF){
	    z = BzFromString(s, 10);
	    n = BzToOldBigNum(z, &nl);
	    printf("Working on %s\n", From(n, nl));
	    tp = runtime();
	    BmInit(n);
	    if(IsProbablePrime(n, nl))
		printf("Time for proving the pseudoprimality");
	    else
		printf("Time for proving the compositness   ");
	    printf(" of N is %lf s\n", (runtime()-tp)/1000);
	}
	fclose(ifile);
    }
}
