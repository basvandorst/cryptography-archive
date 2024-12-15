/* genkey.c - Mark Henderson (markh@wimsey.bc.ca) */
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"

main(int argc,char *argv[])
{
    unsigned char *p1 = NULL; unsigned char *p0 = NULL;
    char outfile[128];
    unsigned char key[16];  /* for key to key */
    unsigned char iv[8];
    int bits,i,k;
    FILE *private, *public;
    MP_INT *e,*p,*q,*n;
    init(e); init(p); init(q); init(n);

    if (argc < 2) {
        fprintf(stderr, "usage error: %s file [bits] [randomdata...]\n",argv[0]);
        exit(1);
    }
    bits = 512; /* default */
    if (argc >= 3) {
        bits = atoi(argv[2]);
        if (bits < 512 || bits > 1920) {
            fprintf(stderr, "valid range for bits is 512-1920\n");
            fprintf(stderr, "usage error: %s file [bits]\n",argv[0]);
            exit(1);
        }
    }

    bits/=2;

    strncpy(outfile,argv[1],120);
    strcat(outfile,".prv");

    if (!(private = fopen(outfile, "wb"))) {
        fprintf(stderr, "could not open %s for write\n", outfile);
        exit(1);
    }
    strncpy(outfile,argv[1],120);
    strcat(outfile,".pub");

    if (!(public = fopen(outfile, "wb"))) {
        fprintf(stderr, "could not open %s for write\n", outfile);
        exit(1);
    }

    initrandom();
	for (i=3; i<argc;i++)
		if (argv[i] && (k=strlen(argv[i])))  {
			seedrandom((unsigned char *)argv[i],k);
			zap((unsigned char *)argv[i],k);
		}

    do {
        if (p0)
            free(p0);
        if (p1)
            free(p1);
        fprintf(stderr, "enter password for key -- end with <CR> \n");
        p0=getstring(1);
        fprintf(stderr, "enter again for verification\n");
        p1=getstring(1);
    } while(strcmp(p0,p1));

    mdstring(p0,strlen(p0),key);
	zap(p0,strlen(p0));; zap(p1,strlen(p1));
    for (i=0;i<8;i++)
        iv[i]=0;
    
    fprintf(stderr, "please enter a random string -- end with <CR>\n");
    (void)getstring(0);
    mpz_set_ui(e,65537);

    /* generate a key */
    genkey(p,q,e,bits);
    mpz_mul(n,p,q);

    write_number(public,n);
    write_number(public,e);
    write_numberx(private,p,key,iv);
    write_numberx(private,q,key,iv);
	zap(key,16);
    write_number(private,e);
    fclose(private); fclose(public); closerandom();
}
