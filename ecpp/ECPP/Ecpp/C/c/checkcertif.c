
/* $Log:	checkcertif.c,v $
 * Revision 1.1  91/09/20  14:52:57  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:21  morain
 * Initial revision
 *  */

/**********************************************************************

                        Checking certificates

**********************************************************************/

#include "ecpp.h"
#include "ecmodp.h"

char s[ECPPSTRMAX];
int cverbose = 0;

/* Reads a list of BigMod from a file */
BmReadFromFile(u, p_ul, file)
BigMod u;
int *p_ul;
FILE *file;
{
    ReadByBlockFromFile(s, file);
    ModSetToZero(u);
    ModFromString(u, s, 10);
    *p_ul = BnnNumDigits(u, Modul);
}

BigNum BnReadFromFile(file, p_nl)
FILE *file;
int *p_nl;
{
    BigZ z;
    BigNum n;

    ReadByBlockFromFile(s, file);
    z = BzFromString(s, 10);
    n = BzToOldBigNum(z, p_nl);
    BzFree(z);
    return(n);
}

ReadListOfFactors(p_lfact, ifile)
ListOfFactors *p_lfact;
FILE *ifile;
{
    ListOfFactors lfact;
    BigNum n;
    int nl;

    lfact = NULL;
    while(1){
	n = BnReadFromFile(ifile, &nl);
	if(BnnIsZero(n, nl)) break;
	if(lfact != NULL){
	    if(!BnnCompare(n, nl, lfact->p, lfact->pl)){
		/* this is not a new factor */
		lfact->e += 1;
		BnFree(n);
		continue;
	    }
	}
	/* this is a new factor */
	AppendFactor(&lfact, n, nl, 1);
	BnFree(n);
    }
    *p_lfact = lfact;
}

ReadListOfBigNum(p_la, ifile)
ListOfBigNum *p_la;
FILE *ifile;
{
    ListOfBigNum la, foo;
    BigNum n;
    int nl;

    la = NULL;
    while(1){
	n = BnReadFromFile(ifile, &nl);
	if(BnnIsZero(n, nl)) break;
	foo = ListOfBigNumAlloc();
	foo->n = n;
	foo->cdr = la;
	la = foo;
    }
    *p_la = la;
}

/********** N-1 test **********/

/* p - 1 = maxp * qk * ... q1, la = [amaxp, ak, ..., a1] */
int ReadCertifCminus1(p, pl, oldm, p_oldml, ifile)
BigNum p, oldm;
int pl, *p_oldml;
FILE *ifile;
{
    ListOfBigNum la;
    ListOfFactors lfacto;
    BigNum m, maxp;
    int h, ml, maxpl, ok;

    /* read theorem used */
    fscanf(ifile, "%d", &h);
    if(!h){
	/* Pratt stuff */
	ReadListOfBigNum(&la, ifile);
	return(CheckPratt(la));
    }
    else{
	/* m <- p-1 */
	m = BNC(pl+1);
	BnnAssign(m, p, pl);
	ml = pl;
	BnnSubtractBorrow(m, ml, 0);
	maxp = BNC(pl+1);
	BnnAssign(maxp, m, ml);
	maxpl = ml;
	ReadListOfFactors(&lfacto, ifile);
	/* recover maxp and append to lfacto */
	DivideMaxByList(maxp, &maxpl, &lfacto);
	ReadListOfBigNum(&la, ifile);
	if(BnnIsGtOne(maxp, maxpl)){
	    Append1Factor(&lfacto, maxp, maxpl, 1);
#ifdef DEBUG
	    if(debuglevel >= 2) PrintListOfFactors(lfacto);
#endif
	}
	switch(h){
	  case 1: 
	    ok = CheckTheorem1(la, p, pl, lfacto);
	    break;
	  case 3:
	    ok = CheckTheorem3(la->n, p, pl, maxp, maxpl);
	    break;
	  default:
	    printf("NYI\n");
	}
	BnnAssign(oldm, m, ml);
	*p_oldml = ml;
	BnFree(m); BnFree(maxp);
	ListOfFactorsFree(lfacto);
	return(ok);
    }
}

/********** N+1 test **********/
int ReadCertifCplus1(p, pl, oldm, p_oldml, ifile)
BigNum p, oldm;
int pl, *p_oldml;
FILE *ifile;
{
    ListOfBigNum lm;
    ListOfFactors lfacto;
    BigNumDigit a[1], u[1];
    BigNum m, maxp;
    int h, ml, maxpl, ok;

    a[0] = 0; u[0] = 0;
    /* read theorem used */
    fscanf(ifile, "%d", &h);
    /* m <- p+1 */
    m = BNC(pl+2);
    BnnAssign(m, p, pl);
    ml = pl+1;
    BnnAddCarry(m, ml, 1);
    ml = BnnNumDigits(m, ml);
    maxp = BNC(pl+2);
    BnnAssign(maxp, m, ml);
    maxpl = ml;
    ReadListOfFactors(&lfacto, ifile);
    /* recover maxp and append to lfacto */
    DivideMaxByList(maxp, &maxpl, &lfacto);
    /* lazy, no? */
    if((*p & 3) == 1)
	fscanf(ifile, "%ld", &a[0]);
    else{
	fscanf(ifile, "%ld", &u[0]);
	a[0] = 1;
    }
    ReadListOfBigNum(&lm, ifile);
    if(BnnIsGtOne(maxp, maxpl)){
	Append1Factor(&lfacto, maxp, maxpl, 1);
#ifdef DEBUG
	if(debuglevel >= 2) PrintListOfFactors(lfacto);
#endif
    }
    ok = CheckCLConditionIII(lm, p, pl, lfacto, u, a);
    BnnAssign(oldm, m, ml);
    *p_oldml = ml;
    BnFree(m); BnFree(maxp);
    ListOfFactorsFree(lfacto);
    return(ok);
}

