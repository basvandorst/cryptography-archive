/* vsign.c (verify signatures) - Mark Henderson (markh@wimsey.bc.ca) */
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"

main(int argc,char *argv[])
{
    MP_INT *e,*c,*n;
    char pubfile[128]; int i;
	unsigned char idstring[128];
    unsigned char hash[16];
    unsigned char sig[MAXC];
    FILE *public; 
    FILE *sigfile;
    init(e);  init(n); init(c);
    if (argc < 3) {
        fprintf(stderr, "usage: vsign pubfile sigfile\n");
        exit(1);
    }
    
    strncpy(pubfile,argv[1],120);
    strcat(pubfile,".pub");

    if (!(public = mfopen(pubfile, "rb"))) {
        fprintf(stderr, "could not open %s for read\n", pubfile);
        exit(1);
    }
    
    if (!read_number(public,n)) {
		fprintf(stderr, "fatal error reading %s (001)\n",pubfile);
		exit(1);
	}
    if (!read_number(public,e)) {
		fprintf(stderr, "fatal error reading %s (002)\n",pubfile);
		exit(1);
	}
	if (!read_string(public,idstring)) {
		fprintf(stderr, "warning: public key in %s is not self signed\n",pubfile);
	}
	else {
		mdpkid(n,e,idstring,hash);
		if (!read_number(public,c) ||  !vsign(c,hash,n,e)) {
			fprintf(stderr, "public key in %s is self signed with an INVALID signature\n",pubfile);
			fprintf(stderr, "public key corruption detected\n");
			exit(1);
		}
		fprintf(stderr, "key is self signed -- verifying signature by %s\n", idstring);
	}
    if (!(sigfile = fopen(argv[2],"rb")))  {
        fprintf(stderr, "could not open %s for read\n", argv[2]);
        exit(1);
    }

    if (!read_number(sigfile,c)) {
		printf("corrupt file signature\n");
		exit(1);
	}

    mdfile(stdin, hash);    
    if (vsign(c,hash,n,e)) {
        printf("file signature verified\n");
    }
    else {
        printf("file signature did not verify\n");
        exit(1);
    }
    exit(0);
}
