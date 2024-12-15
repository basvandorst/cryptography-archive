
/* $Log:	ecpp.c,v $
 * Revision 1.1  91/09/20  14:53:40  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:28:12  morain
 * Initial revision
 *  */

/**********************************************************************
                                 ECPP
**********************************************************************/

#include "ecpp.h"

/* some useful flags */
int verbose = 1, noproof = 0, ibase = 10, obase = 10, cyclic = 0, jumpj = 0;
int nonminus1 = 0, nonplus1 = 0;
char difnp[200], dfd[200];

/* Returns 1 if n is prime and 0 otherwise */
int ecpp(tabcertif, p_ncert, n, nl)
Certif tabcertif[];
BigNum n;
int nl, *p_ncert;
{
    BigNum smallp = BNC(1);
    int ok;
    double runtime(), tp, tot;

    if(verbose){
	printf("%% Entering ECPP\n");
	printf("%% Starting phase 1: building the sequence of primes\n");
    }
    tp = runtime();
    ecppinit(n, nl);
    if(!buildchain(tabcertif, n, nl, smallp)){
	depth--;
	ecppclose();
	return(0);
    }
    tot = runtime()-tp;
    if(verbose){
	printf("\n%% Time for building is %f s\n\n", tot/1000.);
	printf("%% Starting phase 2: proving\n");
    }
    tp = runtime();
    if(noproof)
	ok = 1;
    else{
	ok = proveall(tabcertif, n, nl, smallp);
	if(verbose){
	    printf("%% Time for building is %lf s\n", tot/1000.);
	    printf("%% Time for proving  is %lf s\n", (runtime()-tp)/1000.);
	    tot += (runtime()-tp);
	    printf("%% Total time is        %lf s\n", tot/1000.);
	}
    }
    ecppclose();
    *p_ncert = depth;
    return(ok);
}

int ecppinit(n, nl)
BigNum n;
int nl;
{
    pmin = BNC(1);
    BnnSetDigit(pmin, 100);
    pminl = 1;
    sprintf(difnp, "%s%s", path, DIFNP);
    sprintf(dfd, "%s%s", path, NAMEDFD);
}

/* Cleaning all kind of junk */
ecppclose()
{
    int i;

    rgd0 = 0;
    for(i = 0; i <= depth; i++){
	if(tshanksz[i] != NULL){
	    BnFree(tshanksz[i]);
	    tshanksz[i] = NULL;
	}
    }
    BnFree(pmin);
}

/* First phase of the program, building a decreasing sequence of primes */
int buildchain(tabcertif, n0, n0l, smallp)
Certif tabcertif[];
BigNum n0, smallp;
int n0l;
{
    BigNum n = BNC(n0l);
    int nl = n0l, done, gasp;
    double tp, runtime();

    BnnAssign(n, n0, n0l);
    depth = -1;
    while(BnnCompare(n, nl, pmin, pminl) == 1){
	depth++;
	SetSievepmax(nl);
	presieve(n, nl, difnp, Sievepmax, resp, NBNP);
	if(verbose)
	    {printf("%% N_%d=", depth); BnnPrint(n, nl); printf("\n");}
	gasp = 0;
	/* backtracking should occur somewhere near here... */
	while(!(done = findm(n, nl, &(tabcertif[depth]), gasp))){
	    /* there is a pb */
	    if(gasp)
		return(0);
	    else{
		/* this is the first time at this depth */
		printf("%%!%% Forced to retry, snif...\n");
		gasp = 1;
	    }
	}
	tabcertif[depth].p = BNC(nl);
	BnnAssign(tabcertif[depth].p, n, nl);
	tabcertif[depth].pl = nl;
	if(verbose){
	    printf("%% D[[%d]]=%ld\n", depth, tabcertif[depth].D);
	    if(tabcertif[depth].D > 1){
		printf("%% A[[%d]]=", depth); 
		BzPrint(tabcertif[depth].A); printf("\n");
		printf("%% B[[%d]]=", depth);
		BzPrint(tabcertif[depth].B); printf("\n");
		printf("%% m[[%d]]=", depth);
		BnnPrint(tabcertif[depth].m,tabcertif[depth].ml); 
		printf("\n");
	    }
	    PrintListOfFactors(tabcertif[depth].lfact);
	}
#if 0
	if(done == 1){
	    if(verbose) printf("%% Cofactor is 1\n");
	    BnnAssign(smallp, (tabcertif[depth].lfact)->p, 1);
	    break;
	}
#else
	if(done == 1){
	    if(verbose) printf("%% Cofactor is 1\n");
	    BnnAssign(smallp, (tabcertif[depth].lfact)->p, 1);
	    if(BnnCompare(smallp, 1, pmin, 1) == 1){
		BnnSetToZero(n, nl);
		BnnAssign(n, smallp, 1);
		nl = 1;
	    }
	    else break;
	}
#endif
	else{
	    /* cofactor is in (Modu, Modul) */
	    BnnSetToZero(n, nl);
	    BnnAssign(n, Modu, Modul);
	    nl = Modul;
	}
	if(verbose)
	    printf("%% End of depth %d at %lf s\n\n", depth, runtime()/1000.);
	if(jumpj) break;
    }
    if(depth < 0)
	BnnAssign(smallp, n0, n0l);
    BnFree(n);
    return(1);
}

