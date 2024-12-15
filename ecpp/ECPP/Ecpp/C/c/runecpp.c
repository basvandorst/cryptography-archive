
/* $Log:	runecpp.c,v $
 * Revision 1.1  91/09/20  14:55:29  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:30:24  morain
 * Initial revision
 *  */

#include "ecpp.h"
#include "version.h"

char s[ECPPSTRMAX];
BigZ z;
BigNum n;
int nl, grgd0, hyperquiet, norabin;
double tp, tot;
extern double runtime();
certif_parametres certif_infos;
Certif tabcert[PROFMAX];

#define GETOPTDF "hl:f:A:C:F:cNn:qQr:t:v"
#ifdef DEBUG
#define GETOPTDG "d:j"
#else
#define GETOPTDG ""
#endif
#ifdef DISTRIB
#define GETOPTDS "D:MS"
#else
#define GETOPTDS ""
#endif

/**********************************************************************
                             TESTING
**********************************************************************/

testpford(ifile, certif)
FILE *ifile;
certif_parametres certif;
{
    long D;
    int rg, h, g, ncert;

    tp = runtime();
    while(fscanf(ifile, "%d %ld %d %d", &rg, &D, &h, &g) != EOF){
	fscanf(ifile, "%s", s);
	z = BzFromString(s, 10);
	n = BzToOldBigNum(z, &nl);
	printf("p[%6ld]=%11s:", D, s);
	rgd0 = rg;
	if(ecpp(tabcert, &ncert, n, nl))
	    printf("1\n");
	else
	    printf("0\n");
	FreeCertifTab(tabcert);
	BnFree(n); BzFree(z);
    }
    if(!hyperquiet){
	tp = (runtime()-tp)/1000.;
	printf("%% Time for the test is %lfs\n", tp);
    }
}

/**********************************************************************
                                    RUNNING
**********************************************************************/

/* Cleaning all kind of junk */
FreeCertifTab(tabcertif)
Certif tabcertif[];
{
    int i;

    /* clearing tabcertif */
    for(i = 0; i < depth; i++)
	CertifFree(&(tabcertif[i]));
    /* Praticular case for the last one, the Pratt stuff */
    ListOfFactorsFree(tabcertif[depth].lfacto);
}

runecppfromfile(ifile, certif)
FILE *ifile;
certif_parametres certif;
{
    int ncert;
    char difnp[200];
    long why;

    tot = runtime();
    while(1){
	if(ifile == stdin) printf("Enter number to test:\n");
	if(fscanf(ifile, "%s", s) == EOF) break;
	if(s[0] == '%')
	    /* this is a label */
	    sprintf(certif.label, "%s", s);
	else{
	    z = BzFromString(s, 10);
	    n = BzToOldBigNum(z, &nl);
	    BzFree(z);
	    if(!hyperquiet){
		printf("Working on ");
		if(strcmp(certif.label, "")) printf("%s\n", certif.label);
		else printf("%s\n", s);
	    }
	    tp = runtime();
	    if(!norabin){
		if(verbose) 
		    printf("%% Performing a quick compositeness test\n");
		sprintf(difnp, "%s%s", path, DIFNP);
		if(!IsProbablePrimeWithSieve(n, nl, difnp, 10000, &why)){
		    if(!hyperquiet){
			printf("%% This number is composite ");
			if((why == 1) && (!BnnIsOne(n, nl)))
			    printf("by Miller-Rabin\n");
			else
			    printf("because it has a small factor: %ld\n",why);
			printf("\n");
		    }
		    else {printf("0"); fflush(stdout);}
		    BnFree(n);
		    continue;
		}
		else
		    if(!hyperquiet) printf("%% This number might be prime\n");
	    }
	    rgd0 = grgd0;
	    if(ecpp(tabcert, &ncert, n, nl)){
		if(!hyperquiet){
		    printf("This number is prime\n");
		}
		else {printf("1"); fflush(stdout);}
		if(certif.print) PrintCertif(certif, tabcert, ncert);
	    }
	    else{
		if(!hyperquiet)	printf("This number is composite\n");
		else {printf("0"); fflush(stdout);}
	    }
	    FreeCertifTab(tabcert);
	    tp = runtime()-tp;
	    if(!hyperquiet) 
		printf("%% Time for this number is %lfs\n\n", tp/1000.);
	    BnFree(n);
	}
    }
    tot = runtime()-tot;
    if(!hyperquiet){
	printf("%% ==> Total time for the computations is %lfs\n", tot/1000.);
    }
    else printf("\n");
}

PrintCenteredString(s, width)
char s[];
int width;
{
    int sl = strlen(s), i, s0;

    sl = (width-sl-2);
    s0 = sl%2;
    sl >>= 1;
    printf("%%");
    for(i = 0; i < sl; i++)
	printf(" ");
    printf("%s", s);
    for(i = 0; i < sl+s0; i++)
	printf(" ");
    printf("%%\n");
}

PrintBeautifulHeader()
{
    int width = 60, i;
    char v[200];

    for(i = 0; i < width; i++)
	printf("%%");
    printf("\n");
    PrintCenteredString(" ", width);
    PrintCenteredString("ECPP", width);
    PrintCenteredString(" ", width);
    PrintCenteredString("by Fran\\c{c}ois MORAIN", width);
    PrintCenteredString("morain@inria.inria.fr", width);
    sprintf(v, "Version %s", VERSION);
    PrintCenteredString(v, width);
    PrintCenteredString(" ", width);
    PrintCenteredString("\"3 is prime, 5 is prime, 7 is prime", width);
    PrintCenteredString("so every odd number is prime\"", width);
    PrintCenteredString(" ", width);
    for(i = 0; i < width; i++)
	printf("%%");
    printf("\n");
}

