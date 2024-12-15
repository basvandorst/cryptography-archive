
/* $Log:	testgcd.c,v $
 * Revision 1.1  91/09/20  14:48:29  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:50  morain
 * Initial revision
 *  */

#include "mod.h"
#include "gcd.h"

#define GLMAX 50

BigNum uu, vv, g, gp;

testgcd(u, ul, v, vl)
BigNum u, v;
int ul, vl;
{
    int gl, gpl;

    printf("U="); BnnPrint(u, ul); printf("\n");
    printf("V="); BnnPrint(v, vl); printf("\n");
    gl = BnnGcdLehmer(g, u, ul, v, vl);
    gpl = BnnGcd(gp, u, ul, v, vl);
    if(BnnCompare(g, gl, gp, gpl)){
	printf("Error\n");
	printf("gcd="); BnnPrint(g, gl); printf("\n");
	printf("gcd="); BnnPrint(gp, gpl); printf("\n");
	exit(0);
    }
}

#ifdef HYPERDEBUG
    {
	BigNum gp = BNC(nl+1), ap = BNC(nl+1);
	int gpl, apl;
	
	if(debuglevel >= 1) printf("%% Comparing EEA\n");
	BnnAssign(n, n0, nl);
	BnnAssign(m, m0, ml);
	BnEeaLehmer(gp, &gpl, ap, &apl, n, nl, m, ml);
	if(BnnCompare(ap, apl, a, *p_al)){
	    printf("Error in LehmerEea\n");
	    printf("plain ="); BnnPrint(a, *p_al); printf("\n");
	    printf("lehmer="); BnnPrint(ap, apl); printf("\n");
	    exit(0);
	}
	BnFree(gp); BnFree(ap);
    }
#endif

main(argc, argv)
int argc;
char *argv[];
{
    BigZ z;
    char s[300];
    BigNum u, v;
    FILE *fopen(), *ifile;
    int ul, vl, gl;

    if(argc == 1){
	fprintf(stderr, "Usage: %s file\n", argv[0]);
	exit(0);
    }
    ifile = fopen(argv[1], "r");
    uu = BNC(GLMAX); vv = BNC(GLMAX); g = BNC(GLMAX); gp = BNC(GLMAX);
    while(fscanf(ifile, "%s", s) != EOF){
        z = BzFromString(s, 10);
        u = BzToOldBigNum(z, &ul);
        BzFree(z);
	fscanf(ifile, "%s", s);
        z = BzFromString(s, 10);
        v = BzToOldBigNum(z, &vl);
        BzFree(z);
	testgcd(u, ul, v, vl);
	printf("##################\n");
        BnFree(u); BnFree(v);
    }
    fclose(ifile);
}