/* Second phase, proving all the Theorems */
int proveall(tabcertif, n, nl, smallp)
Certif tabcertif[];
BigNum n, smallp;
int nl;
{
    BigNum maxp;
    long tpratt[PRATTMAX];
    int depthmax, ok = 1, maxpl, ipratt;
    double tp, runtime();

    depthmax = depth;
    /* just a little dirty... */
    for(depth = 0; depth <= depthmax; depth++){
	if(verbose) printf("%% Starting proving job for step %d\n", depth);
	tp = runtime();
	if(depth < depthmax){
	    maxp = tabcertif[depth+1].p;
	    maxpl = tabcertif[depth+1].pl;
	}
	else maxp = NULL;
	if((ok = prove(&tabcertif[depth], maxp, maxpl))){
	    if(verbose) printf("%% N_%d is prime\n", depth);
	}
	else{
	    if(verbose) printf("%% N_%d is not prime\n", depth);
	}
	if(verbose){
	    tp = (runtime()-tp)/1000;
	    printf("%% Time for proof[%d] is %lf s\n",depth,tp);
	    if(depth < depthmax) printf("\n");
	}
	if(!ok)
	    break;
    }
    depth = depthmax;
    if(ok){
	if(!(ok = PrattBuild(tpratt, &ipratt, *smallp)))
	    printf("%%!%% Not a prime %ld\n", *smallp);
	else{
	    depth++;
	    PrattToList(&(tabcertif[depth]), smallp, tpratt, ipratt);
	}
    }
    return(ok);
}

/* Finds a quadratic field Q(sqrt(-D)) for which n=N_K(pi) and m=N_K(pi-1)
   is probably factored. Returns the factorization code */