/********** Ecm **********/
int ReadCertifCEcm(p, pl, oldm, p_oldml, ifile)
BigNum p, oldm;
int pl, *p_oldml;
FILE *ifile;
{
    ListOfFactors lfacto;
    Ecm E;
    Point P;
    BigNum m, n;
    int ml, nl, al, bl, xl, yl, ok;

    /* skip h */
    fscanf(ifile, "%*d");
    /* reading m */
    m = BnReadFromFile(ifile, &ml);
#ifdef DEBUG
    if(debuglevel >= 2){printf("%% m="); BnnPrint(m, ml); printf("\n");}
#endif
    /* skipping factors of m */
    n = NULL;
    do{
	if(n != NULL) BnFree(n);
	n = BnReadFromFile(ifile, &nl);
    } while(!BnnIsZero(n, nl));
    if(n != NULL){
	BnFree(n);
	n = NULL;
    }
    /* read E.a, E.b, P.x, P.y */
    E.a = BmCreate();
    E.b = BmCreate();
    P = PointCreate();
    BmReadFromFile(E.a, &al, ifile);
    BmReadFromFile(E.b, &bl, ifile);
    BmReadFromFile(P->x, &xl, ifile);
    BmReadFromFile(P->y, &yl, ifile);
#ifdef DEBUG
    if(debuglevel >= 2){
	printf("%% E="); EcmPrint(E); printf("\n");
	printf("%% P="); PointPrint(P); printf("\n");
    }
#endif
    lfacto = NULL;
    ReadListOfFactors(&lfacto, ifile);
#ifdef DEBUG
    if(debuglevel >= 2)
	{printf("%% Factors are\n"); PrintListOfFactors(lfacto);}
#endif
    EcmInit();
    ok = CheckEcmOrder(p, pl, m, ml, E, P, lfacto);
    EcmFree(E);
    PointFree(P);
    ListOfFactorsFree(lfacto);
    EcmClose();
    BnnAssign(oldm, m, ml);
    *p_oldml = ml;
    BnFree(m);
    return(ok);
}

int CheckEcmOrder(p, pl, m, ml, E, P, lfact)
BigNum p, m;
Ecm E;
Point P;
ListOfFactors lfact;
{
    ListOfFactors foo;
    Point Q = PointCreate();
    BigNum smin = BNC(ml+1), s = BNC(ml+1), tmp;
    int i, ok, sminl, sl;

#ifdef DEBUG
    if(debuglevel >= 2) {printf("%% Entering CheckEcmOrder\n");}
#endif
    EcmMult(Q, m, ml, P, E);
    if(!EcmIsZero(Q)){
	fprintf(stderr, "%%!%% Point not on the curve\n");
	return(0);
    }
    /* Build s */
    BnnSetDigit(s, 1);
    sl = 1;
    foo = lfact;
    while(foo != NULL){
	for(i = 1; i <= foo->e; i++){
	    sminl = sl + foo->pl;
	    BnnSetToZero(smin, sminl);
	    BnnMultiply(smin, sminl, s, sl, foo->p, foo->pl);
	    sminl = BnnNumDigits(smin, sminl);
	    tmp = s; s = smin; smin = tmp;
	    sl = sminl;
	}
	foo = foo->cdr;
    }
#ifdef DEBUG
    if(debuglevel >= 2){printf("%% s   ="); BnnPrint(s, sl); printf("\n");}
#endif
    /* Compute smin */
    BnnSetToZero(smin, sminl);
    EcmComputeBound(smin, &sminl, p, pl);
#ifdef DEBUG
    if(debuglevel >= 2)
	{printf("%% smin="); BnnPrint(smin, sminl); printf("\n");}
#endif
    if(BnnCompare(s, sl, smin, sminl) == -1){
	printf("%%!%% s too small\n");
	return(0);
    }
    foo = lfact;
    while(foo != NULL){
#ifdef DEBUG
	if(debuglevel >= 2){
	    printf("%% Working with f="); 
	    BnnPrint(foo->p, foo->pl); printf("\n");
	}
#endif
	/* smin <- s / foo->p */
	BnnAssign(smin, s, sl);
	BnnSetDigit((smin+sl), 0);
	sminl = sl+1;
	BnnDivide(smin, sminl, foo->p, foo->pl);
	sminl = BnnNumDigits((smin+foo->pl), sminl-foo->pl);
	EcmMult(Q, (smin+foo->pl), sminl, P, E);
	if(EcmIsZero(Q)){
	    printf("%%!%% Wrong order\n");
	    return(0);
	}
	else{
	    EcmMult(Q, foo->p, foo->pl, Q, E);
	    if(!EcmIsZero(Q)){
		printf("%%!%% Wrong order\n");
            return(0);
	    }
	}
	foo = foo->cdr;
    }
    return(1);
}

