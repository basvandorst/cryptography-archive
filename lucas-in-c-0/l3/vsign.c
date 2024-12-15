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
    
    read_number(public,n);
    read_number(public,e);
    if (!(sigfile = fopen(argv[2],"rb")))  {
        fprintf(stderr, "could not open %s for read\n", argv[2]);
        exit(1);
    }

    read_number(sigfile,c);

    mdfile(stdin, hash);    
    if (vsign(c,hash,n,e)) {
        printf("signature verified\n");
    }
    else {
        printf("signature did not verify\n");
        exit(1);
    }
    exit(0);
}
