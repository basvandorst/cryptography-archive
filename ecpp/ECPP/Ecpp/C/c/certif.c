
/* $Log:	certif.c,v $
 * Revision 1.1  91/09/20  14:52:54  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:27:17  morain
 * Initial revision
 *  */

/**********************************************************************

                        Managing the certificates

**********************************************************************/

#include "ecpp.h"
#include "bzio.h"

#define TOFILE(ofile, x, xl) \
{   fprintf((ofile), "%s", bobn); \
    BnnPrintByBlockToFile((ofile), (x), (xl), obase, MAXCOL, bol, eol); \
    fprintf((ofile), "%s\n", eobn);}

/* Prints the certificate in file of name certif_name */
PrintCertif(certif, tabcertif, ncertif)
certif_parametres certif;
Certif tabcertif[];
int ncertif;
{
    FILE *fopen(), *ofile;
    ListOfFactors foo;
    BigNum np;
    int npl, i, first, last;

    SetCertifStrings(certif.format);
    ofile = fopen(certif.file, certif.type);
    if(strcmp(certif.label, "")){
	if(!strcmp(certif.format, "lelisp"))
	    fprintf(ofile, "; ");
	fprintf(ofile, "%s\n", certif.label);
    }
    for(i = 0; i <= ncertif; i++){
	first = !i; last = (i == ncertif);
	if(!last) {np = tabcertif[i+1].p; npl = tabcertif[i+1].pl;}
	if(tabcertif[i].D > 1)
	    PrintCertifEc(ofile,certif.format,tabcertif[i],np,npl,first,last);
	else{
	    if(tabcertif[i].D == -1)
	       PrintCertifNminus1(ofile,certif.format,tabcertif[i],first,last);
	    else
	       PrintCertifNplus1(ofile,certif.format,tabcertif[i],first,last);
	}
    }
    fclose(ofile);
}

SetCertifStrings(certif_format)
char certif_format[];
{
    sprintf(bol, "");
    sprintf(eol, "\\");
    if(!strcmp(certif_format, "lelisp")){
	sprintf(bobn, "%s", "#B");
	sprintf(eobn, "%s", " ");
    } 
    else{
	if(!strcmp(certif_format, "c")){
	    sprintf(bobn, "%s", "");
	    sprintf(eobn, "%s", "");
	}
	else{
	    if(!strcmp(certif_format, "maple")){
		sprintf(bobn, "%s", "");
		sprintf(eobn, "%s", "");
	    }
	    else{
		if(!strcmp(certif_format, "mathematica")){
		    sprintf(bobn, "%s", "");
		    sprintf(eobn, "%s", "");
		}
	    }
	}
    }
}

PrintCertifNminus1(ofile, certif_format, cert, first, last)
FILE *ofile;
char certif_format[];
Certif cert;
int first, last;
{
    if(!strcmp(certif_format, "lelisp"))
	PrintCertifNminus1ForLelisp(ofile, cert, first, last);
    else{
	if(!strcmp(certif_format, "maple")){
	    PrintCertifNminus1ForMaple(ofile, cert, first, last);
	}
	else{
	    if(!strcmp(certif_format, "mathematica")){
		PrintCertifNminus1ForMathematica(ofile, cert, first, last);
	    }
	    else{
		if(!strcmp(certif_format, "c")){
		    PrintCertifNminus1ForC(ofile, cert, first, last);
		}
	    }
	}
    }
}

PrintCertifNplus1(ofile, certif_format, cert, first, last)
FILE *ofile;
char certif_format[];
Certif cert;
int first, last;
{
    if(!strcmp(certif_format, "lelisp"))
	PrintCertifNplus1ForLelisp(ofile, cert, first, last);
    else{
	if(!strcmp(certif_format, "maple")){
	    PrintCertifNplus1ForMaple(ofile, cert, first, last);
	}
	else{
	    if(!strcmp(certif_format, "mathematica")){
		PrintCertifNplus1ForMathematica(ofile, cert, first, last);
	    }
	    else{
		if(!strcmp(certif_format, "c")){
		    PrintCertifNplus1ForC(ofile, cert, first, last);
		}
	    }
	}
    }
}

PrintCertifEc(ofile, certif_format, cert, nextp, nextpl, first, last)
FILE *ofile;
char certif_format[];
Certif cert;
BigNum nextp;
int nextpl, first, last;
{
    if(!strcmp(certif_format, "lelisp"))
	PrintCertifEcForLelisp(ofile, cert, first, last);
    else{
	if(!strcmp(certif_format, "maple")){
	    PrintCertifEcForMaple(ofile, cert, first, last);
	}
	else{
	    if(!strcmp(certif_format, "mathematica")){
	       PrintCertifEcForMathematica(ofile,cert,nextp,nextpl,first,last);
	    }
	    else{
		if(!strcmp(certif_format, "c")){
		    PrintCertifEcForC(ofile, cert, first, last);
		}
	    }
	}
    }
}

