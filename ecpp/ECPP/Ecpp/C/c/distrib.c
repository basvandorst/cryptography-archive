
/* $Log:	distrib.c,v $
 * Revision 1.1  91/09/20  14:53:32  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:05  morain
 * Initial revision
 *  */

#include "ecpp.h"
#include "distrib.h"

/* distname:    contains the name of the number to be tested
   distrunning: contains N_i (the i-th number) and i itself
   distnumber:  contains i (the depth)
   distrgdfd:   contains the current rank number 
   distspeedup: contains the successful info to be given to the master
*/

char distname[200], distspeedup[200], distrunning[200], distnumber[200];
char distrgdfd[200], diststop[200];

int distecpp, distmaster;


BigNum ReadLelispBigNum(p_nl, s)
int *p_nl;
char s[];
{
    BigZ z;
    BigNum n;

    if(s[0] == '#'){
	char t[STRMAX];

	sscanf(s, "#{%[^}]", t);
#ifdef DEBUG
	if(debuglevel >= 8){
	    printf("%% String read=%s\n", t);
	}
#endif
	z = BzFromString(t, 10);
    }
    else z = BzFromString(s, 10);
    n = BzToOldBigNum(z, p_nl);
    BzFree(z);
    return(n);
}

DistInit()
{
    char h[200];
    int hl;

#ifdef DEBUG
    printf("%% distname=%s\n", distname);
#endif
    sprintf(distrunning, "%s%s", distname, RUNNING);
    sprintf(distnumber, "%s%s", distname, NUMBER);
    sprintf(distrgdfd, "%s%s", distname, RGDFD);
    /* Building stop.mach */
    sprintf(diststop, "%s", distname);
    for(hl = strlen(diststop)-1 ; diststop[hl] != '/'; hl--);
    hl++;
    gethostname(h, 200);
    sprintf(diststop+hl, "stop.%s", h);
#ifdef DEBUG
    printf("%% diststop=%s\n", diststop);
#endif
    sprintf(bol, "#{");
    sprintf(eol, "}");
}

DistGo()
{
    FILE *fopen(), *ifile;
    BigNum n;
    char s[STRMAX];
    int nl, olddepth = -1;

    DistInit();
    do {
	/* read number */
	if((ifile = fopen(distrunning, "r")) == NULL){
	    printf("%% Cannot open %s\n", distrunning);
	    sleep(10);
	}
	else{
	    fscanf(ifile, "%s", s);
	    n = ReadLelispBigNum(&nl, s);
	    /* read step */
	    fscanf(ifile, "%d", &depth);
	    fclose(ifile);
	    if(depth <= olddepth){
		printf("%% Still at depth %d\n", olddepth);
	    }
	    else{
		printf("\n%% Step=%d\n", depth);
#ifdef DEBUG
		printf("%% N_%d=", depth); BnnPrint(n, nl); printf("\n");
#endif
		ecppinit(n, nl);
		DistBuildchain(n, nl);
		olddepth = depth;
	    }
	    BnFree(n);
	    printf("%% Sleeping...\n");
	    sleep(300);
	}
    } while (nl >= 10);
    printf("%% Job finished\n");
    ifile = fopen(diststop, "w");
    fprintf(ifile, "end\n");
    fclose(ifile);
}

/* First phase of the program, building a decreasing sequence of primes */
int DistBuildchain(n0, n0l)
BigNum n0;
int n0l;
{
    Certif cert;
    BigNum n = BNC(n0l);
    int nl = n0l, done, gasp;
    double tp, runtime();
    char msg[200];

    BnnAssign(n, n0, n0l);
    SetSievepmax(nl);
    presieve(n, nl, difnp, Sievepmax, resp, NBNP);
    gasp = 0;
    /* backtracking should occur somewhere near here... */
    while(!(done = findm(n, nl, &(cert), gasp))){
	/* there is a pb */
	if(gasp)
	    return(0);
	else{
	    /* this is the first time at this depth */
	    printf("%%!%% Forced to retry, snif...\n");
	    gasp = 1;
	}
    }
    if(done == -1){
	/* someone else did it */
#ifdef DEBUG
	printf("%% Stopping job for that depth\n");
#endif
    }
    else{
	/* this program did it */
	cert.p = BNC(nl);
	BnnAssign(cert.p, n, nl);
	cert.pl = nl;
	DistOutputToLelispFile(cert);
	sprintf(msg, "mail -s \"ECPP-C: %d/%d\" morain@fleurie < /dev/null",
		cert.D, cert.h);
	system(msg);
    }
    BnFree(n);
    return(1);
}

int DistGetRgd()
{
    int rk;

    if((rk = DistReadNextD()) == -1){
	printf("%%!%% Gasp, could not get next rank\n");
    }
#ifdef DEBUG
    printf("%% Next rank is %d\n", rk);
#endif
    DistPutNextD(rk+1);
    return(rk);
}