main(argc, argv)
int argc;
char *argv[];
{
    FILE *fopen(), *ifile;
    int nbgcd, p, i, pford = 0;
    char cm, co, pfile[50], input_file[200], getopt_str[100];
    int c;
    extern char *optarg;
    extern int  optind, opterr;

    setlinebuf(stdout);
    if(argc == 1){
      usage:
	fprintf(stderr, "Usage: %s \n", argv[0]);
	fprintf(stderr, "-h          help\n");
	fprintf(stderr, "-l<path>    where Data/ is\n");
	fprintf(stderr, "-f<file>    input file; if you want to enter ");
	fprintf(stderr, "numbers from stdin, use -f -\n");
	fprintf(stderr, "-A[file]    appends certificate to file\n");
	fprintf(stderr, "-C[file]    puts certificate in file\n");
	fprintf(stderr, "-F<format>  format of the certificate among: ");
	fprintf(stderr, "c, maple, mathematica\n");
	fprintf(stderr, "-cyc        uses cyclic curves only\n");
#ifdef DEBUG
	fprintf(stderr, "-d#         sets debuglevel to #\n");
	fprintf(stderr, "-j          jumps to the computation of j\n");
#endif
	fprintf(stderr, "-r#         sets initial D rank at #\n");
	fprintf(stderr, "-N          do not perform the proving part\n");
	fprintf(stderr, "-nm         do not use the N-1 test\n");
	fprintf(stderr, "-np         do not use the N+1 test\n");
	fprintf(stderr, "-nr         do not perform Rabin's test ");
	fprintf(stderr, "(valid only with - as input)\n");
#ifdef DISTRIB
	fprintf(stderr, "-Dname      distributed mode\n");
	fprintf(stderr, "-M          use as master\n");
	fprintf(stderr, "-S          use as slave\n");
#endif
	fprintf(stderr, "-t<file>    uses a (rank D h g p) formatted file");
	fprintf(stderr, " for testing the D-polynomial\n");
	fprintf(stderr, "-v          verbose mode\n");
	fprintf(stderr, "-q          quiet mode\n");
	fprintf(stderr, "-Q          super quiet mode\n");
	exit(0);
    }
    else{
	hyperquiet = 0;
	norabin = 0;
	certif_infos.print = 0;
	sprintf(certif_infos.format, "c");
	sprintf(input_file, "");
	opterr = 0;
	sprintf(getopt_str, "%s%s%s", GETOPTDF, GETOPTDG, GETOPTDS);
#ifdef DEBUG
	if(debuglevel >= 10) printf("%% getopt_str=%s\n", getopt_str);
#endif
#ifdef DISTRIB
	distecpp = 0;
#endif
        while((c = getopt(argc, argv, getopt_str)) != EOF){
	    switch(c){
	      case 'h':
		goto usage;
	      case 'l':
		sprintf(path, "%s", optarg);
		break;
	      case 'f':
		sprintf(input_file, "%s", optarg);
		break;
	      case 'A':
		certif_infos.print = 1;
		sprintf(certif_infos.file, "%s", optarg);
		sprintf(certif_infos.type, "a");
		break;
	      case 'C':
		certif_infos.print = 1;
		sprintf(certif_infos.file, "%s", optarg);
		sprintf(certif_infos.type, "w");
		break;
	      case 'F':
		sprintf(certif_infos.format, "%s", optarg);
		break;
	      case 'c':
		cyclic = 1;
		break;
	      case 'N':
		noproof = 1;
		break;
	      case 'n':
		sscanf(optarg, "%c", &cm);
		switch (cm){
		  case 'm':
		    nonminus1 = 1;
		    break;
		  case 'p':
		    nonplus1 = 1;
		    break;
		  case 'r':
		    norabin = 1;
		    break;
		  default:
		    goto usage;
		}
		break;
#ifdef DISTRIB
	      case 'D':
		sprintf(distname, "%s", optarg);
		distecpp = 1;
		break;
	      case 'M':
		distmaster = 1;
		break;
	      case 'S':
		distmaster = 0;
		break;
#endif
	      case 'q':
		verbose = 0;
		break;
	      case 'Q':
		verbose = 0;
		hyperquiet = 1;
		break;
	      case 'r':
		sscanf(optarg, "%d", &grgd0);
		printf("rgd0=%d\n", grgd0);
		break;
	      case 't':
		sprintf(pfile, "%s", optarg);
		pford = 1;
		break;
	      case 'v':
		verbose = 1;
		break;
#ifdef DEBUG
	      case 'd': 
		sscanf(optarg, "%d", &debuglevel);
		printf("debuglevel=%d\n", debuglevel);
		break;
	      case 'j':
		jumpj = 1;
		break;
#endif
	      case '?':
		fprintf(stderr, "Unknown option %s\n", argv[optind-1]);
		goto usage;
	    }
	}
	if(!strlen(path))
	    goto usage;
#ifdef DISTRIB
	if(distecpp){
	    DistGo();
	    exit(0);
	}
#endif
	if(!pford && !strlen(input_file))
	    goto usage;
	if(verbose){
	    PrintBeautifulHeader();
	}
	if(pford){
            if((ifile = fopen(pfile, "r")) == NULL){
		fprintf(stderr, "No such file: %s\n", pfile);
		exit(1);
	    }
	    else{
		testpford(ifile, certif_infos);
		fclose(ifile);
	    }
	}
	else{
	    if(!strcmp(input_file, "-"))
		ifile = stdin;
	    if((ifile != stdin) && ((ifile = fopen(input_file, "r")) == NULL)){
		    fprintf(stderr, "No such file: %s\n", input_file);
		    exit(1);
	    }
	    else{
		runecppfromfile(ifile, certif_infos);
		if(ifile != stdin) fclose(ifile);
	    }
	}
    }
}