/********** Lelisp **********/

PrintCertifNminus1ForLelisp(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
}

PrintCertifNplus1ForLelisp(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
}

PrintCertifEcForLelisp(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    ListOfFactors foo;
    int j;
    
    TOFILE(ofile, cert.p, cert.pl);
    fprintf(ofile, "%ld\n", cert.D);
    fprintf(ofile, "%d\n", cert.h);
    fprintf(ofile, "(0. . 0.)\n");
    TOFILE(ofile, cert.m, cert.ml);
    /* prints list of factors of m, except the largest one */
    foo = cert.lfact;
    while(foo != NULL){
	for(j = 0; j < foo->e; j++)
	    TOFILE(ofile, foo->p, foo->pl);
	foo = foo->cdr;
    }
    fprintf(ofile, "0\n");
    /* ea, eb */
    TOFILE(ofile, cert.ea, cert.eal);
    TOFILE(ofile, cert.eb, cert.ebl);
    /* x, y */
    TOFILE(ofile, cert.px, cert.pxl);
    TOFILE(ofile, cert.py, cert.pyl);
    /* factors of the order of P */
    foo = cert.lfacto;
    while(foo != NULL){
	for(j = 0; j < foo->e; j++)
	    TOFILE(ofile, foo->p, foo->pl);
	foo = foo->cdr;
    }
    fprintf(ofile, "0\n");
    if(!last) fprintf(ofile, "\n"); else fprintf(ofile, "0\n\n");
}

/********** Maple **********/
#define TOMAPLE(ofile, x, xl) \
{   BnnPrintByBlockToFile((ofile), (x), (xl), obase, MAXCOL, bol, eol); \
    fprintf((ofile), ",\n");}

/* if all = 1, then prints [[p1, e1], ..., [pk, ek]], otherwise [p1, ..., pk]*/
MaplePrintListOfFactors(ofile, lf, all)
FILE *ofile;
ListOfFactors lf;
int all;
{
    fprintf(ofile, "  [\n");
    while(lf != NULL){
	if(all) fprintf(ofile, "[\n");
	TOFILE(ofile, lf->p, lf->pl);
	if(all){
	    fprintf(ofile, ", %d", lf->e);
	    fprintf(ofile, "]\n");
	}
	lf = lf->cdr;
	if(lf != NULL) fprintf(ofile, ",\n");
    }
    fprintf(ofile, "  ]\n");
}

PrintCertifNminus1ForMaple(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    if(first) fprintf(ofile, "lprint(`Next certificate`):\n[\n");
    if(last)
	PrintPrattForMaple(ofile, cert);
    else{
	fprintf(ofile, " [\n");
	TOMAPLE(ofile, cert.p, cert.pl);
	fprintf(ofile, "%ld, %d,\n", cert.D, cert.h);
	/* factors of N-1 */
	MaplePrintListOfFactors(ofile, cert.lfact, 1);
	fprintf(ofile, ",\n");
	/* the a_i's */
	MaplePrintListOfFactors(ofile, cert.lfacto, 0);
	fprintf(ofile, " ]");
	if(last) fprintf(ofile, "\n]:\nCheckCertificate(\");\n"); 
	else fprintf(ofile, ",\n");
    }
}

PrintCertifNplus1ForMaple(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    if(first) fprintf(ofile, "lprint(`Next certificate`):\n[\n");
    fprintf(ofile, " [\n");
    TOMAPLE(ofile, cert.p, cert.pl);
    fprintf(ofile, "%ld, %d,\n", cert.D, cert.h);
    /* factors of N+1 */
    MaplePrintListOfFactors(ofile, cert.lfact, 1);
    fprintf(ofile, ",\n");
    if(cert.h == 1){
	/* print u or a */
	TOMAPLE(ofile, cert.ea, 1);
	/* print m_i's */
	MaplePrintListOfFactors(ofile, cert.lfacto, 0);
    }
    else printf("NYI\n");
    fprintf(ofile, " ]");
    if(last) fprintf(ofile, "\n]:\nCheckCertificate(\");\n");
    else fprintf(ofile, ",\n");
}