int findm(n, nl, p_cert, gasp)
BigNum n;
int nl, gasp;
Certif *p_cert;
{
    FILE *fopen(), *file_of_DfD;
    ListOfFactors lfm, lfp;
    ListOfDAB l;
    ListOfABm labm, tmp, foo, dummy;
    long D, fD[TFMAX];
    BigNum copym = BNC(nl+2), sqrtd = BNC(nl+1); /* should be enough ? */
    int i, ml, done, h, g, w, all;

    /* for ease of testing/debugging */
    if(!depth && rgd0)
	rgd[0] = rgd0;
    else
	rgd[depth] = RGDMIN;
    file_of_DfD = fopen(dfd, "r");
#ifdef DISTRIB
    rgd[depth] = -1;
#else
    for(i = 0; i < rgd[depth]; i++)
	ExtractDfD(&D, fD, &h, &g, file_of_DfD);
#endif
    while(1){
	BmInit(n);
	if(!findD(&l, sqrtd, file_of_DfD))
	    return(0);
	tmp = l.ABm;
	if(verbose) printf("%% next D is %ld\n", l.D);
	if(jumpj) break;
	w = 0;
	while(tmp != NULL){
#ifdef DISTRIB
	    if(distecpp && DistAnythingNew()) return(-1);
#endif
#ifdef DEBUG
	    if((l.D > 1) && (debuglevel >= 1)){
		printf("%% next A is %s\n", Z2STR(tmp->A));
		printf("%% next B is %s\n", Z2STR(tmp->B));
	    }
#endif
	    if(l.D <= 1){
		if(!w) l.D = -1; else  l.D = 1;
	    }
	    /* sieves for +/-A at the same time */
	    if(!w){
		lfm = NULL; lfp = NULL;
		if(nl < HACK_SIEVE_MIN) all = 1; else all = 0;
		ecppsieve(n, nl, difnp, Sievepmax, l.D, tmp->A,&lfm,&lfp,all);
	    }
#ifdef DEBUG
	    if(debuglevel >= 1){
		printf("%% -> next m=");
		BnnPrint(tmp->m, nl+1);
		printf("\n");
	    }
#endif
	    /* here is nl instead of Modul, because Modul can be modified */
	    ml = BnnNumDigits(tmp->m, nl+1);
	    BnnAssign(copym, tmp->m, ml);
	    BnnSetToZero((copym+ml), nl-ml+2);
	    if(!w) tmp->lfact = lfm; else tmp->lfact = lfp;
	    if((l.D <= 1) && ((!w && nonminus1) || (w && nonplus1))){
		if(verbose){
		    if(!w && nonminus1) printf("%% No N-1 test\n");
		    else printf("%% No N+1 test\n");
		}
		done = 0;
	    }
	    else
		done=factorm(&(tmp->lfact),copym,ml,n,nl,
			     tmp->A,tmp->B,l.D,difnp,all,gasp);
	    if(done){
		break;
	    }
	    else{
		foo = tmp;
		tmp = tmp->cdr;
		foo->cdr = NULL;
		ListOfABmFree(l.D, foo);
		if(!w){
		    /* This means that we just explored N+1-A */
		    lfm = NULL;
		}
		else{
		    lfp = NULL;
		}
		w = 1-w;
#ifdef DISTRIB
		if(distecpp && DistAnythingNew()) return(-1);
#endif
	    }
	}
	if(tmp != NULL)
	    /* this means that done is nonzero */
	    break;
	else{
	    /* you might clean l, maybe */
	}
    }
    /* At this point, we should have enough results */
    p_cert->D = l.D;
    p_cert->h = l.h;
    p_cert->g = l.g;
    if(l.D > 1){
	p_cert->A = tmp->A;
	p_cert->B = tmp->B;
	p_cert->sqrtd = sqrtd;
	p_cert->m = tmp->m;
	p_cert->ml = BnnNumDigits(tmp->m, nl+1); /* ml;*/
    }
    p_cert->lfact = tmp->lfact;
    p_cert->lfacto = NULL;
    ListOfABmFree(l.D, tmp->cdr);
    fclose(file_of_DfD);
    BnFree(copym);
    if(l.D <= 1) BnFree(sqrtd);
    return(done);
}

ListOfABmFree(D, l)
long D;
ListOfABm l;
{
    ListOfABm foo;

    while(l != NULL){
	foo = l; l = l->cdr; foo->cdr = NULL;
	if(D > 1){
	    BzFree(foo->A);
	    BzFree(foo->B);
	}
	BnFree(foo->m);
	ListOfFactorsFree(foo->lfact);
	free((char *)foo);
    }
}

/********** utilities, is it the right place ? **********/
ListOfLongFree(l)
ListOfLong l;
{
    ListOfLong foo;

    while(l != NULL){
	/* free the pointer on l */
	foo = l->cdr;
	l->cdr = NULL;
	free((ListOfLong *)l);
	l = foo;
    }
}