int ReadCertifC(ifile)
FILE *ifile;
{
    BigZ z;
    BigNum p, oldm;
    double tp, runtime();
    long D;
    int pl, ok, tok = 1, pos, oldml, ii = 0;
    char *occ, *strchr();

    oldm = NULL;
    while(fscanf(ifile, "%s", s) != EOF){
	tp = runtime();
	/* if s = 0$ */
	if((s[0] == '0') && (s[1] == '\0')){
	    /* end of certificate */
	    if(oldm != NULL){
		BnFree(oldm);
		oldm = NULL;
	    }
	    ii = 0;
	    printf("\n");
	    continue;
	}
	else{
	    if(s[0] == '%'){
		printf("%s\n", s);
		continue;
	    }
	}
	/* reading p; s may already contain a \ */
        if((occ = strchr(s, '\\')) != NULL){
	    pos = occ - s;
	    ReadByBlockFromFile(s+pos, ifile);
	}
	z = BzFromString(s, 10);
	p = BzToOldBigNum(z, &pl);
	BzFree(z);
#ifdef DEBUG
	if(debuglevel >= 2){printf("%% p="); BnnPrint(p, pl); printf("\n");}
#endif
	/* checking that p divides oldm */
	if(oldm != NULL){
	    BnnSetDigit((oldm+oldml), 0);
	    BnnDivide(oldm, oldml+1, p, pl);
	    if(!BnnIsZero(oldm, pl)){
		printf("%%!%% p does not divide oldm\n");
		printf("%%!%% oldm="); BnnPrint(oldm, oldml); printf("\n");
		printf("%%!%% p   ="); BnnPrint(p, pl); printf("\n");
		return(0);
	    }
	}
	else oldm = BNC(pl+2);
	BmInit(p);
	/* skipping D */
	fscanf(ifile, "%ld", &D);
	if(D == -1)
	    ok = ReadCertifCminus1(p, pl, oldm, &oldml, ifile);
	else{
	    if(D == 1)
		ok = ReadCertifCplus1(p, pl, oldm, &oldml, ifile);
	    else
		ok = ReadCertifCEcm(p, pl, oldm, &oldml, ifile);
	}
	printf("%3d: %6ld %d %10.2lfs\n", ii, D, ok, (runtime()-tp)/1000.);
	ii++;
	if(!ok) tok = 0;
	BnFree(p);
    }
    return(tok);
}

main(argc, argv)
int argc;
char *argv[];
{
    FILE *fopen(), *ifile;
    int i;
    char input_file[200];
    double tp, runtime();
    int c;
    extern char *optarg;
    extern int  optind, opterr;

    if(argc == 1){
      usage:
	fprintf(stderr, "Usage: %s file\n", argv[0]);
	fprintf(stderr, "-f          file\n");
	fprintf(stderr, "-q          quiet mode\n");
	fprintf(stderr, "-v          verbose mode\n");
#ifdef DEBUG
	fprintf(stderr, "-d#         sets debuglevel to #\n");
#endif
	exit(0);
    }
    else{
	opterr = 0;
	cverbose = 0;
#ifdef DEBUG
	while((c = getopt(argc, argv, "vqf:d:")) != EOF){
#else
	while((c = getopt(argc, argv, "vqf:")) != EOF){
#endif    
	    switch(c){
#ifdef DEBUG
	      case 'd': 
		sscanf(optarg, "%d", &debuglevel);
		printf("debuglevel=%d\n", debuglevel);
		break;
#endif
	      case 'f':
		sprintf(input_file, "%s", optarg);
		break;
	      case 'q':
		cverbose = 0;
		break;
	      case 'v':
		cverbose = 1;
		break;
	      case '?':
		fprintf(stderr, "Unknown option %s\n", argv[i]);
		goto usage;
	    }
	}
	if(!strlen(input_file))
	    goto usage;
    	ifile = fopen(input_file, "r");
	if(cverbose){
	    printf("%% Each line has the following shape:\n");
	    printf("%%\n");
	    printf("%% i: D 1/0 time\n");
	    printf("%% where i is the rank of the certificate,\n");
	    printf("%% D the discriminant\n");
	    printf("%% 1 if the certificate is true and 0 otherwise,\n");
	    printf("%% time is the time needed\n");
	}
	tp = runtime();
	ReadCertifC(ifile);
	printf("Total time is %10.2lfs\n", (runtime()-tp)/1000.);
    }
}