PrintCertifEcForMaple(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    ListOfFactors foo;

    if(first) fprintf(ofile, "lprint(`Next certificate`):\n[\n");
    fprintf(ofile, " [\n");
    TOMAPLE(ofile, cert.p, cert.pl);
    fprintf(ofile, "%ld, %d,\n", cert.D, cert.h);
    TOMAPLE(ofile, cert.m, cert.ml);
    /* ea, eb */
    fprintf(ofile, "  [\n");
    TOMAPLE(ofile, cert.ea, cert.eal);
    TOFILE(ofile, cert.eb, cert.ebl);
    fprintf(ofile, "  ],\n");
    /* x, y */
    fprintf(ofile, "  [\n");
    TOMAPLE(ofile, cert.px, cert.pxl);
    TOMAPLE(ofile, cert.py, cert.pyl);
    fprintf(ofile, "1\n  ],\n");
    /* table of factors */
    MaplePrintListOfFactors(ofile, cert.lfacto, 1);
    fprintf(ofile, " ]");
    if(last) fprintf(ofile, "\n]:\nCheckCertificate(\");\n"); 
    else fprintf(ofile, ",\n");
}

PrintPrattForMaple(ofile, cert)
FILE *ofile;
Certif cert;
{
    ListOfFactors lf;
    int i;

    fprintf(ofile, "[");
    TOMAPLE(ofile, cert.p, cert.pl);
    fprintf(ofile, "-1, 0, ");
    i = 0;
    lf = cert.lfacto;
    PrintPrattForMapleAux(ofile, &lf);
    fprintf(ofile, "]]:\nCheckCertificate(\");\n");
}

#define NEXTL(lg, l) {(lg) = *((l)->p); (l) = (l)->cdr;}

PrintPrattForMapleAux(ofile, p_lf)
FILE *ofile;
ListOfFactors *p_lf;
{
    long p, g, nq;
    int j;

    NEXTL(p, *p_lf);
    NEXTL(g, *p_lf);
    fprintf(ofile, "[%ld, %ld", p, g);
    if(p != 2){
	fprintf(ofile, ", [\n");
	NEXTL(nq, *p_lf);
	for(j = 0; j < nq; j++){
	    PrintPrattForMapleAux(ofile, p_lf);
	    if(j < nq-1) fprintf(ofile, ",\n");
	}
	fprintf(ofile, "]\n");
    }
    fprintf(ofile, "]\n");
	
}

/********** Mathematica **********/

int PrintCertifNminus1ForMathematica(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    if(last){
	if(first) fprintf(ofile, "{");
	PrintPrattForMathematica(ofile, cert);
	fprintf(ofile, "}}\n");
	return(1);
    }
    else{
	printf("%%!%% Not implemented\n");
	return(0);
    }
}

PrintPrattForMathematica(ofile, cert)
FILE *ofile;
Certif cert;
{
    ListOfFactors lf;
    int i;

    i = 0;
    lf = cert.lfacto;
    PrintPrattForMathematicaAux(ofile, &lf);
}

PrintPrattForMathematicaAux(ofile, p_lf)
FILE *ofile;
ListOfFactors *p_lf;
{
    long p, g, nq;
    int j;

    NEXTL(p, *p_lf);
    NEXTL(g, *p_lf);
    if(p == 2){
        fprintf(ofile, "2");
      }
    else{
        fprintf(ofile, "%ld, %ld, {", p, g);
	NEXTL(nq, *p_lf);
	for(j = 0; j < nq; j++){
	    PrintPrattForMathematicaAux(ofile, p_lf);
	    if(j < nq-1) fprintf(ofile, ", {");
	}
	fprintf(ofile, "}");
    }
}

PrintCertifNplus1ForMathematica(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
}

PrintCertifEcForMathematica(ofile, cert, nextp, nextpl, first, last)
FILE *ofile;
Certif cert;
BigNum nextp;
int nextpl, first, last;
{
    BigNum k = BNC(cert.pl+2);
    int kl;

    if(first) fprintf(ofile, "{\n");
    fprintf(ofile, "{CertificatePrime -> ");
    TOMAPLE(ofile, cert.p, cert.pl);
    fprintf(ofile, "CertificatePoint -> PointEC[");
    TOMAPLE(ofile, cert.px, cert.pxl);
    TOMAPLE(ofile, cert.py, cert.pyl);
    TOMAPLE(ofile, cert.ea, cert.eal);
    TOMAPLE(ofile, cert.eb, cert.ebl);
    TOFILE(ofile, cert.p, cert.pl);
    fprintf(ofile, "],\n");
    if(!last){
	BnnAssign(k, cert.m, cert.ml);
	kl = cert.ml;
	BnnDivide(k, kl+1, nextp, nextpl);
	fprintf(ofile, "CertificateK -> ");
	kl = BnnNumDigits((k+nextpl), kl-nextpl+1);
	TOMAPLE(ofile, (k+nextpl), kl);
	fprintf(ofile, "CertificateNextPrime -> ");
	TOMAPLE(ofile, nextp, nextpl);
    }
    fprintf(ofile, "CertificateM -> ");
    TOMAPLE(ofile, cert.m, cert.ml);
    fprintf(ofile, "CertificateDiscriminant -> -%ld}\n", cert.D);
    if(last) fprintf(ofile, "}\n"); else fprintf(ofile, ", ");
    BnFree(k);
}

