/* encrypt.c - Mark Henderson (markh@wimsey.bc.ca) */
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"

main(int argc,char *argv[])
{
    char pubfile[128]; unsigned int i,k;
    unsigned char key[48], cipher[MAXC];
    FILE *public;
    MP_INT *e,*n,*c;
    init(e); init(n); init(c);

    if (argc < 2) {
        fprintf(stderr, "usage: encrypt pubfile [randomdata...] \n");
        exit(1);
    }
    
    strncpy(pubfile,argv[1],120);
    strcat(pubfile,".pub");

    if (!(public = mfopen(pubfile, "rb"))) {
        fprintf(stderr, "could not open %s for read\n", pubfile);
        exit(1);
    }

    initrandom();
	for (i=2; i<argc;i++)
		if (argv[i] && (k=strlen(argv[i])))  {
			seedrandom((unsigned char *)argv[i],k);
			zap((unsigned char *)argv[i],k);
		}
    fprintf(stderr, "please enter a random string -- end with <CR>\n");
    (void)getstring(0);
    for (i=0;i<48;i++)
        key[i] = randombyte();

    read_number(public,n);
    read_number(public,e);
    encrypt_session_key(c,key,n,e);
    write_number(stdout,c);
    desfile(stdin,stdout,&key[0], &key[40], EN0);
	zap(key,48); closerandom();
}