int DistReadNextD()
{
    FILE *fopen(), *ifile;
    int i, rk;

    rk = -1;
    for(i = 0; i < 5; i++){
	if((ifile = fopen(distrgdfd, "r")) == NULL){
	    printf("%%!%% Cannot open %s\n", distrgdfd);
	}
	else{
	    if(fscanf(ifile, "%d", &rk) == EOF){
		printf("%%!%% Error while reading in %s\n", distrgdfd);
	    }
	    else break;
	}
	sleep(5);
    }
    if(ifile != NULL) fclose(ifile);
    return(rk);
}

int DistPutNextD(rk)
int rk;
{
    FILE *fopen(), *ofile;
    int i;

    for(i = 0; i < 5; i++){
	if((ofile = fopen(distrgdfd, "w")) == NULL){
	    printf("%%!%% Cannot open %s\n", distrgdfd);
	}
	else{
	    if(fprintf(ofile, "%d\n", rk) == EOF){
		printf("%%!%% Error while printing in %s\n", distrgdfd);
	    }
	    else break;
	}
	sleep(5);
    }
    if(ofile != NULL) fclose(ofile);
    return(rk);
}

/********** Getting new events **********/

int DistAnythingNew()
{
    int newdepth = DistGetDepth();

    return(newdepth > depth);
}

int DistGetDepth()
{
    FILE *fopen(), *ifile;
    int i, newdepth;

    for(i = 0; i < 5; i++){
	if((ifile = fopen(distnumber, "r")) == NULL){
	    printf("%%!%% Cannot open %s\n", distnumber);
	}
	else{
	    if(fscanf(ifile, "%d", &newdepth) == EOF){
		printf("%%!%% Error while reading %s\n", distnumber);
	    }
	    else break;
	}
	sleep(5);
    }
    if(ifile != NULL) fclose(ifile);
    return(newdepth);
}

/********** Sordid hacks for LeLisp **********/

#define TOFILE(ofile, x, xl) \
{   fprintf((ofile), "%s", bobn); \
    BnnPrintByBlockToFile((ofile), (x), (xl), obase, 0, bol, eol); \
    fprintf((ofile), "%s", eobn);}

DistOutputToLelispFile(cert)
Certif cert;
{
    FILE *fopen(), *ofile;
    ListOfFactors lfact;
    char hostname[50];
    int g;

    sprintf(distspeedup, "%s%s%d", distname, SPEEDUP, depth);
    ofile = fopen(distspeedup, "w");
    /* (n, nl) */
    TOFILE(ofile, cert.p, cert.pl);
    fprintf(ofile, "\n");
    /* rank of the discriminant */
    fprintf(ofile, "%d\n", rgd[depth]);
    /* the discriminant */
    fprintf(ofile, "%d\n", cert.D);
    /* lm: the factors of m, etc. */
    fprintf(ofile, "(");
    TOFILE(ofile, cert.m, cert.ml);
    fprintf(ofile, " t ");
    /*  the cofactor of m */
    fprintf(ofile, "(");
    TOFILE(ofile, Modu, Modul);
    fprintf(ofile, " . 1)");
    /*  the factors of m */
    lfact = cert.lfact;
    while(lfact != NULL){
	fprintf(ofile, " (");
	TOFILE(ofile, lfact->p, lfact->pl);
	fprintf(ofile, " . %d)", lfact->e);
	lfact = lfact->cdr;
    }
    fprintf(ofile, ")\n");
    /* list of quadint... */
    fprintf(ofile, "(#:quadint:#[");
    PrintBzToFile(ofile, cert.A);
    fprintf(ofile, " ");
    PrintBzToFile(ofile, cert.B);
    fprintf(ofile, "])\n");
    /* name of the host */
    gethostname(hostname, 50);
    fprintf(ofile, "%s\n", hostname);
    /* method of factorisation */
    fprintf(ofile, "C\n");
    /* fD */
    fprintf(ofile, "(");
    for(g = cert.g; g >= 1; g >>= 1)
	fprintf(ofile, "%d ", 1);
    fprintf(ofile, ")\n");
    /* h(-D) */
    fprintf(ofile, "%d\n", cert.h);
    fclose(ofile);
}

#if 0
(de OutputToSpeedupFile ()
    (with ((outchan (openo :speedup)))
	; sauvegarde
	  (Bprint :N)
	  (print (vref :rangD :prof))
	  (print (vref :D :prof))
	  (Bprint (vref :lm :prof))
	; :mpi[:prof] is a list of quadint's
	  (Bprint (vref :mpi :prof))
	  (print :hostname)
	  (print #:mod:method)
	  (print (vref :fD :prof))
	  (print (vref :H :prof))
	  (close (outchan))))
#endif
