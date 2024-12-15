/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.  You may
 * not export it, in whole or in part, or cause or allow such export,
 * through act or omission, without prior authorization from the United
 * States government and written permission from AT&T.  In particular,
 * you may not make any part of this software available for general or
 * unrestricted distribution to others, nor may you disclose this software
 * to persons other than citizens and permanent residents of the United
 * States and Canada. 
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * cfs cname - 1.1
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
	char *estr[1024];
	char *cstr;
	int i;
	int ciph=CFS_STD_DES;
	int verb=0;
	
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case 'v':
				verb++;
				break;
			    case '3':
				ciph=CFS_THREE_DES;
				break;
			    default:
				fprintf(stderr,"usage: cname [-3]\n");
				exit(1);
			}
	}
	if (argc!=0) {
		fprintf(stderr,"Usage: cname [-3]\n");
		exit(1);
	}
	if ((pw=getpassword("Key:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	strcpy(pword,pw);
	k.cipher=ciph;
	if (pwcrunch(pw,&k)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	copykey(&k,&kt);
	genmasks(&kt);
	while (gets(estr) != NULL) {
		cstr=decryptname(&kt,estr);
		if (verb)
			printf("%s -> ",estr);
		if (printable(cstr))
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


