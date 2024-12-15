/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
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
 * client cfs mkdir - 1.1
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

main(argc,argv)
     int argc;
     char **argv;
{
	char *pw;
	char pword[256];
	char *getpassword();
	cfs_admkey k;
	cfskey kt;
	char path[1024];
	char str[8];
	FILE *fp;
	char *flg;
	struct timeval tv;
	u_long r;
	int i;
	int ciph=CFS_STD_DES;
	
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case '3':
				ciph=CFS_THREE_DES;
				break;
			    default:
				fprintf(stderr,"usage: cmkdir [-3] dir\n");                               exit(1);
			}
	}
	if (argc!=1) {
		fprintf(stderr,"Usage: cmkdir [-3] dir\n");
		exit(1);
	}
	if ((pw=getpassword("Key:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	strcpy(pword,pw);
	if (strlen(pw)<16) {
		fprintf(stderr,"Key must be at least 16 chars.\n");
		exit(1);
	}
	if ((pw=getpassword("Again:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	if (strcmp(pword,pw)!=0) {
		fprintf(stderr,
			"Keys don't match; drink some coffee and try again\n");
		exit(1);
	}
	k.cipher=ciph;
	if (pwcrunch(pw,&k)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	if (mkdir(argv[0],0777)<0) {
		perror("cmkdir");
		exit(1);
	}
	sprintf(path,"%s/...",argv[0]);
	strcpy(str,"qua!");
	/* now randomize the end of str.. */
	gettimeofday(&tv,NULL);
	r=(getpid()<<16)+tv.tv_sec;
	for (i=0; i<4; i++)
		str[i+4]=(r<<(i*8))&0377;
	copykey(&k,&kt);
	cipher(&kt,str,0);
	mask_cipher(&kt,str,1);
	cipher(&kt,str,0);
	if ((fp=fopen(path,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	fwrite(str,8,1,fp);
	fclose(fp);
	sprintf(path,"%s/..c",argv[0]);
	if ((fp=fopen(path,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	fprintf(fp,"%d",k.cipher);
	fclose(fp);
	exit(0);
}