/********** C **********/

PrintCertifNminus1ForC(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    ListOfFactors foo;

    TOFILE(ofile, cert.p, cert.pl);
    /* type of Theorem used */
    fprintf(ofile, "%ld\n%d\n", cert.D, cert.h);
    if(!cert.h){
	/* This is a Pratt stuff */
	foo = cert.lfacto;
	while(foo != NULL){
	    TOFILE(ofile, foo->p, foo->pl);
	    foo = foo->cdr;
	}
    }
    else{
	foo = cert.lfact;
	while(foo != NULL){
	    TOFILE(ofile, foo->p, foo->pl);
	    foo = foo->cdr;
	}
	fprintf(ofile, "0\n");
	if(cert.h == 3){
	    /* print a */
	    TOFILE(ofile, cert.lfacto->p, 1);
	}
	else{
	    /* Theorem 1 was used, prints factors of N-1, the a_i's */
	    /* printing the a_i's */
	    foo = cert.lfacto;
	    while(foo != NULL){
		TOFILE(ofile, foo->p, foo->pl);
		foo = foo->cdr;
	    }
	}
    }
    fprintf(ofile, "0\n");
    if(!last) fprintf(ofile, "\n"); else fprintf(ofile, "0\n");
}

PrintCertifNplus1ForC(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    ListOfFactors foo;

    TOFILE(ofile, cert.p, cert.pl);
    /* type of Theorem used */
    fprintf(ofile, "%ld\n%d\n", cert.D, cert.h);
    foo = cert.lfact;
    while(foo != NULL){
	TOFILE(ofile, foo->p, foo->pl);
	foo = foo->cdr;
    }
    fprintf(ofile, "0\n");
    if(cert.h == 1){
	/* print u or a */
	TOFILE(ofile, cert.ea, 1);
	/* print m_i's */
	foo = cert.lfacto;
	while(foo != NULL){
	    TOFILE(ofile, foo->p, foo->pl);
	    foo = foo->cdr;
	}
    }
    else printf("NYI\n");
    fprintf(ofile, "0\n");
    if(!last) fprintf(ofile, "\n"); else fprintf(ofile, "0\n");
}

PrintCertifEcForC(ofile, cert, first, last)
FILE *ofile;
Certif cert;
int first, last;
{
    ListOfFactors foo;
    int j;
    
    TOFILE(ofile, cert.p, cert.pl);
    fprintf(ofile, "%ld\n", cert.D);
    fprintf(ofile, "%d\n", cert.h);
    TOFILE(ofile, cert.m, cert.ml);
    /* prints list of factors of m, except the largest one */
    foo = cert.lfact;
    while(foo != NULL){
	for(j = 0; j < foo->e; j++)
	    TOFILE(ofile, foo->p, foo->pl);
	foo = foo->cdr;
    }
    fprintf(ofile, "0\n");
    /* ea, eb */
    TOFILE(ofile, cert.ea, cert.eal);
    TOFILE(ofile, cert.eb, cert.ebl);
    /* x, y */
    TOFILE(ofile, cert.px, cert.pxl);
    TOFILE(ofile, cert.py, cert.pyl);
    /* factors of the order of P */
    foo = cert.lfacto;
    while(foo != NULL){
	for(j = 0; j < foo->e; j++)
	    TOFILE(ofile, foo->p, foo->pl);
	foo = foo->cdr;
    }
    fprintf(ofile, "0\n");
    if(!last) fprintf(ofile, "\n"); else fprintf(ofile, "0\n\n");
}

CertifFree(p_cert)
Certif *p_cert;
{
    BnFree(p_cert->p); p_cert->p = NULL;
    if(p_cert->D > 1){
	BzFree(p_cert->A); p_cert->A = NULL;
	BzFree(p_cert->B); p_cert->B = NULL;
	BnFree(p_cert->sqrtd); p_cert->sqrtd = NULL;
	BnFree(p_cert->m);
    }
    ListOfFactorsFree(p_cert->lfact);
    if(!noproof){
	if(p_cert->D > 1){
	    BnFree(p_cert->ea); p_cert->ea = NULL;
	    BnFree(p_cert->eb); p_cert->eb = NULL;
	    BnFree(p_cert->px); p_cert->px = NULL;
	    BnFree(p_cert->py); p_cert->py = NULL;
	    ListOfFactorsFree(p_cert->lfacto);
	}
    }
}
