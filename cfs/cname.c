/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * cfs cname - 1.3
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <ctype.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

/* following are never used - just so i can re-use the library */
int validhost;
char zerovect[]={0,0,0,0,0,0,0,0,0};
int cursecs=0;

char *gets();

main(argc,argv)
     int argc;
     char **argv;
{
	char *pw;
	char pword[256];
	char *getpassword();
	cfs_admkey k;
	cfskey kt;
	char *flg;
	char estr[1024];
	char *cstr;
	int i;
	int ciph=CFS_THREE_DES;
	int verb=0;
	
	fprintf(stderr,"WARNING: cname works only on old format CFS dirs\n");
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case 'v':
				verb++;
				break;
			    case '1':
				ciph=CFS_STD_DES;
				break;
			    case '3':
				ciph=CFS_THREE_DES;
				break;
			    case 'b':
				ciph=CFS_BLOWFISH;
				break;
			    case 'm':
				ciph=CFS_MACGUFFIN;
				break;
			    case 's':
				ciph=CFS_SAFER_SK128;
				break;
			    default:
				fprintf(stderr,"usage: cname [-3bmsv]\n");
				exit(1);
			}
	}
	if (argc!=0) {
		fprintf(stderr,"Usage: cname [-13bmsv]\n");
		exit(1);
	}
	if ((pw=getpassword("Key:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	strcpy(pword,pw);
	k.cipher=ciph;
	if (old_pwcrunch(pw,&k)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	copykey(&k,&kt);
	kt.smsize=LARGESMSIZE;
	if (((kt.primask=(char*) malloc(kt.smsize)) == NULL)
	    || ((kt.secmask=(char*) malloc(kt.smsize)) == NULL)) {
		fprintf(stderr,"No memory\n");
		exit(2);
	}
	genmasks(&kt);
	while (fgets(estr, sizeof(estr), stdin) != NULL) {
		if (estr[strlen(estr)-1] == '\n')
			estr[strlen(estr)-1] = '\0';
		cstr=decryptname(&kt,estr);
		if (verb)
			printf("%s -> ",estr);
		if ((cstr!=NULL) && printable(cstr))
			puts(cstr);
		else
			puts("???");
	}

}

int printable(s)
     char *s;
{
	while (*s)
		if (!isprint(*s++))
			return 0;
	return 1;
}


